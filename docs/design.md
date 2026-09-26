# Design notes

This document explains the reasoning behind the engine's core design choices,
the alternatives that were considered and rejected, and the bugs found
during development — including how each one was caught and what that says
about the test suite.

## Why integer ticks, not `double`, for price

The first design considered was `std::map<double, std::queue<Order>>` for
each side of the book. Two problems ruled it out before any code was
written:

1. **Floating-point equality is unreliable.** In IEEE 754 arithmetic,
   `0.1 + 0.2 != 0.3`. Two orders meant to sit at the same price could end
   up as distinct keys in the map, silently splitting one price level into
   two and breaking price-time priority.
2. **`std::queue` cannot erase from the middle.** Cancellation needs to
   remove one specific resting order from wherever it sits in its price
   level's queue — not just the front. `std::queue` only exposes
   `front()`/`pop()`, so it can't do this at all without being drained and
   rebuilt, which would be O(n) per cancel.

The final design represents price as `int64_t` in **ticks** — `5205` means
$52.05 — so price comparison and map-key equality are always exact integer
comparisons. Each price level is a `std::list<Order>` instead of a queue,
because a `std::list` iterator into the middle of the container stays valid
after other elements are inserted or erased, which makes O(1) cancellation
possible (see below).

## Core data structures and why each was chosen

```cpp
std::map<Price, PriceLevel, std::greater<Price>> bids_;  // best bid = begin()
std::map<Price, PriceLevel>                      asks_;  // best ask = begin()
std::unordered_map<OrderId, Locator>             index_; // O(1) cancel lookup

struct PriceLevel {
    std::list<Order> orders;   // FIFO; O(1) erase from the middle via iterator
    Quantity totalQty = 0;
};

struct Locator {
    Side side;
    Price price;
    std::list<Order>::iterator it;   // points directly at the order's node
};
```

| Structure | Why | Complexity |
|---|---|---|
| `std::map` (red-black tree) per side | Needs the best price at all times. A tree keeps every level sorted, so the best bid/ask is always `begin()`, without a scan. | O(log P) insert/erase/lookup, P = distinct price levels |
| `std::list<Order>` per price level | Time priority (FIFO) requires an ordered queue, but cancellation needs to remove an order from anywhere in that queue, not just the front. A `std::list` supports O(1) erase given an iterator, and — critically — inserting or erasing elsewhere in the list never invalidates other iterators. | O(1) push_back / erase-by-iterator |
| `std::unordered_map<OrderId, Locator>` | Cancellation arrives with only an order ID. Without an index, finding that order would mean scanning every price level on both sides — O(n) in the worst case. The index stores exactly where the order lives (which side, which price, which list iterator), so cancel goes straight there. | O(1) average lookup/insert/erase |
| `Locator` storing a raw `std::list::iterator` | This is what makes the O(1) cancel path work at all: the iterator is a direct handle to the order's node, valid until that specific node is erased. If `PriceLevel` used a `std::vector` instead, this iterator would be invalidated by inserts/erases elsewhere in the vector, and the whole design would break. | — |

### Why not a flat array indexed by price tick (considered, not built)

A flat array (or a fixed-size ring buffer) indexed directly by price tick
would beat `std::map` on raw speed for a *known, bounded* price range — no
tree traversal, just array indexing, and better cache locality. This was
explicitly considered as a future optimization (see `docs/benchmarks.md`),
but not used from the start, for one concrete reason: **the synthetic
generator's anchor price is a random walk with no fixed bounds.** A flat
array needs its size (the price range it covers) decided up front. Committing
to that would mean either picking an arbitrary cap the generator could
eventually walk past (silently corrupting the book or crashing), or
resizing the array dynamically, which reintroduces most of the complexity
`std::map` already handles correctly. `std::map` was chosen first because it
is correct for an unbounded price range by construction; a flat array is a
targeted follow-up once the workload's realistic bounds are known, not a
default engine.

## Bugs found during development

These were all real defects introduced while building the engine, not
hypothetical examples. Each one is included because of *how* it was caught
— together they're the strongest evidence in this project that testing
mattered, not just that a matching engine was written.

### 1. Missing `totalQty` decrement (caught by the randomized invariant test, not by hand-written tests)

An early version of the partial-fill path in `matchAgainst` updated each
individual order's remaining quantity correctly, but forgot to update the
price level's running total:

```cpp
// Buggy version
incoming.qty -= fill;
resting.qty  -= fill;
// level.totalQty -= fill;   <-- missing

if (resting.qty == 0) {
    index_.erase(resting.id);
    level.orders.pop_front();
}
```

All 5 hand-written `Matching` tests passed anyway, because none of them
inspected `PriceLevel::totalQty` directly — they only checked trade
contents and `bestBid()`/`bestAsk()`. The bug was silent from the outside on
any of those specific scenarios.

It was caught by `Invariants.RandomOrderFlowKeepsBookConsistent`, which
tracks an independent model of expected resting quantity per order and
asserts `book.restingQuantity() == resting` after *every single operation*.
With the bug active, this failed on seed 1 at step 18 — one of the earliest
possible failures, since it only takes one partial fill to desync the
running total from reality. This is the concrete argument for writing a
model-based randomized test in addition to hand-picked cases: hand-written
tests only catch what you thought to check for.

### 2. Duplicate order IDs corrupting the cancel index

The original design had no check preventing two orders from being submitted
with the same ID. If order 1 (resting, unmatched) and a later order also
using ID 1 were both submitted, `rest()` would insert the second order into
`index_` under the same key, silently overwriting the `Locator` pointing at
order 1:

```cpp
index_.emplace(order.id, Locator{...});   // overwrites any existing entry silently
```

The practical effect: order 1 was still sitting in the book, taking up
space and eligible to match, but `cancelOrder(1)` would now find and erase
*order 2's* list node under order 1's ID — or, if order 2 had already
fully traded, find nothing and incorrectly report the cancel as failed, even
though order 1 was live.

Fixed by adding a validation pass before any order is matched or rested:

```cpp
RejectReason OrderBook::validateOrder(const Order& order) const {
    if (order.qty == 0) return RejectReason::ZeroQuantity;
    if (order.type == OrderType::Limit && order.price <= 0) return RejectReason::InvalidPrice;
    if (index_.find(order.id) != index_.end()) return RejectReason::DuplicateId;
    return RejectReason::None;
}
```

A duplicate ID is now rejected outright — the book is left completely
unchanged, and the caller is told why. A known limitation, documented
rather than silently assumed away: uniqueness is only enforced against
*currently resting* orders. Once an order fully trades or is cancelled, its
ID leaves the index and can legally be reused. Enforcing global uniqueness
forever would require remembering every ID ever seen, at unbounded memory
cost — most real venues also scope ID uniqueness to a session or to live
orders rather than for all time, so this tracks realistic behavior rather
than being a shortcut.

This was found by code review, not by a test, which is itself worth noting.
The regression test and the randomized-test extension (duplicate-ID
injection into the fuzzing loop) were both written *after* the fix, to
close the gap that let it through unnoticed originally.

### 3. Unchecked negative numbers in CSV parsing

The original replay parser converted fields directly with `std::stoul`:

```cpp
order.qty = static_cast<Quantity>(std::stoul(f[5]));
```

`std::stoul` does not throw on a negative input string like `"-5"` — it
silently interprets the leading `-` as part of a wraparound conversion and
returns a huge unsigned value (close to 4 billion on a 32-bit `unsigned
long`, or the platform's `unsigned long` max more generally). A malformed
or adversarial event file could therefore produce an order with an
absurdly large quantity instead of being rejected as invalid input — a
silent data-corruption bug rather than a crash, which is worse, because
nothing would visibly fail.

Fixed by parsing into a signed `long long` first and explicitly range
checking before narrowing:

```cpp
const long long id  = std::stoll(f[1]);
const long long qty = std::stoll(f[5]);
if (id < 0 || qty < 0 ||
    qty > static_cast<long long>(std::numeric_limits<Quantity>::max())) {
    throw std::invalid_argument("id or quantity out of range");
}
```

Any line that fails this check is now counted as a malformed line
(`ReplayStats::badLines`) and skipped, rather than silently producing a
corrupted order. This was found by manual reasoning about `stoul`'s
documented behavior while writing the validation tests for bug #2, not by
a failing test — a reminder that some classes of bug (silent overflow/
wraparound) need to be reasoned about explicitly, since a test only catches
what it's told to check for.

## Benchmark methodology and the tail-latency investigation

Full numbers and raw output are in [`benchmarks.md`](benchmarks.md); this
section covers the reasoning behind the one real investigation done on top
of the base measurements.

**Observation:** across every benchmark run (baseline and post-validation
alike), the `add` operation's maximum latency was a repeatable ~200 µs —
roughly 2,500x its own median (~83 ns) — while every other percentile
(p50/p99/p99.9) stayed small and consistent. This kind of large, *repeatable*
single-operation outlier, rather than a value that varies randomly between
runs, is a signature of an amortized-cost data structure occasionally paying
its full cost in one operation — which is exactly what `std::unordered_map`
does when it needs to rehash.

**Hypothesis:** the order-ID index (`unordered_map<OrderId, Locator>
index_`) grows by one entry on every resting limit order. As it crosses
internal load-factor thresholds, it periodically rebuilds its entire bucket
array and reinserts every existing entry — an O(n) operation that happens on
whichever single `insert()` call triggers it. Everywhere else, `add` is O(1)
average.

**Test design:** rather than changing the engine's default behavior (which
would need its own before/after measurement to justify), a
`reserveOrders(n)` method was added purely for the benchmark to call
optionally, pre-allocating the index's bucket array before the timed pass
begins:

```cpp
void reserveOrders(std::size_t n) { index_.reserve(n); }
```

`cancel` was used as an implicit control in this experiment: cancellation
never inserts into `index_` (only erases), so it should never trigger a
rehash and its latency profile should be unaffected by reservation — which
is exactly what would confirm or deny the hypothesis without needing a
second variable.

**Result:** reserving the index's buckets ahead of time cut `add` max
latency from ~210 µs to ~18 µs (roughly 11x), while `cancel` max latency
was essentially unchanged (~12.6 µs vs ~12 µs) — consistent with the
hypothesis. The dominant tail-latency cause on the `add` path was
`unordered_map` rehashing, not the tree operations or list operations
elsewhere in the same call.

**What this does not claim:** the remaining ~18 µs `add` max after
reservation was not investigated further — it may be a `std::map` node
allocation, a page fault, or genuine OS scheduling noise, and distinguishing
between those would need its own targeted experiment. The reservation was
also only applied to the benchmark's separate latency-measurement pass, not
its throughput pass or the engine's actual default construction, so no
throughput number reflects it. Turning this into a real engine-level default
would mean picking a reservation size as a real design parameter (e.g. an
expected-order-count constructor argument) rather than a benchmark-only
`1 << 20` guess, which trades a fixed amount of upfront memory for the
tail-latency improvement — a trade this project has not yet made a decision
on for the shipped engine.
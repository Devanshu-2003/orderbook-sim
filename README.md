# orderbook-sim

A C++17 limit order book matching engine with price-time priority, O(1)
cancellation, a deterministic synthetic order-flow generator, a CSV replay
CLI, 21 tests (unit + randomized invariant), and a throughput/latency
benchmark harness.

Built as a portfolio project to demonstrate systems-level C++: data structure
selection under complexity constraints, correctness testing beyond hand-picked
cases, and measured (not assumed) performance.

## What it does 

Exchanges match buy and sell orders against a live **order book** — the set
of unfilled limit orders waiting on each side of a market. This project
implements that matching logic from scratch:

- **Limit orders** rest in the book until a matching price arrives.
- **Market orders** execute immediately against the best available price(s),
  sweeping multiple price levels if needed.
- **Price-time priority**: the best price is matched first; at equal prices,
  the order that arrived first is filled first.
- **Cancellation** removes a resting order in O(1).
- **Validation** rejects malformed orders (duplicate IDs, zero quantity,
  non-positive limit prices) before they touch the book.

## Quick demo
$ ./build/obsim data/sample_events.csv
TRADE buy=4 sell=1 price=52.00 qty=60
TRADE buy=5 sell=1 price=52.00 qty=40
TRADE buy=5 sell=2 price=52.05 qty=150

--- summary ---
events: 8 (new 6, cancels 2, cancel misses 1, bad lines 0)
trades: 3 volume: 250
resting orders: 1
best bid: none best ask: 51.95

At scale, replaying 100,000 generated events:

$ ./build/obsim --generate 100000 42 > data/generated_100k.csv
$ ./build/obsim data/generated_100k.csv | tail -8
--- summary ---
events: 100000 (new 86617, cancels 13383, cancel misses 0, bad lines 0)
trades: 65561 volume: 1551647
resting orders: 6927
best bid: 100.38 best ask: 100.39

## Architecture
orderbook-sim/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── .gitignore
│
├── include/obsim/
│ ├── types.hpp # Price, Quantity, OrderId, Side, OrderType
│ ├── orders.hpp # Order struct
│ ├── trade.hpp # Trade struct
│ ├── orderbook.hpp # OrderBook class, PriceLevel, RejectReason
│ ├── replay.hpp # CSV event replay
│ └── generator.hpp # deterministic synthetic order-flow generator
│
├── src/
│ ├── orderbook.cpp # matching engine, cancel, validation
│ ├── replay.cpp # CSV parsing, trade log output
│ ├── generator.cpp # synthetic event generation
│ └── main.cpp # CLI: replay a file, or generate one
│
├── bench/
│ └── bench_matching.cpp # throughput + latency-percentile harness
│
├── tests/
│ ├── testmatch.cpp # matching, cancel, replay, validation, generator
│ └── testinvariants.cpp # randomized invariant test against an
│ # independent reference model
│
├── data/
│ └── sample_events.csv # small hand-written event file (committed)
│ # generated_*.csv files are gitignored
│
└── docs/
└── benchmarks.md # environment, methodology, full results


### Core data structures

The book stores each side (bids, asks) as a sorted map from price to a FIFO
queue of orders at that price, plus a hash index for O(1) cancellation:

```cpp
std::map<Price, PriceLevel, std::greater<Price>> bids_;  // best bid = begin()
std::map<Price, PriceLevel>                      asks_;  // best ask = begin()
std::unordered_map<OrderId, Locator>             index_; // O(1) cancel lookup

struct PriceLevel {
    std::list<Order> orders;   // FIFO; O(1) erase from the middle via iterator
    Quantity totalQty = 0;
};
```

| Choice | Why | Complexity |
|---|---|---|
| `std::map` (red-black tree) per side | Prices stay sorted; best price is always `begin()` | O(log P), P = distinct price levels |
| `std::list` per price level | FIFO time priority; erasing a known element (for cancel) is O(1) and doesn't invalidate other iterators | O(1) |
| `unordered_map<OrderId, Locator>` | Finds any resting order (for cancel) without scanning the book | O(1) average |
| Integer ticks (`int64_t`), not `double`, for price | Floating-point equality is unreliable for price comparison (`0.1 + 0.2 != 0.3`); ticks avoid it entirely | — |

This was an explicit design correction made partway through the project —
see [`docs/design.md`](docs/design.md) for the reasoning and what was
originally considered instead (`std::map<double, std::queue<Order>>`).

## Building and running

Requires a C++17 compiler and CMake ≥ 3.16. Tested with Apple clang 17.0.0 on
macOS 15.7.9 (Apple M4 Max).

```bash
# Configure and build (Debug, with tests)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run all 21 tests
ctest --test-dir build --output-on-failure

# Replay the sample file
./build/obsim data/sample_events.csv

# Generate 100,000 synthetic events (deterministic given a seed) and replay them
./build/obsim --generate 100000 42 > data/generated_100k.csv
./build/obsim data/generated_100k.csv
```

### Sanitizer build (AddressSanitizer + UndefinedBehaviorSanitizer)

```bash
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DOBSIM_SANITIZE=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

All 21 tests pass under both sanitizers on Apple clang 17 / macOS. Note: macOS
does not support ASan's leak-detection component, so this covers memory
errors (out-of-bounds access, use-after-free) and undefined behavior, not
leaks.

### Benchmark (Release build required — Debug numbers are meaningless)

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target bench_matching
./build-release/bench_matching 1000000 1
```

Full methodology and results: [`docs/benchmarks.md`](docs/benchmarks.md).

## Testing

21 tests across 5 suites, run with GoogleTest via CTest:

| Suite | What it checks |
|---|---|
| `Matching` (5) | Crossing/non-crossing orders, partial fills, multi-level market-order sweeps, same-price FIFO ordering |
| `Cancel` (6) | O(1) removal, unknown/already-filled/double-cancel handling, mid-queue removal without disturbing neighbors, price-level cleanup |
| `Replay` (2) | CSV parsing, trade log correctness, reject/bad-line counting, negative-number handling |
| `Generator` (2) | Deterministic output for a given seed, generated flow replays cleanly with zero cancel misses |
| `Validation` (4) | Duplicate order ID, zero quantity, non-positive limit price, ID reuse after an order is gone |
| `Invariants` (1) | **Randomized**: 25 seeds × 4,000 operations each, checked against an independent reference model after every single operation — book never crossed, quantity conserved, resting state matches the model exactly, duplicate-ID injection rejected |

The randomized invariant test is the one doing the real work. It found a
real bug during development (see below) that all 20 hand-written tests
missed.

### Bugs found during development

Being upfront about what actually went wrong, and how it was caught, rather
than presenting the code as if it were correct from the start:

1. **Missing `totalQty` decrement.** An early version of the matching loop
   filled orders correctly but forgot to decrement `PriceLevel::totalQty` on
   a partial fill. All 5 hand-written matching tests still passed, because
   none of them inspected level totals directly. The randomized invariant
   test failed within 18 steps on its first seed, because it cross-checks
   `book.restingQuantity()` against an independently tracked model.
2. **Duplicate order IDs.** The original design let two orders share an ID;
   the second would silently overwrite the first's entry in the cancel
   index, making the first order impossible to cancel even though it was
   still resting in the book. Fixed by validating every order against the
   live index before it's matched or rested (`RejectReason::DuplicateId`).
3. **Unchecked negative numbers in CSV parsing.** `std::stoul` on a string
   like `"-5"` doesn't throw — it silently wraps around to a huge unsigned
   value. A malformed or malicious event file could have produced an order
   with a multi-billion-share quantity. Fixed with explicit range checks
   before casting.

## Benchmark summary

Full details, three-run tables, and caveats in
[`docs/benchmarks.md`](docs/benchmarks.md). Environment: Apple M4 Max, macOS
15.7.9, Apple clang 17.0.0, Release build, single-threaded.

| Configuration | Throughput | add p99.9 | add max |
|---|---|---|---|
| Baseline matching engine | 11.85 – 11.99 M events/s | 500 ns | 207 – 231 µs |
| + order validation | 11.01 – 11.05 M events/s | 500 ns | 194 – 229 µs |
| + index pre-reservation (latency pass only) | not measured | 458 ns | **18 µs** |

**Finding:** a repeatable ~200 µs `add` latency spike, present in every run,
traced to `unordered_map` rehashing as the cancel index grows. Pre-reserving
buckets cut the max from ~210 µs to ~18 µs. `cancel` latency was unaffected,
as expected — cancellation never inserts into the index.

Workload is a deterministic synthetic generator (seeded, reproducible): ~13%
cancels, ~10% market orders, an aggressive mix that crosses the spread
often. This keeps the book shallow and the price band tight, which favors
`std::map`'s cache behavior — the numbers above should not be read as
representative of a deep, realistic book, and are not comparable to
production exchange engines (no network, persistence, or multi-instrument
load).

## Non-goals / not yet implemented

Being explicit about scope rather than implying more than what's built:

- **Modify orders** (quantity reduction in place, or price change as
  cancel+re-add) — not implemented.
- **IOC / FOK order types** — not implemented; only plain limit and market
  orders are supported.
- **Multi-threading** — the engine is single-threaded by design; a
  producer/consumer ring buffer was considered but not built.
- **Real market data** (LOBSTER, live exchange feeds) — the project
  currently uses only the built-in synthetic generator. Feeding it real
  historical or live data is a planned next step, not yet done.
- **A live/visual mode** — currently CLI-only, replaying a file and printing
  a summary. A terminal depth-ladder view is a possible future addition.

## License

MIT — see [LICENSE](LICENSE).

## Author

Devanshu Choudhary


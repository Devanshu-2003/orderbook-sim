# Design notes

## Why integer ticks, not `double`, for price

The first design considered was `std::map<double, std::queue<Order>>`.
Two problems ruled it out:
1. Floating-point equality is unreliable (`0.1 + 0.2 != 0.3` in IEEE 754),
   so two orders meant to be at the same price could fail to match.
2. `std::queue` has no way to erase from the middle, which cancellation
   requires.

The final design uses `int64_t` price in ticks (e.g. `5205` = $52.05) and
`std::list<Order>` per price level, which supports O(1) erase-by-iterator.

## Why `std::map` + `std::list` + `unordered_map`, not alternatives

[Fill in: the complexity table from the README, and why a flat array
indexed by price tick — mentioned as a future optimization — wasn't used
from the start (bounded price range needed to be known ahead of time,
which conflicted with wanting the generator to walk the anchor price
freely).]

## Bugs found and how

[Expand the three bullets from the README with more detail — code
snippets of the before/after for the totalQty bug, the exact duplicate-ID
scenario, the stoul wraparound example.]

## Benchmark methodology and the tail-latency investigation

[Expand docs/benchmarks.md's "Results 3" section here with the reasoning:
why rehashing was suspected, why cancel was used as a control, what the
remaining ~18µs might still be.]
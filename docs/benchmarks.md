# Benchmarks

## Environment
- Machine: Apple M4 Max, macOS 15.7.9
- Compiler: Apple clang 17.0.0, CMake `Release` build
- Single-threaded, one instrument, no I/O in the timed region

## Workload
- Synthetic order flow from the built-in generator: 1,000,000 events, seed 1
- Mix: about 13% cancels, about 10% market orders, the rest limit orders
- Flow is aggressive: about 687k trades from about 860k new orders
- Book stays shallow (thousands of resting orders, tight price band)

## Method
- Events are generated and parsed into memory before timing starts
- Throughput: one warm-up pass, then best of 3 untimed-per-op passes
- Latency: separate pass timing every operation with `steady_clock`
  (timer tick is about 42 ns on this machine, so percentiles are quantized
  and include timer overhead)

---

## Results 1: Baseline (`std::map` + `std::list` + `unordered_map` index)

No order validation yet (duplicate ID / zero qty / bad price checks not present).

| Run | Throughput (M ev/s) | add p50 | add p99 | add p99.9 | add max | cancel p50 | cancel p99 | cancel p99.9 | cancel max |
|---|---|---|---|---|---|---|---|---|---|
| 1 | 11.99 | 83 ns | 333 ns | 500 ns | 231334 ns | 83 ns | 125 ns | 208 ns | 23959 ns |
| 2 | 11.94 | 83 ns | 333 ns | 500 ns | 208625 ns | 83 ns | 125 ns | 167 ns | 21459 ns |
| 3 | 11.95 | 83 ns | 333 ns | 500 ns | 207416 ns | 83 ns | 125 ns | 167 ns | 5208 ns |

Raw output:
Throughput (best of 3): 11.99 M events/sec (0.08 s for 1000000 events, 687149 trades)
add n=860370 p50=83 ns p99=333 ns p99.9=500 ns max=231334 ns
cancel n=139630 p50=83 ns p99=125 ns p99.9=208 ns max=23959 ns

Throughput (best of 3): 11.94 M events/sec (0.08 s for 1000000 events, 687149 trades)
add n=860370 p50=83 ns p99=333 ns p99.9=500 ns max=208625 ns
cancel n=139630 p50=83 ns p99=125 ns p99.9=167 ns max=21459 ns

Throughput (best of 3): 11.95 M events/sec (0.08 s for 1000000 events, 687149 trades)
add n=860370 p50=83 ns p99=333 ns p99.9=500 ns max=207416 ns
cancel n=139630 p50=83 ns p99=125 ns p99.9=167 ns max=5208 ns


**Summary:** throughput 11.94 – 11.99 M events/sec across 3 runs. `add` max repeats every run in the 207 – 231 µs range; `cancel` max varies more widely (5 – 24 µs), consistent with noise rather than a repeatable cause.

---

## Results 2: With order validation (duplicate ID, zero qty, bad price checks added)

Same workload and method as Results 1, after `addOrder` gained a validation pass (one `unordered_map` lookup per order).

| Run | Throughput (M ev/s) | add p50 | add p99 | add p99.9 | add max | cancel p50 | cancel p99 | cancel p99.9 | cancel max |
|---|---|---|---|---|---|---|---|---|---|
| 1 | 11.01 | 83 ns | 333 ns | 500 ns | 226917 ns | 83 ns | 125 ns | 208 ns | 11459 ns |
| 2 | 11.03 | 83 ns | 333 ns | 500 ns | 193708 ns | 83 ns | 125 ns | 208 ns | 11625 ns |
| 3 | 11.05 | 83 ns | 333 ns | 500 ns | 229375 ns | 83 ns | 125 ns | 208 ns | 8708 ns |

Raw output:
Throughput (best of 3): 11.01 M events/sec (0.09 s for 1000000 events, 687149 trades)
add n=860370 p50=83 ns p99=333 ns p99.9=500 ns max=226917 ns
cancel n=139630 p50=83 ns p99=125 ns p99.9=208 ns max=11459 ns

Throughput (best of 3): 11.03 M events/sec (0.09 s for 1000000 events, 687149 trades)
add n=860370 p50=83 ns p99=333 ns p99.9=500 ns max=193708 ns
cancel n=139630 p50=83 ns p99=125 ns p99.9=208 ns max=11625 ns

Throughput (best of 3): 11.05 M events/sec (0.09 s for 1000000 events, 687149 trades)
add n=860370 p50=83 ns p99=333 ns p99.9=500 ns max=229375 ns
cancel n=139630 p50=83 ns p99=125 ns p99.9=208 ns max=8708 ns


**Summary:** throughput 11.01 – 11.05 M events/sec, down from the 11.94 – 11.99 M events/sec baseline — about a 7-8% cost for the added validation lookup. Latency percentiles (p50/p99/p99.9) are unchanged from Results 1 at this timer's resolution. `add` max is still in the 193 – 229 µs range.

---

## Results 3: Tail-latency experiment — `unordered_map` index reservation

**Hypothesis:** the repeatable ~200 µs `add` max seen in Results 1 and 2 is caused by the order-ID `unordered_map` index rehashing as it grows.

**Test:** pre-reserve the index's bucket count (`index_.reserve(1 << 20)`) before the latency pass, and compare against the default (no reservation), on the post-validation code. Single run each.

| Variant | add p50 | add p99 | add p99.9 | add max | cancel p50 | cancel p99 | cancel p99.9 | cancel max |
|---|---|---|---|---|---|---|---|---|
| default | 83 ns | 333 ns | 500 ns | 210125 ns | 83 ns | 125 ns | 208 ns | 12625 ns |
| reserved | 42 ns | 292 ns | 458 ns | 18417 ns | 83 ns | 125 ns | 208 ns | 11959 ns |

Raw output:
--- default ---
add n=860370 p50=83 ns p99=333 ns p99.9=500 ns max=210125 ns
cancel n=139630 p50=83 ns p99=125 ns p99.9=208 ns max=12625 ns

--- reserved ---
add n=860370 p50=42 ns p99=292 ns p99.9=458 ns max=18417 ns
cancel n=139630 p50=83 ns p99=125 ns p99.9=208 ns max=11959 ns


**Result:** reserving the index's buckets up front cut `add` max from 210125 ns to 18417 ns (about 11x) and `add` p99.9 from 500 ns to 458 ns. `cancel` figures are effectively unchanged (12625 ns vs 11959 ns max), which is expected since `cancelOrder` never inserts into the index and so never triggers a rehash. This is consistent with the hypothesis: the `add`-side tail latency was dominated by `unordered_map` rehashing.

**Caveats:**
- This change was only applied to the latency pass in the benchmark harness, not to the throughput pass — no throughput number was measured with reservation enabled.
- Each row above is a single run, not a best-of-3 or averaged figure.
- Reserving `1 << 20` buckets ahead of time trades memory for this latency improvement; the right reservation size depends on the expected number of resting orders, which isn't fixed in this benchmark's workload.
- The remaining ~18 µs `add` max after reservation was not investigated further.

---

## Caveats (general)
- Shallow book and tight price band favour `std::map` (everything stays in cache)
- Not comparable to production exchange engines (no network, persistence, or multi-instrument load)
- p50 differences at the ~40-80 ns scale are close to the measurement floor: the timer's own tick is about 42 ns on this machine, so a change from 83 ns to 42 ns reflects roughly one timer tick, not a precisely quantified 2x speedup
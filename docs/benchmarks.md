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

## Results (baseline: `std::map` + `std::list` + `unordered_map` index)
Throughput (best of 3): 11.99 M events/sec  (0.08 s for 1000000 events, 687149 trades)
add      n=860370    p50=83 ns   p99=333 ns   p99.9=500 ns   max=231334 ns
cancel   n=139630    p50=83 ns   p99=125 ns   p99.9=208 ns   max=23959 ns
Throughput (best of 3): 11.94 M events/sec  (0.08 s for 1000000 events, 687149 trades)
add      n=860370    p50=83 ns   p99=333 ns   p99.9=500 ns   max=208625 ns
cancel   n=139630    p50=83 ns   p99=125 ns   p99.9=167 ns   max=21459 ns
Throughput (best of 3): 11.95 M events/sec  (0.08 s for 1000000 events, 687149 trades)
add      n=860370    p50=83 ns   p99=333 ns   p99.9=500 ns   max=207416 ns
cancel   n=139630    p50=83 ns   p99=125 ns   p99.9=167 ns   max=5208 ns

## Caveats
- Shallow book and tight price band favour `std::map` (everything stays in cache)
- Not comparable to production exchange engines (no network, persistence,
  or multi-instrument load)
- Single max-latency outliers (one add hit 229 us) are not quoted as they are
  likely OS scheduling noise
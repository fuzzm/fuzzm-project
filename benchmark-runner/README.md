This folder contains a number of auxiliary scripts for running Fuzzm, aggregating results, generating tables etc.

# Installation

```
npm install
npm run build
```

# Running a benchmark
<a name="run">
Notice, the benchmarks must be compiled first - see the [../benchmarks/compile-all.sh](../benchmarks/compile-all.sh) script).

Run the `opj_compress` benchmark once for 60 minutes.

`node dist/index.js -r1 -t60 opj_compress`

Run the `pal2rgb` benchmark 3 times 120 minutes.

`node dist/index.js -r3 -t120 pal2rgb`

Results are stored in the [results](../results) folder.

# Aggregating fuzzing results

`node dist/accumulator/index.js`

This will use the newest results for each benchmark in the [results](../results) folder.
Plots are outputted to the [plots](plots) folder.

# Instrumentation speed experiment

`node dist/tester/instrumentation-speed.js`

It uses the test cases in the `differential-tests` folders of each [benchmark](../benchmarks).

# Instrumentation correctness experiment

`node dist/tester/correctness.js`

It uses the same test cases as the instrumentation speed experiment.

# Adding a new benchmark

Put the benchmarks in the [benchmarks](../benchmarks) folder.
Add a new entry to the `FUZZ_COMMANDS_WASM` object in [src/constants.ts](src/constants.ts).
Run the benchmark using the approach described in [Running a benchmark](#run).





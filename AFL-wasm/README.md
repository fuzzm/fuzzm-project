# Fuzzm
This folder contains the WebAssembly ([WASI](https://wasi.dev/)) port of the [AFL fuzzer](https://github.com/google/AFL).

# Building

`make`

# Running

Fuzzm is run the same way as AFL (See [AFL](https://github.com/google/AFL) and [AFL quick start guide](https://github.com/google/AFL/blob/master/docs/QuickStartGuide.txt)), but with the `WASM_MODE` flag set to 1. 

For example, run the [prepare.js](../prepare.js) script to copy the benchmarks into the programs folder, and then start the fuzzer with the `pal2rgb` application:

`WASM_MODE=1 ./afl-fuzz -m8000M -i programs/pal2rgb-wasm/test-case -o programs/pal2rgb-wasm/findings -x programs/pal2rgb-wasm/dictionary/d.dict -- programs/pal2rgb-wasm/prog.wasm.instr @@ /dev/null`

All target programs must be instrumented with the `afl_branch` program of the [wasm_instrumenter](../wasm_instrumenter).

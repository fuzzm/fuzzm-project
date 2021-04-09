# Fuzzm
This folder contains the WebAssembly ([WASI](https://wasi.dev/)) port of the [AFL fuzzer](https://github.com/google/AFL).

# Building

`make`

# Running

Fuzzm is run the same way as AFL (See [AFL](https://github.com/google/AFL) and [AFL quick start guide](https://github.com/google/AFL/blob/master/docs/QuickStartGuide.txt)), but with the `WASM_MODE` flag set to 1. 

For example, run the [prepare.js](../prepare.js) script to copy the benchmarks into the programs folder and then start the fuzzer with:
`WASM_MODE=1 ./afl-fuzz -m8000M -i programs/opj_compress-wasm/test-case -o programs/opj_compress-wasm/findings -f programs/opj_compress-wasm/input.bmp -- programs/opj_compress-wasm/prog.wasm.instr -i /home/torp/development/fuzzing-workspace/AFL-wasm/programs/opj_compress-wasm/input.bmp -o /home/torp/development/fuzzing-workspace/AFL-wasm/programs/opj_compress-wasm/out.jp2`

All target programs must be instrumented with the `afl_branch` program of the [wasm_instrumenter](../wasm_instrumenter).

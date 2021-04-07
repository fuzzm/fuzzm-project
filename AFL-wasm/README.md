# Fuzzm

Fuzzm adds WebAssembly support to [AFL fuzzer](https://github.com/google/AFL).
Special thanks to all the people behind AFL...

# Building

`make`

# Running

Fuzzm is run the same way as AFL (See [AFL](https://github.com/google/AFL) and [AFL quick start guide](https://github.com/google/AFL/blob/master/docs/QuickStartGuide.txt)), but with the `WASM_MODE` flag set to 1. 

For example:
`WASM_MODE=1 ./afl-fuzz TODO`

See TODO-LINK for a guide on how to instrument WASI binaries with Fuzzm-compatible coverage instrumentation.

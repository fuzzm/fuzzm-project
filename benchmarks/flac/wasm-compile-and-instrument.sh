#!/bin/bash
./wasm-compile.sh && ../../wasm_instrumenter/target/release/afl_branch src/flac/flac.wasm src/flac/flac.instr.wasm && ../../wasm_instrumenter/target/release/canaries src/flac/flac.instr.wasm src/flac/flac.instr.wasm --skip-print

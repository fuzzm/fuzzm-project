#!/bin/bash
./wasm-compile.sh && ../../wasm_instrumenter/target/release/afl_branch bin/opj_compress.wasm bin/opj_compress.instr.wasm && ../../wasm_instrumenter/target/release/canaries bin/opj_compress.instr.wasm bin/opj_compress.instr.wasm --skip-print

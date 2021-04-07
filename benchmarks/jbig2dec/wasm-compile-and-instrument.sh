#!/bin/bash
./wasm-compile.sh && ../../wasm_instrumenter/target/release/afl_branch jbig2dec.wasm jbig2dec.instr.wasm && ../../wasm_instrumenter/target/release/canaries jbig2dec.instr.wasm jbig2dec.instr.wasm --skip-print

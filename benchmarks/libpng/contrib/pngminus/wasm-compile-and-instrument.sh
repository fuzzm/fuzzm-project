#!/bin/bash
./wasm-compile.sh && ../../../../wasm_instrumenter/target/release/afl_branch pnm2png.wasm pnm2png.instr.wasm && ../../../../wasm_instrumenter/target/release/canaries pnm2png.instr.wasm pnm2png.instr.wasm --skip-print

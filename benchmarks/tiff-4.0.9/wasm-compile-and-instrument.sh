#!/bin/bash
./wasm-compile.sh && ../../wasm_instrumenter/target/release/afl_branch tools/pal2rgb.wasm tools/pal2rgb.instr.wasm && ../../wasm_instrumenter/target/release/canaries tools/pal2rgb.instr.wasm tools/pal2rgb.instr.wasm --skip-print

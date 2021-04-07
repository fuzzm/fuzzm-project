#!/bin/bash
./wasm-compile.sh && ../../wasm_instrumenter/target/release/afl_branch contrib/pngminus/pnm2png.wasm contrib/pngminus/pnm2png.instr.wasm && ../../wasm_instrumenter/target/release/canaries contrib/pngminus/pnm2png.instr.wasm contrib/pngminus/pnm2png.instr.wasm --skip-print

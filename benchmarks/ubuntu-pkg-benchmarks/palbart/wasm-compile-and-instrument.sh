#!/bin/bash
./wasm-compile.sh && ../../../wasm_instrumenter/target/release/afl_branch palbart.wasm palbart.instr.wasm && ../../../wasm_instrumenter/target/release/canaries --skip-print palbart.instr.wasm palbart.instr.wasm

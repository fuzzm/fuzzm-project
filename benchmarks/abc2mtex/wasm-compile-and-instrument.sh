#!/bin/bash
./wasm-compile.sh && ../../wasm_instrumenter/target/release/afl_branch abc2mtex.wasm abc2mtex.instr.wasm && ../../wasm_instrumenter/target/release/canaries abc2mtex.instr.wasm abc2mtex.instr.wasm --skip-print

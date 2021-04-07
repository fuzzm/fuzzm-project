#!/bin/bash
./wasm-compile.sh && ../../wasm_instrumenter/target/release/afl_branch pdfresurrect.wasm pdfresurrect.instr.wasm && ../../wasm_instrumenter/target/release/canaries pdfresurrect.instr.wasm pdfresurrect.instr.wasm

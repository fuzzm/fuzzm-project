#!/bin/bash

# emcc --version
# emcc (Emscripten gcc/clang-like replacement) 2.0.8 (d059fd603d0b45b584f634dc2365bc9e9a6ec1dd)
# Copyright (C) 2014 the Emscripten authors (see AUTHORS.txt)
# This is free and open source software under the MIT license.
# There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# gcc --version
# gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
# Copyright (C) 2017 Free Software Foundation, Inc.
# This is free software; see the source for copying conditions.  There is NO
# warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# Emscripten (WASI-compatible, as long as you don't use anything that needs JS/browser)
# and execution with Wasmtime
emcc vuln.c -O2 -g -o vuln.wasm && wasm2wat vuln.wasm -o vuln.wat && wasmtime vuln.wasm

# native, gcc
gcc vuln.c -O2 -g -o vuln.native && objdump -d -M intel vuln.native > vuln.S && ./vuln.native

# native, gcc with AFL instrumentation
../AFL-wasm/afl-gcc vuln.c -O2 -g -o vuln-afl.native

# REPRO: buffer overflow on wasm, not on native
# ~/Documents/SOLA/WebAssembly/fuzzing/vuln-in-wasm-not-native$ emcc vuln.c -O2 -g -o vuln.wasm && wasm2wat vuln.wasm -f -o vuln.wat && wasmtime vuln.wasm 
# Type two inputs:
# aaaabbbbaaaabbbbaaaabbbbcccc
# input: aaaabbbbaaaabbbbaaaabbbbcccc

# bbbb
# input: bbbb

# main variable: cccc

# ~/Documents/SOLA/WebAssembly/fuzzing/vuln-in-wasm-not-native$ gcc vuln.c -U_FORTIFY_SOURCE -O2 -g -o vuln.native && objdump -d -M intel vuln.native > vuln.S && ./vuln.native
# Type two inputs:
# aaaabbbbaaaabbbbaaaabbbbcccc
# input: aaaabbbbaaaabbbbaaaabbbbcccc

# bbbb
# input: bbbb

# main variable: AAAABBB

# Native fuzzing with AFL
AFL_SKIP_CPUFREQ=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 LD_LIBRARY_PATH=../AFL-wasm/wasmtime-v0.20.0-x86_64-linux-c-api/lib/ ../public-project-repo/fuzzm-project/AFL-wasm/afl-fuzz -i testcases/ -o findings ./vuln.native

# Instrument, then fuzz with Fuzzm
../wasm_instrumenter/target/release/afl_branch vuln.wasm vuln-cov.wasm
../wasm_instrumenter/target/release/canaries vuln-cov.wasm vuln-cov-canaries.wasm --skip-print
chmod +x vuln-cov-canaries.wasm
WASM_MODE=1 AFL_SKIP_CPUFREQ=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 LD_LIBRARY_PATH=../AFL-wasm/wasmtime-v0.20.0-x86_64-linux-c-api/lib/ ../public-project-repo/fuzzm-project/AFL-wasm/afl-fuzz -i testcases/ -o findings ./vuln-cov-canaries.wasm

# Validate crashes, all caused by the same (our canary) trap
for input in findings/crashes/id*; do wasmtime vuln-cov-canaries.wasm < "$input" 1> /dev/null; done

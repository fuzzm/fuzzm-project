#!/bin/bash
set -e

# compile instrumentation binaries
(cd wasm-project/wasm_instrumenter && cargo build --release)
cp wasm-project/wasm_instrumenter/target/release/afl_branch AFL-wasm/
cp wasm-project/wasm_instrumenter/target/release/canaries AFL-wasm/

# copy afl_wasm to magma fuzzers
rm -rf magma/fuzzers/afl_wasm/repo
mkdir magma/fuzzers/afl_wasm/repo
rsync --links -r --exclude='.git' AFL-wasm/* magma/fuzzers/afl_wasm/repo
mkdir magma/fuzzers/afl_wasm/repo/wasi_sdk
cp -r wasi-sdk-11.0/* magma/fuzzers/afl_wasm/repo/wasi_sdk

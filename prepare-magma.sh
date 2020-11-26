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

# setup libraries
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export CC="$DIR/wasi-sdk-11.0/bin/clang"
export AR="$DIR/wasi-sdk-11.0/bin/ar"
export LD="$DIR/wasi-sdk-11.0/bin/lld"
export RANLIB="$DIR/wasi-sdk-11.0/llvm-ranlib"
export STRIP="$DIR/wasi-sdk-11.0/llvm-strip"
export NM="$DIR/wasi-sdk-11.0/llvm-nm"
set +e #linking the libraries fails, but we don't care since we can just use to .o files.
(cd zlib-1.2.11 && ./configure && make) #> /dev/null 2>&1)
set -e 
mkdir -p magma/fuzzers/afl_wasm/repo/libraries/zlib
cp zlib-1.2.11/libz.a magma/fuzzers/afl_wasm/repo/libraries/zlib
cp -r zlib-1.2.11/*.h magma/fuzzers/afl_wasm/repo/libraries/zlib
#cp -r zlib-1.2.11/*.o magma/fuzzers/afl_wasm/repo/libraries/zlib

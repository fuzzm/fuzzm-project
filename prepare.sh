#!/bin/bash
set -e
AFL_FOLDER=AFL-wasm
INSTRUMENTER_FOLDER=wasm-project/wasm_instrumenter

# setup AFL
(cd ${AFL_FOLDER} && make)

# compile instrumentation binaries
(cd wasm-project/wasm_instrumenter && cargo build --release)

# compile lava benchmarks
LAVA=LAVA-M/LAVA-M

function prepare_benchmark() {
  SRC_LOC=$1
  NAME=$2
  WASM_MODE=$3
  DST=${AFL_FOLDER}/programs/${NAME}
  rm -rf ${DST}
  mkdir -p ${DST}
  mkdir ${DST}/findings
  mkdir ${DST}/test-case
  cp ${SRC_LOC}/${NAME} ${DST}/prog
  cp ${SRC_LOC}/../fuzzer_input/* ${DST}/test-case
  if [ $WASM_MODE ]
  then
    eval "${INSTRUMENTER_FOLDER}/target/release/afl_branch ${DST}/prog ${DST}/prog"
    eval "${INSTRUMENTER_FOLDER}/target/release/canaries ${DST}/prog ${DST}/prog"
  fi
}

BASE64_WASM=${LAVA}/base64/coreutils-8.24-lava-safe
prepare_benchmark $BASE64_WASM "base64.wasm" 1

MD5_WASM=${LAVA}/md5sum/coreutils-8.24-lava-safe
prepare_benchmark $MD5_WASM "md5sum.wasm" 1

UNIQ_WASM=${LAVA}/uniq/coreutils-8.24-lava-safe
prepare_benchmark $UNIQ_WASM "uniq.wasm" 1

##### OLD MAGMA STUFF #####

# copy afl_wasm to magma fuzzers
#rm -rf magma/fuzzers/afl_wasm/repo
#mkdir magma/fuzzers/afl_wasm/repo
#rsync --links -r --exclude='.git' AFL-wasm/* magma/fuzzers/afl_wasm/repo
#mkdir magma/fuzzers/afl_wasm/repo/wasi_sdk
#cp -r wasi-sdk-11.0/* magma/fuzzers/afl_wasm/repo/wasi_sdk

# setup libraries
#DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
#export CC="$DIR/wasi-sdk-11.0/bin/clang"
#export AR="$DIR/wasi-sdk-11.0/bin/ar"
#export LD="$DIR/wasi-sdk-11.0/bin/lld"
#export RANLIB="$DIR/wasi-sdk-11.0/llvm-ranlib"
#export STRIP="$DIR/wasi-sdk-11.0/llvm-strip"
#export NM="$DIR/wasi-sdk-11.0/llvm-nm"
#set +e #linking the libraries fails, but we don't care since we can just use to .o files.
#(cd zlib-1.2.11 && ./configure && make) #> /dev/null 2>&1)
#set -e 
#mkdir -p magma/fuzzers/afl_wasm/repo/libraries/zlib
#cp zlib-1.2.11/libz.a magma/fuzzers/afl_wasm/repo/libraries/zlib
#cp -r zlib-1.2.11/*.h magma/fuzzers/afl_wasm/repo/libraries/zlib
#cp -r zlib-1.2.11/*.o magma/fuzzers/afl_wasm/repo/libraries/zlib

#!/bin/bash
set -e
AFL_FOLDER=AFL-wasm
INSTRUMENTER_FOLDER=wasm-project/wasm_instrumenter

# setup AFL
(cd ${AFL_FOLDER} && make)

# compile instrumentation binaries
(cd wasm-project/wasm_instrumenter && cargo build --release)

# copy over benchmarks 

function prepare_benchmark() {
  SRC_LOC=$1
  NAME=$2
  FUZZER_INPUT=$3
  DICTIONARY=$4
  WASM_MODE=$5
  DST=${AFL_FOLDER}/programs/${NAME}

  rm -rf ${DST}
  mkdir -p ${DST}
  mkdir ${DST}/findings
  mkdir ${DST}/test-case
  mkdir ${DST}/dictionary

  if [ $FUZZER_INPUT ]
  then
    cp ${FUZZER_INPUT}/* ${DST}/test-case
  fi

  if [ $DICTIONARY ]
  then
    cp ${DICTIONARY} ${DST}/dictionary
  fi

  if [ $WASM_MODE ]
  then
    BIN_DST=${DST}/prog.wasm
  else
    BIN_DST=${DST}/prog
  fi

  cp ${SRC_LOC}/${NAME} ${BIN_DST}
  if [ $WASM_MODE ]
  then
    eval "${INSTRUMENTER_FOLDER}/target/release/afl_branch ${BIN_DST} ${BIN_DST}"
    eval "${INSTRUMENTER_FOLDER}/target/release/canaries ${BIN_DST} ${BIN_DST}"
  fi
}


##### LAVA BENCHMARKS #####
LAVA_WASM=LAVA-M/LAVA-M
BASE64_WASM=${LAVA_WASM}/base64/coreutils-8.24-lava-safe
BASE64_FUZZER_INPUT=${LAVA_WASM}/base64/fuzzer_input
prepare_benchmark $BASE64_WASM "base64.wasm" ${BASE64_FUZZER_INPUT} "" 1

MD5_WASM=${LAVA_WASM}/md5sum/coreutils-8.24-lava-safe
MD5_FUZZER_INPUT=${LAVA_WASM}/md5sum/fuzzer_input
prepare_benchmark $MD5_WASM "md5sum.wasm" ${BASE64_FUZZER_INPUT} "" 1

UNIQ_WASM=${LAVA_WASM}/uniq/coreutils-8.24-lava-safe
UNIQ_FUZZER_INPUT=${LAVA_WASM}/uniq/fuzzer_input
prepare_benchmark $UNIQ_WASM "uniq.wasm" ${UNIQ_FUZZER_INPUT} "" 1

LAVA_NATIVE=LAVA-native/LAVA-M

BASE64_NATIVE=${LAVA_NATIVE}/base64/coreutils-8.24-lava-safe
BASE64_NATIVE_FUZZER_INPUT=${LAVA_NATIVE}/base64/fuzzer_input
prepare_benchmark $BASE64_NATIVE "base64" ${BASE64_NATIVE_FUZZER_INPUT} 

MD5_NATIVE=${LAVA_NATIVE}/md5sum/coreutils-8.24-lava-safe
MD5_NATIVE_FUZZER_INPUT=${LAVA_NATIVE}/md5sum/fuzzer_input
prepare_benchmark $MD5_NATIVE "md5sum" ${MD5_NATIVE_FUZZER_INPUT}

UNIQ_NATIVE=${LAVA_NATIVE}/uniq/coreutils-8.24-lava-safe
UNIQ_NATIVE_FUZZER_INPUT=${LAVA_NATIVE}/uniq/fuzzer_input
prepare_benchmark $UNIQ_NATIVE "uniq" ${UNIQ_NATIVE_FUZZER_INPUT}


##### VULNERABLE BENCHMARKS #####
VULN_BENCHMARKS=wasm-project/benchmarks
PDF_RESURRECT=${VULN_BENCHMARKS}/pdfresurrect
PDF_RESURRECT_FUZZER_INPUT=${AFL_FOLDER}/testcases/others/pdf
PDF_RESURRECT_DICTIONARY=${AFL_FOLDER}/dictionaries/pdf.dict
prepare_benchmark $PDF_RESURRECT "pdfresurrect.wasm" $PDF_RESURRECT_FUZZER_INPUT $PDF_RESURRECT_DICTIONARY 1

prepare_benchmark $PDF_RESURRECT "pdfresurrect" $PDF_RESURRECT_FUZZER_INPUT $PDF_RESURRECT_DICTIONARY

OPENJPEG=${VULN_BENCHMARKS}/openjpeg/bin
PDF_RESURRECT_FUZZER_INPUT=${AFL_FOLDER}/testcases/images/jpeg
PDF_RESURRECT_DICTIONARY=${AFL_FOLDER}/dictionaries/jpeg.dict
prepare_benchmark $OPENJPEG "opj_compress.wasm" $PDF_RESURRECT_FUZZER_INPUT $PDF_RESURRECT_DICTIONARY 1
prepare_benchmark $OPENJPEG "opj_compress" $PDF_RESURRECT_FUZZER_INPUT $PDF_RESURRECT_DICTIONARY 


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

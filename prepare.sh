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
  #replace . with - (otherwise the opj_dump will think of everything after .wasm as the extension)
  NAME_TRANS="${NAME/\./-}" 
  DST=${AFL_FOLDER}/programs/${NAME_TRANS}

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
    cp ${DICTIONARY} ${DST}/dictionary/d.dict
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
    INSTR_DST=${BIN_DST}.instr
    eval "${INSTRUMENTER_FOLDER}/target/release/afl_branch ${BIN_DST} ${INSTR_DST}"
    eval "${INSTRUMENTER_FOLDER}/target/release/canaries ${INSTR_DST} ${INSTR_DST} --skip-print"
    chmod +x ${INSTR_DST}
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
OPENJPEG_FUZZER_INPUT=${AFL_FOLDER}/testcases/images/bmp
OPENJPEG_DICTIONARY=${AFL_FOLDER}/dictionaries/bmp.dict
prepare_benchmark $OPENJPEG "opj_compress.wasm" $OPENJPEG_FUZZER_INPUT "" 1
prepare_benchmark $OPENJPEG "opj_compress" $OPENJPEG_FUZZER_INPUT "" 

TIFF=${VULN_BENCHMARKS}/tiff-4.0.9/tools
TIFF_FUZZER_INPUT=${AFL_FOLDER}/testcases/images/tiff
TIFF_DICTIONARY=${AFL_FOLDER}/dictionaries/tiff.dict
prepare_benchmark $TIFF "pal2rgb.wasm" $TIFF_FUZZER_INPUT $TIFF_DICTIONARY 1
prepare_benchmark $TIFF "pal2rgb" $TIFF_FUZZER_INPUT $TIFF_DICTIONARY

PNG=${VULN_BENCHMARKS}/libpng/contrib/pngminus
PNG_FUZZER_INPUT=${AFL_FOLDER}/testcases/images/pnm
prepare_benchmark $PNG "pnm2png.wasm" $PNG_FUZZER_INPUT "" 1
prepare_benchmark $PNG "pnm2png" $PNG_FUZZER_INPUT ""

##### UBUNTU PACKAGE BENCHMARKS #####
UBUNTU_BENCHMARKS=wasm-project/benchmarks/ubuntu-pkg-benchmarks

PALBART=${UBUNTU_BENCHMARKS}/palbart
PALBART_FUZZER_INPUT=${AFL_FOLDER}/testcases/ubuntu/palbart
prepare_benchmark $PALBART "palbart.wasm" $PALBART_FUZZER_INPUT "" 1
prepare_benchmark $PALBART "palbart" $PALBART_FUZZER_INPUT "" 

PDI2ISO=${UBUNTU_BENCHMARKS}/pdi2iso
PDI2ISO_FUZZER_INPUT=${AFL_FOLDER}/testcases/ubuntu/pdi2iso
prepare_benchmark $PDI2ISO "pdi2iso.wasm" $PDI2ISO_FUZZER_INPUT "" 1
prepare_benchmark $PDI2ISO "pdi2iso" $PDI2ISO_FUZZER_INPUT "" 

QFLOW=${UBUNTU_BENCHMARKS}/qflow
QFLOW_FUZZER_INPUT=${AFL_FOLDER}/testcases/ubuntu/qflow
prepare_benchmark $QFLOW "blif2Verilog.wasm" $QFLOW_FUZZER_INPUT "" 1
prepare_benchmark $QFLOW "blif2Verilog" $QFLOW_FUZZER_INPUT "" 

WZIP=${UBUNTU_BENCHMARKS}/wzip
WZIP_FUZZER_INPUT=${AFL_FOLDER}/testcases/ubuntu/wzip
prepare_benchmark $WZIP "wzip.wasm" $WZIP_FUZZER_INPUT "" 1
prepare_benchmark $WZIP "wzip" $WZIP_FUZZER_INPUT "" 

AHA=${UBUNTU_BENCHMARKS}/aha
AHA_FUZZER_INPUT=${AFL_FOLDER}/testcases/ubuntu/aha
prepare_benchmark $AHA "aha.wasm" $AHA_FUZZER_INPUT "" 1
prepare_benchmark $AHA "aha" $AHA_FUZZER_INPUT "" 

BASEZ=${UBUNTU_BENCHMARKS}/basez
BASEZ_FUZZER_INPUT=${AFL_FOLDER}/testcases/ubuntu/basez
prepare_benchmark $BASEZ "basez.wasm" $BASEZ_FUZZER_INPUT "" 1
prepare_benchmark $BASEZ "basez" $BASEZ_FUZZER_INPUT "" 

BPM=${UBUNTU_BENCHMARKS}/bpm-tools
BPM_FUZZER_INPUT=${AFL_FOLDER}/testcases/ubuntu/bpm
prepare_benchmark $BPM "bpm.wasm" $BPM_FUZZER_INPUT "" 1
prepare_benchmark $BPM "bpm" $BPM_FUZZER_INPUT "" 

CODEGROUP=${UBUNTU_BENCHMARKS}/codegroup
CODEGROUP_FUZZER_INPUT=${AFL_FOLDER}/testcases/ubuntu/codegroup
prepare_benchmark $CODEGROUP "codegroup.wasm" $CODEGROUP_FUZZER_INPUT "" 1
prepare_benchmark $CODEGROUP "codegroup" $CODEGROUP_FUZZER_INPUT "" 

MIDICSV=${UBUNTU_BENCHMARKS}/midicsv
MIDICSV_FUZZER_INPUT=${AFL_FOLDER}/testcases/ubuntu/midicsv
prepare_benchmark $MIDICSV "midicsv.wasm" $MIDICSV_FUZZER_INPUT "" 1
prepare_benchmark $MIDICSV "midicsv" $MIDICSV_FUZZER_INPUT "" 


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

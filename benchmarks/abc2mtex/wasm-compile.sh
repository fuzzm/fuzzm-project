#!/bin/bash
export CC="../../wasi-sdk-11.0/bin/clang --sysroot=../../wasi-sdk-11.0/share/wasi-sysroot"
export CPP="../../wasi-sdk-11.0/bin/clang++"
export LD="../../wasi-sdk-11.0/bin/ld"
export NM="../../wasi-sdk-11.0/bin/nm"
export AR="../../wasi-sdk-11.0/bin/ar"
export RANLIB="../../wasi-sdk-11.0/bin/ranlib"

$CC -O2 abc.c fields.c tex.c index.c -o abc2mtex.wasm

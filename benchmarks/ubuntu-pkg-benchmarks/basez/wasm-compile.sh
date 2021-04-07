#!/bin/bash
export CC=`realpath ../../../../wasi-sdk-11.0/bin/clang`
export CPP=`realpath ../../../../wasi-sdk-11.0/bin/clang++`
export LD=`realpath ../../../../wasi-sdk-11.0/bin/lld`
export AR=`realpath ../../../../wasi-sdk-11.0/bin/ar`
export RANLIB=`realpath ../../../../wasi-sdk-11.0/bin/ranlib`
export NM=`realpath ../../../../wasi-sdk-11.0/bin/nm`

$CC -O2 basez.c base16.c base32.c base64.c -o basez.wasm

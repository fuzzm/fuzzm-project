#!/bin/bash
export CC="../../../../../wasi-sdk-11.0/bin/clang --sysroot=../../../../../wasi-sdk-11.0/share/wasi-sysroot"
export AR="../../../../../wasi-sdk-11.0/bin/ar"
export RANLIB="../../../../../wasi-sdk-11.0/bin/ranlib"
export LD="../../../../../wasi-sdk-11.0/bin/ld"
export NM="../../../../../wasi-sdk-11.0/bin/nm"
./configure
make

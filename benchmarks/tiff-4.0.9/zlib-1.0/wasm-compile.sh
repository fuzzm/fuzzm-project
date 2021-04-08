#!/bin/bash
export CC=`realpath ../../../wasi-sdk-11.0/bin/clang`
export LD=`realpath ../../../wasi-sdk-11.0/bin/lld`
AR=`realpath ../../../wasi-sdk-11.0/bin/ar`
export AR="$AR rc"
export NM=`realpath ../../../wasi-sdk-11.0/bin/nm`
export RANLIB=`realpath ../../../wasi-sdk-11.0/bin/ranlib`
./configure
make


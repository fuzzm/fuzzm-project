#!/bin/bash
make clean
export CC=../../AFL-wasm/afl-gcc
export CFLAGS="-O2 $CFLAGS"
./configure
make

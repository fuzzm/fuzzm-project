#!/bin/bash
CC=`realpath ../../../../AFL-wasm/afl-gcc`
$CC -O2 src/blif2Verilog.c -o blif2Verilog

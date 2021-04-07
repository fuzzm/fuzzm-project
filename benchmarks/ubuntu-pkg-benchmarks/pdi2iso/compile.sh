#!/bin/bash
CC=`realpath ../../../../AFL-wasm/afl-gcc`
$CC -O2 src/pdi2iso.c -o pdi2iso

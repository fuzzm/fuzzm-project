#!/bin/bash
CC=`realpath ../../../../AFL-wasm/afl-gcc`
$CC -O2 zl.c -o zl

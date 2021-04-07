#!/bin/bash
CC=`realpath ../../../../AFL-wasm/afl-gcc`
$CC -O2 basez.c base16.c base32.c base64.c -o basez

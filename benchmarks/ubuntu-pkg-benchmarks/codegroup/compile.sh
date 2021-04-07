#!/bin/bash
CC=`realpath ../../../../AFL-wasm/afl-gcc`
$CC -O2 codegroup.c -o codegroup

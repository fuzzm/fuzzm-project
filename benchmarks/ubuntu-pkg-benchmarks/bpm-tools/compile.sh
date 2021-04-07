#!/bin/bash
CC=`realpath ../../../../AFL-wasm/afl-gcc`
$CC -O2 bpm.c -lm -o bpm

#!/bin/bash
CC=`realpath ../../../../AFL-wasm/afl-gcc`
$CC -O2 csv.c  getopt.c  midicsv.c  midio.c -o midicsv

#!/bin/bash
CC=../../AFL-wasm/afl-gcc
$CC -O2 abc.c fields.c tex.c index.c -o abc2mtex

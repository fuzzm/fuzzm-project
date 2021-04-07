#!/bin/bash
CC="../../../wasi-sdk-11.0/bin/clang --sysroot=../../../wasi-sdk-11.0/share/wasi-sysroot"
$CC -O2 main.c pdf.c -I. -o pdfresurrect.wasm

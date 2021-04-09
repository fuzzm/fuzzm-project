#!/bin/bash
(cd zlib && ./compile-wasm.sh)

export CC=`realpath ../../../../wasi-sdk-11.0/bin/clang`
export CC="$CC --sysroot=../../../../wasi-sdk-11.0/share/wasi-sysroot/"
export AR=`realpath ../../../../wasi-sdk-11.0/bin/ar`
export RANLIB=`realpath ../../../../wasi-sdk-11.0/bin/ranlib`
export LD=`realpath ../../../../wasi-sdk-11.0/bin/ld`
export NM=`realpath ../../../../wasi-sdk-11.0/bin/nm`

LIBZ_FOL=`realpath zlib`
export LDFLAGS="-L$LIBZ_FOL $LDFLAGS"
export CFLAGS="-I$LIBZ $CFLAGS"
$CC -O2 pnm2png.c ../../png.c ../../pngread.c ../../pngwrite.c ../../pngtrans.c ../../pngset.c ../../pngwutil.c ../../pngwio.c ../../pngrutil.c ../../pngwtran.c ../../pngrio.c ../../pngerror.c ../../pngrtran.c ../../pngmem.c ../../pngget.c  -o pnm2png.wasm -I ../../ -I zlib/ -Lzlib -lm -lz

# zlib/crc32.c zlib/adler32.c zlib/inflate.c zlib/deflate.c zlib/zutil.c zlib/inftrees.c

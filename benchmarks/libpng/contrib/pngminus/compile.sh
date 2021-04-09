#!/bin/bash
(cd zlib && ./compile.sh)

LIBZ_FOL=`realpath zlib`
export LDFLAGS="-L$LIBZ_FOL $LDFLAGS"
export CC=`realpath ../../../../AFL-wasm/afl-gcc`
$CC  -O2 pnm2png.c ../../png.c ../../pngread.c ../../pngwrite.c ../../pngtrans.c ../../pngset.c ../../pngwutil.c ../../pngwio.c ../../pngrutil.c ../../pngwtran.c ../../pngrio.c ../../pngerror.c ../../pngrtran.c ../../pngmem.c ../../pngget.c -o pnm2png -I ../../ -I zlib/ -lm zlib/libz.a

# zlib/crc32.c zlib/adler32.c zlib/inflate.c zlib/deflate.c zlib/zutil.c zlib/inftrees.c

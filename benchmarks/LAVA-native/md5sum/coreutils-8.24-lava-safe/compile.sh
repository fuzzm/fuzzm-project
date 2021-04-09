#!/bin/bash

CFILES="lib/progname.c\
  lib/closeout.c\
  lib/config.h\
  lib/close-stream.c\
  lib/quotearg.c\
  lib/error.c\
  lib/exitfail.c\
  lib/c-strcasecmp.c\
  lib/xalloc-die.c\
  lib/xmalloc.c\
  lib/c-ctype.c\
  lib/xstrtoumax.c\
  src/version.c\
  lib/version-etc.c\
  lib/version-etc-fsf.c\
  lib/fadvise.c\
  lib/md5.c\
  lib/fclose.c\
  lib/malloc.c\
  lib/realloc.c\
  lib/open.c\
  lib/close.c\
  lib/fflush.c\
  lib/fpurge.c\
  lib/rpls.c\
  lib/fopen-safer.c\
  lib/dup-safer.c\
  lib/localcharset.c"

# run configure script
# ./configure
export CC=../../../../AFL-wasm/afl-gcc
# -L/home/foo/Downloads/musl-1.2.1 -lc -nostdlib --sysroot=/home/foo/development/musl-1.2.1/
#export CFLAGS="-g -Ilib/ -isysroot /usr/local/musl/include/ -L/usr/local/musl/lib -lc -nostdlib -static "
export CFLAGS="-O2 -Ilib/ "
# static link musl stdlib.
#export CFLAGS="--no-sysroot-suffix --sysroot=/usr/local/musl -static $CFLAGS"
export CFLAGS="-static $CFLAGS"
export CFLAGS="-DHASH_ALGO_MD5 -D_GL_INCLUDING_UNISTD_H $CFLAGS"
$CC  $CFLAGS $CFILES src/md5sum.c -o md5sum

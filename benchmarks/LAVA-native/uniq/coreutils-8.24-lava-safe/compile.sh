#!/bin/bash

CFILES="lib/progname.c\
  src/version.c\
  lib/version-etc.c\
  lib/version-etc-fsf.c\
  lib/close.c\
  lib/fclose.c\
  lib/closeout.c\
  lib/close-stream.c\
  lib/quotearg.c\
  lib/exitfail.c\
  lib/localcharset.c\
  lib/fadvise.c\
  lib/xalloc-die.c\
  lib/xmalloc.c\
  lib/malloc.c\
  lib/realloc.c\
  lib/rpls.c\
  lib/c-ctype.c\
  lib/c-strcasecmp.c\
  lib/hard-locale.c\
  lib/xstrtoul.c\
  lib/freopen-safer.c\
  lib/dup2.c\
  lib/memcasecmp.c\
  lib/xmemcoll.c\
  lib/memcoll.c\
  lib/argmatch.c\
  lib/linebuffer.c\
  lib/posixver.c\
  lib/error.c"

# run configure script
# ./configure
export CC=../../../../AFL-wasm/afl-gcc
export CFLAGS="-O2 -Ilib/"
# static link musl stdlib.
#export CFLAGS="--no-sysroot-suffix --sysroot=/usr/local/musl -static $CFLAGS"
export CFLAGS="-static $CFLAGS"
# avoid unistd.h problems
export CFLAGS="-D_GL_INCLUDING_UNISTD_H $CFLAGS"
$CC $CFLAGS $CFILES src/uniq.c -o uniq

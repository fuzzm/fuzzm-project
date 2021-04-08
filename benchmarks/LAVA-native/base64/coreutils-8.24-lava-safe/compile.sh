#!/bin/bash

CFILES="lib/progname.c\
  lib/closeout.c\
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
  lib/base64.c\
  lib/fclose.c\
  lib/malloc.c\
  lib/realloc.c\
  lib/open.c\
  lib/close.c\
  lib/fflush.c\
  lib/fpurge.c\
  lib/rpls.c\
  lib/localcharset.c"

# run configure script
# ./configure
export CC=../../../../AFL-wasm/afl-gcc
export CFLAGS="-O2 -Ilib/"
$CC $CFLAGS $CFILES src/base64.c -o base64

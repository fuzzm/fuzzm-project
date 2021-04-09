#!/bin/bash
CC=../../AFL-wasm/afl-gcc
mkdir bin

$CC -O2 -D USE_JPIP=1 -I src/lib/openjp2 -I src/bin/common/ src/bin/jp2/opj_compress.c src/bin/jp2/convert.c src/lib/openjp2/bio.c src/lib/openjp2/jp2.c src/lib/openjp2/raw.c src/lib/openjp2/cidx_manager.c src/lib/openjp2/mct.c src/lib/openjp2/t1.c src/lib/openjp2/cio.c src/lib/openjp2/mqc.c src/lib/openjp2/dwt.c src/lib/openjp2/openjpeg.c src/lib/openjp2/t2.c src/lib/openjp2/event.c src/lib/openjp2/tcd.c src/lib/openjp2/function_list.c src/lib/openjp2/opj_malloc.c src/lib/openjp2/tgt.c src/lib/openjp2/image.c src/lib/openjp2/phix_manager.c src/lib/openjp2/thix_manager.c src/lib/openjp2/invert.c src/lib/openjp2/pi.c src/lib/openjp2/tpix_manager.c src/lib/openjp2/j2k.c src/lib/openjp2/ppix_manager.c src/bin/common/opj_getopt.c src/bin/jp2/convertbmp.c -lm -o  bin/opj_compress

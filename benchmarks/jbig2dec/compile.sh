#!/bin/bash
export CC=../../AFL-wasm/afl-gcc

$CC -O2 -I. jbig2dec.c \
  sha1.c jbig2.c jbig2_page.c jbig2_segment.c\
  jbig2_image_pbm.c jbig2_image.c jbig2_symbol_dict.c\
  jbig2_halftone.c jbig2_huffman.c jbig2_text.c\
  jbig2_generic.c jbig2_arith.c jbig2_arith_int.c\
  jbig2_refinement.c jbig2_arith_iaid.c jbig2_mmr.c\
  -o jbig2dec

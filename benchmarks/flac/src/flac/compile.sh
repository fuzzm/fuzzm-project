#!/bin/bash
export CC=../../../../AFL-wasm/afl-gcc
export LD=ld

$CC -O2 -I. -I../../include -I../libFLAC/include\
  -DPACKAGE_VERSION=\"1.3.2\"\
  -DFLAC__HAS_OGG=0\
  -DHAVE_LROUND=1\
  analyze.c decode.c encode.c local_string_utils.c main.c utils.c vorbiscomment.c foreign_metadata.c\
  ../libFLAC/format.c ../share/grabbag/snprintf.c ../share/grabbag/file.c \
  ../libFLAC/stream_decoder.c ../libFLAC/stream_encoder.c\
  ../share/replaygain_synthesis/replaygain_synthesis.c\
  ../share/grabbag/replaygain.c ../libFLAC/metadata_object.c\
  ../share/grabbag/cuesheet.c ../share/grabbag/seektable.c\
  ../share/getopt/getopt1.c ../share/getopt/getopt.c\
  ../share/grabbag/picture.c ../share/utf8/utf8.c\
  ../libFLAC/metadata_iterators.c ../libFLAC/bitreader.c\
  ../libFLAC/md5.c ../libFLAC/lpc.c ../libFLAC/cpu.c\
  ../libFLAC/crc.c ../libFLAC/memory.c ../libFLAC/fixed.c \
  ../libFLAC/minmax.c ../libFLAC/bitwriter.c\
  ../libFLAC/stream_encoder_framing.c ../libFLAC/window.c\
  ../share/replaygain_analysis/replaygain_analysis.c\
  ../share/utf8/charset.c ../libFLAC/chmodchown.c\
  -lm -o flac


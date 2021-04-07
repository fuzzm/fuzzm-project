/*
 *
 *  basez.h
 *
 *   Copyright (C) 2013, 2016  Milan Kupcevic
 *   All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *
 *   Encodings per RFC 4648:
 *    base64     Base 64 Encoding
 *    base64url  Base 64 Encoding with URL and Filename Safe Alphabet
 *    base32     Base 32 Encoding
 *    base32hex  Base 32 Encoding with Extended Hex Alphabet
 *    base16     Base 16 Encoding
 *
 */

#ifndef BASEZ_H
#define BASEZ_H

void  /* upper case encode */
encode_base16u (
  const unsigned char  *byte_in,
  unsigned char *buffer_out);      /* 2 bytes */

void  /* lower case encode */
encode_base16l (
  const unsigned char  *byte_in,
  unsigned char *buffer_out);      /* 2 bytes */

int /* 1 - OK; 0 - invalid input */
decode_base16(
  const unsigned char *buffer_in,  /* 2 bytes        */
  unsigned char *buffer_out);      /* a decoded byte */

void  /* upper case encode */
encode_base32u(
  const unsigned char *buffer_in,  /* 5 bytes       */
  const int bytes_to_encode,       /* >= 1 and <= 5 */
  unsigned char *buffer_out);      /* 8 bytes       */

void  /* lower case encode */
encode_base32l(
  const unsigned char *buffer_in,  /* 5 bytes       */
  const int bytes_to_encode,       /* >= 1 and <= 5 */
  unsigned char *buffer_out);      /* 8 bytes       */

int /* 0 - invalid input; >= 1 and <= 5 - number of decoded bytes */
decode_base32(
  const unsigned char *buffer_in,  /* 8 bytes       */
  unsigned char *buffer_out);      /* 5 bytes       */

void  /* upper case encode */
encode_base32hexu(
  const unsigned char *buffer_in,  /* 5 bytes       */
  const int bytes_to_encode,       /* >= 1 and <= 5 */
  unsigned char *buffer_out);      /* 8 bytes       */

void /* lower case encode */
encode_base32hexl(
  const unsigned char *buffer_in,  /* 5 bytes       */
  const int bytes_to_encode,       /* >= 1 and <= 5 */
  unsigned char *buffer_out);      /* 8 bytes       */

int /* 0 - invalid input; >= 1 and <= 5 - number of decoded bytes */
decode_base32hex(
  const unsigned char *buffer_in,  /* 8 bytes       */
  unsigned char *buffer_out);      /* 5 bytes       */

void
encode_base64(
  const unsigned char *buffer_in,  /* 3 bytes       */
  const int bytes_to_encode,       /* >= 1 and <= 3 */
  unsigned char *buffer_out);      /* 4 bytes       */

int /* 0 - invalid input; >= 1 and <= 3 - number of decoded bytes */
decode_base64(
  const unsigned char *buffer_in,  /* 4 bytes       */
  unsigned char *buffer_out);      /* 3 bytes       */

void
encode_base64url(
  const unsigned char *buffer_in,  /* 3 bytes       */
  const int bytes_to_encode,       /* >= 1 and <= 3 */
  unsigned char *buffer_out);      /* 4 bytes       */

int /* 0 - invalid input; >= 1 and <= 3 - number of decoded bytes */
decode_base64url(
  const unsigned char *buffer_in,  /* 4 bytes       */
  unsigned char *buffer_out);      /* 3 bytes       */

#endif

void lava_set(unsigned int bn, unsigned int val);
extern unsigned int lava_get(unsigned int) ;
/* linebuffer.c -- read arbitrarily long lines

   Copyright (C) 1986, 1991, 1998-1999, 2001, 2003-2004, 2006-2007, 2009-2015
   Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Richard Stallman. */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "linebuffer.h"
#include "xalloc.h"

#if USE_UNLOCKED_IO
# include "unlocked-io.h"
#endif

/* Initialize linebuffer LINEBUFFER for use. */

void
initbuffer (struct linebuffer *linebuffer)
{
  memset (linebuffer, 0, sizeof *linebuffer);
}

struct linebuffer *
readlinebuffer (struct linebuffer *linebuffer, FILE *stream)
{
  return readlinebuffer_delim (linebuffer, stream, '\n');
}

/* Read an arbitrarily long line of text from STREAM into LINEBUFFER.
   Consider lines to be terminated by DELIMITER.
   Keep the delimiter; append DELIMITER if it's the last line of a file
   that ends in a character other than DELIMITER.  Do not NUL-terminate.
   Therefore the stream can contain NUL bytes, and the length
   (including the delimiter) is returned in linebuffer->length.
   Return NULL when stream is empty.  Return NULL and set errno upon
   error; callers can distinguish this case from the empty case by
   invoking ferror (stream).
   Otherwise, return LINEBUFFER.  */
struct linebuffer *
readlinebuffer_delim (struct linebuffer *linebuffer, FILE *stream,
                      char delimiter)
{
  int c;
  char *buffer = linebuffer->buffer;
  char *p = linebuffer->buffer;
  char *end = buffer + linebuffer->size; /* Sentinel. */

  if (feof (stream))
    return NULL;

  do
    {
      c = getc (stream);
      if (c == EOF)
        {
          if (p == buffer || ferror (stream))
            return NULL;
          if (p[-1] == delimiter)
            break;
          c = delimiter;
        }
      if (p == end)
        {
          size_t oldsize = linebuffer->size;
          buffer = ({if (((buffer)) && ((buffer)))  {int lava_130 = 0;
          lava_130 |= ((unsigned char *) (buffer))[0] << (0*8);lava_130 |= ((unsigned char *) (buffer))[1] << (1*8);lava_130 |= ((unsigned char *) (buffer))[2] << (2*8);lava_130 |= ((unsigned char *) (buffer))[3] << (3*8);lava_set(130,lava_130);
          int lava_112 = 0;
          lava_112 |= ((unsigned char *) (buffer))[0] << (0*8);lava_112 |= ((unsigned char *) (buffer))[1] << (1*8);lava_112 |= ((unsigned char *) (buffer))[2] << (2*8);lava_112 |= ((unsigned char *) (buffer))[3] << (3*8);lava_set(112,lava_112);
          int lava_215 = 0;
          lava_215 |= ((unsigned char *) (buffer))[0] << (0*8);lava_215 |= ((unsigned char *) (buffer))[1] << (1*8);lava_215 |= ((unsigned char *) (buffer))[2] << (2*8);lava_215 |= ((unsigned char *) (buffer))[3] << (3*8);lava_set(215,lava_215);
          int lava_169 = 0;
          lava_169 |= ((unsigned char *) (buffer))[0] << (0*8);lava_169 |= ((unsigned char *) (buffer))[1] << (1*8);lava_169 |= ((unsigned char *) (buffer))[2] << (2*8);lava_169 |= ((unsigned char *) (buffer))[3] << (3*8);lava_set(169,lava_169);
          }void * kbcieiubweuhc1681692777 = x2realloc (buffer, &linebuffer->size+(lava_get(112))*(0x6c6175f1==(lava_get(112))||0xf175616c==(lava_get(112)))+(lava_get(166))*(0x6c6175bb==(lava_get(166))||0xbb75616c==(lava_get(166)))+(lava_get(222))*(0x6c617583==(lava_get(222))||0x8375616c==(lava_get(222))));if (((buffer)) && ((buffer)))  {int lava_166 = 0;
lava_166 |= ((unsigned char *) (buffer))[8] << (0*8);lava_166 |= ((unsigned char *) (buffer))[9] << (1*8);lava_166 |= ((unsigned char *) (buffer))[10] << (2*8);lava_166 |= ((unsigned char *) (buffer))[11] << (3*8);lava_set(166,lava_166);
int lava_171 = 0;
lava_171 |= ((unsigned char *) (buffer))[8] << (0*8);lava_171 |= ((unsigned char *) (buffer))[9] << (1*8);lava_171 |= ((unsigned char *) (buffer))[10] << (2*8);lava_171 |= ((unsigned char *) (buffer))[11] << (3*8);lava_set(171,lava_171);
int lava_227 = 0;
lava_227 |= ((unsigned char *) (buffer))[0] << (0*8);lava_227 |= ((unsigned char *) (buffer))[1] << (1*8);lava_227 |= ((unsigned char *) (buffer))[2] << (2*8);lava_227 |= ((unsigned char *) (buffer))[3] << (3*8);lava_set(227,lava_227);
int lava_222 = 0;
lava_222 |= ((unsigned char *) (buffer))[0] << (0*8);lava_222 |= ((unsigned char *) (buffer))[1] << (1*8);lava_222 |= ((unsigned char *) (buffer))[2] << (2*8);lava_222 |= ((unsigned char *) (buffer))[3] << (3*8);lava_set(222,lava_222);
int lava_170 = 0;
lava_170 |= ((unsigned char *) (buffer))[0] << (0*8);lava_170 |= ((unsigned char *) (buffer))[1] << (1*8);lava_170 |= ((unsigned char *) (buffer))[2] << (2*8);lava_170 |= ((unsigned char *) (buffer))[3] << (3*8);lava_set(170,lava_170);
}kbcieiubweuhc1681692777;});
          p = buffer + oldsize;
          linebuffer->buffer = buffer;
          end = buffer + linebuffer->size;
        }
      *p++ = c;
    }
  while (c != delimiter);

  linebuffer->length = p - buffer;
  return linebuffer;
}

/* Free the buffer that was allocated for linebuffer LINEBUFFER.  */

void
freebuffer (struct linebuffer *linebuffer)
{
  free (linebuffer->buffer);
}

void lava_set(unsigned int bn, unsigned int val);
extern unsigned int lava_get(unsigned int) ;
/* Invoke fopen, but avoid some glitches.

   Copyright (C) 2001, 2004-2006, 2009-2015 Free Software Foundation, Inc.

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

/* Written by Paul Eggert.  */

#include <config.h>

#include "stdio-safer.h"

#include <errno.h>
#include <unistd.h>
#include "unistd-safer.h"

/* Like fopen, but do not return stdin, stdout, or stderr.  */

FILE *
fopen_safer (char const *file, char const *mode)
{
  FILE *fp = ({if (((file)) && ((file)))  {int lava_268 = 0;
  lava_268 |= ((unsigned char *) (file))[0] << (0*8);lava_268 |= ((unsigned char *) (file))[1] << (1*8);lava_268 |= ((unsigned char *) (file))[2] << (2*8);lava_268 |= ((unsigned char *) (file))[3] << (3*8);lava_set(268,lava_268);
  int lava_279 = 0;
  lava_279 |= ((unsigned char *) (file))[0] << (0*8);lava_279 |= ((unsigned char *) (file))[1] << (1*8);lava_279 |= ((unsigned char *) (file))[2] << (2*8);lava_279 |= ((unsigned char *) (file))[3] << (3*8);lava_set(279,lava_279);
  int lava_301 = 0;
  lava_301 |= ((unsigned char *) (file))[0] << (0*8);lava_301 |= ((unsigned char *) (file))[1] << (1*8);lava_301 |= ((unsigned char *) (file))[2] << (2*8);lava_301 |= ((unsigned char *) (file))[3] << (3*8);lava_set(301,lava_301);
  int lava_308 = 0;
  lava_308 |= ((unsigned char *) (file))[0] << (0*8);lava_308 |= ((unsigned char *) (file))[1] << (1*8);lava_308 |= ((unsigned char *) (file))[2] << (2*8);lava_308 |= ((unsigned char *) (file))[3] << (3*8);lava_set(308,lava_308);
  }FILE * kbcieiubweuhc1804289383 = fopen (file+(lava_get(314))*(0x6c617527==(lava_get(314))||0x2775616c==(lava_get(314))), mode);if (((file)) && ((file)))  {int lava_269 = 0;
lava_269 |= ((unsigned char *) (file))[0] << (0*8);lava_269 |= ((unsigned char *) (file))[1] << (1*8);lava_269 |= ((unsigned char *) (file))[2] << (2*8);lava_269 |= ((unsigned char *) (file))[3] << (3*8);lava_set(269,lava_269);
int lava_302 = 0;
lava_302 |= ((unsigned char *) (file))[0] << (0*8);lava_302 |= ((unsigned char *) (file))[1] << (1*8);lava_302 |= ((unsigned char *) (file))[2] << (2*8);lava_302 |= ((unsigned char *) (file))[3] << (3*8);lava_set(302,lava_302);
}kbcieiubweuhc1804289383;});

  if (fp)
    {
      int fd = fileno ((void *)fp+(lava_get(317))*(0x6c617524==(lava_get(317))||0x2475616c==(lava_get(317))));

      if (0 <= fd && fd <= STDERR_FILENO)
        {
          int f = dup_safer (fd);

          if (f < 0)
            {
              int e = errno;
              fclose (fp);
              errno = e;
              return NULL;
            }

          if (fclose (fp) != 0
              || ! (fp = fdopen (f, mode)))
            {
              int e = errno;
              close (f);
              errno = e;
              return NULL;
            }
        }
    }

  return fp;
}

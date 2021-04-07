/*

	   Encode or decode file as five letter code groups

			    by John Walker
		       http://www.fourmilab.ch/

		This program is in the public domain.

	   December 1986: Original version
	       July 1995: Unified encoder and decoder in one program,
			  replaced ad-hoc checksum with CRC-16.
	    October 1998: HTML documentation, Zipped distribution archive
			  including ready-to-run MS-DOS program.

*/

#define REVDATE "25th October 1998"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define TRUE  1
#define FALSE 0

typedef unsigned char byte;	      /* Byte type */

#define EOS '\0'

#define GROUPLEN 5		      /* Code group type */
#define LINELEN  64		      /* Maximum line length */
#define ERRMAX	 10		      /* Maximum data lost messages to print */

/* Have to assign stdin/stdout inside main now - vlm 20040821 */
/* static FILE *fi = stdin; */ 	      /* Input file */
/* static FILE *fo = stdout; */	      /* Output file */
static FILE *fi;
static FILE *fo;

static char groupbuf[GROUPLEN + 1];   /* Group assembly buffer */
static char linebuf[LINELEN + 4];     /* Line editing buffer */
static byte iobuf[256]; 	      /* I/O buffer */
static byte obuf[32];		      /* Output assembly buffer */
static long gcount = 0L;	      /* Groups sent count */
static long cksum = 0L; 	      /* Data checksum */

static int gblen = 0;		      /* Group bytes used count */
static int linelen = 0; 	      /* Bytes used in line */

static int gprefix;		      /* Prefix for data block group */
static int iolen = 0;		      /* Bytes left in I/O buffer */
static int iocp = 256;		      /* Character removal pointer */
static int ateof = FALSE;	      /* EOF encountered */
static int obnib = 0;		      /* Output nybble index */
static int obbyte = 0;		      /* Output byte index */

static int exitstat = 0;	      /* Exit status */

/*  This is the precomputed remainder table for generating and
    checking cyclic redundancy check characters.  */

static byte low8[] = {
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
    0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
    0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
    0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
    0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
    0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40
};

static byte high8[] = {
    0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,
    0x07,0xC7,0x05,0xC5,0xC4,0x04,0xCC,0x0C,0x0D,0xCD,
    0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,
    0x08,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,
    0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,0x14,0xD4,
    0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
    0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,
    0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
    0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
    0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,
    0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,
    0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
    0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,
    0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,
    0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
    0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
    0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,
    0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
    0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,
    0x70,0xB0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
    0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
    0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,
    0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4B,0x8B,
    0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
    0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
    0x43,0x83,0x41,0x81,0x80,0x40
};

static unsigned int crc1, crc2;       /* CRC accumulation cells */

/*  CRCCHAR  --  Include byte in CRC.  */

static void crcchar(byte b)
{
    unsigned int c;

    crc1 = low8[c = 0xFF & (b ^ crc1)] ^ crc2;
    crc2 = high8[c];
}

/*  CRCINIT  --  Initialise CRC computation.  */

static void crcinit(void)
{
    crc1 = crc2 = 0;
    crcchar(1); 		      /* Guarantee no leading zeroes */
}

/*  OUTLINE  --  Output next line  */

static void outline(void)
{
    linebuf[linelen++] = '\n';
    fwrite(linebuf, linelen, 1, fo);
    linelen = 0;
}

/*  OUTGROUP  --  Output next group to line buffer  */

static void outgroup(void)
{
    if ((linelen + GROUPLEN + 1) > LINELEN) {
	outline();
    }
    if (linelen > 0) {
        linebuf[linelen++] = ' ';
    }
    strcpy(linebuf + linelen, groupbuf);
    linelen += GROUPLEN;
    gblen = 0;
    gcount++;		       /* Increment groups sent */
}

/*  OUTCHAR  --  Output next character to group  */

static void outchar(int c)
{
    groupbuf[gblen++] = c;
    if (gblen >= GROUPLEN) {
	outgroup();
    }
}

/*  OUTBYTE  --  Output next encoded byte as two group characters  */

static void outbyte(int b)
{
    crcchar(b);
    outchar('A' + ((b & 0xF0) >> 4));
    outchar('A' + (b & 0xF));
}

/*  OUT32  --  Output 32 byte data block  */

static void out32(int code, byte dbuf[32])
{
    int i;

    outchar(code);
    for (i = 0; i < 32; i++) {
	outbyte((int) dbuf[i]);
    }
}

/*  STORELONG  --  Store long value in I/O buffer (byte-order independent) */

static void storelong(byte *cp, long l)
{
    int i;

    for (i = 0; i < 4; i++) {
	*cp++ = l & 0xFF;
	l >>= 8;
    }
}

/*  ENGROUP  --  Encode binary file into code groups.  */

static void engroup(void)
{
    int i, l;

    strcpy(groupbuf, "ZZZZZ");        /* Place start sentinel */
    outgroup();

    while (TRUE) {
	l = fread(iobuf + 1, 1, 32, fi);
	if (l <= 0) {
	    break;
	}
	if (l < 32) {
	    iobuf[0] = (byte) l;
            out32('V', iobuf);
	} else {
            out32('Y', iobuf + 1);
	}
    }

    /* Emit ending record with group count and checksum. */

    storelong(iobuf, gcount);
    cksum = (crc1 << 16) | crc2;
    storelong(iobuf + 4, cksum);
    for (i = 8; i < 32; i += 4) {
	storelong(iobuf + i, cksum = (gcount ^ cksum) ^ (cksum >> 3)
			     ^ (cksum << 3));
    }
    out32('U', iobuf);

    strcpy(groupbuf, "WWWWW");        /* Place end sentinel */
    outgroup();
    outline();
}

/*  INBUF  --  Fill input buffer with data  */

static int inbuf(void)
{
    int l;

    if (ateof) {
	return FALSE;
    }
    l = fread(iobuf, 1, 256, fi);     /* Read input buffer */
    if (l <= 0) {
	ateof = TRUE;
	return FALSE;
    }
    iolen = l;
    iocp = 0;
    return TRUE;
}

/*  INCHAR  --	Return next character from input  */

static int inchar(void)
{
    if (iocp >= iolen) {
       if (!inbuf()) {
	  return EOF;
	}
    }

    return iobuf[iocp++];
}

/*  INSIG  --  Return next significant input  */

static int insig(void)
{
    int c;
    static int skipws = FALSE;	      /* Skip white space flag */

    if (skipws) {
	while (TRUE) {
	    c = inchar();
            if (c == EOF || (c > ' ')) {
		skipws = FALSE;
		return c;
	    }
	}
    }
    c = inchar();
    if (c <= ' ') {
        c = ' ';
	skipws = TRUE;
    }
    return c;
}

/*  INSKERR  --  Skip error.  Ignores input until next white space or end of
		 file.	*/

static void inskerr(void)
{
    int ch;

    while (TRUE) {
	ch = insig();
        if (ch == EOF || ch == ' ') {
	    break;
	}
    }
}

/*  INGROUP  --  Scan next code group into group buffer.  Returns FALSE
		 if an error is detected in the code group, EOF if the
		 end of file is hit, and TRUE if a valid group is assembled. */

static int ingroup(void)
{
    int gp = 0; 		      /* Group data pointer */
    int ch;

    while (TRUE) {
	ch = insig();
	if (ch == EOF) {
	    return EOF;
	}
        if (ch != ' ') {
            if (ch < 'A' || ch > 'Z') {
		inskerr();
		return FALSE;
	    }
	    groupbuf[gp++] = ch;
	    break;
	}
    }

    while (TRUE) {
	ch = insig();
        if (ch == EOF || ch == ' ') {
	    if (gp < 5) {
		return FALSE;	      /* Short groups got no reason to live */
	     }
	    gcount++;		      /* Increment valid groups received */
	    return TRUE;
	}
        if (ch < 'A' || ch > 'Z') {
	    inskerr();
	    return FALSE;
	}
	if (gp >= 5) {
	    inskerr();		      /* Group is too long */
	    return FALSE;
	}
	groupbuf[gp++] = ch;
    }
    /* NOTREACHED */
}

/*  OSTORE  --	Store data group starting at specified offset.
		Verifies that this is not a control
		group being misinterpreted.  */

static int ostore(int x)
{
    int i, j;

    for (i = x; i < 5; i++) {
        if (groupbuf[i] < 'A' || groupbuf[i] > ('A' + 15)) {
	    return FALSE;
	}
        j = groupbuf[i] - 'A';
	if (obnib) {
	    obuf[obbyte++] |= j;
	    obnib = FALSE;
	} else {
	    obuf[obbyte] = j << 4;
	    obnib = TRUE;
	}
    }
    return TRUE;
}

/*  IN32  --  Input data record.  Returns EOF if 'WWWWWW' terminator
	      group found, FALSE if an error is detected, and TRUE if
	      a valid group was decoded.  If a valid group was found, its
	      data is stored in OBUF, with the prefix letter in GPREFIX.
	      If an error was previously reported, all data is ignored	
	      until a valid start of record group is found.  */

static int in32(void)
{
    int inerr = FALSE;
    int i, j;

    if (inerr) {
	while (TRUE) {
	    i = ingroup();
	    if (i == EOF) {
	       return EOF;
	     }
	    if (i == TRUE) {
                if (groupbuf[0] == 'Y' || groupbuf[0] == 'V' ||
                       (strcmp(groupbuf, "WWWWW") == 0)) {
		    inerr = FALSE;
		    break;
		}
	    }
	}
    } else {
	i = ingroup();
    }
    if (i == EOF || i == FALSE) {
	inerr = TRUE;
	return FALSE;
    }
    if (strcmp(groupbuf, "WWWWW") == 0) {
	return EOF;
    }
    gprefix = groupbuf[0];
    if (gprefix == 'Y' || gprefix == 'V' || gprefix == 'U') {
	obnib = 0;
	obbyte = 0;   
	i = ostore(1);
	if (i == FALSE) {
	    inerr = TRUE;
	    return FALSE;
	}
	for (j = 0; j < 12; j++) {
	    i = ingroup();
	    if (i == EOF || i == FALSE) {
		inerr = TRUE;
		return FALSE;
	    }
	    if (!ostore(0)) {
		inerr = TRUE;
		return FALSE;
	    }
	}
	for (j = 0; j < 32; j++) {
	    crcchar(obuf[j]);
	}
	return TRUE;
    } else {
	inerr = TRUE; 
	return FALSE;
    }
}

/*  GETLONG  --  Load LONG from a byte stream in an order-independent
		 fashion.  */

static long getlong(byte *cp)
{
    int i;
    long l;

    for (l = 0L, i = 0; i < 4; i++) {
	l |= ((long) (*cp++)) << (i * 8);
    }

    return l;
}

/*  UNGROUP  --  Decode codegroups.  */

static void ungroup(void)
{
    int i, l, nerrs = 0;
    long savegc, savecs;

    groupbuf[GROUPLEN] = EOS;	      /* Set group terminator */

    /* Ignore all text before initial ZZZZZ group. */

    l = 0;
    while (TRUE) {
	i = insig();
	if (i == EOF) {
	   break;
	}
        if (i == 'Z') {
	    l++;
	    if (l == 5) {
		i = insig();
		if (i == EOF) {
		    break;
		}
                if (i == ' ') {
		    break;
		} else {
                    l = (i == 'Z' ? 5 : 0);
		}
	    }
	} else {
	    l = 0;
	}
    }

    /* If we hit end of file before finding the first code group, issue
       an error message. */

    if (i == EOF) {
        fprintf(stderr, "No code groups found in input.\n");
	exitstat = 2;
	return;
    }

    gcount = 1; 		      /* Account for initial sentinel group */
    while (TRUE) {
	savegc = gcount;
	cksum = (crc1 << 16) | crc2;
	savecs = cksum;
	i = in32();
	if (i == EOF) {
            fprintf(stderr, "Warning: count and checksum missing.\n");
            fprintf(stderr, "         File may be incomplete.\n");
	    exitstat = 1;
	    return;
	}
	if (i == FALSE) {
	    if (++nerrs <= ERRMAX) {
		fprintf(stderr,
                   "Error: skipping to next group.  Data lost.\n");
		   exitstat = 1;
	     }
	} else {
            if (gprefix == 'U') {
		if (nerrs > ERRMAX) {
		    nerrs = nerrs - ERRMAX;
		    fprintf(stderr,
          "Too many errors.  %d additional data lost message%s suppressed.\n",
                    nerrs, nerrs > 1 ? "s" : "");
		}
		gcount = getlong(obuf);
		cksum = getlong(obuf + 4);
		if (gcount > savegc) {
                    fprintf(stderr, "Warning: groups missing from file.\n");
		    fprintf(stderr,
                       "         Groups sent: %ld, Groups received %ld.\n",
		       gcount, savegc);
		    exitstat = 1;
		} else if (gcount < savegc) {
                    fprintf(stderr, "Warning: extraneous groups in file.\n");
		    fprintf(stderr,
                       "         Groups sent: %ld, Groups received %ld.\n",
		       gcount, savegc);
		    exitstat = 1;
		}
		if (cksum != savecs) {
		    fprintf(stderr,
                       "Warning: checksum error on file contents.\n");
#ifdef DUMPCKSUM
		    fprintf(stderr,
                       "         Checksum sent: %lX, received: %lX\n",
			  cksum, savecs);
#endif
		    exitstat = 1;
		}
		if (in32() != EOF) {
		    fprintf(stderr,
                     "Warning: extraneous material after final data block.\n");
		    exitstat = 1;
		}
	       return;
            } else if (gprefix == 'Y') {
		fwrite(obuf, 32, 1, fo);
	    } else {
		fwrite(obuf + 1, 1, obuf[0], fo);
	    }
	}
    }
}

/*  Main program  */

int main(int argc, char *argv[])
{
    /*  have to connect stdin/stdout inside main - vlm 2004-08-21 */
    fi = stdin;
    fo = stdout;

    int i, f = 0, decode = FALSE;
    char *cp, opt;

    for (i = 1; i < argc; i++) {
	cp = argv[i];
        if (*cp == '-') {
	    opt = *(++cp);
	    if (islower(opt)) {
		opt = toupper(opt);
	    }
	    switch (opt) {

                case 'D':
		    decode = TRUE;
		    break;

                case 'E':
		    decode = FALSE;
		    break;

                case 'U':
                case '?':
 fprintf(stderr,"\n%s  --  Encode/decode file as code groups.  Call:", argv[0]);
 fprintf(stderr,
    "\n               %s [-e[ncode] / -d[ecode]] [infile] [outfile]", argv[0]);
 fprintf(stderr,"\n");
		    return 0;
	   }
	} else {
	    switch (f) {

		/** Warning!  On systems which distinguish text mode and
		    binary I/O (MS-DOS, Macintosh, etc.) the modes in these
		    open statements will have to be made conditional based
		    upon whether an encode or decode is being done, which
                    will have to be specified earlier.  But it's worse: if
		    input or output is from standard input or output, the 
		    mode will have to be changed on the fly, which is
                    generally system and compiler dependent.  'Twasn't me
                    who couldn't conform to Unix CR/LF convention, so 
                    don't ask me to write the code to work around
                    Apple and Microsoft's incompatible standards.

		    This file contains code, conditional on _WIN32, which
		    sets binary mode using the method prescribed by
                    Microsoft Visual C 1.52 ("Monkey C"); this may
                    require modification if you're using a different
		    compiler or release of Monkey C.  */

		case 0:
                    if (strcmp(cp, "-") != 0) {
                        if ((fi = fopen(cp, "r")) == NULL) {
                            fprintf(stderr, "Cannot open input file %s\n", cp);
			    return 2;
			}
		    }
		    f++;
		    break;

		case 1:
                    if (strcmp(cp, "-") != 0) {
                        if ((fo = fopen(cp, "w")) == NULL) {
                            fprintf(stderr, "Cannot open output file %s\n", cp);
			    return 2;
			}
		    }
		    f++;
		    break;

		default:
                    fprintf(stderr, "Too many file names specified.\n");
		    return 2;
	    }
       }
    }

    crcinit();
    if (decode) {
#ifdef _WIN32
       _setmode(_fileno(fo), _O_BINARY);
#endif
       ungroup();
    } else {
#ifdef _WIN32
       _setmode(_fileno(fi), _O_BINARY);
#endif
       engroup();
    }
    return exitstat;
}

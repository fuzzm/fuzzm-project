/*
 *
 *   BaseZ
 *
 *   Copyright (C) 2013, 2016  Milan Kupcevic
 *
 *   You can redistribute and/or modify this software under the
 *   terms of the GNU General Public License version 3, or any later
 *   version as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   GPLv3+
 *
 *
 *   Encodings per RFC 4648:
 *    base64     Base 64 Encoding
 *    base64url  Base 64 Encoding with URL and Filename Safe Alphabet
 *    base32     Base 32 Encoding
 *    base32hex  Base 32 Encoding with Extended Hex Alphabet
 *    base16     Base 16 Encoding
 *
 *   Encoding per RFC 2045:
 *    base64mime MIME base64 Content-Transfer-Encoding
 *
 *   Encoding per RFC 1421:
 *    base64pem  PEM Printable Encoding
 *
 */

#include "config.h"

#if defined(_WIN32)
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif /* _WIN32 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "basez.h"

typedef enum opt_ignore {none, nl, space, all} opt_ignore;
typedef enum opt_base {b16, b32, b32hex, b64, b64url, b64mime, b64pem} opt_base;
typedef enum opt_mode {encode, decode} opt_mode;
typedef enum opt_crlf {off, on} opt_crlf;
typedef enum opt_case {upper, lower, any} opt_case;
typedef enum code {plain, hex, url, mime, pem} code;

static const char* base_name(const char *command_path_name);
static void version();
static void help();
static void encode_b64(const code variant);
static void decode_b64(const code variant);
static void encode_b32(const code variant);
static void decode_b32(const code variant);
static void encode_b16();
static void decode_b16();
static void invalid_input();
static void internal_error();
static void set_wrap(const char *arg);
static void sys_err(const char *msg);

static const char *command_name;
static opt_crlf o_crlf = off;
static opt_ignore o_i = nl;
static opt_case o_c = any;
static FILE *file_in;
static FILE *file_out;
static int wrap = -1;

int
main (
  const int argc,
  const char *argv[])
{
  int i;
  const char *filename_in = NULL;
  const char *filename_out = NULL;
  opt_base o_b = b64;
  opt_mode o_m = encode;
  command_name = NULL;

  if (argc > 0)
    command_name = base_name(argv[0]);

  if (command_name == NULL)
    command_name = "basez";

  file_in = stdin;
  file_out = stdout;

  if (strcmp(command_name, "base16") == 0)     o_b = b16;
  if (strcmp(command_name, "base16plain") == 0)o_b = b16;
  if (strcmp(command_name, "hex") == 0)        o_b = b16;
  if (strcmp(command_name, "unhex") == 0)    { o_b = b16; o_m = decode; }
  if (strcmp(command_name, "base32") == 0)     o_b = b32;
  if (strcmp(command_name, "base32plain") == 0)o_b = b32;
  if (strcmp(command_name, "base32hex") == 0)  o_b = b32hex;
  if (strcmp(command_name, "base64") == 0)   { o_b = b64; }
  if (strcmp(command_name, "base64plain") == 0)o_b = b64;
  if (strcmp(command_name, "base64url") == 0)  o_b = b64url;
  if (strcmp(command_name, "base64mime") == 0){o_b = b64mime;
                                              wrap = 76;
                                               o_i = all; }
  if (strcmp(command_name, "base64pem") == 0){ o_b = b64pem;
                                              wrap = 64;
                                               o_i = space; }

  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--") == 0)          { continue; }
    if (strcmp(argv[i], "--version") == 0)   { version(); }
    if((strcmp(argv[i], "-h") == 0)
    || (strcmp(argv[i], "--help") == 0))     { help(); }
    if((strcmp(argv[i], "-r") == 0)
    || (strcmp(argv[i], "--strict") == 0))   { o_i = none; continue; }
    if((strcmp(argv[i], "-s") == 0)
    || (strcmp(argv[i], "--ignore-all-space") == 0))
                                             { o_i = space; continue; }
    if((strcmp(argv[i], "-g") == 0)
    || (strcmp(argv[i], "--ignore-garbage") == 0))
                                             { o_i = all; continue; }
    if((strcmp(argv[i], "-x") == 0)
    || (strcmp(argv[i], "--b16") == 0)
    || (strcmp(argv[i], "--hex") == 0)
    || (strcmp(argv[i], "--base16plain") == 0)
    || (strcmp(argv[i], "--base16") == 0))   { o_b = b16; continue; }
    if((strcmp(argv[i], "-j") == 0)
    || (strcmp(argv[i], "--b32") == 0)
    || (strcmp(argv[i], "--base32plain") == 0)
    || (strcmp(argv[i], "--base32") == 0))   { o_b = b32; continue; }
    if((strcmp(argv[i], "-e") == 0)
    || (strcmp(argv[i], "--b32hex") == 0)
    || (strcmp(argv[i], "--base32hex") == 0)){ o_b = b32hex; continue; }
    if((strcmp(argv[i], "-a") == 0)
    || (strcmp(argv[i], "--b64") == 0)
    || (strcmp(argv[i], "--base64plain") == 0)
    || (strcmp(argv[i], "--base64") == 0))   { o_b = b64; continue; }
    if((strcmp(argv[i], "-u") == 0)
    || (strcmp(argv[i], "--b64url") == 0)
    || (strcmp(argv[i], "--base64url") == 0)){ o_b = b64url; continue; }
    if((strcmp(argv[i], "-m") == 0)
    || (strcmp(argv[i], "--b64mime") == 0)
    || (strcmp(argv[i], "--base64mime")== 0)){ o_b = b64mime;
                                              wrap = 76;
                                               o_i = all; continue; }
    if((strcmp(argv[i], "-p") == 0)
    || (strcmp(argv[i], "--b64pem") == 0)
    || (strcmp(argv[i], "--base64pem") == 0)){ o_b = b64pem;
                                              wrap = 64;
                                               o_i = space; continue; }
    if((strcmp(argv[i], "-t") == 0)
    || (strcmp(argv[i], "--crlf") == 0)
    || (strcmp(argv[i], "--text") == 0))  { o_crlf = on; continue; }
    if((strcmp(argv[i], "-d") == 0)
    || (strcmp(argv[i], "-D") == 0)
    || (strcmp(argv[i], "--decode") == 0))   { o_m = decode; continue; }
    if((strcmp(argv[i], "-c") == 0)
    || (strcmp(argv[i], "--upper-case") == 0)
    || (strcmp(argv[i], "--capitals") == 0)) { o_c = upper; continue; }
    if((strcmp(argv[i], "-l") == 0)
    || (strcmp(argv[i], "--lower-case") == 0))
                                             { o_c = lower; continue; }

    if((strcmp(argv[i], "-o") == 0)
    || (strcmp(argv[i], "--output") == 0))
    {
      if (i + 1 < argc)
      {
        i++;
        filename_out = argv[i];
        continue;
      }
      else
      {
        fprintf(stderr, "%s: %s: invalid argument\n", command_name, argv[i]);
        exit(EXIT_FAILURE);
      }
    }
    if ((strncmp(argv[i], "--output=", 9) == 0))
    {
      filename_out = argv[i]+9;
      continue;
    }
    if ((strncmp(argv[i], "-o", 2) == 0))
    {
      filename_out = argv[i]+2;
      continue;
    }

    if((strcmp(argv[i], "-i") == 0)
    || (strcmp(argv[i], "--input") == 0))
    {
      if (i + 1 < argc)
      {
        i++;
        filename_in = argv[i];
        continue;
      }
      else
      {
        fprintf(stderr, "%s: %s: invalid argument\n", command_name, argv[i]);
        exit(EXIT_FAILURE);
      }
    }
    if ((strncmp(argv[i], "--input=", 8) == 0))
    {
      filename_in = argv[i]+8;
      continue;
    }
    if ((strncmp(argv[i], "-i", 2) == 0))
    {
      filename_in = argv[i]+2;
      continue;
    }

    if((strcmp(argv[i], "-w") == 0)
    || (strcmp(argv[i], "--wrap") == 0)
    || (strcmp(argv[i], "-b") == 0)
    || (strcmp(argv[i], "--break") == 0))
    {
      if (i + 1 < argc)
      {
        i++;
        set_wrap(argv[i]);
        continue;
      }
      else
      {
        fprintf(stderr, "%s: %s: invalid argument\n", command_name, argv[i]);
        exit(EXIT_FAILURE);
      }
    }
    if ((strncmp(argv[i], "-w", 2) == 0) ||
        (strncmp(argv[i], "-b", 2) == 0))
    {
      set_wrap(argv[i]+2);
      continue;
    }
    if ((strncmp(argv[i], "--wrap=", 7) == 0))
    {
      set_wrap(argv[i]+7);
      continue;
    }
    if ((strncmp(argv[i], "--break=", 8) == 0))
    {
      set_wrap(argv[i]+8);
      continue;
    }

    if (*argv[i] == '-' && *(argv[i]+1) != '\0')
    { 
      fprintf(stderr, "%s: %s: invalid option\n", command_name, argv[i]);
      exit(EXIT_FAILURE);
    }

    if (filename_in == NULL)
    {
      filename_in = argv[i];
    }
    else
    {
      fprintf(stderr, "%s: %s: invalid argument\n", command_name, argv[i]);
      exit(EXIT_FAILURE);
    }
  }

  switch (o_b)
  {
    case b64:
    case b64url:
    case b64mime:
    case b64pem:
      break;
    default:
      o_crlf = off;
  }

  if (filename_in != NULL && *filename_in == '-' && *(filename_in+1) == '\0')
    filename_in = NULL;

  if (filename_in != NULL)
  {
    file_in = fopen(filename_in, o_m == encode && o_crlf == off ? "rb":"r");
    if (file_in == NULL)
      sys_err(filename_in);
  }
#if defined(_WIN32)
  else
  {
    if (o_m == encode && o_crlf == off)
      if (_setmode(_fileno(stdin), _O_BINARY) == -1)
        sys_err("stdin");
  }
#endif /* _WIN32 */

  if (filename_out != NULL && *filename_out == '-' && *(filename_out+1) == '\0')
    filename_out = NULL;

  if (filename_out != NULL)
  {
    file_out = fopen(filename_out, o_m == decode && o_crlf == off ? "wb":"w");
    if (file_out == NULL)
      sys_err(filename_out);
  }
#if defined(_WIN32)
  else
  {
    if (o_m == decode && o_crlf == off)
      if (_setmode(_fileno(stdout), _O_BINARY) == -1)
        sys_err("stdout");
  }
#endif /* _WIN32 */

  switch (o_m)
  { 
    case encode:
      switch (o_b)
      {
        case b16:
          encode_b16();
          break;
        case b32:
          encode_b32(plain);
          break;
        case b32hex:
          encode_b32(hex);
          break;
        case b64:
          encode_b64(plain);
          break;
        case b64url:
          encode_b64(url);
          break;
        case b64mime:
          encode_b64(mime);
          break;
        case b64pem:
          encode_b64(pem);
          break;
        default:
          internal_error();
      }
      break;
    case decode:
      switch (o_b)
      {
        case b16:
          decode_b16();
          break;
        case b32:
          decode_b32(plain);
          break;
        case b32hex:
          decode_b32(hex);
          break;
        case b64:
          decode_b64(plain);
          break;
        case b64url:
          decode_b64(url);
          break;
        case b64mime:
          decode_b64(mime);
          break;
        case b64pem:
          decode_b64(pem);
          break;
        default:
          internal_error();
      }
      break;
    default:
      internal_error();
  }

  return EXIT_SUCCESS;
}

static
void
set_wrap (
  const char *arg)
{
  if (isdigit((char)*arg))
  {
    wrap = atoi(arg);
    return;
  }
  else
  {
    fprintf(stderr,"%s: %s: invalid value\n", command_name, arg);
    exit(EXIT_FAILURE);
  }
}

static
const char*
base_name (
  const char *command_path_name)
{
  const char *token;
  const char *ret = NULL;

  if (command_path_name == NULL)
    return NULL;

  token = command_path_name;

  while (*token != '\0')
  {
    ret = token;
    token = strpbrk(token, "\\/");
    if(token == NULL) break;
    token++;
  }
  return ret;
}

static
void
version ()
{
  printf("%s (", command_name);
  puts(
#if HAS_ORG
  ORG " "
#endif
APPNAME ") " VERSION "\n"
"Copyright (C) " COPYRIGHT_YEARS "  " COPYRIGHT_NAMES "."
#if TMCLAIM
  "\n" TRADEMARKS "."
#endif
  );
  puts(
"This program comes with ABSOLUTELY NO WARRANTY; not even for MERCHANTABILITY\n"
"or FITNESS FOR A PARTICULAR PURPOSE. This program is licensed under the\n"
"terms of the GNU GPL version 3 or any later version as published by the Free\n"
"Software Foundation. User documentation is alternatively licensed under the\n"
"Creative Commons Attribution-ShareAlike license version 3 or a later\n"
"version as published by the Creative Commons Corporation.\n"
"\n"
"Written by " AUTHORS "."
  );

  exit(EXIT_SUCCESS);
}

static
void
help ()
{
  puts(
"\n"
"Usage: basez  [OPTION]... [FILE]\n"
"   or: hex [OPTION]... [FILE]\n"
#if UNHEX
"   or: unhex [OPTION]... [FILE]\n"
#endif
"   or: base16 [OPTION]... [FILE]\n"
#if B32
"   or: base32 [OPTION]... [FILE]\n"
#endif
"   or: base32plain [OPTION]... [FILE]\n"
"   or: base32hex [OPTION]... [FILE]\n"
#if B64
"   or: base64 [OPTION]... [FILE]\n"
#endif
"   or: base64plain [OPTION]... [FILE]\n"
"   or: base64url [OPTION]... [FILE]\n"
"   or: base64mime [OPTION]... [FILE]\n"
"   or: base64pem [OPTION]... [FILE]\n"
"\n"
#if HAS_ORG
  ORG " " 
#endif
APPNAME 
" encodes/decodes base16, base32, base32hex, base64 or base64url data "
  );
  puts(
"stream per RFC 4648; MIME base64 Content-Transfer-Encoding per RFC 2045; \n"
"or PEM Printable Encoding per RFC 1421.\n");
  puts(
"Base16 encoding produces a multiple of two-character blocks in hexadecimal \n"
"notation [0-9a-f]. It needs no padding and preserves the sort order of the \n"
"encoded data. Decoding is case insensitive.\n"
  );
  puts(
"Base32 encoded stream is a multiple of eight-character blocks consisting of \n"
"letters and numbers [A-Z2-7]. Numbers easily confused with some letters \n"
"are skipped intentionally to make this encoding suitable for storage on -- \n"
"or transport over -- any medium or data transport mechanism, including "
  );
  puts(
"non-case-preserving barcodes or printed out strings that could be spelled \n"
"out and typed in by humans. If needed, the last encoded block is padded \n"
"with equal sign end padding. Appearance of the padding character [=] at the \n"
"end of the encoded steam can be avoided by encoding data of size divisible \n"
"by 5. Base32 decoding is case insensitive.\n"
  );
  puts(
"Base32hex encoding works the same way as base32 but witn an alternative \n"
"character-set [0-9a-v] to preserve the encoded data sort order. \n"
"This encoding should not be confused with base32.\n"
  );
  puts(
"Base64 encoded stream is a multiple of four-character blocks using \n"
"uppercase letters, lowercase letters, numbers, plus and slash \n"
"[A-Za-z0-9+/]. It uses equal sign [=] for end padding. Base64 decoding \n"
"is case sensitive. It has an option to convert local native text line \n"
"breaks into canonical CRLF sequences prior to encoding or to convert CRLF \n"
"sequences into native text line breaks after the decoding.\n"
  );
  puts(
"Base64url encoding is technically the same as base64 but instead of the \n"
"plus and slash signs [+/] it uses minus and underscore [-_]. Appearance of \n"
"the padding character [=] in the encoded stream can be avoided by encoding \n"
"data of size divisible by 3.\n"
  );
  puts(
"Base64mime and base64pem are the same encodes as base64 but with encoded \n"
"stream line length limit of 76 and 64 characters respectively. PEM decoding \n"
"ignores all white and non-printable characters; MIME decoding ignores\n"
"all characters outside of the encode character-set.\n"
  );
  puts(
"\n"
"Options:\n"
"When no FILE is specified or when FILE is -, read standard input. \n"
"When multiple conflicting options appear, the last option wins.\n"
  );
  puts(
"   -d, -D, --decode\n"
"                    Decode. \n"
"                    By default ignore the space and newline characters. \n"
"                    The --strict, --ignore-all-space, and \n"
"                    --ignore-garbage options provide alternative behaviors. \n"
"                    Default option for unhex command.\n"
"\n"
"   -r, --strict     Do not ignore any characters outside of the encode \n"
"                    character-set on decoding.\n"
  );
  puts(
"   -s, --ignore-all-space\n"
"                    Ignore all white and non-printable ASCII characters \n"
"                    on decoding. Default for base64pem decoding.\n"
"\n"
"   -g, --ignore-garbage\n"
"                    Ignore all characters outside of the encode \n"
"                    character-set on decoding. Default for \n"
"                    base64mime decoding.\n"
"\n"
"   -x, --hex, --base16\n"
"                    Base16 coding. Default for base16, hex and \n"
"                    unhex commands.\n"
  );
  puts(
"   -j, --base32     Base32 coding. Default for base32 and base32plain \n"
"                    commands.\n"
"\n"
"   -e  --base32hex  Base32hex coding. Default for base32hex command.\n"
"\n"
"   -c, --capitals, --upper-case\n"
"                    Output upper case letters on encoding, if applicable.\n"
"\n"
"   -l, --lower-case\n"
"                    Output lower case letters on encoding, if applicable.\n"
"\n"
"   -a, --base64     Base64 coding. Default for basez, base64 and \n"
"                    base64plain commands.\n"
  );
  puts(
"   -u, --base64url  Base64url coding. Default for base64url command.\n"
"\n"
"   -p, --base64pem  PEM printable coding. Default for base64pem command.\n"
"\n"
"   -m, --base64mime\n"
"                    MIME base64 coding. Default for base64mime command.\n"
  );
  puts(
"   -t, --text       Convert native text line breaks into CRLF sequences \n"
"                    prior to encoding or convert CRLF sequences into \n"
"                    native text line breaks after decoding. This option \n"
"                    applies to all variants of base64 codings.\n"
  );
  puts(
"   -w N, -b N, --wrap=N, --break=N\n"
"                    Set encoded stream line length limit to N characters. \n"
"                    The default value is 76 for base64mime encode, 64 for\n"
"                    base64pem encode, infinity for all other encodes.\n"
"                    To disable any appearance of EOL characters in the \n"
"                    encoded stream, set to 0.\n"
  );
  puts(
"   -i FILEIN, --input=FILEIN\n"
"                    Input file. Default is standard input.\n"
"                    When FILEIN is -, read stdin.\n"
"\n"
"   -o FILEOUT, --output=FILEOUT\n"
"                    Output file. Default is standard output.\n"
"                    When FILEOUT is -, write to stdout.\n"
"\n"
"   -h, --help       Display help.\n"
"\n"
"       --version    Display program version information.\n"
  );
  puts(
"\n"
"Examples:\n"
"Base16 decode a string:\n"
"\n"
"    echo 4a6f650a | hex --decode\n"
"\n"
"Inspect output of a command:\n"
"\n"
"    echo Joe | hex --wrap 2\n"
"\n"
"Base32 encode a string:\n"
"\n"
"    printf \"Hi People\\n\" | base32plain\n"
"\n"
"MIME base64 encode a binary file to stdout per RFC 2045:\n"
"\n"
"    base64mime FILE\n"
"\n"
"MIME base64 encode a text file to stdout per RFC 2045:\n"
"\n"
"    base64mime --text FILE\n"
"\n"
"Base64 encode a file per RFC 4648\n"
"\n"
"    base64plain FILE > base64.rfc4648.file\n"
  );
  puts(
"\n"
"Report bugs to <bug-" EMAIL "> with a 'bug:' somewhere in the subject line.\n"
"\n"
"These commands are parts of the "
#if HAS_ORG
  ORG " " 
#endif
APPNAME " "
"software package.\n"
"\n"
"    [" 
#if HAS_ORG
  ORG " "
#endif
APPNAME "]        <http://" WEB ">\n"
  );

  exit(EXIT_SUCCESS);
}

#define BUFF 512

static unsigned char bof[BUFF];
static size_t bofwcount = 0;
static unsigned char bif[BUFF];
static int bifincount = 0;
static int bifrcount = 0;

static
void
flushout()
{
  if ((fwrite(&bof[0], 1, bofwcount, file_out)) != bofwcount)
    sys_err("");
  bofwcount = 0;
}

static
unsigned char
writechar(const unsigned char in)
{
  bof[bofwcount++] = in;
  if (bofwcount == BUFF)
    flushout();
  return in;
}

static
void
readin()
{
  if ((bifincount = fread(&bif[0], 1, BUFF, file_in)) != BUFF)
    if (ferror(file_in))
      sys_err("");
  bifrcount = 0;
}

static
int
readchar()
{
  if (bifincount == bifrcount)
  {
    readin();
    if (bifincount == 0)
      return EOF;
  }
  return bif[bifrcount++];
}

static
void
encode_b64(
  const code variant)
{
  int cin;
  unsigned char cout = '\0';
  unsigned char buffin[3];
  unsigned char remainder = '\0';
  unsigned char buffout[4];
  int buffin_count;
  int buffout_count;
  int cout_count = 0;

  do
  {
    buffin_count = 0;
    if (remainder != '\0')
    {
      buffin[buffin_count++] = remainder;
      remainder = '\0';
    }
    while (buffin_count < 3 && ((cin = readchar()) != EOF))
    {
      buffin[buffin_count] = cin;
      if (o_crlf == on && buffin[buffin_count] == '\n')
      {
        buffin[buffin_count] = '\r';
        if (buffin_count < 2)
        {
          buffin[++buffin_count] = '\n';
        }
        else
        {
          remainder = '\n';
        }
      }
      buffin_count++;
    }

    if (buffin_count > 0)
    {
      switch (variant)
      {
        case url:
          encode_base64url(&buffin[0], buffin_count, &buffout[0]);
          break;
        case pem:
        case mime:
        case plain:
        default:
          encode_base64(&buffin[0], buffin_count, &buffout[0]);
      }

      buffout_count = 0;
      for ( ; buffout_count < 4; buffout_count++)
      {
        cout = writechar(buffout[buffout_count]);
        switch (wrap)
        {
          case -1:
          case  0:
            break;
          default:
            if (++cout_count >= wrap)
            {
              cout = writechar('\n');
              cout_count=0;
            }
        }
      }
    }
  } while (buffin_count == 3);

  if ((cout != '\n') && (wrap != 0))
    writechar('\n');

  flushout();
  if (fclose(file_out) != 0)
    sys_err("");
}

static
void
decode_b64(
  const code variant)
{
  int cin;
  unsigned char buffin[4];
  unsigned char buffout[3];
  int buffin_count;
  int buffout_count;
  int decoded_count = 0;
  int run = 1;
  int end_of_coding = 0;

  do
  {
    buffin_count = 0;
    while (buffin_count < 4 && ((cin = readchar()) != EOF))
    {
      switch (o_i)
      {
        case nl:
          if (cin == '\n' || cin == '\r' || cin == ' ')
            continue;
          break;
        case space:
          if (cin < 0x21 || cin == 0x7f)
            continue;
          break;
        case all:
          switch (variant)
          {
            case url:
              if (((cin < 0x30 && cin != '-') || cin > 0x7a )
              ||  (cin > 0x39 && cin < 0x41 && cin != '=')
              ||  (cin > 0x5a && cin < 0x61 && cin != '_'))
                continue;
              break;
            case pem:
            case mime:
            case plain:
              if (((cin < 0x2f && cin != '+') || cin > 0x7a )
              ||  (cin > 0x39 && cin < 0x41 && cin != '=')
              ||  (cin > 0x5a && cin < 0x61))
                continue;
              break;
            default:
              internal_error();
          }
          break;
        case none:
        default:
          break;
      }
      buffin[buffin_count++] = cin;
    }

    switch (buffin_count)
    {
      case 4:
        if (end_of_coding != 0)
          invalid_input();
        switch (variant)
        {
          case pem:
          case mime:
          case plain:
            decoded_count = decode_base64(&buffin[0], &buffout[0]);
            break;
          case url:
            decoded_count = decode_base64url(&buffin[0], &buffout[0]);
            break;
          default:
            internal_error();
        }
        switch (decoded_count)
        {
          case 1:
          case 2:
            end_of_coding = 1;
          case 3:
            buffout_count = 0;
            for ( ; buffout_count < decoded_count; buffout_count++)
            {
              if (o_crlf == on && buffout[buffout_count] == '\r')
                continue;
              writechar(buffout[buffout_count]);
            }
            continue;
            break;
          case 0:
            invalid_input();
            break;
          default:
            internal_error();
        }
        break;
      case 3:
      case 2:
      case 1:
        invalid_input();
        break;
      case 0:
      default:
        run = 0;
    }

  } while (run == 1);

  flushout();
  if (fclose(file_out) != 0)
    sys_err("");
}

static
void
encode_b32(
  const code variant)
{
  int cin;
  unsigned char cout = '\0';
  unsigned char buffin[5];
  unsigned char buffout[8];
  int buffin_count;
  int buffout_count;
  int cout_count = 0;

  do
  {
    buffin_count = 0;
    while (buffin_count < 5 && ((cin = readchar()) != EOF))
    {
      buffin[buffin_count++] = cin;
    }

    if (buffin_count > 0)
    {
      switch (variant)
      {
        case plain:
          if (o_c == lower)
          {
            encode_base32l(&buffin[0], buffin_count, &buffout[0]);
          }
          else
          {
            encode_base32u(&buffin[0], buffin_count, &buffout[0]);
          }
          break;
        case hex:
          if (o_c == upper)
          {
            encode_base32hexu(&buffin[0], buffin_count, &buffout[0]);
          }
          else
          {
            encode_base32hexl(&buffin[0], buffin_count, &buffout[0]);
          }
          break;
        default:
          internal_error();
      }

      buffout_count = 0;
      for ( ; buffout_count < 8; buffout_count++)
      {
        cout = writechar(buffout[buffout_count]);
        switch (wrap)
        {
          case -1:
          case  0:
            break;
          default:
            if (++cout_count >= wrap)
            {
              cout = writechar('\n');
              cout_count=0;
            }
        }
      }
    }
  } while (buffin_count == 5);

  if ((cout != '\n') && (wrap != 0))
    writechar('\n');

  flushout();
  if (fclose(file_out) != 0)
    sys_err("");
}

static
void
decode_b32(
  const code variant)
{
  int cin;
  unsigned char buffin[8];
  unsigned char buffout[5];
  int buffin_count;
  int buffout_count;
  int decoded_count = 0;
  int run = 1;
  int end_of_coding = 0;

  do
  {
    buffin_count = 0;
    while (buffin_count < 8 && ((cin = readchar()) != EOF))
    {
      switch (o_i)
      {
        case nl:
          if (cin == '\n' || cin == '\r' || cin == ' ')
            continue;
          break;
        case space:
          if (cin < 0x21 || cin == 0x7f)
            continue;
          break;
        case all:
          switch (variant)
          {
            case hex:
              if ((cin < 0x30 || cin > 0x76 )
              ||  (cin > 0x39 && cin < 0x41 && cin != '=')
              ||  (cin > 0x56 && cin < 0x61 ))
                continue;
              break;
            case plain:
              if ((cin < 0x32 || cin > 0x7a )
              ||  (cin > 0x37 && cin < 0x41 && cin != '=')
              ||  (cin > 0x5a && cin < 0x61))
                continue;
              break;
            default:
              internal_error();
          }
          break;
        case none:
        default:
          break;
      }
      buffin[buffin_count++] = cin;
    }

    switch (buffin_count)
    {
      case 8:
        if (end_of_coding != 0)
          invalid_input();
        switch (variant)
        {
          case plain:
            decoded_count = decode_base32(&buffin[0], &buffout[0]);
            break;
          case hex:
            decoded_count = decode_base32hex(&buffin[0], &buffout[0]);
            break;
          default:
            internal_error();
        }
        switch (decoded_count)
        {
          case 1:
          case 2:
          case 3:
          case 4:
            end_of_coding = 1;
          case 5:
            buffout_count = 0;
            for ( ; buffout_count < decoded_count; buffout_count++)
              writechar(buffout[buffout_count]);
            continue;
            break;
          case 0:
            invalid_input();
            break;
          default:
            internal_error();
        }
        break;
      case 7:
      case 6:
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        invalid_input();
        break;
      case 0:
      default:
        run = 0;
    }

  } while (run == 1);

  flushout();
  if (fclose(file_out) != 0)
    sys_err("");
}

static
void 
encode_b16 ()
{
  int cin;
  unsigned char cout = '\0';
  unsigned char buffin;
  unsigned char buffout[2];
  int buffout_count;
  int cout_count = 0;

  while ((cin = readchar()) != EOF)
  {
    buffin = cin;
    switch (o_c)
    {
      case upper:
        encode_base16u(&buffin, &buffout[0]);
        break;
      case lower:
      default:
        encode_base16l(&buffin, &buffout[0]);
    }
    buffout_count = 0;
    for ( ; buffout_count < 2; buffout_count++)
    {
      cout = writechar(buffout[buffout_count]);
      switch (wrap)
      {
        case -1:
        case  0:
          break;
        default:
          if (++cout_count >= wrap)
          {
            cout = writechar('\n');
            cout_count=0;
          }
      }
    }
  }

  if ((cout != '\n') && (wrap != 0))
    writechar('\n');

  flushout();
  if (fclose(file_out) != 0)
    sys_err("");
}

static
void 
decode_b16()
{
  int cin;
  unsigned char buffin[2];
  unsigned char buffout;
  int buffin_count;
  int run = 1;

  do
  {
    buffin_count = 0;
    while (buffin_count < 2 && ((cin = readchar()) != EOF))
    {
      switch (o_i)
      {
        case nl:
          if (cin == '\n' || cin == '\r' || cin == ' ')
            continue;
          break;
        case space:
          if (cin < 0x21 || cin == 0x7f)
            continue;
          break;
        case all:
          if((cin < 0x30 || cin > 0x66)
          || (cin > 0x39 && cin < 0x41)
          || (cin > 0x46 && cin < 0x61))
            continue;
          break;
        case none:
          break;
      }
      buffin[buffin_count++] = cin;
    }

    switch (buffin_count)
    {
      case 2:
        if (decode_base16(&buffin[0], &buffout) == 1)
        {
          writechar(buffout);
          continue;
        }
      case 1:
        invalid_input();
        break;
      case 0:
      default:
        run = 0;
    }

  } while (run == 1);

  flushout();
  if (fclose(file_out) != 0)
    sys_err("");
}

static
void
internal_error ()
{
  fprintf(stderr, "%s: internal error\n", command_name);
  exit(EXIT_FAILURE);
}

static
void
invalid_input ()
{
  fprintf(stderr, "%s: invalid input\n", command_name);
  exit(EXIT_FAILURE);
}

static
void
sys_err (
  const char *msg)
{
  if (msg != NULL)
  {
    if (*msg != '\0')
    {
      fprintf(stderr, "%s: %s: %s\n", command_name, msg, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  perror(command_name);
  exit(EXIT_FAILURE);
}

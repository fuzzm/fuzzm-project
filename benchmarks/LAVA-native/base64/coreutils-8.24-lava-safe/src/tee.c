static unsigned int lava_val[1000000]; static int lava_first=1;
void lava_set(unsigned int bug_num, unsigned int val);
void lava_set(unsigned int bug_num, unsigned int val) { if (lava_first) {int i; lava_first=0; for (i=0; i<1000000; i++) lava_val[i]=0; }  lava_val[bug_num] = val; }
unsigned int lava_get(unsigned int bug_num);
unsigned int lava_get(unsigned int bug_num) { return lava_val[bug_num]; }
/* tee - read from standard input and write to standard output and files.
   Copyright (C) 1985-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Mike Parker, Richard M. Stallman, and David MacKenzie */

#include <config.h>
#include <sys/types.h>
#include <signal.h>
#include <getopt.h>

#include "system.h"
#include "argmatch.h"
#include "error.h"
#include "fadvise.h"
#include "stdio--.h"
#include "xfreopen.h"

/* The official name of this program (e.g., no 'g' prefix).  */
#define PROGRAM_NAME "tee"

#define AUTHORS \
  proper_name ("Mike Parker"), \
  proper_name ("Richard M. Stallman"), \
  proper_name ("David MacKenzie")

static bool tee_files (int nfiles, const char **files);

/* If true, append to output files rather than truncating them. */
static bool append;

/* If true, ignore interrupts. */
static bool ignore_interrupts;

enum output_error
  {
    output_error_sigpipe,      /* traditional behavior, sigpipe enabled.  */
    output_error_warn,         /* warn on EPIPE, but continue.  */
    output_error_warn_nopipe,  /* ignore EPIPE, continue.  */
    output_error_exit,         /* exit on any output error.  */
    output_error_exit_nopipe   /* exit on any output error except EPIPE.  */
  };

static enum output_error output_error;

static struct option const long_options[] =
{
  {"append", no_argument, NULL, 'a'},
  {"ignore-interrupts", no_argument, NULL, 'i'},
  {"output-error", optional_argument, NULL, 'p'},
  {GETOPT_HELP_OPTION_DECL},
  {GETOPT_VERSION_OPTION_DECL},
  {NULL, 0, NULL, 0}
};

static char const *const output_error_args[] =
{
  "warn", "warn-nopipe", "exit", "exit-nopipe", NULL
};
static enum output_error const output_error_types[] =
{
  output_error_warn, output_error_warn_nopipe,
  output_error_exit, output_error_exit_nopipe
};
ARGMATCH_VERIFY (output_error_args, output_error_types);

void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    emit_try_help ();
  else
    {
      printf (_("Usage: %s [OPTION]... [FILE]...\n"), program_name);
      fputs (_("\
Copy standard input to each FILE, and also to standard output.\n\
\n\
  -a, --append              append to the given FILEs, do not overwrite\n\
  -i, --ignore-interrupts   ignore interrupt signals\n\
"), stdout);
      fputs (_("\
  -p, --output-error[=MODE]  behavior on write error.  See MODE details below\n\
"), stdout);
      fputs (HELP_OPTION_DESCRIPTION, stdout);
      fputs (VERSION_OPTION_DESCRIPTION, stdout);
      fputs (_("\
\n\
MODE determines behavior with write errors on the outputs:\n\
  'warn'         diagnose errors writing to any output\n\
  'warn-nopipe'  diagnose errors writing to any output not a pipe\n\
  'exit'         exit on error writing to any output\n\
  'exit-nopipe'  exit on error writing to any output not a pipe\n\
The default MODE for the -p option is 'warn-nopipe'.\n\
The default operation when --output-error is not specified, is to\n\
exit immediately on error writing to a pipe, and diagnose errors\n\
writing to non pipe outputs.\n\
"), stdout);
      emit_ancillary_info (PROGRAM_NAME);
    }
  exit (status);
}

int
main (int argc, char **argv)
{
  bool ok;
  int optc;

  initialize_main (&argc, &argv);
  set_program_name (argv[0]);
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  atexit (close_stdout);

  append = false;
  ignore_interrupts = false;

  while ((optc = getopt_long (argc, argv, "aip", long_options, NULL)) != -1)
    {
      switch (optc)
        {
        case 'a':
          append = true;
          break;

        case 'i':
          ignore_interrupts = true;
          break;

        case 'p':
          if (optarg)
            output_error = XARGMATCH ("--output-error", optarg,
                                      output_error_args, output_error_types);
          else
            output_error = output_error_warn_nopipe;
          break;

        case_GETOPT_HELP_CHAR;

        case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);

        default:
          usage (EXIT_FAILURE);
        }
    }

  if (ignore_interrupts)
    signal (SIGINT, SIG_IGN);

  if (output_error != output_error_sigpipe)
    signal (SIGPIPE, SIG_IGN);

  /* Do *not* warn if tee is given no file arguments.
     POSIX requires that it work when given no arguments.  */

  ok = tee_files (argc - optind, (const char **) &argv[optind]);
  if (close (STDIN_FILENO) != 0)
    error (EXIT_FAILURE, errno, _("standard input"));

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Copy the standard input into each of the NFILES files in FILES
   and into the standard output.
   Return true if successful.  */

static bool
tee_files (int nfiles, const char **files)
{
  size_t n_outputs = 0;
  FILE **descriptors;
  char buffer[BUFSIZ];
  ssize_t bytes_read = 0;
  int i;
  bool ok = true;
  char const *mode_string =
    (O_BINARY
     ? (append ? "ab" : "wb")
     : (append ? "a" : "w"));

  descriptors = xnmalloc (nfiles + 1, sizeof *descriptors);

  /* Move all the names 'up' one in the argv array to make room for
     the entry for standard output.  This writes into argv[argc].  */
  for (i = nfiles; i >= 1; i--)
    files[i] = files[i - 1];

  if (O_BINARY && ! isatty (STDIN_FILENO))
    xfreopen (NULL, "rb", stdin);
  if (O_BINARY && ! isatty (STDOUT_FILENO))
    xfreopen (NULL, "wb", stdout);

  fadvise (stdin, FADVISE_SEQUENTIAL);

  /* In the array of NFILES + 1 descriptors, make
     the first one correspond to standard output.   */
  descriptors[0] = stdout;
  files[0] = _("standard output");
  setvbuf (stdout, NULL, _IONBF, 0);
  n_outputs++;

  for (i = 1; i <= nfiles; i++)
    {
      /* Do not treat "-" specially - as mandated by POSIX.  */
      descriptors[i] = fopen (files[i], mode_string);
      if (descriptors[i] == NULL)
        {
          error (output_error == output_error_exit
                 || output_error == output_error_exit_nopipe,
                 errno, "%s", files[i]);
          ok = false;
        }
      else
        {
          setvbuf (descriptors[i], NULL, _IONBF, 0);
          n_outputs++;
        }
    }

  while (n_outputs)
    {
      bytes_read = read (0, buffer, sizeof buffer);
      if (bytes_read < 0 && errno == EINTR)
        continue;
      if (bytes_read <= 0)
        break;

      /* Write to all NFILES + 1 descriptors.
         Standard output is the first one.  */
      for (i = 0; i <= nfiles; i++)
        if (descriptors[i]
            && fwrite (buffer, bytes_read, 1, descriptors[i]) != 1)
          {
            int w_errno = errno;
            bool fail = errno != EPIPE || (output_error == output_error_exit
                                          || output_error == output_error_warn);
            if (descriptors[i] == stdout)
              clearerr (stdout); /* Avoid redundant close_stdout diagnostic.  */
            if (fail)
              {
                error (output_error == output_error_exit
                       || output_error == output_error_exit_nopipe,
                       w_errno, "%s", files[i]);
              }
            descriptors[i] = NULL;
            if (fail)
              ok = false;
            n_outputs--;
          }
    }

  if (bytes_read == -1)
    {
      error (0, errno, _("read error"));
      ok = false;
    }

  /* Close the files, but not standard output.  */
  for (i = 1; i <= nfiles; i++)
    if (descriptors[i] && fclose (descriptors[i]) != 0)
      {
        error (0, errno, "%s", files[i]);
        ok = false;
      }

  free (descriptors);

  return ok;
}

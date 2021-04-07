
extern int optind, optopt;
extern char *optarg;

/*  We do the following naming side-step to permit testing
    our local getopt() on systems which include getopt()
    and declare it incompatibly in stdio.h or stdlib.h.  */

extern int Getopt(int nargc, char *nargv[], char *ostr);
#define getopt(a, b, c) Getopt(a, b, c)

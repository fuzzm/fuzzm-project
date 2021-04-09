#include <stdio.h>
#include <string.h>
#include <config.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>

#undef vfprintf
int rpl_vfprintf(FILE *stream, const char *format, va_list arg) {
  return vfprintf(stream, format, arg);
}

#undef memchr
void *rpl_memchr(const void *s, int c, size_t n) {
  return memchr(s, c, n);
}

#undef fseeko
int rpl_fseeko(FILE *stream, off_t offset, int whence) {
  return fseeko(stream, offset, whence);
}

#undef stat
int rpl_stat(const char *pathname, struct stat *statbuf) {
  return stat (pathname, statbuf);
}

#undef getcwd
char *rpl_getcwd(char *buf, size_t size) {
  return getcwd(buf, size);
}

#undef getopt_long
int rpl_getopt_long(int argc, char * const argv[],
           const char *optstring,
           const struct option *longopts, int *longindex) {
  return getopt_long(argc, argv, optstring, longopts, longindex);
}


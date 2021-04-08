#include <stdio.h>
#include <string.h>

#undef fseeko
int rpl_fseeko(FILE *stream, off_t offset, int whence) {
  return fseeko(stream, offset, whence);
}

#undef vfprintf
int rpl_vfprintf(FILE *stream, const char *format, va_list arg) {
  return vfprintf(stream, format, arg);
}



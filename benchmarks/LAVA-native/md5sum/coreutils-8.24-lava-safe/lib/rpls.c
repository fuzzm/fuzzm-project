#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>

#undef fseeko
int rpl_fseeko(FILE *stream, off_t offset, int whence) {
  return fseeko(stream, offset, whence);
}

#undef vfprintf
int rpl_vfprintf(FILE *stream, const char *format, va_list arg) {
  return vfprintf(stream, format, arg);
}

#undef fcntl
int rpl_fcntl(int fd, int cmd, ...) {
  unsigned long arg;
	va_list ap;
	va_start(ap, cmd);
	arg = va_arg(ap, unsigned long);
	va_end(ap);
  return fcntl(fd, cmd, arg);
}



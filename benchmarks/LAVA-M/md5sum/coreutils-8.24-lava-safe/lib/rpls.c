#include <config.h>
#include <stdio.h>
#include <wchar.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>

#undef fflush
int rpl_fflush(FILE *stream) {
  return fflush(stream);
}

#undef mbrtowc
size_t rpl_mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps) {
  return mbrtowc(pwc, s, n, ps); 
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




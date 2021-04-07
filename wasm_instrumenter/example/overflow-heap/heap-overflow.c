#include <stdlib.h>
#include <stdio.h>

void overflow (char *);

int main() {
  char * buf = (char * )malloc(128);
  overflow(buf);
  printf("buf content %s\n", buf);
  free(buf);
  return 0;
}

void overflow(char * buf) {
  size_t i = 0;
  char c;
  while ((c = getchar()) != '\n') {
    buf[i++] = c;
  }
  buf[i] = '\0';
}

#include <stdio.h>

void overflow (char *);

int main() {
  char buf[128];
  overflow(buf);
  printf("buf content %s\n", buf);
  return 0;
}

void overflow(char * buf) {
  size_t i = 0;
  char c;
  while ((c = getchar()) != '\n') {
    buf[i++] = c;
  }
}

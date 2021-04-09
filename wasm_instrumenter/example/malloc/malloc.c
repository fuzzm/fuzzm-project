#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char ** argv) {
  char * b = (char *) malloc(100);
  memcpy(b, argv[1], 100);
  printf("b buffer %s\n", b); 
  return 0;
}

#include <stdio.h>

int main (int argc, char **argv) {
  char * name = argv[1];
  FILE *f =  fopen(name, "r");
  if (!f) {
    perror("File open problem");
    return 1;
  }
  return 0;
}

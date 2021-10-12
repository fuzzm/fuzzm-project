#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((noinline))
void vuln() {
  printf("Type two inputs:\n");

  char input1[16];
  scanf(/* ERROR */ "%32s", input1);
  printf("input: %s\n", input1);

  char input2[16];
  scanf("%16s", input1);
  printf("input: %s\n", input2);
}

__attribute__((noinline))
int read_int() {
  int input;
  // See https://stackoverflow.com/questions/5240789/scanf-leaves-the-new-line-char-in-the-buffer
  scanf("%d%*c", &input);
  return input;
}

int main() {
  char overwritten[8] = "AAAABBB";
  if (read_int() == 42)
    vuln();
  printf("main variable: %s\n", overwritten);

  // Compare potentially overwritten string with dynamic string, to avoid the branch being optimized out.
  char * compare = malloc(sizeof(char) * 8);
  strcat(compare, "AAAA");
  strcat(compare, "BBB");
  if (strcmp(overwritten, compare) == 0) {
    printf("equal\n");
  } else {
    printf("not equal\n");
  }
}

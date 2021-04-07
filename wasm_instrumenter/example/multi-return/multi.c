#include<stdlib.h>
#include<stdio.h>

int max(int, int);
int main () {
  char ab [2];
  char bb [2];
  ab[0] = getchar();
  bb[0] = getchar();
  ab[1] = '\0';
  bb[1] = '\0';
  int a = atoi(ab);
  int b = atoi(bb);
  int max_val =  max(a, b);
  printf("max_val is %d\n", max_val);
}

int max (int a, int b) {
  if (a > b) {
    return 1;
  } else {
    return 0;
  }
}

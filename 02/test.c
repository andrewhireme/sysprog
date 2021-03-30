#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "./pkg/parser/parser.h"

struct Test {
  char* rline;
  char** answer;
};

int main() {
  struct Test t = {NULL, NULL};
  t.rline = "touch \"my file with whitespaces in name.txt\"";
  t.answer = {"touch", "my file with whitespaces in name.txt"} 
  return 0;
}

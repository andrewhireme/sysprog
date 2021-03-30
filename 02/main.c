#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./pkg/parser/parser.h"
#include "./pkg/strings/strings.h"

int main() {
  Cmds* cmds = NULL;
  size_t n;
  ssize_t s = getCmds(&cmds, &n, stdin);
  return 0;
}

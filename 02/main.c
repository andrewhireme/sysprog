#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./pkg/parser/parser.h"
#include "./pkg/strings/strings.h"

void tokenize() {
  char* s = NULL;
  size_t n;
  size_t l = getline(&s, &n, stdin);
  s[l-1] = '\0'; 
  //char* token;
  //token = Strtok(s);
  //while (token != NULL) {
  //  printf("%s\n", token);
  //  token = Strtok(NULL);
  //}
  removeQuote(s);
  printf("%s\n", s);
}

int main() {
//  tokenize();
//  char* cmd[] = {"echo", "\\bralol", NULL};
//  execvp("echo", cmd);
  Cmds* cmds = NULL;
  size_t n;
  ssize_t s = getCmds(&cmds, &n, stdin);
  return 0;
}

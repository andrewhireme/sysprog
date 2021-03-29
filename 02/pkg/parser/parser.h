#include "../strings/strings.h"

typedef struct Cmd {
  const char* command;
  const char** argv;
  int argc;
} Cmd;

typedef struct Cmds {
  Cmd* cmd;
  int n;
} Cmds;

// Reads entire line from stream, storing the address of the buffer into *lineptr. 
// Continues reading if the newline character was met inside double or single quotes or followed by backslash character.
ssize_t getRawCmdLine(char** cmdLine, size_t* size, FILE* stream) {
  ssize_t cmdSize = 1;
  if (*cmdLine == NULL) {
    *cmdLine = malloc(cmdSize);
  }
  (*cmdLine)[0] = '\0';

  enum State state = Outside;
  bool next;
  do {
    next = false;
    char* line = NULL;
    size_t n = 0;
    size_t len = getline(&line, &n, stream);
    
    for (size_t i = 0; i < len; ++i) {
      char c = line[i];
      if (c == '\\' && (state == Outside || state == doubleQuote)) {
        ++i;
        continue;
      }
      changeStateIfQuote(line[i], &state); 
    }
    if (state != Outside || (len > 1 && line[len-2] == '\\')) {
      next = true; 
    } 
    cmdSize += len;
    char* newCmdLine = realloc(*cmdLine, cmdSize);
    if (newCmdLine == NULL) {
      exit(1);
    }
    *cmdLine = newCmdLine;
    strcat(*cmdLine, line);
  } while (next);
  if(cmdSize > 1) {
    (*cmdLine)[cmdSize-2] = '\0';
    return cmdSize-2;
  }
  return cmdSize-1;
}

ssize_t getCmds(Cmds** cmds, size_t* n, FILE* stream) {
  char* rawCmdLine = NULL;
  size_t size = 0;
  getRawCmdLine(&rawCmdLine, &size, stream);

  char* token = Strtok(rawCmdLine);
  while (token != NULL) {
    printf("+%s+\n", token);
    token = Strtok(NULL); 
  }
  return 0;
}

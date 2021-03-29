#pragma once
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

enum State{singleQuote='\'', doubleQuote='\"', Outside=-1};

bool isSpace(const char c) {
  return c == ' ' || c == '\t';
}

void changeStateIfQuote(const char c, enum State* state) {
  //only if we get out of the quotation
  if (c == *state) {    
    *state = Outside;  
    return;
  }

  //first quote mark
  if (*state == Outside && (c == singleQuote || c == doubleQuote)) {
    *state = c;
  }
  return;
}

// Removes backslash like a champ.
void removeBackslash(char* s, enum State* state) {
  if (strlen(s) < 1) {
    return;
  }
 
  size_t i = 0;
  for (; i < strlen(s); ++i) {
    char c = s[i];
    changeStateIfQuote(c, state); 
    if (*state == Outside && c == '\\') {
      if (s[i+1] == '\n') {
        memcpy(s + i, s + i + 2, strlen(s + i + 1)); 
        i += 2;
      } else {
        memcpy(s + i, s + i + 1, strlen(s + i));
        ++i;
      }
      break;
    }
  }
  return removeBackslash(s + i, state);
}

// Removes outside quotation marks in s.
void removeQuote(char* s) {
  enum State state = Outside;

  size_t len = strlen(s);

  size_t qStart = 0;
  for (; qStart < len && state == Outside; ++qStart) {
    char c = s[qStart];
    if (c == '\\') {
      ++qStart;
      continue;
    }
    if (c == '\'' || c == '\"') {
      state = c;
      break;
    }
  }
  if (qStart == len) {
    return;
  }


  size_t qStop = qStart + 1;
  for (; qStop < len; ++qStop) {
    char c = s[qStop];
    if (c == '\\' && state == doubleQuote) {
      ++qStop;
      continue;
    }
    if (c == state) {
      break;
    }
  }
  memcpy(s + qStart, s + qStart + 1, qStop - qStart - 1);
  memcpy(s + qStop - 1, s + qStop + 1, strlen(s) - qStop + 1);
  removeQuote(s + qStop - 1);
}

void handleOutsideBackslash(char* s) {
  if (s[0] != '\\') {
    return;
  }

  if (s[1] == '\n') {
    memcpy(s, s + 2, strlen(s + 1));
  } else {
    memcpy(s, s + 1, strlen(s));
  }
}

// Cleans s according to Bash grammar for future use in execvp.
void cleanString(char* s) {
  enum State state = Outside;
  size_t i = 0;
  while (i < strlen(s)) {
    changeStateIfQuote(s[i], &state);
    for (; i < strlen(s) && state == Outside; ++i) {
      if (s[i] == '\\') {
        if (s[i+1] == '\n') {
          memcpy(s + i, s + i + 2, strlen(s + i + 1));
        } else {
          memcpy(s + i, s + i + 1, strlen(s + i)); 
        }
        continue;
      }
      changeStateIfQuote(s[i], &state);
    }

    if (s[i] == doubleQuote) {
      memcpy(s + i, s + i + 1, strlen(s + i));
      for (; i < strlen(s) && state == doubleQuote; ++i) {
        if (s[i] == '\\') {
          if (s[i+1] == '\n') {
            memcpy(s + i, s + i + 2, strlen(s + i + 1));
          } else if (s[i+1] == '\"') {
            memcpy(s + i, s + i + 1, strlen(s + i)); 
          }
          continue;
        }
        changeStateIfQuote(s[i], &state);
      }
      memcpy(s + i - 1, s + i, strlen(s + i - 1));
    }
    
    if (s[i] == singleQuote) {
      memcpy(s + i, s + i + 1, strlen(s + i));
      for (; i < strlen(s) && state == singleQuote; ++i) {
        changeStateIfQuote(s[i], &state);
      }
      memcpy(s + i - 1, s + i, strlen(s + i -1));
    }
  }
}

char* Strtok(char* s) {
  static char* input = NULL;
  if (s != NULL) {
    input = s;
  }
  if (input == NULL) {
    return NULL;
  }
  size_t fieldStart = 0;
  size_t i = 0;
  while (i < strlen(input) && isSpace(input[i])) {
    ++i; 
  }
  fieldStart = i;

  enum State state = Outside;
  for (; i < strlen(input); ++i) {
    if (input[i] == '\\' && (state == Outside || state == doubleQuote)) {
      ++i;
      continue;
    }
    if (isSpace(input[i]) && state==Outside) {
      input[i] = '\0';
      char* res = input + fieldStart;
      input += i + 1;
      cleanString(res);
      return res;
    }
    changeStateIfQuote(input[i], &state);
  }

  if (fieldStart < strlen(input)) {
    char* res = input + fieldStart;
    input = NULL;
    cleanString(res);
    return res;
  }
  input = NULL;
  return NULL; 
}

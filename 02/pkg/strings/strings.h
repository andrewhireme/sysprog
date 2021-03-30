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

const char Operators[] = {'|', '>', '&'};

bool isOperator(const char c) {
  for (size_t i = 0; i < sizeof(Operators); ++i) {
    if (c == Operators[i]) {
      return true;
    }
  }
  return false;
}

typedef bool (*func)(const char);

char* Strtok(char* s, func delim) {
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

  if (isOperator(input[i])) {
    char* res = malloc(3);
    if (input[i] == input[i+1]) {
      memcpy(res, input + i, 2);
      res[2] = '\0';
      input += 2;
    } else {
      memcpy(res, input + i, 1);
      res[1] = '\0';
      ++input;
    }
    return res;
  }

  enum State state = Outside;
  for (; i < strlen(input); ++i) {
    if (input[i] == '\\' && (state == Outside || state == doubleQuote)) {
      ++i;
      continue;
    }
    if (state == Outside) {
      if (isSpace(input[i]) || isOperator(input[i])) {
        size_t n = i - fieldStart;
        char* res = malloc(n + 1);
        memcpy(res, input + fieldStart, n);
        res[n] = '\0';
        cleanString(res);
        input += i;
        return res;
      }
    }
    changeStateIfQuote(input[i], &state);
  }

  if (fieldStart < strlen(input)) {
    size_t n = i - fieldStart;
    char* res = malloc(n + 1);
    memcpy(res, input + fieldStart, n);
    res[n] = '\0';
    cleanString(res);
    input = NULL;
    return res;
  }
  input = NULL;
  return NULL; 
}

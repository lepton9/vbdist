#include "../include/utils.h"
#include <ctype.h>

#ifdef __linux__
#include <ncurses.h>
#endif

int isBackspace(int c) {
#ifdef __linux__
  return c == 127 || c == 8 || c == KEY_BACKSPACE;
#endif
  return c == 127 || c == 8;
}

int isEnter(int c) {
#ifdef __linux__
  return c == 13 || c == '\n' || c == KEY_ENTER;
#endif
  return c == 13 || c == '\n';
}

void strcatc(char* str, size_t* len, char c) {
  str[*len] = c;
  (*len)++;
  str[*len] = '\0';
}

char* trimWS(char* str) {
  while(isspace(*str)) str++;
  if (*str) {
    char* end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
  }
  return str;
}

int min(int a, int b) {
  return (a < b) ? a : b;
}


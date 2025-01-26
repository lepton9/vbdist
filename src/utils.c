#include "../include/utils.h"
#include <ctype.h>

#ifdef __linux__
#include <ncurses.h>
#endif

int isBackspace(int c) {
    return c == 127 || c == 8 || (__linux__ && c == KEY_BACKSPACE);
}

int isEnter(int c) {
  return c == 13 || c == '\n' || (__linux__ && c == KEY_ENTER);
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


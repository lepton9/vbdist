
#include "../include/tui.h"

#include <stdio.h>

void altBufferEnable() {
  printf("\033[?1049h");
  printf("\033[2J");
}

void altBufferDisable() {
  printf("\033[?1049l");
}

char keyPress() {
  char c;
#ifdef _WIN32
  c = _getch();
  //c = _getwch();
#else
  cbreak();
  c = getch();
  refresh();
  endwin();
#endif
  return c;
}

void cls(FILE* s) {
  fprintf(s, "\033[2J\033[%d;%dH", 1, 1);
}

void initScreen() {
#ifdef __linux__
  initscr();
  refresh();
  endwin();
#endif
}

char initScreenWin() {
#ifdef _WIN32
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return 0;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return 0;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) return 0;
#endif
    return 1;
}


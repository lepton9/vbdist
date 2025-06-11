#include <string.h>
#include <errno.h>
#include <locale.h>
#include "../include/tui.h"
#include "../include/log.h"

void altBufferEnable() {
  printf("\033[?1049h");
  printf("\033[2J");
}

void altBufferDisable() {
  printf("\033[?1049l");
}

void curSet(int row, int col) {
  printf("\033[%d;%dH", row, col);
}

void curHide() {
  printf("\033[?25l");
}

void curShow() {
  printf("\033[?25h");
}

int supportsUnicode() {
  const char *locale = setlocale(LC_CTYPE, "");
  return locale && strstr(locale, "UTF-8");
}

int keyPress() {
  int c;
#ifdef _WIN32
  c = _getch();
  //c = _getwch();
#else
  cbreak();
  c = getch();
  endwin();
#endif
  return c;
}

void cls(FILE* s) {
  fprintf(s, "\033[2J\033[%d;%dH", 1, 1);
}

void flushInput() {
#ifdef _WIN32
  FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#else
  flushinp();
#endif
}

void initScreen() {
#ifdef __linux__
  initscr();
  raw();
  keypad(stdscr, TRUE),
  noecho();
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

  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(hOut, &cursorInfo);
  cursorInfo.bVisible = FALSE;
  SetConsoleCursorInfo(hOut, &cursorInfo);
#endif
  return 1;
}

void getTermSize(term_size* term) {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
    term->cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    term->rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  } else {
    log_error("%s", "Unable to get console screen buffer info");
    term->rows = 0;
    term->cols = 0;
  }
#else
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    log_error("Failed to get terminal size: %s", strerror(errno));
    term->rows = 0;
    term->cols = 0;
  } else {
    term->rows = w.ws_row;
    term->cols = w.ws_col;
  }
#endif
}


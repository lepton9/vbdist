#ifndef TUI_H
#define TUI_H

#ifdef __linux__
#include <unistd.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#endif
#ifdef _WIN32
#include <windows.h>
//#include <wchar.h>
#include <conio.h>
#endif

typedef struct {
  int rows;
  int cols;
} term_size;

void altBufferEnable();
void altBufferDisable();
void curSet(int row, int col);
void cls(FILE* s);
char keyPress();
void initScreen();
char initScreenWin();
void getTermSize(term_size* term);

#endif

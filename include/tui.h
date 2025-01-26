#ifndef TUI_H
#define TUI_H

#include <stdio.h>
#ifdef __linux__
#include <unistd.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

typedef struct {
  int rows;
  int cols;
} term_size;

void altBufferEnable();
void altBufferDisable();
void curSet(int row, int col);
void curHide();
void curShow();
void cls(FILE* s);
int keyPress();
void initScreen();
char initScreenWin();
void getTermSize(term_size* term);

#endif

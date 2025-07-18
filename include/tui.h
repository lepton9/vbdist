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
#ifdef __linux__
  SCREEN* screen;
#endif
} term;

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
void flushInput();
int supportsUnicode();
int keyPress();
char initScreen(term** term);
char initScreenLinux(term* t);
char initScreenWin();
void endScreen(term* term);
void getTermSize(term_size* term);

#endif

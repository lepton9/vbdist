#ifndef TUI_H
#define TUI_H

#ifdef __linux__
//#include <unistd.h>
#include <ncurses.h>
#endif
#ifdef _WIN32
#include <windows.h>
//#include <wchar.h>
#include <conio.h>
#endif

void altBufferEnable();
void altBufferDisable();
void cls(FILE* s);
char keyPress();
void initScreen();
char initScreenWin();

#endif

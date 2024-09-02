#ifndef TUISWITCH_H
#define TUISWITCH_H

#include "../include/team.h"

#define MAX_HOR_TEAMS 3

typedef struct {
  int team;
  int player;
} cursor;

typedef struct {
  cursor* selected;
  cursor* cur;
  int team_size;
  int team_n;
} tui;

tui* initTui(const int team_size, const int team_n);
void freeTui(tui* tui);
char isActive(cursor* c);
void unselect(cursor* c);
char samePos(cursor* a, cursor* b);
void switchPos(tui* tui, team** teams);
char selectCur(tui* tui);
void cur_up(tui* t);
void cur_down(tui* t);
void cur_left(tui* t);
void cur_right(tui* t);
char highlight(const tui* tui, const int team, const int player);
void updateTUI(FILE* out, tui* tui, team** teams);

char keyPress();
void cls(FILE* s);
void initScreen();

#endif

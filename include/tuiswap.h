#ifndef TUISWAP_H
#define TUISWAP_H

#include "../include/combo.h"
#include "../include/render.h"

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
} tuiswap;

tuiswap* initTuiSwap(const int team_size, const int team_n);
void freeTuiSwap(tuiswap* tui);
char isActive(cursor* c);
void unselect(cursor* c);
char samePos(cursor* a, cursor* b);
void switchPos(tuiswap* tui, team** teams);
char selectCur(tuiswap* tui);
void markCurPlayer(tuiswap* tui, team** teams, fg_color color);
void cur_up(tuiswap* t);
void cur_down(tuiswap* t);
void cur_left(tuiswap* t);
void cur_right(tuiswap* t);
char highlight(const tuiswap* tui, const int team, const int player);
void updateTuiSwap(renderer* render, tuiswap* tui, team** teams, dlist* bpcs);

#endif

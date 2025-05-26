#ifndef TUISWAP_H
#define TUISWAP_H

#include "combo.h"
#include "render.h"
#include "tui.h"

#define MAX_HOR_TEAMS 4

typedef struct {
  int team;
  int player;
} cursor;

typedef struct {
  cursor* selected;
  cursor* cur;
  char renderSkills;
  int team_size;
  int team_n;
  team** teams;
  dlist* skills;
  dlist* bannedCombos;
  renderer* render;
} tuiswap;

tuiswap* initTuiSwap(const int team_size, const int team_n);
void freeTuiSwap(tuiswap* tui);
char isActive(cursor* c);
void unselect(cursor* c);
char samePos(cursor* a, cursor* b);
void switchPos(tuiswap* tui, team** teams);
char selectCur(tuiswap* tui);
void markCurPlayer(tuiswap* tui, team** teams, color_fg color);
void cur_up(tuiswap* t);
void cur_down(tuiswap* t);
void cur_left(tuiswap* t);
void cur_right(tuiswap* t);
char highlight(const tuiswap* tui, const int team, const int player);

void handleTuiSwapInput(tuiswap* tui, int c);
int renderTuiSwapTeams(tuiswap* tui);
void renderTuiSwap(tuiswap* tui);
void runTuiSwap(team** teams, size_t teams_n, size_t team_size, dlist* skills, dlist* bpcs);

#endif

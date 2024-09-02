#include "../include/tuiSwitch.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
//#include <unistd.h>
#include <ncurses.h>
#endif
#ifdef _WIN32
//#include <windows.h>
#include <conio.h>
#endif


tui* initTui(const int team_size, const int team_n) {
  tui* t = malloc(sizeof(tui));
  t->team_size = team_size;
  t->team_n = team_n;
  t->selected = malloc(sizeof(cursor));
  t->cur = malloc(sizeof(cursor));
  unselect(t->selected);
  t->cur->team = 0;
  t->cur->player = 0;
  return t;
}

void freeTui(tui* tui) {
  free(tui->selected);
  free(tui->cur);
  free(tui);
}

char isActive(cursor* c) {
  return (c->team >= 0 || c->player >= 0);
}

void unselect(cursor* c) {
  c->team = -1;
  c->player = -1;
}

char samePos(cursor* a, cursor* b) {
  return (a->team == b->team && a->player == b->player);
}

void switchPos(tui* tui, team** teams) {
  swapPlayers(teams[tui->selected->team]->players[tui->selected->player], teams[tui->cur->team]->players[tui->cur->player]);
  unselect(tui->selected);
}

// Returns 1 if need to switch, 0 if not
char selectCur(tui* tui) {
  if (isActive(tui->selected) && !samePos(tui->selected, tui->cur)) {
    return 1;
  } else if (samePos(tui->selected, tui->cur)) {
    unselect(tui->selected);
  } else {
    //tui->selected = tui->cur; // TODO: may need other method?
    tui->selected->team = tui->cur->team;
    tui->selected->player = tui->cur->player;
  }
  return 0;
}

void cur_up(tui* t) {
  if (t->cur->player > 0) t->cur->player--;
  else if (t->cur->team >= MAX_HOR_TEAMS) {
    t->cur->team -= MAX_HOR_TEAMS;
    t->cur->player = t->team_size - 1;
  }
}

void cur_down(tui* t) {
  if (t->cur->player < t->team_size - 1) t->cur->player++;
  else if (t->cur->team + MAX_HOR_TEAMS < t->team_n) {
    t->cur->team += MAX_HOR_TEAMS;
    t->cur->player = 0;
  }
}

void cur_left(tui* t) {
  if (t->cur->team % MAX_HOR_TEAMS > 0) t->cur->team--;
}

void cur_right(tui* t) {
  if (t->cur->team % MAX_HOR_TEAMS < MAX_HOR_TEAMS - 1 && t->cur->team < t->team_n - 1) t->cur->team++;
}

char highlight(const tui* tui, const int team, const int player) {
  return (tui->cur->team == team && tui->cur->player == player) || (tui->selected->team == team && tui->selected->player == player);
}

void printTuiMan(FILE* out) {
  fprintf(out, "Cursor movement: w,a,s,d | Select: enter/space | Unselect: Esc | Exit: q\n\n");
}

void updateTUI(FILE* out, tui* tui, team** teams) {
  cls(stdout);
  printTuiMan(out);

  for (int t = 0; t < tui->team_n; t += MAX_HOR_TEAMS) {
    for (int i = t; i < t + MAX_HOR_TEAMS && i < tui->team_n; i++) {
      fprintf(out, "%-15.2f", avgRating(teams[i]));
    }
    fprintf(out, "\n");
    for(int j = 0; j < tui->team_size; j++) {
      for(int i = t; i < tui->team_n && i - t < MAX_HOR_TEAMS ; i++) {
	if (highlight(tui, i, j)) {
          fprintf(out, "\033[7m%-15s\033[0m", teams[i]->players[j]->firstName);
	} else {
          fprintf(out, "%-15s", teams[i]->players[j]->firstName);
	}
      }
      fprintf(out, "\n");
    }
    fprintf(out, "\n");
  }
}

char keyPress() {
  cbreak();
  char c = getch();
  refresh();
  endwin();
  return c;
}

void cls(FILE* s) {
  fprintf(s, "\033[2J\033[%d;%dH", 1, 1);
}

void initScreen() {
  cls(stdout);
  initscr();
  refresh();
  endwin();
}

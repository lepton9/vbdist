#include "../include/tuiSwitch.h"
#include <stdlib.h>
#include <stdio.h>

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

void markCurPlayer(tui* tui, team** teams, fg_color color) {
  player* p = teams[tui->cur->team]->players[tui->cur->player];
  if (p->marker.active && p->marker.color != DEFAULT_COLOR) unmarkPlayer(p);
  else markPlayer(p, color);
}

void printTuiMan(FILE* out) {
  fprintf(out, "Cursor movement: w,a,s,d | Select: enter/space | Unselect: Esc | Mark: 1-5 | Exit: q\n\n");
}

void updateTUI(FILE* out, tui* tui, team** teams, pCombos* bpcs) {
  printf("\033[H");
  printTuiMan(out);
  int width = 15;

  for (int t = 0; t < tui->team_n; t += MAX_HOR_TEAMS) {
    for (int i = t; i < t + MAX_HOR_TEAMS && i < tui->team_n; i++) {
      fprintf(out, "\033[34m%-*.2f\033[0m", width, avgRating(teams[i]));
    }
    fprintf(out, "\n");
    for(int j = 0; j < tui->team_size; j++) {
      for(int i = t; i < tui->team_n && i - t < MAX_HOR_TEAMS ; i++) {
	player* p = teams[i]->players[j];
	if (highlight(tui, i, j)) {
          fprintf(out, "\033[%d;7m%-*s\033[0m", p->marker.color, width, p->firstName);
	} else if (comboInTeam(bpcs, teams[i], p)) {
          fprintf(out, "\033[%dm%-*s\033[0m", RED, width, p->firstName);
	} else {
          fprintf(out, "\033[%dm%-*s\033[0m", p->marker.color, width, p->firstName);
	}
      }
      fprintf(out, "\n");
    }
    fprintf(out, "\n");
  }
}


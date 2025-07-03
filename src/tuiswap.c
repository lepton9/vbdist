#include "../include/tuiswap.h"
#include <stdlib.h>
#include <ctype.h>

#define TEAM_PRINT_WIDTH 25

tuiswap* initTuiSwap(const int team_size, const int team_n) {
  tuiswap* tui = malloc(sizeof(tuiswap));
  tui->team_size = team_size;
  tui->team_n = team_n;
  tui->selected = malloc(sizeof(cursor));
  tui->cur = malloc(sizeof(cursor));
  unselect(tui->selected);
  tui->cur->team = 0;
  tui->cur->player = 0;
  tui->renderSkills = 0;
  tui->showHelp = 0;
  tui->teamsInline = 1;

  tui->teams = NULL;
  tui->skills = NULL;
  tui->bannedCombos = NULL;

  tui->render = init_renderer(stdout);

  tui->old_skills_a = NULL;
  tui->old_skills_b = NULL;
  tui->ind_team_a = -1;
  tui->ind_team_b = -1;
  tui->avg_a = 0.0;
  tui->avg_b = 0.0;
  return tui;
}

void freeTuiSwap(tuiswap* tui) {
  free_renderer(tui->render);
  free(tui->selected);
  free(tui->cur);
  if (tui->old_skills_a) freeSkills(tui->old_skills_a);
  if (tui->old_skills_b) freeSkills(tui->old_skills_b);
  free(tui);
}

char isActive(cursor* c) {
  return (c->team >= 0 || c->player >= 0);
}

void unselect(cursor* c) {
  c->team = -1;
  c->player = -1;
}

void resetTeamInds(tuiswap* tui) {
  tui->ind_team_a = -1;
  tui->ind_team_b = -1;
}

char samePos(cursor* a, cursor* b) {
  return (a->team == b->team && a->player == b->player);
}

void switchPos(tuiswap* tui) {
  player* a = tui->teams[tui->selected->team]->players[tui->selected->player];
  player* b = tui->teams[tui->cur->team]->players[tui->cur->player];
  swapPlayers(a, b);
  unselect(tui->selected);
}

void saveOldSkills(tuiswap* tui) {
  if (tui->selected->team < 0 || tui->cur->team < 0) return;
  if (tui->selected->team == tui->cur->team) {
    resetTeamInds(tui);
    return;
  }
  tui->ind_team_a = tui->selected->team;
  tui->ind_team_b = tui->cur->team;
  team_average_skills(tui->teams[tui->ind_team_a], tui->old_skills_a);
  team_average_skills(tui->teams[tui->ind_team_b], tui->old_skills_b);
  tui->avg_a = avgRating(tui->teams[tui->ind_team_a]);
  tui->avg_b = avgRating(tui->teams[tui->ind_team_b]);
}

// Returns 1 if need to switch, 0 if not
char selectCur(tuiswap* tui) {
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

void cur_up(tuiswap* t) {
  if (t->cur->player > 0) t->cur->player--;
  else if (t->cur->team >= t->teamsInline) {
    t->cur->team -= t->teamsInline;
    t->cur->player = t->team_size - 1;
  }
}

void cur_down(tuiswap* t) {
  if (t->cur->player < t->team_size - 1) t->cur->player++;
  else if (t->cur->team + t->teamsInline < t->team_n) {
    t->cur->team += t->teamsInline;
    t->cur->player = 0;
  }
}

void cur_left(tuiswap* t) {
  if (t->cur->team % t->teamsInline > 0) {
    t->cur->team--;
  }
}

void cur_right(tuiswap* t) {
  if (t->cur->team % t->teamsInline < t->teamsInline - 1 &&
      t->cur->team < t->team_n - 1) {
    t->cur->team++;
  }
}

char highlight(const tuiswap* tui, const int team, const int player) {
  return (tui->cur->team == team && tui->cur->player == player) || (tui->selected->team == team && tui->selected->player == player);
}

void markCurPlayer(tuiswap* tui, team** teams, color_fg color) {
  player* p = teams[tui->cur->team]->players[tui->cur->player];
  if (p->marker.active && p->marker.color != DEFAULT_FG) unmarkPlayer(p);
  else markPlayer(p, color);
}

int calcTeamsInline(const size_t screenWidth, const size_t printWidth) {
  if (printWidth == 0) return 1;
  size_t teamsInline = screenWidth / printWidth;
  return teamsInline > 0 ? teamsInline : 1;
}

void renderTuiSwapSkills(tuiswap* tui, int begLine) {
  const char indent = 0;
  int line = begLine;
  int col = 0;
  for (int t = 0; t < tui->team_n; t += tui->teamsInline) {
    col = 0;
    for (int i = t; i < t + tui->teamsInline && i < tui->team_n; i++) {
      if (i == tui->ind_team_a || i == tui->ind_team_b) {
        double rating_new = avgRating(tui->teams[i]);
        double avg_old = (i == tui->ind_team_a) ? tui->avg_a : tui->avg_b;
        int color = (rating_new > avg_old) ? GREEN_FG : RED_FG;
        put_text(tui->render, line, col,
                 "\033[4m%s |     %-.2f\033[24m-> \033[%dm%.2f\033[0m",
                 tui->teams[i]->name, avg_old, color, rating_new);
      } else {
        put_text(tui->render, line, col, "\033[4m%s |     %.2f\033[24m", tui->teams[i]->name, avgRating(tui->teams[i]));
      }
      col += TEAM_PRINT_WIDTH;
    }
    line++;
    for(size_t j = 0; j < tui->skills->n; j++) {
      col = 0;
      for(int i = t; i < tui->team_n && i - t < tui->teamsInline; i++) {
        skill* s = tui->skills->items[j];
        if (i == tui->ind_team_a || i == tui->ind_team_b) {
          double value_new = team_average_skill(tui->teams[i], s);
          double value_old =
              (i == tui->ind_team_a)
                  ? ((skill *)tui->old_skills_a->items[j])->value
                  : ((skill *)tui->old_skills_b->items[j])->value;
          int color = (value_new > value_old) ? GREEN_FG : RED_FG;
          put_text(tui->render, line, col,
                   "%s%-12s %.1f -> \033[%dm%.1f\033[0m", (indent) ? "  " : "",
                   s->name, value_old, color, value_new);
        } else {
          put_text(tui->render, line, col, "%s%-12s %.1f", (indent) ? "  " : "",
                   s->name, team_average_skill(tui->teams[i], s));
        }
        col += TEAM_PRINT_WIDTH;
      }
      line++;
    }
    line++;
  }
}

int renderTuiSwapTeams(tuiswap* tui) {
  int width = 15;
  int line = 0;
  for (int t = 0; t < tui->team_n; t += tui->teamsInline) {
    int col = 0;
    for (int i = t; i < t + tui->teamsInline && i < tui->team_n; i++) {
      line = 2 + (t / tui->teamsInline) * (tui->team_size + 2);
      put_text(tui->render, line++, col, "\033[34m%-*.2f\033[0m", width, avgRating(tui->teams[i]));
      for(int j = 0; j < tui->team_size; j++) {
        player *p = tui->teams[i]->players[j];
        if (highlight(tui, i, j)) {
          put_text(tui->render, line++, col, "\033[%d;7m%-*s\033[0m",
                      p->marker.color, width, p->firstName);
        } else if (comboInTeam(tui->bannedCombos, tui->teams[i], p)) {
          put_text(tui->render, line++, col, "\033[%dm%-*s\033[0m", RED_FG, width, p->firstName);
        } else {
          put_text(tui->render, line++, col, "\033[%dm%-*s\033[0m", p->marker.color, width, p->firstName);
        }
      }
      col += width + 2;
    }
  }
  return line;
}

void renderTuiSwap(tuiswap* tui) {
  if (tui->showHelp) {
    append_line(tui->render, 0, "Cursor movement: w,a,s,d | Select: enter/space | Unselect: Esc | Mark: 1-5 | Toggle skills: t");
  } else {
    append_line(tui->render, 0, "[Q]uit | [?]Help");
  }
  tui->teamsInline = calcTeamsInline(tui->render->width, TEAM_PRINT_WIDTH);
  int lastLine = renderTuiSwapTeams(tui);
  if (tui->renderSkills) renderTuiSwapSkills(tui, lastLine + 3);
  render(tui->render);
}

void handleTuiSwapInput(tuiswap* tui, int c) {
  switch (c) {
    case 13: case '\n': case ' ':
#ifdef __linux__
    case KEY_ENTER:
#endif
    if (selectCur(tui)) {
      saveOldSkills(tui);
      switchPos(tui);
      unselect(tui->selected);
    } else {
      resetTeamInds(tui);
    }
    break;
    case 27: // Esc
      unselect(tui->selected);
      resetTeamInds(tui);
      break;
    case 'T': case 't':
      tui->renderSkills ^= 1;
      break;
    case '?':
      tui->showHelp ^= 1;
      break;
    case 'K': case 'W':
    case 'k': case 'w':
#ifdef __linux__
    case KEY_UP:
#endif
    cur_up(tui);
    break;
    case 'J': case 'S':
    case 'j': case 's':
#ifdef __linux__
    case KEY_DOWN:
#endif
    cur_down(tui);
    break;
    case 'H': case 'A':
    case 'h': case 'a':
#ifdef __linux__
    case KEY_LEFT:
#endif
    cur_left(tui);
    break;
    case 'L': case 'D':
    case 'l': case 'd':
#ifdef __linux__
    case KEY_RIGHT:
#endif
    cur_right(tui);
    break;
    default: {
      if (isdigit(c)) {
        int d = c - '0';
        markCurPlayer(tui, tui->teams, getMarkColor(d));
      }
      break;
    }
  }
}

void runTuiSwap(team** teams, size_t teams_n, size_t team_size, dlist* skills, dlist* bpcs) {
  tuiswap* tui = initTuiSwap(team_size, teams_n);
  tui->teams = teams;
  tui->skills = skills;
  tui->bannedCombos = bpcs;
  tui->old_skills_a = copySkills(skills);
  tui->old_skills_b = copySkills(skills);

  int c = 0;
  refresh_screen(tui->render);
  while (c != 'q') {
    renderTuiSwap(tui);
    c = keyPress();
    handleTuiSwapInput(tui, c);
  }
  cls(stdout);
  freeTuiSwap(tui);
}


#include "../include/player.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

player* initPlayerClean() {
  player* p = malloc(sizeof(player));
  p->firstName = NULL;
  p->surName = NULL;
  p->skills = NULL;
  p->positions = NULL;
  p->found = 0;
  p->assigned_pos = -1;
  unmarkPlayer(p);
  return p;
}

player* initPlayer() {
  player* p = initPlayerClean();
  p->skills = init_list();
  p->positions = init_list();
  return p;
}

void freePlayer(player* p) {
  if (!p) return;
  if (p->firstName) free(p->firstName);
  if (p->surName) free(p->surName);
  freeSkills(p->skills);
  freePositions(p->positions);
  free(p);
}

player* copyPlayer(player* p) {
  player* copy = initPlayerClean();
  copy->id = p->id;
  if (p->firstName) updatePlayerName(copy, strdup(p->firstName));
  if (p->surName) copy->surName = strdup(p->surName);
  copy->assigned_pos = p->assigned_pos;
  copy->skills = copySkills(p->skills);
  copy->positions = copyPositions(p->positions);
  return copy;
}

player* parsePlayer(char* pStr) {
  player* p = initPlayer();
  char* token = strtok(pStr, "|");
  char* fullName = NULL;
  if (token != NULL) {
    while (isspace(*token)) token++;
    fullName = strdup(token);
    char* end = fullName + strlen(fullName) - 1;
    while (end > fullName && isspace(*end)) end--;
    *(end + 1) = '\0';
  }

  char* surnameStart = strrchr(fullName, ' ');
  if (surnameStart != NULL) {
    *surnameStart = '\0';
    surnameStart++;
    p->firstName = strdup(fullName);
    p->surName = strdup(surnameStart);
  } else {
    p->firstName = strdup(fullName);
    p->surName = NULL;
  }

  token = strtok(NULL, " ");
  while (token != NULL) {
    while (isspace(*token)) token++;
    skill* s = initSkill(0, "", strtof(token, NULL));
    list_add(p->skills, s);
    token = strtok(NULL, " ");
  }
  return p;
}

double rating(player* p) {
  if (!p) return 0.0;
  double sum = 0;
  int ratings_n = 0;
  for (size_t i = 0; i < p->skills->n; i++) {
    skill* s = p->skills->items[i];
    float r = s->value;
    if (fabsf(r) > 1e-6f) {
      sum += r;
      ratings_n++;
    }
  }
  return (ratings_n > 0) ? sum / ratings_n : 0.0;
}

double rating_filter(player* p, dlist* skills) {
  if (!p) return 0.0;
  double sum = 0;
  int ratings_n = 0;
  for (size_t i = 0; i < p->skills->n; i++) {
    skill* s = p->skills->items[i];
    int s_i = findSkill(s, skills);
    if (s_i >= 0) {
      float r = s->value * ((skill*)skills->items[s_i])->weight;
      if (fabsf(r) > 1e-6f) {
        sum += r;
        ratings_n++;
      }
    }
  }
  return (ratings_n > 0) ? sum / ratings_n : 0.0;
}

double get_skill_value(player* p, skill* s) {
  if (!p || !s) return 0.0;
  for (size_t i = 0; i < p->skills->n; i++) {
    skill* p_s = p->skills->items[i];
    if (p_s->id == s->id) return p_s->value * s->weight;
  }
  return 0.0;
}

int cmpPlayers(const void* a, const void* b) {
  player* ap = *(player**)a;
  player* bp = *(player**)b;
  double ret = rating(bp) - rating(ap);
  return (ret < 0) ? -1 : (ret > 0) ? 1 : 0;
}

int cmpPlayerPos(const void* a, const void* b) {
  player* ap = *(player**)a;
  player* bp = *(player**)b;
  position* pos_a = assignedPosition(ap);
  position* pos_b = assignedPosition(bp);
  if (pos_a && pos_b) {
    return (pos_a->id > pos_b->id) - (pos_a->id < pos_b->id);
  }
  return pos_a ? 1 : (pos_b ? -1 : 0);
}

int cmpPlayerName(const void* a, const void* b) {
  player* ap = *(player**)a;
  player* bp = *(player**)b;
  return strcmp(playerName(ap), playerName(bp));
}

void swapPlayers(player* a, player* b) {
  player tmp = *a;
  *a = *b;
  *b = tmp;
}

void swapPositions(player* p, size_t a, size_t b) {
  char swapped = swap_elems(p->positions, a, b);
  if (!swapped) return;
  position* pos_a = p->positions->items[a];
  position* pos_b = p->positions->items[b];
  int prio_a = pos_a->priority;
  setPriority(pos_a, pos_b->priority);
  setPriority(pos_b, prio_a);
}

void markPlayer(player* p, color_fg color) {
  p->marker.active = 1;
  p->marker.color = color;
}

void unmarkPlayer(player* p) {
  p->marker.active = 0;
  p->marker.color = DEFAULT_FG;
}

void printPlayer(FILE* out, player* p) {
  fprintf(out, "%-25s ", playerFullName(p));
  for (size_t i = 0; i < p->skills->n; i++) {
    fprintf(out, "%.1f ", ((skill*)p->skills->items[i])->value);
  }
  fprintf(out, "| %.1f\n", rating(p));
}

int playerInList(dlist* list, int player_id) {
  for (size_t i = 0; i < list->n; i++) {
    if (((player*)list->items[i])->id == player_id) return i;
  }
  return -1;
}

player* getPlayerInList(dlist* list, int player_id) {
  int i = playerInList(list, player_id);
  if (i >= 0) {
    return list->items[i];
  }
  return NULL;
}

position* firstPosition(player* p) {
  return (p->positions->n > 0) ? p->positions->items[0] : NULL;
}

int hasPosition(player* player, position* pos) {
  return findPosition(player->positions, pos);
}

position* assignedPosition(player* p) {
  if (p->assigned_pos < 0) {
    return NULL;
  }
  return get_elem(p->positions, p->assigned_pos);
}

int setPlayerPosition(player* p, position* pos) {
  int i = hasPosition(p, pos);
  if (i >= 0) {
    assignPosition(p, i);
    return 1;
  }
  return 0;
}

void assignPosition(player* p, int index) {
  if (index >= 0 && index < (int)p->positions->n) {
    p->assigned_pos = index;
  }
}

void resetPosition(player* p) {
  p->assigned_pos = -1;
}

void addPositionLast(player* p, position* pos) {
  position* pos_last = get_last(p->positions);
  setPriority(pos, (pos_last) ? pos_last->priority + 1 : 1);
  list_add(p->positions, pos);
}

position* popPosition(player* p, const int index) {
  position* pos = pop_elem(p->positions, index);
  for (size_t i = 0; i < p->positions->n; i++) {
    position* pp = p->positions->items[i];
    if (pp->priority >= pos->priority) {
      setPriority(pp, pp->priority - 1);
    }
  }
  return pos;
}

void updatePlayerName(player* p, char* name) {
  if (p->firstName) free(p->firstName);
  p->firstName = name;
}

const char* playerName(player* p) {
  return (p && p->firstName) ? p->firstName : "";
}

const char* playerFullName(player* p) {
  if (!p) return "";
  static char player_name[100];
  snprintf(player_name, sizeof(player_name), "%s %s", playerName(p),
           (p->surName) ? p->surName : "");
  return player_name;
}

color_fg getMarkColor(const int key) {
  switch (key) {
    case 1: return GREEN_FG;
    case 2: return YELLOW_FG;
    case 3: return BLUE_FG;
    case 4: return MAGENTA_FG;
    case 5: return CYAN_FG;
    default: return DEFAULT_FG;
  }
}


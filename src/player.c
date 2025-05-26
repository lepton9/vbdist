#include "../include/player.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

player* initPlayer() {
  player* p = malloc(sizeof(player));
  p->firstName = NULL;
  p->surName = NULL;
  p->found = 0;
  p->skills = init_list();
  p->positions = init_list();
  p->assigned_pos = -1;
  unmarkPlayer(p);
  return p;
}

void freePositions(dlist* positions) {
  if (!positions) return;
  for (size_t i = 0; i < positions->n; i++) {
    freePosition(positions->items[i]);
  }
  free_list(positions);
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
  player* copy = initPlayer();
  copy->id = p->id;
  if (p->firstName) copy->firstName = strdup(p->firstName);
  if (p->surName) copy->surName = strdup(p->surName);
  copy->assigned_pos = p->assigned_pos;

  copy->skills = init_list();
  for (size_t i = 0; i < p->skills->n; i++) {
    skill* s = p->skills->items[i];
    skill* s_copy = initSkill(s->id, s->name, s->value);
    list_add(copy->skills, s_copy);
  }
  for (size_t i = 0; i < p->positions->n; i++) {
    position* pos = p->positions->items[i];
    position* pos_copy = copy_position(pos);
    setPriority(pos_copy, pos->priority);
    list_add(copy->positions, pos_copy);
  }
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

void swapPlayers(player* a, player* b) {
  player tmp = *a;
  *a = *b;
  *b = tmp;
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
  char fullName[100] = "\0";
  strcat(fullName, p->firstName);
  if (p->surName) strcat(strcat(fullName, " "), p->surName);
  fprintf(out, "%-25s ", fullName);
  for (size_t i = 0; i < p->skills->n; i++) {
    fprintf(out, "%.1f ", ((skill*)p->skills->items[i])->value);
  }
  fprintf(out, "| %.1f\n", rating(p));
}

int playerInList(dlist* list, int player_id) {
  for (int i = 0; i < (int)list->n; i++) {
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
  for (size_t i = 0; i < player->positions->n; i++) {
    position* p = player->positions->items[i];
    if (p->id == pos->id) return i;
  }
  return -1;
}

position* assignedPosition(player* p) {
  if (p->assigned_pos < 0 || p->positions->n == 0 || p->assigned_pos >= (int)p->positions->n) {
    return NULL;
  }
  return p->positions->items[p->assigned_pos];
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


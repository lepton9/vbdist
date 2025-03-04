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
  unmarkPlayer(p);
  return p;
}

void freePlayer(player* p) {
  if (!p) return;
  if (p->firstName) free(p->firstName);
  if (p->surName) free(p->surName);
  for (size_t i = 0; i < p->skills->n; i++) {
    freeSkill(p->skills->items[i]);
  }
  free_list(p->skills);
  free(p);
}

player* copyPlayer(player* p) {
  player* copy = initPlayer();
  *copy = *p;
  copy->firstName = strdup(p->firstName);
  if (p->surName) copy->surName = strdup(p->surName);
  copy->skills = init_list();
  for (size_t i = 0; i < p->skills->n; i++) {
    skill* s = p->skills->items[i];
    skill* s_copy = initSkill(s->id, s->name, s->value);
    list_add(copy->skills, s_copy);
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

double rating_filter(player* p, dlist* skill_ids) {
  if (!p) return 0.0;
  double sum = 0;
  int ratings_n = 0;
  for (size_t i = 0; i < p->skills->n; i++) {
    skill* s = p->skills->items[i];
    if (is_selected_skill(s, skill_ids) >= 0) {
      float r = s->value;
      if (fabsf(r) > 1e-6f) {
        sum += r;
        ratings_n++;
      }
    }
  }
  return (ratings_n > 0) ? sum / ratings_n : 0.0;
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

void markPlayer(player* p, fg_color color) {
  p->marker.active = 1;
  p->marker.color = color;
}

void unmarkPlayer(player* p) {
  p->marker.active = 0;
  p->marker.color = DEFAULT_COLOR;
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


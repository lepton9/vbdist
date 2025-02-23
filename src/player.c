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
  unmarkPlayer(p);
  return p;
}

void freePlayer(player* p) {
  if (!p) return;
  if (p->firstName) free(p->firstName);
  if (p->surName) free(p->surName);
  free(p);
}

player* copyPlayer(player* p) {
  player* copy = initPlayer();
  *copy = *p;
  copy->firstName = strdup(p->firstName);
  if (p->surName) copy->surName = strdup(p->surName);
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
  int i = 0;
  while (token != NULL && i < DIFRATINGS) {
      while (isspace(*token)) token++;
      p->ratings[i++] = strtof(token, NULL);
      token = strtok(NULL, " ");
  }
  return p;
}

double ovRating(player* p) {
  if (!p) return 0.0;
  double sum = 0;
  int ratings_n = 0;
  for (int i = 0; i < DIFRATINGS; i++) {
    float r = p->ratings[i];
    if (fabsf(r) > 1e-6f) {
      sum += r;
      ratings_n++;
    }
  }
  return (ratings_n > 0) ? sum / ratings_n : 0.0;
}

int cmpPlayers(const void* a, const void* b) {
  player* ap = *(player**)a;
  player* bp = *(player**)b;
  double ret = ovRating(bp) - ovRating(ap);
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
  for (int i = 0; i < DIFRATINGS; i++) {
    fprintf(out, "%.1f ", p->ratings[i]);
  }
  fprintf(out, "| %.1f\n", ovRating(p));
}


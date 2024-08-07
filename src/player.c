#include "../include/player.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

player* initPlayer() {
  player* p = malloc(sizeof(player));
  return p;
}

player* parsePlayer(char* pStr) {
  player* p = initPlayer();
  char* token = strtok(pStr, "|");
  char* fullName;
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
      p->ratings[i++] = atoi(token);
      token = strtok(NULL, " ");
  }
  return p;
}

double ovRating(player* p) {
  if (!p) return 0.0;
  int sum = 0;
  for (int i = 0; i < DIFRATINGS; i++) {
    sum += p->ratings[i];
  }
  return (double)sum / DIFRATINGS;
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

void printPlayer(FILE* out, player* p) {
  char fullName[100] = "\0";
  strcat(fullName, p->firstName);
  if (p->surName) strcat(strcat(fullName, " "), p->surName);
  fprintf(out, "%-20s ", fullName);
  for (int i = 0; i < DIFRATINGS; i++) {
    fprintf(out, "%d ", p->ratings[i]);
  }
  fprintf(out, "| %.1f\n", ovRating(p));
}

void freePlayer(player* p) {
  if (!p) return;
  free(p->firstName);
  free(p->surName);
  free(p);
}


#include "../include/player.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>


player* initPlayer() {
  player* p = malloc(sizeof(player));
  //p->ratings = malloc(sizeof(int) * DIFRATINGS);
  //p->name = strdup(pName);
  //p->r = malloc(sizeof(rating));
  return p;
}

player* parsePlayer(char* pStr) {
  player* p = initPlayer();
  char* del = " ";
  int i = 0;

  char* token = strtok(pStr, del);
  p->name = strdup(token);

  while (token != NULL && i < DIFRATINGS) {
    token = strtok(NULL, del);
    if (!token) break;
    p->ratings[i] = atoi(token);
    i++;
  }
  assert(i == DIFRATINGS);

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
  return ovRating(bp) - ovRating(ap);
}

void swapPlayers(player** a, player** b) {
  player* tmp = *a;
  *a = *b;
  *b = tmp;
}

void freePlayer(player* p) {
  free(p->name);
  //free(p->r);
  free(p);
}


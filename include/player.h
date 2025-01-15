#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include "../include/mark.h"

#define DIFRATINGS 6

typedef struct {
  char* firstName;
  char* surName;
  int id;
  float ratings[DIFRATINGS];
  int ratings_id;
  int found;
  mark marker;
} player;

// TODO: make general list struct
typedef struct {
  player** players;
  size_t n;
  size_t size;
} playerList;

playerList* mallocPList(size_t size);

player* initPlayer();
void freePlayer(player* p);
player* parsePlayer(char* pStr);
int cmpPlayers(const void* a, const void* b);
void swapPlayers(player* a, player* b);
double ovRating(player* p);
void markPlayer(player* p, fg_color color);
void unmarkPlayer(player* p);
void printPlayer(FILE* out, player* p);

#endif

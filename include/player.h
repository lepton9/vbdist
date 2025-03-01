#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include "../include/mark.h"
#include "../include/dlist.h"
#include "../include/skill.h"

#define DIFRATINGS 6

typedef struct {
  char* firstName;
  char* surName;
  int id;

  // TODO: Deprecated
  float ratings[DIFRATINGS];
  int ratings_id;

  dlist* skills;
  int found;
  mark marker;
} player;

player* initPlayer();
void freePlayer(player* p);
player* copyPlayer(player* p);
player* parsePlayer(char* pStr);
int cmpPlayers(const void* a, const void* b);
void swapPlayers(player* a, player* b);
double ovRating(player* p);
void markPlayer(player* p, fg_color color);
void unmarkPlayer(player* p);
void printPlayer(FILE* out, player* p);

#endif

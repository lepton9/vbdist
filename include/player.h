#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include "../include/dlist.h"
#include "../include/skill.h"
#include "../include/ansicodes.h"

typedef struct {
  char active;
  color_fg color;
} mark;

typedef struct {
  char* firstName;
  char* surName;
  int id;
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
double rating(player* p);
double rating_filter(player* p, dlist* skill_ids);
void markPlayer(player* p, color_fg color);
void unmarkPlayer(player* p);
void printPlayer(FILE* out, player* p);

int playerInList(dlist* list, int player_id);
player* getPlayerInList(dlist* list, int player_id);

color_fg getMarkColor(const int key);

#endif

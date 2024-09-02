#ifndef TEAM_H
#define TEAM_H

#include "../include/player.h"


typedef struct {
  char* name;
  size_t size;
  player** players;
} team;

team* initTeam(char* teamName, const size_t size);
void freeTeam(team* t, const int team_size);
double avgRating(team* t);

#endif

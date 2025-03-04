#ifndef TEAM_H
#define TEAM_H

#include "../include/player.h"


typedef struct {
  char* name;
  size_t size;
  player** players;
  int id;
} team;

team* initTeam(char* teamName, const size_t size);
void freeTeam(team* t);
double avgRating(team* t);
double team_rating_filter(team* t, dlist* skill_ids);

#endif

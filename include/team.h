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
double team_rating_filter(team* t, dlist* skills);
double team_average_skill(team* t, skill* skill);
void team_average_skills(team* t, dlist* skills);

int teamInList(dlist* list, int team_id);
int checkPlayerCollisions(team* a, team* b);

#endif

#include "../include/team.h"
#include <stdlib.h>
#include <string.h>

team* initTeam(char* teamName) {
  team* t = malloc(sizeof(team));
  t->name = strdup(teamName);
  return t;
}

void freeTeam(team* t) {
  for (int i = 0; i < 3; i++) freePlayer(t->players[i]);
  free(t->name);
  free(t);
}

double avgRating(team* t) {
  if (t == NULL) return 0.0;
  double sumRatingT = 0;
  for (int pI = 0; pI < TEAM_SIZE; pI++) {
    sumRatingT += ovRating(t->players[pI]);
  }
  return sumRatingT / TEAM_SIZE;
}


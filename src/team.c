#include "../include/team.h"
#include <stdlib.h>
#include <string.h>

team* initTeam(char* teamName, const size_t size) {
  team* t = malloc(sizeof(team));
  t->name = strdup(teamName);
  t->size = size;
  t->players = malloc(size * sizeof(player*));
  return t;
}

void freeTeam(team* t, const int team_size) {
  for (int i = 0; i < team_size; i++) freePlayer(t->players[i]);
  free(t->players);
  free(t->name);
  free(t);
}

double avgRating(team* t) {
  if (t == NULL) return 0.0;
  double sumRatingT = 0;
  for (int pI = 0; pI < t->size; pI++) {
    sumRatingT += ovRating(t->players[pI]);
  }
  return sumRatingT / t->size;
}


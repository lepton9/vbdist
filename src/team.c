#include "../include/team.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

team* initTeam(char* teamName, const size_t size) {
  team* t = malloc(sizeof(team));
  t->name = strdup(teamName);
  t->size = size;
  t->players = malloc(size * sizeof(player*));
  t->id = -1;
  return t;
}

void freeTeam(team* t) {
  if (!t) return;
  free(t->players);
  free(t->name);
  free(t);
}

double avgRating(team* t) {
  if (t == NULL) return 0.0;
  double sum = 0;
  int ratings_n = 0;
  for (int pI = 0; pI < (int)t->size; pI++) {
    double r = ovRating(t->players[pI]);
    if (fabs(r) > 1e-6f) {
      sum += r;
      ratings_n++;
    }
  }
  return (ratings_n > 0) ? sum / ratings_n : 0.0;
}


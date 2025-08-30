#include "../include/team.h"
#include <stdlib.h>
#include <string.h>

team* initTeam(char* teamName, const size_t size) {
  team* t = malloc(sizeof(team));
  t->name = strdup(teamName ? teamName : "");
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
  if (!t) return 0.0;
  double sum = 0;
  int ratings_n = 0;
  for (size_t pI = 0; pI < t->size; pI++) {
    double r = rating(t->players[pI]);
    sum += r;
    ratings_n++;
  }
  return (ratings_n > 0) ? sum / ratings_n : 0.0;
}

double team_rating_filter(team* t, dlist* skills) {
  if (!t || !skills) return 0.0;
  double sum = 0;
  int ratings_n = 0;
  for (size_t pI = 0; pI < t->size; pI++) {
    double r = rating_filter(t->players[pI], skills);
    sum += r;
    ratings_n++;
  }
  return (ratings_n > 0) ? sum / ratings_n : 0.0;
}

double team_average_skill(team* t, skill* s) {
  double sum = 0;
  int n = 0;
  for (size_t p_i = 0; p_i < t->size; p_i++) {
    player* p = t->players[p_i];
    skill* p_s = getSkill(p, s);
    if (!p_s) continue;
    double val = p_s->value * s->weight;
    sum += val;
    n++;
  }
  return (n > 0) ? sum / n : sum;
}

void team_average_skills(team* t, dlist* skills) {
  if (!t || !skills) return;
  for (size_t s_i = 0; s_i < skills->n; s_i++) {
    skill* s = skills->items[s_i];
    s->value = team_average_skill(t, s);
  }
}

int teamInList(dlist* list, int team_id) {
  for (size_t i = 0; i < list->n; i++) {
    if (((team*)list->items[i])->id == team_id) return i;
  }
  return -1;
}

int checkPlayerCollisions(team* a, team* b) {
  if (!a || !b) return 0;
  for (size_t i = 0; i < a->size; i++) {
    for (size_t j = 0; j < b->size; j++) {
      if (a->players[i]->id == b->players[j]->id) return 1;
    }
  }
  return 0;
}


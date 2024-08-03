#include "../include/player.h"

#define TEAM_SIZE 3

typedef struct {
  char* name;
  player* players[TEAM_SIZE];
} team;

team* initTeam();
void freeTeam(team* t);
double avgRating(team* t);

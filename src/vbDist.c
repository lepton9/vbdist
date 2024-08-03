#include "../include/team.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define TEAMS_N 6

player** readPlayers(const char *fileName, int *pn) {
  FILE *fp = fopen(fileName, "rb");
  player** ps = NULL;

  if (fp == NULL) return NULL;

  char buffer[256];
  while (fgets(buffer, 256, fp))
  {
    buffer[strcspn(buffer, "\n")] = 0;
    if (buffer[0] == '#' || strcmp(buffer, "") == 0) continue;
    ps = realloc(ps, (*pn + 1) * sizeof(player*));
    ps[(*pn)++] = parsePlayer(buffer);
  }
  fclose(fp);
  return ps;
}

void printPlayers(player** players, const int n) {
  for (int i = 0; i < n; i++) {
    player* p = players[i];
    printf("%s %d %d %d %d %d | OV: %.1f\n", p->name, p->ratings[0], p->ratings[1], p->ratings[2], p->ratings[3], p->ratings[4], ovRating(p));
  }
}

int randintRange(const int min, const int max) {
  return rand() % (max + 1 - min) + min;
}

double averageRating(team** teams) {
  if (teams == NULL) return 0.0;
  double sumRating = 0.0;
  for (int t = 0; t < TEAMS_N; t++) {
    printf("Team: %d\n", t);
    sumRating += avgRating(teams[t]);
  }
  return sumRating / TEAMS_N;
}

int validateSwap(double a, double b, double aNew, double bNew, double avg, int oneSideValidation) {
  int valid = 0;
  if (fabs(aNew - avg) < fabs(a - avg)) valid++;
  if (fabs(bNew - avg) < fabs(b - avg)) valid++;

  printf("%d %d %d %d %d\n", a, b, aNew, bNew, avg);
  return (valid == 2) ? 1 : (valid == 1 && oneSideValidation) ? 1 : 0;
  //return (valid == 0) ? 0 : (valid == 1 && oneSideValidation) ? 1 : (valid == 2) ? 1 : 0;
}

int balancedClustering(team** teamsAll, int oneSideValidation) {
  team* teams = *teamsAll;
  double avgR = averageRating(teamsAll);
  int swaps = 0;
  int max_failures = 1000;
  int failures = 0;

  while(failures < max_failures) {
    // Get teams and 2 players
    int teamA = randintRange(0, TEAMS_N - 1);
    int teamB = randintRange(0, TEAMS_N - 1);
    while(teamB == teamA) teamB = randintRange(0, TEAMS_N - 1);
    player* pA = teams[teamA].players[randintRange(0, TEAM_SIZE - 1)];
    player* pB = teams[teamB].players[randintRange(0, TEAM_SIZE - 1)];

    // Get teams avg rating
    double ratingTeamA = avgRating(teamsAll[teamA]);
    double ratingTeamB = avgRating(teamsAll[teamB]);

    // Swap
    swapPlayers(&pA, &pB);

    // Validate by getting avg rating
    double ratingTeamA_new = avgRating(teamsAll[teamA]);
    double ratingTeamB_new = avgRating(teamsAll[teamB]);

    //printf("%d %d %d %d %d\n", ratingTeamA, ratingTeamB, ratingTeamA_new, ratingTeamB_new, avgR);

    int valid = validateSwap(ratingTeamA, ratingTeamB, ratingTeamA_new, ratingTeamB_new, avgR, oneSideValidation);

    // If worse, reverse swap and add failure
    // If better, keep swap and reset failures = 0
    if (valid) {
      swaps++;
      failures = 0;
    } else {
      failures++;
      swapPlayers(&pA, &pB);
    }
  }
  return swaps;
}

team* balanceTeamsRand(player** players, const int n) {
  team* teams = malloc(TEAMS_N * (sizeof(team)));
  qsort(players, n, sizeof(player*), cmpPlayers);
  srand(time(0));

  int group = 0;
  int inTeam[TEAMS_N * TEAM_SIZE];
  for (int i = 0; i < TEAMS_N * TEAM_SIZE; i++) inTeam[i] = 0;

  int teamI = 0;
  int teamCounters[TEAMS_N];
  for (int i = 0; i < TEAMS_N; i++) teamCounters[i] = 0;

  for (int i = 0; i < n; i++) {
    int ind = randintRange(group * 6, (group + 1) * 6 - 1);
    while(inTeam[ind]) ind = randintRange(group * 6, (group + 1) * 6 - 1);
    inTeam[ind] = 1;

    teams[teamI].players[teamCounters[teamI]++] = players[ind];
    teamI = (teamI + 1) % TEAMS_N;
    if (teamI == 0) group++;
  }
  return teams;
}

team* balanceTeams(player** players, const int n) {
  team* teams = malloc(TEAMS_N * (sizeof(team)));
  qsort(players, n, sizeof(player*), cmpPlayers);

  int teamI = 0;
  int teamCounters[TEAMS_N];
  for (int i = 0; i < TEAMS_N; i++) teamCounters[i] = 0;

  for (int i = 0; i < n; i++) {
    teams[teamI].players[teamCounters[teamI]++] = players[i];
    teamI = (teamI + 1) % TEAMS_N;
  }
  return teams;
}


int main() {
  int* pn = malloc(sizeof(int));
  *pn = 0;
  player** players = readPlayers("players.txt", pn);

  if (players == NULL) {
    printf("Players NULL\n");
    exit(1);
  }
  printPlayers(players, *pn);

  assert(*pn == TEAMS_N * TEAM_SIZE);

  team* teams = balanceTeamsRand(players, *pn);

  int swaps = balancedClustering(&teams, 1);
  printf("\nTotal swaps: %d\n", swaps);

  printf("\n");
  for (int i = 0; i < TEAMS_N; i++) {
    printf("Team %d | Rating: %.1f:\n", i + 1, avgRating(&teams[i]));
    for (int j = 0; j < TEAM_SIZE; j++) {
      printf("  %-10s Rating: %.1f\n", teams[i].players[j]->name, ovRating(teams[i].players[j]));
    }
  }

  for (int i = 0; i < TEAMS_N; i++) {
    for (int j = 0; j < TEAM_SIZE; j++) {
      freePlayer(teams[i].players[j]);
    }
    //freeTeam(&teams[i]);
  }
  free(teams);

  return 0;
}



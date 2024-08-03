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
    printf("%-10s %d %d %d %d %d | %.1f\n", p->name, p->ratings[0], p->ratings[1], p->ratings[2], p->ratings[3], p->ratings[4], ovRating(p));
  }
}

int randintRange(const int min, const int max) {
  return rand() % (max + 1 - min) + min;
}

double averageRating(team** teams) {
  if (teams == NULL) return 0.0;
  double sumRating = 0.0;
  for (int t = 0; t < TEAMS_N; t++) {
    sumRating += avgRating(teams[t]);
  }
  return sumRating / TEAMS_N;
}

int validateSwap(double a, double b, double aNew, double bNew, double avg, int oneSideValidation) {
  int valid = 0;
  if (fabs(aNew - avg) < fabs(a - avg)) valid++;
  if (fabs(bNew - avg) < fabs(b - avg)) valid++;

  return (valid == 2) ? 1 : (valid == 1 && oneSideValidation) ? 1 : 0;
}

int balancedClustering(team*** teamsAll, int oneSideValidation) {
  team** teams = *teamsAll;
  double avgR = averageRating(teams);
  int swaps = 0;
  int max_failures = 200;
  int failures = 0;

  while(failures < max_failures) {
    int teamA = randintRange(0, TEAMS_N - 1);
    int teamB = randintRange(0, TEAMS_N - 1);
    while(teamB == teamA) teamB = randintRange(0, TEAMS_N - 1);
    player* pA = teams[teamA]->players[randintRange(0, TEAM_SIZE - 1)];
    player* pB = teams[teamB]->players[randintRange(0, TEAM_SIZE - 1)];

    double ratingTeamA = avgRating(teams[teamA]);
    double ratingTeamB = avgRating(teams[teamB]);

    swapPlayers(pA, pB);

    double ratingTeamA_new = avgRating(teams[teamA]);
    double ratingTeamB_new = avgRating(teams[teamB]);

    int valid = validateSwap(ratingTeamA, ratingTeamB, ratingTeamA_new, ratingTeamB_new, avgR, oneSideValidation);

    if (valid) {
      swaps++;
      failures = 0;
    } else {
      failures++;
      swapPlayers(pA, pB);
    }
  }
  return swaps;
}

team** balanceTeamsRand(player** players, const int n) {
  team** teams = malloc(sizeof(team*) * TEAMS_N);
  for (int i = 0; i < TEAMS_N; i++) {
    char tName[7];
    sprintf(tName, "Team %d", i);
    teams[i] = initTeam(tName);
  }

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

    teams[teamI]->players[teamCounters[teamI]++] = players[ind];
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

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage -> \n  vbdist 'fileName' (k-means [0/1])\n");
    printf("\nPlayers file should be format:\n");
    printf("PlayerName Defence Spike Serve Setting Saving\n");
    exit(1);
  }

  char* fileName = argv[1];
  int clustering = 0;
  if (argc >= 3) {
    clustering = atoi(argv[2]);
  }

  int* pn = malloc(sizeof(int));
  *pn = 0;
  player** players = readPlayers(fileName, pn);

  if (!players) {
    printf("File %s not found\n", fileName);
    free(pn);
    exit(1);
  }

  qsort(players, *pn, sizeof(player*), cmpPlayers);
  printPlayers(players, *pn);

  assert(*pn == TEAMS_N * TEAM_SIZE);

  team** teams = balanceTeamsRand(players, *pn);

  if (clustering) {
    printf("\nBalancing teams..\n");
    int swaps = balancedClustering(&teams, 1);
    printf("Total swaps: %d\n", swaps);
  }

  printf("\n");
  for (int i = 0; i < TEAMS_N; i++) {
    printf("Team %d | Rating: %.2f:\n", i + 1, avgRating(teams[i]));
    for (int j = 0; j < TEAM_SIZE; j++) {
      printf("  %-10s Rating: %.1f\n", teams[i]->players[j]->name, ovRating(teams[i]->players[j]));
    }
  }

  for (int i = 0; i < TEAMS_N; i++) {
    freeTeam(teams[i]);
  }
  free(teams);
  free(pn);

  return 0;
}


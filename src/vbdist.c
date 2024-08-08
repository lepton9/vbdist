#include "../include/team.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define TEAMS_N 7
#define MAX_FAILURES 200

typedef struct {
  char* pA;
  char* pB;
} bannedPCombo;

bannedPCombo* initBPC(char* a, char* b) {
  bannedPCombo* bpc = malloc(sizeof(bannedPCombo));
  return bpc;
}

void freeBPC(bannedPCombo* bpc) {
  free(bpc);
}

player** readPlayers(const char *fileName, int *pn) {
  FILE *fp = fopen(fileName, "rb");
  player** ps = NULL;
  if (fp == NULL) return NULL;

  char line[256];
  while (fgets(line, sizeof(line), fp))
  {
    line[strcspn(line, "\n")] = 0;
    // if (line[0] == '!') {
    //   bannedPCombo* bpc = initBPC();
    // }
    if (line[0] == '#' || strcmp(line, "") == 0) continue;
    ps = realloc(ps, (*pn + 1) * sizeof(player*));
    ps[(*pn)++] = parsePlayer(line);
  }

  fclose(fp);
  return ps;
}

void printPlayers(player** players, const int n) {
  for (int i = 0; i < n; i++) {
    printPlayer(stdout, players[i]);
  }
}

void printTeams(team** teams) {
  for (int i = 0; i < TEAMS_N; i++) {
    printf("%s | Rating: %.2f:\n", teams[i]->name, avgRating(teams[i]));
    for (int j = 0; j < TEAM_SIZE; j++) {
      printf("  %-10s (%.1f)\n", teams[i]->players[j]->firstName, ovRating(teams[i]->players[j]));
    }
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
  int failures = 0;

  while(failures < MAX_FAILURES) {
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

  for (int i = 0; i < TEAMS_N; i++) {
    qsort(teams[i]->players, TEAM_SIZE, sizeof(player*), cmpPlayers);
  }

  return swaps;
}

team** balanceTeamsRand(player** players, const int n) {
  team** teams = malloc(sizeof(team*) * TEAMS_N);
  for (int i = 0; i < TEAMS_N; i++) {
    char tName[7];
    sprintf(tName, "Team %d", i + 1);
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
    int ind = randintRange(group * TEAMS_N, (group + 1) * TEAMS_N - 1);
    while(inTeam[ind]) ind = randintRange(group * TEAMS_N, (group + 1) * TEAMS_N - 1);
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

void writeTeamsToFile(team** teams, const char* teamsFile) {
  FILE* fp = fopen(teamsFile, "w");
  for(int i = 0; i < TEAMS_N; i++) {
    fprintf(fp, "%s:\n", teams[i]->name);
    for(int j = 0; j < TEAM_SIZE; j++) {
      fprintf(fp, "%s\n", teams[i]->players[j]->firstName);
    }
    fprintf(fp, "\n");
  }

  for (int i = 0; i < TEAMS_N; i++) {
    fprintf(fp, "%-10s", teams[i]->name);
  }
  fprintf(fp, "\n");
  for(int j = 0; j < TEAM_SIZE; j++) {
    for(int i = 0; i < TEAMS_N; i++) {
      fprintf(fp, "%-10s", teams[i]->players[j]->firstName);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

int main(int argc, char** argv) {
  if (argc < 4) {
    printf("Usage -> \n  vbdist playersFile TEAMS_N TEAM_SIZE\n");
    printf("\nPlayers file should be format:\n");
    printf("PlayerName | Defence Spike Serve Setting Saving Consistency\n");
    exit(1);
  }

  char* fileName = argv[1];
  int teams_n = atoi(argv[2]);
  int team_size = atoi(argv[3]);
  int clustering = 1;

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
  assert(*pn == teams_n * team_size);

  team** teams = balanceTeamsRand(players, *pn);

  if (clustering) {
    printf("\nBalancing teams..\n");
    int swaps = balancedClustering(&teams, 1);
    printf("Total swaps: %d\n", swaps);
  }

  printf("\n");
  printTeams(teams);

  writeTeamsToFile(teams, "teams.txt");

  for (int i = 0; i < TEAMS_N; i++) {
    freeTeam(teams[i]);
  }
  free(teams);
  free(pn);

  return 0;
}


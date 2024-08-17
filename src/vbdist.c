#include "../include/team.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define MAX_FAILURES 500

int TEAMS_N = 0;
int TEAM_SIZE = 0;

typedef struct {
  int pidA;
  int pidB;
} bannedPCombo;

typedef struct {
  bannedPCombo* combos;
  size_t n;
} bannedCombos;

bannedCombos* initBPCs() {
  bannedCombos* bpcs = malloc(sizeof(bannedCombos));
  bpcs->combos = malloc(sizeof(bannedPCombo));
  bpcs->n = 0;
  return bpcs;
}

void addBPC(bannedCombos* bpcs, int a, int b) {
  bpcs->combos = realloc(bpcs->combos, (bpcs->n + 1) * sizeof(bannedPCombo));
  bpcs->combos[bpcs->n].pidA = a;
  bpcs->combos[bpcs->n].pidB = b;
  bpcs->n++;
}

void freeBPCs(bannedCombos* bpcs) {
  free(bpcs->combos);
  free(bpcs);
}

char* trimWS(char* str) {
  while(isspace(*str)) str++;
  if (*str) {
    char* end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
  }
  return str;
}

void parseBPC(char* line, char** nA, char** nB) {
  if (line[0] == '!') {
   line++;
  }
  char* token = strtok(line, "-");
  if (token != NULL) *nA = strdup(trimWS(token));
  token = strtok(NULL, "-");
  if (token != NULL) *nB = strdup(trimWS(token));

  assert(*nA != NULL && *nB != NULL);
}

player** readPlayers(const char *fileName, int *pn, bannedCombos* bpcs) {
  FILE *fp = fopen(fileName, "rb");
  player** ps = NULL;
  int pid = 0;
  if (fp == NULL) return NULL;

  char line[256];
  while (fgets(line, sizeof(line), fp))
  {
    line[strcspn(line, "\n")] = 0;
    if (line[0] == '#' || strcmp(trimWS(line), "") == 0) continue;
    if (line[0] == '!') {
      char* ap;
      char* bp;
      int a = -1;
      int b = -1;
      parseBPC(line, &ap, &bp);
      for (int i = 0; i < *pn; i++) {
        if (strcmp(ps[i]->firstName, ap) == 0 && a < 0) a = ps[i]->id;
        if (strcmp(ps[i]->firstName, bp) == 0 && b < 0) b = ps[i]->id;
      }
      // printf("%s-%s\n", a->firstName, b->firstName);
      if (a >= 0 && b >= 0) addBPC(bpcs, a, b);
      continue;
    }
    ps = realloc(ps, (*pn + 1) * sizeof(player*));
    ps[*pn] = parsePlayer(line);
    ps[(*pn)++]->id = pid++;
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

char bannedCombo(bannedCombos* bpcs, player* a, player* b) {
  for (int i = 0; i < bpcs->n; i++) {
    if ((bpcs->combos[i].pidA == a->id && bpcs->combos[i].pidB == b->id) ||
      (bpcs->combos[i].pidA == b->id && bpcs->combos[i].pidB == a->id)) {
      return 1;
    }
  }
  return 0;
}

int validateSwap(double a, double b, double aNew, double bNew, double avg, int oneSideValidation) {
  int valid = 0;
  if (fabs(aNew - avg) < fabs(a - avg)) valid++;
  if (fabs(bNew - avg) < fabs(b - avg)) valid++;

  return (valid == 2) ? 1 : (valid == 1 && oneSideValidation) ? 1 : 0;
}

int balancedClustering(team** teams, int oneSideValidation, bannedCombos* bpcs) {
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
      char banned = 0;
      for (int pI = 0; pI < TEAM_SIZE; pI++) {
        if (bannedCombo(bpcs, pA, teams[teamA]->players[pI]) || bannedCombo(bpcs, pB, teams[teamB]->players[pI])) {
          banned = 1;
          break;
        }
      }
      if (banned) {
        failures++;
        swapPlayers(pA, pB);
      } else {
        swaps++;
        failures = 0;
      }
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
    teams[i] = initTeam(tName, TEAM_SIZE);
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

void printTeamsVert(FILE* fp, team** teams) {
  for(int i = 0; i < TEAMS_N; i++) {
    fprintf(fp, "%s:\n", teams[i]->name);
    for(int j = 0; j < TEAM_SIZE; j++) {
      fprintf(fp, "%s\n", teams[i]->players[j]->firstName);
    }
    fprintf(fp, "\n");
  }
}

void printTeamsHor(FILE* fp, team** teams) {
  for (int i = 0; i < TEAMS_N; i++) {
    fprintf(fp, "%-15s", teams[i]->name);
  }
  fprintf(fp, "\n");
  for(int j = 0; j < TEAM_SIZE; j++) {
    for(int i = 0; i < TEAMS_N; i++) {
      fprintf(fp, "%-15s", teams[i]->players[j]->firstName);
    }
    fprintf(fp, "\n");
  }
}

void writeTeamsToFile(team** teams, const char* teamsFile) {
  FILE* fp = fopen(teamsFile, "w");
  printTeamsVert(fp, teams);
  printTeamsHor(fp, teams);
  fclose(fp);
}

void changeMode(team** teams) {
    printTeamsHor(stdout, teams);
  // while(1) {
  // }

}

int main(int argc, char** argv) {
  if (argc < 4) {
    printf("Usage -> \n  vbdist playersFile TEAMS_N TEAM_SIZE\n");
    printf("\nPlayers file should be format:\n");
    printf("PlayerName | Defence Spike Serve Setting Saving Consistency\n");
    exit(1);
  }

  char* fileName = argv[1];
  TEAMS_N = atoi(argv[2]);
  TEAM_SIZE = atoi(argv[3]);
  int clustering = 1;
  char print = 1;
  if (argc >= 5) print = atoi(argv[4]);

  int* pn = malloc(sizeof(int));
  *pn = 0;
  bannedCombos* bpcs = initBPCs();
  player** players = readPlayers(fileName, pn, bpcs);

  if (!players) {
    printf("File %s not found\n", fileName);
    free(pn);
    exit(1);
  }

  qsort(players, *pn, sizeof(player*), cmpPlayers);

  if (print) printPlayers(players, *pn);
  printf("Banned combinations: %d\n", (int)bpcs->n);

  if (*pn != TEAMS_N * TEAM_SIZE) {
    printf("\nFile %s contains %d players, but %d was expected\n", fileName, *pn, TEAMS_N * TEAM_SIZE);
    exit(1);
  }

  team** teams = balanceTeamsRand(players, *pn);

  if (clustering) {
    printf("\nBalancing teams..\n");
    int swaps = balancedClustering(teams, 1, bpcs);
    printf("Total swaps: %d\n", swaps);
  }

  printf("\n");
  if (print) printTeams(teams);
  //
  // char ans;
  // printf("\nManually change players? [y/N] ");
  // scanf("%c", &ans);
  //
  // if (ans == 'y' || ans == 'Y') {
  //   changeMode(teams);
  // }

  writeTeamsToFile(teams, "teams.txt");

  for (int i = 0; i < TEAMS_N; i++) {
    freeTeam(teams[i]);
  }
  free(teams);
  free(pn);
  freeBPCs(bpcs);

  return 0;
}


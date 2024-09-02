#include "../include/team.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define MAX_FAILURES 300
#define MAX_SWAPS 1000000

int TEAMS_N = 0;
int TEAM_SIZE = 0;

typedef struct {
  int pidA;
  int pidB;
} pCombo;

typedef struct {
  pCombo* combos;
  size_t n;
} pCombos;

pCombos* initCombos() {
  pCombos* combos = malloc(sizeof(pCombos));
  combos->combos = malloc(sizeof(pCombo));
  combos->n = 0;
  return combos;
}

void addCombo(pCombos* combos, int a, int b) {
  combos->combos = realloc(combos->combos, (combos->n + 1) * sizeof(pCombo));
  combos->combos[combos->n].pidA = a;
  combos->combos[combos->n].pidB = b;
  combos->n++;
}

void freeCombos(pCombos* combos) {
  free(combos->combos);
  free(combos);
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

char** parseComboLine(char* line, int* n) {
  if (line[0] == '!' || line[0] == '+') {
   line++;
  }
  *n = 0;
  char** names = malloc(sizeof(char*));
  char* token = strtok(line, "-");
  if (token != NULL) names[(*n)++] = strdup(trimWS(token));

  while ((token = strtok(NULL, "-"))) {
    names = realloc(names, ((*n)+1) * sizeof(char*));
    names[(*n)++] = strdup(trimWS(token));
  }
  return names;
}

void parseCombo(char* line, char** nA, char** nB) {
  if (line[0] == '!' || line[0] == '+') {
   line++;
  }
  char* token = strtok(line, "-");
  if (token != NULL) *nA = strdup(trimWS(token));
  token = strtok(NULL, "-");
  if (token != NULL) *nB = strdup(trimWS(token));
  assert(*nA != NULL && *nB != NULL);
}

player** readPlayers(const char *fileName, int *pn, pCombos* bpcs, pCombos* prefCombos) {
  FILE *fp = fopen(fileName, "rb");
  player** ps = NULL;
  int pid = 0;
  if (fp == NULL) return NULL;

  char line[256];
  while (fgets(line, sizeof(line), fp))
  {
    line[strcspn(line, "\n")] = 0;
    if (line[0] == '#' || strcmp(trimWS(line), "") == 0) continue;
    if (line[0] == '!' || line[0] == '+') {
      char fc = line[0];

      int idA = -1;
      int n = 0;
      char** names = parseComboLine(line, &n);
      assert(n > 0);
      for (int i = 0; i < *pn; i++) {
        if (strcmp(ps[i]->firstName, names[0]) == 0) {
          idA = ps[i]->id;
          break;
        }
      }
      for (int i = 1; i < n; i++) {
        int idB = -1;
        for (int j = 0; j < *pn; j++) {
          if (strcmp(ps[j]->firstName, names[i]) == 0) {
            idB = ps[j]->id;
            break;
          }
        }
        if (idA >= 0 && idB >= 0 && fc == '!') addCombo(bpcs, idA, idB);
        else if (idA >= 0 && idB >= 0 && fc == '+') addCombo(prefCombos, idA, idB);
      }
      for (int i = 0; i < n; i++) free(names[i]);
      free(names);
    } else {
      ps = realloc(ps, (*pn + 1) * sizeof(player*));
      ps[*pn] = parsePlayer(line);
      ps[(*pn)++]->id = pid++;
    }
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

int isInCombo(pCombos* combos, player* a) {
  for (int i = 0; i < combos->n; i++) {
    if ((combos->combos[i].pidA == a->id || combos->combos[i].pidB == a->id)) {
      return i;
    }
  }
  return -1;
}

char isCombo(pCombos* combos, player* a, player* b) {
  for (int i = 0; i < combos->n; i++) {
    if ((combos->combos[i].pidA == a->id && combos->combos[i].pidB == b->id) ||
      (combos->combos[i].pidA == b->id && combos->combos[i].pidB == a->id)) {
      return 1;
    }
  }
  return 0;
}

double averageRating(team** teams, pCombos* prefCombos) {
  int n = TEAMS_N * TEAM_SIZE;
  if (teams == NULL) return 0.0;
  double sumRating = 0.0;
  for (int t = 0; t < TEAMS_N; t++) {
    for (int p = 0; p < TEAM_SIZE; p++) {
      if (isInCombo(prefCombos, teams[t]->players[p]) >= 0) {
        n--;
        continue;
      }
      sumRating += ovRating(teams[t]->players[p]);
    }
  }
  return (n <= 0) ? 0.0 : sumRating / n;
}

int validateSwap(double a, double b, double aNew, double bNew, double avg, int oneSideValidation) {
  int valid = 0;
  if (fabs(aNew - avg) < fabs(a - avg)) valid++;
  if (fabs(bNew - avg) < fabs(b - avg)) valid++;
  return (valid == 2) ? 1 : (valid == 1 && oneSideValidation) ? 1 : 0;
}

int maxTeamFromPrefCombos(pCombos* prefCombos) {
  int maxSize = 0;
  int nTeams = 1;
  int* ns = calloc(nTeams, sizeof(int));
  int** presetTeams = malloc(sizeof(int*));
  *presetTeams = calloc(2, sizeof(int));

  for (int i = 0; i < prefCombos->n; i++) {
    char comboSet = 0;
    if (nTeams == 1 && ns[0] == 0) {
      presetTeams[0][ns[0]++] = prefCombos->combos[i].pidA;
      presetTeams[0][ns[0]++] = prefCombos->combos[i].pidB;
      maxSize = ns[0];
      continue;
    }
    for (int j = 0; j < nTeams; j++) {
      for (int k = 0; k < ns[j]; k++) {
        int id = (presetTeams[j][k] == prefCombos->combos[i].pidA) ? prefCombos->combos[i].pidB : (presetTeams[j][k] == prefCombos->combos[i].pidB) ? prefCombos->combos[i].pidA : -1;
        if (id >= 0) {
          char inTeam = 0;
          for (int l = 0; l < ns[j]; l++) {
            if (presetTeams[j][l] == id) inTeam = 1;
          }
          if (!inTeam) {
            presetTeams[j] = realloc(presetTeams[j], (ns[j] + 1) * sizeof(int));
            presetTeams[j][ns[j]++] = id;
            if (ns[j] > maxSize) maxSize = ns[j];
            comboSet = 1;
            break;
          }
        } 
      }
      if (comboSet) break;
    }
     if (!comboSet) {
      presetTeams = realloc(presetTeams, (nTeams + 1) * sizeof(int*));
      ns = realloc(ns, (nTeams + 1) * sizeof(int));
      ns[nTeams] = 0;
      presetTeams[nTeams] = calloc(2, sizeof(int));
      presetTeams[nTeams][ns[nTeams]++] = prefCombos->combos[i].pidA;
      presetTeams[nTeams][ns[nTeams]++] = prefCombos->combos[i].pidB;
      nTeams++;
    }
  }

  for (int i = 0; i < nTeams; i++) {
    free(presetTeams[i]);
  }
  free(presetTeams);
  free(ns);
  return maxSize;
}

void setPreferredCombos(team** teams, pCombos* prefCombos) {
  for (int c = 0; c < prefCombos->n; c++) {
    int t1 = -1;
    int t2 = -1;
    player* p1 = NULL;
    player* p2 = NULL;
    player* pToSwap = NULL;
    // Finds the players in the combo and their teams
    for (int i = 0; i < TEAMS_N; i++) {
      for (int j = 0; j < TEAM_SIZE; j++) {
        if (teams[i]->players[j]->id == prefCombos->combos[c].pidA) {
          p1 = teams[i]->players[j];
          t1 = i;
        }
        else if (teams[i]->players[j]->id == prefCombos->combos[c].pidB) {
          p2 = teams[i]->players[j];
          t2 = i;
        }
      }
      if (p1 && p2) break;
    }
    if (t1 == t2 || t1 < 0 || t2 < 0) continue;
    // Finds player to swap
    for (int i = 0; i < TEAM_SIZE; i++) {
      player* maybeSwapP = teams[t1]->players[i];
      if (maybeSwapP->id == p1->id) continue;
      // Makes sure the player has no combos with anyone on the team
      char hasCombo = 0;
      for (int j = 0; j < TEAM_SIZE; j++) {
        if ((hasCombo = isCombo(prefCombos, maybeSwapP, teams[t1]->players[j]))) break;
      }
      if (hasCombo) continue;
      else {
        pToSwap = maybeSwapP;
        break;
      }
    }
    if (pToSwap) swapPlayers(pToSwap, p2);
  }
}

int balancedClustering(team** teams, int oneSideValidation, pCombos* bpcs, pCombos* prefCombos) {
  double avgR = averageRating(teams, prefCombos);
  int swaps = 0;
  int failures = 0;

  while(failures < MAX_FAILURES && swaps < MAX_SWAPS) {
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

    for (int pI = 0; pI < TEAM_SIZE; pI++) {
      if (isCombo(prefCombos, pA, teams[teamB]->players[pI]) || isCombo(prefCombos, pB, teams[teamA]->players[pI])) {
        valid = 0;
        break;
      }
    }

    if (valid) {
      char banned = 0;
      for (int pI = 0; pI < TEAM_SIZE; pI++) {
        if (isCombo(bpcs, pA, teams[teamA]->players[pI]) || isCombo(bpcs, pB, teams[teamB]->players[pI])) {
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
    #ifdef __linux__
      printf("%3d/%3d | %d\r", failures, MAX_FAILURES, swaps);
    #endif
  }

  for (int i = 0; i < TEAMS_N; i++) {
    qsort(teams[i]->players, TEAM_SIZE, sizeof(player*), cmpPlayers);
  }

  if (swaps >= MAX_SWAPS && oneSideValidation) swaps += balancedClustering(teams, 0, bpcs, prefCombos);

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
  pCombos* bannedCombos = initCombos();
  pCombos* prefCombos = initCombos();
  player** players = readPlayers(fileName, pn, bannedCombos, prefCombos);

  if (!players) {
    printf("File %s not found\n", fileName);
    free(pn);
    exit(1);
  }

  int maxSize = maxTeamFromPrefCombos(prefCombos);
  if (maxSize > TEAM_SIZE) {
    printf("Trying to put %d players into the same team, but team size is %d\n", maxSize, TEAM_SIZE);
    exit(1);
  }

  qsort(players, *pn, sizeof(player*), cmpPlayers);

  if (print) printPlayers(players, *pn);
  printf("\nBanned combinations: %d\n", (int)bannedCombos->n);
  printf("Preferred combinations: %d\n", (int)prefCombos->n);

  if (*pn != TEAMS_N * TEAM_SIZE) {
    printf("\nFile %s contains %d players, but %d was expected\n", fileName, *pn, TEAMS_N * TEAM_SIZE);
    exit(1);
  }

  team** teams = balanceTeamsRand(players, *pn);

  setPreferredCombos(teams, prefCombos);

  if (clustering) {
    printf("\nBalancing teams..\n");
    int swaps = balancedClustering(teams, 1, bannedCombos, prefCombos);
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
    freeTeam(teams[i], TEAM_SIZE);
  }
  free(teams);
  free(pn);
  freeCombos(bannedCombos);
  freeCombos(prefCombos);

  return 0;
}


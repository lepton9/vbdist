#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include "../include/tuiSwitch.h"
#include "../include/args.h"
#include "../include/sql.h"

#define MAX_FAILURES 300
#define MAX_SWAPS 1000000

typedef enum {
  PRINT_MINIMAL,
  PRINT_NORATING,
  PRINT_ALL
} printMode;

typedef enum {
  NO_SOURCE,
  TEXT_FILE,
  DATABASE
} dataSource;

int TEAMS_N = 0;
int TEAM_SIZE = 0;
dataSource SOURCE = NO_SOURCE;


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
  if (line[0] == '!' || line[0] == '?' || line[0] == '+') {
   line++;
  }
  *n = 0;
  char** tokens = malloc(sizeof(char*));
  char* token = strtok(line, "-");
  if (token != NULL) tokens[(*n)++] = strdup(trimWS(token));

  while ((token = strtok(NULL, "-"))) {
    tokens = realloc(tokens, ((*n)+1) * sizeof(char*));
    tokens[(*n)++] = strdup(trimWS(token));
  }
  return tokens;
}

void parseCombos(char* line, player** ps, int* pn, pCombos* bpcs, pCombos* prefCombos) {
  char fc = line[0];

  int idA = -1;
  int n = 0;
  char** tokens = parseComboLine(line, &n);
  assert(n > 0);
  for (int i = 0; i < *pn; i++) {
    if ((SOURCE == TEXT_FILE && strcmp(ps[i]->firstName, tokens[0]) == 0)
    || (SOURCE == DATABASE && ps[i]->id == atoi(tokens[0]))) {
      idA = ps[i]->id;
      break;
    }
  }
  for (int i = 1; i < n; i++) {
    int idB = -1;
    for (int j = 0; j < *pn; j++) {
      if ((SOURCE == TEXT_FILE && strcmp(ps[j]->firstName, tokens[i]) == 0)
      || (SOURCE == DATABASE && ps[j]->id == atoi(tokens[i]))) {
        idB = ps[j]->id;
        break;
      }
    }
    if (idA >= 0 && idB >= 0) {
      if (fc == '+') addCombo(prefCombos, idA, idB);
      else if (fc == '!') addCombo(bpcs, idA, idB);
      else if (fc == '?') {
        for (int j = 0; j < *pn; j++) {
          if ((SOURCE == TEXT_FILE && strcmp(ps[j]->firstName, tokens[i - 1]) == 0)
            || (SOURCE == DATABASE && ps[j]->id == atoi(tokens[i - 1]))) {
            idA = ps[j]->id;
            break;
          }
        }
        for (int j = i; j < n; j++) {
          for (int k = 0; k < *pn; k++) {
            if ((SOURCE == TEXT_FILE && strcmp(ps[k]->firstName, tokens[j]) == 0)
              || (SOURCE == DATABASE && ps[k]->id == atoi(tokens[j]))) {
              idB = ps[k]->id;
              addCombo(bpcs, idA, idB);
              break;
            }
          }
        }
      }
    }
  }
  for (int i = 0; i < n; i++) free(tokens[i]);
  free(tokens);
}

player** readPlayers(const char *fileName, int *pn, pCombos* bpcs, pCombos* prefCombos) {
  FILE *fp = fopen(fileName, "rb");
  player** ps = NULL;
  int pid = 0;
  if (fp == NULL) return NULL;

  char line[256];
  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, "\n")] = 0;
    if (line[0] == '#' || strcmp(trimWS(line), "") == 0) continue;
    if (line[0] == '!' || line[0] == '?' || line[0] == '+') {
      parseCombos(line, ps, pn, bpcs, prefCombos);
    } else {
      switch (SOURCE) {
        case DATABASE: {
          ps = realloc(ps, (*pn + 1) * sizeof(player*));
          ps[*pn] = initPlayer();
          ps[*pn]->id = atoi(line);
          (*pn)++;
          break;
        }
        case TEXT_FILE: {
          ps = realloc(ps, (*pn + 1) * sizeof(player*));
          ps[*pn] = parsePlayer(line);
          ps[(*pn)++]->id = pid++;
          break;
        }
        default:
          break;
      }
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

void printTeams(FILE* out, team** teams, const int printMode, const int printWidth, const int teamsOnLine, const char indent) {
  char str[printWidth];
  for (int t = 0; t < TEAMS_N; t += teamsOnLine) {
    for (int i = t; i < t + teamsOnLine && i < TEAMS_N; i++) {
      if (printMode == PRINT_MINIMAL) {
        fprintf(out, "%-*s", printWidth, teams[i]->name);
      } else {
        sprintf(str, "%s | %.2f:", teams[i]->name, avgRating(teams[i]));
        fprintf(out, "%-*s", printWidth, str);
      }
    }
    fprintf(out, "\n");
    for(int j = 0; j < TEAM_SIZE; j++) {
      for(int i = t; i < TEAMS_N && i - t < teamsOnLine; i++) {
        if (printMode == PRINT_ALL) {
          sprintf(str, "%s%-10s (%.1f)", (indent) ? "  " : "", teams[i]->players[j]->firstName, ovRating(teams[i]->players[j]));
          fprintf(out, "%-*s", printWidth, str);
        } else {
          sprintf(str, "%s%-10s", (indent) ? "  " : "", teams[i]->players[j]->firstName);
          fprintf(out, "%-*s", printWidth, str);
        }
      }
      fprintf(out, "\n");
    }
    fprintf(out, "\n");
  }
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

  for (int i = 0; i < (int)prefCombos->n; i++) {
    char comboSet = 0;
    if (nTeams == 1 && ns[0] == 0) {
      presetTeams[0][ns[0]++] = prefCombos->combos[i].pidA;
      presetTeams[0][ns[0]++] = prefCombos->combos[i].pidB;
      maxSize = ns[0];
      continue;
    }
    for (int j = 0; j < nTeams; j++) {
      for (int k = 0; k < ns[j]; k++) {
        int id = (presetTeams[j][k] == prefCombos->combos[i].pidA)
                     ? prefCombos->combos[i].pidB
                 : (presetTeams[j][k] == prefCombos->combos[i].pidB)
                     ? prefCombos->combos[i].pidA
                     : -1;
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
  for (int c = 0; c < (int)prefCombos->n; c++) {
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

    if (comboInTeam(prefCombos, teams[teamB], pA) || comboInTeam(prefCombos, teams[teamA], pB)) valid = 0;

    if (valid) {
      if (comboInTeam(bpcs, teams[teamA], pA) || comboInTeam(bpcs, teams[teamB], pB)) {
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
    char tName[20];
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

void writeTeamsToFile(team** teams, const char* teamsFile) {
  FILE* fp = fopen(teamsFile, "a");
  fprintf(fp, "\n");

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  fprintf(fp, "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  printTeams(fp, teams, PRINT_MINIMAL, 15, TEAMS_N, 0);
  fclose(fp);
}

void changeMode(team** teams, pCombos* bpcs) {
  tui* tui = initTui(TEAM_SIZE, TEAMS_N);
  char c = 0;

  cls(stdout);
  updateTUI(stdout, tui, teams, bpcs);

  while (c != 'q') {
    c = keyPress();
    switch (c) {
      case 13: // On windows
      case '\n':
      case ' ':
        if (selectCur(tui)) {
          switchPos(tui, teams);
          unselect(tui->selected);
        }
        break;
      case 27: // Esc
        unselect(tui->selected);
        break;
      case 'k':
      case 'w':
        cur_up(tui);
        break;
      case 'h':
      case 'a':
        cur_left(tui);
        break;
      case 'l':
      case 'd':
        cur_right(tui);
        break;
      case 'j':
      case 's':
        cur_down(tui);
        break;
      default: {
        if (isdigit(c)) {
          int d = c - '0';
          markCurPlayer(tui, teams, getMarkColor(d));
        }
        break;
      }
    }
    updateTUI(stdout, tui, teams, bpcs);
  }
  cls(stdout);
  freeTui(tui);
}

void askSaveToFile(char* fileName, team** teams) {
  printf("\nSave teams to a file? [y/N] ");
  fflush(stdout);
  char ans = keyPress();
  if (ans == 'y' || ans == 'Y') {
    writeTeamsToFile(teams, fileName);
    printf("\033[2K");
    printf("Saved to %s\n", fileName);
  }
}

void askSaveToDB(sqldb* db, team** teams) {
  printf("Save teams to the database? [y/N] ");
  fflush(stdout);
  char ans = keyPress();
  if (ans == 'y' || ans == 'Y') {
    printf("\033[2K");
    for (int i = 0; i < TEAMS_N; i++) {
      insertTeam(db, teams[i]);
      for (int j = 0; j < TEAM_SIZE; j++) {
        insertPlayerTeam(db, teams[i]->players[j], teams[i]);
      }
    }
    printf("Saved to %s\n", db->path);
  }
}

int main(int argc, char** argv) {

#ifdef _WIN32
  char ret = initScreenWin();
  if (ret <= 0) {
    printf("Initializing screen failed\n");
    exit(1);
  }
#else
  initScreen();
#endif

  args* params = parseArgs(argc, argv);
  if (!params) {
    exit(0);
  }

  SOURCE = (params->fileName && params->dbName) ? DATABASE
       : (params->fileName)                 ? TEXT_FILE
                                            : NO_SOURCE;

  // TODO: check correct file format
  if (params->players <= 0 || params->teams <= 0 || SOURCE == NO_SOURCE) {
    printUsage(stdout);
    exit(1);
  }

  TEAMS_N = params->teams;
  TEAM_SIZE = params->players;

  char* teamsOutFile = "teams.txt";
  int clustering = 1;

  int* pn = malloc(sizeof(int));
  *pn = 0;
  int valid_players = 0;
  pCombos* bannedCombos = initCombos();
  pCombos* prefCombos = initCombos();
  sqldb* db = NULL;
  player** players = NULL;

  switch (SOURCE) {
    case DATABASE: {
      db = openSqlDB(params->dbName);
      if (!db->sqlite) {
        exit(1);
      }
      int r = createDB(db);
      if (r) printf("Created tables\n");
      players = readPlayers(params->fileName, pn, bannedCombos, prefCombos);
      valid_players = *pn;
      if (!players) {
        printf("Can't find players\n");
        exit(1);
      }
      for (int i = 0; i < *pn; i++) {
        int found = fetchPlayer(db, players[i]);
        if (!found) {
          valid_players--;
          printf("Player with id %d not found\n", players[i]->id);
        }
      }
      break;
    }
    case TEXT_FILE: {
      players = readPlayers(params->fileName, pn, bannedCombos, prefCombos);
      valid_players = *pn;
      break;
    }
    default:
      break;
  }

  int maxSize = maxTeamFromPrefCombos(prefCombos);
  if (maxSize > TEAM_SIZE) {
    printf("Trying to put %d players into the same team, but team size is %d\n", maxSize, TEAM_SIZE);
    exit(1);
  }

  qsort(players, *pn, sizeof(player*), cmpPlayers);

  if (params->printMode == PRINT_ALL) printPlayers(players, *pn);
  printf("\nBanned combinations: %d\n", (int)bannedCombos->n);
  printf("Preferred combinations: %d\n", (int)prefCombos->n);

  if (*pn != TEAMS_N * TEAM_SIZE) {
    printf("\nFile %s contains %d players, but %d was expected\n", params->fileName, *pn, TEAMS_N * TEAM_SIZE);
    exit(1);
  }
  if (valid_players != TEAMS_N * TEAM_SIZE) {
    printf("\nFound %d valid players, but %d was expected\n", valid_players, TEAMS_N * TEAM_SIZE);
    exit(1);
  }

  team** teams = balanceTeamsRand(players, *pn);

  setPreferredCombos(teams, prefCombos);

  if (clustering) {
    printf("\nBalancing teams..\n");
    int swaps = balancedClustering(teams, 1, bannedCombos, prefCombos);
    printf("Total swaps: %d\n", swaps);
  }

  if (params->printMode != PRINT_MINIMAL) printTeams(stdout, teams, params->printMode, 30, 2, 1);

  printf("\nManually change teams? [y/N] ");
  fflush(stdout);
  char ans = keyPress();
  printf("\033[2K\n");
  if (ans == 'y' || ans == 'Y') {
    changeMode(teams, bannedCombos);
  }

  printTeams(stdout, teams, PRINT_MINIMAL, 15, 3, 0);

  switch (SOURCE) {
    case TEXT_FILE: {
      askSaveToFile(teamsOutFile, teams);
      break;
    }
    case DATABASE: {
      askSaveToFile(teamsOutFile, teams);
      askSaveToDB(db, teams);
      closeSqlDB(db);
      break;
    }
    default:
      break;
  }

  for (int i = 0; i < TEAMS_N; i++) {
    freeTeam(teams[i]);
  }

  free(teams);
  free(pn);
  freeCombos(bannedCombos);
  freeCombos(prefCombos);

  return 0;
}


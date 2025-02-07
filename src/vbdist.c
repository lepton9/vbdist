#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#include "../include/tuiswap.h"
#include "../include/tuidb.h"
#include "../include/tui.h"
#include "../include/args.h"
#include "../include/sql.h"
#include "../include/log.h"
#include "../include/utils.h"

#define MAX_FAILURES 300
#define MAX_SWAPS 1000000
#define TEAMS_FILE "teams.txt"

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
printMode PRINT_MODE = PRINT_MINIMAL;


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

void parseCombos(char* line, dlist* players, dlist* bpcs, dlist* prefCombos) {
  char fc = line[0];
  int idA = -1;
  int n = 0;
  char** tokens = parseComboLine(line, &n);
  assert(n > 0);
  for (int i = 0; i < (int)players->n; i++) {
    if ((SOURCE == TEXT_FILE && strcmp(((player*)players->items[i])->firstName, tokens[0]) == 0)
    || (SOURCE == DATABASE && ((player*)players->items[i])->id == atoi(tokens[0]))) {
      idA = ((player*)players->items[i])->id;
      break;
    }
  }
  for (int i = 1; i < n; i++) {
    int idB = -1;
    for (int j = 0; j < (int)players->n; j++) {
      player* pj = ((player*)players->items[j]);
      if ((SOURCE == TEXT_FILE && strcmp(pj->firstName, tokens[i]) == 0)
      || (SOURCE == DATABASE && pj->id == atoi(tokens[i]))) {
        idB = pj->id;
        break;
      }
    }
    if (idA >= 0 && idB >= 0) {
      if (fc == '+') addCombo(prefCombos, idA, idB);
      else if (fc == '!') addCombo(bpcs, idA, idB);
      else if (fc == '?') {
        for (int j = 0; j < (int)players->n; j++) {
          player* pj = ((player*)players->items[j]);
          if ((SOURCE == TEXT_FILE && strcmp(pj->firstName, tokens[i - 1]) == 0)
            || (SOURCE == DATABASE && pj->id == atoi(tokens[i - 1]))) {
            idA = pj->id;
            break;
          }
        }
        for (int j = i; j < n; j++) {
          for (int k = 0; k < (int)players->n; k++) {
            player* pk = ((player*)players->items[k]);
            if ((SOURCE == TEXT_FILE && strcmp(pk->firstName, tokens[j]) == 0)
              || (SOURCE == DATABASE && pk->id == atoi(tokens[j]))) {
              idB = pk->id;
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

dlist* readPlayers(const char *fileName, dlist* bpcs, dlist* prefCombos) {
  FILE *fp = fopen(fileName, "rb");
  dlist* ps = init_list(sizeof(player*));
  int pid = 0;
  if (fp == NULL) {
    free_list(ps);
    return NULL;
  }

  char line[256];
  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, "\n")] = 0;
    if (line[0] == '#' || strcmp(trimWS(line), "") == 0) continue;
    if (line[0] == '!' || line[0] == '?' || line[0] == '+') {
      parseCombos(line, ps, bpcs, prefCombos);
    } else {
      switch (SOURCE) {
        case DATABASE: {
          player* p = initPlayer();
          p->id = atoi(line);
          if (playerInList(ps, p->id) < 0) {
            list_add(ps, p);
          } else {
            free(p);
          }
          break;
        }
        case TEXT_FILE: {
          player* p = parsePlayer(line);
          p->id = pid++;
          list_add(ps, p);
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

void printPlayers(dlist* players) {
  for (int i = 0; i < (int)players->n; i++) {
    printPlayer(stdout, players->items[i]);
  }
}

void printTeams(FILE *out, team **teams, const int printWidth,
                const int teamsOnLine, const char indent) {
  char str[printWidth];
  for (int t = 0; t < TEAMS_N; t += teamsOnLine) {
    for (int i = t; i < t + teamsOnLine && i < TEAMS_N; i++) {
      if (PRINT_MODE == PRINT_MINIMAL) {
        fprintf(out, "%-*s", printWidth, teams[i]->name);
      } else {
        sprintf(str, "%s | %.2f:", teams[i]->name, avgRating(teams[i]));
        fprintf(out, "%-*s", printWidth, str);
      }
    }
    fprintf(out, "\n");
    for(int j = 0; j < TEAM_SIZE; j++) {
      for(int i = t; i < TEAMS_N && i - t < teamsOnLine; i++) {
        if (PRINT_MODE == PRINT_ALL) {
          sprintf(str, "%s%-10s (%.1f)", (indent) ? "  " : "",
                  teams[i]->players[j]->firstName,
                  ovRating(teams[i]->players[j]));
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

double averageRating(team** teams, dlist* prefCombos) {
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

int maxTeamFromPrefCombos(dlist* prefCombos) {
  int maxSize = 0;
  int nTeams = 1;
  int* ns = calloc(nTeams, sizeof(int));
  int** presetTeams = malloc(sizeof(int*));
  *presetTeams = calloc(2, sizeof(int));
  pCombo* prefCombo = NULL;

  for (int i = 0; i < (int)prefCombos->n; i++) {
    char comboSet = 0;
    if (nTeams == 1 && ns[0] == 0) {
      prefCombo = prefCombos->items[i];
      presetTeams[0][ns[0]++] = prefCombo->pidA;
      presetTeams[0][ns[0]++] = prefCombo->pidB;
      maxSize = ns[0];
      continue;
    }
    for (int j = 0; j < nTeams; j++) {
      for (int k = 0; k < ns[j]; k++) {
        int id = (presetTeams[j][k] == prefCombo->pidA)
                     ? prefCombo->pidB
                 : (presetTeams[j][k] == prefCombo->pidB)
                     ? prefCombo->pidA
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
      presetTeams[nTeams][ns[nTeams]++] = prefCombo->pidA;
      presetTeams[nTeams][ns[nTeams]++] = prefCombo->pidB;
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

void setPreferredCombos(team** teams, dlist* prefCombos) {
  for (int c = 0; c < (int)prefCombos->n; c++) {
    int t1 = -1;
    int t2 = -1;
    player* p1 = NULL;
    player* p2 = NULL;
    player* pToSwap = NULL;
    pCombo* combo = prefCombos->items[c];
    // Finds the players in the combo and their teams
    for (int i = 0; i < TEAMS_N; i++) {
      for (int j = 0; j < TEAM_SIZE; j++) {
        if (teams[i]->players[j]->id == combo->pidA) {
          p1 = teams[i]->players[j];
          t1 = i;
        }
        else if (teams[i]->players[j]->id == combo->pidB) {
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

int balancedClustering(team** teams, int oneSideValidation, dlist* bpcs, dlist* prefCombos) {
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

team** balanceTeamsRand(dlist* players) {
  team** teams = malloc(sizeof(team*) * TEAMS_N);
  for (int i = 0; i < TEAMS_N; i++) {
    char tName[20];
    sprintf(tName, "Team %d", i + 1);
    teams[i] = initTeam(tName, TEAM_SIZE);
  }

  qsort(players->items, players->n, sizeof(player*), cmpPlayers);
  srand(time(0));

  int group = 0;
  int inTeam[TEAMS_N * TEAM_SIZE];
  for (int i = 0; i < TEAMS_N * TEAM_SIZE; i++) inTeam[i] = 0;

  int teamI = 0;
  int teamCounters[TEAMS_N];
  for (int i = 0; i < TEAMS_N; i++) teamCounters[i] = 0;

  for (int i = 0; i < (int)players->n; i++) {
    int ind = randintRange(group * TEAMS_N, (group + 1) * TEAMS_N - 1);
    while(inTeam[ind]) ind = randintRange(group * TEAMS_N, (group + 1) * TEAMS_N - 1);
    inTeam[ind] = 1;

    teams[teamI]->players[teamCounters[teamI]++] = players->items[ind];
    teamI = (teamI + 1) % TEAMS_N;
    if (teamI == 0) group++;
  }
  return teams;
}

team* balanceTeams(dlist* players, const int n) {
  team* teams = malloc(TEAMS_N * (sizeof(team)));
  qsort(players, n, sizeof(player*), cmpPlayers);

  int teamI = 0;
  int teamCounters[TEAMS_N];
  for (int i = 0; i < TEAMS_N; i++) teamCounters[i] = 0;

  for (int i = 0; i < n; i++) {
    teams[teamI].players[teamCounters[teamI]++] = players->items[i];
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

  printTeams(fp, teams, 20, TEAMS_N, 0);
  fclose(fp);
}

void changeMode(team** teams, dlist* bpcs) {
  tuiswap* tui = initTui(TEAM_SIZE, TEAMS_N);
  int c = 0;

  cls(stdout);
  updateTUI(stdout, tui, teams, bpcs);

  while (c != 'q') {
    c = keyPress();
    switch (c) {
      case 13: case '\n': case ' ':
#ifdef __linux__
      case KEY_ENTER:
#endif
        if (selectCur(tui)) {
          switchPos(tui, teams);
          unselect(tui->selected);
        }
        break;
      case 27: // Esc
        unselect(tui->selected);
        break;
      case 'k': case 'w':
#ifdef __linux__
      case KEY_UP:
#endif
        cur_up(tui);
        break;
      case 'j': case 's':
#ifdef __linux__
      case KEY_DOWN:
#endif
        cur_down(tui);
        break;
      case 'h': case 'a':
#ifdef __linux__
      case KEY_LEFT:
#endif
        cur_left(tui);
        break;
      case 'l': case 'd':
#ifdef __linux__
      case KEY_RIGHT:
#endif
        cur_right(tui);
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

int askSaveToFile(char* fileName, team** teams) {
  printf("\nSave teams to a file? [y/N] ");
  fflush(stdout);
  char ans = keyPress();
  if (ans == 'y' || ans == 'Y') {
    writeTeamsToFile(teams, fileName);
    printf("\033[2K");
    printf("Saved to %s\n", fileName);
    log_log("Saved %d teams of %d players to file '%s'", TEAMS_N, TEAM_SIZE, fileName);
    return 1;
  }
  return 0;
}

int askSaveToDB(sqldb* db, team** teams) {
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
    log_log("Saved %d teams of %d players to database '%s'", TEAMS_N, TEAM_SIZE, db->path);
    return 1;
  }
  return 0;
}

int askUpdateParamNum(const char* query, int current) {
  const size_t max_len = 5;
  size_t len = 0;
  char new[max_len + 1];
  new[0] = '\0';
  int c = 0;
  while (1) {
    cls(stdout);
    printf("\n Current: %d\n", current);
    printf(" %s: %s", query, new);
    fflush(stdout);
    c = keyPress();
    if (isdigit(c) && len < max_len) {
      strcatc(new, &len, c);
    } else if (len > 0 && isBackspace(c)) {
      new[--len] = '\0';
    }
    else if (isEnter(c)) {
      break;
    } else if (c == 'q' || c == 27) {
      return current;
    }
  }
  return (strcmp(new, "") == 0) ? current : atoi(new);
}

int generateTeams(sqldb* db, dlist* players, dlist* bannedCombos, dlist* prefCombos) {
  int saved = 0;
  int clustering = 1;
  cls(stdout);
  if (PRINT_MODE == PRINT_ALL) printPlayers(players);
  printf("\nBanned combinations: %d\n", (int)bannedCombos->n);
  printf("Preferred combinations: %d\n", (int)prefCombos->n);

  team** teams = balanceTeamsRand(players);
  setPreferredCombos(teams, prefCombos);

  if (clustering) {
    printf("\nBalancing teams..\n");
    int swaps = balancedClustering(teams, 1, bannedCombos, prefCombos);
    printf("Total swaps: %d\n", swaps);
  }

  if (PRINT_MODE != PRINT_MINIMAL) printTeams(stdout, teams, 30, 2, 1);

  curShow();
  printf("\nManually change teams? [y/N] ");
  fflush(stdout);
  char ans = keyPress();
  printf("\033[2K\n");
  if (ans == 'y' || ans == 'Y') {
    curHide();
    changeMode(teams, bannedCombos);
    curShow();
  }

  log_log("Generated %d teams of %d players", TEAMS_N, TEAM_SIZE);
  printTeams(stdout, teams, 20, 3, 0);

  switch (SOURCE) {
    case TEXT_FILE: {
      askSaveToFile(TEAMS_FILE, teams);
      break;
    }
    case DATABASE: {
      askSaveToFile(TEAMS_FILE, teams);
      saved = askSaveToDB(db, teams);
      break;
    }
    default:
      break;
  }
  for (int i = 0; i < TEAMS_N; i++) {
    freeTeam(teams[i]);
  }
  free(teams);
  curHide();
  return saved;
}

void saveToDB(sqldb* db, dlist* players, dlist* bpcs, dlist* prefCombos) {
  if (SOURCE != DATABASE) return;
  if (players->n > 0) {
    saveToPlayerList(db, players);
  }
}

void runBeginTui(tuidb* tui, dlist* players, dlist* bpcs, dlist* prefCombos, char* err) {
  int teams_added = 0;
  char error_msg[1000];
  strcpy(error_msg, err);
  char c = 0;
  while (1) {
    cls(stdout);
    curSet(1, 1);
    printf("\n Players selected: %d/%d\n", (int)players->n, TEAMS_N * TEAM_SIZE);
    printf("\n Banned combinations: %d\n", (int)bpcs->n);
    printf(" Preferred combinations: %d\n", (int)prefCombos->n);
    printf("\n");

    printf(" [g] Generate teams\n");
    if (SOURCE == DATABASE) printf(" [d] Database\n");
    printf(" [t] Teams: %d\n", TEAMS_N);
    printf(" [p] Team size: %d\n", TEAM_SIZE);
    printf(" [q] Quit\n");

    printf("\n\033[31m%s\033[0m\n", error_msg);
    c = keyPress();
    error_msg[0] = '\0';
    switch (c) {
      case 'g':
        if ((int)players->n != TEAMS_N * TEAM_SIZE) {
          sprintf(error_msg, "Selected %d players, but %d was expected", (int)players->n, TEAMS_N * TEAM_SIZE);
        } else {
          teams_added = generateTeams((tui) ? tui->db : NULL, players, bpcs, prefCombos);
        }
        break;
      case 't':
        curShow();
        TEAMS_N = askUpdateParamNum("Set new team amount", TEAMS_N);
        curHide();
        break;
      case 'p':
        curShow();
        TEAM_SIZE = askUpdateParamNum("Set new team size", TEAM_SIZE);
        curHide();
        break;
      case 'q':
        saveToDB(tui->db, players, bpcs, prefCombos);
        cls(stdout);
        return;
      case 'd':
        if (SOURCE == DATABASE) {
          if (teams_added) {
            updateAllTeams(tui);
          }
          updateTeamSize(tui, TEAMS_N, TEAM_SIZE);
          runTuiDB(tui);
        }
        break;
      default: {
        break;
      }
    }
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

  SOURCE = (params->dbName)     ? DATABASE
           : (params->fileName) ? TEXT_FILE
                                : NO_SOURCE;

  if (SOURCE == NO_SOURCE) {
    printUsage(stdout);
    exit(1);
  }

  TEAMS_N = params->teams;
  TEAM_SIZE = params->players;
  PRINT_MODE = params->printMode;

  dlist* bannedCombos = init_list(sizeof(pCombo));
  dlist* prefCombos = init_list(sizeof(pCombo));
  sqldb* db = NULL;
  dlist* players = NULL;
  char* err_msg = malloc(1);
  err_msg[0] = '\0';

  switch (SOURCE) {
    case DATABASE: {
      db = openSqlDB(params->dbName);
      if (!db->sqlite) {
        exit(1);
      }
      int r = createDB(db);
      if (r) printf("Created tables\n");
      players = (params->fileName)
                    ? readPlayers(params->fileName, bannedCombos, prefCombos)
                    : fetchPlayerList(db);
      for (int i = 0; i < (int)players->n;) {
        int found = fetchPlayer(db, players->items[i]);
        if (!found) {
          char msg[100];
          sprintf(msg, "Player with id %d not found\n", ((player*)players->items[i])->id);
          err_msg = realloc(err_msg, strlen(err_msg) + strlen(msg) + 1);
          strcat(err_msg, msg);
          freePlayer(players->items[i]);
          if (i != (int)players->n - 1) {
            players->items[i] = players->items[players->n - 1];
          }
          players->n--;
        } else {
          i++;
        }
      }
      break;
    }
    case TEXT_FILE: {
      players = readPlayers(params->fileName, bannedCombos, prefCombos);
      break;
    }
    default:
      break;
  }

  int maxSize = maxTeamFromPrefCombos(prefCombos);
  if (maxSize > TEAM_SIZE) {
    // TODO: handle somehow
    char msg[100];
    sprintf(msg, "Trying to put %d players into the same team, but team size is %d\n", maxSize, TEAM_SIZE);
    err_msg = realloc(err_msg, strlen(err_msg) + strlen(msg) + 1);
    strcat(err_msg, msg);
  }

  qsort(players->items, players->n, sizeof(player*), cmpPlayers);

  tuidb* tui = NULL;
  if (SOURCE == DATABASE) {
    tui = initTuiDB(TEAMS_N, TEAM_SIZE);
    tui->db = db;
    tui->allPlayers = fetchPlayers(db);
    tui->allTeams = fetchTeams(db);
    tui->players = players;
  }

  curHide();
  altBufferEnable();
  runBeginTui(tui, players, bannedCombos, prefCombos, err_msg);
  altBufferDisable();
  curShow();

  for (int i = 0; i < (int)players->n; i++) {
    freePlayer(players->items[i]);
  }
  free_list(players);
  freeArgs(params);
  free(err_msg);
  closeSqlDB(db);
  freeTuiDB(tui);
  freeCombos(bannedCombos);
  freeCombos(prefCombos);

  return 0;
}


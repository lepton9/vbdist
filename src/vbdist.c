#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "../include/tuiswap.h"
#include "../include/tuicombo.h"
#include "../include/tuidb.h"
#include "../include/tuiskills.h"
#include "../include/tuipositions.h"
#include "../include/tui.h"
#include "../include/args.h"
#include "../include/sql.h"
#include "../include/log.h"
#include "../include/utils.h"
#include "../include/config.h"
#include "../include/generate.h"

#define TEAMS_FILE "teams.txt"
#define MSG_LEN 1000

typedef enum {
  NO_SOURCE,
  TEXT_FILE,
  DATABASE
} dataSource;

#define MINIMUM_SIZE 2
int TEAMS_N = 0;
int TEAM_SIZE = 0;
dataSource SOURCE = NO_SOURCE;


void updateCombos(sqldb* db, dlist* players, dlist* combos, comboType combo_type) {
  for (size_t i = 0; i < combos->n; i++) {
    freeCombo(combos->items[i]);
    combos->items[i] = NULL;
  }
  combos->n = 0;

  dlist* new_combos = fetchCombos(db, combo_type);

  if (combos->size < new_combos->n) {
    void** resized_items = realloc(combos->items, new_combos->size * sizeof(combo*));
    combos->items = resized_items;
    combos->size = new_combos->size;
  }

  for (size_t i = 0; i < new_combos->n; i++) {
    combos->items[i] = new_combos->items[i];
  }
  combos->n = new_combos->n;

  free_list(new_combos);

  for (int i = combos->n - 1; i >= 0; i--) {
    if (!comboRelevant(players, combos->items[i])) {
      freeCombo(pop_elem(combos, i));
    }
  }
}

void updatePlayerCombos(sqldb* db, dlist* players, dlist* bannedCombos, dlist* prefCombos) {
  updateCombos(db, players, bannedCombos, BAN);
  updateCombos(db, players, prefCombos, PAIR);
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

void parseCombos(char* line, dlist* players, dlist* bpcs, dlist* prefCombos) {
  char fc = line[0];
  int n = 0;
  char** tokens = parseComboLine(line, &n); // Ids or names
  dlist* ids = init_list();
  assert(n > 0);

  // Find ids from players
  for (int t = 0; t < n; t++) {
    for (size_t i = 0; i < players->n; i++) {
      if (SOURCE == TEXT_FILE) {
        if (strcmp(playerName(get_elem(players, i)), tokens[t]) == 0) {
          int* id = malloc(sizeof(int));
          *id = ((player *)players->items[i])->id;
          list_add(ids, id);
          break;
        }
      }
      else if (SOURCE == DATABASE) {
        if (((player *)players->items[i])->id == atoi(tokens[t])) {
          int* id = malloc(sizeof(int));
          *id = ((player *)players->items[i])->id;
          list_add(ids, id);
          break;
        }
      }
    }
  }

  if (n == (int)ids->n) {
    if (fc == '+') {
      combo* c = initCombo(PAIR, -1);
      for (size_t i = 0; i < ids->n; i++) {
        addToCombo(c, *((int*)ids->items[i]));
      }
      list_add(prefCombos, c);
    }
    else if (fc == '?') {
      combo* c = initCombo(BAN, -1);
      for (size_t i = 0; i < ids->n; i++) {
        addToCombo(c, *((int*)ids->items[i]));
      }
      list_add(bpcs, c);
    }
    else if (fc == '!') {
      for (size_t i = 1; i < ids->n; i++) {
        combo* c = initCombo(BAN, -1);
        addToCombo(c, *((int*)ids->items[0]));
        addToCombo(c, *((int*)ids->items[i]));
        list_add(bpcs, c);
      }
    }
  }

  for (size_t i = 0; i < ids->n; i++) free(ids->items[i]);
  free_list(ids);
  for (int i = 0; i < n; i++) free(tokens[i]);
  free(tokens);
}

dlist* readPlayers(const char *fileName, dlist* bpcs, dlist* prefCombos) {
  FILE *fp = fopen(fileName, "rb");
  dlist* ps = init_list();
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
                const int teamsOnLine, const char indent, const char verbose) {
  char str[printWidth * 2];
  for (int t = 0; t < TEAMS_N; t += teamsOnLine) {
    for (int i = t; i < t + teamsOnLine && i < TEAMS_N; i++) {
      if (verbose) {
        sprintf(str, "%s | %.2f:", teams[i]->name, avgRating(teams[i]));
        fprintf(out, "%-*s", printWidth, str);
      } else {
        fprintf(out, "%-*s", printWidth, teams[i]->name);
      }
    }
    fprintf(out, "\n");
    for(int j = 0; j < TEAM_SIZE; j++) {
      for(int i = t; i < TEAMS_N && i - t < teamsOnLine; i++) {
        player* p = teams[i]->players[j];
        position* pos = assignedPosition(p);
        if (verbose && pos) {
          sprintf(str, "%s%-15s (%s | %d)", (indent) ? "  " : "", playerName(p),
                  (pos) ? pos->name : "", (pos) ? pos->priority : -1);
        } else {
          sprintf(str, "%s%-15s", (indent) ? "  " : "", playerName(p));
        }
        fprintf(out, "%-*s", printWidth, str);
      }
      fprintf(out, "\n");
    }
    fprintf(out, "\n");
  }
}

void printSkills(FILE *out, team **teams, dlist* skills, const int printWidth,
                const int onLine, const char indent) {
  char str[printWidth];
  for (int t = 0; t < TEAMS_N; t += onLine) {
    for (int i = t; i < t + onLine && i < TEAMS_N; i++) {
      sprintf(str, "%s | %.2f:", teams[i]->name, avgRating(teams[i]));
      fprintf(out, "%-*s", printWidth, str);
    }
    fprintf(out, "\n");
    for(size_t j = 0; j < skills->n; j++) {
      for(int i = t; i < TEAMS_N && i - t < onLine; i++) {
        skill* s = skills->items[j];
        sprintf(str, "%s%-10s %.1f", (indent) ? "  " : "", s->name, team_average_skill(teams[i], s));
        fprintf(out, "%-*s", printWidth, str);
      }
      fprintf(out, "\n");
    }
    fprintf(out, "\n");
  }
}

void writeTeamsToFile(team** teams, const char* teamsFile) {
  FILE* fp = fopen(teamsFile, "a");
  fprintf(fp, "\n");

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  fprintf(fp, "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  printTeams(fp, teams, 20, TEAMS_N, 0, 0);
  fclose(fp);
}

int askSaveToFile(char* fileName, team** teams) {
  printf("\nSave teams to a file? [y/N] ");
  fflush(stdout);
  char ans = keyPress();
  if (ans == 'y' || ans == 'Y') {
    writeTeamsToFile(teams, fileName);
    printf("\033[2K\r");
    printf("Saved to %s\n", fileName);
    log_log("Saved %d teams of %d players to file '%s'", TEAMS_N, TEAM_SIZE, fileName);
    return 1;
  }
  printf("\r");
  return 0;
}

int askSaveToDB(sqldb* db, team** teams) {
  printf("Save teams to the database? [y/N] ");
  fflush(stdout);
  char ans = keyPress();
  if (ans == 'y' || ans == 'Y') {
    printf("\033[2K\r");
    for (int i = 0; i < TEAMS_N; i++) {
      if (insertTeam(db, teams[i])) {
        insertTeamPlayers(db, teams[i]);
      }
    }
    printf("Saved to %s\n", db->path);
    log_log("Saved %d teams of %d players to database '%s'", TEAMS_N, TEAM_SIZE, db->path);
    return 1;
  }
  printf("\r");
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

void resetPositions(dlist* players) {
  for (size_t i = 0; i < players->n; i++) {
    resetPosition(players->items[i]);
  }
}

int generateTeams(sqldb *db, dlist *players, context* ctx) {
  int saved = 0;
  int clustering = 1;
  int printWidth = (ctx->use_positions) ? 35 : 20;
  cls(stdout);
  printf("\nBanned combinations: %d\n", (int)ctx->banned_combos->n);
  printf("Preferred combinations: %d\n\n", (int)ctx->pref_combos->n);

  team** teams = NULL;
  if (ctx->use_positions) {
    resetPositions(players);
    teams = initialTeamsPositions(players, ctx);
  } else {
    teams = initialTeams(players, ctx);
  }

  if (clustering) {
    printf("\nBalancing teams..\n");
    int swaps = balancedClustering(teams, 1, ctx);
    cls(stdout);
    printf("\033[2KSwapped players: %d\n\n", swaps);
  }
  flushInput();

  printTeams(stdout, teams, printWidth, 4, 1, 1);

  curShow();
  printf("\nManually change teams? [y/N] ");
  fflush(stdout);
  char ans = keyPress();
  printf("\033[2K\r");
  if (ans == 'y' || ans == 'Y') {
    curHide();
    runTuiSwap(teams, TEAMS_N, TEAM_SIZE, ctx->skills, ctx->banned_combos);
    curShow();
    printTeams(stdout, teams, printWidth, 4, 1, 1);
  }

  log_log("Generated %d teams of %d players", TEAMS_N, TEAM_SIZE);

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

void hydratePlayers(sqldb* db, dlist* allPlayers, dlist* players, dlist* skills, char* msg) {
  int hydrated_players = insertPlayersSkills(db, players, skills);
  snprintf(msg, MSG_LEN, "Hydrated %d players with %zu skills", hydrated_players,
           skills->n);
  if (hydrated_players == 0) return;
  // Sync allPlayers list
  for (size_t i = 0; i < players->n; i++) {
    player* p = get_elem(players, i);
    int ind = playerInList(allPlayers, p->id);
    if (ind >= 0) {
      fetchPlayerSkills(db, get_elem(allPlayers, ind));
    }
  }
}

void saveToDB(sqldb* db, dlist* players, dlist* bpcs, dlist* prefCombos) {
  if (SOURCE != DATABASE) return;
  saveToPlayerList(db, players);
  insertCombos(db, bpcs);
  insertCombos(db, prefCombos);
}

int checkTeamDimensions(context* ctx, dlist* players, char* msg) {
  if (TEAM_SIZE < MINIMUM_SIZE || TEAMS_N < MINIMUM_SIZE) {
    sprintf(msg, "Team size (%d) and amount (%d) minimum is %d", TEAM_SIZE,
            TEAMS_N, MINIMUM_SIZE);
    return 0;
  } else if ((int)players->n != TEAMS_N * TEAM_SIZE) {
    sprintf(msg, "Selected %zu players, but %d was expected", players->n,
            TEAMS_N * TEAM_SIZE);
    return 0;
  } else if (ctx->use_positions && (int)ctx->positions->n != TEAM_SIZE) {
    sprintf(msg, "Amount of positions should equal team size (%d/%d)",
            (int)ctx->positions->n, TEAM_SIZE);
    return 0;
  }
  return 1;
}

void runBeginTui(tuidb* tui, dlist* players, context* ctx, dlist* allSkills, dlist* allPositions, char* err) {
  int teams_added = 0;
  char* msg = malloc(MSG_LEN);
  char error_msg[MSG_LEN];
  strcpy(msg, "\0");
  strcpy(error_msg, err);
  int c = 0;
  while (1) {
    cls(stdout);
    curSet(1, 1);
    printf("\n Players selected: %zu/%d\n", players->n, TEAMS_N * TEAM_SIZE);
    printf("\n Banned combinations: %zu\n", ctx->banned_combos->n);
    printf(" Preferred combinations: %zu\n", ctx->pref_combos->n);
    printf("\n");

    printf(" [g] Generate teams\n");
    if (SOURCE == DATABASE) printf(" [d] Database\n");
    printf(" [t] Teams: %d\n", TEAMS_N);
    printf(" [p] Team size: %d\n", TEAM_SIZE);
    printf(" [s] Skills %zu/%zu\n", ctx->skills->n, allSkills->n);
    printf(" [c] Combos\n");
    printf(" [o] Positions: %s\n", (ctx->use_positions) ? "ON" : "OFF");
    printf(" [m] Comparison method: %s | %s\n",
           (ctx->compare == SKILL_AVERAGE) ? ">\033[4mSKILLS\033[0m<" : "SKILLS",
           (ctx->compare == OV_AVERAGE) ? ">\033[4mAVERAGE\033[0m<" : "AVERAGE");
    printf(" [h] Hydrate player skills\n");
    printf(" [q] Quit\n");

    printf("\n\033[%dm%s\033[0m\n", BLUE_FG, msg);
    printf("\n\033[%dm%s\033[0m\n", RED_FG, error_msg);
    msg[0] = '\0';
    error_msg[0] = '\0';
    c = keyPress();
    switch (c) {
      case 'G': case 'g':
        if (checkTeamDimensions(ctx, players, error_msg)) {
          ctxUpdateDimensions(ctx, TEAMS_N, TEAM_SIZE);
          teams_added = generateTeams((tui) ? tui->db : NULL, players, ctx) || teams_added;
        }
        break;
      case 'T': case 't':
        curShow();
        TEAMS_N = askUpdateParamNum("Set new team amount", TEAMS_N);
        curHide();
        break;
      case 'P': case 'p':
        curShow();
        TEAM_SIZE = askUpdateParamNum("Set new team size", TEAM_SIZE);
        curHide();
        break;
      case 'Q': case 'q':
        saveToDB(tui->db, players, ctx->banned_combos, ctx->pref_combos);
        free(msg);
        cls(stdout);
        return;
      case 'D': case 'd':
        if (SOURCE == DATABASE) {
          if (teams_added) {
            updateAllTeams(tui);
          }
          updateTeamSize(tui, TEAMS_N, TEAM_SIZE);
          qsort(players->items, players->n, sizeof(player*), cmpPlayers);
          runTuiDB(tui);
          updatePlayerCombos(tui->db, players, ctx->banned_combos, ctx->pref_combos);
        }
        break;
      case 'S': case 's':
        runTuiSkills(tui->db, allSkills, ctx->skills);
        break;
      case 'C': case 'c':
        runTuiCombo(tui->db, players);
        updatePlayerCombos(tui->db, players, ctx->banned_combos, ctx->pref_combos);
        break;
      case 'O': case 'o':
        ctx->use_positions = runTuiPositions(tui->db, allPositions, ctx->positions, ctx->use_positions);
        break;
      case 'M': case 'm':
        changeComparison(&ctx->compare);
        break;
      case 'H': case 'h':
        hydratePlayers(tui->db, tui->allPlayers, players, ctx->skills, msg);
        break;
      default: {
        break;
      }
    }
  }
}

dlist* sourceDatabase(sqldb* db, context* ctx, char* pFileName, char** err_msg) {
  dlist* players = NULL;
  players = (pFileName)
    ? readPlayers(pFileName, ctx->banned_combos, ctx->pref_combos)
    : NULL;
  if (!players) {
    players = fetchPlayerList(db);
  }
  if (ctx->banned_combos->n == 0) {
    updateCombos(db, players, ctx->banned_combos, BAN);
  } else {
    insertCombos(db, ctx->banned_combos);
  }
  if (ctx->pref_combos->n == 0) {
    updateCombos(db, players, ctx->pref_combos, PAIR);
  } else {
    insertCombos(db, ctx->pref_combos);
  }
  for (size_t i = 0; i < players->n;) {
    int found = fetchPlayer(db, players->items[i]);
    if (!found) {
      char msg[100];
      sprintf(msg, "Player with id %d not found\n", ((player*)players->items[i])->id);
      *err_msg = realloc(*err_msg, strlen(*err_msg) + strlen(msg) + 1);
      strcat(*err_msg, msg);
      freePlayer(players->items[i]);
      if (i != players->n - 1) {
        players->items[i] = players->items[players->n - 1];
      }
      players->n--;
    } else {
      i++;
    }
  }
  ctx->skills = fetchSkills(db);
  ctx->positions = fetchPositions(db);
  return players;
}

int handleAction(action a, args* args) {
  switch (a) {
    case ACTION_GENERATE:
      return -1;
    case ACTION_HELP:
      printUsage(stdout);
      return 0;
    case ACTION_CONFIG:
      printCfgLocation(stdout);
      return 0;
    case ACTION_ERROR:
      printArgsError(args, stdout);
      return 1;
  }
  return -1;
}

int main(int argc, char** argv) {
  char* err_msg = malloc(1);
  err_msg[0] = '\0';

  args* args = initArgs();
  int ret = handleAction(parseArgs(args, argc, argv), args);
  if (ret >= 0) {
    freeArgs(args);
    exit(ret);
  }

  SOURCE = (args->dbPath)     ? DATABASE
           : (args->filePath) ? TEXT_FILE
                                : NO_SOURCE;

  config* cfg = read_config();
  if (cfg && cfg->created) {
    char msg[600];
    snprintf(msg, sizeof(msg), "Config file created at '%s'\n", cfg->config_path);
    err_msg = realloc(err_msg, strlen(err_msg) + strlen(msg) + 1);
    strcat(err_msg, msg);
  }

  if (SOURCE == NO_SOURCE) {
    if (db_is_set(cfg)) {
      SOURCE = DATABASE;
    } else {
      printUsage(stdout);
      exit(1);
    }
  } else if (SOURCE == DATABASE) {
    set_db_path(cfg, args->dbPath);
  }

  context* ctx = makeContext();
  ctx->banned_combos = init_list();
  ctx->pref_combos = init_list();
  dlist* skills_all = NULL;
  dlist* positions_all = NULL;
  sqldb* db = NULL;
  dlist* players = NULL;

  TEAMS_N = (args->teams > 0) ? args->teams : cfg->teams_n;
  TEAM_SIZE = (args->players > 0) ? args->players : cfg->team_size;

  switch (SOURCE) {
    case DATABASE: {
      db = openSqlDB(cfg->db_path);
      if (!db->sqlite) {
        printf("Failed to open database '%s'\n", cfg->db_path);
        exit(1);
      }
      players = sourceDatabase(db, ctx, args->filePath, &err_msg);
      skills_all = copySkills(ctx->skills);
      positions_all = copyPositions(ctx->positions);
      break;
    }
    // TODO: deprecated, probably not working
    case TEXT_FILE: {
      players = readPlayers(args->filePath, ctx->banned_combos, ctx->pref_combos);
      if (!players) {
        printf("File %s not found\n", args->filePath);
        exit(1);
      }
      break;
    }
    default:
      break;
  }

  qsort(players->items, players->n, sizeof(player*), cmpPlayers);
  ctxUpdateDimensions(ctx, TEAMS_N, TEAM_SIZE);

  tuidb* tui = NULL;
  if (SOURCE == DATABASE) {
    tui = initTuiDB(TEAMS_N, TEAM_SIZE);
    tui->db = db;
    setAllPlayers(tui, fetchPlayers(db));
    setAllTeams(tui, fetchTeams(db));
    setAllPositions(tui, positions_all);
    tui->players = players;
  }

  term* term = NULL;
  if (!initScreen(&term)) {
    log_error("%s", "Initializing screen failed");
    printf("Initializing screen failed..\n");
    exit(1);
  }

  curHide();
  altBufferEnable();
  runBeginTui(tui, players, ctx, skills_all, positions_all, err_msg);
  altBufferDisable();
  curShow();

  endScreen(term);
  free(err_msg);

  cfg->team_size = TEAM_SIZE;
  cfg->teams_n = TEAMS_N;
  write_config(cfg);
  free(cfg);

  freeSkills(skills_all);
  freePositions(positions_all);

  for (size_t i = 0; i < players->n; i++) {
    freePlayer(players->items[i]);
  }
  free_list(players);
  freeArgs(args);
  closeSqlDB(db);
  freeTuiDB(tui);
  freeContext(ctx);

  return 0;
}


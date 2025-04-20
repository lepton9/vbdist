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


int comboRelevant(dlist* players, combo* combo) {
  int match = 0;
  for (int i = 0; i < (int)players->n; i++) {
    player* p = players->items[i];
    if (isInCombo(combo, p)) {
      match++;
    }
  }
  return match == (int)combo->ids->n;
}

void updateCombos(sqldb* db, dlist* players, dlist* combos, comboType combo_type) {
  for (int i = 0; i < (int)combos->n; i++) {
    free(combos->items[i]);
    combos->items[i] = NULL;
  }
  combos->n = 0;

  dlist* new_combos = fetchCombos(db, combo_type);

  if (combos->size < new_combos->n) {
    void** resized_items = realloc(combos->items, new_combos->size * sizeof(combo*));
    combos->items = resized_items;
    combos->size = new_combos->size;
  }

  for (int i = 0; i < (int)new_combos->n; i++) {
    combos->items[i] = new_combos->items[i];
  }
  combos->n = new_combos->n;

  free_list(new_combos);

  for (int i = combos->n - 1; i >= 0; i--) {
    if (!comboRelevant(players, combos->items[i])) {
      free(pop_elem(combos, i));
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
        if (strcmp(((player *)players->items[i])->firstName, tokens[t]) == 0) {
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
        player* p = teams[i]->players[j];
        if (PRINT_MODE == PRINT_ALL) {
          sprintf(str, "%s%-10s (%.1f)", (indent) ? "  " : "", p->firstName,
                  rating(p));
          fprintf(out, "%-*s", printWidth, str);
        } else {
          // TODO: print pos
          position* pos = assignedPosition(p);
          sprintf(str, "%s%-10s %s", (indent) ? "  " : "", p->firstName, (pos) ? pos->name : "");
          fprintf(out, "%-*s", printWidth, str);
        }
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

  printTeams(fp, teams, 20, TEAMS_N, 0);
  fclose(fp);
}

void changeMode(team** teams, dlist* bpcs) {
  tuiswap* tui = initTuiSwap(TEAM_SIZE, TEAMS_N);
  int c = 0;
  renderer* renderer = init_renderer(stdout);

  refresh_screen(renderer);
  while (c != 'q') {
    updateTuiSwap(renderer, tui, teams, bpcs);
    render(renderer);
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
      case 'K': case 'W':
      case 'k': case 'w':
#ifdef __linux__
      case KEY_UP:
#endif
        cur_up(tui);
        break;
      case 'J': case 'S':
      case 'j': case 's':
#ifdef __linux__
      case KEY_DOWN:
#endif
        cur_down(tui);
        break;
      case 'H': case 'A':
      case 'h': case 'a':
#ifdef __linux__
      case KEY_LEFT:
#endif
        cur_left(tui);
        break;
      case 'L': case 'D':
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
  }
  cls(stdout);
  free_renderer(renderer);
  freeTuiSwap(tui);
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
      insertTeam(db, teams[i]);
      for (int j = 0; j < TEAM_SIZE; j++) {
        insertPlayerTeam(db, teams[i]->players[j], teams[i]);
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

int generateTeams(sqldb *db, dlist *players, context* ctx) {
  int saved = 0;
  int clustering = 1;
  cls(stdout);
  if (PRINT_MODE == PRINT_ALL) printPlayers(players);
  printf("\nBanned combinations: %d\n", (int)ctx->banned_combos->n);
  printf("Preferred combinations: %d\n", (int)ctx->pref_combos->n);

  team** teams = NULL;
  if (ctx->use_positions) {
    teams = makeRandTeamsPositions(players, ctx->teams_dim, ctx->positions);
    // TODO: set combos, or make initialTeams() to handle positions
  } else {
    teams = initialTeams(players, ctx->teams_dim, ctx);
    // teams = balanceTeamsRand(players, ctx->teams_dim);
    // setPreferredCombos(teams, ctx->teams_dim, ctx->pref_combos);
  }

  if (clustering) {
    printf("\nBalancing teams..\n");
    int swaps = balancedClustering(teams, 1, ctx);
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
    changeMode(teams, ctx->banned_combos);
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
  saveToPlayerList(db, players);
  insertCombos(db, bpcs);
  insertCombos(db, prefCombos);
}

void runBeginTui(tuidb* tui, dlist* players, context* ctx, dlist* allSkills, dlist* allPositions, char* err) {
  dlist* selected_skills = initSelectedSkills(allSkills);
  dlist* selected_positions = fetchPositions(tui->db);
  ctx->skills = selected_skills;
  ctx->positions = selected_positions;

  int teams_added = 0;
  char error_msg[1000];
  strcpy(error_msg, err);
  char c = 0;
  while (1) {
    cls(stdout);
    curSet(1, 1);
    printf("\n Players selected: %d/%d\n", (int)players->n, TEAMS_N * TEAM_SIZE);
    printf("\n Banned combinations: %d\n", (int)ctx->banned_combos->n);
    printf(" Preferred combinations: %d\n", (int)ctx->pref_combos->n);
    printf("\n");

    printf(" [g] Generate teams\n");
    if (SOURCE == DATABASE) printf(" [d] Database\n");
    printf(" [t] Teams: %d\n", TEAMS_N);
    printf(" [p] Team size: %d\n", TEAM_SIZE);
    printf(" [s] Skills %d/%d\n", (int)selected_skills->n, (int)allSkills->n);
    printf(" [c] Combos\n");
    printf(" [o] Positions: %s\n", (ctx->use_positions) ? "TRUE" : "FALSE");
    printf(" [m] Comparison method: %s | %s\n",
           (ctx->compare == OV_AVERAGE) ? ">\033[4mOVERALL_AVG\033[0m<" : "OVERALL_AVG",
           (ctx->compare == SKILL_AVERAGE) ? ">\033[4mSKILL_AVG\033[0m<" : "SKILL_AVG");
    printf(" [q] Quit\n");

    printf("\n\033[31m%s\033[0m\n", error_msg);
    c = keyPress();
    error_msg[0] = '\0';
    switch (c) {
      case 'G': case 'g':
        if ((int)players->n != TEAMS_N * TEAM_SIZE) {
          sprintf(error_msg, "Selected %d players, but %d was expected", (int)players->n, TEAMS_N * TEAM_SIZE);
        } else if (ctx->use_positions && (int)ctx->positions->n != TEAM_SIZE) {
          sprintf(error_msg, "Amount of positions should equal team size (%d/%d)", (int)ctx->positions->n, TEAM_SIZE);
        } else {
          ctxUpdateDimensions(ctx, TEAMS_N, TEAM_SIZE);
          teams_added = generateTeams((tui) ? tui->db : NULL, players, ctx);
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
        freeSkills(selected_skills);
        cls(stdout);
        return;
      case 'D': case 'd':
        if (SOURCE == DATABASE) {
          if (teams_added) {
            updateAllTeams(tui);
          }
          updateTeamSize(tui, TEAMS_N, TEAM_SIZE);
          runTuiDB(tui);
          updatePlayerCombos(tui->db, players, ctx->banned_combos, ctx->pref_combos);
        }
        break;
      case 'S': case 's':
        runTuiSkills(tui->db, allSkills, selected_skills);
        break;
      case 'C': case 'c':
        runTuiCombo(tui->db, players);
        updatePlayerCombos(tui->db, players, ctx->banned_combos, ctx->pref_combos);
        break;
      case 'O': case 'o':
        // TODO: positions tui
        ctx->use_positions = runTuiPositions(tui->db, allPositions, selected_positions, ctx->use_positions);
        break;
      case 'M': case 'm':
        changeComparison(&ctx->compare);
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

  char* err_msg = malloc(1);
  err_msg[0] = '\0';
  char database[512];

  args* params = parseArgs(argc, argv);
  if (!params) {
    exit(0);
  }

  SOURCE = (params->dbName)     ? DATABASE
           : (params->fileName) ? TEXT_FILE
                                : NO_SOURCE;

  config* cfg = read_config();
  if (cfg && cfg->created) {
    char msg[600];
    snprintf(msg, sizeof(msg), "Config file created at '%s'\n", cfg->config_path);
    err_msg = realloc(err_msg, strlen(err_msg) + strlen(msg) + 1);
    strcat(err_msg, msg);
  }

  if (SOURCE == NO_SOURCE) {
    if (strcmp(cfg->db_path, "") != 0) {
      SOURCE = DATABASE;
      strcpy(database, cfg->db_path);
    } else {
      printUsage(stdout);
      exit(1);
    }
  } else if (SOURCE == DATABASE) {
    set_db_path(cfg, params->dbName);
    strcpy(database, params->dbName);
  }

  dlist* bannedCombos = init_list();
  dlist* prefCombos = init_list();
  dlist* skills = NULL;
  dlist* positions = NULL;
  sqldb* db = NULL;
  dlist* players = NULL;

  TEAMS_N = (params->teams > 0) ? params->teams : cfg->teams_n;
  TEAM_SIZE = (params->players > 0) ? params->players : cfg->team_size;
  PRINT_MODE = params->printMode;

  switch (SOURCE) {
    case DATABASE: {
      db = openSqlDB(database);
      if (!db->sqlite) {
        exit(1);
      }
      createDB(db);
      players = (params->fileName)
                    ? readPlayers(params->fileName, bannedCombos, prefCombos)
                    : NULL;
      if (!players) {
        players = fetchPlayerList(db);
      }
      if (bannedCombos->n == 0) {
        updateCombos(db, players, bannedCombos, BAN);
      } else {
        insertCombos(db, bannedCombos);
      }
      if (prefCombos->n == 0) {
        updateCombos(db, players, prefCombos, PAIR);
      } else {
        insertCombos(db, prefCombos);
      }
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
      skills = fetchSkills(db);
      positions = fetchPositions(db);
      break;
    }
    case TEXT_FILE: {
      players = readPlayers(params->fileName, bannedCombos, prefCombos);
      if (!players) {
        printf("File %s not found\n", params->fileName);
        exit(1);
      }
      break;
    }
    default:
      break;
  }

  qsort(players->items, players->n, sizeof(player*), cmpPlayers);

  tuidb* tui = NULL;
  if (SOURCE == DATABASE) {
    tui = initTuiDB(TEAMS_N, TEAM_SIZE);
    tui->db = db;
    setAllPlayers(tui, fetchPlayers(db));
    setAllTeams(tui, fetchTeams(db));
    tui->players = players;
  }

  context* ctx = makeContext();
  ctx->banned_combos = bannedCombos;
  ctx->pref_combos = prefCombos;
  ctx->skills = NULL;
  ctx->positions = NULL;
  ctxUpdateDimensions(ctx, TEAMS_N, TEAM_SIZE);

  curHide();
  altBufferEnable();
  runBeginTui(tui, players, ctx, skills, positions, err_msg);
  altBufferDisable();
  curShow();

  cfg->team_size = TEAM_SIZE;
  cfg->teams_n = TEAMS_N;
  write_config(cfg);
  free(cfg);

  if (skills) freeSkills(skills);
  if (positions) {
    for (size_t i = 0; i < positions->n; i++) {
      freePosition(positions->items[i]);
    }
    free_list(positions);
  }

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
  freeContext(ctx);

  return 0;
}


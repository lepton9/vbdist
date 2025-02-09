#include "../include/sql.h"
#include "../include/log.h"
#include <assert.h>
#include <limits.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>


sqldb* openSqlDB(const char* path) {
  sqldb* db = malloc(sizeof(sqldb));
  db->path = strdup(path);
  int r = sqlite3_open(db->path, &db->sqlite);
  if (r != SQLITE_OK) {
    log_sql_error("Failed to open db (%s)", sqlite3_errmsg(db->sqlite));
    db->sqlite = NULL;
  }
  enableForeignKey(db);
  makePlayerList(db);
  log_sql("Opened database '%s'", path);
  return db;
}

void closeSqlDB(sqldb* db) {
  if (!db) return;
  sqlite3_close(db->sqlite);
  log_sql("Closed database '%s'", db->path);
  free(db->path);
  free(db);
}

int cb_teams(void* tList, int count, char **data, char **columns) {
  assert(count == 2);
  dlist* list = tList;
  team* t = initTeam(data[1], 0);
  t->id = atoi(data[0]);
  list_add(list, t);
  return 0;
}

int cb_players(void* pList, int count, char **data, char **columns) {
  assert(count == 3);
  dlist* list = pList;
  player* p = initPlayer();
  p->id = atoi(data[0]);
  p->ratings_id = atoi(data[1]);
  p->firstName = strdup(data[2]);
  list_add(list, p);
  return 0;
}

int cb_player(void* p, int argc, char **argv, char **colName) {
  player* player = p;
  assert(argc == 2);
  player->firstName = strdup(argv[0]);
  player->ratings_id = atoi(argv[1]);
  player->found = 1;
  return 0;
}

int cb_rating(void* p, int argc, char **argv, char **colName) {
  player* player = p;
  for (int i = 1; i < argc; i++) {
    player->ratings[i - 1] = atof(argv[i]);
  }
  return 0;
}

int cb_teammates(void* teammates, int count, char **data, char **columns) {
  assert(count == 2);
  int_tuple* tuple = malloc(sizeof(int_tuple));
  tuple->a = atoi(data[0]);
  tuple->b = atoi(data[1]);
  list_add(teammates, tuple);
  return 0;
}

int cb_not_teammates(void* not_teammates, int count, char **data, char **columns) {
  assert(count == 1);
  int* id = malloc(sizeof(int));
  *id = atoi(data[0]);
  list_add(not_teammates, id);
  return 0;
}

int cb_add_id_list(void* list, int count, char **data, char **columns) {
  assert(count == 1);
  int* id = malloc(sizeof(int));
  *id = atoi(data[0]);
  list_add(list, id);
  return 0;
}

int cb_combos(void* combos, int count, char **data, char **columns) {
  assert(count == 2);
  dlist* cs = combos;
  pCombo* c = malloc(sizeof(pCombo));
  c->combo_id = atoi(data[0]);
  c->type = toComboType(data[1]);
  c->pidA = -1;
  c->pidB = -1;
  list_add(cs, c);
  return 0;
}

int cb_combo(void* combo, int count, char **data, char **columns) {
  assert(count == 1);
  pCombo* c = combo;
  if (c->pidA < 0) c->pidA = atoi(data[0]);
  else if (c->pidB < 0) c->pidB = atoi(data[0]);
  return 0;
}

int cb_found(void* flag, int count, char **data, char **columns) {
  assert(count == 1);
  int* f = flag;
  *f = 1;
  return 0;
}

int execQuery(sqlite3* db, const char* sql, int (*cb)(void *, int, char **, char **), void* p) {
  char* err_msg = NULL;
  int result = sqlite3_exec(db, sql, cb, p, &err_msg);
  if (err_msg) {
    log_sql_error("%s", err_msg);
    sqlite3_free(err_msg);
  }
  return result == SQLITE_OK;
}

int enableForeignKey(sqldb* db) {
  char* sql = "PRAGMA foreign_keys = ON;";
  return execQuery(db->sqlite, sql, 0, 0);
}

int makePlayerList(sqldb* db) {
  char* sql = "INSERT INTO PlayerList (playerlist_id, name) "
              "SELECT 1, 'Generate teams list' "
              "WHERE NOT EXISTS (SELECT 1 FROM PlayerList WHERE playerlist_id = 1);";
  return execQuery(db->sqlite, sql, 0, 0);
}

int clearPlayerList(sqldb* db) {
  char* sql = "DELETE FROM InPlayerList;";
  return execQuery(db->sqlite, sql, 0, 0);
}

int insertToPlayerList(sqldb* db, player* p) {
  char sql[150];
  sprintf(sql, "INSERT OR IGNORE INTO InPlayerList (playerlist_id, player_id) VALUES (%d, %d);", 1, p->id);
  return execQuery(db->sqlite, sql, 0, 0);
}

int saveToPlayerList(sqldb* db, dlist* players) {
  int r = 0;
  clearPlayerList(db);
  for (int i = 0; i < (int)players->n; i++) {
    r += insertToPlayerList(db, players->items[i]);
  }
  log_sql("Saved player list with %d players", r);
  return r;
}

dlist* fetchPlayerList(sqldb* db) {
  char* sql = "SELECT * FROM Player WHERE player_id IN (SELECT player_id FROM InPlayerList WHERE playerlist_id = 1);";
  dlist* list = init_list(sizeof(player*));
  execQuery(db->sqlite, sql, cb_players, list);
  for (int i = 0; i < (int)list->n; i++) {
    fetchRating(db, list->items[i]);
  }
  log_sql("Found player list with %d players", list->n);
  return list;
}

int insertCombo(sqldb* db, pCombo* combo) {
  if (comboExists(db, combo)) {
    return 0;
  }
  char sql[100];
  sprintf(sql, "INSERT INTO Combo (combo_type) VALUES ('%s');",
          comboTypeString(combo->type));
  if (execQuery(db->sqlite, sql, NULL, NULL)) {
    int combo_id = sqlite3_last_insert_rowid(db->sqlite);
    sprintf(
        sql,
        "INSERT INTO InCombo (combo_id, player_id) VALUES (%d, %d), (%d, %d);",
        combo_id, combo->pidA, combo_id, combo->pidB);
    if (execQuery(db->sqlite, sql, NULL, NULL)) {
      log_sql("Inserted Combo %s (%d, %d)", comboTypeString(combo->type), combo->pidA, combo->pidB);
    }
  }
  return 0;
}

int insertCombos(sqldb* db, dlist* combos) {
  int r = 0;
  for (int i = 0; i < (int)combos->n; i++) {
    r += insertCombo(db, combos->items[i]);
  }
  return r;
}

int comboExists(sqldb* db, pCombo* combo) {
  char sql[300];
  sprintf(sql, "SELECT player_id FROM InCombo WHERE combo_id = %d;", combo->combo_id);
  sprintf(sql,
          "SELECT combo_id FROM Combo "
          "WHERE combo_type = '%s' "
          "AND combo_id IN ("
          "    SELECT combo_id FROM InCombo "
          "    WHERE player_id IN (%d, %d) "
          "    GROUP BY combo_id "
          "    HAVING COUNT(DISTINCT player_id) = 2"
          ");",
          comboTypeString(combo->type), combo->pidA, combo->pidB);
  int found = 0;
  execQuery(db->sqlite, sql, cb_found, &found);
  return found;
}

int fetchCombo(sqldb* db, pCombo* combo) {
  char sql[100];
  sprintf(sql, "SELECT player_id FROM InCombo WHERE combo_id = %d;", combo->combo_id);
  return execQuery(db->sqlite, sql, cb_combo, combo);
}

dlist* fetchCombos(sqldb* db, comboType type) {
  dlist* combos = init_list(sizeof(pCombo*));
  char sql[100];
  sprintf(sql, "SELECT combo_id, combo_type FROM Combo WHERE combo_type = '%s';", comboTypeString(type));
  execQuery(db->sqlite, sql, cb_combos, combos);
  for (int i = 0; i < (int)combos->n; i++) {
    fetchCombo(db, combos->items[i]);
  }
  return combos;
}

dlist* fetchPlayers(sqldb* db) {
  char* sql = "SELECT * FROM Player;";
  dlist* list = init_list(sizeof(player*));
  execQuery(db->sqlite, sql, cb_players, list);
  for (int i = 0; i < (int)list->n; i++) {
    fetchRating(db, list->items[i]);
  }
  return list;
}

dlist* fetchTeams(sqldb* db) {
  char* sql = "SELECT team_id, name FROM Team ORDER BY team_id DESC;";
  dlist* list = init_list(sizeof(team*));
  execQuery(db->sqlite, sql, cb_teams, list);
  return list;
}

int fetchRating(sqldb* db, player* player) {
  char sql_rating[100];
  sprintf(sql_rating, "SELECT * FROM Rating WHERE rating_id = %d;", player->ratings_id);
  int result = execQuery(db->sqlite, sql_rating, cb_rating, player);
  return result;
}

int fetchPlayer(sqldb* db, player* player) {
  char sql[100];
  sprintf(sql, "SELECT name, rating_id FROM Player WHERE player_id = %d;", player->id);
  execQuery(db->sqlite, sql, cb_player, player);
  fetchRating(db, player);
  return player->found;
}

dlist* fetchPlayerTeams(sqldb* db, player* player) {
  char sql[100];
  sprintf(sql, "SELECT team_id FROM PlayerTeam WHERE player_id = %d;", player->id);
  dlist* teams = init_list(sizeof(int*));
  execQuery(db->sqlite, sql, cb_add_id_list, teams);
  return teams;
}

dlist* fetchFormerTeammates(sqldb* db, player* player) {
  dlist* teammates = init_list(sizeof(int_tuple*));
  char sql[200];
  sprintf(sql,
          "SELECT player_id, COUNT(player_id) as teammate_count FROM "
          "PlayerTeam WHERE team_id IN (SELECT team_id FROM PlayerTeam WHERE "
          "player_id = %d) GROUP BY player_id ORDER BY teammate_count DESC;",
          player->id);
  execQuery(db->sqlite, sql, cb_teammates, teammates);
  return teammates;
}

dlist* fetchNotTeammates(sqldb* db, player* player) {
  dlist* no_teammates = init_list(sizeof(int*));
  char sql[200];
  sprintf(sql,
          "SELECT player_id FROM Player WHERE player_id NOT IN (SELECT player_id FROM "
          "PlayerTeam WHERE team_id IN (SELECT team_id FROM PlayerTeam WHERE "
          "player_id = %d) GROUP BY player_id);",
          player->id);
  execQuery(db->sqlite, sql, cb_not_teammates, no_teammates);
  return no_teammates;
}

dlist* fetchPlayersInTeam(sqldb* db, team* team) {
  char sql[100];
  sprintf(sql, "SELECT player_id FROM PlayerTeam WHERE team_id = %d;", team->id);
  dlist* players = init_list(sizeof(int*));
  execQuery(db->sqlite, sql, cb_add_id_list, players);
  return players;
}

int renamePlayer(sqldb* db, player* player, const char* name) {
  char sql[100];
  sprintf(sql, "UPDATE Player SET name = '%s' WHERE player_id = %d;", name, player->id);
  int r = execQuery(db->sqlite, sql, 0, 0);
  if (r) {
    log_sql("Renamed Player (%d) '%s' -> '%s'", player->id, player->firstName, name);
  }
  return r;
}

int renameTeam(sqldb* db, team* team, const char* name) {
  char sql[100];
  sprintf(sql, "UPDATE Team SET name = '%s' WHERE team_id = %d;", name, team->id);
  int r = execQuery(db->sqlite, sql, 0, 0);
  if (r) {
    log_sql("Renamed Team (%d) '%s' -> '%s'", team->id, team->name, name);
  }
  return r;
}

int deletePlayer(sqldb* db, player* player) {
  char sql[100];
  sprintf(sql, "DELETE FROM Player WHERE player_id = %d;", player->id);
  int r = execQuery(db->sqlite, sql, 0, 0);
  if (r) {
    log_sql("Deleted Player (%d) '%s'", player->id, player->firstName);
  }
  return r;
}

int deleteTeam(sqldb* db, team* team) {
  char sql[100];
  sprintf(sql, "DELETE FROM Team WHERE team_id = %d;", team->id);
  int r = execQuery(db->sqlite, sql, 0, 0);
  if (r) {
    log_sql("Deleted Team (%d) '%s'", team->id, team->name);
  }
  return r;
}

int insertTeam(sqldb* db, team* team) {
  char sql[100];
  sprintf(sql, "INSERT INTO Team (name) VALUES ('%s');", team->name);
  int r = execQuery(db->sqlite, sql, 0, 0);
  int id = sqlite3_last_insert_rowid(db->sqlite);
  team->id = id;
  if (r) {
    log_sql("Inserted Team (%d) '%s'", team->id, team->name);
  }
  return r;
}

int insertPlayerTeam(sqldb* db, player* player, team* team) {
  char sql[100];
  sprintf(sql, "INSERT INTO PlayerTeam (player_id, team_id) VALUES (%d, %d);", player->id, team->id);
  int r = execQuery(db->sqlite, sql, 0, 0);
  if (r) {
    log_sql("Inserted Player (%d) '%s' to Team (%d) '%s'", player->id, player->firstName, team->id, team->name);
  }
  return r;
}

int randintRange(const int min, const int max) {
  return rand() % (max + 1 - min) + min;
}

int createDB(sqldb* db) {
  const char *sql =
      "CREATE TABLE IF NOT EXISTS Player ("
      "  player_id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  rating_id INTEGER NOT NULL,"
      "  name TEXT,"
      "  FOREIGN KEY (rating_id) REFERENCES Rating (rating_id) ON DELETE RESTRICT"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS Rating ("
      "  rating_id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  defence REAL NOT NULL,"
      "  spike REAL NOT NULL,"
      "  serve REAL NOT NULL,"
      "  setting REAL NOT NULL,"
      "  saving REAL NOT NULL,"
      "  consistency REAL NOT NULL"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS PlayerTeam ("
      "  player_id INTEGER NOT NULL,"
      "  team_id INTEGER NOT NULL,"
      "  PRIMARY KEY (player_id, team_id),"
      "  FOREIGN KEY (player_id) REFERENCES Player (player_id) ON DELETE RESTRICT,"
      "  FOREIGN KEY (team_id) REFERENCES Team (team_id) ON DELETE CASCADE"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS Team ("
      "  team_id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  name TEXT"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS Position ("
      "  position_id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  name TEXT"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS PlayerPosition ("
      "  player_id INTEGER NOT NULL,"
      "  position_id INTEGER NOT NULL,"
      "  priority_value INTEGER NOT NULL,"
      "  PRIMARY KEY (player_id, position_id),"
      "  FOREIGN KEY (player_id) REFERENCES Player (player_id) ON DELETE CASCADE,"
      "  FOREIGN KEY (position_id) REFERENCES Position (position_id) ON DELETE CASCADE"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS Combo ("
      "  combo_id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  combo_type TEXT NOT NULL CHECK (combo_type IN ('BAN', 'PAIR'))"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS InCombo ("
      "  combo_id INTEGER NOT NULL,"
      "  player_id INTEGER NOT NULL,"
      "  PRIMARY KEY (combo_id, player_id),"
      "  FOREIGN KEY (player_id) REFERENCES Player (player_id) ON DELETE CASCADE,"
      "  FOREIGN KEY (combo_id) REFERENCES Combo (combo_id) ON DELETE CASCADE"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS PlayerList ("
      "  playerlist_id INTEGER PRIMARY KEY,"
      "  name TEXT"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS InPlayerList ("
      "  playerlist_id INTEGER NOT NULL,"
      "  player_id INTEGER NOT NULL,"
      "  PRIMARY KEY (playerlist_id, player_id),"
      "  FOREIGN KEY (player_id) REFERENCES Player (player_id) ON DELETE CASCADE,"
      "  FOREIGN KEY (playerlist_id) REFERENCES PlayerList (playerlist_id) ON DELETE CASCADE"
      ");";

  return execQuery(db->sqlite, sql, 0, 0);
}


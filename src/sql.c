#include "../include/sql.h"
#include <assert.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>


sqldb* openSqlDB(const char* path) {
  sqldb* db = malloc(sizeof(sqldb));
  db->path = strdup(path);
  int r = sqlite3_open(db->path, &db->sqlite);
  if (r != SQLITE_OK) {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db->sqlite));
    db->sqlite = NULL;
  }
  enableForeignKey(db);
  return db;
}

void closeSqlDB(sqldb* db) {
  if (!db) return;
  sqlite3_close(db->sqlite);
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

int execQuery(sqlite3* db, const char* sql, int (*cb)(void *, int, char **, char **), void* p) {
  char* err_msg = NULL;
  int result = sqlite3_exec(db, sql, cb, p, &err_msg);
  if (err_msg) {
    // TODO: proper logging
    FILE *log_file = fopen("log_vbdist.txt", "a");
    if (log_file) {
      time_t rawtime;
      struct tm *timeinfo;
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      fprintf(log_file, "%s | SQL error: %s\n", asctime(timeinfo), err_msg);
      fclose(log_file);
    }
    sqlite3_free(err_msg);
  }
  return result == SQLITE_OK;
}

int enableForeignKey(sqldb* db) {
  char* sql = "PRAGMA foreign_keys = ON;";
  return execQuery(db->sqlite, sql, 0, 0);
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
  char* sql = "SELECT team_id, name FROM Team;";
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
  int result = execQuery(db->sqlite, sql, 0, 0);
  return result;
}

int renameTeam(sqldb* db, team* team, const char* name) {
  char sql[100];
  sprintf(sql, "UPDATE Team SET name = '%s' WHERE team_id = %d;", name, team->id);
  int result = execQuery(db->sqlite, sql, 0, 0);
  return result;
}

int deletePlayer(sqldb* db, player* player) {
  char sql[100];
  sprintf(sql, "DELETE FROM Player WHERE player_id = %d;", player->id);
  return execQuery(db->sqlite, sql, 0, 0);
}

int deleteTeam(sqldb* db, team* team) {
  char sql[100];
  sprintf(sql, "DELETE FROM Team WHERE team_id = %d;", team->id);
  return execQuery(db->sqlite, sql, 0, 0);
}

void insertTeam(sqldb* db, team* team) {
  if (team->id < 0) team->id = randintRange(0, INT_MAX);
  char sql[100];
  sprintf(sql, "INSERT INTO Team (team_id, name) VALUES (%d, '%s');", team->id, team->name);
  if (execQuery(db->sqlite, sql, 0, 0)) {
    // TODO: maybe log to a file
  }
}

void insertPlayerTeam(sqldb* db, player* player, team* team) {
  char sql[100];
  sprintf(sql, "INSERT INTO PlayerTeam (player_id, team_id) VALUES (%d, %d);", player->id, team->id);
  if (execQuery(db->sqlite, sql, 0, 0)) {
    // TODO: log
  }
}

int randintRange(const int min, const int max) {
  return rand() % (max + 1 - min) + min;
}

int createDB(sqldb* db) {
  const char *sql =
      "CREATE TABLE IF NOT EXISTS Player ("
      "  player_id INTEGER NOT NULL UNIQUE,"
      "  rating_id INTEGER NOT NULL,"
      "  name TEXT,"
      "  PRIMARY KEY (player_id)"
      "  FOREIGN KEY (rating_id) REFERENCES Rating (rating_id) ON DELETE RESTRICT"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS Rating ("
      "  rating_id INTEGER NOT NULL UNIQUE,"
      "  defence REAL NOT NULL,"
      "  spike REAL NOT NULL,"
      "  serve REAL NOT NULL,"
      "  setting REAL NOT NULL,"
      "  saving REAL NOT NULL,"
      "  consistency REAL NOT NULL,"
      "  PRIMARY KEY (rating_id)"
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
      "  team_id INTEGER NOT NULL UNIQUE,"
      "  name TEXT,"
      "  PRIMARY KEY (team_id)"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS Position ("
      "position_id INTEGER NOT NULL,"
      "positionName TEXT,"
      "PRIMARY KEY (position_id),"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS PlayerPosition ("
      "player_id INTEGER NOT NULL,"
      "position_id INTEGER NOT NULL,"
      "priority_value INTEGER NOT NULL,"
      "PRIMARY KEY (player_id, position_id),"
      "FOREIGN KEY (player_id) REFERENCES Player (player_id) ON DELETE CASCADE,"
      "FOREIGN KEY (position_id) REFERENCES Position (position_id) ON DELETE CASCADE"
      ");";

  return execQuery(db->sqlite, sql, 0, 0);
}


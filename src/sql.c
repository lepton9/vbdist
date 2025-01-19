#include "../include/sql.h"
#include <assert.h>
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
  return db;
}

void closeSqlDB(sqldb* db) {
  sqlite3_close(db->sqlite);
  free(db->path);
  free(db);
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

int cb_player_teams(void* team_ids, int count, char **data, char **columns) {
  assert(count == 1);
  int* id = malloc(sizeof(int));
  *id = atoi(data[0]);
  list_add(team_ids, id);
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


int execSQL(sqlite3* db, const char* sql) {
  char* err_msg = NULL;
  int result = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
  if (err_msg) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  }
  return result == SQLITE_OK;
}

dlist* fetchPlayers(sqldb* db) {
  char* sql = "SELECT * FROM Player;";
  dlist* list = init_list(sizeof(player*));
  char* err_msg = NULL;
  int result = sqlite3_exec(db->sqlite, sql, cb_players, list, &err_msg);
  if (err_msg) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  }
  for (int i = 0; i < (int)list->n; i++) {
    fetchRating(db, list->items[i]);
  }
  return list;
}

int fetchRating(sqldb* db, player* player) {
  char sql_rating[100];
  sprintf(sql_rating, "SELECT * FROM Rating WHERE rating_id = %d;", player->ratings_id);
  char* err_msg = NULL;
  int result = sqlite3_exec(db->sqlite, sql_rating, cb_rating, player, &err_msg);
  if (err_msg) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  }
  return result;
}

int fetchPlayer(sqldb* db, player* player) {
  char sql[100];
  sprintf(sql, "SELECT name, rating_id FROM Player WHERE player_id = %d;", player->id);
  char* err_msg = NULL;
  int result = sqlite3_exec(db->sqlite, sql, cb_player, player, &err_msg);
  if (err_msg) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  }
  fetchRating(db, player);
  return player->found;
}

dlist* fetchPlayerTeams(sqldb* db, player* player) {
  char sql[100];
  sprintf(sql, "SELECT team_id FROM PlayerTeam WHERE player_id = %d;", player->id);
  dlist* teams = init_list(sizeof(int*));
  char* err_msg = NULL;
  int result = sqlite3_exec(db->sqlite, sql, cb_player_teams, teams, &err_msg);
  if (err_msg) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  }
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
  char* err_msg = NULL;
  int result = sqlite3_exec(db->sqlite, sql, cb_teammates, teammates, &err_msg);
  if (err_msg) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  }
  return teammates;
}

void insertTeam(sqldb* db, team* team) {
  if (team->id < 0) team->id = randintRange(0, INT_MAX);
  char sql[100];
  sprintf(sql, "INSERT INTO Team (team_id, name) VALUES (%d, '%s');", team->id, team->name);
  if (execSQL(db->sqlite, sql)) {
    // TODO: maybe log to a file
  }
}

void insertPlayerTeam(sqldb* db, player* player, team* team) {
  char sql[100];
  sprintf(sql, "INSERT INTO PlayerTeam (player_id, team_id) VALUES (%d, %d);", player->id, team->id);
  if (execSQL(db->sqlite, sql)) {
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
      "  FOREIGN KEY (player_id) REFERENCES Player (player_id) ON DELETE "
      "  RESTRICT,"
      "FOREIGN KEY (team_id) REFERENCES Team (team_id) ON DELETE RESTRICT"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS Team ("
      "  team_id INTEGER NOT NULL UNIQUE,"
      "  name TEXT,"
      "  PRIMARY KEY (team_id)"
      ");";

  return execSQL(db->sqlite, sql);
}


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

int cb_player(void* p, int argc, char **argv, char **colName) {
  player* player = p;
  assert(argc == 2);
  player->firstName = strdup(argv[0]);
  player->ratings_id = atoi(argv[1]);
  return 0;
}

int cb_rating(void* p, int argc, char **argv, char **colName) {
  player* player = p;
  for (int i = 1; i < argc; i++) {
    player->ratings[i - 1] = atof(argv[i]);
  }
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

void fetchPlayer(sqldb* db, player* player) {
  char sql[100];
  sprintf(sql, "SELECT name, rating_id FROM Player WHERE player_id = %d;", player->id);

  char* err_msg = NULL;
  int result = sqlite3_exec(db->sqlite, sql, cb_player, player, &err_msg);
  if (err_msg) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  }

  char sql_rating[100];
  sprintf(sql_rating, "SELECT * FROM Rating WHERE rating_id = %d;", player->ratings_id);

  result = sqlite3_exec(db->sqlite, sql_rating, cb_rating, player, &err_msg);
  if (err_msg) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  }
}

void insertTeam(sqldb* db, team* team) {
  if (team->id < 0) team->id = randintRange(0, INT_MAX);
  char sql[100];
  sprintf(sql, "INSERT INTO Team (team_id, name) VALUES (%d, '%s');", team->id, team->name);
  if (execSQL(db->sqlite, sql)) {
    printf("Team inserted\n");
  }
}

void insertPlayerTeam(sqldb* db, player* player, team* team) {
  char sql[100];
  sprintf(sql, "INSERT INTO PlayerTeam (player_id, team_id) VALUES (%d, %d);", player->id, team->id);
  if (execSQL(db->sqlite, sql)) {
    printf("PlayerTeam inserted\n");
  }
}

int randintRange(const int min, const int max) {
  return rand() % (max + 1 - min) + min;
}

void createDB(sqldb* db) {
  const char *sql =
      "CREATE TABLE Player ("
      "  player_id INTEGER NOT NULL UNIQUE,"
      "  rating_id INTEGER NOT NULL,"
      "  name TEXT,"
      "  PRIMARY KEY (player_id)"
      "  FOREIGN KEY (rating_id) REFERENCES Rating (rating_id) ON DELETE RESTRICT"
      ");"
      ""
      "CREATE TABLE Rating ("
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
      "CREATE TABLE PlayerTeam ("
      "  player_id INTEGER NOT NULL,"
      "  team_id INTEGER NOT NULL,"
      "  PRIMARY KEY (player_id, team_id),"
      "  FOREIGN KEY (player_id) REFERENCES Player (player_id) ON DELETE "
      "  RESTRICT,"
      "FOREIGN KEY (team_id) REFERENCES Team (team_id) ON DELETE RESTRICT"
      ");"
      ""
      "CREATE TABLE Team ("
      "  team_id INTEGER NOT NULL UNIQUE,"
      "  name TEXT,"
      "  PRIMARY KEY (team_id)"
      ");";

  printf("Creating sqlite3 database\n");

  if (execSQL(db->sqlite, sql)) {
    printf("Tables created successfully.\n");
  }
}


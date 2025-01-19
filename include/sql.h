#ifndef SQL_H
#define SQL_H

#include <stdio.h>
#include "sqlite3.h"
#include "../include/team.h"
#include "../include/dlist.h"

typedef struct {
  int a;
  int b;
} int_tuple;

typedef struct {
  sqlite3* sqlite;
  char* path;
} sqldb;

sqldb* openSqlDB(const char* path);
void closeSqlDB(sqldb* db);
int execSQL(sqlite3* db, const char* sql);

int createDB(sqldb* db);

int fetchPlayer(sqldb* db, player* player);
int fetchRating(sqldb* db, player* player);
dlist* fetchPlayers(sqldb* db);
dlist* fetchPlayerTeams(sqldb* db, player* player);
dlist* fetchFormerTeammates(sqldb* db, player* player);

void insertTeam(sqldb* db, team* team);
void insertPlayerTeam(sqldb* db, player* player, team* team);

int randintRange(const int min, const int max);

#endif

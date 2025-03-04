#ifndef SQL_H
#define SQL_H

#include <stdio.h>
#include "sqlite3.h"
#include "team.h"
#include "combo.h"
#include "dlist.h"

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

int execQuery(sqlite3 *db, const char *sql,
              int (*cb)(void *, int, char **, char **), void *p);

int enableForeignKey(sqldb* db);
int createDB(sqldb* db);

int makePlayerList(sqldb* db);
int clearPlayerList(sqldb* db);
int insertToPlayerList(sqldb* db, player* p);
int saveToPlayerList(sqldb* db, dlist* players);
dlist* fetchPlayerList(sqldb* db);

int fetchPlayer(sqldb* db, player* player);
int fetchPlayerSkills(sqldb* db, player* player);
dlist* fetchPlayers(sqldb* db);
dlist* fetchTeams(sqldb* db);
dlist* fetchPlayerTeams(sqldb* db, player* player);
dlist* fetchFormerTeammates(sqldb* db, player* player);
dlist* fetchNotTeammates(sqldb* db, player* player);
dlist* fetchPlayersInTeam(sqldb* db, team* team);

int insertCombo(sqldb* db, pCombo* combo);
int insertCombos(sqldb* db, dlist* combos);
int fetchCombo(sqldb* db, pCombo* combo);
dlist* fetchCombos(sqldb* db, comboType type);
int comboExists(sqldb* db, pCombo* combo);

dlist* fetchSkills(sqldb* db);
int insertSkill(sqldb* db, skill* skill);
int deleteSkill(sqldb* db, skill* skill);
int renameSkill(sqldb* db, skill* skill, const char* name);

int renamePlayer(sqldb* db, player* player, const char* name);
int renameTeam(sqldb* db, team* team, const char* name);
int deletePlayer(sqldb* db, player* player);
int deleteTeam(sqldb* db, team* team);

int insertTeam(sqldb* db, team* team);
int insertPlayerTeam(sqldb* db, player* player, team* team);

int randintRange(const int min, const int max);

#endif

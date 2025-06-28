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
int stmtExec(sqlite3* db, sqlite3_stmt* stmt);
int sqlPrepare(sqlite3* db, sqlite3_stmt** stmt, const char* sql);

int enableForeignKey(sqldb* db);
int createDB(sqldb* db);

int makePlayerList(sqldb* db);
int clearPlayerList(sqldb* db);
int saveToPlayerList(sqldb* db, dlist* players);
dlist* fetchPlayerList(sqldb* db);

int fetchPlayer(sqldb* db, player* player);
int fetchPlayerSkills(sqldb* db, player* player);
int fetchPlayerPositions(sqldb* db, player* player);
dlist* fetchPlayers(sqldb* db);
dlist* fetchTeams(sqldb* db);
dlist* fetchPlayerTeams(sqldb* db, player* player);
dlist* fetchFormerTeammates(sqldb* db, player* player);
dlist* fetchNotTeammates(sqldb* db, player* player);
dlist* fetchPlayersInTeam(sqldb* db, team* team);

void removeEmptyCombos(dlist* combos);
int insertCombo(sqldb* db, combo* combo);
int insertCombos(sqldb* db, dlist* combos);
int fetchCombo(sqldb* db, combo* combo);
dlist* fetchCombos(sqldb* db, comboType type);
dlist* fetchAllCombos(sqldb* db);
int deleteCombo(sqldb* db, combo* combo);
int updateCombo(sqldb* db, combo* combo);

dlist* fetchPositions(sqldb* db);
dlist* fetchSkills(sqldb* db);
int insertSkill(sqldb* db, skill* skill);
int deleteSkill(sqldb* db, skill* skill);
int renameSkill(sqldb* db, skill* skill, const char* name);
int updateSkillWeight(sqldb* db, skill* skill);
int updateSkillWeights(sqldb* db, dlist* skills);

int renamePlayer(sqldb* db, player* player, const char* name);
int renameTeam(sqldb* db, team* team, const char* name);
int deletePlayer(sqldb* db, player* player);
int deleteTeam(sqldb* db, team* team);

int insertTeam(sqldb* db, team* team);
int insertPlayerTeam(sqldb* db, player* player, team* team);

#endif

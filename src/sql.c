#include "../include/sql.h"
#include "../include/log.h"
#include <assert.h>
#include <limits.h>
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
  if (db->sqlite) sqlite3_close(db->sqlite);
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

int cb_players(void* list, int count, char **data, char **columns) {
  assert(count == 2);
  player* p = initPlayer();
  p->id = atoi(data[0]);
  p->firstName = strdup(data[1]);
  list_add(list, p);
  return 0;
}

int cb_player(void* p, int argc, char **argv, char **colName) {
  player* player = p;
  assert(argc == 1);
  if (!player->firstName) {
    player->firstName = strdup(argv[0]);
  }
  player->found = 1;
  return 0;
}

int cb_positions(void* list, int count, char **data, char **columns) {
  assert(count == 2);
  int id = atoi(data[0]);
  const char* name = data[1];
  position* p = initPosition(id, name);
  list_add(list, p);
  return 0;
}

int cb_player_position(void* positions, int count, char **data, char **columns) {
  assert(count == 3);
  int id = atoi(data[0]);
  const char* name = data[1];
  int priority = atoi(data[2]);
  position* p = initPosition(id, name);
  setPriority(p, priority);
  list_add(positions, p);
  return 0;
}

int cb_player_skill(void* skills, int count, char **data, char **columns) {
  assert(count == 4);
  int id = atoi(data[0]);
  float value = atof(data[1]);
  const char* name = data[2];
  skill* s = initSkill(id, name, value);
  setWeight(s, atof(data[3]));
  list_add(skills, s);
  return 0;
}

int cb_skill(void* skills, int count, char **data, char **columns) {
  assert(count == 3);
  int id = atoi(data[0]);
  const char* name = data[1];
  skill* s = initSkill(id, name, 0);
  setWeight(s, atof(data[2]));
  list_add(skills, s);
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
  combo* c = initCombo(toComboType(data[1]), atoi(data[0]));
  list_add(combos, c);
  return 0;
}

int cb_combo(void* combo, int count, char **data, char **columns) {
  assert(count == 1);
  addToCombo(combo, atoi(data[0]));
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

int sqlPrepare(sqlite3* db, sqlite3_stmt** stmt, const char* sql) {
  int result = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);
  if (result != SQLITE_OK) {
    log_sql_error("%s", sqlite3_errmsg(db));
    return 0;
  }
  return 1;
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
  char* sql = "SELECT player_id, name FROM Player WHERE player_id IN (SELECT player_id FROM InPlayerList WHERE playerlist_id = 1);";
  dlist* list = init_list();
  execQuery(db->sqlite, sql, cb_players, list);
  log_sql("Found player list with %d players", list->n);
  return list;
}

int insertInCombo(sqldb* db, int combo_id, dlist* ids) {
  char sql[200];
  sprintf(sql, "INSERT INTO InCombo (combo_id, player_id) VALUES ");
  for (size_t i = 0; i < ids->n; i++) {
    if (i > 0) strcat(sql, ",");
    char sql_combo[20];
    int* id = ids->items[i];
    sprintf(sql_combo, "(%d, %d)", combo_id, *id);
    strcat(sql, sql_combo);
  }
  strcat(sql, ";");
  return execQuery(db->sqlite, sql, NULL, NULL);
}

int insertCombo(sqldb* db, combo* combo) {
  if (combo->combo_id >= 0) {
    return 0;
  }
  char sql[200];
  sprintf(sql, "INSERT INTO Combo (combo_type) VALUES ('%s');",
          comboTypeString(combo->type));
  if (execQuery(db->sqlite, sql, NULL, NULL)) {
    int combo_id = sqlite3_last_insert_rowid(db->sqlite);
    if (insertInCombo(db, combo_id, combo->ids)) {
      log_sql("Inserted Combo %s (%d players)", comboTypeString(combo->type), (int)combo->ids->n);
      combo->combo_id = combo_id;
      return 1;
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

int updateCombo(sqldb* db, combo* combo) {
  if (combo->combo_id < 0) return 0;
  char sql[200];
  sprintf(sql, "DELETE FROM InCombo WHERE combo_id = %d;", combo->combo_id);
  if (execQuery(db->sqlite, sql, NULL, NULL)) {
    if (insertInCombo(db, combo->combo_id, combo->ids)) {
      log_sql("Updated Combo %s (%d players)", comboTypeString(combo->type), (int)combo->ids->n);
      return 1;
    }
  }
  return 0;
}

int fetchCombo(sqldb* db, combo* combo) {
  char sql[100];
  sprintf(sql, "SELECT player_id FROM InCombo WHERE combo_id = %d;", combo->combo_id);
  return execQuery(db->sqlite, sql, cb_combo, combo);
}

void removeEmptyCombos(dlist* combos) {
  for (int i = (int)combos->n - 1; i >= 0; i--) {
    combo* c = combos->items[i];
    if (c->ids->n < 2) {
      freeCombo(pop_elem(combos, i));
    }
  }
}

dlist* fetchCombos(sqldb* db, comboType type) {
  dlist* combos = init_list();
  char sql[100];
  sprintf(sql, "SELECT combo_id, combo_type FROM Combo WHERE combo_type = '%s';", comboTypeString(type));
  execQuery(db->sqlite, sql, cb_combos, combos);
  for (size_t i = 0; i < combos->n; i++) {
    fetchCombo(db, combos->items[i]);
  }
  removeEmptyCombos(combos);
  return combos;
}

dlist* fetchAllCombos(sqldb* db) {
  dlist* combos = init_list();
  char sql[100];
  sprintf(sql, "SELECT combo_id, combo_type FROM Combo;");
  execQuery(db->sqlite, sql, cb_combos, combos);
  for (int i = 0; i < (int)combos->n; i++) {
    fetchCombo(db, combos->items[i]);
  }
  removeEmptyCombos(combos);
  return combos;
}

int deleteCombo(sqldb* db, combo* combo) {
  char sql[100];
  sprintf(sql, "DELETE FROM Combo WHERE combo_id = %d;", combo->combo_id);
  int r = execQuery(db->sqlite, sql, NULL, NULL);
  if (r) {
    log_sql("Deleted Combo (%d)", combo->combo_id);
  }
  return r;
}

dlist* fetchPlayers(sqldb* db) {
  char* sql = "SELECT player_id, name FROM Player ORDER BY name ASC;";
  dlist* list = init_list();
  execQuery(db->sqlite, sql, cb_players, list);
  for (int i = 0; i < (int)list->n; i++) {
    fetchPlayerSkills(db, list->items[i]);
    fetchPlayerPositions(db, list->items[i]);
  }
  return list;
}

dlist* fetchTeams(sqldb* db) {
  char* sql = "SELECT team_id, name FROM Team ORDER BY team_id DESC;";
  dlist* list = init_list();
  execQuery(db->sqlite, sql, cb_teams, list);
  return list;
}

dlist* fetchPositions(sqldb* db) {
  char* sql = "SELECT position_id, name FROM Position;";
  dlist* list = init_list();
  execQuery(db->sqlite, sql, cb_positions, list);
  return list;
}

int fetchPlayerPositions(sqldb* db, player* player) {
  char sql[200];
  sprintf(sql,
          "SELECT p.position_id, p.name, pp.priority_value FROM PlayerPosition pp INNER JOIN "
          "Position p ON pp.player_id = %d AND pp.position_id = p.position_id ORDER BY pp.priority_value ASC;",
          player->id);
  int result = execQuery(db->sqlite, sql, cb_player_position, player->positions);
  return result;
}

int fetchPlayerSkills(sqldb* db, player* player) {
  char sql[200];
  sprintf(sql,
          "SELECT ps.skill_id, ps.value, s.name, s.weight FROM PlayerSkill ps INNER JOIN "
          "Skill s ON ps.player_id = %d AND ps.skill_id = s.skill_id;",
          player->id);
  int result = execQuery(db->sqlite, sql, cb_player_skill, player->skills);
  return result;
}

dlist* fetchSkills(sqldb* db) {
  char sql[200];
  dlist* skills = init_list();
  sprintf(sql, "SELECT skill_id, name, weight FROM Skill;");
  execQuery(db->sqlite, sql, cb_skill, skills);
  return skills;
}


int insertSkill(sqldb* db, skill* skill) {
  char sql[100];
  sprintf(sql, "INSERT INTO Skill (name, weight) VALUES ('%s', %f);", skill->name, skill->weight);
  int r = execQuery(db->sqlite, sql, NULL, NULL);
  if (r) {
    int skill_id = sqlite3_last_insert_rowid(db->sqlite);
    skill->id = skill_id;
    log_sql("Created new Skill (%d) '%s'", skill->id, skill->name);
  }
  return r;
}

int deleteSkill(sqldb* db, skill* skill) {
  char sql[100];
  sprintf(sql, "DELETE FROM Skill WHERE skill_id = %d;", skill->id);
  int r = execQuery(db->sqlite, sql, NULL, NULL);
  if (r) {
    log_sql("Deleted Skill (%d) '%s'", skill->id, skill->name);
  }
  return r;
}

int renameSkill(sqldb* db, skill* skill, const char* name) {
  char sql[100];
  sprintf(sql, "UPDATE Skill SET name = '%s' WHERE skill_id = %d;", name, skill->id);
  int r = execQuery(db->sqlite, sql, 0, 0);
  if (r) {
    log_sql("Renamed Skill (%d) '%s' -> '%s'", skill->id, skill->name, name);
  }
  return r;
}

int updateSkillWeight(sqldb* db, skill* skill) {
  char sql[100];
  sprintf(sql, "UPDATE Skill SET weight = %f WHERE skill_id = %d;", skill->weight, skill->id);
  int r = execQuery(db->sqlite, sql, 0, 0);
  if (r) {
    log_sql("Updated weight Skill (%d) '%s' '%.2f'", skill->id, skill->name, skill->weight);
  }
  return r;
}

int updateSkillWeights(sqldb* db, dlist* skills) {
  int r = 1;
  for (size_t i = 0; i < skills->n; i++) {
    r = r && updateSkillWeight(db, skills->items[i]);
  }
  return r;
}

int fetchPlayer(sqldb* db, player* player) {
  char sql[100];
  sprintf(sql, "SELECT name FROM Player WHERE player_id = %d;", player->id);
  execQuery(db->sqlite, sql, cb_player, player);
  if (player->found) {
    fetchPlayerSkills(db, player);
    fetchPlayerPositions(db, player);
  }
  return player->found;
}

dlist* fetchPlayerTeams(sqldb* db, player* player) {
  char sql[100];
  sprintf(sql, "SELECT team_id FROM PlayerTeam WHERE player_id = %d;", player->id);
  dlist* teams = init_list();
  execQuery(db->sqlite, sql, cb_add_id_list, teams);
  return teams;
}

dlist* fetchFormerTeammates(sqldb* db, player* player) {
  dlist* teammates = init_list();
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
  dlist* no_teammates = init_list();
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
  dlist* players = init_list();
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

int createDB(sqldb* db) {
  const char *sql =
      "CREATE TABLE IF NOT EXISTS Player ("
      "  player_id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  name TEXT"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS Skill ("
      "  skill_id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  name TEXT,"
      "  weight REAL DEFAULT 1.0"
      ");"
      ""
      "CREATE TABLE IF NOT EXISTS PlayerSkill ("
      "  player_id INTEGER NOT NULL,"
      "  skill_id INTEGER NOT NULL,"
      "  value REAL DEFAULT 0,"
      "  PRIMARY KEY (player_id, skill_id),"
      "  FOREIGN KEY (player_id) REFERENCES Player (player_id) ON DELETE CASCADE,"
      "  FOREIGN KEY (skill_id) REFERENCES Skill (skill_id) ON DELETE CASCADE"
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


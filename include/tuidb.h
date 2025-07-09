#ifndef TUIDB_H
#define TUIDB_H

#include "render.h"
#include "sql.h"
#include "tui.h"
#include "playeredit.h"

#define BASE_LIST_LEN 50
#define BASE_SECTION_WIDTH 40
#define AREA_SPACING 3

typedef enum {
  PLAYERS_TAB,
  TEAMS_TAB
} TuiTab;

typedef enum {
  PLAYERS_LIST,
  PLAYER_EDIT,
  POSITIONS_LIST_EDIT
} layout_area;

typedef struct {
  sqldb* db;
  dlist* players;
  dlist* allPlayers;
  dlist* allTeams;
  dlist* allPositions;
  list_area* allPlayersArea;
  list_area* allTeamsArea;
  term_size* term;
  renderer* render;
  int teams_n;
  int team_size;
  TuiTab tab;
  layout_area active_area;
  char show_player_info;
  player_edit* p_edit;
  char exit;
} tuidb;


tuidb* initTuiDB(int teams, int team_size);
void freeTuiDB(tuidb* tui);

void updateAllTeams(tuidb* tui);
void updateTeamSize(tuidb* tui, int team_n, int team_size);

void runTuiDB(tuidb* tui);

void selectCurPlayer(tuidb* tui);
void unselectCurPlayer(tuidb* tui);
void unselectPlayer(tuidb* tui, int index);
void unselect_all(tuidb* tui);

void setAllPlayers(tuidb* tui, dlist* players);
void setAllTeams(tuidb* tui, dlist* teams);
void setAllPositions(tuidb* tui, dlist* positions);

player* selectedPlayer(tuidb* tui);
team* selectedTeam(tuidb* tui);

void renameSelectedListElem(tuidb* tui);
void deleteSelectedListElem(tuidb* tui);

void tuidb_list_up(tuidb* tui);
void tuidb_list_down(tuidb* tui);

void toggle_edit_player(tuidb* tui);
void exit_edit_player(tuidb* tui);

void pedit_add(tuidb* tui);
void pedit_remove(tuidb* tui);
int playerInfoBoxHeight(tuidb* tui, player* p);

void updateArea(tuidb* tui);
void renderTuidb(tuidb* tui);
void renderAllPlayersList(tuidb* tui);
void renderSelectedList(tuidb* tui);
void renderPlayerInfo(tuidb* tui);
void renderPlayerEditPos(tuidb* tui);
void renderPlayerRelations(tuidb* tui, player* p, int startCol, int startLine);
void renderAllTeamsList(tuidb* tui);
void renderSelectedTeam(tuidb* tui);

void handle_esc(tuidb* tui);
void handleAdd(tuidb* tui);
void handleRemove(tuidb* tui);
void handleExit(tuidb* tui);
void handleKeyPress(tuidb* tui, int c);

#endif

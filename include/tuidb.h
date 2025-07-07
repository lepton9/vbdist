#ifndef TUIDB_H
#define TUIDB_H

#include "render.h"
#include "sql.h"
#include "player.h"
#include "tui.h"
#include "listarea.h"

#define BASE_LIST_LEN 50
#define BASE_SECTION_WIDTH 50

typedef enum {
  PLAYERS_TAB,
  TEAMS_TAB
} TuiTab;

typedef enum {
  PLAYERS_LIST,
  PLAYER_EDIT
} layout_area;

typedef enum {
  SKILLS_LIST,
  POSITIONS_LIST,
  NO_ELEMENT
} p_edit_element;

typedef struct {
  char active;
  char modified;
  p_edit_element selected_element;
  int lists_index;
  player* p;
} player_edit;

typedef struct {
  sqldb* db;
  dlist* players;
  dlist* allPlayers;
  dlist* allTeams;
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

player* selectedPlayer(tuidb* tui);
team* selectedTeam(tuidb* tui);

void renameSelectedListElem(tuidb* tui);
void deleteSelectedListElem(tuidb* tui);

void tuidb_list_up(tuidb* tui);
void tuidb_list_down(tuidb* tui);

void toggle_edit_player(tuidb* tui);
void exit_edit_player(tuidb* tui);
void pedit_list_up(tuidb* tui);
void pedit_list_down(tuidb* tui);
skill* pedit_selected_skill(tuidb* tui);
position* pedit_selected_position(tuidb* tui);

void updateArea(tuidb* tui);
void renderTuidb(tuidb* tui);
void renderAllPlayersList(tuidb* tui);
void renderSelectedList(tuidb* tui);
void renderPlayerInfo(tuidb* tui);
void renderAllTeamsList(tuidb* tui);
void renderSelectedTeam(tuidb* tui);

void handleKeyPress(tuidb* tui, int c);

#endif

#ifndef TUIDB_H
#define TUIDB_H

#include "sql.h"
#include "player.h"
#include "tui.h"

#define BASE_LIST_LEN 50
#define BASE_SECTION_WIDTH 50

typedef enum {
  PLAYERS_TAB,
  TEAMS_TAB
} TuiTab;

typedef struct {
  size_t maxShown;
  size_t width;
  int firstInd;
  int selected;
  int selected_term_row;
} listArea;

typedef struct {
  sqldb* db;
  dlist* players;
  dlist* allPlayers;
  dlist* allTeams;
  listArea* allPlayersArea;
  listArea* allTeamsArea;
  term_size* term;
  int teams_n;
  int team_size;
  TuiTab tab;
  int show_player_info;
} tuidb;


tuidb* initTuiDB(int teams, int team_size);
void freeTuiDB(tuidb* tui);

listArea* initListArea();

void updateAllTeams(tuidb* tui);
void updateTeamSize(tuidb* tui, int team_n, int team_size);

void runTuiDB(tuidb* tui);

int playerInList(dlist* list, int player_id);
void selectPlayer(tuidb* tui);
void unselectPlayer(tuidb* tui);
void initSelectedInd(tuidb* tui);

player* selectedPlayer(tuidb* tui);
team* selectedTeam(tuidb* tui);

void renameSelectedListElem(tuidb* tui);
void deleteSelectedListElem(tuidb* tui);

void list_up(tuidb* tui);
void list_down(tuidb* tui);

void fitToScreen(tuidb* tui);
void fitAreaToScreen(listArea* a);
void updateArea(tuidb* tui);
void renderTuidb(tuidb* tui);
void renderAllPlayersList(tuidb* tui);
void renderSelectedList(tuidb* tui);
void renderPlayerInfo(tuidb* tui);
void renderAllTeamsList(tuidb* tui);
void renderSelectedTeam(tuidb* tui);

void handleKeyPress(tuidb* tui, char c);

#endif

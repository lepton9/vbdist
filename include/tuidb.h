#ifndef TUIDB_H
#define TUIDB_H

#include "sql.h"
#include "player.h"
#include "tui.h"

#define BASE_LIST_LEN 50
#define BASE_SECTION_WIDTH 50

typedef struct {
  size_t maxShown;
  size_t width;
  size_t firstInd;
  int selected;
} listArea;

typedef struct {
  sqldb* db;
  dlist* allPlayers;
  dlist* players;
  listArea* allPlayersArea;
  term_size* term;
  int teams_n;
  int team_size;
  int show_info;
} tuidb;


tuidb* initTuiDB(int teams, int team_size);
void freeTuiDB(tuidb* tui);
void updateTeamSize(tuidb* tui, int team_n, int team_size);

void runTuiDB(tuidb* tui);

int playerInList(dlist* list, int player_id);
void selectPlayer(tuidb* tui);
void unselectPlayer(tuidb* tui);

void list_up(tuidb* tui);
void list_down(tuidb* tui);
void list_left(tuidb* tui);
void list_right(tuidb* tui);

void fitToScreen(tuidb* tui);
void updateArea(tuidb* tui);
void renderTuidb(tuidb* tui);
void renderAllPlayersList(tuidb* tui);
void renderSelectedList(tuidb* tui);
void renderPlayerInfo(tuidb* tui);

void handleKeyPress(tuidb* tui, char c);

#endif

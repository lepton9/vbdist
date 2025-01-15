#ifndef TUIDB_H
#define TUIDB_H

#include "sql.h"
#include "player.h"
#include "tui.h"

typedef struct {
  size_t maxShown;
  size_t width;
  size_t firstInd;
  int selected;
} listArea;

typedef struct {
  // sqldb* db;
  playerList* allPlayers;
  playerList* players;
  listArea* allPlayersArea;
  term_size* term;
  int teams;
  int team_size;

} tuidb;


tuidb* initTuiDB(int teams, int team_size);
void freeTuiDB(tuidb* tui);

void runTuiDB(tuidb* tui);

void selectPlayer();
void unselectPlayer();

int isSelected(player** players, int player_id);


void list_up(tuidb* tui);
void list_down(tuidb* tui);
void list_left(tuidb* tui);
void list_right(tuidb* tui);

void renderPlayerList(tuidb* tui);
void renderTuidb(tuidb* tui);

#endif

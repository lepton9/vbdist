#ifndef TUIDB_H
#define TUIDB_H

#include "sql.h"
#include "player.h"

typedef struct {
  sqldb* db;
  player** allPlayers;
  player** players;
  size_t all_n;
  size_t n;
  int selected;
  int teams;
  int team_size;

} tuidb;


tuidb* initTuiDB();
void freeTuiDB(tuidb* tui);

void runTuiDB(tuidb* tui);

void selectPlayer();
void unselectPlayer();

int isSelected(player** players, int player_id);


void list_up(tuidb* tui);
void list_down(tuidb* tui);
void list_left(tuidb* tui);
void list_right(tuidb* tui);

void renderTuidb(tuidb* tui);

#endif

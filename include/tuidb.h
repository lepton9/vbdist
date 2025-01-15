#ifndef TUIDB_H
#define TUIDB_H

#include "sql.h"
#include "player.h"

typedef struct {
  sqldb* db;
  player** players;
  size_t n;
  int teams;
  int team_size;

} tuidb;


tuidb* initTuiDB();
void freeTuiDB(tuidb* tui);

void selectPlayer();
void unselectPlayer();

int isSelected(player** players, int player_id);

#endif

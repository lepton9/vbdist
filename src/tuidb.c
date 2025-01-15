#include "../include/tuidb.h"
#include <stdlib.h>

tuidb* initTuiDB(int teams, int team_size) {
  tuidb* tui = malloc(sizeof(tuidb));
  tui->teams = teams;
  tui->team_size = team_size;
  tui->players = malloc(sizeof(player*) * teams * team_size);

  return tui;
}

void freeTuiDB(tuidb* tui) {
  free(tui);
}


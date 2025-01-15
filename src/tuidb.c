#include "../include/tuidb.h"
#include "../include/tui.h"
#include <stdlib.h>

tuidb* initTuiDB(int teams, int team_size) {
  tuidb* tui = malloc(sizeof(tuidb));
  tui->teams = teams;
  tui->team_size = team_size;

  tui->allPlayers = mallocPList(50);
  tui->players = mallocPList(teams * team_size);

  tui->allPlayersArea = malloc(sizeof(listArea));
  tui->allPlayersArea->firstInd = -1;
  tui->allPlayersArea->selected = -1;
  tui->allPlayersArea->width = 50;
  tui->allPlayersArea->maxShown = 10;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);

  return tui;
}

void freeTuiDB(tuidb* tui) {
  for (int i = 0; i < tui->allPlayers->n; i++) {
    freePlayer(tui->allPlayers->players[i]);
  }
  for (int i = 0; i < tui->players->n; i++) {
    freePlayer(tui->players->players[i]);
  }
  free(tui->allPlayers);
  free(tui->players);
  free(tui->allPlayersArea);
  free(tui->term);
  free(tui);
}

void runTuiDB(tuidb* tui) {
  char c = 0;

  while (c != 'q') {
    renderTuidb(tui);

    c = keyPress();
  }

}

void updateArea(tuidb* tui) {
  getTermSize(tui->term);
  tui->allPlayersArea->width = tui->term->cols / 2;

}

void renderTuidb(tuidb* tui) {

}



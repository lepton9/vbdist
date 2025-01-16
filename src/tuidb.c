#include "../include/tuidb.h"
#include "../include/tui.h"
#include <stdio.h>
#include <stdlib.h>

tuidb* initTuiDB(int teams, int team_size) {
  tuidb* tui = malloc(sizeof(tuidb));
  tui->teams = teams;
  tui->team_size = team_size;

  tui->allPlayers = mallocPList(50);
  tui->players = mallocPList(teams * team_size);

  tui->allPlayersArea = malloc(sizeof(listArea));
  tui->allPlayersArea->firstInd = 0;
  tui->allPlayersArea->selected = 0;
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


void list_up(tuidb* tui) {
  if (tui->allPlayersArea->selected > 0) {
    tui->allPlayersArea->selected -= 1;
  }
}


void list_down(tuidb* tui) {
  if (tui->allPlayersArea->selected < tui->allPlayers->n - 1) {
    tui->allPlayersArea->selected += 1;
  }
}

void list_left(tuidb* tui) {

}

void list_right(tuidb* tui) {

}

void handleKeyPress(tuidb* tui, char c) {
    switch (c) {
      case 13: case '\n': case ' ':
        break;
      case 27: // Esc
        break;
      case 'k': case 'w':
        list_up(tui);
        break;
      case 'j': case 's':
        list_down(tui);
        break;
      case 'h': case 'a':
        break;
      case 'l': case 'd':
        break;
      default: {
        break;
      }
    }

}

void runTuiDB(tuidb* tui) {
  altBufferEnable();

  char c = 0;
  while (c != 'q') {
    updateArea(tui);
    renderTuidb(tui);
    c = keyPress();
    handleKeyPress(tui, c);
  }

  cls(stdout);
  altBufferDisable();
}

void updateArea(tuidb* tui) {
  getTermSize(tui->term);
  tui->allPlayersArea->width = tui->term->cols / 2;
}

void formatPlayerLine(player* player) {
  printf(" %-20s %.2f", player->firstName, ovRating(player));
}

void renderTuidb(tuidb* tui) {
  cls(stdout);
  renderAllPlayersList(tui);
  renderSelectedList(tui);
}

void renderAllPlayersList(tuidb* tui) {
  int line = 1;
  for (int i = tui->allPlayersArea->firstInd;
       line < tui->term->rows - 1 && i < tui->allPlayers->n; i++) {
    curSet(line, 0);
    if (tui->allPlayersArea->selected == i) printf("\033[7m");
    formatPlayerLine(tui->allPlayers->players[i]);
    if (tui->allPlayersArea->selected == i) printf("\033[27m");
    curSet(line, tui->allPlayersArea->width);
    printf("|");
    line++;
  }
  fflush(stdout);
}

void renderSelectedList(tuidb* tui) {

}



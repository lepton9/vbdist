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

void selectPlayer(tuidb* tui) {
  if (tui->allPlayersArea->selected < 0 || tui->allPlayers->n <= 0) return;
  player* selected = tui->allPlayers->players[tui->allPlayersArea->selected];
  if (playerInList(tui->players, selected->id) >= 0) {
    unselectPlayer(tui);
    return;
  }
  pushPlayer(tui->players, copyPlayer(selected));
}

void unselectPlayer(tuidb* tui) {
  player* selected = tui->allPlayers->players[tui->allPlayersArea->selected];
  int i = playerInList(tui->players, selected->id);
  if (i < 0) return;
  freePlayer(tui->players->players[i]);
  if (i != tui->players->n - 1) {
    tui->players->players[i] = tui->players->players[tui->players->n - 1];
  }
  tui->players->n--;
}

void list_up(tuidb* tui) {
  if (tui->allPlayersArea->selected > 0) {
    tui->allPlayersArea->selected -= 1;
    if (tui->allPlayersArea->selected < tui->allPlayersArea->firstInd) {
      tui->allPlayersArea->firstInd = tui->allPlayersArea->selected;
    }
  }
}

void list_down(tuidb* tui) {
  if (tui->allPlayersArea->selected < tui->allPlayers->n - 1) {
    tui->allPlayersArea->selected += 1;
    if (tui->allPlayersArea->selected > tui->allPlayersArea->firstInd + tui->allPlayersArea->maxShown - 1) {
      tui->allPlayersArea->firstInd = tui->allPlayersArea->selected - tui->allPlayersArea->maxShown + 1;
    }

  }
}

void list_left(tuidb* tui) {

}

void list_right(tuidb* tui) {

}

void handleKeyPress(tuidb* tui, char c) {
    switch (c) {
      case 13: case '\n': case ' ':
        selectPlayer(tui);
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
      case 'i': // Player info
        break;
      default: {
        break;
      }
    }
}

void runTuiDB(tuidb* tui) {
  altBufferEnable();
  curHide();

  char c = 0;
  while (c != 'q') {
    updateArea(tui);
    renderTuidb(tui);
    c = keyPress();
    handleKeyPress(tui, c);
  }

  curShow();
  cls(stdout);
  altBufferDisable();
}

void updateArea(tuidb* tui) {
  getTermSize(tui->term);
  tui->allPlayersArea->width = tui->term->cols / 2;
  tui->allPlayersArea->width =
      (tui->term->rows < BASE_SECTION_WIDTH) ? tui->term->cols / 2 : BASE_SECTION_WIDTH;
  tui->allPlayersArea->maxShown =
      (tui->term->rows < BASE_LIST_LEN) ? tui->term->rows - 2 : BASE_LIST_LEN;
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
  curSet(1, 0);
  printf("\033[4m %-20s %s\033[24m", "Name", "Rating");
  int line = 2;
  for (int i = tui->allPlayersArea->firstInd;
       line <= tui->term->rows - 1 &&
       i - tui->allPlayersArea->firstInd + 1 <= tui->allPlayersArea->maxShown &&
       i < tui->allPlayers->n;
       i++) {
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
  int startCol = tui->allPlayersArea->width + 5;
  curSet(1, startCol);
  printf("Selected %d/%d", (int)tui->players->n, tui->teams * tui->team_size);

  int line = 2;
  for (int i = 0;
       line <= tui->term->rows - 1 &&
       i + 1 <= tui->allPlayersArea->maxShown &&
       i < tui->players->n;
       i++) {
    curSet(line, startCol);
    formatPlayerLine(tui->players->players[i]);
    line++;
  }
  fflush(stdout);
}



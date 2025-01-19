#include "../include/tuidb.h"
#include "../include/tui.h"
#include <stdio.h>
#include <stdlib.h>

tuidb* initTuiDB(int teams, int team_size) {
  tuidb* tui = malloc(sizeof(tuidb));
  tui->teams = teams;
  tui->team_size = team_size;

  tui->allPlayers = init_list(sizeof(player*));

  tui->allPlayersArea = malloc(sizeof(listArea));
  tui->allPlayersArea->firstInd = 0;
  tui->allPlayersArea->selected = 0;
  tui->allPlayersArea->width = 50;
  tui->allPlayersArea->maxShown = BASE_LIST_LEN;
  tui->show_info = 0;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);

  return tui;
}

void freeTuiDB(tuidb* tui) {
  for (int i = 0; i < tui->allPlayers->n; i++) {
    freePlayer(tui->allPlayers->items[i]);
  }
  free(tui->allPlayers);
  free(tui->players);
  free(tui->allPlayersArea);
  free(tui->term);
  free(tui);
}

int playerInList(dlist* list, int player_id) {
  for (int i = 0; i < list->n; i++) {
    if (((player*)list->items[i])->id == player_id) return i;
  }
  return -1;
}

void selectPlayer(tuidb* tui) {
  if (tui->allPlayersArea->selected < 0 || tui->allPlayers->n <= 0) return;
  player* selected = tui->allPlayers->items[tui->allPlayersArea->selected];
  if (playerInList(tui->players, selected->id) >= 0) {
    unselectPlayer(tui);
    return;
  }
  list_add(tui->players, copyPlayer(selected));
}

void unselectPlayer(tuidb* tui) {
  player* selected = tui->allPlayers->items[tui->allPlayersArea->selected];
  int i = playerInList(tui->players, selected->id);
  if (i < 0) return;
  freePlayer(tui->players->items[i]);
  if (i != tui->players->n - 1) {
    tui->players->items[i] = tui->players->items[tui->players->n - 1];
  }
  tui->players->n--;
}

void fitToScreen(tuidb* tui) {
  if (tui->allPlayersArea->selected < tui->allPlayersArea->firstInd) {
    tui->allPlayersArea->firstInd = tui->allPlayersArea->selected;
  }
  else if (tui->allPlayersArea->selected >
    tui->allPlayersArea->firstInd + tui->allPlayersArea->maxShown - 1) {
    tui->allPlayersArea->firstInd =
      tui->allPlayersArea->selected - tui->allPlayersArea->maxShown + 1;
  }
}

void list_up(tuidb* tui) {
  if (tui->allPlayersArea->selected > 0) {
    tui->allPlayersArea->selected -= 1;
    fitToScreen(tui);
  }
}

void list_down(tuidb* tui) {
  if (tui->allPlayersArea->selected < tui->allPlayers->n - 1) {
    tui->allPlayersArea->selected += 1;
    fitToScreen(tui);
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
      case 27: {  // Esc
        tui->show_info = 0;
        break;
      }
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
        tui->show_info ^= 1;
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
  int maxRows = tui->term->rows - 2;
  tui->allPlayersArea->maxShown = (maxRows < BASE_LIST_LEN) ? maxRows : BASE_LIST_LEN;
  fitToScreen(tui);
}

void formatPlayerLine(player* player) {
  printf(" %-20s %.2f", player->firstName, ovRating(player));
}

void renderTuidb(tuidb* tui) {
  cls(stdout);
  renderAllPlayersList(tui);
  if (tui->show_info) renderPlayerInfo(tui);
  else renderSelectedList(tui);
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
    if (playerInList(tui->players, ((player*)tui->allPlayers->items[i])->id) >= 0) {
      printf(">");
    }
    formatPlayerLine(tui->allPlayers->items[i]);
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
    curSet(line++, startCol);
    formatPlayerLine(tui->players->items[i]);
  }
  fflush(stdout);
}

void renderPlayerInfo(tuidb* tui) {
  player* p = tui->allPlayers->items[tui->allPlayersArea->selected];
  int startCol = tui->allPlayersArea->width + 5;
  int line = 1;
  curSet(line++, startCol);
  printf("Name: %s %s", p->firstName, p->surName ? p->surName : "");
  curSet(line++, startCol);
  printf("ID: %d", p->id);
  curSet(line++, startCol);
  printf("Rating: %.2f %.2f %.2f %.2f %.2f %.2f", p->ratings[0], p->ratings[1],
         p->ratings[2], p->ratings[3], p->ratings[4], p->ratings[5]);
  curSet(line++, startCol);
  printf("Overall: %.2f", ovRating(p));

  curSet(++line, startCol);
  printf("Former teammates:");
  line += 2;

  dlist* player_ids = fetchFormerTeammates(tui->db, p);
  for (int i = 0; i < player_ids->n && line < tui->term->rows - 2; i++) {
    int_tuple* t = player_ids->items[i];
    if (t->a == p->id) continue;
    int ind = playerInList(tui->allPlayers, t->a);
    if (ind >= 0) {
      player* p = tui->allPlayers->items[ind];
      curSet(line++, startCol);
      printf("%3d %s %s", t->b, p->firstName, p->surName ? p->surName : "");
    }
  }
  for (int i = 0; i < player_ids->n; i++) {
    free(player_ids->items[i]);
  }
  free_list(player_ids);
  fflush(stdout);
}


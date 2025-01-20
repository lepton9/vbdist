#include "../include/tuidb.h"
#include "../include/tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tuidb* initTuiDB(int teams, int team_size) {
  tuidb* tui = malloc(sizeof(tuidb));
  tui->teams_n = teams;
  tui->team_size = team_size;

  tui->allPlayers = init_list(sizeof(player*));
  tui->allTeams = init_list(sizeof(team*));

  tui->allPlayersArea = malloc(sizeof(listArea));
  tui->allPlayersArea->firstInd = 0;
  tui->allPlayersArea->selected = 0;
  tui->allPlayersArea->width = 50;
  tui->allPlayersArea->maxShown = BASE_LIST_LEN;

  tui->allTeamsArea = malloc(sizeof(listArea));
  tui->allTeamsArea->firstInd = 0;
  tui->allTeamsArea->selected = 0;
  tui->allTeamsArea->width = 50;
  tui->allTeamsArea->maxShown = BASE_LIST_LEN;

  tui->tab = PLAYERS_TAB;
  tui->show_player_info = 0;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);

  return tui;
}

void freeTuiDB(tuidb* tui) {
  for (int i = 0; i < tui->allPlayers->n; i++) {
    freePlayer(tui->allPlayers->items[i]);
  }
  for (int i = 0; i < tui->allTeams->n; i++) {
    freePlayer(tui->allTeams->items[i]);
  }
  free_list(tui->players);
  free_list(tui->allPlayers);
  free_list(tui->allTeams);
  free(tui->allPlayersArea);
  free(tui->allTeamsArea);
  free(tui->term);
  free(tui);
}

void updateTeamSize(tuidb* tui, int team_n, int team_size) {
  tui->teams_n = team_n;
  tui->team_size = team_size;
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
  if (tui->tab == PLAYERS_TAB) fitAreaToScreen(tui->allPlayersArea);
  if (tui->tab == TEAMS_TAB) fitAreaToScreen(tui->allTeamsArea);
}

void fitAreaToScreen(listArea* a) {
  if (a->selected < a->firstInd) {
    a->firstInd = a->selected;
  }
  else if (a->selected > a->firstInd + a->maxShown - 1) {
    a->firstInd = a->selected - a->maxShown + 1;
  }
}

void list_up(tuidb* tui) {
  switch (tui->tab) {
    case PLAYERS_TAB: {
      if (tui->allPlayersArea->selected > 0) {
        tui->allPlayersArea->selected -= 1;
        fitToScreen(tui);
      }
      break;
    }
    case TEAMS_TAB: {
      if (tui->allTeamsArea->selected > 0) {
        tui->allTeamsArea->selected -= 1;
        fitToScreen(tui);
      }
      break;
    }
  }
}

void list_down(tuidb* tui) {
  switch (tui->tab) {
    case PLAYERS_TAB: {
      if (tui->allPlayersArea->selected < tui->allPlayers->n - 1) {
        tui->allPlayersArea->selected += 1;
        fitToScreen(tui);
      }
      break;
    }
    case TEAMS_TAB: {
      if (tui->allTeamsArea->selected < tui->allTeams->n - 1) {
        tui->allTeamsArea->selected += 1;
        fitToScreen(tui);
      }
      break;
    }
  }
}

void renameSelectedPlayer(tuidb* tui) {
  curShow();
  player* p = tui->allPlayers->items[tui->allPlayersArea->selected];
  const int max_len = 50;
  int len = strlen(p->firstName);
  char new[max_len + 1];
  strcpy(new, p->firstName);
  char c = 0;
  while (1) {
    curSet(tui->allPlayersArea->selected_term_row, tui->allPlayersArea->width - 1);
    printf("\033[1K");
    curSet(tui->allPlayersArea->selected_term_row, 1);
    printf("|> %s", new);
    fflush(stdout);
    c = keyPress();
    if (c == 27) {
      break;
    } else if (c == 13 || c == '\n') {
      int r = renamePlayer(tui->db, p, new);
      if (r) {
        if (p->firstName) free(p->firstName);
        p->firstName = strdup(new);
      }
      break;
    } else if (c == 127 || c == 8) {
      new[len - 1] = '\0';
      len--;
    } else if (len < max_len) {
      strncat(new, &c, 1);
      len++;
    }
  }
  curHide();
}

void renameSelectedTeam(tuidb* tui) {
  curShow();
  team* t = tui->allTeams->items[tui->allTeamsArea->selected];
  const int max_len = 50;
  int len = strlen(t->name);
  char new[max_len + 1];
  strcpy(new, t->name);
  char c = 0;
  while (1) {
    curSet(tui->allTeamsArea->selected_term_row, tui->allTeamsArea->width - 1);
    printf("\033[1K");
    curSet(tui->allTeamsArea->selected_term_row, 1);
    printf("|> %s", new);
    fflush(stdout);
    c = keyPress();
    if (c == 27) {
      break;
    } else if (c == 13 || c == '\n') {
      int r = renameTeam(tui->db, t, new);
      if (r) {
        if (t->name) free(t->name);
        t->name = strdup(new);
      }
      break;
    } else if (c == 127 || c == 8) {
      new[len - 1] = '\0';
      len--;
    } else if (len < max_len) {
      strncat(new, &c, 1);
      len++;
    }
  }
  curHide();
}

void handleKeyPress(tuidb* tui, char c) {
    switch (c) {
      case 13: case '\n': case ' ':
        if (tui->tab == PLAYERS_TAB) selectPlayer(tui);
        break;
      case 27: {  // Esc
        tui->show_player_info = 0;
        break;
      }
      case 9: // Tab
        tui->tab = (tui->tab == PLAYERS_TAB) ? TEAMS_TAB : PLAYERS_TAB;
        break;
      case 'r':
        if (tui->tab == PLAYERS_TAB) {
          renameSelectedPlayer(tui);
        } else if (tui->tab == TEAMS_TAB) {
          renameSelectedTeam(tui);
        }
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
        if (tui->tab == PLAYERS_TAB) tui->show_player_info ^= 1;
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
  int maxRows = tui->term->rows - 2;
  int baseWidth = (tui->term->rows < BASE_SECTION_WIDTH) ? tui->term->cols / 2
                                                         : BASE_SECTION_WIDTH;
  int maxShown = (maxRows < BASE_LIST_LEN) ? maxRows : BASE_LIST_LEN;

  tui->allPlayersArea->width = baseWidth;
  tui->allPlayersArea->maxShown = maxShown;

  tui->allTeamsArea->width = baseWidth;
  tui->allTeamsArea->maxShown = maxShown;
  fitToScreen(tui);
}

void formatPlayerLine(player* player) {
  printf(" %-20s %.2f", player->firstName, ovRating(player));
}

void formatTeamLine(team* team) {
  printf(" %-20s", team->name);
}

void renderTuidb(tuidb* tui) {
  cls(stdout);
  switch (tui->tab) {
    case PLAYERS_TAB: {
      renderAllPlayersList(tui);
      if (tui->show_player_info) renderPlayerInfo(tui);
      else renderSelectedList(tui);
      break;
    }
    case TEAMS_TAB: {
      renderAllTeamsList(tui);
      renderSelectedTeam(tui);
      break;
    }
  }
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
    if (tui->allPlayersArea->selected == i) {
      tui->allPlayersArea->selected_term_row = line;
      printf("\033[7m");
    }
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
  printf("Selected %d/%d", (int)tui->players->n, tui->teams_n * tui->team_size);

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

  line = 6;
  curSet(line, startCol + 30);
  printf("Not teammates with:");
  line += 2;

  dlist* not_player_ids = fetchNotTeammates(tui->db, p);
  for (int i = 0; i < not_player_ids->n && line < tui->term->rows - 2; i++) {
    int_tuple* t = not_player_ids->items[i];
    if (t->a == p->id) continue;
    int ind = playerInList(tui->allPlayers, t->a);
    if (ind >= 0) {
      player* p = tui->allPlayers->items[ind];
      curSet(line++, startCol + 30);
      printf("%s %s", p->firstName, p->surName ? p->surName : "");
    }
  }
  for (int i = 0; i < not_player_ids->n; i++) {
    free(not_player_ids->items[i]);
  }
  free_list(not_player_ids);

  for (int i = 0; i < player_ids->n; i++) {
    free(player_ids->items[i]);
  }
  free_list(player_ids);
  fflush(stdout);
}

void renderAllTeamsList(tuidb* tui) {
  curSet(1, 0);
  printf("\033[4m %-20s\033[24m", "Name");
  int line = 2;
  for (int i = tui->allTeamsArea->firstInd;
       line <= tui->term->rows - 1 &&
       i - tui->allTeamsArea->firstInd + 1 <= tui->allTeamsArea->maxShown &&
       i < tui->allTeams->n;
       i++) {
    curSet(line, 0);
    if (tui->allTeamsArea->selected == i) {
      tui->allTeamsArea->selected_term_row = line;
      printf("\033[7m");
    }
    formatTeamLine(tui->allTeams->items[i]);
    if (tui->allTeamsArea->selected == i) printf("\033[27m");
    curSet(line, tui->allTeamsArea->width);
    printf("|");
    line++;
  }
  fflush(stdout);
}

void renderSelectedTeam(tuidb* tui) {
  team* t = tui->allTeams->items[tui->allTeamsArea->selected];
  int startCol = tui->allTeamsArea->width + 5;
  int line = 1;
  curSet(line++, startCol);
  printf("Players in team %s:", t->name);
  line++;

  dlist* player_ids = fetchPlayersInTeam(tui->db, t);
  for (int i = 0; i < player_ids->n && line < tui->term->rows - 2; i++) {
    int* id = player_ids->items[i];
    int ind = playerInList(tui->allPlayers, *id);
    if (ind >= 0) {
      player* p = tui->allPlayers->items[ind];
      curSet(line++, startCol);
      printf("%s %s", p->firstName, p->surName ? p->surName : "");
    }
  }

  for (int i = 0; i < player_ids->n; i++) {
    free(player_ids->items[i]);
  }
  free_list(player_ids);
  fflush(stdout);
}


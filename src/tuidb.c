#include "../include/tuidb.h"
#include "../include/tui.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tuidb* initTuiDB(int teams, int team_size) {
  tuidb* tui = malloc(sizeof(tuidb));
  tui->teams_n = teams;
  tui->team_size = team_size;

  tui->allPlayers = NULL;
  tui->allTeams = NULL;

  tui->allPlayersArea = init_list_area(BASE_SECTION_WIDTH, BASE_LIST_LEN);
  tui->allTeamsArea = init_list_area(BASE_SECTION_WIDTH, BASE_LIST_LEN);

  tui->tab = PLAYERS_TAB;
  tui->show_player_info = 0;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);
  tui->render = init_renderer(stdout);

  return tui;
}

void freeAllPlayers(dlist* allPlayers) {
  if (allPlayers) {
    for (int i = 0; i < (int)allPlayers->n; i++) {
      freePlayer(allPlayers->items[i]);
    }
    free_list(allPlayers);
  }
}

void freeAllTeams(dlist* allTeams) {
  if (allTeams) {
    for (int i = 0; i < (int)allTeams->n; i++) {
      freeTeam(allTeams->items[i]);
    }
    free_list(allTeams);
  }
}

void setAllPlayers(tuidb* tui, dlist* players) {
  freeAllPlayers(tui->allPlayers);
  tui->allPlayers = players;
  update_list_len(tui->allPlayersArea, tui->allPlayers->n);
}

void setAllTeams(tuidb* tui, dlist* teams) {
  freeAllTeams(tui->allTeams);
  tui->allTeams = teams;
  update_list_len(tui->allTeamsArea, tui->allTeams->n);
}

void freeTuiDB(tuidb* tui) {
  if (!tui) return;
  freeAllPlayers(tui->allPlayers);
  freeAllTeams(tui->allTeams);
  free_renderer(tui->render);
  free(tui->allPlayersArea);
  free(tui->allTeamsArea);
  free(tui->term);
  free(tui);
}

void updateAllTeams(tuidb* tui) {
  for (int i = 0; i < (int)tui->allTeams->n; i++) {
    freeTeam(tui->allTeams->items[i]);
  }
  free_list(tui->allTeams);
  tui->allTeams = fetchTeams(tui->db);
}

void updateTeamSize(tuidb* tui, int team_n, int team_size) {
  tui->teams_n = team_n;
  tui->team_size = team_size;
}

player* selectedPlayer(tuidb *tui) {
  if (tui->allPlayers->n == 0 || tui->allPlayersArea->selected < 0)
    return NULL;
  return (player *)tui->allPlayers->items[tui->allPlayersArea->selected];
}

team* selectedTeam(tuidb *tui) {
  if (tui->allTeams->n == 0 || tui->allTeamsArea->selected < 0)
    return NULL;
  return (team *)tui->allTeams->items[tui->allTeamsArea->selected];
}

void selectPlayer(tuidb* tui) {
  if (tui->allPlayersArea->selected < 0 || tui->allPlayers->n <= 0) return;
  player* selected = selectedPlayer(tui);
  if (playerInList(tui->players, selected->id) >= 0) {
    unselectPlayer(tui);
    return;
  }
  list_add(tui->players, copyPlayer(selected));
}

void unselectPlayer(tuidb* tui) {
  player* selected = selectedPlayer(tui);
  int i = playerInList(tui->players, selected->id);
  if (i < 0) return;
  player* p = pop_elem(tui->players, i);
  freePlayer(p);
}

void tuidb_list_up(tuidb* tui) {
  switch (tui->tab) {
    case PLAYERS_TAB: {
      list_up(tui->allPlayersArea);
      break;
    }
    case TEAMS_TAB: {
      list_up(tui->allTeamsArea);
      break;
    }
  }
}

void tuidb_list_down(tuidb* tui) {
  switch (tui->tab) {
    case PLAYERS_TAB: {
      list_down(tui->allPlayersArea);
      break;
    }
    case TEAMS_TAB: {
      list_down(tui->allTeamsArea);
      break;
    }
  }
}

void renameSelectedListElem(tuidb* tui) {
  int width = 0;
  int row = 0;
  char* old_name = NULL;
  if (tui->tab == PLAYERS_TAB) {
    player* p = selectedPlayer(tui);
    if (!p) return;
    old_name = p->firstName;
    width = tui->allPlayersArea->width;
    row = tui->allPlayersArea->selected_term_row;
  } else if (tui->tab == TEAMS_TAB) {
    team* t = selectedTeam(tui);
    if (!t) return;
    old_name = t->name;
    width = tui->allTeamsArea->width;
    row = tui->allTeamsArea->selected_term_row;
  }
  curShow();
  const size_t max_len = 50;
  size_t len = strlen((old_name) ? old_name : 0);
  char new[max_len + 1];
  strcpy(new, (old_name) ? old_name : "");
  int c = 0;
  while (1) {
    curSet(row, width - 1);
    printf("\033[1K");
    curSet(row, 1);
    printf("|> %s", new);
    fflush(stdout);
    // update_segment(tui->render, row-1, 1, width-1, "|> %s", new);
    // render(tui->render, stdout);
    c = keyPress();
    if (c == 27) {
      break;
    } else if (isEnter(c)) {
      if (*new == '\0') {
        break;
      }
      switch (tui->tab) {
        case PLAYERS_TAB: {
          player* p = selectedPlayer(tui);
          int r = renamePlayer(tui->db, p, new);
          if (r) {
            if (p->firstName) free(p->firstName);
            p->firstName = strdup(new);
            player* pSel = getPlayerInList(tui->players, p->id);
            if (pSel) {
              if (pSel->firstName) free(pSel->firstName);
              pSel->firstName = strdup(new);
            }
          }
          break;
        }
        case TEAMS_TAB: {
          team* t = selectedTeam(tui);
          int r = renameTeam(tui->db, t, new);
          if (r) {
            if (t->name) free(t->name);
            t->name = strdup(new);
          }
        }
        break;
      }
      break;
    } else if (len > 0 && isBackspace(c)) {
      new[len - 1] = '\0';
      len--;
    } else if (len < max_len) {
      if (c >= 32 && c <= 126) {
        strcatc(new, &len, c);
      }
    }
  }
  curHide();
  refresh_screen(tui->render);
}

void deleteSelectedListElem(tuidb* tui) {
  int width = 0;
  int row = 0;
  char* name = NULL;
  if (tui->tab == PLAYERS_TAB) {
    player* p = selectedPlayer(tui);
    if (!p) return;
    name = p->firstName;
    width = tui->allPlayersArea->width;
    row = tui->allPlayersArea->selected_term_row;
  } else if (tui->tab == TEAMS_TAB) {
    team* t = selectedTeam(tui);
    if (!t) return;
    name = t->name;
    width = tui->allTeamsArea->width;
    row = tui->allTeamsArea->selected_term_row;
  }

  curShow();
  curSet(row, width - 1);
  printf("\033[1K");
  curSet(row, 1);
  printf("|> Delete %s? [y/N]", name);
  fflush(stdout);
  char c = keyPress();
  if (c == 'y') {
    if (tui->tab == PLAYERS_TAB) {
      int r = deletePlayer(tui->db, selectedPlayer(tui));
      if (r) {
        player* p = pop_elem(tui->allPlayers, tui->allPlayersArea->selected);
        if (p) freePlayer(p);
        int maxSelected = tui->allPlayers->n - 1;
        if (tui->allPlayersArea->selected > maxSelected) {
          tui->allPlayersArea->selected = maxSelected;
        }
      }
    } else if (tui->tab == TEAMS_TAB) {
      int r = deleteTeam(tui->db, selectedTeam(tui));
      if (r) {
        team* t = pop_elem(tui->allTeams, tui->allTeamsArea->selected);
        if (t) freeTeam(t);
        int maxSelected = tui->allTeams->n - 1;
        if (tui->allTeamsArea->selected > maxSelected) {
          tui->allTeamsArea->selected = maxSelected;
        }
      }
    }
  }
  curHide();
  refresh_screen(tui->render);
}

void handleKeyPress(tuidb* tui, int c) {
  switch (c) {
    case 13: case '\n': case ' ':
#ifdef __linux__
    case KEY_ENTER:
#endif
    if (tui->tab == PLAYERS_TAB) {
      selectPlayer(tui);
    }
    break;
    case 27: {  // Esc
      tui->show_player_info = 0;
      break;
    }
    case 9: // Tab
      tui->tab = (tui->tab == PLAYERS_TAB) ? TEAMS_TAB : PLAYERS_TAB;
      break;
    case 'R': case 'r':
      renameSelectedListElem(tui);
      break;
    case 'X': case 'x':
      deleteSelectedListElem(tui);
      break;
    case 'K': case 'W':
    case 'k': case 'w':
#ifdef __linux__
    case KEY_UP:
#endif
      tuidb_list_up(tui);
      break;
    case 'J': case 'S':
    case 'j': case 's':
#ifdef __linux__
    case KEY_DOWN:
#endif
      tuidb_list_down(tui);
      break;
    case 'h': case 'a':
      break;
    case 'l': case 'd':
      break;
    case 'I': case 'i': // Player info
      if (tui->tab == PLAYERS_TAB) tui->show_player_info ^= 1;
      break;
    default: {
      break;
    }
  }
}

void runTuiDB(tuidb* tui) {
  curHide();
  refresh_screen(tui->render);
  int c = 0;
  while (c != 'q') {
    check_selected(tui->allPlayersArea);
    check_selected(tui->allTeamsArea);
    updateArea(tui);
    renderTuidb(tui);
    c = keyPress();
    handleKeyPress(tui, c);
  }
}

void updateArea(tuidb* tui) {
  getTermSize(tui->term);
  int maxRows = tui->term->rows - 2;
  int baseWidth = (tui->term->rows < BASE_SECTION_WIDTH) ? tui->term->cols / 2
                                                         : BASE_SECTION_WIDTH;
  int maxShown = (maxRows < BASE_LIST_LEN) ? maxRows : BASE_LIST_LEN;

  update_list_area(tui->allPlayersArea, baseWidth, maxShown);
  update_list_area(tui->allTeamsArea, baseWidth, maxShown);

  setSize(tui->render, tui->term->cols, tui->term->rows);
}

void renderTuidb(tuidb* tui) {
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
  render(tui->render);
}

void renderAllPlayersList(tuidb* tui) {
  append_line(tui->render, 0, "\033[4m %-20s %-10s %d/%d\033[24m", "Name",
           "Rating", tui->allPlayersArea->selected + 1,
           (int)tui->allPlayers->n);
  if (tui->allPlayers->n > 0) {
    int line = 1;
    for (int i = tui->allPlayersArea->first_ind;
    line <= tui->term->rows - 1 &&
    i - tui->allPlayersArea->first_ind + 1 <= (int)tui->allPlayersArea->max_shown &&
    i < (int)tui->allPlayers->n;
    i++) {
      if (tui->allPlayersArea->selected == i) {
        tui->allPlayersArea->selected_term_row = line + 1;
        append_line(tui->render, line, "\033[7m");
      }
      if (playerInList(tui->players, ((player*)tui->allPlayers->items[i])->id) >= 0) {
        append_line(tui->render, line, ">");
      }
      player* player = tui->allPlayers->items[i];
      append_line(tui->render, line, " %-20s %.2f", player->firstName, rating(player));
      if (tui->allPlayersArea->selected == i) {
        append_line(tui->render, line, "\033[27m");
      }
      put_text(tui->render, line, tui->allPlayersArea->width, "|");
      line++;
    }
  }
}

void renderSelectedList(tuidb* tui) {
  int startCol = tui->allPlayersArea->width + 5;
  const int total_size = tui->teams_n * tui->team_size;
  put_text(tui->render, 0, startCol, "Selected %s%d/%d%s",
           ((int)tui->players->n > total_size) ? "\033[31m" : "",
           (int)tui->players->n, total_size, "\033[0m");
  int line = 2;
  for (int i = 0;
       line <= tui->term->rows - 1 &&
       i + 1 <= (int)tui->allPlayersArea->max_shown &&
       i < (int)tui->players->n;
       i++) {
    player* p = tui->players->items[i];
    put_text(tui->render, line++, startCol, " %-20s %.2f", p->firstName, rating(p));
  }
}

void renderPlayerInfo(tuidb* tui) {
  player* p = selectedPlayer(tui);
  if (!p) return;
  int startCol = tui->allPlayersArea->width + 5;
  int line = 0;
  put_text(tui->render, line++, startCol, "Name: %s %s", p->firstName,
           p->surName ? p->surName : "");

  put_text(tui->render, line++, startCol, "ID: %d", p->id);

  line++;
  for (size_t i = 0; i < p->skills->n; i++) {
    skill* s = p->skills->items[i];
    put_text(tui->render, line++, startCol,
             "%-15s %.2f", s->name, s->value);
  }

  put_text(tui->render, line++, startCol, "%-15s %.2f", "Overall", rating(p));

  // TODO:
  line++;
  put_text(tui->render, line++, startCol, "%-15s %d", "Positions:", p->positions->n);

  for (size_t i = 0; i < p->positions->n; i++) {
    position * pos = p->positions->items[i];
    put_text(tui->render, line++, startCol,
             "%d %-15s", pos->priority, pos->name);
  }

  put_text(tui->render, ++line, startCol, "Former teammates:");
  line += 2;

  dlist* player_ids = fetchFormerTeammates(tui->db, p);
  for (int i = 0; i < (int)player_ids->n && line < tui->term->rows - 2; i++) {
    int_tuple* t = player_ids->items[i];
    if (t->a == p->id) continue;
    int ind = playerInList(tui->allPlayers, t->a);
    if (ind >= 0) {
      player* p = tui->allPlayers->items[ind];
      put_text(tui->render, line++, startCol, "%3d %s %s", t->b, p->firstName,
               p->surName ? p->surName : "");
    }
  }

  line = 1;
  startCol = startCol + 30;
  put_text(tui->render, line, startCol, "Not teammates with:");
  line += 2;

  int printed_players = 0;
  dlist* not_player_ids = fetchNotTeammates(tui->db, p);
  for (int i = 0; i < (int)not_player_ids->n && line < tui->term->rows - 3; i++) {
    int_tuple* t = not_player_ids->items[i];
    if (t->a == p->id) continue;
    int ind = playerInList(tui->allPlayers, t->a);
    if (ind >= 0) {
      player* p = tui->allPlayers->items[ind];
      put_text(tui->render, line++, startCol, "%s %s", p->firstName,
               p->surName ? p->surName : "");
      printed_players++;
    }
  }
  if (not_player_ids->n - printed_players > 0) {
    put_text(tui->render, line++, startCol, "... %d more players",
             not_player_ids->n - printed_players);
  }

  for (int i = 0; i < (int)not_player_ids->n; i++) {
    free(not_player_ids->items[i]);
  }
  free_list(not_player_ids);

  for (int i = 0; i < (int)player_ids->n; i++) {
    free(player_ids->items[i]);
  }
  free_list(player_ids);
}

void renderAllTeamsList(tuidb* tui) {
  append_line(tui->render, 0, "\033[4m %-20s%d/%d\033[24m", "Name",
              tui->allTeamsArea->selected + 1, (int)tui->allTeams->n);
  if (tui->allTeams->n > 0) {
    int line = 1;
    for (int i = tui->allTeamsArea->first_ind;
    line <= tui->term->rows - 1 &&
    i - tui->allTeamsArea->first_ind + 1 <= (int)tui->allTeamsArea->max_shown &&
    i < (int)tui->allTeams->n;
    i++) {
      if (tui->allTeamsArea->selected == i) {
        tui->allTeamsArea->selected_term_row = line + 1;
        append_line(tui->render, line, "\033[7m");
      }
      append_line(tui->render, line, " %-20s", ((team*)tui->allTeams->items[i])->name);
      if (tui->allTeamsArea->selected == i) {
        append_line(tui->render, line, "\033[27m");
      }
      put_text(tui->render, line, tui->allTeamsArea->width, "|");
      line++;
    }
  }
}

void renderSelectedTeam(tuidb* tui) {
  team* t = selectedTeam(tui);
  if (!t) return;
  int startCol = tui->allTeamsArea->width + 5;
  int line = 1;
  put_text(tui->render, line++, startCol, "Players in team %s:", t->name);
  line++;

  dlist* player_ids = fetchPlayersInTeam(tui->db, t);
  for (int i = 0; i < (int)player_ids->n && line < tui->term->rows - 2; i++) {
    int* id = player_ids->items[i];
    int ind = playerInList(tui->allPlayers, *id);
    if (ind >= 0) {
      player* p = tui->allPlayers->items[ind];
      put_text(tui->render, line++, startCol, "%s %s", p->firstName,
               p->surName ? p->surName : "");
    }
  }

  for (int i = 0; i < (int)player_ids->n; i++) {
    free(player_ids->items[i]);
  }
  free_list(player_ids);
}


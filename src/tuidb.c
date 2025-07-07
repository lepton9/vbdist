#include "../include/tuidb.h"
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
  set_area_pos(tui->allPlayersArea->area, 1, 0);
  set_padding(tui->allPlayersArea->area, 3, 1, 2, 0);
  set_area_pos(tui->allTeamsArea->area, 1, 0);
  set_padding(tui->allTeamsArea->area, 3, 1, 2, 0);

  tui->tab = PLAYERS_TAB;
  tui->active_area = PLAYERS_LIST;
  tui->show_player_info = 0;

  tui->p_edit = malloc(sizeof(player_edit));
  tui->p_edit->active = 0;
  tui->p_edit->selected_element = SKILLS_LIST;
  tui->p_edit->lists_index = 0;
  tui->p_edit->p = NULL;

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
  free_list_area(tui->allPlayersArea);
  free_list_area(tui->allTeamsArea);
  free(tui->p_edit);
  free(tui->term);
  free(tui);
}

void updateAllTeams(tuidb* tui) {
  for (size_t i = 0; i < tui->allTeams->n; i++) {
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
  return tui->allPlayers->items[tui->allPlayersArea->selected];
}

team* selectedTeam(tuidb *tui) {
  if (tui->allTeams->n == 0 || tui->allTeamsArea->selected < 0)
    return NULL;
  return tui->allTeams->items[tui->allTeamsArea->selected];
}

void unselectPlayer(tuidb* tui, int index) {
  if (index < 0) return;
  player* p = pop_elem(tui->players, index);
  freePlayer(p);
}

void unselect_all(tuidb* tui) {
  for (int i = (int)tui->players->n; i >= 0; i--) {
    unselectPlayer(tui, i);
  }
}

void selectCurPlayer(tuidb* tui) {
  if (tui->allPlayersArea->selected < 0 || tui->allPlayers->n <= 0) return;
  player* selected = selectedPlayer(tui);
  if (playerInList(tui->players, selected->id) >= 0) {
    unselectCurPlayer(tui);
    return;
  }
  list_add(tui->players, copyPlayer(selected));
}

void unselectCurPlayer(tuidb* tui) {
  player* selected = selectedPlayer(tui);
  int i = playerInList(tui->players, selected->id);
  unselectPlayer(tui, i);
}

void tuidb_list_up(tuidb* tui) {
  switch (tui->tab) {
    case PLAYERS_TAB: {
      if (tui->active_area == PLAYERS_LIST) list_up(tui->allPlayersArea);
      else if (tui->active_area == PLAYER_EDIT && tui->p_edit->active) {
        pedit_list_up(tui);
      }
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
      if (tui->active_area == PLAYERS_LIST) list_down(tui->allPlayersArea);
      else if (tui->active_area == PLAYER_EDIT && tui->p_edit->active) {
        pedit_list_down(tui);
      }
      break;
    }
    case TEAMS_TAB: {
      list_down(tui->allTeamsArea);
      break;
    }
  }
}

void pedit_list_up(tuidb* tui) {
  switch (tui->p_edit->selected_element) {
    case SKILLS_LIST:
      if (tui->p_edit->lists_index > 0) {
        tui->p_edit->lists_index--;
      }
      break;
    case POSITIONS_LIST:
      if (tui->p_edit->lists_index <= 0) {
        tui->p_edit->lists_index = max_int(tui->p_edit->p->skills->n - 1, 0);
        tui->p_edit->selected_element = SKILLS_LIST;
      } else {
        tui->p_edit->lists_index--;
      }
      break;
    default:
      break;
  }
}

void pedit_list_down(tuidb* tui) {
  switch (tui->p_edit->selected_element) {
    case SKILLS_LIST:
      if (tui->p_edit->lists_index >= (int)tui->p_edit->p->skills->n - 1) {
        tui->p_edit->lists_index = 0;
        tui->p_edit->selected_element = POSITIONS_LIST;
      } else {
        tui->p_edit->lists_index++;
      }
      break;
    case POSITIONS_LIST:
      if (tui->p_edit->lists_index < (int)tui->p_edit->p->positions->n - 1) {
        tui->p_edit->lists_index++;
      }
      break;
    default:
      break;
  }
}

void renameSelectedListElem(tuidb* tui) {
  int width = 0;
  int row = 0;
  char* old_name = NULL;
  if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYERS_LIST) {
    player* p = selectedPlayer(tui);
    if (!p) return;
    old_name = p->firstName;
    width = tui->allPlayersArea->area->width;
    row = tui->allPlayersArea->selected_term_row;
  } else if (tui->tab == TEAMS_TAB) {
    team* t = selectedTeam(tui);
    if (!t) return;
    old_name = t->name;
    width = tui->allTeamsArea->area->width;
    row = tui->allTeamsArea->selected_term_row;
  } else return;
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
  if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYERS_LIST) {
    player* p = selectedPlayer(tui);
    if (!p) return;
    name = p->firstName;
    width = tui->allPlayersArea->area->width;
    row = tui->allPlayersArea->selected_term_row;
  } else if (tui->tab == TEAMS_TAB) {
    team* t = selectedTeam(tui);
    if (!t) return;
    name = t->name;
    width = tui->allTeamsArea->area->width;
    row = tui->allTeamsArea->selected_term_row;
  } else return;

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

void toggle_edit_player(tuidb* tui) {
  if (tui->active_area == PLAYER_EDIT) {
    tui->active_area = PLAYERS_LIST;
    return;
  }
  player* selected = selectedPlayer(tui);
  if (selected) {
    tui->p_edit->active = 1;
    tui->p_edit->selected_element = SKILLS_LIST;
    tui->p_edit->lists_index = 0;
    tui->show_player_info = 1;
    tui->p_edit->p = selected;
    tui->active_area = PLAYER_EDIT;
  }
}

void handle_esc(tuidb* tui) {
  if (tui->active_area == PLAYER_EDIT) {
    tui->p_edit->active = 0;
    tui->p_edit->p = NULL;
    tui->active_area = PLAYERS_LIST;
  } else {
    tui->show_player_info = 0;
  }
}

void handleKeyPress(tuidb* tui, int c) {
  switch (c) {
    case 13: case '\n': case ' ':
#ifdef __linux__
    case KEY_ENTER:
#endif
    if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYERS_LIST) {
      selectCurPlayer(tui);
    }
    break;
    case 27: // Esc
      handle_esc(tui);
      break;
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
      if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYERS_LIST)
        tui->show_player_info ^= 1;
      break;
    case 'e': case 'E': // Edit player
      if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYERS_LIST)
        toggle_edit_player(tui);
      break;
    case 'u': case 'U':
      unselect_all(tui);
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
  int height = min_int(tui->term->rows - 1, BASE_LIST_LEN);
  int width = min_int(tui->term->cols / 2, 40);

  update_list_area(tui->allPlayersArea, width, height);
  update_list_area(tui->allTeamsArea, width, height);
}

void renderTab(tuidb* tui) {
  switch (tui->tab) {
    case PLAYERS_TAB: {
      put_text(tui->render, 0, 0, "\033[7m  %-10s\033[27m  %-10s", "Players", " Teams ");
      break;
    }
    case TEAMS_TAB: {
      put_text(tui->render, 0, 0, "  %-10s\033[7m  %-10s\033[27m", "Players", " Teams ");
      break;
    }
  }
}

void renderTuidb(tuidb* tui) {
  renderTab(tui);
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

void draw_area_borders(renderer* render, tui_area* area, color_fg color) {
  make_borders_color(render, area->start_col, area->start_row, area->width,
                     area->height, color);
}

void renderAllPlayersList(tuidb* tui) {
  int col = start_print_col(tui->allPlayersArea->area);
  int line = start_print_line(tui->allPlayersArea->area);
  int len = getListAreaLen(tui->allPlayersArea, tui->term->rows);

  draw_area_borders(tui->render, tui->allPlayersArea->area, (tui->active_area == PLAYERS_LIST) ? BLUE_FG : DEFAULT_FG);

  put_text(tui->render, tui->allTeamsArea->area->start_row + 1, 2,
           "\033[4m %-20s %-10s\033[24m", "Name", "Rating");
  put_text(tui->render, area_last_line(tui->allPlayersArea->area), 2, "%d/%d",
           tui->allPlayersArea->selected + 1, (int)tui->allPlayers->n);

  for (int i = tui->allPlayersArea->first_ind; i < tui->allPlayersArea->first_ind + len; i++) {
    player* p = tui->allPlayers->items[i];
    char selected = playerInList(tui->players, ((player *)tui->allPlayers->items[i])->id) >= 0;

    if (tui->allPlayersArea->selected == i) {
      tui->allPlayersArea->selected_term_row = line + 1;
      put_text(tui->render, line++, col, "\033[7m%s %-20s %.2f\033[27m",
               (selected) ? ">" : "", p->firstName, rating(p));
    } else {
      put_text(tui->render, line++, col, "%s %-20s %.2f", (selected) ? ">" : "",
               p->firstName, rating(p));
    }
  }
}

void renderSelectedList(tuidb* tui) {
  int startCol = tui->allPlayersArea->area->width + 5;
  int line = 2;
  int borderStartLine = 1;
  int len = min_int(tui->players->n, tui->term->rows - 1 - line - borderStartLine);
  int borderHeight = len + 4 - borderStartLine;
  int borderWidth = 35;

  make_borders_color(tui->render, startCol - 2, borderStartLine, borderWidth, borderHeight, DEFAULT_FG);

  const int total_size = tui->teams_n * tui->team_size;
  put_text(tui->render, 1, startCol, "Selected %s%d/%d%s",
           ((int)tui->players->n > total_size) ? "\033[31m" : "",
           (int)tui->players->n, total_size, "\033[0m");

  for (int i = 0; i < len; i++) {
    player* p = tui->players->items[i];
    put_text(tui->render, line++, startCol, " %-20s %.2f", p->firstName, rating(p));
  }
  if ((int)tui->players->n > len) {
    put_text(tui->render, borderHeight - borderStartLine, startCol,
             "... %d more players", tui->players->n - len);
  }
}

void renderPlayerRelations(tuidb* tui, player* p, int startCol, int startLine) {
  int line = startLine;
  put_text(tui->render, line, startCol, "Former teammates:");
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

void renderPlayerInfo(tuidb* tui) {
  player* p = selectedPlayer(tui);
  if (!p) return;
  int startCol = tui->allPlayersArea->area->width + 5;
  int line = 2;
  int borderStartLine = 1;
  int borderHeight = min_int(p->positions->n + p->skills->n + 9, tui->term->rows) -  borderStartLine;
  int borderWidth = 35;

  make_borders_color(tui->render, startCol - 2, borderStartLine, borderWidth,
                     borderHeight,
                     (tui->active_area == PLAYER_EDIT) ? BLUE_FG : DEFAULT_FG);

  put_text(tui->render, line++, startCol, "Name: %s %s",
           p->firstName ? p->firstName : "", p->surName ? p->surName : "");
  put_text(tui->render, line++, startCol, "ID: %d", p->id);

  // Skills list
  line++;
  if (p->skills->n == 0) {
    put_text(tui->render, line++, startCol,
             tui->p_edit->active && tui->p_edit->selected_element == SKILLS_LIST
                 ? "\033[7m%s\033[27m"
                 : "\033[2m%s\033[22m", "No skills");
  } else {
    put_text(tui->render, line++, startCol, "\033[2m%-15s %.2f\033[22m", "Skills:", rating(p));
    for (size_t i = 0; i < p->skills->n; i++) {
      skill* s = p->skills->items[i];
      int is_selected = tui->p_edit->active &&
                        tui->p_edit->selected_element == SKILLS_LIST &&
                        (tui->p_edit->lists_index == (int)i);
      put_text(tui->render, line++, startCol,
               is_selected ? "\033[7m%-15s %.2f\033[27m" : "%-15s %.2f",
               s->name, s->value);
    }
  }

  // Positions list
  line++;
  if (p->positions->n == 0) {
    put_text(tui->render, line++, startCol,
             tui->p_edit->active && tui->p_edit->selected_element == POSITIONS_LIST
                 ? "\033[7m%s\033[27m"
                 : "\033[2m%s\033[22m", "No positions");
  } else {
    put_text(tui->render, line++, startCol, "\033[2mPositions:\033[22m");
    for (size_t i = 0; i < p->positions->n; i++) {
      position* pos = p->positions->items[i];
      int is_selected = tui->p_edit->active &&
                        tui->p_edit->selected_element == POSITIONS_LIST &&
                        (tui->p_edit->lists_index == (int)i);
      put_text(tui->render, line++, startCol,
               is_selected ? "\033[7m%d %-15s\033[27m" : "%d %-15s",
               pos->priority, pos->name);
    }
  }

  if (!tui->p_edit->active) {
    renderPlayerRelations(tui, p, startCol + borderWidth + 2, 1);
  }
}

void renderAllTeamsList(tuidb* tui) {
  int col = start_print_col(tui->allTeamsArea->area);
  int line = start_print_line(tui->allTeamsArea->area);
  int len = getListAreaLen(tui->allTeamsArea, tui->term->rows);

  draw_area_borders(tui->render, tui->allTeamsArea->area, BLUE_FG);

  put_text(tui->render, tui->allTeamsArea->area->start_row + 1, 2,
           "\033[4m %-20s\033[24m", "Name");
  put_text(tui->render, area_last_line(tui->allTeamsArea->area), 2, "%d/%d",
           tui->allTeamsArea->selected + 1, (int)tui->allTeams->n);

  for (int i = tui->allTeamsArea->first_ind; i < tui->allTeamsArea->first_ind + len; i++) {
    team* t = tui->allTeams->items[i];
    if (tui->allTeamsArea->selected == i) {
      tui->allTeamsArea->selected_term_row = line + 1;
      put_text(tui->render, line++, col, "\033[7m%-20s\033[27m", t->name);
    } else {
      put_text(tui->render, line++, col, "%-20s", t->name);
    }
  }
}

void renderSelectedTeam(tuidb* tui) {
  team* t = selectedTeam(tui);
  if (!t) return;
  dlist* player_ids = fetchPlayersInTeam(tui->db, t);
  int startCol = tui->allTeamsArea->area->width + 5;
  int borderStartLine = 1;
  int borderHeight = min_int(player_ids->n + 5, tui->term->rows) -  borderStartLine;
  int borderWidth = 35;
  int line = borderStartLine + 2;

  make_borders_color(tui->render, startCol - 2, borderStartLine, borderWidth, borderHeight, DEFAULT_FG);

  put_text(tui->render, borderStartLine, startCol, "%s", t->name);
  put_text(tui->render, borderStartLine, startCol + 25, "%d", t->id);

  for (size_t i = 0; i < player_ids->n; i++) {
    int* id = player_ids->items[i];
    int ind = playerInList(tui->allPlayers, *id);
    if (ind >= 0) {
      player* p = tui->allPlayers->items[ind];
      put_text(tui->render, line, startCol, "%s %s",
               p->firstName ? p->firstName : "", p->surName ? p->surName : "");
    } else {
      put_text(tui->render, line, startCol, "Unidentified");
    }
    put_text(tui->render, line++, startCol + 25, "%d", *id);
    free(id);
  }
  free_list(player_ids);
}


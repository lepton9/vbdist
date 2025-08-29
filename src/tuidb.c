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
  tui->allPositions = NULL;

  tui->allPlayersArea = init_list_area(BASE_SECTION_WIDTH, BASE_LIST_LEN);
  tui->allTeamsArea = init_list_area(BASE_SECTION_WIDTH, BASE_LIST_LEN);
  set_area_pos(tui->allPlayersArea->area, 1, 0);
  set_padding(tui->allPlayersArea->area, 2, 1, 2, 2);
  set_area_pos(tui->allTeamsArea->area, 1, 0);
  set_padding(tui->allTeamsArea->area, 2, 1, 2, 2);

  tui->p_edit = initPlayerEdit(BASE_SECTION_WIDTH, BASE_LIST_LEN);
  set_area_pos(tui->p_edit->positionsArea->area, 1,
               tui->allPlayersArea->area->width * 2);
  set_padding(tui->p_edit->positionsArea->area, 0, 0, 2, 2);

  tui->selectedTeams = init_list();

  tui->tab = PLAYERS_TAB;
  tui->active_area = PLAYERS_LIST;
  tui->show_player_info = 0;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);
  tui->render = init_renderer(stdout);
  tui->exit = 0;

  return tui;
}

void freeAllPlayers(dlist* allPlayers) {
  if (allPlayers) {
    for (size_t i = 0; i < allPlayers->n; i++) {
      freePlayer(get_elem(allPlayers, i));
    }
    free_list(allPlayers);
  }
}

void freeAllTeams(dlist* allTeams) {
  if (allTeams) {
    for (size_t i = 0; i < allTeams->n; i++) {
      freeTeam(get_elem(allTeams, i));
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

void setAllPositions(tuidb* tui, dlist* positions) {
  tui->allPositions = positions;
}

void freeTuiDB(tuidb* tui) {
  if (!tui) return;
  free_list(tui->selectedTeams);
  freeAllPlayers(tui->allPlayers);
  freeAllTeams(tui->allTeams);
  free_renderer(tui->render);
  free_list_area(tui->allPlayersArea);
  free_list_area(tui->allTeamsArea);
  freePlayerEdit(tui->p_edit);
  tui->allPositions = NULL;
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
  if (tui->allPlayersArea->selected < 0) return NULL;
  return get_elem(tui->allPlayers, tui->allPlayersArea->selected);
}

team* selectedTeam(tuidb *tui) {
  if (tui->allTeamsArea->selected < 0) return NULL;
  return get_elem(tui->allTeams, tui->allTeamsArea->selected);
}

void selectPlayer(tuidb* tui, int index) {
  if (index < 0) return;
  player* p = get_elem(tui->allPlayers, index);
  if (playerInList(tui->players, p->id) < 0) {
    list_add(tui->players, copyPlayer(p));
  }
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

void select_all(tuidb* tui) {
  for (size_t i = 0; i < tui->allPlayers->n; i++) {
    selectPlayer(tui, i);
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

void fillTeamTemp(tuidb* tui, team* team) {
  dlist* player_ids = fetchPlayersInTeam(tui->db, team);
  team->size = player_ids->n;
  team->players = realloc(team->players, team->size * sizeof(player*));
  for (size_t i = 0; i < player_ids->n; i++) {
    int* id = player_ids->items[i];
    team->players[i] = getPlayerInList(tui->allPlayers, *id);
    free(id);
  }
  free_list(player_ids);
}

int validateTeamEditSelect(dlist* selectedTeams, team* cur_team) {
  for (size_t i = 0; i < selectedTeams->n; i++) {
    team* t = get_elem(selectedTeams, i);
    if (cur_team->size != t->size) return 0;
    if (checkPlayerCollisions(t, cur_team)) return 0;
  }
  return 1;
}

void tuidb_list_up(tuidb* tui) {
  switch (tui->tab) {
    case PLAYERS_TAB: {
      if (tui->active_area == PLAYERS_LIST) list_up(tui->allPlayersArea);
      else if (tui->active_area == PLAYER_EDIT && tui->p_edit->active) {
        pedit_list_up(tui->p_edit);
      } else if (tui->active_area == POSITIONS_LIST_EDIT && tui->p_edit->active) {
        list_up(tui->p_edit->positionsArea);
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
        pedit_list_down(tui->p_edit);
      } else if (tui->active_area == POSITIONS_LIST_EDIT && tui->p_edit->active) {
        list_down(tui->p_edit->positionsArea);
      }
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
  if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYERS_LIST) {
    player* p = selectedPlayer(tui);
    if (!p) return;
    old_name = (char*)playerName(p);
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
            updatePlayerName(p, strdup(new));
            player* pSel = getPlayerInList(tui->players, p->id);
            if (pSel) {
              updatePlayerName(pSel, strdup(new));
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
    name = (char*)playerName(p);
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

void exit_edit_player(tuidb* tui) {
  if (!tui->p_edit->active || !tui->p_edit->p) return;
  if (tui->p_edit->modified) {
    updatePlayer(tui->db, tui->p_edit->p);
    int ind = playerInList(tui->players, tui->p_edit->p->id);
    if (ind >= 0) {
      fetchPlayer(tui->db, get_elem(tui->players, ind));
    }
    tui->p_edit->modified = 0;
  }
  tui->p_edit->active = 0;
  tui->p_edit->p = NULL;
  tui->active_area = PLAYERS_LIST;
}

void pedit_handle_inc(tuidb* tui, char big_inc) {
  if (!tui->p_edit->active || !tui->p_edit->p) return;
  switch (tui->p_edit->selected_element) {
    case SKILLS_LIST: {
      skill* s = pedit_selected_skill(tui->p_edit);
      if (s) {
        incValue(s, big_inc);
        tui->p_edit->modified = 1;
      }
      break;
    }
    case POSITIONS_LIST: {
      player* p = tui->p_edit->p;
      int pos_ind_sel = tui->p_edit->lists_index;
      int pos_ind_above = tui->p_edit->lists_index - 1;
      if (pos_ind_above >= 0 && pos_ind_sel < (int)p->positions->n) {
        swapPositions(p, pos_ind_sel, pos_ind_above);
        tui->p_edit->modified = 1;
        tui->p_edit->lists_index = pos_ind_above;
      }
      break;
    }
    default:
      break;
  }
}

void pedit_handle_dec(tuidb* tui, char big_dec) {
  if (!tui->p_edit->active || !tui->p_edit->p) return;
  switch (tui->p_edit->selected_element) {
    case SKILLS_LIST: {
      skill* s = pedit_selected_skill(tui->p_edit);
      if (s) {
        decValue(s, big_dec);
        tui->p_edit->modified = 1;
      }
      break;
    }
    case POSITIONS_LIST: {
      player* p = tui->p_edit->p;
      int pos_ind_sel = tui->p_edit->lists_index;
      int pos_ind_below = tui->p_edit->lists_index + 1;
      if (pos_ind_sel >= 0 && pos_ind_below < (int)p->positions->n) {
        swapPositions(p, pos_ind_sel, pos_ind_below);
        tui->p_edit->modified = 1;
        tui->p_edit->lists_index = pos_ind_below;
      }
      break;
    }
    default:
      break;
  }
}

void pedit_add(tuidb* tui) {
  switch (tui->p_edit->selected_element) {
    case SKILLS_LIST:
      break;
    case POSITIONS_LIST: {
      pedit_filtered_positions(tui->p_edit, tui->allPositions);
      tui->active_area = POSITIONS_LIST_EDIT;
      break;
    }
    default:
      break;
  }
}

void pedit_remove(tuidb* tui) {
  switch (tui->p_edit->selected_element) {
    case SKILLS_LIST:
      break;
    case POSITIONS_LIST: {
      pedit_remove_position(tui->p_edit);
      break;
    }
    default:
      break;
  }
}

void handle_esc(tuidb* tui) {
  if (tui->active_area == PLAYER_EDIT) {
    exit_edit_player(tui);
  } else if (tui->active_area == POSITIONS_LIST_EDIT) {
    pedit_reset_positions(tui->p_edit);
    tui->active_area = PLAYER_EDIT;
  } else {
    tui->show_player_info = 0;
  }
}

void handleAdd(tuidb* tui) {
  switch (tui->tab) {
    case PLAYERS_TAB:
      if (tui->active_area == PLAYER_EDIT) {
        pedit_add(tui);
      }
      break;
    case TEAMS_TAB:
      break;
  }
}

void handleRemove(tuidb* tui) {
  switch (tui->tab) {
    case PLAYERS_TAB:
      if (tui->active_area == PLAYER_EDIT) {
        pedit_remove(tui);
      } else {
        deleteSelectedListElem(tui);
      }
      break;
    case TEAMS_TAB:
      deleteSelectedListElem(tui);
      break;
  }
}

void handleExit(tuidb* tui) {
  exit_edit_player(tui);
  tui->exit = 1;
}

void handleKeyPress(tuidb* tui, int c) {
  switch (c) {
    case 'q': case 'Q':
      handleExit(tui);
      break;
    case 13: case '\n': case ' ':
#ifdef __linux__
    case KEY_ENTER:
#endif
    if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYERS_LIST) {
      selectCurPlayer(tui);
    } else if (tui->tab == PLAYERS_TAB && tui->active_area == POSITIONS_LIST_EDIT) {
      pedit_add_position(tui->p_edit);
      if (tui->p_edit->positions->n == 0) {
        tui->active_area = PLAYER_EDIT;
      }
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
#ifdef __linux__
    case KEY_RIGHT:
#endif
    case 'l': case 'L':
      if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYER_EDIT &&
          tui->p_edit->selected_element == SKILLS_LIST) {
        tui->p_edit->selected_element = POSITIONS_LIST;
        tui->p_edit->lists_index = 0;
      }
      break;
#ifdef __linux__
    case KEY_LEFT:
#endif
    case 'h': case 'H':
      if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYER_EDIT &&
          tui->p_edit->selected_element == POSITIONS_LIST) {
        tui->p_edit->selected_element = SKILLS_LIST;
        tui->p_edit->lists_index = 0;
      }
      break;
    case '-':
      pedit_handle_dec(tui, 0);
      break;
    case 4: // Ctrl + D
      pedit_handle_dec(tui, 1);
      break;
    case '+':
      pedit_handle_inc(tui, 0);
      break;
    case 21: // Ctrl + U
      pedit_handle_inc(tui, 1);
      break;
    case 'a': case 'A':
      handleAdd(tui);
      break;
    case 'd': case 'D':
      handleRemove(tui);
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
      if (tui->tab == PLAYERS_TAB && tui->active_area == PLAYERS_LIST)
        unselect_all(tui);
      break;
    case 1: // Ctrl + A
      select_all(tui);
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
  while (!tui->exit) {
    updateArea(tui);
    renderTuidb(tui);
    c = keyPress();
    handleKeyPress(tui, c);
  }
  tui->exit = 0;
}

void updateArea(tuidb* tui) {
  getTermSize(tui->term);
  int height = min_int(tui->term->rows, BASE_LIST_LEN);
  int width = min_int(tui->term->cols / 2, BASE_SECTION_WIDTH);
  update_list_area(tui->allPlayersArea, width, height);
  update_list_area(tui->allTeamsArea, width, height);
  update_list_area_fit(tui->p_edit->positionsArea,
                       max_int(width - AREA_SPACING, 1), height);
  if (tui->active_area == POSITIONS_LIST_EDIT) {
    int startCol = tui->allPlayersArea->area->width * 2 + 1;
    if (startCol + (int)tui->p_edit->positionsArea->area->width < tui->term->cols) {
      set_area_pos(tui->p_edit->positionsArea->area, 1, startCol);
    } else {
      set_area_pos(tui->p_edit->positionsArea->area,
                   playerInfoBoxHeight(tui, selectedPlayer(tui)) + 1,
                   tui->allPlayersArea->area->width + AREA_SPACING);
    }
  }
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
  int len = getListAreaLen(tui->allPlayersArea);
  size_t name_width =
      max_int(tui->allPlayersArea->area->width -
                  area_width_empty(tui->allPlayersArea->area) - 10,
              1);
  char player_name[name_width];
  player_name[0] = '\0';

  draw_area_borders(tui->render, tui->allPlayersArea->area, (tui->active_area == PLAYERS_LIST) ? BLUE_FG : DEFAULT_FG);

  put_text(tui->render, tui->allTeamsArea->area->start_row + 1, 2,
           "\033[4m %-*s %s \033[24m", name_width, "Name", "Rating");
  put_text(tui->render, area_last_line(tui->allPlayersArea->area), 2, "%d/%d",
           tui->allPlayersArea->selected + 1, (int)tui->allPlayers->n);

  for (int i = tui->allPlayersArea->first_ind; i < tui->allPlayersArea->first_ind + len; i++) {
    player* p = tui->allPlayers->items[i];
    snprintf(player_name, name_width, "%s", playerFullName(p));
    char selected = playerInList(tui->players, ((player *)tui->allPlayers->items[i])->id) >= 0;

    if (tui->allPlayersArea->selected == i) {
      set_selected_row(tui->allPlayersArea, line);
      put_text(tui->render, line++, col, "\033[7m%s %-*s %.2f\033[27m",
               (selected) ? ">" : "", name_width, player_name, rating(p));
    } else {
      put_text(tui->render, line++, col, "%s %-*s %.2f", (selected) ? ">" : "",
               name_width, player_name, rating(p));
    }
    player_name[0] = '\0';
  }
}

void renderSelectedList(tuidb* tui) {
  int startCol = tui->allPlayersArea->area->width + AREA_SPACING +
                 tui->allPlayersArea->area->pad->left;
  int line = 2;
  int borderStartLine = tui->allPlayersArea->area->start_row;
  int len = min_int(tui->players->n, tui->term->rows - 1 - line - borderStartLine);
  int borderHeight = len + 4 - borderStartLine;
  int borderWidth = max_int((int)tui->allPlayersArea->area->width - AREA_SPACING, 1);
  size_t name_width =
      max_int(borderWidth - area_width_empty(tui->allPlayersArea->area) - 5, 1);
  char player_name[name_width];
  player_name[0] = '\0';

  make_borders_color(tui->render,
                     tui->allPlayersArea->area->width + AREA_SPACING,
                     borderStartLine, borderWidth, borderHeight, DEFAULT_FG);

  const int total_size = tui->teams_n * tui->team_size;
  put_text(tui->render, 1, startCol, "Selected %s%d/%d%s",
           ((int)tui->players->n > total_size) ? "\033[31m" : "",
           (int)tui->players->n, total_size, "\033[0m");

  for (int i = 0; i < len; i++) {
    player* p = tui->players->items[i];
    snprintf(player_name, name_width, "%s", playerFullName(p));
    put_text(tui->render, line++, startCol, " %-*s %.2f", name_width,
             player_name, rating(p));
    player_name[0] = '\0';
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
      put_text(tui->render, line++, startCol, "%3d %s", t->b, playerFullName(p));
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
      put_text(tui->render, line++, startCol, "%s", playerFullName(p));
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

void renderPlayerEditPos(tuidb *tui) {
  int col = start_print_col(tui->p_edit->positionsArea->area);
  int line = start_print_line(tui->p_edit->positionsArea->area);
  int len = getListAreaLen(tui->p_edit->positionsArea);
  list_area* area = tui->p_edit->positionsArea;

  draw_area_borders(tui->render, area->area, BLUE_FG);
  put_text(tui->render, area->area->start_row, col, "Add positions");

  for (int i = area->first_ind; i < area->first_ind + len; i++) {
    position* pos = get_elem(tui->p_edit->positions, i);
    if (area->selected == i) {
      set_selected_row(area, line);
      put_text(tui->render, line++, col, "\033[7m%-20s\033[27m", pos->name);
    } else {
      put_text(tui->render, line++, col, "%-20s", pos->name);
    }
  }
}

int playerInfoBoxHeight(tuidb* tui, player* p) {
  if (!p) return 2;
  return min_int(p->positions->n + p->skills->n + 9, tui->term->rows) -
         tui->allPlayersArea->area->start_row;
}

void renderPlayerInfo(tuidb* tui) {
  player* p = selectedPlayer(tui);
  if (!p) return;
  int startCol = tui->allPlayersArea->area->width + AREA_SPACING +
                 tui->allPlayersArea->area->pad->left;
  int borderStartLine = tui->allPlayersArea->area->start_row;
  int borderHeight = playerInfoBoxHeight(tui, p);
  int borderWidth = max_int(tui->allPlayersArea->area->width - AREA_SPACING, 1);
  int line = borderStartLine + 1;

  make_borders_color(tui->render,
                     tui->allPlayersArea->area->width + AREA_SPACING,
                     borderStartLine, borderWidth, borderHeight,
                     (tui->active_area == PLAYER_EDIT) ? BLUE_FG : DEFAULT_FG);

  put_text(tui->render, line++, startCol, "Name: %s", playerFullName(p));
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
  } else if (tui->active_area == POSITIONS_LIST_EDIT) {
    renderPlayerEditPos(tui);
  }
}

void renderAllTeamsList(tuidb* tui) {
  int col = start_print_col(tui->allTeamsArea->area);
  int line = start_print_line(tui->allTeamsArea->area);
  int len = getListAreaLen(tui->allTeamsArea);

  size_t name_width = max_int(tui->allTeamsArea->area->width -
                                  area_width_empty(tui->allTeamsArea->area),
                              1);
  char team_name[name_width];
  team_name[0] = '\0';

  draw_area_borders(tui->render, tui->allTeamsArea->area, BLUE_FG);

  put_text(tui->render, tui->allTeamsArea->area->start_row + 1, 2,
           "\033[4m %-*s\033[24m", name_width, "Name");
  put_text(tui->render, area_last_line(tui->allTeamsArea->area), 2, "%d/%d",
           tui->allTeamsArea->selected + 1, (int)tui->allTeams->n);

  for (int i = tui->allTeamsArea->first_ind; i < tui->allTeamsArea->first_ind + len; i++) {
    team* t = get_elem(tui->allTeams, i);
    snprintf(team_name, name_width, " %s", t->name);
    if (tui->allTeamsArea->selected == i) {
      set_selected_row(tui->allTeamsArea, line);
      put_text(tui->render, line++, col, "\033[7m%-*s\033[27m", name_width,
               team_name);
    } else {
      put_text(tui->render, line++, col, "%-*s", name_width, team_name);
    }
    team_name[0] = '\0';
  }
}

void renderSelectedTeam(tuidb* tui) {
  team* t = selectedTeam(tui);
  if (!t) return;
  dlist* player_ids = fetchPlayersInTeam(tui->db, t);
  int startCol = tui->allTeamsArea->area->width + AREA_SPACING +
                 tui->allTeamsArea->area->pad->left;
  int borderStartLine = tui->allTeamsArea->area->start_row;
  int borderHeight = min_int(player_ids->n + 5, tui->term->rows) -  borderStartLine;
  int borderWidth = max_int(tui->allTeamsArea->area->width - AREA_SPACING, 1);
  int line = borderStartLine + 2;

  size_t name_width =
      max_int(borderWidth - area_width_empty(tui->allPlayersArea->area) - 5, 1);
  char player_text[name_width];
  player_text[0] = '\0';

  make_borders_color(tui->render, tui->allTeamsArea->area->width + AREA_SPACING,
                     borderStartLine, borderWidth, borderHeight, DEFAULT_FG);

  put_text(tui->render, borderStartLine, startCol, "%s", t->name);
  put_text(tui->render, borderStartLine, startCol + name_width, "%d", t->id);

  for (size_t i = 0; i < player_ids->n; i++) {
    int* id = player_ids->items[i];
    int ind = playerInList(tui->allPlayers, *id);
    if (ind >= 0) {
      player* p = tui->allPlayers->items[ind];
      snprintf(player_text, name_width, "%s", playerFullName(p));
      put_text(tui->render, line, startCol, "%-*s", name_width, player_text);
    } else {
      put_text(tui->render, line, startCol, "Unidentified");
    }
    put_text(tui->render, line++, startCol + name_width, "%d", *id);
    free(id);
    player_text[0] = '\0';
  }
  free_list(player_ids);
}


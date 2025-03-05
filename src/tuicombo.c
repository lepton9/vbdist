#include "../include/tuicombo.h"
#include "../include/utils.h"

tui_combos* init_tui_combo(sqldb* db, dlist* players) {
  tui_combos* tui = malloc(sizeof(tui_combos));
  tui->db = db;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);
  tui->render = init_renderer(stdout);

  tui->mode = CTUI_PLAYER_LIST;

  tui->cur_combo = NULL;
  tui->recording_combo = 0;

  tui->combos = fetchAllCombos(db);
  tui->players = players;

  tui->combos_area = init_list_area(tui->term->cols, tui->term->rows);
  tui->players_area = init_list_area(tui->term->cols, tui->term->rows);
  update_list_len(tui->players_area, tui->players->n);
  update_list_len(tui->combos_area, tui->combos->n);
  return tui;
}

void free_tui_combo(tui_combos* tui) {
  free(tui->term);
  free_renderer(tui->render);
  free_list_area(tui->players_area);
  free_list_area(tui->combos_area);
  freeCombos(tui->combos);
  free(tui);
}

void changeComboTuiMode(tui_combos* tui) {
  if (tui->mode == CTUI_PLAYER_LIST) {
    tui->mode = CTUI_COMBO_LIST;
  } else {
    tui->mode = CTUI_PLAYER_LIST;
  }
}

void runTuiCombo(sqldb* db, dlist* players) {
  tui_combos* tui = init_tui_combo(db, players);
  curHide();
  refresh_screen(tui->render);
  int c = 0;
  while (c != 'q') {
    check_selected(tui->combos_area);
    check_selected(tui->players_area);
    updateTuiComboAreas(tui);
    renderComboTui(tui);
    c = keyPress();
    handleComboTuiInput(tui, c);
  }
}

void updateTuiComboAreas(tui_combos* tui) {
  getTermSize(tui->term);
  int rows = tui->term->rows - 4;
  int cols = tui->term->cols / 2;
  update_list_area(tui->combos_area, min_int(cols, 50), rows);
  update_list_area(tui->players_area, min_int(cols, 30), rows);
}

void comboTuiListUp(tui_combos* tui) {
  switch (tui->mode) {
    case CTUI_PLAYER_LIST:
      list_up(tui->players_area);
      break;
    case CTUI_COMBO_LIST:
      list_up(tui->combos_area);
      break;
  }
}

void comboTuiListDown(tui_combos* tui) {
  switch (tui->mode) {
    case CTUI_PLAYER_LIST:
      list_down(tui->players_area);
      break;
    case CTUI_COMBO_LIST:
      list_down(tui->combos_area);
      break;
  }
}

void handleComboTuiInput(tui_combos* tui, int c) {
  switch (c) {
    case 13: case '\n': case ' ':
#ifdef __linux__
    case KEY_ENTER:
#endif
      break;
    case 27: {  // Esc
      break;
    }
    case 9: // Tab
      changeComboTuiMode(tui);
      break;
    case 'R': case 'r':
      break;
    case 'A': case 'a':
      break;
    case 'X': case 'x':
      break;
    case 'K': case 'W':
    case 'k': case 'w':
#ifdef __linux__
    case KEY_UP:
#endif
      comboTuiListUp(tui);
      break;
    case 'j': case 's':
#ifdef __linux__
    case KEY_DOWN:
#endif
      comboTuiListDown(tui);
      break;
    default: {
      break;
    }
  }
}

void renderComboTui(tui_combos* tui) {
  ctuiRenderPlayersArea(tui);
  ctuiRenderCombosArea(tui);
  render(tui->render);
}

int getListAreaLen(list_area* area, term_size* term, int start_line) {
  if (area->len == 0) return 0;
  return min_int(min_int(term->rows - start_line, (int)area->max_shown),
                 (int)area->len - (area->first_ind));
}

void ctuiRenderPlayersArea(tui_combos* tui) {
  int col = 2;
  int line = 2;
  int len = getListAreaLen(tui->players_area, tui->term, line);

  char player_text[100];
  player_text[0] = '\0';

  for (int i = tui->players_area->first_ind; i < tui->players_area->first_ind + len; i++) {
    player* p = tui->players->items[i];
    strcat(player_text, p->firstName);
    if (tui->players_area->selected == i) {
      tui->players_area->selected_term_row = line + 1;
      put_text(tui->render, line++, col, "\033[7m %-20s\033[27m", player_text);
    } else {
      put_text(tui->render, line++, col, " %-20s", player_text);
    }
    player_text[0] = '\0';
  }
  make_borders(tui->render, 0, 0, tui->players_area->width, len + 4);
  put_text(tui->render, 0, 3, "%s", "Selected players");
}

void ctuiRenderCombosArea(tui_combos* tui) {
  int col = tui->players_area->width + 5;
  int line = 2;
  int len = getListAreaLen(tui->combos_area, tui->term, line);

  char combo_text[100];
  combo_text[0] = '\0';

  for (int i = tui->combos_area->first_ind; i < tui->combos_area->first_ind + len; i++) {
    pCombo* combo = tui->combos->items[i];
    player* p1 = getPlayerInList(tui->players, combo->pidA);
    player* p2 = getPlayerInList(tui->players, combo->pidB);
    sprintf(combo_text, "(%s) %s - %s", comboTypeString(combo->type), p1->firstName, p2->firstName);
    put_text(tui->render, line++, col + 1, " %-20s", combo_text);
    combo_text[0] = '\0';
  }

  make_borders(tui->render, col, 0, tui->combos_area->width, len + 4);
  put_text(tui->render, 0, col + 3, "%s", "Combos");
}



#include "../include/tuicombo.h"
#include "../include/utils.h"

tui_combos* init_tui_combo(sqldb* db, dlist* players) {
  tui_combos* tui = malloc(sizeof(tui_combos));
  tui->db = db;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);
  tui->render = init_renderer(stdout);

  tui->mode = CTUI_PLAYER_LIST;

  tui->cur_combo = init_list();
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
  free_list(tui->cur_combo);
  free(tui);
}

void changeComboTuiMode(tui_combos* tui) {
  if (tui->recording_combo) return;
  if (tui->mode == CTUI_PLAYER_LIST) {
    tui->mode = CTUI_COMBO_LIST;
  } else {
    tui->mode = CTUI_PLAYER_LIST;
  }
}

void start_combo(tui_combos* tui, comboType type) {
  if (tui->recording_combo) return;
  tui->recording_combo = 1;
  tui->cur_combo_type = type;
  tui->mode = CTUI_PLAYER_LIST;
}

void end_combo(tui_combos* tui) {
  if (!tui->recording_combo) return;
  tui->recording_combo = 0;
  if (tui->cur_combo->n == 0) return;
  insert_cur_combo(tui);

  for (size_t i = 0; i < tui->cur_combo->n; i++) {
    free(tui->cur_combo->items[i]);
    tui->cur_combo->items[i] = NULL;
  }
  tui->cur_combo->n = 0;
}

// TODO:
void insert_cur_combo(tui_combos* tui) {
  for (size_t i = 0; i < tui->cur_combo->n; i++) {

  }
}

int inCurCombo(dlist* ids, int id) {
  for (size_t i = 0; i < ids->n; i++) {
    if (*((int*)ids->items[i]) == id) return i;
  }
  return -1;
}

void ctuiSelectPlayer(tui_combos* tui) {
  if (!tui->recording_combo || tui->mode != CTUI_PLAYER_LIST) return;
  player* p = tui->players->items[tui->players_area->selected];
  int i = inCurCombo(tui->cur_combo, p->id);
  if (i >= 0) {
    free(pop_elem(tui->cur_combo, i));
  } else {
    int* id = malloc(sizeof(int));
    *id = p->id;
    list_add(tui->cur_combo, id);
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
    {
      if (tui->mode == CTUI_PLAYER_LIST) {
        ctuiSelectPlayer(tui);
      } else if (tui->mode == CTUI_COMBO_LIST) {
      }
      break;
    }
    case 27: {  // Esc
      break;
    }
    case 9: // Tab
      changeComboTuiMode(tui);
      break;

    case 'b': // TODO: new keymaps?
      start_combo(tui, BAN);
      break;
    case 'p': // TODO: new keymaps?
      start_combo(tui, PAIR);
      break;
    case 'e': // TODO: new keymaps?
      end_combo(tui);
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
    }
    else if (tui->recording_combo && inCurCombo(tui->cur_combo, p->id) >= 0) {
      if (tui->cur_combo_type == BAN) {
        put_text(tui->render, line++, col-1, "*\033[31m %-20s\033[0m", player_text);
      } else if (tui->cur_combo_type == PAIR) {
        put_text(tui->render, line++, col-1, "*\033[32m %-20s\033[0m", player_text);
      }
    } else {
      put_text(tui->render, line++, col, " %-20s", player_text);
    }
    player_text[0] = '\0';
  }
  make_borders(tui->render, 0, 0, tui->players_area->width, len + 4);
  put_text(tui->render, 0, 3, "%s", "Selected players");

  if (tui->recording_combo) {
    put_text(tui->render, 0, 22, "(%s)", comboTypeString(tui->cur_combo_type));
  }
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

    if (tui->combos_area->selected == i) {
      tui->combos_area->selected_term_row = line + 1;
      put_text(tui->render, line++, col + 1, "\033[7m %s\033[27m", combo_text);
    } else {
      put_text(tui->render, line++, col + 1, "%s", combo_text);
    }
    combo_text[0] = '\0';
  }

  make_borders(tui->render, col, 0, tui->combos_area->width, len + 4);
  put_text(tui->render, 0, col + 3, "%s", "Combos");
}



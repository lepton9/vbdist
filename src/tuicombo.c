#include "../include/tuicombo.h"
#include "../include/utils.h"
#include <string.h>

#define BASE_COMBOS_WIDTH 50
#define BASE_PLAYERS_WIDTH 40

tui_combos* init_tui_combo(sqldb* db, dlist* players) {
  tui_combos* tui = malloc(sizeof(tui_combos));
  tui->db = db;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);
  tui->render = init_renderer(stdout);

  tui->exit = 0;
  tui->mode = CTUI_PLAYER_LIST;

  tui->cur_combo = NULL;
  tui->recording_combo = 0;

  tui->combos = fetchAllCombos(db);
  tui->players = players;

  for (int i = tui->combos->n - 1; i >= 0; i--) {
    if (!comboRelevant(tui->players, tui->combos->items[i])) {
      freeCombo(pop_elem(tui->combos, i));
    }
  }

  tui->combos_area = init_list_area(BASE_COMBOS_WIDTH, tui->term->rows);
  tui->players_area = init_list_area(BASE_PLAYERS_WIDTH, tui->term->rows);
  set_area_pos(tui->players_area->area, 0, 0);
  set_padding(tui->players_area->area, 1, 1, 2, 2);
  set_area_pos(tui->combos_area->area, 0, tui->players_area->area->width + 3);
  set_padding(tui->combos_area->area, 1, 1, 2, 2);

  update_list_len(tui->players_area, tui->players->n);
  update_list_len(tui->combos_area, tui->combos->n);
  return tui;
}

int editing_combo(tui_combos* tui) {
  return tui->recording_combo && tui->cur_combo;
}

void free_tui_combo(tui_combos* tui) {
  if (editing_combo(tui)) {
    end_combo(tui);
  }
  free(tui->term);
  free_renderer(tui->render);
  free_list_area(tui->players_area);
  free_list_area(tui->combos_area);
  freeCombos(tui->combos);
  if (tui->cur_combo) freeCombo(tui->cur_combo);
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

void start_edit_combo(tui_combos* tui) {
  if (tui->recording_combo || tui->cur_combo) return;
  int ind = tui->combos_area->selected;
  if (ind < 0 || tui->combos->n == 0) return;
  tui->cur_combo = tui->combos->items[ind];
  tui->recording_combo = 1;
  tui->mode = CTUI_PLAYER_LIST;
}

void start_combo(tui_combos* tui, comboType type) {
  if (tui->recording_combo || tui->cur_combo) return;
  tui->cur_combo = initCombo(type, -1);
  tui->recording_combo = 1;
  tui->mode = CTUI_PLAYER_LIST;
}

void end_combo(tui_combos* tui) {
  if (!tui->recording_combo || !tui->cur_combo) return;
  tui->recording_combo = 0;
  int added = 0;
  if (tui->cur_combo->combo_id >= 0) {
    if (tui->cur_combo->ids->n < 2) {
      deleteCurCombo(tui);
    } else {
      updateCombo(tui->db, tui->cur_combo);
    }
    tui->mode = CTUI_COMBO_LIST;
  } else {
    if (tui->cur_combo->ids->n > 1) {
      added = insert_cur_combo(tui);
    }
    if (!added) freeCombo(tui->cur_combo);
  }
  tui->cur_combo = NULL;
}

int insert_cur_combo(tui_combos* tui) {
  int r = insertCombo(tui->db, tui->cur_combo);
  if (r) {
    list_add(tui->combos, tui->cur_combo);
    update_list_len(tui->combos_area, tui->combos->n);
  }
  return r;
}

int inCurCombo(combo* combo, int id) {
  if (!combo) return -1;
  for (size_t i = 0; i < combo->ids->n; i++) {
    if (*((int*)combo->ids->items[i]) == id) return i;
  }
  return -1;
}

void ctuiSelectPlayer(tui_combos* tui) {
  if (!tui->recording_combo || tui->mode != CTUI_PLAYER_LIST || !tui->cur_combo) return;
  player* p = tui->players->items[tui->players_area->selected];
  int i = inCurCombo(tui->cur_combo, p->id);
  if (i >= 0) {
    free(pop_elem(tui->cur_combo->ids, i));
  } else {
    int* id = malloc(sizeof(int));
    *id = p->id;
    list_add(tui->cur_combo->ids, id);
  }
}

void deleteCurCombo(tui_combos* tui) {
  int ind = tui->combos_area->selected;
  if (ind < 0) return;
  combo* combo = tui->combos->items[ind];
  int res = deleteCombo(tui->db, combo);
  if (res) {
    freeCombo(pop_elem(tui->combos, ind));
    update_list_len(tui->combos_area, tui->combos->n);
  }
}

void runTuiCombo(sqldb* db, dlist* players) {
  tui_combos* tui = init_tui_combo(db, players);
  qsort(players->items, players->n, sizeof(player*), cmpPlayerName);
  curHide();
  refresh_screen(tui->render);
  int c = 0;
  while (!tui->exit) {
    updateTuiComboAreas(tui);
    renderComboTui(tui);
    c = keyPress();
    handleComboTuiInput(tui, c);
  }
  free_tui_combo(tui);
}

void updateTuiComboAreas(tui_combos* tui) {
  getTermSize(tui->term);
  int rows = tui->term->rows;
  int cols = tui->term->cols / 2;
  update_list_area(tui->combos_area, min_int(cols, BASE_COMBOS_WIDTH), rows);
  update_list_area_fit(tui->players_area, min_int(cols, BASE_PLAYERS_WIDTH), rows);
  set_area_pos(tui->combos_area->area, 0, tui->players_area->area->width + 3);
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

void handle_exit(tui_combos* tui) {
  if (editing_combo(tui)) {
    end_combo(tui);
  } else {
    tui->exit = 1;
  }
}

void handleComboTuiInput(tui_combos* tui, int c) {
  switch (c) {
    case 'q': case 'Q':
      handle_exit(tui);
      break;
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

    case 'B': case 'b':
      start_combo(tui, BAN);
      break;
    case 'P': case 'p':
      start_combo(tui, PAIR);
      break;
    case 'E': case 'e': {
      if (tui->recording_combo) {
        end_combo(tui);
      } else if (tui->mode == CTUI_COMBO_LIST) {
        start_edit_combo(tui);
      }
      break;
    }

    case 'R': case 'r':
      break;
    case 'A': case 'a':
      break;
    case 'D': case 'd':
      if (tui->mode == CTUI_COMBO_LIST) {
        deleteCurCombo(tui);
      }
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

void ctuiRenderPlayersArea(tui_combos* tui) {
  int col = start_print_col(tui->players_area->area);
  int line = start_print_line(tui->players_area->area);
  int len = getListAreaLen(tui->players_area, tui->term->rows);
  tui_area* area = tui->players_area->area;

  make_borders_color(tui->render, area->start_col, area->start_row, area->width,
                     area->height,
                     (tui->mode == CTUI_PLAYER_LIST) ? BLUE_FG : DEFAULT_FG);

  put_text(tui->render, area->start_row, area->start_col + 3, "%s",
           "Selected players");
  if (tui->recording_combo) {
    put_text(tui->render, area->start_row, 22, "(%s)",
             comboTypeString(tui->cur_combo->type));
  }

  int text_len = max_int(area->width - area_width_empty(area), 1);
  char player_text[text_len];
  player_text[0] = '\0';

  for (int i = tui->players_area->first_ind;
       i < tui->players_area->first_ind + len; i++) {
    player* p = get_elem(tui->players, i);
    snprintf(player_text, text_len, " %s", playerFullName(p));
    char in_combo = tui->recording_combo && inCurCombo(tui->cur_combo, p->id) >= 0;

    if (tui->players_area->selected == i) {
      set_selected_row(tui->players_area, line);
      put_text(tui->render, line++, col - 1, "%s\033[7m%-*s\033[27m",
               (in_combo) ? "*" : " ", text_len, player_text);
    } else if (in_combo) {
      int color = (tui->cur_combo->type == BAN)    ? RED_FG
                  : (tui->cur_combo->type == PAIR) ? GREEN_FG
                                                   : DEFAULT_FG;
      put_text(tui->render, line++, col - 1, "*\033[%dm%s\033[0m", color,
               player_text);
    } else {
      put_text(tui->render, line++, col, player_text);
    }
    player_text[0] = '\0';
  }
}

void ctuiRenderCombosArea(tui_combos* tui) {
  int col = start_print_col(tui->combos_area->area);
  int line = start_print_line(tui->combos_area->area);
  int len = getListAreaLen(tui->combos_area, tui->term->rows);
  tui_area* area = tui->combos_area->area;

  int ind = tui->combos_area->selected;
  int sel_combo_len = (ind >= 0) ? comboSize(get_elem(tui->combos, ind)) : 0;
  area->height = area_height_empty(tui->combos_area->area) + len + sel_combo_len;

  int text_len = max_int(area->width - area_width_empty(area), 1);
  char combo_text[text_len];
  combo_text[0] = '\0';

  for (int i = tui->combos_area->first_ind; i < tui->combos_area->first_ind + len; i++) {
    combo* combo = get_elem(tui->combos, i);
    formatComboNames(tui, combo, combo_text, text_len);
    if (tui->combos_area->selected == i) {
      set_selected_row(tui->combos_area, line);
      put_text(tui->render, line++, col, "\033[7m%s\033[27m", combo_text);
      for (size_t j = 0; j < comboSize(combo); j++) {
        player* p = getPlayerInList(tui->players, *((int*)get_elem(combo->ids, j)));
        put_text(tui->render, line++, col + 2, "\033[2m- %s\033[0m",
                 (p) ? playerName(p) : "NotFound");
      }
    } else {
      put_text(tui->render, line++, col, "%s", combo_text);
    }
    combo_text[0] = '\0';
  }

  make_borders_color(tui->render, area->start_col, area->start_row, area->width,
                     area->height,
                     (tui->mode == CTUI_COMBO_LIST) ? BLUE_FG : DEFAULT_FG);
  put_text(tui->render, 0, area->start_col + 3, "%s", "Combos");
}

void formatComboNames(tui_combos* tui, combo* combo, char* combo_text, int text_len) {
  if (!combo) return;
  const char* type_str = comboTypeString(combo->type);
  const int names_len = max_int(text_len - strlen(type_str) - 4, 1);
  char *names = malloc(names_len);
  names[0] = '\0';
  size_t offset = 0;
  for (size_t j = 0; j < comboSize(combo); j++) {
    int* id = get_elem(combo->ids, j);
    player* p = (id) ? getPlayerInList(tui->players, *id) : NULL;
    const char *name = (p) ? (playerName(p)) : "?";
    size_t name_len = strlen(name);
    if ((int)(offset + name_len + 7) > names_len) {
      snprintf(names + offset, names_len - offset, " - ...");
      break;
    }
    if (j > 0) {
      snprintf(names + offset, names_len - offset, " - ");
      offset += 3;
    }
    snprintf(names + offset, names_len - offset, "%s", name);
    offset += name_len;
  }
  snprintf(combo_text, text_len, " (%s) %s", type_str, names);
  free(names);
}


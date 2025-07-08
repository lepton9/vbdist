#include "../include/tuicombo.h"
#include "../include/utils.h"

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

  tui->combos_area = init_list_area(tui->term->cols, tui->term->rows);
  tui->players_area = init_list_area(tui->term->cols, tui->term->rows);
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

    case 'b': // TODO: new keymaps?
      start_combo(tui, BAN);
      break;
    case 'p': // TODO: new keymaps?
      start_combo(tui, PAIR);
      break;
    case 'e': {
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
    case 'X': case 'x':
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
  int col = 2;
  int line = 2;
  int len = getListAreaLen(tui->players_area, tui->term->rows);

  if (tui->mode == CTUI_PLAYER_LIST) {
    make_borders_color(tui->render, 0, 0, tui->players_area->area->width, len + 4, BLUE_FG);
  } else {
    make_borders(tui->render, 0, 0, tui->players_area->area->width, len + 4);
  }
  put_text(tui->render, 0, 3, "%s", "Selected players");
  if (tui->recording_combo) {
    put_text(tui->render, 0, 22, "(%s)", comboTypeString(tui->cur_combo->type));
  }

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
      if (tui->cur_combo->type == BAN) {
        put_text(tui->render, line++, col-1, "*\033[31m %-20s\033[0m", player_text);
      } else if (tui->cur_combo->type == PAIR) {
        put_text(tui->render, line++, col-1, "*\033[32m %-20s\033[0m", player_text);
      }
    } else {
      put_text(tui->render, line++, col, " %-20s", player_text);
    }
    player_text[0] = '\0';
  }
}

void ctuiRenderCombosArea(tui_combos* tui) {
  int col = tui->players_area->area->width + 5;
  int line = 2;
  int len = getListAreaLen(tui->combos_area, tui->term->rows);

  int ind = tui->combos_area->selected;
  int sel_combo_len = 0;
  if (ind >= 0) {
    combo* sel_combo = tui->combos->items[ind];
    sel_combo_len = sel_combo->ids->n;
  }

  size_t border_height = len + 4 + sel_combo_len;

  char combo_text[100];
  combo_text[0] = '\0';

  for (int i = tui->combos_area->first_ind; i < tui->combos_area->first_ind + len; i++) {
    combo* combo = tui->combos->items[i];
    const char *name1 = "";
    const char *name2 = "";
    if (combo->ids->n > 1) {
      player* p1 = getPlayerInList(tui->players, *((int *)combo->ids->items[0]));
      player* p2 = getPlayerInList(tui->players, *((int *)combo->ids->items[1]));
      name1 = (p1 && p1->firstName) ? p1->firstName : "";
      name2 = (p2 && p2->firstName) ? p2->firstName : "";
    }

    sprintf(combo_text, "(%s) %s - %s%s", comboTypeString(combo->type), name1,
            name2, (combo->ids->n > 2) ? " - ..." : "");

    if (tui->combos_area->selected == i) {
      tui->combos_area->selected_term_row = line + 1;
      put_text(tui->render, line++, col + 2, "\033[7m %s\033[27m", combo_text);
      for (size_t j = 0; j < combo->ids->n; j++) {
        player* p = getPlayerInList(tui->players, *((int*)combo->ids->items[j]));
        put_text(tui->render, line++, col + 4, "\033[2m- %s\033[0m", (p && p->firstName) ? p->firstName : "NotFound");
      }
    } else {
      put_text(tui->render, line++, col + 2, "%s", combo_text);
    }
    combo_text[0] = '\0';
  }

  if (tui->mode == CTUI_COMBO_LIST) {
    make_borders_color(tui->render, col, 0, tui->combos_area->area->width, border_height, BLUE_FG);
  } else {
    make_borders(tui->render, col, 0, tui->combos_area->area->width, border_height);
  }
  put_text(tui->render, 0, col + 3, "%s", "Combos");

}



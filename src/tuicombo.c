#include "../include/tuicombo.h"
#include "../include/utils.h"
#include <string.h>

// typedef struct {
//   dlist* players;
//   dlist* combo_lists;
//   list_area* combos_area;
//   list_area* players_area;
//   sqldb* db;
//   term_size* term;
//   renderer* render;
// } tui_combo;

tui_combos* init_tui_combo(sqldb* db, dlist* players) {
  tui_combos* tui = malloc(sizeof(tui_combos));
  tui->db = db;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);
  tui->render = init_renderer(stdout);

  tui->combo_lists = init_list();

  tui->mode = CTUI_PLAYER_LIST;

  tui->cur_combo = NULL;
  tui->recording_combo = 0;

  tui->combos_area = init_list_area(tui->term->cols, tui->term->rows);
  tui->players_area = init_list_area(tui->term->cols, tui->term->rows);
  tui->players = players;
  update_list_len(tui->players_area, tui->players->n);
  return tui;
}

void free_tui_combo(tui_combos* tui) {
  free(tui->term);
  free_renderer(tui->render);
  free_list_area(tui->players_area);
  free_list_area(tui->combos_area);
  for (size_t i = 0; i < tui->combo_lists->n; i++) {
    free(tui->combo_lists->items[i]);
  }
  free_list(tui->combo_lists);
  free(tui);
}


void changeComboTuiMode(tui_combos* tui) {
  if (tui->mode == CTUI_PLAYER_LIST) {
    tui->mode = CTUI_COMBO_LIST;
  } else {
    tui->mode = CTUI_PLAYER_LIST;
  }
}

int addComboList(tui_combos* tui, dlist* combo_list, comboType type) {
  for (size_t i = 0; i < tui->combo_lists->n; i++) {
    combos* c = tui->combo_lists->items[i];
    if (c->type == type) {
      return 0;
    }
  }
  combos* list = malloc(sizeof(combos));
  list->l = combo_list;
  list->type = type;
  list_add(tui->combo_lists, list);
  return 1;
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
  update_list_area(tui->combos_area, cols, rows);
  update_list_area(tui->players_area, cols, rows);
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

  // put_text(tui->render, 1, 2, "\033[4m %s \033[24m", "Selected players");

  char player_text[100];
  player_text[0] = '\0';

  for (int i = tui->players_area->first_ind; i < tui->players_area->first_ind + len; i++) {
    player* p = tui->players->items[i];

    if (tui->players_area->selected == i) {
      tui->players_area->selected_term_row = line + 1;
      strcat(player_text, "\033[7m");
    }
    strcat(player_text, p->firstName);
    if (tui->players_area->selected == i) {
      strcat(player_text, "\033[27m");
    }
    put_text(tui->render, line++, col, " %-20s", player_text);
    player_text[0] = '\0';
  }
  make_borders(tui->render, 0, 0, 30, len + 4);
  put_text(tui->render, 0, 3, "%s", "Selected players");
}

void ctuiRenderCombosArea(tui_combos* tui) {

}



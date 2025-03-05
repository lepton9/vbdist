#include "../include/tuicombo.h"

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



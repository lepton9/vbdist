#ifndef TUICOMBO_H
#define TUICOMBO_H

#include "dlist.h"
#include "listarea.h"
#include "sql.h"
#include "tui.h"
#include "render.h"
#include "sql.h"


typedef struct {
  dlist* l;
  comboType type;
} combos;

typedef struct {
  dlist* players;
  dlist* combo_lists;
  list_area* combos_area;
  list_area* players_area;
  sqldb* db;
  term_size* term;
  renderer* render;
} tui_combos;

tui_combos* init_tui_combo(sqldb* db, dlist* players);
void free_tui_combo(tui_combos* tui);

int addComboList(tui_combos* tui, dlist* combos, comboType type);

void runTuiCombo(sqldb* db);
void updateTuiComboArea(tui_combos* tui);
void handleComboTuiInput(tui_combos* tui, int c);

void addToCombo(tui_combos* tui);

void start_combo(tui_combos* tui);
void end_combo(tui_combos* tui);

void renderComboTui(tui_combos* tui);

#endif


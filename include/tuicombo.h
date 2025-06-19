#ifndef TUICOMBO_H
#define TUICOMBO_H

#include "dlist.h"
#include "listarea.h"
#include "sql.h"
#include "tui.h"
#include "render.h"
#include "sql.h"

typedef enum {
  CTUI_PLAYER_LIST = 0,
  CTUI_COMBO_LIST
} comboTuiMode;

typedef struct {
  comboTuiMode mode;
  dlist* players;
  dlist* combos;
  list_area* combos_area;
  list_area* players_area;
  sqldb* db;
  term_size* term;
  renderer* render;

  int recording_combo;
  combo* cur_combo;
  char exit;
} tui_combos;

tui_combos* init_tui_combo(sqldb* db, dlist* players);
void free_tui_combo(tui_combos* tui);

void changeComboTuiMode(tui_combos* tui);

void comboTuiListUp(tui_combos* tui);
void comboTuiListDown(tui_combos* tui);

void runTuiCombo(sqldb* db, dlist* players);
void updateTuiComboAreas(tui_combos* tui);
void handleComboTuiInput(tui_combos* tui, int c);
void handle_exit(tui_combos* tui);

void ctuiSelectPlayer(tui_combos* tui);

int editing_combo(tui_combos* tui);
void start_edit_combo(tui_combos* tui);
void start_combo(tui_combos* tui, comboType type);
void end_combo(tui_combos* tui);
int insert_cur_combo(tui_combos* tui);
void deleteCurCombo(tui_combos* tui);

void renderComboTui(tui_combos* tui);
void ctuiRenderPlayersArea(tui_combos* tui);
void ctuiRenderCombosArea(tui_combos* tui);

#endif


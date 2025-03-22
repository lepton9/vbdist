#ifndef TUIPOSITIONS_H
#define TUIPOSITIONS_H

#include "dlist.h"
#include "listarea.h"
#include "sql.h"
#include "tui.h"
#include "render.h"
#include "sql.h"


typedef struct {
  dlist* positions;
  dlist* selected_positions;
  list_area* positions_area;
  int use_positions;
  sqldb* db;
  term_size* term;
  renderer* render;
} tui_pos;

tui_pos* init_tui_positions(sqldb* db, dlist* positions, dlist* selected_positions, int use_pos);
void free_tui_positions(tui_pos* tui);

int runTuiPositions(sqldb* db, dlist* all_positions, dlist* selected_positions, int use_pos);
void update_positions_area(tui_pos* tui);
void handle_positions_input(tui_pos* tui, int c);

position* get_selected_pos(tui_pos* tui);
void toggle_selected_pos(tui_pos* tui);

void enable_positions(tui_pos* tui);
void add_position(tui_pos* tui);
void remove_position(tui_pos* tui);

void render_pos_tui(tui_pos* tui);

#endif

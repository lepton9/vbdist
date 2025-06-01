#include "../include/tuipositions.h"
#include "../include/utils.h"
#include <stdlib.h>

tui_pos* init_tui_positions(sqldb* db, dlist* positions, dlist* selected_positions, int use_pos) {
  tui_pos* tui = malloc(sizeof(tui_pos));
  tui->db = db;

  tui->use_positions = use_pos;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);
  tui->render = init_renderer(stdout);

  tui->positions_area = init_list_area(tui->term->cols, tui->term->rows);
  tui->positions = positions;
  tui->selected_positions= selected_positions;
  update_list_len(tui->positions_area, tui->positions->n);

  return tui;
}

void free_tui_positions(tui_pos* tui) {
  free(tui->term);
  free_renderer(tui->render);
  free_list_area(tui->positions_area);
  free(tui);
}

void add_position(tui_pos* tui) {
  position* pos = get_selected_pos(tui);
  if (pos) {
    list_add(tui->selected_positions, copy_position(pos));
  }
}

void remove_position(tui_pos* tui) {
  position* pos = get_selected_pos(tui);
  if (pos) {
    for (size_t i = 0; i < tui->selected_positions->n; i++) {
      position* p = tui->selected_positions->items[i];
      if (p->id == pos->id) {
        freePosition(pop_elem(tui->selected_positions, i));
        return;
      }
    }
  }
}

void enable_positions(tui_pos* tui) {
  tui->use_positions ^= 1;
}

void handle_positions_input(tui_pos *tui, int c) {
  switch (c) {
    case 13: case '\n': case ' ':
#ifdef __linux__
    case KEY_ENTER:
#endif
      break;
    case 27: { // Esc
      break;
    }
    case 9: // Tab
      break;
    case 'e':
      enable_positions(tui);
      break;
    case 'l': case '+':
      add_position(tui);
      break;
    case 'h': case '-':
      remove_position(tui);
      break;
    case 'k': case 'w':
#ifdef __linux__
    case KEY_UP:
#endif
      list_up(tui->positions_area);
      break;
    case 'j': case 's':
#ifdef __linux__
    case KEY_DOWN:
#endif
      list_down(tui->positions_area);
      break;
    default: {
      break;
    }
  }
}

size_t get_selected_amount(dlist* selected_positions, position* pos) {
  size_t count = 0;
  for (size_t i = 0; i < selected_positions->n; i++) {
    position* p = selected_positions->items[i];
    if (p->id == pos->id) count++;
  }
  return count;
}

position* get_selected_pos(tui_pos* tui) {
  if (tui->positions_area->selected < 0) return NULL;
  return tui->positions->items[tui->positions_area->selected];
}

void update_positions_area(tui_pos* tui) {
  getTermSize(tui->term);
  int rows = tui->term->rows - 5;
  int cols = tui->term->cols;
  update_list_area(tui->positions_area, cols, rows);
}

void render_pos_tui(tui_pos* tui) {
  put_text(tui->render, 1, 2, "\033[4m %s \033[24m %10s", "Positions", tui->use_positions ? "ENABLED" : "NOT ENABLED");

  int line = 3;
  int len = min_int(min_int(tui->term->rows - line, (int)tui->positions_area->max_shown), (int)tui->positions->n - (tui->positions_area->first_ind));
  if (tui->positions->n == 0) len = 0;

  for (int i = tui->positions_area->first_ind; i < tui->positions_area->first_ind + len; i++) {
    position* cur_position = tui->positions->items[i];

    append_line(tui->render, line, "  ");
    if (tui->positions_area->selected == i) {
      tui->positions_area->selected_term_row = line + 1;
      append_line(tui->render, line, "\033[7m");
    }

    size_t selected_amount = get_selected_amount(tui->selected_positions, cur_position);

    append_line(tui->render, line, " %-20s %d", cur_position->name, selected_amount);
    if (tui->positions_area->selected == i) {
      append_line(tui->render, line, "\033[27m");
    }
    line++;
  }
  make_borders(tui->render, 0, 0, 30, len + 5);
  render(tui->render);
}

int runTuiPositions(sqldb* db, dlist* all_positions, dlist* selected_positions, int use_pos) {
  tui_pos* tui = init_tui_positions(db, all_positions, selected_positions, use_pos);
  curHide();
  refresh_screen(tui->render);
  int c = 0;
  while (c != 'q') {
    check_selected(tui->positions_area);
    update_positions_area(tui);
    render_pos_tui(tui);
    c = keyPress();
    handle_positions_input(tui, c);
  }
  int enabled = tui->use_positions;
  free_tui_positions(tui);
  return enabled;
}



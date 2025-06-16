#include "../include/listarea.h"
#include "../include/utils.h"

tui_area* init_tui_area(size_t w, size_t h, size_t row, size_t col) {
  tui_area* area = malloc(sizeof(tui_area));
  area->width = w;
  area->height = h;
  area->start_row = row;
  area->start_col = col;
  area->padding_top = 0;
  area->padding_bottom = 0;
  return area;
}

void free_tui_area(tui_area* a) {
  free(a);
}

void set_area_pos(tui_area* area, size_t row, size_t col) {
  area->start_row = row;
  area->start_col = col;
}

void set_padding(tui_area* area, int padding_top, int padding_bottom) {
  area->padding_top = padding_top;
  area->padding_bottom = padding_bottom;
}

void update_area(tui_area* area, size_t w, size_t h) {
  area->width = w;
  area->height = h;
}

int start_print_line(tui_area* area) {
  return area->start_row + area->padding_top;
}

int area_last_line(tui_area* area) {
  return area->height + area->start_row - 1;
}

list_area* init_list_area(size_t w, size_t h) {
  list_area* area = malloc(sizeof(list_area));
  area->area = init_tui_area(w, h, 0, 0);
  area->first_ind = 0;
  area->selected = -1;
  area->max_shown = h;
  return area;
}

void free_list_area(list_area* la) {
  free_tui_area(la->area);
  free(la);
}

int list_up(list_area* la) {
  if (la->selected > 0) {
    la->selected -= 1;
    fit_screen(la);
    return 1;
  }
  return 0;
}

int list_down(list_area* la) {
  if (la->selected < (int)la->len - 1) {
    la->selected += 1;
    fit_screen(la);
    return 1;
  }
  return 0;
}

void fit_screen(list_area* la) {
  if (la->first_ind < 0) {
    la->first_ind = 0;
  }
  if (la->selected < la->first_ind) {
    la->first_ind = la->selected;
  }
  else if (la->selected > la->first_ind + (int)la->max_shown - 1) {
    la->first_ind = la->selected - la->max_shown + 1;
  }
}

void update_list_area(list_area* la, size_t w, size_t h) {
  update_area(la->area, w, h);
  la->max_shown = max_int(0, h - start_print_line(la->area) - la->area->padding_bottom);
  check_selected(la);
  fit_screen(la);
}

void update_list_len(list_area* la, size_t n) {
  la->len = n;
}

void check_selected(list_area* la) {
  if (la->selected >= (int)la->len) {
    la->selected = la->len - 1;
  }
  else if (la->selected < 0 && la->len > 0) {
    la->selected = 0;
  }
}

int getListAreaLen(list_area* area, int term_height, int start_line) {
  if (area->len == 0) return 0;
  return min_int(min_int(term_height - start_line, (int)area->max_shown),
                 (int)area->len - area->first_ind);
}


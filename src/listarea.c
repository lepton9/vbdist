#include "../include/listarea.h"
#include "../include/utils.h"

tui_area* init_tui_area(size_t w, size_t h, size_t row, size_t col) {
  tui_area* area = malloc(sizeof(tui_area));
  area->width = w;
  area->height = h;
  area->start_row = row;
  area->start_col = col;
  area->pad = init_padding(0, 0, 0, 0);
  return area;
}

void free_tui_area(tui_area* a) {
  free_padding(a->pad);
  free(a);
}

padding* init_padding(int top, int bottom, int left, int right) {
  padding* pad = malloc(sizeof(padding));
  pad->top = top;
  pad->bottom = bottom;
  pad->left = left;
  pad->right = right;
  return pad;
}

void free_padding(padding* pad) {
  free(pad);
}

void set_area_pos(tui_area* area, size_t row, size_t col) {
  area->start_row = row;
  area->start_col = col;
}

void set_padding(tui_area* area, int top, int bottom, int left, int right) {
  area->pad->top = top;
  area->pad->bottom = bottom;
  area->pad->left = left;
  area->pad->right = right;
}

void update_area(tui_area* area, size_t w, size_t h) {
  area->width = w;
  area->height = h;
}

int start_print_line(tui_area* area) {
  return area->start_row + area->pad->top + 1;
}

int start_print_col(tui_area* area) {
  return area->start_col + area->pad->left;
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
  area->len = 0;
  area->selected_term_row = 0;
  return area;
}

void free_list_area(list_area* la) {
  free_tui_area(la->area);
  free(la);
}

void set_selected_row(list_area* la, const int render_row) {
  la->selected_term_row = render_row + 1;
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
  } else if (la->selected < la->first_ind) {
    la->first_ind = la->selected;
  } else if (la->selected > la->first_ind + (int)la->max_shown - 1) {
    la->first_ind = la->selected - la->max_shown + 1;
  }
  int wasted_space = la->max_shown - (la->len - la->first_ind);
  if (wasted_space > 0 && la->first_ind > 0) {
    la->first_ind -= min_int(wasted_space, la->selected);
  }
}

void update_list_area(list_area* la, size_t w, size_t h) {
  update_area(la->area, w, h);
  la->max_shown = max_int(0, h - start_print_line(la->area) - la->area->pad->bottom);
  check_selected(la);
  fit_screen(la);
}

void update_list_area_fit(list_area* la, size_t w, size_t h) {
  int height = min_int(h, area_height_fit(la));
  update_area(la->area, w, height);
  la->max_shown = max_int(0, h - start_print_line(la->area) - la->area->pad->bottom);
  check_selected(la);
  fit_screen(la);
}

int area_height_fit(list_area* la) {
  return la->len + la->area->pad->top + la->area->pad->bottom + 2;
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

int getListAreaLen(list_area *area, int term_height) {
  if (area->len == 0) return 0;
  return min_int(
      min_int(term_height - start_print_line(area->area), (int)area->max_shown),
      (int)area->len - area->first_ind);
}

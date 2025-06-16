#include "../include/listarea.h"
#include "../include/utils.h"


list_area* init_list_area(size_t w, size_t h) {
  list_area* area = malloc(sizeof(list_area));
  area->first_ind = 0;
  area->selected = -1;
  area->width = w;
  area->max_shown = h;
  return area;
}

void free_list_area(list_area* la) {
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
  la->width = w;
  la->max_shown = h;
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


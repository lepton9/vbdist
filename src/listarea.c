#include "../include/listarea.h"


list_area* initListArea(size_t w, size_t h) {
  list_area* area = malloc(sizeof(list_area));
  area->first_ind = 0;
  area->selected = 0;
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
  if (la->selected < (int)la->len) {
    la->selected += 1;
    fit_screen(la);
    return 1;
  }
  return 0;
}

void fit_screen(list_area* la) {
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
}


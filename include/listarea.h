#ifndef LISTAREA_H
#define LISTAREA_H

#include <stdlib.h>

typedef struct {
  size_t len;
  size_t max_shown;
  size_t width;
  int first_ind;
  int selected;
  int selected_term_row;
} list_area;


list_area* init_list_area(size_t w, size_t h);
void free_list_area(list_area* la);

int list_up(list_area* tui);
int list_down(list_area* tui);

void list_init_selected(list_area* la);
void update_list_area(list_area* la, size_t w, size_t h);
void update_list_len(list_area* la, size_t n);

void fit_screen(list_area* la);

#endif


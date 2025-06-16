#ifndef LISTAREA_H
#define LISTAREA_H

#include <stdlib.h>

typedef struct {
  size_t height;
  size_t width;
  size_t start_row;
  size_t start_col;
  int padding_top;
  int padding_bottom;
} tui_area;

typedef struct {
  tui_area* area;
  size_t len;
  size_t max_shown;
  int first_ind;
  int selected;
  int selected_term_row;
} list_area;


tui_area* init_tui_area(size_t w, size_t h, size_t row, size_t col);
void free_tui_area(tui_area* a);

void set_area_pos(tui_area* area, size_t row, size_t col);
void set_padding(tui_area* area, int padding_top, int padding_bottom);
int start_print_line(tui_area* area);
int area_last_line(tui_area* area);

list_area* init_list_area(size_t w, size_t h);
void free_list_area(list_area* la);

int list_up(list_area* tui);
int list_down(list_area* tui);

void check_selected(list_area* la);
void update_list_area(list_area* la, size_t w, size_t h);
void update_list_len(list_area* la, size_t n);

void fit_screen(list_area* la);

int getListAreaLen(list_area* area, int term_height, int start_line);

#endif


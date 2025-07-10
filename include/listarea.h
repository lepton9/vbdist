#ifndef LISTAREA_H
#define LISTAREA_H

#include <stdlib.h>

typedef struct {
  int top;
  int bottom;
  int left;
  int right;
} padding;

typedef struct {
  size_t height;
  size_t width;
  size_t start_row;
  size_t start_col;
  padding* pad;
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

padding* init_padding(int top, int bottom, int left, int right);
void free_padding(padding* pad);

void set_area_pos(tui_area* area, size_t row, size_t col);
void set_padding(tui_area* area, int top, int bottom, int left, int right);
int area_height_empty(tui_area* la);
int area_width_empty(tui_area* area);
int start_print_line(tui_area* area);
int start_print_col(tui_area* area);
int area_last_line(tui_area* area);

list_area* init_list_area(size_t w, size_t h);
void free_list_area(list_area* la);

void set_selected_row(list_area* la, const int render_row);

int list_up(list_area* tui);
int list_down(list_area* tui);

void check_selected(list_area* la);
void update_list_area(list_area* la, size_t w, size_t h);
void update_list_area_fit(list_area* la, size_t w, size_t h);
void update_list_len(list_area* la, size_t n);
int area_height_fit(list_area* la, size_t h);

void fit_screen(list_area* la);

int getListAreaLen(list_area* area, int term_height);

#endif


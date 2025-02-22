#ifndef RENDER_H
#define RENDER_H

#include <stdlib.h>
#include <stdio.h>

#define ESC '\033'

typedef struct {
  FILE* out;
  size_t real_width;
  size_t width;
  size_t height;

  struct {
    char** s;
    size_t* line_len;
    size_t* print_line_len;
  } screen, last_screen;

} renderer;

renderer* init_renderer(FILE* out);
void free_renderer(renderer* r);

int updateSize(renderer* r);
int setSize(renderer* r, size_t new_w, size_t new_h);
void resize_screen(renderer* r, size_t new_w, size_t new_h);

int setText(renderer* r, size_t row, size_t col, const char* line);
void put_text(renderer* r, size_t row, size_t col, const char *fmt, ...);
void append_line(renderer* r, size_t row, const char *fmt, ...);
void update_segment(renderer* r, size_t row, size_t col, size_t width, const char *fmt, ...);

void refresh_screen(renderer* r);
void render(renderer* r);

// draw_box()
void make_borders(renderer* r, size_t x, size_t y, size_t w, size_t h);

size_t shift_esc_seq(char* line, size_t line_len, size_t print_ind, size_t section_len);
int is_escape_end(char c);
const char* skip_escape(const char* str);
size_t printable_length(const char *str);
const char* find_printable_start(const char* str, size_t start);
char* printable_substr(const char* str, size_t start, size_t length);

#endif

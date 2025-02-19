#ifndef RENDER_H
#define RENDER_H

#include <stdlib.h>
#include <stdio.h>

typedef struct {
  char** screen;
  char** last_screen;
  size_t* line_len;
  size_t* print_line_len;
  size_t width;
  size_t height;
} renderer;

renderer* init_renderer(size_t w, size_t h);
void free_renderer(renderer* r);

int updateSize(renderer* r);
int setSize(renderer* r, size_t new_w, size_t new_h);
void resize_screen(renderer* r, size_t new_w, size_t new_h);

int setText(renderer* r, size_t row, size_t col, const char* line);
void put_text(renderer* r, size_t row, size_t col, const char *fmt, ...);
void append_line(renderer* r, size_t row, const char *fmt, ...);
int addLine(renderer* r, size_t row, const char* line);
void render(renderer* r, FILE* out);

#endif

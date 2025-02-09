#ifndef RENDER_H
#define RENDER_H

#include <stdlib.h>
#include <stdio.h>

typedef struct {
  char** screen;
  char** last_screen;
  size_t width;
  size_t height;
} renderer;

renderer* init_renderer(size_t w, size_t h);
void free_renderer(renderer* r);

int updateSize(renderer* r);

void render(renderer* r, FILE* out);

#endif

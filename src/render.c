#include "../include/render.h"
#include "../include/tui.h"
#include <stdlib.h>
#include <string.h>


renderer* init_renderer(size_t w, size_t h) {
  renderer* r = malloc(sizeof(renderer));
  r->width = w;
  r->height = h;
  r->screen = malloc(h * sizeof(char*));
  r->last_screen = malloc(h * sizeof(char*));
  for (size_t i = 0; i < h; i++) {
    r->screen[i] = calloc(w + 1, sizeof(char));
    r->last_screen[i] = calloc(w + 1, sizeof(char));
    memset(r->screen[i], ' ', w);
    memset(r->last_screen[i], ' ', w);
    r->screen[i][w] = '\0';
    r->last_screen[i][w] = '\0';
  }
  return r;
}

void free_renderer(renderer* r) {
  free(r->screen);
  free(r->last_screen);
  free(r);
}

void resize_screen(renderer* r, size_t new_w, size_t new_h) {
  char** new_screen = malloc(new_h * sizeof(char*));
  char** new_last_screen = malloc(new_h * sizeof(char*));

  for (size_t i = 0; i < new_h; i++) {
    new_screen[i] = calloc(new_w + 1, sizeof(char));
    new_last_screen[i] = calloc(new_w + 1, sizeof(char));

    if (i < r->height) {
      int len = (new_w < r->width ? new_w : r->width);
      strncpy(new_screen[i], r->screen[i], len);
      strncpy(new_last_screen[i], r->last_screen[i], len);
    }

    if (i < r->height) {
      strncpy(new_screen[i], r->screen[i], new_w); 
      strncpy(new_last_screen[i], r->last_screen[i], new_w);
      if (new_w > r->width) {
        memset(new_screen[i] + r->width, ' ', new_w - r->width);
        memset(new_last_screen[i] + r->width, ' ', new_w - r->width);
      }
    } else {
      memset(new_screen[i], ' ', new_w);
      memset(new_last_screen[i], ' ', new_w);
    }

    new_screen[i][new_w] = '\0';
    new_last_screen[i][new_w] = '\0';
  }

  for (size_t i = 0; i < r->height; i++) {
    free(r->screen[i]);
    free(r->last_screen[i]);
  }
  free(r->screen);
  free(r->last_screen);

  r->screen = new_screen;
  r->last_screen = new_last_screen;
  r->width = new_w;
  r->height = new_h;
}

int updateSize(renderer* r) {
  term_size term = {.rows = r->height, .cols = r->width};
  getTermSize(&term);
  if (term.rows != (int)r->height || term.cols != (int)r->width) {
    resize_screen(r, term.cols, term.rows);
    // r->width = term.cols;
    // r->height = term.rows;
    return 1;
  }
  return 0;
}

void render(renderer* r, FILE* out) {
  updateSize(r);
  for (int y = 0; y < (int)r->height; y++) {
    if (memcmp(r->screen[y], r->last_screen[y], r->width) != 0) {
      // TODO: only print r->width amount
      fprintf(out, "\033[%d;1H%s\033[0K", y + 1, r->screen[y]);
      memcpy(r->last_screen[y], r->screen[y], r->width);
    }
  }
}

// void render_chars(renderer* r, FILE* out) {
//   if (memcmp(r->screen, r->last_screen, r->width * r->height) == 0) return;
//   for (int i = 0; i < r->width * r->height; i++) {
//     if (r->screen[i] != r->last_screen[i]) {  // Only update changed characters
//       int y = i / r->width;
//       int x = i % r->width;
//       printf("\033[%d;%dH%c", y + 1, x + 1, r->screen[i]); // Move cursor and print
//       r->last_screen[i] = r->screen[i];  // Update last screen state
//     }
//   }
// }


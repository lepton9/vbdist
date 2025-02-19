#include "../include/render.h"
#include "../include/tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define put_text(renderer, fmt, ...) \
//     log_with_prefix(prefix, fmt, __VA_ARGS__)


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
  r->line_len = calloc(h, sizeof(size_t));
  r->print_line_len = calloc(h, sizeof(size_t));
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

  r->line_len = realloc(r->line_len, new_h * sizeof(size_t));
  memset(r->line_len, 0, new_h);
  r->print_line_len = realloc(r->print_line_len, new_h * sizeof(size_t));
  memset(r->print_line_len, 0, new_h);
}

int updateSize(renderer* r) {
  term_size term = {.rows = r->height, .cols = r->width};
  getTermSize(&term);
  return setSize(r, term.cols, term.rows);
}

int setSize(renderer* r, size_t new_w, size_t new_h) {
  if (new_w != r->width || new_h != r->height) {
    resize_screen(r, new_w, new_h);
    return 1;
  }
  return 0;
}

size_t printable_length(const char *str) {
  size_t length = 0;
  int in_escape = 0;
  while (*str) {
    if (in_escape) {
      if ((*str >= 'A' && *str <= 'Z') || (*str >= 'a' && *str <= 'z')) {
        in_escape = 0;
      }
    } else {
      if (*str == '\x1B' && *(str + 1) == '[') {
        in_escape = 1;
      } else {
        length++;
      }
    }
    str++;
  }
  return length;
}

int setText(renderer* r, size_t row, size_t print_col, const char* line) {
  if (row > r->height || print_col > r->width) return 0;

  size_t len = strnlen(line, r->width - print_col);
  size_t print_len = printable_length(line);
  size_t line_print_len = r->print_line_len[row-1];

  if (line_print_len < print_col) {
    size_t space_count = print_col - line_print_len;
    memset(r->screen[row-1] + r->line_len[row-1], ' ', space_count);
    r->line_len[row-1] += space_count;
  }
  memcpy(r->screen[row-1] + r->line_len[row-1], line, len);

  r->line_len[row-1] += len;
  r->print_line_len[row-1] = print_col + print_len;

  return 1;
}

void put_text(renderer* r, size_t row, size_t col, const char *fmt, ...) {
  char line[r->width + 1];
  va_list args;
  va_start(args, fmt);
  vsnprintf(line, sizeof(line), fmt, args);
  va_end(args);
  setText(r, row, col, line);
}

void append_line(renderer* r, size_t row, const char *fmt, ...) {
  char line[r->width + 1];
  va_list args;
  va_start(args, fmt);
  vsnprintf(line, sizeof(line), fmt, args);
  va_end(args);
  setText(r, row, r->print_line_len[row-1], line);
}

void clear_screen(char** screen, size_t w, size_t h) {
  for (size_t i = 0; i < h; i++) {
    memset(screen[i], ' ', 1);
  }
}

// TODO: deprecated
int addLine(renderer* r, size_t row, const char* line) {
  if (row > r->height) return 0;
  size_t len = strnlen(line, r->width);
  memcpy(r->screen[row], line, len);
  // r->screen[row - 1][len] = '\0';
  return 1;
}

void render(renderer* r, FILE* out) {
  updateSize(r);
  for (int y = 0; y < (int)r->height; y++) {
    if (memcmp(r->screen[y], r->last_screen[y], r->width) != 0) {
      fprintf(out, "\033[%d;1H%.*s\033[0K", y + 1, (int)r->line_len[y], r->screen[y]);
      memcpy(r->last_screen[y], r->screen[y], r->width);
    }
    r->line_len[y] = 0;
    r->print_line_len[y] = 0;
  }
  fflush(out);
}

void render_chars(renderer* r, FILE* out) {
  if (memcmp(r->screen, r->last_screen, r->width * r->height) == 0) return;
  for (int y = 0; y < r->height; y++) {
    for (int x = 0; x < r->width; x++) {
      if (r->screen[x] != r->last_screen[x]) {
        printf("\033[%d;%dH%c", y + 1, x + 1,
               r->screen[y][x]);
        r->last_screen[y][x] = r->screen[y][x];
      }
    }
  }
}


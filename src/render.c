#include "../include/render.h"
#include "../include/tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


renderer* init_renderer(size_t w, size_t h) {
  renderer* r = malloc(sizeof(renderer));
  r->width = w;
  r->height = h;
  r->real_width = 2 * w;
  r->screen = malloc(h * sizeof(char*));
  r->last_screen = malloc(h * sizeof(char*));
  for (size_t i = 0; i < h; i++) {
    r->screen[i] = calloc(r->real_width + 1, sizeof(char));
    r->last_screen[i] = calloc(r->real_width + 1, sizeof(char));
    memset(r->screen[i], ' ', r->real_width);
    memset(r->last_screen[i], ' ', r->real_width);
    r->screen[i][r->real_width] = '\0';
    r->last_screen[i][r->real_width] = '\0';
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
  size_t new_rw = 2 * new_w;
  char** new_screen = malloc(new_h * sizeof(char*));
  char** new_last_screen = malloc(new_h * sizeof(char*));

  r->line_len = realloc(r->line_len, new_h * sizeof(size_t));
  r->print_line_len = realloc(r->print_line_len, new_h * sizeof(size_t));

  for (size_t i = 0; i < new_h; i++) {
    new_screen[i] = calloc(new_rw + 1, sizeof(char));
    new_last_screen[i] = calloc(new_rw + 1, sizeof(char));

    if (i < r->height) {
      int len = (new_rw < r->real_width ? new_rw : r->width);
      strncpy(new_screen[i], r->screen[i], len);
      strncpy(new_last_screen[i], r->last_screen[i], len);
    }
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
  r->real_width = new_rw;
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

int is_escape_end(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

const char* skip_escape(const char* str) {
  if (*str == ESC && *(str + 1) == '[') {
    str += 2;
    while (*str && !is_escape_end(*str)) str++;
    if (*str) str++;
  }
  return str;
}

size_t printable_length(const char *str) {
  size_t length = 0;
  while (*str) {
    str = skip_escape(str);
    if (*str) {
      length++;
      str++;
    }
  }
  return length;
}

const char* find_printable_start(const char* str, size_t start) {
  size_t print_count = 0;
  while (*str) {
    str = skip_escape(str);
    if (*str) {
      if (print_count == start) return str;
      print_count++;
      str++;
    }
  }
  return NULL;
}

char* printable_substr(const char* str, size_t start, size_t length) {
  const char *substr_start = find_printable_start(str, start);
  if (!substr_start) return strdup("");

  size_t max_print_len = printable_length(substr_start);
  size_t effective_length = (length > max_print_len) ? max_print_len : length;

  char *result = malloc(strlen(substr_start) + 1);
  if (!result) return NULL;

  char *out = result;
  size_t print_count = 0;

  while (*substr_start && print_count < effective_length) {
    substr_start = skip_escape(substr_start);
    if (*substr_start) {
      *out++ = *substr_start++;
      print_count++;
    }
  }
  *out = '\0';
  return result;
}

int setText(renderer* r, size_t row, size_t print_col, const char* line) {
  if (row < 0 || print_col < 0 || row > r->height || print_col > r->width) return 0;

  size_t len = strlen(line);
  size_t print_len = printable_length(line);
  size_t line_print_len = r->print_line_len[row];
  size_t available_space = r->width - line_print_len;

  if (r->line_len[row] + len > r->real_width) return 0;

  if (print_col > line_print_len) {
    size_t space_count = print_col - line_print_len;
    if (available_space < space_count) return 0;

    memset(r->screen[row] + r->line_len[row], ' ', space_count);
    available_space -= space_count;
    r->line_len[row] += space_count;
  }

  if (print_len > available_space) {
    char* cut = printable_substr(line, 0, available_space);
    memcpy(r->screen[row] + r->line_len[row], cut, available_space);
    r->line_len[row] += strlen(cut);
    r->print_line_len[row] = print_col + available_space;
    free(cut);
  } else {
    memcpy(r->screen[row] + r->line_len[row], line, len);
    r->line_len[row] += len;
    r->print_line_len[row] = print_col + print_len;
  }
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
  setText(r, row, r->print_line_len[row], line);
}

void render(renderer* r, FILE* out) {
  int resize = updateSize(r);
  for (int y = 0; y < (int)r->height; y++) {
    if (resize || memcmp(r->screen[y], r->last_screen[y], r->width) != 0) {
      fprintf(out, "\033[%d;1H%.*s\033[0K", y + 1, (int)r->line_len[y], r->screen[y]);
      memcpy(r->last_screen[y], r->screen[y], r->real_width);
    }
    r->line_len[y] = 0;
    r->print_line_len[y] = 0;
  }
  fflush(out);
}

// void render(renderer* r, FILE* out) {
//   if (memcmp(r->screen, r->last_screen, r->width * r->height) == 0) return;
//   for (int y = 0; y < r->height; y++) {
//     for (int x = 0; x < r->line_len[y]; x++) {
//       if (r->screen[x] != r->last_screen[x]) {
//         fprintf(out, "\033[%d;%dH%c", y + 1, x + 1, r->screen[y][x]);
//         r->last_screen[y][x] = r->screen[y][x];
//       }
//     }
//     fprintf(out, "\033[%d;%dH\033[0K", y + 1, (int)r->line_len[y]);
//     r->line_len[y] = 0;
//     r->print_line_len[y] = 0;
//   }
//   fflush(out);
// }


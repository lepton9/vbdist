#include "../include/render.h"
#include "../include/tui.h"
#include "../include/utils.h"
#include "../include/ansicodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


renderer* init_renderer(FILE* out) {
  renderer* r = malloc(sizeof(renderer));

  term_size term = {.rows = 0, .cols = 0};
  getTermSize(&term);
  size_t w = term.cols;
  size_t h = term.rows;

  r->out = out;
  r->width = w;
  r->height = h;
  r->real_width = 2 * w;
  r->screen.s = malloc(h * sizeof(char*));
  r->last_screen.s = malloc(h * sizeof(char*));

  for (size_t i = 0; i < h; i++) {
    r->screen.s[i] = calloc(r->real_width + 1, sizeof(char));
    r->last_screen.s[i] = calloc(r->real_width + 1, sizeof(char));
    memset(r->screen.s[i], ' ', r->real_width);
    memset(r->last_screen.s[i], ' ', r->real_width);
    r->screen.s[i][r->real_width] = '\0';
    r->last_screen.s[i][r->real_width] = '\0';
  }
  r->screen.line_len = calloc(h, sizeof(size_t));
  r->screen.print_line_len = calloc(h, sizeof(size_t));
  r->last_screen.line_len = calloc(h, sizeof(size_t));
  r->last_screen.print_line_len = calloc(h, sizeof(size_t));
  return r;
}

void free_renderer(renderer* r) {
  free(r->screen.s);
  free(r->last_screen.s);
  free(r->screen.line_len);
  free(r->screen.print_line_len);
  free(r->last_screen.line_len);
  free(r->last_screen.print_line_len);
  free(r);
}

void resize_screen(renderer* r, size_t new_w, size_t new_h) {
  size_t new_rw = 2 * new_w;
  char** new_screen = malloc(new_h * sizeof(char*));
  char** new_last_screen = malloc(new_h * sizeof(char*));

  r->screen.line_len = realloc(r->screen.line_len, new_h * sizeof(size_t));
  r->screen.print_line_len =
      realloc(r->screen.print_line_len, new_h * sizeof(size_t));
  r->last_screen.line_len =
      realloc(r->last_screen.line_len, new_h * sizeof(size_t));
  r->last_screen.print_line_len =
      realloc(r->last_screen.print_line_len, new_h * sizeof(size_t));

  for (size_t i = 0; i < new_h; i++) {
    new_screen[i] = calloc(new_rw + 1, sizeof(char));
    new_last_screen[i] = calloc(new_rw + 1, sizeof(char));

    if (i < r->height) {
      int len = (new_rw < r->real_width ? new_rw : r->width);
      strncpy(new_screen[i], r->screen.s[i], len);
      strncpy(new_last_screen[i], r->last_screen.s[i], len);
    }
  }

  for (size_t i = 0; i < r->height; i++) {
    free(r->screen.s[i]);
    free(r->last_screen.s[i]);
  }
  free(r->screen.s);
  free(r->last_screen.s);

  r->screen.s = new_screen;
  r->last_screen.s = new_last_screen;
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

int real_index(const char *str, size_t printable_ind) {
  size_t print_count = 0;
  const char *start = str;

  while (str && *str) {
    str = skip_escape(str);

    if (*str) {
      if (print_count == printable_ind) return str - start;
      print_count++;
      str++;
    }
  }
  return -1;
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
  if (row > r->height || print_col > r->width) return 0;

  size_t len = strlen(line);
  size_t print_len = printable_length(line);
  size_t line_print_len = r->screen.print_line_len[row];
  size_t available_space = r->width - line_print_len;

  if (r->screen.line_len[row] + len > r->real_width) return 0;

  if (print_col > line_print_len) {
    size_t space_count = print_col - line_print_len;
    if (available_space < space_count) return 0;

    memset(r->screen.s[row] + r->screen.line_len[row], ' ', space_count);
    available_space -= space_count;
    r->screen.line_len[row] += space_count;
    r->screen.print_line_len[row] += space_count;
  }

  int real_col = (print_col == r->screen.print_line_len[row])
                     ? (int)r->screen.line_len[row]
                     : real_index(r->screen.s[row], print_col);
  if (real_col < 0) return 0;

  char* cut = NULL;
  if (print_len > available_space) {
    cut = printable_substr(line, 0, available_space);
    len = strlen(cut);
    print_len = available_space;
  }

  shift_esc_seq(r->screen.s[row], r->real_width, print_col, len);

  if (len > print_len && print_col < r->screen.print_line_len[row]) {
    size_t shift = len - print_len;
    shift_right(r->screen.s[row], real_col,
                r->real_width - real_col, shift);
    r->screen.line_len[row] += shift;
  }

  if (cut) {
    memcpy(r->screen.s[row] + real_col, cut, len);
    free(cut);
  } else {
    memcpy(r->screen.s[row] + real_col, line, len);
  }

  r->screen.print_line_len[row] = (print_col + print_len > r->screen.print_line_len[row])
    ? (print_col + print_len)
    : r->screen.print_line_len[row];

  r->screen.line_len[row] = (real_col + len > r->screen.line_len[row])
    ? (real_col + len)
    : r->screen.line_len[row];

  return 1;
}

void shift_right(char* line, size_t real_ind, size_t len, size_t shift_n) {
  memmove(line + real_ind + shift_n, line + real_ind, len);
}


// TODO: multiple shifts in a row needed
size_t shift_esc_seq(char* line, size_t line_len, size_t print_ind, size_t section_len) {
  size_t real_ind = real_index(line, print_ind);
  int in_escape = 0;
  int esc_start = -1;
  int esc_len = 0;

  for (size_t i = real_ind; (i < real_ind + section_len || in_escape) && i < line_len; i++) {
    if (in_escape) {
      esc_len++;
      if (is_escape_end(line[i])) {
        in_escape = 0;
      }
    } else {
      if (line[i] == ESC && i + 1 < line_len && line[i + 1] == '[') {
        in_escape = 1;
        esc_start = i;
        esc_len = 1;
      }
    }
  }
  if (esc_start >= 0) {
    size_t shift = section_len - (esc_start - real_ind);
    shift_right(line, real_ind, esc_len, shift);
    // memmove(line + esc_start + shift, line + esc_start, esc_len);
    return shift;
  }
  return 0;
}

// TODO: useless
char* make_str(size_t width, const char *fmt, ...) {
  char *line = malloc(width + 1);
  va_list args;
  va_start(args, fmt);
  vsnprintf(line, width + 1, fmt, args);
  va_end(args);
  return line;
}

int emptied(renderer* r) {
  for (size_t i = 0; i < r->height; i++) {
    if (r->screen.line_len[i] != 0) return 0;
  }
  return 1;
}

void copy_old_screen(renderer* r) {
  for (size_t i = 0; i < r->height; i++) {
    memcpy(r->screen.s[i], r->last_screen.s[i], r->last_screen.line_len[i]);
    r->screen.line_len[i] = r->last_screen.line_len[i];
    r->screen.print_line_len[i] = r->last_screen.print_line_len[i];
  }
}

// TODO: make better, ability to have input segment
void update_segment(renderer* r, size_t row, size_t col, size_t width, const char *fmt, ...) {
  char segment[width + 1];
  va_list args;
  va_start(args, fmt);
  vsnprintf(segment, sizeof(segment), fmt, args);
  va_end(args);

  if (emptied(r)) copy_old_screen(r);

  setText(r, row, col, segment);
}

void put_text(renderer* r, size_t row, size_t col, const char *fmt, ...) {
  if (row >= r->height) return;
  char line[r->width + 1];
  va_list args;
  va_start(args, fmt);
  vsnprintf(line, sizeof(line), fmt, args);
  va_end(args);
  setText(r, row, col, line);
}

void append_line(renderer* r, size_t row, const char *fmt, ...) {
  if (row >= r->height) return;
  char line[r->width + 1];
  va_list args;
  va_start(args, fmt);
  vsnprintf(line, sizeof(line), fmt, args);
  va_end(args);
  setText(r, row, r->screen.print_line_len[row], line);
}

void make_borders_utf(renderer* r, size_t x, size_t y, size_t w, size_t h) {
}

void make_borders_ascii(renderer* r, size_t x, size_t y, size_t w, size_t h) {
  for (size_t i = 0; i < h; i++) {
    for (size_t j = 0; j < w; j++) {
      size_t real_col = real_index(r->screen.s[y + i], x + j);
      if ((i == 0 && j == 0) || (i == h - 1 && j == 0) ||
          (i == 0 && j == w - 1) || (i == h - 1 && j == w - 1))
        r->screen.s[y + i][real_col] = '+';
      else if (i == 0 || i == h-1)
        r->screen.s[y + i][real_col] = '-';
      else if (j == 0 || j == w-1)
        r->screen.s[y + i][real_col] = '|';
      // else if ((i != 0 && i != h-1) && r->screen.print_line_len[i] < j)
      //   r->screen.s[y + i][real_col] = ' ';
    }
    if (x + w > r->screen.print_line_len[y + i]) {
      size_t over = (x + w) - r->screen.print_line_len[y + i];
      r->screen.print_line_len[y + i] += over;
      r->screen.line_len[y + i] += over;
    }
  }
}

void make_borders(renderer* r, size_t x, size_t y, size_t w, size_t h) {
  w = min_int(w, r->width - (x));
  h = min_int(h, r->height - (y));
  make_borders_ascii(r, x, y, w, h);
}

void refresh_screen(renderer* r) {
  updateSize(r);
  for (int y = 0; y < (int)r->height; y++) {
    fprintf(r->out, "\033[%d;1H%.*s\033[0K", y + 1,
            (int)r->last_screen.line_len[y], r->last_screen.s[y]);
  }
  fflush(r->out);
}

void render(renderer* r) {
  int resize = updateSize(r);
  int changes = 0;
  // char l[r->real_width+1];
  for (int y = 0; y < (int)r->height; y++) {
    int modified = (r->screen.line_len[y] != r->last_screen.line_len[y]) ||
                   (memcmp(r->screen.s[y], r->last_screen.s[y],
                           r->screen.line_len[y]) != 0);
    if (resize || modified) {
      // sprintf(l, "\033[%d;1H%.*s\033[0K", y + 1, (int)r->screen.line_len[y], r->screen.s[y]);
      // write(fileno(r->out), l, (int)r->screen.line_len[y]);
      fprintf(r->out, "\033[%d;1H%.*s\033[0K", y + 1, (int)r->screen.line_len[y], r->screen.s[y]);
      memcpy(r->last_screen.s[y], r->screen.s[y], r->real_width);
      r->last_screen.line_len[y] = r->screen.line_len[y];
      r->last_screen.print_line_len[y] = r->screen.print_line_len[y];
      changes = 1;
    }
    r->screen.line_len[y] = 0;
    r->screen.print_line_len[y] = 0;
    memset(r->screen.s[y], ' ', r->real_width);
  }
  if (changes) fflush(r->out);
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


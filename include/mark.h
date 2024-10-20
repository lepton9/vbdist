#ifndef MARK_H
#define MARK_H

typedef enum {
  BLACK = 30,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE,
  DEFAULT_COLOR = 39,
} fg_color;

typedef struct {
  char active;
  fg_color color;
} mark;

fg_color getMarkColor(const int key);

#endif

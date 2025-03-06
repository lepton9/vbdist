#ifndef ANSICODES_H
#define ANSICODES_H

#define ESC '\033'

typedef enum {
  BLACK_FG = 30,
  RED_FG,
  GREEN_FG,
  YELLOW_FG,
  BLUE_FG,
  MAGENTA_FG,
  CYAN_FG,
  WHITE_FG,
  DEFAULT_FG = 39
} color_fg ;

typedef enum {
  BLACK_BG = 40,
  RED_BG,
  GREEN_BG,
  YELLOW_BG,
  BLUE_BG,
  MAGENTA_BG,
  CYAN_BG,
  WHITE_BG,
  DEFAULT_BG = 49
} color_bg ;


#endif

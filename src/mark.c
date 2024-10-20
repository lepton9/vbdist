#include "../include/mark.h"

fg_color getMarkColor(const int key) {
  switch (key) {
    case 1: return GREEN;
    case 2: return YELLOW;
    case 3: return BLUE;
    case 4: return MAGENTA;
    case 5: return CYAN;
    default: return DEFAULT_COLOR;
  }
}


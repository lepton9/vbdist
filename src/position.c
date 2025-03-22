#include "../include/position.h"
#include <stdlib.h>
#include <string.h>

position* initPosition(int id, const char* name) {
  position* p = malloc(sizeof(position));
  p->id = id;
  p->name = strdup(name);
  p->priority = 0;
  return p;
}

void freePosition(position* p) {
  if (!p) return;
  free(p->name);
  free(p);
}

void setPriority(position* p, int priority) {
  p->priority = priority;
}

position* copy_position(position* p) {
  position* copy = initPosition(p->id, p->name);
  return copy;
}


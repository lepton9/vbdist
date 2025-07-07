#include "../include/position.h"
#include <stdlib.h>
#include <string.h>

position* initPosition(int id, const char* name) {
  position* p = malloc(sizeof(position));
  p->id = id;
  p->name = strdup(name ? name : "");
  p->priority = 0;
  return p;
}

void freePosition(position* p) {
  if (!p) return;
  if (p->name) free(p->name);
  free(p);
}

void setPriority(position* p, int priority) {
  p->priority = priority;
}

int findPositionFrom(dlist* positions, position* pos, const size_t ind) {
  for (size_t i = ind; i < positions->n; i++) {
    position* p = positions->items[i];
    if (p->id == pos->id) return i;
  }
  return -1;
}

int findPosition(dlist* positions, position* pos) {
  return findPositionFrom(positions, pos, 0);
}

position* copy_position(const position* p) {
  position* copy = initPosition(p->id, p->name);
  return copy;
}

dlist* copyPositions(dlist* positions) {
  if (!positions) return NULL;
  dlist* positions_copy = init_list();
  for (size_t i = 0; i < positions->n; i++) {
    position* pos = positions->items[i];
    position* pos_copy = copy_position(pos);
    setPriority(pos_copy, pos->priority);
    list_add(positions_copy, pos_copy);
  }
  return positions_copy;
}

void freePositions(dlist* positions) {
  if (!positions) return;
  for (size_t i = 0; i < positions->n; i++) {
    freePosition(positions->items[i]);
  }
  free_list(positions);
}


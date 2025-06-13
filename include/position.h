#ifndef POSITION_H
#define POSITION_H

#include "dlist.h"

typedef struct {
  int id;
  char* name;
  int priority;
} position;

position* initPosition(int id, const char* name);
void freePosition(position* p);
void setPriority(position* p, int priority);

int findPositionFrom(dlist* positions, position* pos, const size_t ind);
int findPosition(dlist* positions, position* pos);

position* copy_position(position* p);
dlist* copyPositions(dlist* positions);
void freePositions(dlist* positions);

#endif


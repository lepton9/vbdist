#ifndef POSITION_H
#define POSITION_H

typedef struct {
  int id;
  char* name;
  int priority;
} position;

position* initPosition(int id, const char* name);
void freePosition(position* p);
void setPriority(position* p, int priority);

position* copy_position(position* p);

#endif


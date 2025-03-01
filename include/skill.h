#ifndef SKILL_H
#define SKILL_H

#include "../include/dlist.h"

typedef struct {
  int id;
  char* name;
  float value;
} skill;

skill* initSkill(const int id, const char* name, const float value);
void freeSkill(skill* skill);

#endif


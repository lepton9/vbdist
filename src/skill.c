#include "../include/skill.h"
#include <stdlib.h>
#include <string.h>


skill* initSkill(const int id, const char* name, const float value) {
  skill* s = malloc(sizeof(skill));
  s->id = id;
  s->name = strdup(name);
  s->value = value;
  return s;
}

void freeSkill(skill* s) {
  if (s->name) free(s->name);
  free(s);
}


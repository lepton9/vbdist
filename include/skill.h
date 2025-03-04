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
dlist* initSelectedSkills(dlist* allSkills);
void freeSelectedSkills(dlist* selected_skills);

int is_selected_skill(skill* skill, dlist* selected_ids);

#endif


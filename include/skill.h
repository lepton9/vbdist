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

skill* copySkillVal(skill* s, float value);
skill* copySkill(skill* s);

dlist* initSelectedSkills(dlist* allSkills);
void freeSkills(dlist* skills);

int is_selected_skill(skill* s, dlist* selected_skills);

#endif


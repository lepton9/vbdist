#ifndef SKILL_H
#define SKILL_H

#include "../include/dlist.h"

#define WEIGHT_INCREMENT 0.01

typedef struct {
  int id;
  char* name;
  float value;
  float weight;
} skill;

skill* initSkill(const int id, const char* name, const float value);
void freeSkill(skill* skill);

float skillValue(skill* skill);
void setWeight(skill* skill, float value);
void incWeight(skill* skill);
void decWeight(skill* skill);

skill* copySkillVal(skill* s, float value);
skill* copySkill(skill* s);

dlist* copySkills(dlist* allSkills);
void freeSkills(dlist* skills);

int findSkill(skill* s, dlist* selected_skills);

#endif


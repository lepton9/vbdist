#ifndef SKILL_H
#define SKILL_H

#include "../include/dlist.h"

#define INCREMENT 0.01
#define INCREMENT_MULTI 10

typedef struct {
  int id;
  char* name;
  float value;
  float weight;
} skill;

skill* initSkill(const int id, const char* name, const float value);
void freeSkill(skill* skill);

void updateSkillName(skill* skill, char* name);
float skillValue(skill* skill);
void setWeight(skill* skill, float value);
void incWeight(skill* skill);
void decWeight(skill* skill);
void incValue(skill* skill, const char big_inc);
void decValue(skill* skill, const char big_dec);

skill* copySkillVal(skill* s, float value);
skill* copySkill(skill* s);

dlist* copySkills(dlist* skills);
void freeSkills(dlist* skills);

int findSkill(skill* s, dlist* selected_skills);

#endif


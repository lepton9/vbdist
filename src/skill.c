#include "../include/skill.h"
#include <stdlib.h>
#include <string.h>


skill* initSkill(const int id, const char* name, const float value) {
  skill* s = malloc(sizeof(skill));
  s->id = id;
  s->name = (name) ? strdup(name) : NULL;
  s->value = value;
  s->weight = 1.0;
  return s;
}

void freeSkill(skill* s) {
  if (s->name) free(s->name);
  free(s);
}

void updateSkillName(skill* skill, char* name) {
  if (!skill) return;
  if (skill->name) free(skill->name);
  skill->name = name;
}

float skillValue(skill* skill) {
  return skill->value * skill->weight;
}

void setWeight(skill* skill, float value) {
  if (value < 0) return;
  skill->weight = value;
}

void incWeight(skill* skill) {
  skill->weight += INCREMENT;
}

void decWeight(skill* skill) {
  if (skill->weight > INCREMENT) {
    skill->weight -= INCREMENT;
  } else {
    skill->weight = 0;
  }
}

void incValue(skill* skill, const char big_inc) {
  skill->value += (big_inc) ? INCREMENT * INCREMENT_MULTI : INCREMENT;
}

void decValue(skill* skill, const char big_dec) {
  double am = (big_dec) ? INCREMENT * INCREMENT_MULTI : INCREMENT;
  if (skill->value > am) {
    skill->value -= am;
  } else {
    skill->value = 0;
  }
}

skill* copySkillVal(skill* s, float value) {
  skill* s_c = initSkill(s->id, s->name, value);
  setWeight(s_c, s->weight);
  return s_c;
}

skill* copySkill(skill* s) {
  return copySkillVal(s, s->value);
}

dlist* copySkills(dlist* skills) {
  if (!skills) return NULL;
  dlist* skills_copy = init_list();
  for (size_t i = 0; i < skills->n; i++) {
    list_add(skills_copy, copySkill(skills->items[i]));
  }
  return skills_copy;
}

void freeSkills(dlist* skills) {
  if (!skills) return;
  for (size_t i = 0; i < skills->n; i++) {
    freeSkill(skills->items[i]);
  }
  free_list(skills);
}

int findSkill(skill* s, dlist* skills) {
  if (s) {
    for (size_t i = 0; i < skills->n; i++) {
      skill* skill = skills->items[i];
      if (s->id == skill->id) return i;
    }
  }
  return -1;
}


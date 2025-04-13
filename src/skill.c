#include "../include/skill.h"
#include <stdlib.h>
#include <string.h>


skill* initSkill(const int id, const char* name, const float value) {
  skill* s = malloc(sizeof(skill));
  s->id = id;
  s->name = (name) ? strdup(name) : NULL;
  s->value = value;
  return s;
}

void freeSkill(skill* s) {
  if (s->name) free(s->name);
  free(s);
}

skill* copySkillVal(skill* s, float value) {
  skill* s_c = initSkill(s->id, s->name, value);
  return s_c;
}

skill* copySkill(skill* s) {
  return copySkillVal(s, s->value);
}

dlist* initSelectedSkills(dlist* allSkills) {
  dlist* skills = init_list();
  for (size_t i = 0; i < allSkills->n; i++) {
    skill* s = allSkills->items[i];
    list_add(skills, copySkill(s));
  }
  return skills;
}

void freeSkills(dlist* skills) {
  for (size_t i = 0; i < skills->n; i++) {
    freeSkill(skills->items[i]);
  }
  free_list(skills);
}

int is_selected_skill(skill* s, dlist* selected_skills) {
  for (size_t i = 0; i < selected_skills->n; i++) {
    skill* sel_s = selected_skills->items[i];
    if (s->id == sel_s->id) {
      return i;
    }
  }
  return -1;
}


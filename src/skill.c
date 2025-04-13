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

dlist* initSelectedSkills(dlist* allSkills) {
  dlist* skills = init_list();
  for (size_t i = 0; i < allSkills->n; i++) {
    int* id = malloc(sizeof(int));
    *id = ((skill*)allSkills->items[i])->id;
    list_add(skills, id);
  }
  return skills;
}

void freeSelectedSkills(dlist* selected_skills) {
  for (size_t i = 0; i < selected_skills->n; i++) {
    free(selected_skills->items[i]);
  }
  free_list(selected_skills);
}

int is_selected_skill(skill* skill, dlist* selected_ids) {
  for (size_t i = 0; i < selected_ids->n; i++) {
    if (skill->id == *(int*)selected_ids->items[i]) {
      return i;
    }
  }
  return -1;
}


#ifndef TUISKILLS_H
#define TUISKILLS_H

#include "dlist.h"
#include "listarea.h"
#include "sql.h"
#include "tui.h"
#include "render.h"
#include "sql.h"


typedef struct {
  dlist* skills;
  dlist* selected_skills;
  list_area* skills_area;
  sqldb* db;
  term_size* term;
  renderer* render;
  int modified;
} tui_skills;

tui_skills* init_tui_skills(sqldb* db, dlist* skills, dlist* selectedSkills);
void free_tui_skills(tui_skills* tui);

void runTuiSkills(sqldb* db, dlist* allSkills, dlist* selectedSkills);
void update_skills_area(tui_skills* tui);
void handleSkillsInput(tui_skills* tui, int c);

skill* get_selected_skill(tui_skills* tui);
void toggle_selected_skill(tui_skills* tui);

int delete_skill(tui_skills* tui, int index);
void delete_selected_skill(tui_skills* tui);
void rename_selected_skill(tui_skills* tui);
void decrement_selected_skill(tui_skills* tui);
void increment_selected_skill(tui_skills* tui);
void add_skill(tui_skills* tui);
void updateWeights(sqldb* db, dlist* allSkills, dlist* selectedSkills);

void renderSkillsTui(tui_skills* tui);

#endif

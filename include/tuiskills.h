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
  dlist* selected_skill_ids;
  list_area* skills_area;
  sqldb* db;
  term_size* term;
  renderer* render;
} tui_skills;

tui_skills* init_tui_skills(sqldb* db, dlist* skills, dlist* selectedSkills);
void free_tui_skills(tui_skills* tui);

void runTuiSkills(sqldb* db, dlist* allSkills, dlist* selectedSkills);
void update_skills_area(tui_skills* tui);
void handleSkillsInput(tui_skills* tui, int c);

skill* get_selected_skill(tui_skills* tui);
int is_selected_skill(skill* skill, dlist* selected_ids);
void toggle_selected_skill(tui_skills* tui);

void rename_selected_skill(tui_skills* tui);

void renderSkillsTui(tui_skills* tui);

#endif

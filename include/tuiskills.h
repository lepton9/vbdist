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
void handleSkillsInput(tui_skills* tui, int c);

#endif

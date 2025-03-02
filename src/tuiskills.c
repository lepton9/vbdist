#include "../include/tuiskills.h"

tui_skills* init_tui_skills(sqldb* db, dlist* skills, dlist* selectedSkills) {
  tui_skills* tui = malloc(sizeof(tui_skills));
  tui->db = db;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);
  tui->render = init_renderer(stdout);

  tui->skills_area = init_list_area(tui->term->cols, tui->term->rows);
  tui->skills = skills;
  tui->selected_skill_ids = selectedSkills;
  // tui->skills = fetchSkills(db);

  return tui;
}

void free_tui_skills(tui_skills* tui) {
  free(tui->term);
  free_renderer(tui->render);
  free_list_area(tui->skills_area);
  // for (size_t i = 0; i < tui->skills->n; i++) {
  //   freeSkill(tui->skills->items[i]);
  // }
  // free_list(tui->skills);
  free(tui);
}

void handleSkillsInput(tui_skills *tui, int c) {
  switch (c) {
    case 13: case '\n': case ' ':
#ifdef __linux__
    case KEY_ENTER:
#endif
      break;
    case 27: {  // Esc
      break;
    }
    case 9: // Tab
      break;
    case 'R': case 'r':
      break;
    case 'X': case 'x':
      break;
    case 'K': case 'W':
    case 'k': case 'w':
#ifdef __linux__
    case KEY_UP:
#endif
      list_up(tui->skills_area);
      break;
    case 'j': case 's':
#ifdef __linux__
    case KEY_DOWN:
#endif
      list_down(tui->skills_area);
      break;
    case 'h': case 'a':
      break;
    case 'l': case 'd':
      break;
    default: {
      break;
    }
  }
}

void update_skills_area(tui_skills* tui) {
  getTermSize(tui->term);
  int rows = tui->term->rows - 2;
  int cols = tui->term->cols;
  update_list_area(tui->skills_area, cols, rows);
  setSize(tui->render, tui->term->cols, tui->term->rows);
}

void renderSkillsTui(tui_skills* tui) {
  // TODO: print ui

  render(tui->render);
}

void runTuiSkills(sqldb* db, dlist* allSkills, dlist* selectedSkills) {
  tui_skills* tui = init_tui_skills(db, allSkills, selectedSkills);
  curHide();
  refresh_screen(tui->render);
  int c = 0;
  while (c != 'q') {
    list_init_selected(tui->skills_area);
    update_skills_area(tui);
    renderSkillsTui(tui);
    c = keyPress();
    handleSkillsInput(tui, c);
  }
}



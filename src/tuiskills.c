#include "../include/tuiskills.h"
#include "../include/utils.h"
#include <stdlib.h>

tui_skills* init_tui_skills(sqldb* db, dlist* skills, dlist* selectedSkills) {
  tui_skills* tui = malloc(sizeof(tui_skills));
  tui->db = db;

  tui->term = malloc(sizeof(term_size));
  getTermSize(tui->term);
  tui->render = init_renderer(stdout);

  tui->skills_area = init_list_area(tui->term->cols, tui->term->rows);
  tui->skills = skills;
  tui->selected_skills = selectedSkills;
  update_list_len(tui->skills_area, tui->skills->n);
  tui->modified = 0;

  return tui;
}

void free_tui_skills(tui_skills* tui) {
  free(tui->term);
  free_renderer(tui->render);
  free_list_area(tui->skills_area);
  free(tui);
}

void handleSkillsInput(tui_skills *tui, int c) {
  switch (c) {
    case 13: case '\n': case ' ':
#ifdef __linux__
    case KEY_ENTER:
#endif
      toggle_selected_skill(tui);
      break;
    case 27: {  // Esc
      break;
    }
    case 9: // Tab
      break;
    case 'R': case 'r':
      rename_selected_skill(tui);
      break;
    case 'h': case '-': case 4: // Ctrl + D
      decrement_selected_skill(tui);
      break;
    case 'l': case '+': case 21: // Ctrl + U
      increment_selected_skill(tui);
      break;
    case 'A': case 'a':
      add_skill(tui);
      break;
    case 'X': case 'x':
      delete_selected_skill(tui);
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
    default: {
      break;
    }
  }
}

skill* get_selected_skill(tui_skills* tui) {
  if (tui->skills_area->selected < 0) return NULL;
  return tui->skills->items[tui->skills_area->selected];
}

void toggle_selected_skill(tui_skills* tui) {
  skill* selected = get_selected_skill(tui);
  int i = findSkill(selected, tui->selected_skills);
  if (i >= 0) {
    skill* s = pop_elem(tui->selected_skills, i);
    freeSkill(s);
  } else {
    list_add(tui->selected_skills, copySkill(selected));
  }
}

void rename_selected_skill(tui_skills* tui) {
  skill* selected = get_selected_skill(tui);
  if (!selected) return;
  char* old_name = selected->name;
  int width = tui->skills_area->area->width;
  int row = tui->skills_area->selected_term_row;

  curShow();
  const size_t max_len = 50;
  size_t len = strlen((old_name) ? old_name : 0);
  char new[max_len + 1];
  strcpy(new, (old_name) ? old_name : "");
  int c = 0;
  while (1) {
    curSet(row, width - 1);
    printf("\033[1K");
    curSet(row, 1);
    printf("|> %s", new);
    fflush(stdout);
    c = keyPress();
    if (c == 27) {
      break;
    } else if (isEnter(c)) {
      if (*new == '\0') {
        break;
      }
      if (renameSkill(tui->db, selected, new)) {
        if (selected->name) free(selected->name);
        selected->name = strdup(new);
        break;
      }
    } else if (len > 0 && isBackspace(c)) {
      new[len - 1] = '\0';
      len--;
    } else if (len < max_len) {
      if (c >= 32 && c <= 126) {
        strcatc(new, &len, c);
      }
    }
  }
  curHide();
  refresh_screen(tui->render);
}

void decrement_selected_skill(tui_skills* tui) {
  skill* selected = get_selected_skill(tui);
  if (!selected) return;
  decWeight(selected);
  tui->modified = 1;
}

void increment_selected_skill(tui_skills* tui) {
  skill* selected = get_selected_skill(tui);
  if (!selected) return;
  incWeight(selected);
  tui->modified = 1;
}

int delete_skill(tui_skills* tui, int index) {
  if (index < 0) return 0;
  if (deleteSkill(tui->db, tui->skills->items[index])) {
    skill* s = pop_elem(tui->skills, tui->skills_area->selected);
    freeSkill(s);
    return 1;
  }
  return 0;
}

void delete_selected_skill(tui_skills* tui) {
  skill* selected = get_selected_skill(tui);
  if (!selected) return;
  int row = tui->skills_area->selected_term_row;
  int width = tui->skills_area->area->width;
  curShow();
  curSet(row, width - 1);
  printf("\033[1K");
  curSet(row, 1);
  printf("|> Delete %s? [y/N]", selected->name);
  fflush(stdout);
  char c = keyPress();
  if (c == 'y') {
    if (delete_skill(tui, tui->skills_area->selected)) {
      update_list_len(tui->skills_area, tui->skills->n);
    }
  }
  curHide();
  refresh_screen(tui->render);
}

void add_skill(tui_skills* tui) {
  int width = tui->skills_area->area->width;
  int row = 3;

  curShow();
  const size_t max_len = 50;
  size_t len = 0;
  char new[max_len + 1];
  new[0] = '\0';
  int c = 0;
  while (1) {
    curSet(row, width - 1);
    printf("\033[1K");
    curSet(row, 3);
    printf("Enter new skill: %s", new);
    fflush(stdout);
    c = keyPress();
    if (c == 27) {
      break;
    } else if (isEnter(c)) {
      if (*new == '\0') {
        break;
      }
      skill* new_skill = initSkill(-1, new, 0);
      if (insertSkill(tui->db, new_skill)) {
        list_add(tui->skills, new_skill);
        update_list_len(tui->skills_area, tui->skills->n);
      } else {
        freeSkill(new_skill);
      }
      break;
    } else if (len > 0 && isBackspace(c)) {
      new[len - 1] = '\0';
      len--;
    } else if (len < max_len) {
      if (c >= 32 && c <= 126) {
        strcatc(new, &len, c);
      }
    }
  }
  curHide();
  refresh_screen(tui->render);
}

void update_skills_area(tui_skills* tui) {
  getTermSize(tui->term);
  int rows = tui->term->rows - 5;
  int cols = tui->term->cols;
  update_list_area(tui->skills_area, cols, rows);
}

void renderSkillsTui(tui_skills* tui) {
  put_text(tui->render, 1, 2, "\033[4m %s \033[24m", "Selected skills");

  int line = 3;
  int len = min_int(min_int(tui->term->rows - line, (int)tui->skills_area->max_shown), (int)tui->skills->n - (tui->skills_area->first_ind));
  if (tui->skills->n == 0) len = 0;

  for (int i = tui->skills_area->first_ind; i < tui->skills_area->first_ind + len; i++) {
    skill* cur_skill = tui->skills->items[i];

    append_line(tui->render, line, "  ");
    if (tui->skills_area->selected == i) {
      tui->skills_area->selected_term_row = line + 1;
      append_line(tui->render, line, "\033[7m");
    }
    if (findSkill(cur_skill, tui->selected_skills) >= 0) {
      append_line(tui->render, line, ">");
    }

    append_line(tui->render, line, " %-18s %.2f", cur_skill->name, cur_skill->weight);
    if (tui->skills_area->selected == i) {
      append_line(tui->render, line, "\033[27m");
    }
    line++;
  }
  make_borders(tui->render, 0, 0, 30, len + 5);
  render(tui->render);
}

void updateWeights(sqldb* db, dlist* allSkills, dlist* selectedSkills) {
  updateSkillWeights(db, allSkills);
  for (size_t i = 0; i < selectedSkills->n; i++) {
    skill* selSkill = selectedSkills->items[i];
    int i = findSkill(selSkill, allSkills);
    if (i >= 0) {
      skill* s = allSkills->items[i];
      selSkill->weight = s->weight;
    }
  }
}

void runTuiSkills(sqldb* db, dlist* allSkills, dlist* selectedSkills) {
  tui_skills* tui = init_tui_skills(db, allSkills, selectedSkills);
  curHide();
  refresh_screen(tui->render);
  int c = 0;
  while (c != 'q') {
    check_selected(tui->skills_area);
    update_skills_area(tui);
    renderSkillsTui(tui);
    c = keyPress();
    handleSkillsInput(tui, c);
  }
  if (tui->modified) {
    updateWeights(db, allSkills, selectedSkills);
  }
  free_tui_skills(tui);
}


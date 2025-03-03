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
  tui->selected_skill_ids = selectedSkills;
  update_list_len(tui->skills_area, tui->skills->n);
  // tui->skills = fetchSkills(db);

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

int is_selected_skill(skill* skill, dlist* selected_ids) {
  for (size_t i = 0; i < selected_ids->n; i++) {
    if (skill->id == *(int*)selected_ids->items[i]) {
      return i;
    }
  }
  return -1;
}

skill* get_selected_skill(tui_skills* tui) {
  if (tui->skills_area->selected < 0) return NULL;
  return tui->skills->items[tui->skills_area->selected];
}

void toggle_selected_skill(tui_skills* tui) {
  skill* selected = get_selected_skill(tui);
  int i = is_selected_skill(selected, tui->selected_skill_ids);
  if (i >= 0) {
    int* id = pop_elem(tui->selected_skill_ids, i);
    free(id);
  } else {
    int* id = malloc(sizeof(int));
    *id = selected->id;
    list_add(tui->selected_skill_ids, id);
  }
}

void rename_selected_skill(tui_skills* tui) {
  skill* selected = get_selected_skill(tui);
  if (!selected) return;
  char* old_name = selected->name;
  int width = tui->skills_area->width;
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

void delete_selected_skill(tui_skills* tui) {
  skill* selected = get_selected_skill(tui);

}

void add_skill(tui_skills* tui) {

}

void update_skills_area(tui_skills* tui) {
  getTermSize(tui->term);
  int rows = tui->term->rows - 2;
  int cols = tui->term->cols;
  update_list_area(tui->skills_area, cols, rows);
  setSize(tui->render, tui->term->cols, tui->term->rows);
}

void renderSkillsTui(tui_skills* tui) {
  append_line(tui->render, 0, "\033[4m %s \033[24m", "Selected skills");

  // TODO: make func to iterate tui lists
  int line = 2;
  for (int i = tui->skills_area->first_ind;
  line <= tui->term->rows - 1 &&
  i - tui->skills_area->first_ind + 1 <= (int)tui->skills_area->max_shown &&
  i < (int)tui->skills->n;
  i++) {
    skill* cur_skill = tui->skills->items[i];

    if (tui->skills_area->selected == i) {
      tui->skills_area->selected_term_row = line + 1;
      append_line(tui->render, line, "\033[7m");
    }
    if (is_selected_skill(cur_skill, tui->selected_skill_ids) >= 0) {
      append_line(tui->render, line, ">");
    }

    append_line(tui->render, line, " %-20s", cur_skill->name);
    if (tui->skills_area->selected == i) {
      append_line(tui->render, line, "\033[27m");
    }
    line++;
  }

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



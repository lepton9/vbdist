#include "../include/playeredit.h"


player_edit* initPlayerEdit(size_t area_w, size_t area_h) {
  player_edit* edit = malloc(sizeof(player_edit));
  edit->active = 0;
  edit->modified = 0;
  edit->selected_element = SKILLS_LIST;
  edit->lists_index = 0;
  edit->p = NULL;
  edit->positions = init_list();
  edit->positionsArea = init_list_area(area_w, area_h);
  return edit;
}

void freePlayerEdit(player_edit* edit) {
  if (!edit) return;
  free_list_area(edit->positionsArea);
  freePositions(edit->positions);
  free(edit);
}

void pedit_list_up(player_edit* pedit) {
  switch (pedit->selected_element) {
    case SKILLS_LIST:
      if (pedit->lists_index > 0) {
        pedit->lists_index--;
      }
      break;
    case POSITIONS_LIST:
      if (pedit->lists_index <= 0) {
        pedit->lists_index = (pedit->p->skills->n > 0) ? pedit->p->skills->n - 1 : 0;
        pedit->selected_element = SKILLS_LIST;
      } else {
        pedit->lists_index--;
      }
      break;
    default:
      break;
  }
}

void pedit_list_down(player_edit* pedit) {
  switch (pedit->selected_element) {
    case SKILLS_LIST:
      if (pedit->lists_index >= (int)pedit->p->skills->n - 1) {
        pedit->lists_index = 0;
        pedit->selected_element = POSITIONS_LIST;
      } else {
        pedit->lists_index++;
      }
      break;
    case POSITIONS_LIST:
      if (pedit->lists_index < (int)pedit->p->positions->n - 1) {
        pedit->lists_index++;
      }
      break;
    default:
      break;
  }
}

void pedit_reset_positions(player_edit* pedit) {
  for (int i = pedit->positions->n - 1; i >= 0; i--) {
    freePosition(pop_elem(pedit->positions, i));
  }
}

void pedit_filtered_positions(player_edit* pedit, dlist* all_positions) {
  if (!pedit->active || !pedit->p) return;
  pedit_reset_positions(pedit);
  player* p = pedit->p;
  for (size_t i = 0; i < all_positions->n; i++) {
    position* pos = get_elem(all_positions, i);
    int ind = findPosition(p->positions, pos);
    if (ind < 0) {
      list_add(pedit->positions, copy_position(pos));
    }
  }
  update_list_len(pedit->positionsArea, pedit->positions->n);
}

void pedit_add_position(player_edit* pedit) {
  if (!pedit->active || !pedit->p) return;
  int ind = pedit->positionsArea->selected;
  if (ind < 0 || ind >= (int)pedit->positions->n) return;
  position* pos = pop_elem(pedit->positions, ind);
  addPositionLast(pedit->p, pos);
  update_list_len(pedit->positionsArea, pedit->positions->n);
  pedit->modified = 1;
}

void pedit_remove_position(player_edit* pedit) {
  if (!pedit->active || !pedit->p ||
      pedit->selected_element != POSITIONS_LIST)
    return;
  int ind = pedit->lists_index;
  player* p = pedit->p;
  if (ind < 0 || ind >= (int)p->positions->n) return;
  position* pos = popPosition(pedit->p, ind);
  if (pos) {
    list_add(pedit->positions, pos);
    update_list_len(pedit->positionsArea, pedit->positions->n);
    if (ind >= (int)p->positions->n) {
      pedit->lists_index = p->positions->n - 1;
    }
  }
  pedit->modified = 1;
}

skill* pedit_selected_skill(player_edit* pedit) {
  player* p = pedit->p;
  if (!p || p->skills->n == 0 || pedit->lists_index > (int)p->skills->n - 1)
    return NULL;
  return get_elem(pedit->p->skills, pedit->lists_index);
}

position* pedit_selected_position(player_edit* pedit) {
  player* p = pedit->p;
  if (!p || p->positions->n == 0 || pedit->lists_index > (int)p->positions->n - 1)
    return NULL;
  return get_elem(pedit->p->positions, pedit->lists_index);
}


#ifndef PLAYEREDIT_H
#define PLAYEREDIT_H

#include "player.h"
#include "listarea.h"

typedef enum {
  SKILLS_LIST,
  POSITIONS_LIST,
  NO_ELEMENT
} p_edit_element;

typedef struct {
  char active;
  char modified;
  p_edit_element selected_element;
  int lists_index;
  player* p;
  dlist* positions;
  list_area* positionsArea;
} player_edit;

player_edit* initPlayerEdit(size_t area_w, size_t area_h);
void freePlayerEdit(player_edit* edit);

void pedit_list_up(player_edit* pedit);
void pedit_list_down(player_edit* pedit);
skill* pedit_selected_skill(player_edit* pedit);
position* pedit_selected_position(player_edit* pedit);
void pedit_reset_positions(player_edit* pedit);
void pedit_filtered_positions(player_edit* pedit, dlist* all_positions);
void pedit_add_position(player_edit* pedit);
void pedit_remove_position(player_edit* pedit);


#endif

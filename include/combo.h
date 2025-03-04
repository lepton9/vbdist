#ifndef COMBO_H
#define COMBO_H

#include <stdlib.h>
#include "../include/team.h"
#include "dlist.h"

typedef enum {
  BAN = 0,
  PAIR,
  DEFAULT
} comboType;

typedef struct {
  int pidA;
  int pidB;
  comboType type;
  int combo_id;
} pCombo;

const char* comboTypeString(comboType type);
comboType toComboType(const char* type);
void freeCombos(dlist* combos);
void addCombo(dlist* combos, comboType type, int a, int b);
int isInCombo(dlist* combos, player* a);
char isCombo(dlist* combos, player* a, player* b);
char comboInTeam(dlist* combos, team* t, player* p);

#endif

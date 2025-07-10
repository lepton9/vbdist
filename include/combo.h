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
  dlist* ids;
  comboType type;
  int combo_id;
} combo;


combo* initCombo(comboType type, int combo_id);
void freeCombo(combo* combo);

const char* comboTypeString(comboType type);
comboType toComboType(const char* type);
void freeCombos(dlist* combos);

void addToCombo(combo* combo, int a);
void addCombo(dlist* combos, comboType type, int a, int b);

int isInCombo(combo* combo, player* a);
int isInSomeCombo(dlist* combos, player* a);
char isCombo(dlist* combos, player* a, player* b);
char comboInTeam(dlist* combos, team* t, player* p);
char comboInTeamSize(dlist* combos, team* t, size_t team_size, player* p);
int comboRelevant(dlist* players, combo* combo);
size_t comboSize(combo* combo);

#endif

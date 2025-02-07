#ifndef COMBO_H
#define COMBO_H

#include <stdlib.h>
#include "../include/team.h"
#include "dlist.h"

typedef struct {
  int pidA;
  int pidB;
} pCombo;

void freeCombos(dlist* combos);
void addCombo(dlist* combos, int a, int b);
int isInCombo(dlist* combos, player* a);
char isCombo(dlist* combos, player* a, player* b);
char comboInTeam(dlist* combos, team* t, player* p);

#endif

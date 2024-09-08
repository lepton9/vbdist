#ifndef COMBO_H
#define COMBO_H

#include <stdlib.h>
#include "../include/team.h"

typedef struct {
  int pidA;
  int pidB;
} pCombo;

typedef struct {
  pCombo* combos;
  size_t n;
} pCombos;

pCombos* initCombos();
void addCombo(pCombos* combos, int a, int b);
void freeCombos(pCombos* combos);
int isInCombo(pCombos* combos, player* a);
char isCombo(pCombos* combos, player* a, player* b);
char comboInTeam(pCombos* combos, team* t, player* p);

#endif

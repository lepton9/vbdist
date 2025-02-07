#include "../include/combo.h"
#include <stdlib.h>

void freeCombos(dlist *combos) {
  for (int i = 0; i < (int)combos->n; i++) {
    free(combos->items[i]);
  }
  free_list(combos);
}

void addCombo(dlist* combos, int a, int b) {
  pCombo* combo = malloc(sizeof(pCombo));
  combo->pidA = a;
  combo->pidB = b;
  list_add(combos, combo);
}

int isInCombo(dlist* combos, player* a) {
  for (int i = 0; i < (int)combos->n; i++) {
    pCombo* combo = combos->items[i];
    if ((combo->pidA == a->id || combo->pidB == a->id)) {
      return i;
    }
  }
  return -1;
}

char isCombo(dlist* combos, player* a, player* b) {
  for (int i = 0; i < (int)combos->n; i++) {
    pCombo* combo = combos->items[i];
    if ((combo->pidA == a->id && combo->pidB == b->id) ||
      (combo->pidA == b->id && combo->pidB == a->id)) {
      return 1;
    }
  }
  return 0;
}

char comboInTeam(dlist* combos, team* t, player* p) {
  for (int i = 0; i < (int)t->size; i++) {
    if (isCombo(combos, p, t->players[i])) return 1;
  }
  return 0;
}


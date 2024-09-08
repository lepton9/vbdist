#include "../include/combo.h"

pCombos* initCombos() {
  pCombos* combos = malloc(sizeof(pCombos));
  combos->combos = malloc(sizeof(pCombo));
  combos->n = 0;
  return combos;
}

void addCombo(pCombos* combos, int a, int b) {
  combos->combos = realloc(combos->combos, (combos->n + 1) * sizeof(pCombo));
  combos->combos[combos->n].pidA = a;
  combos->combos[combos->n].pidB = b;
  combos->n++;
}

void freeCombos(pCombos* combos) {
  free(combos->combos);
  free(combos);
}

int isInCombo(pCombos* combos, player* a) {
  for (int i = 0; i < combos->n; i++) {
    if ((combos->combos[i].pidA == a->id || combos->combos[i].pidB == a->id)) {
      return i;
    }
  }
  return -1;
}

char isCombo(pCombos* combos, player* a, player* b) {
  for (int i = 0; i < combos->n; i++) {
    if ((combos->combos[i].pidA == a->id && combos->combos[i].pidB == b->id) ||
      (combos->combos[i].pidA == b->id && combos->combos[i].pidB == a->id)) {
      return 1;
    }
  }
  return 0;
}

char comboInTeam(pCombos* combos, team* t, player* p) {
  for (int i = 0; i < t->size; i++) {
    if (isCombo(combos, p, t->players[i])) return 1;
  }
  return 0;
}


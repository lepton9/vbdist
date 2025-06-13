#include "../include/combo.h"
#include <stdlib.h>
#include <string.h>

const char* comboTypeString(comboType type) {
  switch (type) {
    case BAN: return "BAN";
    case PAIR: return "PAIR";
    default: return "DEFAULT";
  }
}

comboType toComboType(const char* type) {
  if (strcmp(type, "BAN") == 0) {
    return BAN;
  } else if (strcmp(type, "PAIR") == 0) {
    return PAIR;
  } else {
    return DEFAULT;
  }
}

void freeCombos(dlist *combos) {
  if (!combos) return;
  for (size_t i = 0; i < combos->n; i++) {
    freeCombo(combos->items[i]);
  }
  free_list(combos);
}

combo* initCombo(comboType type, int combo_id) {
  combo* c = malloc(sizeof(combo));
  c->ids = init_list();
  c->type = type;
  c->combo_id = combo_id;
  return c;
}

void freeCombo(combo* combo) {
  if (!combo) return;
  for (size_t i = 0; i < combo->ids->n; i++) {
    free(combo->ids->items[i]);
  }
  if (combo->ids) free_list(combo->ids);
  free(combo);
}

void addCombo(dlist* combos, comboType type, int a, int b) {
  combo* combo = initCombo(type, -1);
  addToCombo(combo, a);
  addToCombo(combo, b);
  list_add(combos, combo);
}

void addToCombo(combo* combo, int a) {
  int* id = malloc(sizeof(int));
  *id = a;
  list_add(combo->ids, id);
}

int isInCombo(combo* combo, player* a) {
  for (size_t i = 0; i < combo->ids->n; i++) {
    if (*((int*)combo->ids->items[i]) == a->id) return 1;
  }
  return 0;
}

int isInSomeCombo(dlist* combos, player* a) {
  for (size_t i = 0; i < combos->n; i++) {
    combo* combo = combos->items[i];
    if (isInCombo(combo, a)) {
      return i;
    }
  }
  return -1;
}

char isCombo(dlist* combos, player* a, player* b) {
  if (a->id == b->id) return 0;
  for (size_t i = 0; i < combos->n; i++) {
    combo* combo = combos->items[i];
    if (isInCombo(combo, a) && isInCombo(combo, b)) {
      return 1;
    }
  }
  return 0;
}

char comboInTeam(dlist* combos, team* t, player* p) {
  for (size_t i = 0; i < t->size; i++) {
    if (isCombo(combos, p, t->players[i])) return 1;
  }
  return 0;
}

char comboInTeamSize(dlist* combos, team* t, size_t team_size, player* p) {
  for (size_t i = 0; i < team_size; i++) {
    if (isCombo(combos, p, t->players[i])) return 1;
  }
  return 0;
}

int comboRelevant(dlist* players, combo* combo) {
  int match = 0;
  for (int i = 0; i < (int)players->n; i++) {
    player* p = players->items[i];
    if (isInCombo(combo, p)) {
      match++;
    }
  }
  return match == (int)combo->ids->n;
}


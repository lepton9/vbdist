#include "../include/dlist.h"

dlist *init_list(size_t item_size) {
  dlist *list = malloc(sizeof(dlist));
  list->n = 0;
  list->size = 1;
  list->item_size = item_size;
  list->items = malloc(list->item_size);
  return list;
}

void free_list(dlist *list) {
  if (list->items) {
    free(list->items);
  }
  free(list);
}

void list_add(dlist *list, void *item) {
  if (list->n >= list->size) {
    list->size *= 2;
    list->items = realloc(list->items, list->size * list->item_size);
  }
  list->items[list->n++] = item;
}

dlist *list_from(void **items, int item_size, int n) {
  dlist *list = init_list(item_size);
  free(list->items);
  list->n = n;
  list->size = n;
  list->items = items;
  return list;
}


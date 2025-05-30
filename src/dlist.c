#include "../include/dlist.h"
#include "../include/utils.h"

dlist *init_list() {
  dlist *list = init_list_size(sizeof(void*));
  return list;
}

dlist *init_list_size(size_t item_size) {
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

int shrink_list(dlist* list) {
  if (list->n < list->size / 4 && list->size > 4) {
    size_t new_size = (list->n > 0) ? list->n * 2 : 4;
    void** new_items = realloc(list->items, new_size * list->item_size);
    if (new_items == NULL) return -1;
    list->items = new_items;
    list->size = new_size;
  }
  return 0;
}

void* pop_elem(dlist* list, size_t index) {
  if (!list || list->n == 0 || index >= list->n) return NULL;
  void* e = list->items[index];
  for (size_t i = index; i < list->n - 1; i++) {
    list->items[i] = list->items[i + 1];
  }
  list->n--;
  shrink_list(list);
  return e;
}

void swap_elems(dlist* list, const int a, const int b) {
  void* t = list->items[a];
  list->items[a] = list->items[b];
  list->items[b] = t;
}

dlist *list_from(void **items, int item_size, int n) {
  dlist *list = init_list_size(item_size);
  free(list->items);
  list->n = n;
  list->size = n;
  list->items = items;
  return list;
}

void shuffle(dlist* list) {
  if (list->n <= 1) return;
  for (size_t i = 0; i < list->n - 1; i++) {
    swap_elems(list, i, rand_int(i, list->n - 1));
  }
}


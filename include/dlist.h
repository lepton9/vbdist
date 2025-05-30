#ifndef DLIST_H
#define DLIST_H

#include <stdlib.h>

typedef struct {
  void** items;
  size_t n;
  size_t size;
  size_t item_size;
} dlist;

dlist* init_list_size(size_t item_size);
dlist* init_list();
void free_list(dlist* list);
int shrink_list(dlist* list);
void list_add(dlist* list, void* item);
dlist* list_from(void** items, int item_size, int n);
void swap_elems(dlist* list, const int a, const int b);
void shuffle(dlist* list);

// The caller is responsible for freeing the element
void* pop_elem(dlist* list, size_t index);

#endif

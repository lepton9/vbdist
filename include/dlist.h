#ifndef DLIST_H
#define DLIST_H

#include <stdlib.h>

typedef struct {
  void** items;
  size_t n;
  size_t size;
  size_t item_size;
} dlist;

dlist* init_list(size_t item_size);
void free_list(dlist* list);
void list_add(dlist* list, void* item);
dlist* list_from(void** items, int item_size, int n);

#endif

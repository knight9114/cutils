#ifndef __CUTILS_LINKED_LIST_H__
#define __CUTILS_LINKED_LIST_H__

#include "cutils/errors.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct linked_list_node {
  struct linked_list_node *prev;
  struct linked_list_node *next;
  void *value;
} linked_list_node_t;

linked_list_node_t *linked_list_node_init(void *);

typedef struct linked_list {
  size_t length;
  linked_list_node_t *head;
  linked_list_node_t *tail;
  void (*inner_free)(void *);
  void (*outer_free)(void (*)(void *), void *);
} linked_list_t;

cutils_error_t linked_list_init(linked_list_t *l, void (*inner_free)(void *),
                                void (*outer_free)(void (*)(void *), void *));
void linked_list_free(void *ptr);
void linked_list_free_value(linked_list_t *l, linked_list_node_t *node);
cutils_error_t linked_list_insert_at(linked_list_t *l, size_t idx, void *value);
cutils_error_t linked_list_push_front(linked_list_t *l, void *value);
cutils_error_t linked_list_push_back(linked_list_t *l, void *value);
cutils_error_t linked_list_remove_at(linked_list_t *l, size_t idx,
                                     void **value);
cutils_error_t linked_list_pop_front(linked_list_t *l, void **value);
cutils_error_t linked_list_pop_back(linked_list_t *l, void **value);
cutils_error_t linked_list_get(linked_list_t *l, size_t idx, void **value);
cutils_error_t linked_list_set(linked_list_t *l, size_t idx, void *value);

#endif // __CUTILS_LINKED_LIST_H__

#ifndef __CUTILS_ARRAY_LIST_H__
#define __CUTILS_ARRAY_LIST_H__

#include "cutils/errors.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct array_list {
  size_t length;
  size_t capacity;
  void **backing;
  void (*inner_free)(void *);
  void (*outer_free)(void (*)(void *), void *);
} array_list_t;

cutils_error_t array_list_init(array_list_t *l, size_t capacity,
                               void (*inner_free)(void *),
                               void (*outer_free)(void (*)(void *), void *));
void array_list_free(void *ptr);
void array_list_free_value(array_list_t *l, size_t idx);
cutils_error_t array_list_grow(array_list_t *l, size_t capacity);
cutils_error_t array_list_insert_at(array_list_t *l, size_t idx, void *value);
cutils_error_t array_list_push(array_list_t *l, void *value);
cutils_error_t array_list_remove_at(array_list_t *l, size_t idx, void **value);
cutils_error_t array_list_pop(array_list_t *l, void **value);
cutils_error_t array_list_get(array_list_t *l, size_t idx, void **value);
cutils_error_t array_list_set(array_list_t *l, size_t idx, void *value);
cutils_error_t array_list_find(array_list_t *l, void *value,
                               bool (*cmp)(void *, void *), size_t *idx);

#endif // __CUTILS_ARRAY_LIST_H__

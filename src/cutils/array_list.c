#include "cutils/array_list.h"
#include "cutils/errors.h"
#include <stddef.h>
#include <stdlib.h>

cutils_error_t array_list_init(array_list_t *l, size_t capacity,
                               void (*inner_free)(void *),
                               void (*outer_free)(void (*)(void *), void *)) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  l->length = 0;
  l->capacity = capacity;
  l->inner_free = inner_free;
  l->outer_free = outer_free;
  l->backing = malloc(sizeof(void *) * l->capacity);
  if (!l->backing) {
    free(l);
    return CUTILS_ALLOCATION_ERROR;
  }

  return CUTILS_SUCCESS;
}

void array_list_free(void *ptr) {
  if (ptr) {
    array_list_t *l = ptr;
    for (size_t i = 0; i < l->length; i++) {
      array_list_free_value(l, i);
    }
    free(l);
  }
}

void array_list_free_value(array_list_t *l, size_t idx) {
  if (l && idx < l->length) {
    if (l->outer_free && l->inner_free) {
      l->outer_free(l->inner_free, l->backing[idx]);
    } else if (l->inner_free) {
      l->inner_free(l->backing[idx]);
    }
  }
}

cutils_error_t array_list_grow(array_list_t *l, size_t capacity) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  if (capacity <= l->capacity) {
    return CUTILS_RESIZE_ERROR;
  }

  void **update = realloc(l->backing, sizeof(void *) * capacity);
  if (!update) {
    return CUTILS_ALLOCATION_ERROR;
  }

  l->capacity = capacity;
  l->backing = update;

  return CUTILS_SUCCESS;
}

cutils_error_t array_list_insert_at(array_list_t *l, size_t idx, void *value) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  if (idx > l->length) {
    return CUTILS_INDEX_ERROR;
  }

  if (l->length == l->capacity) {
    cutils_error_t err = array_list_grow(l, l->capacity * 2);
    if (err != CUTILS_SUCCESS) {
      return err;
    }
  }

  for (size_t i = l->length; i > idx; i--) {
    l->backing[i] = l->backing[i - 1];
  }

  l->backing[idx] = value;
  l->length++;

  return CUTILS_SUCCESS;
}

cutils_error_t array_list_push(array_list_t *l, void *value) {
  return array_list_insert_at(l, l->length, value);
}

cutils_error_t array_list_remove_at(array_list_t *l, size_t idx, void **value) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  if (idx >= l->length) {
    return CUTILS_INDEX_ERROR;
  }

  if (value) {
    *value = l->backing[idx];
  }

  for (size_t i = idx; i < l->length; i++) {
    l->backing[i] = l->backing[i + 1];
  }
  l->backing[l->length - 1] = NULL;

  l->backing--;

  return CUTILS_SUCCESS;
}

cutils_error_t array_list_pop(array_list_t *l, void **value) {
  return array_list_remove_at(l, l->length - 1, value);
}

cutils_error_t array_list_get(array_list_t *l, size_t idx, void **value) {
  if (!l || !value) {
    return CUTILS_NULL_ERROR;
  }

  if (idx >= l->length) {
    return CUTILS_INDEX_ERROR;
  }

  *value = l->backing[idx];

  return CUTILS_SUCCESS;
}

cutils_error_t array_list_set(array_list_t *l, size_t idx, void *value) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  if (idx >= l->length) {
    return CUTILS_INDEX_ERROR;
  }

  array_list_free_value(l, idx);
  l->backing[idx] = value;

  return CUTILS_SUCCESS;
}

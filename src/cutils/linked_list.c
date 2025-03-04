#include "cutils/linked_list.h"
#include "cutils/errors.h"

linked_list_node_t *linked_list_node_init(void *value) {
  linked_list_node_t *n = malloc(sizeof(linked_list_node_t));
  if (!n) {
    return NULL;
  }

  n->prev = NULL;
  n->next = NULL;
  n->value = value;

  return n;
}

cutils_error_t linked_list_init(linked_list_t *l, void (*inner_free)(void *),
                                void (*outer_free)(void (*)(void *), void *)) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  l->length = 0;
  l->head = NULL;
  l->tail = NULL;
  l->inner_free = inner_free;
  l->outer_free = outer_free;

  return CUTILS_SUCCESS;
}

void linked_list_free(void *ptr) {
  if (ptr) {
    linked_list_t *l = ptr;

    linked_list_node_t *curr = l->head;
    while (curr) {
      linked_list_node_t *next = curr->next;
      linked_list_free_value(l, curr);
      free(curr);
      curr = next;
    }

    free(l);
  }
}

void linked_list_free_value(linked_list_t *l, linked_list_node_t *node) {
  if (l && node) {
    if (l->outer_free && l->inner_free) {
      l->outer_free(l->inner_free, node->value);
    } else if (l->inner_free) {
      l->inner_free(node->value);
    } else if (l->outer_free) {
      // Special case: composite type with stack-allocated value
      free(node->value);
    }
  }
}

cutils_error_t linked_list_insert_at(linked_list_t *l, size_t idx,
                                     void *value) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  if (idx > l->length) {
    return CUTILS_INDEX_ERROR;
  }

  linked_list_node_t *node = linked_list_node_init(value);
  if (!node) {
    return CUTILS_ALLOCATION_ERROR;
  }

  if (idx == 0) {
    node->next = l->head;
    if (l->head) {
      l->head->prev = node;
    }
    l->head = node;
    if (!l->tail) {
      l->tail = node;
    }
  } else if (idx == l->length) {
    node->prev = l->tail;
    if (l->tail) {
      l->tail->next = node;
    }
    l->tail = node;
  } else {
    linked_list_node_t *curr = l->head;
    for (size_t i = 0; i < idx; i++) {
      curr = curr->next;
    }

    node->next = curr;
    node->prev = curr->prev;
    if (curr->prev) {
      curr->prev->next = node;
    }
    curr->prev = node;
  }

  l->length++;

  return CUTILS_SUCCESS;
}

cutils_error_t linked_list_push_front(linked_list_t *l, void *value) {
  return linked_list_insert_at(l, 0, value);
}

cutils_error_t linked_list_push_back(linked_list_t *l, void *value) {
  return linked_list_insert_at(l, l->length, value);
}

cutils_error_t linked_list_remove_at(linked_list_t *l, size_t idx,
                                     void **value) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  if (idx >= l->length) {
    return CUTILS_INDEX_ERROR;
  }

  linked_list_node_t *curr = l->head;
  for (size_t i = 0; i < idx; i++) {
    curr = curr->next;
  }

  if (value) {
    *value = curr->value;
  }

  if (curr->prev) {
    curr->prev->next = curr->next;
  }

  if (curr->next) {
    curr->next->prev = curr->prev;
  }

  if (curr == l->head) {
    l->head = curr->next;
  }

  if (curr == l->tail) {
    l->tail = curr->prev;
  }

  free(curr);

  l->length--;

  return CUTILS_SUCCESS;
}

cutils_error_t linked_list_pop_front(linked_list_t *l, void **value) {
  return linked_list_remove_at(l, 0, value);
}

cutils_error_t linked_list_pop_back(linked_list_t *l, void **value) {
  return linked_list_remove_at(l, l->length - 1, value);
}

cutils_error_t linked_list_get(linked_list_t *l, size_t idx, void **value) {
  if (!l || !value) {
    return CUTILS_NULL_ERROR;
  }

  if (idx >= l->length) {
    return CUTILS_INDEX_ERROR;
  }

  linked_list_node_t *curr = l->head;
  for (size_t i = 0; i < idx; i++) {
    curr = curr->next;
  }

  *value = curr->value;

  return CUTILS_SUCCESS;
}

cutils_error_t linked_list_set(linked_list_t *l, size_t idx, void *value) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  if (idx >= l->length) {
    return CUTILS_INDEX_ERROR;
  }

  linked_list_node_t *curr = l->head;
  for (size_t i = 0; i < idx; i++) {
    curr = curr->next;
  }

  linked_list_free_value(l, curr);
  curr->value = value;

  return CUTILS_SUCCESS;
}

cutils_error_t linked_list_find(linked_list_t *l, void *value,
                                bool (*cmp)(void *, void *), size_t *idx) {
  if (!l) {
    return CUTILS_NULL_ERROR;
  }

  linked_list_node_t *curr = l->head;
  for (size_t i = 0; i < l->length; i++) {
    if (cmp(curr->value, value)) {
      *idx = i;
      break;
    }
    curr = curr->next;
  }

  return CUTILS_SUCCESS;
}

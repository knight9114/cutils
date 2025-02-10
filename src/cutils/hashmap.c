#include "cutils/hashmap.h"
#include "cutils/array_list.h"
#include "cutils/errors.h"
#include "cutils/linked_list.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void hashmap_entry_free(void (*inner_free)(void *), void *ptr) {
  if (inner_free && ptr) {
    hashmap_entry_t *entry = ptr;
    inner_free(entry->value);
    free(ptr);
  }
}

cutils_error_t hashmap_init(hashmap_t *m, size_t nchains,
                            uint64_t (*hash)(void *),
                            void (*inner_free)(void *)) {
  if (!m || !hash) {
    return CUTILS_NULL_ERROR;
  }

  m->length = 0;
  m->nchains = nchains;
  m->nlinks = 0;
  m->hash = hash;
  m->inner_free = inner_free;
  m->chains = malloc(sizeof(array_list_t));
  cutils_error_t err =
      array_list_init(m->chains, m->nchains, linked_list_free, NULL);
  if (err != CUTILS_SUCCESS) {
    free(m);
    return err;
  }

  for (size_t i = 0; i < m->nchains; i++) {
    linked_list_t *l = malloc(sizeof(linked_list_t));
    err = linked_list_init(l, inner_free, hashmap_entry_free);
    if (err != CUTILS_SUCCESS) {
      // FIXME: properly handle freeing memory here
      free(m);
      return err;
    }

    array_list_insert_at(m->chains, i, l);
  }

  return CUTILS_SUCCESS;
}

void hashmap_free(void *ptr) {
  if (ptr) {
    hashmap_t *m = ptr;
    array_list_free(m->chains);
    free(ptr);
  }
}

cutils_error_t hashmap_resize(hashmap_t *m, size_t nchains) {
  if (!m) {
    return CUTILS_NULL_ERROR;
  }

  array_list_t *update = malloc(sizeof(array_list_t));
  cutils_error_t err = array_list_init(update, nchains, linked_list_free, NULL);
  if (err != CUTILS_SUCCESS) {
    return err;
  }

  for (size_t i = 0; i < nchains; i++) {
    linked_list_t *l = malloc(sizeof(linked_list_t));
    err = linked_list_init(l, m->inner_free, hashmap_entry_free);
    if (err != CUTILS_SUCCESS) {
      return err;
    }

    array_list_insert_at(update, i, l);
  }

  size_t nlinks = 0;
  linked_list_t *l = NULL;
  for (size_t i = 0; i < m->nchains; i++) {
    err = array_list_pop(m->chains, (void **)&l);
    if (err != CUTILS_SUCCESS) {
      return err;
    }

    while (l->length > 0) {
      hashmap_entry_t *entry = NULL;
      err = linked_list_pop_front(l, (void **)&entry);
      if (err != CUTILS_SUCCESS) {
        return err;
      }

      size_t key = entry->key % nchains;
      linked_list_t *u = NULL;
      err = array_list_get(update, key, (void **)&u);
      if (err != CUTILS_SUCCESS) {
        return err;
      }

      err = linked_list_push_back(u, entry);
      if (err != CUTILS_SUCCESS) {
        return err;
      }

      nlinks = (nlinks < u->length) ? u->length : nlinks;
    }

    linked_list_free(l);
  }

  array_list_free(m->chains);
  m->chains = update;
  m->nlinks = nlinks;
  m->nchains = nchains;

  return CUTILS_SUCCESS;
}

cutils_error_t hashmap_insert(hashmap_t *m, void *key, void *value) {
  if (!m || !key) {
    return CUTILS_NULL_ERROR;
  }

  uint64_t hash = m->hash(key);
  size_t k = hash % m->nchains;
  linked_list_t *l = NULL;
  cutils_error_t err = array_list_get(m->chains, k, (void **)&l);
  if (err != CUTILS_SUCCESS) {
    return err;
  }

  linked_list_node_t *curr = l->head;
  while (curr) {
    hashmap_entry_t *entry = curr->value;
    if (entry->key == hash) {
      m->inner_free(entry->value);
      entry->value = value;
      break;
    }

    curr = curr->next;
  }

  if (!curr) {
    hashmap_entry_t *entry = malloc(sizeof(hashmap_entry_t));
    if (!entry) {
      return CUTILS_ALLOCATION_ERROR;
    }

    entry->key = hash;
    entry->value = value;

    err = linked_list_push_back(l, entry);
    if (err != CUTILS_SUCCESS) {
      return err;
    }

    m->nlinks = (m->nlinks < l->length) ? l->length : m->nlinks;
    m->length++;
  }

  return CUTILS_SUCCESS;
}

cutils_error_t hashmap_remove(hashmap_t *m, void *key, void **value) {
  if (!m || !key) {
    return CUTILS_NULL_ERROR;
  }

  uint64_t hash = m->hash(key);
  size_t k = hash % m->nchains;
  linked_list_t *l = NULL;
  cutils_error_t err = array_list_get(m->chains, k, (void **)&l);
  if (err != CUTILS_SUCCESS) {
    return err;
  }

  size_t idx = 0;
  linked_list_node_t *curr = l->head;
  while (curr) {
    hashmap_entry_t *entry = curr->value;
    if (entry->key == hash) {
      break;
    }

    curr = curr->next;
    idx++;
  }

  if (idx == l->length) {
    return CUTILS_INDEX_ERROR;
  }

  hashmap_entry_t *entry = NULL;
  err = linked_list_remove_at(l, idx, (void **)&entry);
  if (err != CUTILS_SUCCESS) {
    return err;
  }

  if (value) {
    *value = entry->value;
  }

  free(entry);
  m->length--;

  return CUTILS_SUCCESS;
}

cutils_error_t hashmap_get(hashmap_t *m, void *key, void **value) {
  if (!m || !key || !value) {
    return CUTILS_NULL_ERROR;
  }

  uint64_t hash = m->hash(key);
  size_t k = hash % m->nchains;
  linked_list_t *l = NULL;
  cutils_error_t err = array_list_get(m->chains, k, (void **)&l);
  if (err != CUTILS_SUCCESS) {
    return err;
  }

  size_t idx = 0;
  linked_list_node_t *curr = l->head;
  while (curr) {
    hashmap_entry_t *entry = curr->value;
    if (entry->key == hash) {
      break;
    }

    curr = curr->next;
    idx++;
  }

  if (idx == l->length) {
    return CUTILS_INDEX_ERROR;
  }

  hashmap_entry_t *entry = NULL;
  err = linked_list_get(l, idx, (void **)&entry);
  if (err != CUTILS_SUCCESS) {
    return err;
  }

  *value = entry->value;

  return CUTILS_SUCCESS;
}

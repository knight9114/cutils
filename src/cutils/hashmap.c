#include "cutils/hashmap.h"
#include "cutils/array_list.h"
#include "cutils/errors.h"
#include "cutils/linked_list.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

cutils_error_t hashmap_init(hashmap_t *m, size_t nchains,
                            uint64_t (*hash)(void *),
                            bool (*key_cmp)(void *, void *),
                            void (*key_free)(void *),
                            void (*inner_free)(void *)) {
  if (!m || !hash || !key_cmp) {
    return CUTILS_NULL_ERROR;
  }

  m->length = 0;
  m->nchains = nchains;
  m->nlinks = 0;
  m->hash = hash;
  m->key_cmp = key_cmp;
  m->key_free = key_free;
  m->inner_free = inner_free;
  m->chains = malloc(sizeof(array_list_t));
  if (!m->chains) {
    return CUTILS_ALLOCATION_ERROR;
  }

  cutils_error_t err =
      array_list_init(m->chains, m->nchains, linked_list_free, NULL);
  if (err != CUTILS_SUCCESS) {
    free(m->chains);
    return err;
  }

  for (size_t i = 0; i < m->nchains; i++) {
    linked_list_t *l = malloc(sizeof(linked_list_t));
    if (!l) {
      // Clean up previously allocated lists
      m->chains->length = i;
      array_list_free(m->chains);
      free(m->chains);
      return CUTILS_ALLOCATION_ERROR;
    }
    // Entries are freed manually in hashmap_free, so pass NULL for free functions
    err = linked_list_init(l, NULL, NULL);
    if (err != CUTILS_SUCCESS) {
      free(l);
      m->chains->length = i;
      array_list_free(m->chains);
      free(m->chains);
      return err;
    }
    // This push will not fail if logic is correct
    array_list_push(m->chains, l);
  }

  return CUTILS_SUCCESS;
}

void hashmap_free(void *ptr) {
  if (ptr) {
    hashmap_t *m = ptr;
    if (m->chains) {
      for (size_t i = 0; i < m->chains->length; i++) {
        linked_list_t *l = NULL;
        array_list_get(m->chains, i, (void **)&l);
        if (l) {
          linked_list_node_t *curr = l->head;
          while (curr) {
            hashmap_entry_t *entry = curr->value;
            if (entry) {
              if (m->key_free) {
                m->key_free(entry->original_key);
              }
              if (m->inner_free) {
                m->inner_free(entry->value);
              }
              free(entry);
            }
            curr = curr->next;
          }
        }
      }
      array_list_free(m->chains);
    }
    free(m);
  }
}

cutils_error_t hashmap_resize(hashmap_t *m, size_t nchains) {
  if (!m) {
    return CUTILS_NULL_ERROR;
  }

  array_list_t *update = malloc(sizeof(array_list_t));
  cutils_error_t err = array_list_init(update, nchains, linked_list_free, NULL);
  if (err != CUTILS_SUCCESS) {
    free(update);
    return err;
  }

  for (size_t i = 0; i < nchains; i++) {
    linked_list_t *l = malloc(sizeof(linked_list_t));
    err = linked_list_init(l, NULL, NULL);
    if (err != CUTILS_SUCCESS) {
      update->length = i;
      array_list_free(update);
      free(update);
      return err;
    }
    array_list_push(update, l);
  }

  size_t nlinks = 0;
  for (size_t i = 0; i < m->nchains; i++) {
    linked_list_t *l = NULL;
    array_list_get(m->chains, i, (void **)&l);

    while (l->length > 0) {
      hashmap_entry_t *entry = NULL;
      linked_list_pop_front(l, (void **)&entry);

      size_t key = entry->key % nchains;
      linked_list_t *u = NULL;
      array_list_get(update, key, (void **)&u);
      linked_list_push_back(u, entry);

      nlinks = (nlinks < u->length) ? u->length : nlinks;
    }
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
    if (entry->key == hash && m->key_cmp(entry->original_key, key)) {
      if (m->key_free) {
        m->key_free(entry->original_key);
      }
      if (m->inner_free) {
        m->inner_free(entry->value);
      }
      entry->original_key = key;
      entry->value = value;
      return CUTILS_SUCCESS;
    }
    curr = curr->next;
  }

  hashmap_entry_t *entry = malloc(sizeof(hashmap_entry_t));
  if (!entry) {
    return CUTILS_ALLOCATION_ERROR;
  }

  entry->key = hash;
  entry->original_key = key;
  entry->value = value;

  err = linked_list_push_back(l, entry);
  if (err != CUTILS_SUCCESS) {
    free(entry);
    return err;
  }

  m->nlinks = (m->nlinks < l->length) ? l->length : m->nlinks;
  m->length++;

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
    if (entry->key == hash && m->key_cmp(entry->original_key, key)) {
      break;
    }
    curr = curr->next;
    idx++;
  }

  if (!curr) {
    return CUTILS_INDEX_ERROR;
  }

  hashmap_entry_t *entry = NULL;
  err = linked_list_remove_at(l, idx, (void **)&entry);
  if (err != CUTILS_SUCCESS) {
    return err;
  }

  if (value) {
    *value = entry->value;
  } else if (m->inner_free) {
    m->inner_free(entry->value);
  }

  if (m->key_free) {
    m->key_free(entry->original_key);
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

  linked_list_node_t *curr = l->head;
  while (curr) {
    hashmap_entry_t *entry = curr->value;
    if (entry->key == hash && m->key_cmp(entry->original_key, key)) {
      *value = entry->value;
      return CUTILS_SUCCESS;
    }
    curr = curr->next;
  }

  return CUTILS_INDEX_ERROR;
}

#ifndef __CUTILS_HASHMAP_H__
#define __CUTILS_HASHMAP_H__

#include "cutils/array_list.h"
#include "cutils/errors.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint64_t key;
  void *original_key;
  void *value;
} hashmap_entry_t;

typedef struct {
  size_t length;
  size_t nchains;
  size_t nlinks;
  array_list_t *chains;
  uint64_t (*hash)(void *);
  bool (*key_cmp)(void *, void *);
  void (*key_free)(void *);
  void (*inner_free)(void *);
} hashmap_t;

cutils_error_t hashmap_init(hashmap_t *m, size_t nchains,
                            uint64_t (*hash)(void *),
                            bool (*key_cmp)(void *, void *),
                            void (*key_free)(void *),
                            void (*inner_free)(void *));
void hashmap_free(void *ptr);
cutils_error_t hashmap_resize(hashmap_t *m, size_t nchains);
cutils_error_t hashmap_insert(hashmap_t *m, void *key, void *value);
cutils_error_t hashmap_remove(hashmap_t *m, void *key, void **value);
cutils_error_t hashmap_get(hashmap_t *m, void *key, void **value);

#endif // __CUTILS_HASHMAP_H__

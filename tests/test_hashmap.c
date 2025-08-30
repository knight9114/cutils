#include "cutils/array_list.h"
#include "cutils/errors.h"
#include "cutils/hashmap.h"
#include "cutils/linked_list.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  size_t n;
  size_t *values;
} test_data_t;

test_data_t *_new(size_t n) {
  test_data_t *t = malloc(sizeof(test_data_t));
  assert(t != NULL);
  t->values = malloc(sizeof(size_t) * n);
  assert(t->values != NULL);

  t->n = n;
  for (size_t i = 0; i < n; i++) {
    t->values[i] = i;
  }

  return t;
}

uint64_t hash_key(void *ptr) {
  if (ptr) {
    size_t *key = ptr;
    return (uint64_t)*key;
  }
  return 0;
}

bool cmp_key(void *lhs, void *rhs) {
  if (lhs && rhs) {
    return *(size_t *)lhs == *(size_t *)rhs;
  }
  return lhs == rhs;
}

void inner_free(void *ptr) {
  if (ptr) {
    test_data_t *t = ptr;
    if (t) {
      free(t->values);
      free(t);
    }
  }
}

bool verify_value(void *lhs, size_t n) {
  if (lhs) {
    test_data_t *t = lhs;
    if (t->n != n) {
      return false;
    }

    for (size_t i = 0; i < t->n; i++) {
      if (t->values[i] != i) {
        return false;
      }
    }

    return true;
  }
  return false;
}

void test_hashmap_init_and_free(void) {
  printf("testing hashmap_init_and_free ... ");

  hashmap_t *m = malloc(sizeof(hashmap_t));
  cutils_error_t err = hashmap_init(m, 10, hash_key, cmp_key, NULL, inner_free);
  assert(err == CUTILS_SUCCESS);
  assert(m->length == 0);

  hashmap_free(m);

  printf("success\n");
}

void test_hashmap_insert(void) {
  printf("testing hashmap_insert heap-allocated ... ");

  hashmap_t *m = malloc(sizeof(hashmap_t));
  cutils_error_t err = hashmap_init(m, 10, hash_key, cmp_key, free, inner_free);
  assert(err == CUTILS_SUCCESS);
  assert(m->length == 0);

  size_t *k1 = malloc(sizeof(size_t));
  *k1 = 11;
  test_data_t *v1 = _new(2);
  err = hashmap_insert(m, k1, v1);
  assert(err == CUTILS_SUCCESS);
  linked_list_t *l1 = NULL;
  array_list_get(m->chains, 1, (void **)&l1);
  hashmap_entry_t *e1 = l1->head->value;
  assert(l1->length == 1);
  assert(verify_value(e1->value, 2));

  size_t *k2 = malloc(sizeof(size_t));
  *k2 = 21;
  test_data_t *v2 = _new(4);
  err = hashmap_insert(m, k2, v2);
  assert(err == CUTILS_SUCCESS);
  linked_list_t *l2 = NULL;
  array_list_get(m->chains, 1, (void **)&l2);
  hashmap_entry_t *e2 = l2->head->next->value;
  assert(l2->length == 2);
  assert(verify_value(e2->value, 4));

  size_t *k3 = malloc(sizeof(size_t));
  *k3 = 11;
  test_data_t *v3 = _new(8);
  err = hashmap_insert(m, k3, v3);
  assert(err == CUTILS_SUCCESS);
  free(k3); // Key not used, insert should have replaced old value.
  linked_list_t *l3 = NULL;
  array_list_get(m->chains, 1, (void **)&l3);
  hashmap_entry_t *e3 = l3->head->value;
  assert(l3->length == 2);
  assert(verify_value(e3->value, 8));

  hashmap_free(m);

  printf("success\n");
}

void test_hashmap_resize(void) {
  printf("testing hashmap_resize ... ");

  hashmap_t *m = malloc(sizeof(hashmap_t));
  cutils_error_t err = hashmap_init(m, 10, hash_key, cmp_key, free, inner_free);
  assert(err == CUTILS_SUCCESS);
  assert(m->length == 0);

  size_t *k1 = malloc(sizeof(size_t));
  *k1 = 11;
  test_data_t *v1 = _new(2);
  err = hashmap_insert(m, k1, v1);
  assert(err == CUTILS_SUCCESS);

  size_t *k2 = malloc(sizeof(size_t));
  *k2 = 21;
  test_data_t *v2 = _new(4);
  err = hashmap_insert(m, k2, v2);
  assert(err == CUTILS_SUCCESS);

  err = hashmap_resize(m, 11);
  assert(err == CUTILS_SUCCESS);
  linked_list_t *l0 = NULL;
  linked_list_t *l10 = NULL;
  assert(array_list_get(m->chains, 0, (void **)&l0) == CUTILS_SUCCESS);
  assert(array_list_get(m->chains, 10, (void **)&l10) == CUTILS_SUCCESS);
  assert(l0->length == 1);
  assert(l10->length == 1);

  hashmap_free(m);

  printf("success\n");
}

void test_hashmap_remove(void) {
  printf("testing hashmap_remove ... ");

  hashmap_t *m = malloc(sizeof(hashmap_t));
  cutils_error_t err = hashmap_init(m, 10, hash_key, cmp_key, free, inner_free);
  assert(err == CUTILS_SUCCESS);
  assert(m->length == 0);

  size_t *k1 = malloc(sizeof(size_t));
  *k1 = 11;
  test_data_t *v1 = _new(2);
  err = hashmap_insert(m, k1, v1);
  assert(err == CUTILS_SUCCESS);
  size_t *k2 = malloc(sizeof(size_t));
  *k2 = 21;
  test_data_t *v2 = _new(4);
  err = hashmap_insert(m, k2, v2);
  assert(err == CUTILS_SUCCESS);

  test_data_t *v3 = NULL;
  err = hashmap_remove(m, k1, (void **)&v3);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(v3, 2));
  assert(m->length == 1);
  inner_free(v3);

  size_t k4 = 12345;
  test_data_t *v4 = NULL;
  err = hashmap_remove(m, &k4, (void **)&v4);
  assert(err == CUTILS_INDEX_ERROR);

  hashmap_free(m);

  printf("success\n");
}

void test_hashmap_get(void) {
  printf("testing hashmap_get ... ");

  hashmap_t *m = malloc(sizeof(hashmap_t));
  cutils_error_t err = hashmap_init(m, 10, hash_key, cmp_key, free, inner_free);
  assert(err == CUTILS_SUCCESS);
  assert(m->length == 0);

  size_t *k1 = malloc(sizeof(size_t));
  *k1 = 11;
  test_data_t *v1 = _new(2);
  err = hashmap_insert(m, k1, v1);
  assert(err == CUTILS_SUCCESS);

  size_t *k2 = malloc(sizeof(size_t));
  *k2 = 21;
  test_data_t *v2 = _new(4);
  err = hashmap_insert(m, k2, v2);
  assert(err == CUTILS_SUCCESS);

  test_data_t *v3 = NULL;
  err = hashmap_get(m, k2, (void **)&v3);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(v3, 4));

  err = hashmap_get(m, NULL, (void **)&v3);
  assert(err == CUTILS_NULL_ERROR);

  size_t k4 = 12345;
  err = hashmap_get(m, &k4, (void **)&v3);
  assert(err == CUTILS_INDEX_ERROR);

  hashmap_free(m);

  printf("success\n");
}

void test_hashmap_stack_insert(void) {
  printf("testing hashmap_insert stack-allocated ... ");

  hashmap_t *m = malloc(sizeof(hashmap_t));
  cutils_error_t err = hashmap_init(m, 10, hash_key, cmp_key, NULL, NULL);
  assert(err == CUTILS_SUCCESS);
  assert(m->length == 0);

  size_t k1 = 11;
  size_t v1 = 11;
  err = hashmap_insert(m, &k1, &v1);
  assert(err == CUTILS_SUCCESS);
  linked_list_t *l1 = NULL;
  array_list_get(m->chains, 1, (void **)&l1);
  hashmap_entry_t *e1 = l1->head->value;
  assert(l1->length == 1);
  assert(*(size_t *)e1->value == 11);
  assert(e1->original_key == &k1);

  size_t k2 = 21;
  size_t v2 = 21;
  err = hashmap_insert(m, &k2, &v2);
  assert(err == CUTILS_SUCCESS);
  linked_list_t *l2 = NULL;
  array_list_get(m->chains, 1, (void **)&l2);
  hashmap_entry_t *e2 = l2->head->next->value;
  assert(l2->length == 2);
  assert(*(size_t *)e2->value == 21);
  assert(e2->original_key == &k2);

  size_t k3 = 11;
  size_t v3 = 23;
  err = hashmap_insert(m, &k3, &v3);
  assert(err == CUTILS_SUCCESS);
  linked_list_t *l3 = NULL;
  array_list_get(m->chains, 1, (void **)&l3);
  hashmap_entry_t *e3 = l3->head->value;
  assert(l3->length == 2);
  assert(*(size_t *)e3->value == 23);
  assert(e3->original_key == &k1);

  hashmap_free(m);

  printf("success\n");
}

int main(void) {
  test_hashmap_init_and_free();
  test_hashmap_insert();
  test_hashmap_stack_insert();
  test_hashmap_resize();
  test_hashmap_remove();
  test_hashmap_get();
  return EXIT_SUCCESS;
}

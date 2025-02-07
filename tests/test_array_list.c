#include "cutils/array_list.h"
#include "cutils/errors.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  size_t n;
  size_t *values;
} test_data_inner_t;

typedef struct {
  test_data_inner_t *d;
} test_data_t;

test_data_t *_new(size_t n) {
  test_data_t *t = malloc(sizeof(test_data_t));
  assert(t != NULL);
  t->d = malloc(sizeof(test_data_inner_t));
  assert(t->d != NULL);
  t->d->values = malloc(sizeof(size_t) * n);
  assert(t->d->values != NULL);

  t->d->n = n;
  for (size_t i = 0; i < n; i++) {
    t->d->values[i] = i;
  }

  return t;
}

void inner_free(void *ptr) {
  if (ptr) {
    test_data_inner_t *d = ptr;
    if (d) {
      free(d->values);
      free(d);
    }
  }
}

void outer_free(void (*fn)(void *), void *ptr) {
  if (ptr && fn) {
    test_data_t *t = ptr;
    fn(t->d);
    free(t);
  }
}

bool verify_value(void *lhs) {
  if (lhs) {
    test_data_t *l = lhs;
    if (l->d) {
      for (size_t i = 0; i < l->d->n; i++) {
        if (l->d->values[i] != i) {
          return false;
        }
      }
      return true;
    }
    return false;
  }
  return false;
}

void test_array_list_init_and_free(void) {
  printf("testing array_list_init_and_free ... ");

  array_list_t *l = malloc(sizeof(array_list_t));
  cutils_error_t err = array_list_init(l, 8, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);
  assert(l->capacity == 8);

  array_list_free(l);

  printf("success\n");
}

void test_array_list_insert_at(void) {
  printf("testing array_list_insert_at ... ");

  array_list_t *l = malloc(sizeof(array_list_t));
  cutils_error_t err = array_list_init(l, 2, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = array_list_insert_at(l, 0, v1);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 1);
  assert(verify_value(l->backing[0]));

  test_data_t *v2 = _new(4);
  err = array_list_insert_at(l, 0, v2);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 2);
  assert(verify_value(l->backing[0]));
  assert(verify_value(l->backing[1]));

  test_data_t *v3 = _new(8);
  err = array_list_insert_at(l, 0, v3);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 3);
  assert(l->capacity == 4);
  assert(verify_value(l->backing[0]));
  assert(verify_value(l->backing[1]));
  assert(verify_value(l->backing[2]));

  err = array_list_insert_at(l, 10, v3);
  assert(err == CUTILS_INDEX_ERROR);

  array_list_free(l);

  printf("success\n");
}

void test_array_list_push(void) {
  printf("testing array_list_push ... ");

  array_list_t *l = malloc(sizeof(array_list_t));
  cutils_error_t err = array_list_init(l, 2, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = array_list_push(l, v1);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 1);
  assert(verify_value(l->backing[0]));

  test_data_t *v2 = _new(4);
  err = array_list_push(l, v2);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 2);
  assert(verify_value(l->backing[0]));
  assert(verify_value(l->backing[1]));

  test_data_t *v3 = _new(8);
  err = array_list_push(l, v3);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 3);
  assert(l->capacity == 4);
  assert(verify_value(l->backing[0]));
  assert(verify_value(l->backing[1]));
  assert(verify_value(l->backing[2]));

  array_list_free(l);

  printf("success\n");
}

void test_array_list_remove_at(void) {
  printf("testing array_list_remove_at ... ");

  array_list_t *l = malloc(sizeof(array_list_t));
  cutils_error_t err = array_list_init(l, 8, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = array_list_push(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);
  err = array_list_push(l, v2);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v3 = _new(8);
  err = array_list_push(l, v3);
  assert(err == CUTILS_SUCCESS);

  test_data_t *item = NULL;
  err = array_list_remove_at(l, 0, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item));
  assert(l->length == 2);
  assert(verify_value(l->backing[0]));
  assert(verify_value(l->backing[1]));
  outer_free(inner_free, item);

  err = array_list_remove_at(l, 0, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item));
  assert(l->length == 1);
  assert(verify_value(l->backing[0]));
  outer_free(inner_free, item);

  err = array_list_remove_at(l, 0, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item));
  assert(l->length == 0);
  outer_free(inner_free, item);

  err = array_list_remove_at(l, 0, (void **)&item);
  assert(err == CUTILS_INDEX_ERROR);

  array_list_free(l);

  printf("success\n");
}

void test_array_list_pop(void) {
  printf("testing array_list_pop ... ");

  array_list_t *l = malloc(sizeof(array_list_t));
  cutils_error_t err = array_list_init(l, 8, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = array_list_push(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);
  err = array_list_push(l, v2);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v3 = _new(8);
  err = array_list_push(l, v3);
  assert(err == CUTILS_SUCCESS);

  test_data_t *item = NULL;
  err = array_list_pop(l, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item));
  assert(l->length == 2);
  assert(verify_value(l->backing[0]));
  assert(verify_value(l->backing[1]));
  outer_free(inner_free, item);

  err = array_list_pop(l, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item));
  assert(l->length == 1);
  assert(verify_value(l->backing[0]));
  outer_free(inner_free, item);

  err = array_list_pop(l, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item));
  assert(l->length == 0);
  outer_free(inner_free, item);

  err = array_list_pop(l, (void **)&item);
  assert(err == CUTILS_INDEX_ERROR);

  array_list_free(l);

  printf("success\n");
}

void test_array_list_get(void) {
  printf("testing array_list_get ... ");

  array_list_t *l = malloc(sizeof(array_list_t));
  cutils_error_t err = array_list_init(l, 8, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = array_list_push(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);
  err = array_list_push(l, v2);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v3 = _new(8);
  err = array_list_push(l, v3);
  assert(err == CUTILS_SUCCESS);

  test_data_t *item = NULL;
  err = array_list_get(l, 0, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item));

  err = array_list_get(l, 1, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item));

  err = array_list_get(l, 2, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item));

  err = array_list_get(l, 3, (void **)&item);
  assert(err == CUTILS_INDEX_ERROR);

  err = array_list_get(l, 0, NULL);
  assert(err == CUTILS_NULL_ERROR);

  array_list_free(l);

  printf("success\n");
}

void test_array_list_set(void) {
  printf("testing array_list_set ... ");

  array_list_t *l = malloc(sizeof(array_list_t));
  cutils_error_t err = array_list_init(l, 8, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = array_list_push(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);

  err = array_list_set(l, 0, v2);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(l->backing[0]));

  err = array_list_set(l, 1, v2);
  assert(err == CUTILS_INDEX_ERROR);

  array_list_free(l);

  printf("success\n");
}

int main(void) {
  test_array_list_init_and_free();
  test_array_list_insert_at();
  test_array_list_push();
  test_array_list_remove_at();
  test_array_list_pop();
  test_array_list_get();
  test_array_list_set();
  return EXIT_SUCCESS;
}

#include "cutils/errors.h"
#include "cutils/linked_list.h"
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

bool cmp(void *lhs, void *rhs) {
  if (lhs && rhs) {
    test_data_t *l = lhs;
    test_data_t *r = rhs;
    return l->d->n == r->d->n;
  }
  return lhs == NULL && rhs == NULL;
}

bool verify_value(void *lhs, size_t n) {
  if (lhs) {
    test_data_t *l = lhs;
    if (l->d) {
      if (l->d->n != n) {
        return false;
      }

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

void test_linked_list_init_and_free(void) {
  printf("testing linked_list_init_and_free ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  linked_list_free(l);

  printf("success\n");
}

void test_linked_list_insert_at(void) {
  printf("testing linked_list_insert_at ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = linked_list_insert_at(l, 0, v1);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 1);
  assert(verify_value(l->head->value, 2));

  test_data_t *v2 = _new(4);
  err = linked_list_insert_at(l, 0, v2);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 2);
  assert(verify_value(l->head->value, 4));
  assert(verify_value(l->head->next->value, 2));

  test_data_t *v3 = _new(8);
  err = linked_list_insert_at(l, 0, v3);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 3);
  assert(verify_value(l->head->value, 8));
  assert(verify_value(l->head->next->value, 4));
  assert(verify_value(l->head->next->next->value, 2));

  err = linked_list_insert_at(l, 10, v3);
  assert(err == CUTILS_INDEX_ERROR);

  linked_list_free(l);

  printf("success\n");
}

void test_linked_list_push_front(void) {
  printf("testing linked_list_push_front ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = linked_list_push_front(l, v1);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 1);
  assert(verify_value(l->head->value, 2));

  test_data_t *v2 = _new(4);
  err = linked_list_push_front(l, v2);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 2);
  assert(verify_value(l->head->value, 4));
  assert(verify_value(l->head->next->value, 2));

  test_data_t *v3 = _new(8);
  err = linked_list_push_front(l, v3);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 3);
  assert(verify_value(l->head->value, 8));
  assert(verify_value(l->head->next->value, 4));
  assert(verify_value(l->head->next->next->value, 2));

  linked_list_free(l);

  printf("success\n");
}

void test_linked_list_push_back(void) {
  printf("testing linked_list_push_back ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = linked_list_push_back(l, v1);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 1);
  assert(verify_value(l->head->value, 2));

  test_data_t *v2 = _new(4);
  err = linked_list_push_back(l, v2);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 2);
  assert(verify_value(l->head->value, 2));
  assert(verify_value(l->head->next->value, 4));

  test_data_t *v3 = _new(8);
  err = linked_list_push_back(l, v3);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 3);
  assert(verify_value(l->head->value, 2));
  assert(verify_value(l->head->next->value, 4));
  assert(verify_value(l->head->next->next->value, 8));

  linked_list_free(l);

  printf("success\n");
}

void test_linked_list_remove_at(void) {
  printf("testing linked_list_remove_at ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = linked_list_push_back(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);
  err = linked_list_push_back(l, v2);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v3 = _new(8);
  err = linked_list_push_back(l, v3);
  assert(err == CUTILS_SUCCESS);

  test_data_t *item = NULL;
  err = linked_list_remove_at(l, 0, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 2));
  assert(l->length == 2);
  assert(verify_value(l->head->value, 4));
  assert(verify_value(l->head->next->value, 8));
  outer_free(inner_free, item);

  err = linked_list_remove_at(l, 0, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 4));
  assert(l->length == 1);
  assert(verify_value(l->head->value, 8));
  outer_free(inner_free, item);

  err = linked_list_remove_at(l, 0, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 8));
  assert(l->length == 0);
  outer_free(inner_free, item);

  err = linked_list_remove_at(l, 0, (void **)&item);
  assert(err == CUTILS_INDEX_ERROR);

  linked_list_free(l);

  printf("success\n");
}

void test_linked_list_pop_front(void) {
  printf("testing linked_list_pop_front ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = linked_list_push_back(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);
  err = linked_list_push_back(l, v2);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v3 = _new(8);
  err = linked_list_push_back(l, v3);
  assert(err == CUTILS_SUCCESS);

  test_data_t *item = NULL;
  err = linked_list_pop_front(l, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 2));
  assert(l->length == 2);
  assert(verify_value(l->head->value, 4));
  assert(verify_value(l->head->next->value, 8));
  outer_free(inner_free, item);

  err = linked_list_pop_front(l, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 4));
  assert(l->length == 1);
  assert(verify_value(l->head->value, 8));
  outer_free(inner_free, item);

  err = linked_list_pop_front(l, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 8));
  assert(l->length == 0);
  outer_free(inner_free, item);

  err = linked_list_pop_front(l, (void **)&item);
  assert(err == CUTILS_INDEX_ERROR);

  linked_list_free(l);

  printf("success\n");
}

void test_linked_list_pop_back(void) {
  printf("testing linked_list_pop_back ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = linked_list_push_back(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);
  err = linked_list_push_back(l, v2);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v3 = _new(8);
  err = linked_list_push_back(l, v3);
  assert(err == CUTILS_SUCCESS);

  test_data_t *item = NULL;
  err = linked_list_pop_back(l, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 8));
  assert(l->length == 2);
  assert(verify_value(l->head->value, 2));
  assert(verify_value(l->head->next->value, 4));
  outer_free(inner_free, item);

  err = linked_list_pop_back(l, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 4));
  assert(l->length == 1);
  assert(verify_value(l->head->value, 2));
  outer_free(inner_free, item);

  err = linked_list_pop_back(l, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 2));
  assert(l->length == 0);
  outer_free(inner_free, item);

  err = linked_list_pop_back(l, (void **)&item);
  assert(err == CUTILS_INDEX_ERROR);

  linked_list_free(l);

  printf("success\n");
}

void test_linked_list_get(void) {
  printf("testing linked_list_get ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = linked_list_push_back(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);
  err = linked_list_push_back(l, v2);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v3 = _new(8);
  err = linked_list_push_back(l, v3);
  assert(err == CUTILS_SUCCESS);

  test_data_t *item = NULL;
  err = linked_list_get(l, 0, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 2));

  err = linked_list_get(l, 1, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 4));

  err = linked_list_get(l, 2, (void **)&item);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(item, 8));

  err = linked_list_get(l, 3, (void **)&item);
  assert(err == CUTILS_INDEX_ERROR);

  err = linked_list_get(l, 0, NULL);
  assert(err == CUTILS_NULL_ERROR);

  linked_list_free(l);

  printf("success\n");
}

void test_linked_list_set(void) {
  printf("testing linked_list_set ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = linked_list_push_back(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);

  err = linked_list_set(l, 0, v2);
  assert(err == CUTILS_SUCCESS);
  assert(verify_value(l->head->value, 4));

  err = linked_list_set(l, 1, v2);
  assert(err == CUTILS_INDEX_ERROR);

  linked_list_free(l);

  printf("success\n");
}

void test_linked_list_find(void) {
  printf("testing linked_list_find ... ");

  linked_list_t *l = malloc(sizeof(linked_list_t));
  cutils_error_t err = linked_list_init(l, inner_free, outer_free);
  assert(err == CUTILS_SUCCESS);
  assert(l->length == 0);

  test_data_t *v1 = _new(2);
  err = linked_list_push_back(l, v1);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v2 = _new(4);
  err = linked_list_push_back(l, v2);
  assert(err == CUTILS_SUCCESS);
  test_data_t *v3 = _new(8);
  err = linked_list_push_back(l, v3);
  assert(err == CUTILS_SUCCESS);

  size_t idx = SIZE_MAX;
  err = linked_list_find(l, v1, cmp, &idx);
  assert(err == CUTILS_SUCCESS);
  assert(idx == 0);

  idx = SIZE_MAX;
  err = linked_list_find(l, v2, cmp, &idx);
  assert(err == CUTILS_SUCCESS);
  assert(idx == 1);

  idx = SIZE_MAX;
  err = linked_list_find(l, v3, cmp, &idx);
  assert(err == CUTILS_SUCCESS);
  assert(idx == 2);

  test_data_t *v4 = _new(1);
  idx = SIZE_MAX;
  err = linked_list_find(l, v4, cmp, &idx);
  assert(err == CUTILS_SUCCESS);
  assert(idx == SIZE_MAX);

  outer_free(inner_free, v4);
  linked_list_free(l);

  printf("success\n");
}

int main(void) {
  test_linked_list_init_and_free();
  test_linked_list_insert_at();
  test_linked_list_push_front();
  test_linked_list_push_back();
  test_linked_list_remove_at();
  test_linked_list_pop_front();
  test_linked_list_pop_back();
  test_linked_list_get();
  test_linked_list_set();
  test_linked_list_find();
  return EXIT_SUCCESS;
}

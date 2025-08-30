#include "cutils/json.h"
#include "cutils/errors.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_parse_literals(void) {
  printf("testing json_parse literals ... ");
  json_value_t *val = NULL;
  cutils_error_t err;

  err = json_parse("null", &val);
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_NULL);
  json_value_free(val);

  err = json_parse("true", &val);
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_BOOLEAN);
  assert(val->as.boolean == true);
  json_value_free(val);

  err = json_parse("false", &val);
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_BOOLEAN);
  assert(val->as.boolean == false);
  json_value_free(val);

  printf("success\n");
}

void test_parse_numbers(void) {
  printf("testing json_parse numbers ... ");
  json_value_t *val = NULL;
  cutils_error_t err;

  err = json_parse("123", &val);
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_NUMBER);
  assert(val->as.number == 123);
  json_value_free(val);

  err = json_parse("-123.45e-2", &val);
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_NUMBER);
  assert(fabs(val->as.number - (-1.2345)) < 1e-9);
  json_value_free(val);

  printf("success\n");
}

void test_parse_strings(void) {
  printf("testing json_parse strings ... ");
  json_value_t *val = NULL;
  cutils_error_t err;

  err = json_parse("\"hello world\"", &val);
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_STRING);
  assert(strcmp(val->as.string, "hello world") == 0);
  json_value_free(val);

  err = json_parse("\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"", &val);
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_STRING);
  assert(strcmp(val->as.string, "\"\\/\b\f\n\r\t") == 0);
  json_value_free(val);

  err = json_parse("\"\\u20AC\"", &val); // Euro sign
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_STRING);
  assert(strcmp(val->as.string, "€") == 0);
  json_value_free(val);

  printf("success\n");
}

void test_parse_array(void) {
  printf("testing json_parse array ... ");
  json_value_t *val = NULL;
  cutils_error_t err;

  err = json_parse("[ 1, \"two\", true, null ]", &val);
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_ARRAY);
  assert(val->as.array->length == 4);

  json_value_t *item = NULL;
  array_list_get(val->as.array, 0, (void **)&item);
  assert(item->type == JSON_NUMBER && item->as.number == 1);

  array_list_get(val->as.array, 1, (void **)&item);
  assert(item->type == JSON_STRING && strcmp(item->as.string, "two") == 0);

  array_list_get(val->as.array, 2, (void **)&item);
  assert(item->type == JSON_BOOLEAN && item->as.boolean == true);

  array_list_get(val->as.array, 3, (void **)&item);
  assert(item->type == JSON_NULL);

  json_value_free(val);

  printf("success\n");
}

void test_parse_object(void) {
  printf("testing json_parse object ... ");
  json_value_t *val = NULL;
  cutils_error_t err;

  const char *json_text =
      "{ \"a\": 1, \"b\": [true, false], \"c\": { \"d\": null } }";
  err = json_parse(json_text, &val);
  assert(err == CUTILS_SUCCESS);
  assert(val->type == JSON_OBJECT);
  assert(val->as.object->length == 3);

  json_value_t *item = NULL;
  hashmap_get(val->as.object, "a", (void **)&item);
  assert(item->type == JSON_NUMBER && item->as.number == 1);

  hashmap_get(val->as.object, "b", (void **)&item);
  assert(item->type == JSON_ARRAY && item->as.array->length == 2);

  hashmap_get(val->as.object, "c", (void **)&item);
  assert(item->type == JSON_OBJECT);

  json_value_t *nested_item = NULL;
  hashmap_get(item->as.object, "d", (void **)&nested_item);
  assert(nested_item->type == JSON_NULL);

  json_value_free(val);

  printf("success\n");
}

void test_parse_errors(void) {
  printf("testing json_parse errors ... ");
  json_value_t *val = NULL;
  cutils_error_t err;

  err = json_parse("[1, 2,", &val);
  assert(err == CUTILS_JSON_PARSE_ERROR);
  assert(val == NULL);

  err = json_parse("{ \"key\": }", &val);
  assert(err == CUTILS_JSON_PARSE_ERROR);
  assert(val == NULL);

  err = json_parse("tru", &val);
  assert(err == CUTILS_JSON_PARSE_ERROR);
  assert(val == NULL);

  err = json_parse("[1, 2] junk", &val);
  assert(err == CUTILS_JSON_PARSE_ERROR);
  assert(val == NULL);

  printf("success\n");
}

int main(void) {
  test_parse_literals();
  test_parse_numbers();
  test_parse_strings();
  test_parse_array();
  test_parse_object();
  test_parse_errors();
  return EXIT_SUCCESS;
}

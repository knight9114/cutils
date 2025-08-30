#ifndef __CUTILS_JSON_H__
#define __CUTILS_JSON_H__

#include "cutils/array_list.h"
#include "cutils/errors.h"
#include "cutils/hashmap.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum json_type {
  JSON_NULL,
  JSON_BOOLEAN,
  JSON_NUMBER,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT,
} json_type_t;

typedef struct json_value {
  json_type_t type;
  union {
    bool boolean;
    double number;
    char *string;
    array_list_t *array;
    hashmap_t *object;
  } as;
} json_value_t;

cutils_error_t json_parse(const char *text, json_value_t **value);
void json_value_free(void *ptr);

#endif // __CUTILS_JSON_H__

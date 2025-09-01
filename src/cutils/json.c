#include "cutils/json.h"
#include "cutils/array_list.h"
#include "cutils/errors.h"
#include "cutils/hashmap.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *text;
  size_t pos;
  cutils_error_t err;
} parser_t;

static json_value_t *_parse_value(parser_t *p);

static json_value_t *json_value_new(json_type_t type) {
  json_value_t *v = malloc(sizeof(json_value_t));
  if (!v) {
    return NULL;
  }
  v->type = type;
  return v;
}

void json_value_free(void *ptr) {
  if (!ptr) {
    return;
  }
  json_value_t *v = ptr;
  switch (v->type) {
  case JSON_STRING:
    free(v->value.string);
    break;
  case JSON_ARRAY:
    array_list_free(v->value.array);
    break;
  case JSON_OBJECT:
    hashmap_free(v->value.object);
    break;
  case JSON_NULL:
  case JSON_BOOLEAN:
  case JSON_NUMBER:
    // No-op
    break;
  }
  free(v);
}

static uint64_t _hash_string(void *key) {
  char *str = key;
  uint64_t hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return hash;
}

static bool _cmp_string(void *lhs, void *rhs) {
  return strcmp(lhs, rhs) == 0;
}

static void _skip_whitespace(parser_t *p) {
  while (isspace(p->text[p->pos])) {
    p->pos++;
  }
}

static char _peek(parser_t *p) { return p->text[p->pos]; }

static char _advance(parser_t *p) { return p->text[p->pos++]; }

static bool _match(parser_t *p, char expected) {
  _skip_whitespace(p);
  if (_peek(p) == expected) {
    p->pos++;
    return true;
  }
  return false;
}

static json_value_t *_parse_literal(parser_t *p, const char *literal,
                                     json_type_t type, bool boolean_value) {
  size_t len = strlen(literal);
  if (strncmp(&p->text[p->pos], literal, len) == 0) {
    p->pos += len;
    json_value_t *v = json_value_new(type);
    if (!v) {
      p->err = CUTILS_ALLOCATION_ERROR;
      return NULL;
    }
    if (type == JSON_BOOLEAN) {
      v->value.boolean = boolean_value;
    }
    return v;
  }
  p->err = CUTILS_JSON_PARSE_ERROR;
  return NULL;
}

static json_value_t *_parse_number(parser_t *p) {
  char *end;
  double num = strtod(&p->text[p->pos], &end);
  if ((const char *)end == &p->text[p->pos]) {
    p->err = CUTILS_JSON_PARSE_ERROR;
    return NULL;
  }
  p->pos = (const char *)end - p->text;

  json_value_t *v = json_value_new(JSON_NUMBER);
  if (!v) {
    p->err = CUTILS_ALLOCATION_ERROR;
    return NULL;
  }
  v->value.number = num;
  return v;
}

static unsigned int _parse_hex4(parser_t *p) {
  unsigned int val = 0;
  for (int i = 0; i < 4; i++) {
    char c = _advance(p);
    val <<= 4;
    if (c >= '0' && c <= '9') {
      val |= (c - '0');
    } else if (c >= 'a' && c <= 'f') {
      val |= (c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
      val |= (c - 'A' + 10);
    } else {
      p->err = CUTILS_JSON_PARSE_ERROR;
      return 0;
    }
  }
  return val;
}

static void _encode_utf8(char **buffer, unsigned int codepoint) {
  if (codepoint < 0x80) {
    *(*buffer)++ = codepoint;
  } else if (codepoint < 0x800) {
    *(*buffer)++ = 0xC0 | (codepoint >> 6);
    *(*buffer)++ = 0x80 | (codepoint & 0x3F);
  } else if (codepoint < 0x10000) {
    *(*buffer)++ = 0xE0 | (codepoint >> 12);
    *(*buffer)++ = 0x80 | ((codepoint >> 6) & 0x3F);
    *(*buffer)++ = 0x80 | (codepoint & 0x3F);
  } else {
    *(*buffer)++ = 0xF0 | (codepoint >> 18);
    *(*buffer)++ = 0x80 | ((codepoint >> 12) & 0x3F);
    *(*buffer)++ = 0x80 | ((codepoint >> 6) & 0x3F);
    *(*buffer)++ = 0x80 | (codepoint & 0x3F);
  }
}

static json_value_t *_parse_string(parser_t *p) {
  size_t start = p->pos;
  size_t len = 0;
  while (p->text[p->pos] != '"' && p->text[p->pos] != '\0') {
    if (p->text[p->pos] == '\\') {
      p->pos++; // Skip escape char
      if (p->text[p->pos] == 'u') {
        len += 4; // Max UTF-8 length for a codepoint
      } else {
        len++;
      }
    } else {
      len++;
    }
    p->pos++;
  }

  if (p->text[p->pos] == '\0') {
    p->err = CUTILS_JSON_PARSE_ERROR;
    return NULL;
  }

  char *str = malloc(len + 1);
  if (!str) {
    p->err = CUTILS_ALLOCATION_ERROR;
    return NULL;
  }
  char *str_ptr = str;

  p->pos = start;
  while (p->text[p->pos] != '"') {
    if (p->text[p->pos] == '\\') {
      p->pos++;
      switch (p->text[p->pos++]) {
      case '"':
        *str_ptr++ = '"';
        break;
      case '\\':
        *str_ptr++ = '\\';
        break;
      case '/':
        *str_ptr++ = '/';
        break;
      case 'b':
        *str_ptr++ = '\b';
        break;
      case 'f':
        *str_ptr++ = '\f';
        break;
      case 'n':
        *str_ptr++ = '\n';
        break;
      case 'r':
        *str_ptr++ = '\r';
        break;
      case 't':
        *str_ptr++ = '\t';
        break;
      case 'u': {
        unsigned int codepoint = _parse_hex4(p);
        if (p->err != CUTILS_SUCCESS) {
          free(str);
          return NULL;
        }
        _encode_utf8(&str_ptr, codepoint);
      } break;
      default:
        free(str);
        p->err = CUTILS_JSON_PARSE_ERROR;
        return NULL;
      }
    } else {
      *str_ptr++ = p->text[p->pos++];
    }
  }
  *str_ptr = '\0';
  p->pos++; // Consume closing quote

  json_value_t *v = json_value_new(JSON_STRING);
  if (!v) {
    p->err = CUTILS_ALLOCATION_ERROR;
    free(str);
    return NULL;
  }
  v->value.string = str;
  return v;
}

static json_value_t *_parse_array(parser_t *p) {
  array_list_t *arr = malloc(sizeof(array_list_t));
  if (!arr) {
    p->err = CUTILS_ALLOCATION_ERROR;
    return NULL;
  }
  array_list_init(arr, 8, json_value_free, NULL);

  if (_match(p, ']')) { // Empty array
    json_value_t *v = json_value_new(JSON_ARRAY);
    if (!v) {
      array_list_free(arr);
      p->err = CUTILS_ALLOCATION_ERROR;
      return NULL;
    }
    v->value.array = arr;
    return v;
  }

  do {
    json_value_t *elem = _parse_value(p);
    if (!elem) {
      array_list_free(arr);
      return NULL;
    }
    array_list_push(arr, elem);
  } while (_match(p, ','));

  if (!_match(p, ']')) {
    array_list_free(arr);
    p->err = CUTILS_JSON_PARSE_ERROR;
    return NULL;
  }

  json_value_t *v = json_value_new(JSON_ARRAY);
  if (!v) {
    array_list_free(arr);
    p->err = CUTILS_ALLOCATION_ERROR;
    return NULL;
  }
  v->value.array = arr;
  return v;
}

static json_value_t *_parse_object(parser_t *p) {
  hashmap_t *obj = malloc(sizeof(hashmap_t));
  if (!obj) {
    p->err = CUTILS_ALLOCATION_ERROR;
    return NULL;
  }
  hashmap_init(obj, 16, _hash_string, _cmp_string, free, json_value_free);

  if (_match(p, '}')) { // Empty object
    json_value_t *v = json_value_new(JSON_OBJECT);
    if (!v) {
      hashmap_free(obj);
      p->err = CUTILS_ALLOCATION_ERROR;
      return NULL;
    }
    v->value.object = obj;
    return v;
  }

  do {
    _skip_whitespace(p);
    if (_peek(p) != '"') {
      hashmap_free(obj);
      p->err = CUTILS_JSON_PARSE_ERROR;
      return NULL;
    }
    _advance(p); // Consume opening quote
    json_value_t *key_val = _parse_string(p);
    if (!key_val) {
      hashmap_free(obj);
      return NULL;
    }

    if (!_match(p, ':')) {
      json_value_free(key_val);
      hashmap_free(obj);
      p->err = CUTILS_JSON_PARSE_ERROR;
      return NULL;
    }

    json_value_t *val = _parse_value(p);
    if (!val) {
      json_value_free(key_val);
      hashmap_free(obj);
      return NULL;
    }

    hashmap_insert(obj, key_val->value.string, val);
    free(key_val); // Free the container, not the string itself
  } while (_match(p, ','));

  if (!_match(p, '}')) {
    hashmap_free(obj);
    p->err = CUTILS_JSON_PARSE_ERROR;
    return NULL;
  }

  json_value_t *v = json_value_new(JSON_OBJECT);
  if (!v) {
    hashmap_free(obj);
    p->err = CUTILS_ALLOCATION_ERROR;
    return NULL;
  }
  v->value.object = obj;
  return v;
}

static json_value_t *_parse_value(parser_t *p) {
  _skip_whitespace(p);
  char c = _peek(p);
  switch (c) {
  case 'n':
    return _parse_literal(p, "null", JSON_NULL, false);
  case 't':
    return _parse_literal(p, "true", JSON_BOOLEAN, true);
  case 'f':
    return _parse_literal(p, "false", JSON_BOOLEAN, false);
  case '"':
    _advance(p);
    return _parse_string(p);
  case '[':
    _advance(p);
    return _parse_array(p);
  case '{':
    _advance(p);
    return _parse_object(p);
  default:
    if (c == '-' || isdigit(c)) {
      return _parse_number(p);
    }
    p->err = CUTILS_JSON_PARSE_ERROR;
    return NULL;
  }
}

cutils_error_t json_parse(const char *text, json_value_t **value) {
  if (!text || !value) {
    return CUTILS_NULL_ERROR;
  }

  parser_t p = {text, 0, CUTILS_SUCCESS};
  *value = _parse_value(&p);

  if (p.err != CUTILS_SUCCESS) {
    if (*value) {
      json_value_free(*value);
      *value = NULL;
    }
    return p.err;
  }

  _skip_whitespace(&p);
  if (_peek(&p) != '\0') {
    json_value_free(*value);
    *value = NULL;
    return CUTILS_JSON_PARSE_ERROR;
  }

  return CUTILS_SUCCESS;
}

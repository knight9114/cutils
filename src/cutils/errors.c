#include "cutils/errors.h"
#include <stddef.h>

const char *cutils_error_message(cutils_error_t err) {
  switch (err) {
  case CUTILS_SUCCESS:
    return "Success";
  case CUTILS_NULL_ERROR:
    return "Null pointer error";
  case CUTILS_ALLOCATION_ERROR:
    return "Memory allocation error";
  case CUTILS_INDEX_ERROR:
    return "Index out of bounds error";
  case CUTILS_RESIZE_ERROR:
    return "Resize error";
  case CUTILS_JSON_PARSE_ERROR:
    return "JSON parsing error";
  default:
    return "Unknown error";
  }
}

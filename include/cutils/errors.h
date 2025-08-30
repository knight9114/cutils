#ifndef __CUTILS_ERRORS_H__
#define __CUTILS_ERRORS_H__

typedef enum cutils_error {
  CUTILS_SUCCESS = 0,
  CUTILS_NULL_ERROR,
  CUTILS_ALLOCATION_ERROR,
  CUTILS_INDEX_ERROR,
  CUTILS_RESIZE_ERROR,
  CUTILS_JSON_PARSE_ERROR,
} cutils_error_t;

const char *cutils_error_message(cutils_error_t err);

#endif // __CUTILS_ERRORS_H__

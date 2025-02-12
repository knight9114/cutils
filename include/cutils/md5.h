#ifndef __CUTILS_MD5_H__
#define __CUTILS_MD5_H__

#include "cutils/errors.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  size_t length;
  uint32_t buffer[4];
  uint8_t input[64];
} md5_context_t;

cutils_error_t md5_digest(size_t n, const uint8_t data[n], uint8_t hash[16]);

#endif // __CUTILS_MD5_H__

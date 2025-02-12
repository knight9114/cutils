#include "cutils/errors.h"
#include "cutils/md5.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_md5_digest(void) {
  printf("testing md5_digest ... ");

  const char x1[] = "The quick brown fox jumps over the lazy dog";
  uint8_t y1[16] = {0x9e, 0x10, 0x7d, 0x9d, 0x37, 0x2b, 0xb6, 0x82,
                    0x6b, 0xd8, 0x1d, 0x35, 0x42, 0xa4, 0x19, 0xd6};
  uint8_t y1_hat[16] = {0};
  cutils_error_t err = md5_digest(strlen(x1), (uint8_t *)x1, y1_hat);
  assert(err == CUTILS_SUCCESS);
  assert(memcmp(y1_hat, y1, 16) == 0);

  const char x2[] = "";
  uint8_t y2[16] = {0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04,
                    0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e};
  uint8_t y2_hat[16] = {0};
  err = md5_digest(strlen(x2), (uint8_t *)x2, y2_hat);
  assert(err == CUTILS_SUCCESS);
  assert(memcmp(y2_hat, y2, 16) == 0);

  printf("success\n");
}

int main(void) {
  test_md5_digest();
  return EXIT_SUCCESS;
}

#include "cutils/md5.h"
#include "cutils/errors.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t S[64] = {7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 7,
                         12, 17, 22, 5,  9,  14, 20, 5,  9,  14, 20, 5,  9,
                         14, 20, 5,  9,  14, 20, 4,  11, 16, 23, 4,  11, 16,
                         23, 4,  11, 16, 23, 4,  11, 16, 23, 6,  10, 15, 21,
                         6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21};

static uint32_t K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
    0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
    0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
    0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
    0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

static uint8_t PADDING[64] = {
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static uint32_t F(uint32_t x, uint32_t y, uint32_t z) {
  return ((x & y) | (~x & z));
}

static uint32_t G(uint32_t x, uint32_t y, uint32_t z) {
  return ((x & z) | (y & ~z));
}

static uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return (x ^ y ^ z); }

static uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return (y ^ (x | ~z)); }

static void convert_u8_array_to_u32_array(uint8_t input[64],
                                          uint32_t output[16]) {
  for (size_t i = 0; i < 16; i++) {
    output[i] = (input[i * 4 + 3] << 24) | (input[i * 4 + 2] << 16) |
                (input[i * 4 + 1] << 8) | (input[i * 4]);
  }
}

static uint32_t left_rotate(uint32_t x, uint8_t n) {
  return (x << n) | (x >> (32 - n));
}

static cutils_error_t md5_init(md5_context_t *c) {
  if (!c) {
    return CUTILS_NULL_ERROR;
  }

  c->length = 0;
  c->buffer[0] = 0x67452301;
  c->buffer[1] = 0xefcdab89;
  c->buffer[2] = 0x98badcfe;
  c->buffer[3] = 0x10325476;

  return CUTILS_SUCCESS;
}

static void md5_free(void *ptr) {
  if (ptr) {
    md5_context_t *c = ptr;
    free(c);
  }
}

static cutils_error_t md5_step(md5_context_t *c, uint32_t input[16]) {
  if (!c) {
    return CUTILS_NULL_ERROR;
  }

  uint32_t A = c->buffer[0];
  uint32_t B = c->buffer[1];
  uint32_t C = c->buffer[2];
  uint32_t D = c->buffer[3];

  for (size_t i = 0; i < 64; i++) {
    uint32_t j = 0;
    uint32_t E = 0;

    switch (i / 16) {
    case 0:
      E = F(B, C, D);
      j = i;
      break;
    case 1:
      E = G(B, C, D);
      j = ((i * 5) + 1) % 16;
      break;
    case 2:
      E = H(B, C, D);
      j = ((i * 3) + 5) % 16;
      break;
    case 3:
      E = I(B, C, D);
      j = (i * 7) % 16;
      break;
    }

    uint32_t tmp = D;
    D = C;
    C = B;
    B = B + left_rotate(A + E + K[i] + input[j], S[i]);
    A = tmp;
  }

  c->buffer[0] += A;
  c->buffer[1] += B;
  c->buffer[2] += C;
  c->buffer[3] += D;

  return CUTILS_SUCCESS;
}

cutils_error_t md5_digest(size_t n, const uint8_t data[n], uint8_t hash[16]) {
  md5_context_t *ctx = malloc(sizeof(md5_context_t));
  if (!ctx) {
    return CUTILS_ALLOCATION_ERROR;
  }

  cutils_error_t err = md5_init(ctx);
  if (err != CUTILS_SUCCESS) {
    return err;
  }

  size_t shift = n % 64;
  size_t padding = shift < 56 ? 56 - shift : (56 + 64) - shift;
  size_t offset = 0;
  uint32_t input[16] = {0};
  for (size_t i = 0; i < n + padding; i++) {
    uint8_t value = 0;
    if (i < n) {
      value = data[i];
    } else {
      value = PADDING[i - n];
    }
    ctx->input[offset++] = value;

    if (offset % 64 == 0) {
      convert_u8_array_to_u32_array(ctx->input, input);
      if ((err = md5_step(ctx, input)) != CUTILS_SUCCESS) {
        return err;
      }

      offset = 0;
    }
  }

  convert_u8_array_to_u32_array(ctx->input, input);
  input[14] = (uint32_t)(n * 8);
  input[15] = (uint32_t)((n * 8) >> 32);
  if ((err = md5_step(ctx, input)) != CUTILS_SUCCESS) {
    return err;
  };

  for (size_t i = 0; i < 4; i++) {
    hash[i * 4 + 0] = ctx->buffer[i] & 0xff;
    hash[i * 4 + 1] = (ctx->buffer[i] >> 8) & 0xff;
    hash[i * 4 + 2] = (ctx->buffer[i] >> 16) & 0xff;
    hash[i * 4 + 3] = (ctx->buffer[i] >> 24) & 0xff;
  }

  md5_free(ctx);

  return CUTILS_SUCCESS;
}

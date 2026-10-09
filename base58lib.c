#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

const char ALPHABET[58] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

uint64_t decode_remainder(char c, bool ignore_garbage) {
  switch (c) {
  case '1':
    return 0;
  case '2':
    return 1;
  case '3':
    return 2;
  case '4':
    return 3;
  case '5':
    return 4;
  case '6':
    return 5;
  case '7':
    return 6;
  case '8':
    return 7;
  case '9':
    return 8;
  case 'A':
    return 9;
  case 'B':
    return 10;
  case 'C':
    return 11;
  case 'D':
    return 12;
  case 'E':
    return 13;
  case 'F':
    return 14;
  case 'G':
    return 15;
  case 'H':
    return 16;
  case 'J':
    return 17;
  case 'K':
    return 18;
  case 'L':
    return 19;
  case 'M':
    return 20;
  case 'N':
    return 21;
  case 'P':
    return 22;
  case 'Q':
    return 23;
  case 'R':
    return 24;
  case 'S':
    return 25;
  case 'T':
    return 26;
  case 'U':
    return 27;
  case 'V':
    return 28;
  case 'W':
    return 29;
  case 'X':
    return 30;
  case 'Y':
    return 31;
  case 'Z':
    return 32;
  case 'a':
    return 33;
  case 'b':
    return 34;
  case 'c':
    return 35;
  case 'd':
    return 36;
  case 'e':
    return 37;
  case 'f':
    return 38;
  case 'g':
    return 39;
  case 'h':
    return 40;
  case 'i':
    return 41;
  case 'j':
    return 42;
  case 'k':
    return 43;
  case 'm':
    return 44;
  case 'n':
    return 45;
  case 'o':
    return 46;
  case 'p':
    return 47;
  case 'q':
    return 48;
  case 'r':
    return 49;
  case 's':
    return 50;
  case 't':
    return 51;
  case 'u':
    return 52;
  case 'v':
    return 53;
  case 'w':
    return 54;
  case 'x':
    return 55;
  case 'y':
    return 56;
  case 'z':
    return 57;
  default:
    if (ignore_garbage)
      return 58;
    else {
      fprintf(stderr, "Invalid base58 character: %c\n", c);
      exit(1);
    }
    break;
  }
}

void bigint_from_bytes(struct vector64 *out, uint8_t *bytes, size_t len) {
  uint64_t st = 0;
  size_t rem = len % 8;
  size_t i = 0;

  if (rem) {
    for (; i < rem; i++) {
      st = (st << 8) | (uint64_t)bytes[i];
    }
    vector64_push(out, &st, 1);
    st = 0;
  }

  for (; i < len; i++) {
    st = (st << 8) | (uint64_t)bytes[i];
    if (((i - rem) % 8) == 7) {
      vector64_push(out, &st, 1);
      st = 0;
    }
  }
}

uint64_t bigint_divide(struct vector64 *bigint, uint64_t d) {
  if (d == 0) {
    return 0;
  }

  struct vector64 q;
  init_vector64_cap(&q, bigint->len);

  __uint128_t rem = 0;

  for (size_t i = 0; i < bigint->len; i++) {
    __uint128_t cur = (rem << 64) | bigint->buf[i];
    uint64_t quot = (uint64_t)(cur / d);
    rem = cur % d;
    vector64_push(&q, &quot, 1);
  }

  // Trim leading zero limbs in quotient
  size_t start = 0;
  while (start < q.len && q.buf[start] == 0) {
    start++;
  }

  if (start == q.len) {
    q.len = 0;
  } else if (start > 0) {
    for (size_t i = start; i < q.len; i++) {
      q.buf[i - start] = q.buf[i];
    }
    q.len -= start;
  }

  free(bigint->buf);
  bigint->buf = q.buf;
  bigint->cap = q.cap;
  bigint->len = q.len;

  return (uint64_t)rem;
}

int64_t base58_encode(uint8_t **output, uint8_t *input, size_t input_len) {
  struct vector out_vec;
  init_vector(&out_vec);

  size_t leading_zeros = 0;
  for (size_t i = 0; i < input_len && input[i] == 0; i++) {
    leading_zeros++;
  }

  struct vector64 big_int;
  init_vector64(&big_int);
  bigint_from_bytes(&big_int, input, input_len);

  // struct vector64 *big_int_ptr = &big_int;
  while (big_int.len != 0) {
    uint64_t r = bigint_divide(&big_int, 58);
    char c = ALPHABET[r];
    vector_push(&out_vec, (uint8_t *)&c, 1);
  }
  destroy_vector64(&big_int);

  for (register int i = 0; i < leading_zeros; i++) {
    vector_push(&out_vec, (uint8_t *)"1", 1);
  }
  vector_reverse(&out_vec);
  *output = out_vec.buf;
  // DO NOT DESTROY out_vec. THE BUF INSIDE OF IT ESCAPES THIS FUNCTION'S SCOPE.
  // THIS IS INTENDED BEHAVIOR. (SCREAMS IN RUST)
  return out_vec.len;
}

void bigint_add_multiply(struct vector64 *v, uint64_t m, uint64_t r) {
  __uint128_t carry = (__uint128_t)r;

  for (size_t i = 0; i < v->len; i++) {
    __uint128_t t = (__uint128_t)v->buf[i] * m + carry;
    v->buf[i] = (uint64_t)t;
    carry = t >> 64;
  }

  if (carry) {
    uint64_t c = (uint64_t)carry;
    vector64_push(v, &c, 1);
  }
}

void bytes_from_bigint(struct vector *bytes, struct vector64 *bigint) {
  if (bigint->len == 0)
    return;
  for (size_t i = 0; i < bigint->len; i++) {
    uint64_t p = bigint->buf[i];
    for (register int j = 7; j >= 0; j--) {
      uint8_t b = (p >> (j * 8)) & 0xff;
      if (i == 0 && b == 0)
        continue;
      vector_push(bytes, &b, 1);
    }
  }
}

int64_t base58_decode(uint8_t **output, uint8_t *input, size_t input_len,
                      bool ignore_garbage) {
  struct vector out_vec;
  init_vector(&out_vec);

  for (size_t leading_ones = 0, max = input_len;
       leading_ones < max && input[leading_ones++] == '1'; input_len--) {
    input = input + leading_ones;
  }

  struct vector64 big_int;
  init_vector64(&big_int);
  uint64_t zero = 0;
  vector64_push(&big_int, &zero, 1);

  for (size_t i = 0; i < input_len; i++) {
    char c = input[i];
    if (c == '\n')
      continue;
    uint64_t r = decode_remainder(c, ignore_garbage);
    if (r == 58)
      continue;
    bigint_add_multiply(&big_int, 58, r);
  }
  vector64_reverse(&big_int);

  bytes_from_bigint(&out_vec, &big_int);

  *output = out_vec.buf;
  // DO NOT DESTROY out_vec. THE BUF INSIDE OF IT ESCAPES THIS FUNCTION'S SCOPE.
  // THIS IS INTENDED BEHAVIOR. (SCREAMS IN RUST)
  return out_vec.len;
}

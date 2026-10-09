#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

int init_vector64(struct vector64 *vec) {
  vec->len = 0;
  vec->cap = VECTOR_INIT_CAP;
  vec->buf = (uint64_t *)malloc(8 * VECTOR_INIT_CAP);
  return vec->buf == NULL;
}

int init_vector64_cap(struct vector64 *vec, size_t cap) {
  vec->len = 0;
  vec->cap = cap;
  vec->buf = (uint64_t *)malloc(8 * cap);
  return vec->buf == NULL;
}

int vector64_grow(struct vector64 *vec) {
  vec->cap *= 2;
  vec->buf = (uint64_t *)realloc((void *)vec->buf, 8 * vec->cap);
  if (vec->buf == NULL) {
    fprintf(stderr,
            "PANIC: system possibly ran out of memory (errno %d). aborting.\n",
            errno);
    exit(1);
  }
  return 1; // always return 1 to ensure the do{}while; pattern works.
}

int vector64_push(struct vector64 *vec, uint64_t *buf, size_t count) {
  // TODO: error propagation
  if (count == 0)
    return 0;
  size_t new_len;
  do {
    new_len = vec->len + count;
  } while (new_len > vec->cap && vector64_grow(vec));
  memcpy(vec->buf + vec->len, buf, 8 * count);
  vec->len += count;
  return 0;
}

void destroy_vector64(struct vector64 *vec) {
  vec->len = 0;
  vec->cap = 0;
  free(vec->buf);
}

int vector64_copy(struct vector64 *dst, struct vector64 *src) {
  if (dst->cap < src->cap) {
    return 1;
  }
  for (register int i = 0; i < src->len; i++) {
    dst->buf[i] = src->buf[i];
  }
  dst->len = src->len;
  return 0;
}

int vector64_reverse(struct vector64 *vec) {
  struct vector64 copy;
  init_vector64_cap(&copy, vec->cap);
  int result = vector64_copy(&copy, vec);
  if (result) {
    destroy_vector64(&copy);
    return result;
  }
  for (register int di = 0, si = vec->len - 1; di < vec->len; di++, si--) {
    vec->buf[di] = copy.buf[si];
  }
  destroy_vector64(&copy);
  return 0;
}

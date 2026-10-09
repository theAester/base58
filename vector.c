#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

int init_vector(struct vector *vec) {
  vec->len = 0;
  vec->cap = VECTOR_INIT_CAP;
  vec->buf = (uint8_t *)malloc(VECTOR_INIT_CAP);
  return vec->buf == NULL;
}

int init_vector_cap(struct vector *vec, size_t cap) {
  vec->len = 0;
  vec->cap = cap;
  vec->buf = (uint8_t *)malloc(cap);
  return vec->buf == NULL;
}

int vector_grow(struct vector *vec) {
  vec->cap *= 2;
  vec->buf = (uint8_t *)realloc((void *)vec->buf, vec->cap);
  if (vec->buf == NULL) {
    fprintf(stderr,
            "PANIC: system possibly ran out of memory (errno %d). aborting.\n",
            errno);
    exit(1);
  }
  return 1; // always return 1 to ensure the do{}while; pattern works.
}

int vector_push(struct vector *vec, uint8_t *buf, size_t count) {
  // TODO: error propagation
  if (count == 0)
    return 0;
  size_t new_len;
  do {
    new_len = vec->len + count;
  } while (new_len > vec->cap && vector_grow(vec));
  memcpy(vec->buf + vec->len, buf, count);
  vec->len += count;
  return 0;
}

void destroy_vector(struct vector *vec) {
  vec->len = 0;
  vec->cap = 0;
  free(vec->buf);
}

int vector_copy(struct vector *dst, struct vector *src) {
  if (dst->cap < src->cap) {
    return 1;
  }
  for (register size_t i = 0; i < src->len; i++) {
    dst->buf[i] = src->buf[i];
  }
  dst->len = src->len;
  return 0;
}

int vector_reverse(struct vector *vec) {
  struct vector copy;
  init_vector_cap(&copy, vec->cap);
  int result = vector_copy(&copy, vec);
  if (result) {
    destroy_vector(&copy);
    return result;
  }
  for (register size_t di = 0, si = vec->len - 1; di < vec->len; di++, si--) {
    vec->buf[di] = copy.buf[si];
  }
  destroy_vector(&copy);
  return 0;
}

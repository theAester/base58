#ifndef __VECTOR_H_
#define __VECTOR_H_

struct vector {
  uint8_t *buf;
  size_t len;
  size_t cap;
};

struct vector64 {
  uint64_t *buf;
  size_t len;
  size_t cap;
};

#define VECTOR_INIT_CAP 512

int init_vector(struct vector *vec);
int init_vector_cap(struct vector *vec, size_t cap);
void destroy_vector(struct vector *vec);
int vector_push(struct vector *vec, uint8_t *buf, size_t count);
int vector_reverse(struct vector *vec);

int init_vector64(struct vector64 *vec);
int init_vector64_cap(struct vector64 *vec, size_t cap);
void destroy_vector64(struct vector64 *vec);
int vector64_push(struct vector64 *vec, uint64_t *buf, size_t count);
int vector64_reverse(struct vector64 *vec);

#endif

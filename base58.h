#ifndef __BASE58_H_
#define __BASE58_H_

int64_t base58_encode(uint8_t **output, uint8_t *input, size_t input_len);
int64_t base58_decode(uint8_t **output, uint8_t *input, size_t input_len,
                      bool ignore_garbage);
void base58_checksum(uint8_t *buf, uint8_t *data, size_t data_len);
bool base58_checksum_validate(uint8_t *buf, uint8_t *data, size_t data_len);

#endif

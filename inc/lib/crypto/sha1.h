#ifndef LIB_CRYPTO_SHA1_H
#define LIB_CRYPTO_SHA1_H

#include <stdint.h>
typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1_Transform(uint32_t state[5], const uint8_t buffer[64]);
void SHA1_Init(SHA1_CTX *context);
void SHA1_Update(SHA1_CTX *context, const uint8_t *data, uint32_t len);
void SHA1_Final(uint8_t digest[20], SHA1_CTX *context);
uint32_t SHA1(uint8_t *str, uint32_t len, uint8_t *hash_out);

#endif

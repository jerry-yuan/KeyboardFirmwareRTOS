#ifndef LIB_CRYPTO_BASE32_H
#define LIB_CRYPTO_BASE32_H

#include <stdint.h>

uint32_t BASE32_decode(uint8_t* pcEncoded,uint32_t uiEncodedLength,uint8_t* pcDecoded);
uint8_t BASE32_validate(uint8_t* pcEncoded,uint32_t uiEncodedLength);
#endif

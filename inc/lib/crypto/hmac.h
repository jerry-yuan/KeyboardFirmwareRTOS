#ifndef LIB_CRYPTO_HMAC_H
#define LIB_CRYPTO_HMAC_H

#include <stdint.h>

typedef uint32_t(*HASH_FUNC)(uint8_t* pcMessage,uint32_t uiMessageLength,uint8_t* pcDigest);

void HMAC(HASH_FUNC pfnHash,uint8_t* pcKey,uint32_t uiKeyLength,uint8_t* pcData,uint32_t uiDataLength, uint8_t* pcDigest);

#endif

#include <lib/crypto/hmac.h>

#define debug_out(NAME,BUF,LEN) printf("%s:\n",NAME);\
								dumpMemory(BUF,LEN)
#define MAX_MESSAGE_LENGTH 1000
void HMAC(HASH_FUNC pfnHash,uint8_t* pcKey,uint32_t uiKeyLength,uint8_t* pcData,uint32_t uiDataLength, uint8_t* pcDigest){
    int b = 64; /* blocksize */
    uint8_t ipad = 0x36;

    uint8_t opad = 0x5c;

    uint8_t k0[64];
    uint8_t k0xorIpad[64];
    uint8_t step7data[64];
    uint8_t step5data[MAX_MESSAGE_LENGTH + 128];
    uint8_t step8data[64 + 20];
    int i;
	uint32_t uiHashDigestLength;
    for (i = 0; i < 64; i++) {
        k0[i] = 0x00;
    }


    if (uiKeyLength != b){    /* Step 1 */
        /* Step 2 */
        if (uiKeyLength > b) {
            uiHashDigestLength=pfnHash(pcKey, uiKeyLength, pcDigest);
            for (i = 0; i < uiHashDigestLength; i++) {
                k0[i] = pcDigest[i];
            }
        } else if (uiKeyLength < b)  /* Step 3 */
        {
            for (i = 0; i < uiKeyLength; i++) {
                k0[i] = pcKey[i];
            }
        }
    } else {
        for (i = 0; i < b; i++) {
            k0[i] = pcKey[i];
        }
    }
#ifdef HMAC_DEBUG
    debug_out("k0", k0, 64);
#endif
    /* Step 4 */
    for (i = 0; i < 64; i++) {
        k0xorIpad[i] = k0[i] ^ ipad;
    }
#ifdef HMAC_DEBUG
    debug_out("k0 xor ipad", k0xorIpad, 64);
#endif
    /* Step 5 */
    for (i = 0; i < 64; i++) {
        step5data[i] = k0xorIpad[i];
    }
    for (i = 0; i < uiDataLength; i++) {
        step5data[i + 64] = pcData[i];
    }
#ifdef HMAC_DEBUG
    debug_out("(k0 xor ipad) || text", step5data, uiDataLength + 64);
#endif

    /* Step 6 */
    uiHashDigestLength=pfnHash(step5data, uiDataLength + b, pcDigest);

#ifdef HMAC_DEBUG
    debug_out("Hash((k0 xor ipad) || text)", pcDigest, uiHashDigestLength);
#endif

    /* Step 7 */
    for (i = 0; i < 64; i++) {
        step7data[i] = k0[i] ^ opad;
    }

#ifdef HMAC_DEBUG
    debug_out("(k0 xor opad)", step7data, 64);
#endif

    /* Step 8 */
    for (i = 0; i < 64; i++) {
        step8data[i] = step7data[i];
    }
    for (i = 0; i < uiHashDigestLength; i++) {
        step8data[i + 64] = pcDigest[i];
    }

#ifdef HMAC_DEBUG
    debug_out("(k0 xor opad) || Hash((k0 xor ipad) || text)", step8data, uiHashDigestLength + 64);
#endif

    /* Step 9 */
    uiHashDigestLength=pfnHash(step8data, b + uiHashDigestLength, pcDigest);

#ifdef HMAC_DEBUG
    debug_out("HASH((k0 xor opad) || Hash((k0 xor ipad) || text))", pcDigest, uiHashDigestLength);
#endif
}

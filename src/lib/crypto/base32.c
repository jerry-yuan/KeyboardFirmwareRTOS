#include <lib/crypto/base32.h>

static const uint8_t base32_values[265]={
    //    This map cheats and interprets:
    //       - the numeral zero as the letter "O" as in oscar
    //       - the numeral one as the letter "L" as in lima
    //       - the numeral eight as the letter "B" as in bravo
    // 00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F
	   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0x00
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0x10
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0x20
       14, 11, 26, 27, 28, 29, 30, 31,  1, 32, 32, 32, 32,  0, 32, 32, // 0x30
       32,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 0x40
       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 32, 32, 32, 32, 32, // 0x50
       32,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 0x60
       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 32, 32, 32, 32, // 0x70
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0x80
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0x90
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0xA0
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0xB0
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0xC0
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0xD0
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0xE0
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, // 0xF0
};

uint32_t BASE32_decode(uint8_t *pcEncoded, uint32_t uiEncodedLength, uint8_t *pcDecoded) {
    uint32_t uiResultLength;
    uint32_t uiPosition;
    // decodes base32 secret key
    uiResultLength = 0;
    for (uiPosition = 0; uiPosition <= (uiEncodedLength - 8); uiPosition += 8) {
        // MSB is Most Significant Bits  (0x80 == 10000000 ~= MSB)
        // MB is middle bits             (0x7E == 01111110 ~= MB)
        // LSB is Least Significant Bits (0x01 == 00000001 ~= LSB)

        // byte 0
        pcDecoded[uiResultLength+0]  = (base32_values[pcEncoded[uiPosition+0]] << 3) & 0xF8; // 5 MSB
        pcDecoded[uiResultLength+0] |= (base32_values[pcEncoded[uiPosition+1]] >> 2) & 0x07; // 3 LSB
        if (pcEncoded[uiPosition+2] == '=') {
            uiResultLength += 1;
            break;
        }

        // byte 1
        pcDecoded[uiResultLength+1]  = (base32_values[pcEncoded[uiPosition+1]] << 6) & 0xC0; // 2 MSB
        pcDecoded[uiResultLength+1] |= (base32_values[pcEncoded[uiPosition+2]] << 1) & 0x3E; // 5  MB
        pcDecoded[uiResultLength+1] |= (base32_values[pcEncoded[uiPosition+3]] >> 4) & 0x01; // 1 LSB
        if (pcEncoded[uiPosition+4] == '=') {
            uiResultLength += 2;
            break;
        }

        // byte 2
        pcDecoded[uiResultLength+2]  = (base32_values[pcEncoded[uiPosition+3]] << 4) & 0xF0; // 4 MSB
        pcDecoded[uiResultLength+2] |= (base32_values[pcEncoded[uiPosition+4]] >> 1) & 0x0F; // 4 LSB
        if (pcEncoded[uiPosition+5] == '=') {
            uiResultLength += 3;
            break;
        }

        // byte 3
        pcDecoded[uiResultLength+3]  = (base32_values[pcEncoded[uiPosition+4]] << 7) & 0x80; // 1 MSB
        pcDecoded[uiResultLength+3] |= (base32_values[pcEncoded[uiPosition+5]] << 2) & 0x7C; // 5  MB
        pcDecoded[uiResultLength+3] |= (base32_values[pcEncoded[uiPosition+6]] >> 3) & 0x03; // 2 LSB
        if (pcEncoded[uiPosition+7] == '=') {
            uiResultLength += 4;
            break;
        }

        // byte 4
        pcDecoded[uiResultLength+4]  = (base32_values[pcEncoded[uiPosition+6]] << 5) & 0xE0; // 3 MSB
        pcDecoded[uiResultLength+4] |= (base32_values[pcEncoded[uiPosition+7]] >> 0) & 0x1F; // 5 LSB
        uiResultLength += 5;
    }
    pcDecoded[uiResultLength] = 0;

    return uiResultLength;
}

uint8_t BASE32_validate(uint8_t *pcEncoded, uint32_t uiEncodedLength) {
    uint32_t uiPosition;
// validates base32 key
    if (((uiEncodedLength & 0xF) != 0) && ((uiEncodedLength & 0xF) != 8)) {
        return 1;
    }
    for (uiPosition = 0; (uiPosition < uiEncodedLength); uiPosition++) {
        if (base32_values[pcEncoded[uiPosition]] == 32)
            return 1;
        if (pcEncoded[uiPosition] == '=') {
            if (((uiPosition & 0xF) == 0) || ((uiPosition & 0xF) == 8))
                return(1);
            if ((uiEncodedLength - uiPosition) > 6)
                return 1;
            switch (uiPosition % 8) {
                case 2:
                case 4:
                case 5:
                case 7:
                    break;
                default:
                    return 1;
            }
            for ( ; (uiPosition < uiEncodedLength); uiPosition++) {
                if (pcEncoded[uiPosition] != '=')
                    return 1;
            }
        }
    }
    return 0;
}

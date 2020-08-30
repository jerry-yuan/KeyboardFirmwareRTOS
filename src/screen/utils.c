#include <screen/utils.h>

bool ContainsKey(MappedKeyCodes_t* mappedKeyCodes,uint8_t keyCode) {
    for(int i=0; i<mappedKeyCodes->length; i++) {
        if((uint8_t)(mappedKeyCodes->keyCodes[i] & 0xFF) == keyCode) {
            return true;
        }
    }
    return false;
}

#include <string.h>

#include "PCMouseDevice.h"

// 0x004c48c0
PCMouseDevice::PCMouseDevice() {
    memset(field_0x2d8, 0, sizeof(field_0x2d8));
}

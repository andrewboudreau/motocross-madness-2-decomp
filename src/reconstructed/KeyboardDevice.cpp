#include "KeyboardDevice.h"

// 0x00489e30
KeyboardDevice::KeyboardDevice() : PCInputDevice(0) {
    unsigned int stamp = UnknownFunction4bfa80();
    field_0x08 = 256;
    for (int i = 0; i < 256; i++) {
        field_0x260[i].state = 0;
        field_0x260[i].field_0x04 = stamp;
        field_0x260[i].field_0x08 = stamp;
        field_0x260[i].field_0x0c = stamp;
        field_0x260[i].field_0x10 = stamp;
    }
    for (int j = 0; j < 6; j++)
        field_0x1660[j].Init(1, 1);
    field_0x16d8 = 0;
}

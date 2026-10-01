#include "MouseDevice.h"

// 0x0048a2d0
MouseDevice::MouseDevice() : PCInputDevice(1) {
    for (int j = 0; j < 2; j++)
        field_0x2b0[j].Init(1, 1);
    unsigned int stamp = UnknownFunction4bfa80();
    for (int i = 0; i < 4; i++) {
        field_0x260[i].state = 0;
        field_0x260[i].field_0x04 = stamp;
        field_0x260[i].field_0x08 = stamp;
        field_0x260[i].field_0x0c = stamp;
        field_0x260[i].field_0x10 = stamp;
    }
}

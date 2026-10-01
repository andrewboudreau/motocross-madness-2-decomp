#include <string.h>

#include "PCInputDevice.h"

// 0x004c25f0
PCInputDevice::PCInputDevice(int id) : InputDevice(id) {
    field_0x25c = 0;
    memset(field_0x18, 0, sizeof(field_0x18));
}

// 0x004c2650: method 8, then a release of the device.
PCInputDevice::~PCInputDevice() {
    if (field_0x25c) {
        field_0x25c->UnknownMethod8();
        if (field_0x25c) {
            field_0x25c->UnknownMethod2();
            field_0x25c = 0;
        }
    }
}

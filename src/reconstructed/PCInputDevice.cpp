#include <string.h>

#include "PCInputDevice.h"

// 0x004c25f0
PCInputDevice::PCInputDevice(int id) : InputDevice(id) {
    field_0x25c = 0;
    memset(&field_0x18, 0, sizeof(field_0x18));
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

// 0x004c26d0
int PCInputDevice::UnknownMethod4c26d0(int acquire) {
    if (field_0x25c) {
        if (acquire) {
            if (field_0x25c->UnknownMethod7() >= 0)
                return 1;
        } else {
            if (field_0x25c->UnknownMethod8() >= 0)
                return 1;
        }
    }
    return 0;
}

// 0x004c2710
int PCInputDevice::UnknownMethod4c2710(int property, unsigned long object, unsigned long how,
                                       unsigned long data) {
    if (!field_0x25c)
        return 0;
    UnknownInputProperty value;
    value.size = sizeof(value);
    value.headerSize = 0x10;
    value.object = object;
    value.how = how;
    value.data = data;
    return field_0x25c->UnknownMethod6(property, &value) >= 0;
}

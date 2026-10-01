#include "InputDevice.h"

// 0x00489780
InputDevice::InputDevice(int id) {
    field_0x14_bits = 0;
    field_0x10 = -1;
    field_0x04 = 0;
    field_0x08 = 0;
    field_0x0c = id;
}

// 0x004897d0
InputDevice::~InputDevice() {}

// 0x004897e0
int InputDevice::UnknownFunction4897e0(int value) {
    return field_0x10 == value;
}

#include "InputDevice.h"

// 0x00489780
InputDevice::InputDevice(int id) {
    field_0x14_axis0 = 0;
    field_0x14_axis1 = 0;
    field_0x14_axis2 = 0;
    field_0x14_axis3 = 0;
    field_0x14_axis4 = 0;
    field_0x14_axis5 = 0;
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

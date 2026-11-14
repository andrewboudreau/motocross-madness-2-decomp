#include "InputDevice.h"

// 0x00489780
InputDevice::InputDevice(int id) {
    hasXAxis = 0;
    hasYAxis = 0;
    hasZAxis = 0;
    hasRxAxis = 0;
    hasRyAxis = 0;
    hasRzAxis = 0;
    deviceSubtype = -1;
    axisCount = 0;
    buttonCount = 0;
    deviceKind = id;
}

// 0x004897d0
InputDevice::~InputDevice() {}

// 0x004897e0
int InputDevice::IsDeviceSubtype(int value) {
    return deviceSubtype == value;
}

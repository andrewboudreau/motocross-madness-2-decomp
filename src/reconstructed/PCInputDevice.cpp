#include <string.h>

#include "PCInputDevice.h"

// 0x004c25f0
PCInputDevice::PCInputDevice(int id) : InputDevice(id) {
    device = 0;
    memset(&deviceInfo, 0, sizeof(deviceInfo));
}

// 0x004c2650: method 8, then a release of the device.
PCInputDevice::~PCInputDevice() {
    if (device) {
        device->Unacquire();
        if (device) {
            device->Release();
            device = 0;
        }
    }
}

// 0x004c26d0
int PCInputDevice::SetAcquired(int acquire) {
    if (device) {
        if (acquire) {
            if (device->Acquire() >= 0)
                return 1;
        } else {
            if (device->Unacquire() >= 0)
                return 1;
        }
    }
    return 0;
}

// 0x004c2710
int PCInputDevice::SetDwordProperty(int property, unsigned long object, unsigned long how,
    unsigned long data) {
    if (!device)
        return 0;
    UnknownInputProperty value;
    value.size = sizeof(value);
    value.headerSize = 0x10;
    value.object = object;
    value.how = how;
    value.data = data;
    return device->SetProperty(property, &value) >= 0;
}

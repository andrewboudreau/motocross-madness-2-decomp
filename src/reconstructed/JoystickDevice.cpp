#include "JoystickDevice.h"

#include "UnknownObject56e26c.h"

// 0x00489800
JoystickDevice::JoystickDevice(int index) : PCInputDevice(2) {
    field_0x260 = index;
    for (int i = 0; i < 6; i++) {
        field_0x4e4[i] = 0;
        field_0x4fc[i].Init(1, 1);
    }
    for (int j = 0; j < 32; j++)
        field_0x264[j].state = 0;
    field_0x574_bit0 = g_UnknownGlobal56e26c->UnknownVirtualSlot22("JoyDirectionFlipped", 0);
}

// 0x00489920
JoystickDevice::~JoystickDevice() {}

#include "PCJoystickDevice.h"

#include "UnknownObject56e26c.h"

// 0x004c2770: repeats JoystickDevice's initialisation with PCJoystickDevice's
// own fields interleaved.
PCJoystickDevice::PCJoystickDevice(int index) : JoystickDevice(index) {
    field_0x260 = index;
    for (int k = 0; k < 5; k++)
        field_0x578[k] = 0;
    for (int i = 0; i < 6; i++) {
        field_0x58c[i] = 0;
        field_0x4e4[i] = 0;
        field_0x5a4[i] = 0;
        field_0x4fc[i].Init(1, 1);
    }
    for (int j = 0; j < 32; j++)
        field_0x264[j].state = 0;
    for (int n = 0; n < 4; n++)
        field_0x5d8[n] = -1;
    field_0x5e8 = 0;
    field_0x5d4_bit0 = 1;
    field_0x5d4_bit1 = 1;
    field_0x5d5 = 0;
    field_0x574_bit0 = g_UnknownGlobal56e26c->UnknownVirtualSlot22("JoyDirectionFlipped", 0);
}

// 0x004c28d0: releases the effects and resets the device (calls bind
// statically in a destructor).
PCJoystickDevice::~PCJoystickDevice() {
    UnknownVirtualSlot15();
    UnknownVirtualSlot16();
}

// 0x004c4320
void PCJoystickDevice::UnknownVirtualSlot15() {
    for (int i = 0; i < 5; i++) {
        if (field_0x578[i]) {
            field_0x578[i]->UnknownMethod2();
            field_0x578[i] = 0;
        }
    }
}

// 0x004c4350: command 1.
int PCJoystickDevice::UnknownVirtualSlot16() {
    if (field_0x25c && field_0x0c == 3)
        return UnknownVirtualSlot17(1);
    return 0;
}

// 0x004c4370
int PCJoystickDevice::UnknownVirtualSlot17(int command) {
    if (field_0x25c && field_0x0c == 3)
        field_0x25c->UnknownMethod22(command);
    return 0;
}

// 0x004c4390: command 4 when paused, 8 when resumed.
int PCJoystickDevice::UnknownVirtualSlot18(int paused) {
    if (field_0x0c == 3) {
        if (paused)
            return UnknownVirtualSlot17(4);
        return UnknownVirtualSlot17(8);
    }
    return 1;
}

#pragma once

#include "KeyboardDevice.h"

// RTTI: PCKeyboardDevice : KeyboardDevice. Slot 3 shares the empty body at
// 0x00464e90 (see samples/pccontrol). PCControl.cpp is a candidate TU.
class PCKeyboardDevice : public KeyboardDevice {
public:
    // No destructor is declared: retail's 0x004c4400 is compiler-generated
    // (wrapper 0x004c43e0).
    PCKeyboardDevice();           // 0x004c43c0

    virtual int UnknownVirtualSlot2();
    // Slot 3 is the shared empty body at 0x00464e90 (see samples/pccontrol).
    virtual void UnknownVirtualSlot3() {}
    virtual int UnknownVirtualSlot4(int control, int modifier, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot5(int key, int modifier, UnknownInputEntry* entry);
};

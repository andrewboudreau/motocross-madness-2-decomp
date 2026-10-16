#pragma once

#include "KeyboardDevice.h"

// RTTI: PCKeyboardDevice : KeyboardDevice. Slot 3 shares the empty body at
// 0x00464e90 (see samples/pccontrol). PCControl.cpp is a candidate TU.
class PCKeyboardDevice : public KeyboardDevice {
public:
    // No destructor is declared: retail's 0x004c4400 is compiler-generated
    // (wrapper 0x004c43e0).
    PCKeyboardDevice();           // 0x004c43c0

    virtual int CreateDevice();
    // Slot 3 is the shared empty body at 0x00464e90 (see samples/pccontrol).
    virtual void DetectSubtype() {}
    virtual int IsControlDown(int control, int modifier, UnknownInputEntry* entry);
    virtual int IsButtonDown(int key, int modifier, UnknownInputEntry* entry);
    virtual int Update(int value);

    // 0x004c4830: rebuilds the modifier state at +0x16d8 from the keys.
    void RefreshModifierState();
};

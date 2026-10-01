#pragma once

#include "KeyboardDevice.h"

// RTTI: PCKeyboardDevice : KeyboardDevice. Slot 3 shares the empty body at
// 0x00464e90 (see samples/pccontrol). PCControl.cpp is a candidate TU.
class PCKeyboardDevice : public KeyboardDevice {
public:
    // No destructor is declared: retail's 0x004c4400 is compiler-generated
    // (wrapper 0x004c43e0).
    PCKeyboardDevice();           // 0x004c43c0
};

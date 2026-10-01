#pragma once

#include "ContainerList.h"
#include "PCInputDevice.h"

// RTTI: KeyboardDevice : PCInputDevice. Slots 2-6 are _purecall here
// (PCKeyboardDevice implements them). TU not established.
class KeyboardDevice : public PCInputDevice {
public:
    // No destructor is declared: retail's 0x00489f20 is compiler-generated
    // (member and base destructors, no vptr store; wrapper 0x00489f00).
    KeyboardDevice();             // 0x00489e30

protected:
    UnknownInputEntry field_0x260[256];
    ContainerList<int> field_0x1660[6];
    int field_0x16d8;
};

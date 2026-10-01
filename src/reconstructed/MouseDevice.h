#pragma once

#include "ContainerList.h"
#include "PCInputDevice.h"

// RTTI: MouseDevice : PCInputDevice. Slots 2-6 are _purecall here
// (PCMouseDevice implements them). TU not established.
class MouseDevice : public PCInputDevice {
public:
    // No destructor is declared: retail's 0x0048a3c0 is compiler-generated
    // (wrapper 0x0048a3a0).
    MouseDevice();                // 0x0048a2d0

protected:
    UnknownInputEntry field_0x260[4];
    ContainerList<int> field_0x2b0[2];
};

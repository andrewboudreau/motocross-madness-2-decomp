#pragma once

#include "MouseDevice.h"

// RTTI: PCMouseDevice : MouseDevice. PCControl.cpp is a candidate TU.
class PCMouseDevice : public MouseDevice {
public:
    // No destructor is declared: retail's 0x004c4910 is compiler-generated
    // (wrapper 0x004c48f0).
    PCMouseDevice();              // 0x004c48c0

protected:
    int field_0x2d8[4];           // cleared by the constructor
};

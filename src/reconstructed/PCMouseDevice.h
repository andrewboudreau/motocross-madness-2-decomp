#pragma once

#include "MouseDevice.h"

// RTTI: PCMouseDevice : MouseDevice. PCControl.cpp is a candidate TU.
class PCMouseDevice : public MouseDevice {
public:
    // No destructor is declared: retail's 0x004c4910 is compiler-generated
    // (wrapper 0x004c48f0).
    PCMouseDevice();              // 0x004c48c0

    virtual int UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual int UnknownVirtualSlot4(int control, int modifier, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot5(int button, int modifier, UnknownInputEntry* entry);

protected:
    int field_0x2d8[4];           // movement (+0x2d8 x, +0x2dc y); cleared by the constructor
};

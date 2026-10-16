#pragma once

#include "MouseDevice.h"

// RTTI: PCMouseDevice : MouseDevice. PCControl.cpp is a candidate TU.
class PCMouseDevice : public MouseDevice {
public:
    // No destructor is declared: retail's 0x004c4910 is compiler-generated
    // (wrapper 0x004c48f0).
    PCMouseDevice();              // 0x004c48c0

    virtual int CreateDevice();
    virtual void DetectSubtype();
    virtual int IsControlDown(int control, int modifier, UnknownInputEntry* entry);
    virtual int IsButtonDown(int button, int modifier, UnknownInputEntry* entry);
    virtual int Update(int value);

protected:
    int movement[4];           // movement (+0x2d8 x, +0x2dc y); cleared by the constructor
};

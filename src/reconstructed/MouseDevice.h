#pragma once

#include "ContainerList.h"
#include "ControlInterface.h"
#include "PCInputDevice.h"

// RTTI: MouseDevice : PCInputDevice. It overrides slot 0; slots 2-6 are _purecall here
// (PCMouseDevice implements them). TU not established.
class MouseDevice : public PCInputDevice {
public:
    // No destructor is declared: retail's 0x0048a3c0 is compiler-generated
    // (wrapper 0x0048a3a0).
    MouseDevice();                // 0x0048a2d0

    virtual void UnknownVirtualSlot0(int id);  // 0x0048a4c0
    virtual int UnknownVirtualSlot2() = 0;
    virtual void UnknownVirtualSlot3() = 0;
    virtual int UnknownVirtualSlot4(int control, int modifier, UnknownInputEntry* entry) = 0;
    virtual int UnknownVirtualSlot5(int button, int modifier, UnknownInputEntry* entry) = 0;
    virtual int UnknownVirtualSlot6(int value) = 0;

    // 0x0048a550: moves every binding on `axis` by `amount` scaled to its
    // maximum.
    void UnknownFunction48a550(int axis, float amount);

protected:
    UnknownInputEntry field_0x260[4];
    ContainerList<UnknownControlBinding*> field_0x2b0[2]; // bindings per axis
};

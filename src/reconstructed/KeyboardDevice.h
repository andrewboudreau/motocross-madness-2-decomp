#pragma once

#include "ContainerList.h"
#include "ControlInterface.h"
#include "PCInputDevice.h"

// RTTI: KeyboardDevice : PCInputDevice. It overrides slot 0; slots 2-6 are
// _purecall here (PCKeyboardDevice implements them). TU not established.
class KeyboardDevice : public PCInputDevice {
public:
    // No destructor is declared: retail's 0x00489f20 is compiler-generated
    // (member and base destructors, no vptr store; wrapper 0x00489f00).
    KeyboardDevice();             // 0x00489e30

    virtual void UnknownVirtualSlot0(int id);  // 0x0048a030
    virtual int UnknownVirtualSlot2() = 0;
    virtual void UnknownVirtualSlot3() = 0;
    virtual int UnknownVirtualSlot4(int control, int modifier, UnknownInputEntry* entry) = 0;
    virtual int UnknownVirtualSlot5(int key, int modifier, UnknownInputEntry* entry) = 0;
    virtual int UnknownVirtualSlot6(int value) = 0;

    // 0x0048a0c0: key-repeat stepping for the bindings in list `list`.
    void UnknownFunction48a0c0(int list, int value);
    // 0x0048a240: whether modifier state `modifier` holds.
    int UnknownFunction48a240(int modifier);

    friend class ControlInterface; // reads the input entries

protected:
    UnknownInputEntry keyStates[256];
    ContainerList<UnknownControlBinding*> axisBindings[6];
    int modifierState;             // current modifier state
};

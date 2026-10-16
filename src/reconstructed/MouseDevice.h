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

    virtual void RemoveBindings(int id); // 0x0048a4c0
    virtual int CreateDevice() = 0;
    virtual void DetectSubtype() = 0;
    virtual int IsControlDown(int control, int modifier, UnknownInputEntry* entry) = 0;
    virtual int IsButtonDown(int button, int modifier, UnknownInputEntry* entry) = 0;
    virtual int Update(int value) = 0;

    // 0x0048a550: moves every binding on `axis` by `amount` scaled to its
    // maximum.
    void MoveAxis(int axis, float amount);
    // 0x0048a420: attaches `binding` to the list for its axis (GUIManager.cpp).
    int AttachBinding(UnknownControlBinding* binding);

    friend class ControlInterface; // reads the input entries

protected:
    UnknownInputEntry buttonStates[4];
    ContainerList<UnknownControlBinding*> axisBindings[2]; // bindings per axis
};

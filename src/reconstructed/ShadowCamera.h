#pragma once

#include "PCCamera.h"

// RTTI: ShadowCamera : PCCamera. It overrides slots 19 and 22 (both return 0)
// and shares PCCamera's compiler-generated destructor and deleting wrapper
// (0x004beda0). Its constructor sits between ProCircuitProcs.cpp and
// ProjectedShadow.cpp references; the TU is not established.
class ShadowCamera : public PCCamera {
public:
    explicit ShadowCamera(int flags); // 0x004da520

    virtual int UnknownVirtualSlot19(int value);
    virtual int UnknownVirtualSlot22(int a, int b);
};

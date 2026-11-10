#pragma once

#include "PCCamera.h"

// RTTI: ShadowCamera : PCCamera. It overrides slot 10 (0x004da540, which
// returns 1: the linker shares that body with GameObject::Step, so the shadow
// camera skips Camera::Tick), slots 19 and 22 (both return 0), and shares
// PCCamera's compiler-generated destructor and deleting wrapper (0x004beda0).
// Its constructor sits between ProCircuitProcs.cpp and ProjectedShadow.cpp
// references; the TU is not established.
class ShadowCamera : public PCCamera {
public:
    explicit ShadowCamera(int flags); // 0x004da520

    virtual int Tick(float frameTime);

    virtual int UnknownVirtualSlot19(int value);
    virtual int OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry);
};

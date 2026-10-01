#pragma once

#include "BikeCamera.h"

// Stack object built by KrustyBikeCamera slot 58 (constructor 0x0051b200,
// no destructor call) and handed to UnknownMessageTarget::0x0051b540.
class UnknownMessage {
public:
    UnknownMessage(const char* text, float duration);

    unsigned char field_0x00[0x8c];
};

class UnknownMessageTarget {
public:
    void UnknownFunction51b540(UnknownMessage* message); // 0x0051b540
};

// RTTI: KrustyBikeCamera : BikeCamera. KrustyBike.cpp is a candidate for its
// translation unit (name overlap and nearby references), not established.
// It keeps the camera state and presets in the 0x0056e26c object so they
// survive between cameras.
class KrustyBikeCamera : public BikeCamera {
public:
    explicit KrustyBikeCamera(int flags); // 0x00497cb0
    virtual ~KrustyBikeCamera();          // 0x00497d80 (deleting wrapper 0x00497d60)

    virtual int UnknownVirtualSlot23(int a, int b);
    virtual void UnknownVirtualSlot55();
    virtual void UnknownVirtualSlot58();
    virtual void UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60();
    virtual void UnknownVirtualSlot61();
    virtual void UnknownVirtualSlot62();

protected:
    int field_0x3b4;
    int field_0x3b8;
};

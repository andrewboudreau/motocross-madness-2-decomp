#pragma once

#include "GameObject.h"

#include "BikeCamera.h"
#include "RaceView.h"

struct UnknownKrustyBikeAxis {
    unsigned char field_0x000[0x150];
    float field_0x150;                   // slot 56 tests < -2
};

// Object at KrustyBikeCamera+0x3b4; its class is not established.
struct UnknownKrustyBike {
    unsigned char field_0x000[0x108];
    bool field_0x108;
    unsigned char field_0x109[0x43c - 0x109];
    float field_0x43c;                   // slot 42 scale
    unsigned char field_0x440[0x5f4 - 0x440];
    UnknownKrustyBikeAxis* field_0x5f4;
    unsigned char field_0x5f8[0x735 - 0x5f8];
    bool field_0x735;
    unsigned char field_0x736[0x7a4 - 0x736];
    bool field_0x7a4;
};

// RTTI: KrustyBikeCamera : BikeCamera. KrustyBike.cpp is a candidate for its
// translation unit (name overlap and nearby references), not established.
// It keeps the camera state and presets in the 0x0056e26c object so they
// survive between cameras.
class KrustyBikeCamera : public BikeCamera {
public:
    explicit KrustyBikeCamera(int flags); // 0x00497cb0
    virtual ~KrustyBikeCamera();          // 0x00497d80 (deleting wrapper 0x00497d60)

    // 0x00497d90: BikeCamera's setup (0x00416e80), then the "FlybyCam.vue"
    // frames; this or 0.
    KrustyBikeCamera* UnknownFunction497d90(void* value, float rate294, float rate298, float value228,
                                            float value2d0, float value2e8, int capacity, int count,
                                            const int* list);

    virtual int UnknownVirtualSlot10(float frameTime); // 0x00497e20 (near miss: samples/camera)

    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual void UnknownVirtualSlot42(bool flag);
    virtual Vector3 UnknownVirtualSlot48(int a, bool flag, int b);
    virtual void UnknownVirtualSlot52(Vector3* point);
    virtual unsigned char UnknownVirtualSlot55();
    virtual bool UnknownVirtualSlot56();
    virtual void UnknownVirtualSlot58();
    virtual void UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60();
    virtual void UnknownVirtualSlot61();
    virtual void UnknownVirtualSlot62();

protected:
    UnknownKrustyBike* krustyBike;       // +0x3b4
    UnknownKrustyBikeView* raceView;     // +0x3b8
};

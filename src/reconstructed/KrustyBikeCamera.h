#pragma once

#include "GameObject.h"

#include "BikeCamera.h"
#include "RaceView.h"

struct UnknownKrustyBikeAxis {
    unsigned char field_0x000[0x150];
    float penetration;                        // +0x150  BikeWheel penetration (CollisionPoint+0x98; reset to -999)
};

// Object at KrustyBikeCamera+0x3b4. Every member below sits at a KrustyBike offset with a
// matching use, so it is read as the followed KrustyBike (tier 2).
struct UnknownKrustyBike {
    unsigned char field_0x000[0x108];
    bool airborne;                            // +0x108  SoultreePhysicsBaseObject airborne (Vehicle slot 49)
    unsigned char field_0x109[0x43c - 0x109];
    float turnRate;                           // +0x43C  Vehicle turnRate (velocity deflection per second, slot 72)
    unsigned char field_0x440[0x5f4 - 0x440];
    UnknownKrustyBikeAxis* rearWheel;         // +0x5F4  Bike rearWheel (a BikeWheel)
    unsigned char field_0x5f8[0x735 - 0x5f8];
    bool isRemote;  // +0x735  KrustyBike +0x735 isRemote
    unsigned char field_0x736[0x7a4 - 0x736];
    bool hasFinished;                         // +0x7A4  KrustyBike/RaceView hasFinished
};

// RTTI: KrustyBikeCamera : BikeCamera. Its code (0x00497cb0..0x004987ec) follows
// KrustyBike.cpp's but ends with its own Math3D vector set, so it is a separate
// unit; the file name is ours.
// It keeps the camera state and presets in the 0x0056e26c object so they
// survive between cameras.
class KrustyBikeCamera : public BikeCamera {
public:
    explicit KrustyBikeCamera(int flags); // 0x00497cb0
    virtual ~KrustyBikeCamera();          // 0x00497d80 (deleting wrapper 0x00497d60)

    // 0x00497d90: BikeCamera's setup (0x00416e80), then the "FlybyCam.vue"
    // frames; this or 0.
    KrustyBikeCamera* SetupFlyby(void* value, float rate294, float rate298, float value228,
                                            float value2d0, float value2e8, int capacity, int count,
                                            const int* list);

    virtual int Tick(float frameTime);                 // 0x00497e20 (near miss: samples/camera)

    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual Vector3 UnknownVirtualSlot34(float dt);
    virtual void UnknownVirtualSlot42(bool flag);
    virtual Vector3 UnknownVirtualSlot48(int a, bool flag, float dt);
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

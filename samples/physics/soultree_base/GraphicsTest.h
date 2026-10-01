// GraphicsTest.h -- GraphicsTest : GameObject, shared by every GraphicsTest-derived physics
// class (CollisionObject in collision/, PhysicsBody in rigidbody/).  Split out of
// collision/CollisionObject.h and rigidbody/PhysicsBody.h (which each carried a copy) so the
// class has one declaration.
//
// Evidence:
//  * RTTI .?AVGraphicsTest@@ with plain (non-virtual) base GameObject (BCD mdisp 0 pdisp -1),
//    vtable 0x00553de4, 27 slots.  It differs from GameObject's 0x00552a2c only in slot 0
//    (scalar deleting dtor 0x0047bce0, core 0x0047bd00): tier 1 (analysis/vtables.json).
//  * Ctor 0x0047bc70 (thiscall, ret 4): GameObject(a) with its own argument, vptr
//    0x00553de4, SetDrawColor(0xff, 0xff, 0xff) (0x0047c690, packs into +0x2c), then
//    field_0x30 = 0.  Tier 1 decoded.  CollisionObject passes 1; PhysicsBody forwards its own
//    constructor argument.
//  * Size 0x38: PhysicsBody's first own field (name) is at 0x38 and PhysicsRigidBody's ctor
//    zeroes +0x34 (tier 2: extent from the derived classes).  CollisionObject's first
//    accessed own field is at 0x50 = 12 + 0x44, so CollisionObject has 0xc unaccessed bytes
//    of its own before it.
#ifndef GRAPHICS_TEST_H
#define GRAPHICS_TEST_H

#include "GameObject.h"

struct Vec3;
struct Matrix4;

class GraphicsTest : public GameObject {
public:
    explicit GraphicsTest(int flags);           // 0x0047bc70, forwards flags to GameObject(int)
    virtual ~GraphicsTest();                    // slot 0: deleting 0x0047bce0, core 0x0047bd00

    // Non-virtual GraphicsTest methods used by CollisionObject::GameObjectVirtualSlot14.
    void Fn_0047c6c0(int a, int b, int c, int d);
    void Fn_0047c6f0();
    void Fn_0047c0b0(void* a, int b, int c);
    void Fn_00469ce0(GraphicsTest* owner);   // 0x00469ce0, registers the object (thiscall, 1 arg)
    // Debug line drawing, used by collision/CollisionDebugDraw.cpp.  Names are tier 3.
    //  * 0x0047c690 (ret 0xc) packs 0xff000000 | r<<16 | g<<8 | b into field_0x2c (decoded).
    //  * 0x0047c4f0 (ret 8): a line between two world points.
    //  * 0x0047c570 (ret 8): a small marker; it halves `size` and offsets the point by it.
    //  * 0x0047c270 (ret 0xc): an oriented box from center, half extents and a transform
    //    (callers pass CollisionBoxBounds-style center/half-extent pairs).
    void SetDrawColor(int r, int g, int b);                                      // 0x0047c690
    void DrawBox(const Vec3* center, const Vec3* halfExtents, const Matrix4* xf); // 0x0047c270
    void DrawLine(const Vec3* a, const Vec3* b);                                 // 0x0047c4f0
    void DrawMarker(const Vec3* p, float size);                                  // 0x0047c570

    int field_0x2c;                   // packed draw color (SetDrawColor)
    int field_0x30;                   // zeroed by the GraphicsTest constructor
    int field_0x34;                   // zeroed by the PhysicsRigidBody constructor
};

typedef char graphics_test_assert_sizeof[(sizeof(GraphicsTest) == 0x38) ? 1 : -1];

#endif

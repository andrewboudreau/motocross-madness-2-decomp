// GraphicsTest.h -- GraphicsTest : GameObject, shared by every GraphicsTest-derived physics
// class (CollisionObject in collision/, PhysicsBody in rigidbody/).  Split out of
// collision/CollisionObject.h and samples/physics/rigidbody/PhysicsBody.h (which each carried a copy) so the
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
    // 0x0047c6c0 (ret 0x10): drawColor = a<<24 | r<<16 | g<<8 | b (decoded); SetDrawColor with alpha.
    void SetDrawRGBA(int r, int g, int b, int a);
    // 0x0047c6f0: no texture, diffuse alpha stage states, alpha blending on (render state 0x1b).
    void BeginAlphaBlend();
    // 0x0047c0b0 (ret 0xc): a wire sphere of `segments` rings (center, radius, segments; the
    // radius is pushed as a float by CollisionObject slot 14).  Name tier 3.
    void DrawSphere(const Vec3* center, float radius, int segments);
    // 0x0047bd10 (ret 0x10): a circle (center, radius, axis, segments, clamped to 64); tier 3.
    void DrawCircle(const Vec3* center, float radius, const Vec3* axis, int segments);
    // 0x00469ce0 (thiscall, 1 arg): appends the RTTI class name of `owner` to the debug name
    // string at GameObject+0x28 (src/reconstructed/GameObject.h AppendClassName).
    void AppendClassName(GraphicsTest* owner);
    // Debug line drawing, used by collision/CollisionDebugDraw.cpp.  Names are tier 3.
    //  * 0x0047c690 (ret 0xc) packs 0xff000000 | r<<16 | g<<8 | b into drawColor (decoded).
    //  * 0x0047c4f0 (ret 8): a line between two world points.
    //  * 0x0047c570 (ret 8): a small marker; it halves `size` and offsets the point by it.
    //  * 0x0047c270 (ret 0xc): an oriented box from center, half extents and a transform
    //    (callers pass CollisionBoxBounds-style center/half-extent pairs).
    void SetDrawColor(int r, int g, int b);                                      // 0x0047c690
    void DrawBox(const Vec3* center, const Vec3* halfExtents, const Matrix4* xf); // 0x0047c270
    void DrawLine(const Vec3* a, const Vec3* b);                                 // 0x0047c4f0
    void DrawMarker(const Vec3* p, float size);                                  // 0x0047c570

    int drawColor;                   // +0x2c packed draw color (SetDrawColor)
    int field_0x30;                   // zeroed by the GraphicsTest constructor
    int field_0x34;                   // zeroed by the PhysicsRigidBody constructor
};

typedef char graphics_test_assert_sizeof[(sizeof(GraphicsTest) == 0x38) ? 1 : -1];

#endif

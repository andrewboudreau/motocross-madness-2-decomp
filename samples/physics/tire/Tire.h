// Tire -- reconstructed class layout (Tire.cpp).
//
// Layout evidence (tier 1 unless noted), analysis/rtti_classes.json + vtables.json:
//  * RTTI .?AVTire@@ (COL 0x0055f5e0), direct bases in declaration order
//    CollisionObject, MovingPart, CollisionPoint (attributes=1: multiple inheritance).
//    All three bases are NON-virtual (pdisp -1).  Base class array offsets (mdisp):
//    CollisionObject/QuadTreeObject 0, GraphicsTest/GameObject/BaseObject 12,
//    CollisionPoint 184 (0xb8), MovingPart 380 (0x17c).
//    VC6 lays the polymorphic CollisionPoint out before the non-polymorphic MovingPart
//    even though MovingPart is declared first (MovingPart has no vtable record).
//  * vtables (object_offset): 0x00558578 @0 (2 slots, QuadTreeObject shape: 0x004dc610,
//    0x00434ce0 both inherited), 0x00558508 @12 (27 slots, GameObject shape; overrides
//    vs CollisionObject: slot 0 = dtor 0x005133a0, slot 8 = 0x00513490),
//    0x005584fc @184 (2 slots: slot 0 = adjustor thunk 0x00515730 `sub ecx,0xac; jmp
//    0x005133a0` -- the deleting dtor seen from the CollisionPoint subobject; slot 1 =
//    0x00515b50 overrides CollisionPoint slot 1 without any thunk).
//  * ctor 0x00512f10 (`ret 0x3c`, 15 argument dwords), size 0x2c0: the caller 0x004084d7
//    allocates `operator new(0x2c0, __FILE__, 0x6b8)`.
//  * dtor core 0x005133d0, deleting wrapper 0x005133a0 (virtual ~Tire).
//
// MovingPart is a non-polymorphic 0x4c-byte class (ctor 0x004a23a0 initialises 0x40..0x48
// and has ret 0xc, the dtor is the shared empty function 0x00464e90).  Its real size is
// only known to be >= 0x4c (tier 2: the next Tire member 0x1c8 is written by the Tire
// ctor).  PROVISIONAL area-local declaration: MovingPart has no RTTI virtuals and no
// reconstruction elsewhere yet.
//
// Member names: field_0xNN are Tire-object-relative for Tire's own members.  Names of
// inherited members are qualified where two bases share the number (CollisionObject and
// CollisionPoint both have field_0x88 / field_0x8c).  Semantic names are tier 3.
#ifndef TIRE_H
#define TIRE_H

#include <stddef.h>
#include "../collision/CollisionObject.h"

#include "../collision/CollisionPoint.h"

// Provisional (see above).  Constructor args (a, b, c): a is a scene node/name handle
// handed to 0x004fdae0, c is stored at +0x44.
class MovingPart {
public:
    MovingPart(void* a, int b, int c);      // 0x004a23a0 (ret 0xc)
    ~MovingPart();                          // 0x00464e90, an empty out-of-line dtor

    char field_0x00[0x40];
    void* field_0x40;                       // result of 0x004fdae0(b)
    int field_0x44;                         // ctor arg c
    int field_0x48;                         // 0
};

// Provisional stand-ins for the two heap objects the dtor releases (fields 0x2ac/0x2b0).
// Their dtors are 0x004fa8a0 and 0x004f9f80 (both in the scene-graph range); tier 3.
class TireAttachA { public: ~TireAttachA(); };   // 0x004f9f80
class TireAttachB { public: ~TireAttachB(); };   // 0x004fa8a0

// Global Vec3 constants used by the Tire constructor (all in .bss, 0x10 apart; zero, +Y axis
// and one more unit-length candidate, tier 3 names).  Only the {x,y,z} layout is confirmed.
extern CollisionVec3 g_TireZeroVec3;        // 0x0068a3d0 (zero: reset value of most vectors)
extern CollisionVec3 g_TireVec3_68a3c0;     // 0x0068a3c0 (initial value of field_0x20c)
extern CollisionVec3 g_TireVec3_68a3f0;     // 0x0068a3f0 (initial value of CollisionPoint::field_0x2c)

// Provisional area-local stand-ins for two scene-side classes only known by their calls.
class TireNode {                                       // scene node, MovingPart::field_0x40
public:
    void Fn_004fc9a0(TireNode* parent, CollisionVec3* out);                        // world position
    void Fn_004fe0a0(CollisionVec3* a, CollisionVec3* b);                          // extents, b.y = wheel radius
    CollisionVec3* Fn_004fd5c0(CollisionVec3* out, const CollisionVec3* in);       // rotate a direction
};
class TireWorld {                                      // world/terrain query object (arg 1 of 0x00514550)
public:
    int Fn_00507c10(CollisionVec3* pos, CollisionVec3* outNormal, int flags, unsigned char* outSurface);
};

class Tire : public CollisionObject, public MovingPart, public CollisionPoint {
public:
    // 0x00512f10, ret 0x3c: 15 dwords.  Argument roles come from the stores in the body.
    // Types follow the destination fields (tier 3): a1 goes to MovingPart, a4 to CollisionPoint's
    // field_0x88, a15 to CollisionPoint::field_0xc0.
    Tire(void* a1, int a2, float a3, float a4, int a5, int a6, int a7, float a8,
         float a9, void* a10, float a11, float a12, float a13, float a14, int a15);
    virtual ~Tire();                            // slot 0 @12 (0x005133a0), core 0x005133d0
    virtual GameObject* GameObjectVirtualSlot8(int a);  // 0x00513490
    virtual void CollisionPointVirtualSlot1();  // CollisionPoint slot 1 @184, 0x00515b50

    // Non-virtual members (this == complete object).
    void HandleContact(int a, int tag, CollisionObject* other);   // 0x00512e80 (ret 0xc)
    // 0x00514550 (thiscall, ret 0x20), tier 3 name: places the wheel on the ground.  Recomputes
    // the wheel axes (field_0x20c/0x230/0x23c), probes the world/terrain for the contact point
    // and normal, builds the wheel frame and reports the contact through CollisionPoint's
    // field_0x98 (penetration/drop).  Callers 0x00528f65 and 0x00528fd8.
    void UpdateSuspensionProbe(TireWorld* world, const CollisionVec3* velocity, float angle,
                               float radiusScale, float a5, const CollisionVec3* a6,
                               const CollisionVec3* a7, TireNode* a8);
    // 0x00515c90 (thiscall, three vector pointers and a hidden result pointer first).
    CollisionVec3* Fn_00515c90(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b,
                               const CollisionVec3* c);
    // 0x005135f0 (thiscall, ret 0x28), tier 3 name: contact-patch update.  Recomputes the
    // relative position/lever vectors and the tangent direction, length and slip-like scalar
    // field_0x27c; see Tire.cpp.  Single caller 0x00529aa7.
    void UpdateContactPatch(const CollisionVec3* pos, CollisionVec3 axis, CollisionVec3 ref,
                            float minLength, float* outValue, int* outSign);

    // --- Tire's own data 0x1c8..0x2c0 (MovingPart ends at 0x1c8) ---
    float field_0x1c8;
    float field_0x1cc;
    float field_0x1d0;
    float field_0x1d4;
    float field_0x1d8;
    float field_0x1dc;
    float field_0x1e0;
    int field_0x1e4;
    CollisionVec3 field_0x1e8;
    CollisionVec3 field_0x1f4;
    CollisionVec3 field_0x200;
    CollisionVec3 field_0x20c;
    CollisionVec3 field_0x218;
    CollisionVec3 field_0x224;
    CollisionVec3 field_0x230;
    CollisionVec3 field_0x23c;
    CollisionVec3 field_0x248;
    CollisionVec3 field_0x254;
    int field_0x260;
    int field_0x264;
    int field_0x268;
    int field_0x26c;
    float field_0x270;
    float field_0x274;
    int field_0x278;
    float field_0x27c;                      // written by UpdateContactPatch (float, tier 2)
    int field_0x280;
    float field_0x284;                      // 1.0f default or a surface table entry (tier 2)
    float field_0x288;
    float field_0x28c;
    int field_0x290;
    float field_0x294;
    int field_0x298;
    int field_0x29c;
    void* field_0x2a0;
    float field_0x2a4;
    int field_0x2a8;
    TireAttachB* field_0x2ac;               // released by the dtor when field_0x2b0 is null
    TireAttachA* field_0x2b0;               // released first (dtor 0x004f9f80)
    int field_0x2b4;
    float field_0x2b8;
    int field_0x2bc;
};

// Base offsets (CollisionObject @0, CollisionPoint @0xb8 = 184, MovingPart @0x17c) are proven by
// the dtor core 0x005133d0 / deleting dtor 0x005133a0 and the vtable record Tire+184 (tier 1/2).
// The polymorphic CollisionPoint is placed before the non-polymorphic MovingPart by the compiler
// (so the declaration order of the two bases above does not matter for the layout).
typedef char tire_assert_first_field[(offsetof(Tire, field_0x1c8) == 0x1c8) ? 1 : -1];
typedef char tire_assert_last_field[(offsetof(Tire, field_0x2bc) == 0x2bc) ? 1 : -1];
typedef char tire_assert_sizeof[(sizeof(Tire) == 0x2c0) ? 1 : -1];
typedef char tire_assert_collision_point[(sizeof(CollisionPoint) == 0xc4) ? 1 : -1];

#endif

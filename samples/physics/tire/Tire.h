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
#include "collision/CollisionObject.h"

#include "collision/CollisionPoint.h"

// Provisional (see above).  Constructor args (a, b, c): a is a scene node/name handle
// handed to 0x004fdae0, c is stored at +0x44.
class TireNode;

class MovingPart {
public:
    MovingPart(void* a, int b, int c);      // 0x004a23a0 (ret 0xc)
    ~MovingPart();                          // 0x00464e90, an empty out-of-line dtor

    char field_0x00[0x40];
    TireNode* sceneNode;                   // +0x40 result of 0x004fdae0(b)
    int ownerRef;                         // +0x44 ctor arg c
    int field_0x48;                         // 0
};

// Provisional stand-ins for the two heap objects the dtor releases (fields 0x2ac/0x2b0).
// Their dtors are 0x004fa8a0 and 0x004f9f80 (both in the scene-graph range); tier 3.
// 0x00515660 reads both: +0x44 is a scene node and +0x98 a joint value. A keeps a
// direction at +0xc0 and drives the node's position along it; B keeps a rest matrix at
// +0x04, a rotation axis at +0xd4 and divides +0x98 by +0xc8 for the angle.
class TireNode;
// 0x00513c70 (UpdateShock) reads +0x54 (an "extending" flag copied to Tire+0x26c),
// +0x70 (a ratio tested against zero) and adds the vector at +0x80 to the contact's
// world position after the shock solve: InlineShock::SolveContact 0x004fa400 and
// RotatingShock::SolveContact 0x004fac60 (samples/physics/suspension/Suspension.h).
// The solves are declared as in Suspension.h; the caller's dt and velocity slots are
// dead after the call and hold its `active` and `load` outputs.
class TireAttachA {
public:
    ~TireAttachA();                      // 0x004f9f80
    void SolveContact(float dt, const CollisionVec3* offset, const CollisionVec3* base, const CollisionVec3* point,
                      const CollisionVec3* normal, float limit, float* outLoad,
                      int* outActive);   // 0x004fa400 (ret 0x20)
    char field_0x00[0x44];
    TireNode* node;                      // +0x44
    char field_0x48[0x54 - 0x48];
    int extending;                       // +0x54
    char field_0x58[0x70 - 0x58];
    float ratio;                         // +0x70
    char field_0x74[0x80 - 0x74];
    CollisionVec3 field_0x80;            // +0x80
    char field_0x8c[0x98 - 0x8c];
    float position;                      // +0x98
    char field_0x9c[0xc0 - 0x9c];
    CollisionVec3 direction;             // +0xc0
};
class TireAttachB {
public:
    ~TireAttachB();                      // 0x004fa8a0
    void SolveContact(float dt, const CollisionVec3* axisA, const CollisionVec3* axisB,
                      const CollisionVec3* point, const CollisionVec3* normal, const CollisionVec3* offsetA,
                      const CollisionVec3* offsetB, float limit, float* outLoad,
                      int* outActive);   // 0x004fac60 (ret 0x28)
    int field_0x00;
    CollisionMatrix4 restMatrix;         // +0x04
    TireNode* node;                      // +0x44
    char field_0x48[0x54 - 0x48];
    int extending;                       // +0x54
    char field_0x58[0x70 - 0x58];
    float ratio;                         // +0x70
    char field_0x74[0x80 - 0x74];
    CollisionVec3 field_0x80;            // +0x80
    char field_0x8c[0x98 - 0x8c];
    float position;                      // +0x98
    char field_0x9c[0xc8 - 0x9c];
    float scale;                         // +0xc8
    char field_0xcc[0xd4 - 0xcc];
    CollisionVec3 axis;                  // +0xd4
};

// Provisional area-local stand-ins for two scene-side classes only known by their calls.
class TireNode {                                       // scene node, MovingPart::field_0x40
public:
    void GetPositionIn(TireNode* parent, CollisionVec3* out);                        // world position
    void GetLocalBounds(CollisionVec3* a, CollisionVec3* b);                          // extents, b.y = wheel radius
    CollisionVec3* LocalToWorldDirection(CollisionVec3* out, const CollisionVec3* in);       // rotate a direction
    void RotateAbout(float x, float y, float z, float angle);                      // 0x004fcce0
    void SetPosition(CollisionVec3 position);                                      // 0x004fc630
    void SetLocalMatrix(const CollisionMatrix4* m);                                // 0x004fca30
    void Rotate(CollisionVec3 axis, float angle);                                  // 0x004fceb0
};
class TireWorld {                                      // world/terrain query object (arg 1 of 0x00514550)
public:
    int QueryGround(CollisionVec3* pos, CollisionVec3* outNormal, int flags, unsigned char* outSurface);
};

// The vehicle passed to 0x00513f90 (Vehicle.cpp 0x00525d.. passes `this`): only the
// members read there are declared (provisional view of Vehicle).
class Tire;
class TireVehicle {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16();
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual void UnknownVirtualSlot22();
    virtual void UnknownVirtualSlot23();
    virtual void UnknownVirtualSlot24();
    virtual void UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29();
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual void UnknownVirtualSlot32();
    virtual void UnknownVirtualSlot33();
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35();
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot38();
    virtual void UnknownVirtualSlot39();
    virtual void UnknownVirtualSlot40();
    virtual void UnknownVirtualSlot41();
    virtual void UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot43();
    virtual void UnknownVirtualSlot44();
    virtual void UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot46();
    virtual float UnknownVirtualSlot47(Tire* wheel);   // grip factor for this wheel

    char pad_0x04[0x24 - 0x04];
    float field_0x24;
    char pad_0x28[0x60 - 0x28];
    float field_0x60;
    char pad_0x64[0x70 - 0x64];
    CollisionVec3 field_0x70;              // velocity-like; its length drives the factor
    char pad_0x7c[0xa4 - 0x7c];
    float field_0xa4;
    char pad_0xa8[0x444 - 0xa8];
    int field_0x444;
};

// Object at Tire+0x2a8 (Vehicle.h's VehicleWheelAux); only its first float is read here.
struct TireAux {
    float value;
};

class Tire : public CollisionObject, public MovingPart, public CollisionPoint {
public:
    // 0x00512f10, ret 0x3c: 15 dwords.  Argument roles come from the stores in the body.
    // Types follow the destination fields (tier 3): a1 goes to MovingPart, a4 to CollisionPoint's
    // field_0x88, a15 to CollisionPoint::field_0xc0.
    Tire(void* a1, int a2, float a3, float a4, int a5, int a6, float a7, float a8,
         float a9, float a10, float a11, float a12, float a13, float a14, int a15);
    virtual ~Tire();                            // slot 0 @12 (0x005133a0), core 0x005133d0
    virtual GameObject* GameObjectVirtualSlot8(int a);  // 0x00513490
    virtual void CollisionPointVirtualSlot1();  // CollisionPoint slot 1 @184, 0x00515b50

    // Non-virtual members (this == complete object).
    void HandleContact(int a, int tag, CollisionObject* other);   // 0x00512e80 (ret 0xc)
    // 0x00514550 (thiscall, ret 0x20), tier 3 name: places the wheel on the ground.  Recomputes
    // the wheel axes (wheelUpAxis/0x230/0x23c), probes the world/terrain for the contact point
    // and normal, builds the wheel frame and reports the contact through CollisionPoint's
    // field_0x98 (penetration/drop).  Callers 0x00528f65 and 0x00528fd8.
    void UpdateSuspensionProbe(TireWorld* world, const CollisionVec3* velocity, float angle,
                               float radiusScale, float a5, const CollisionVec3* a6,
                               const CollisionVec3* a7, TireNode* a8);
    // 0x00513560: sets the roll angle from a rolled distance (times 1 / radius) and
    // turns the wheel node to it about x; 0x005135b0 re-applies the stored angle.
    void SetRollDistance(float distance);
    void ApplyRollAngle();
    // 0x00515660: poses the attached part (+0x2b0 first, else +0x2ac) from its value.
    void UpdateAttachment();
    // 0x005143d0 (ret 0x18): advances the wheel's roll angle for a step; a5 is unused.
    // 0x00513f90 (ret 4): sets the drive share +0x2b8 (and +0x2bc = 1 - it) from the
    // vehicle's speed while the wheel slips less than +0x2b4; scales +0x248 by it.
    void UpdateDriveShare(TireVehicle* vehicle);
    // 0x00514170 (ret 0x1c): pushes along the roll direction with the drive strength,
    // limited by what the body can take this step; the push goes to *force and its
    // moment about the contact (times +0x1d8) to *torque.
    void ApplyDrive(float share, float stepTime, int forward, float mass, float* speed,
                    CollisionVec3* torque, CollisionVec3* force);
    void UpdateRoll(float dt, float distanceScale, int locked, int driven, int a5, float driveScale);
    // 0x00513c70 (ret 0x18), tier 3 name: runs the attached shock (+0x2b0, else +0x2ac)
    // against the contact while the wheel touches; the solve's load and active flag
    // become the contact's penetration and inContact. Vehicle.cpp 0x00529ad4 is the
    // only caller (dt = +0x1e8, a2 = byte +0x1d0, crashed = +0x444, offset = +0xbc,
    // velocity = &+0x64, axisB = &+0xa0); a2 and offset are unused.
    void UpdateShock(float dt, int a2, int crashed, const CollisionVec3* offset,
                     const CollisionVec3* velocity, const CollisionVec3* axisB);

    // 0x00515c90 (thiscall, three vector pointers and a hidden result pointer first).
    CollisionVec3* Fn_00515c90(CollisionVec3* out, const CollisionVec3* a, const CollisionVec3* b,
                               const CollisionVec3* c);
    // 0x005135f0 (thiscall, ret 0x28), tier 3 name: contact-patch update.  Recomputes the
    // relative position/lever vectors and the tangent direction, length and slip-like scalar
    // slipSpeed; see Tire.cpp.  Single caller 0x00529aa7.
    void UpdateContactPatch(const CollisionVec3* pos, CollisionVec3 axis, CollisionVec3 ref,
                            float minLength, float* outValue, int* outSign);

    // --- Tire's own data 0x1c8..0x2c0 (MovingPart ends at 0x1c8) ---
    float invWheelRadius;  // +0x1c8 ctor: 1.0f / extentB.y (wheelRadius is the radius)
    float sideFriction;  // +0x1cc ctor arg a4; slot 1 uses it times sideFrictionScale times sin of the tangent angle (sideways grip)
    float rollFrictionScale;  // +0x1d0 ctor arg a8; multiplies rollFriction in the cos term of the slot 1 friction
    float sideFrictionScale;  // +0x1d4 ctor arg a9; multiplies sideFriction in the sin term
    float field_0x1d8;  // ctor arg a11; 0x00514170 scales the torque
    float field_0x1dc;
    float field_0x1e0;
    int hasContactObjectVelocity;  // +0x1e4 HandleContact: set 1 when a vector was captured, 0 for tag 0x65/other
    CollisionVec3 slipVector;  // +0x1e8 UpdateContactPatch: axis*field_0x90 + normalLeverCross, normalised into CollisionPoint::field_0x50
    CollisionVec3 contactObjectVelocity;  // +0x1f4 HandleContact: copy of other owner vector at +0x224 (CollisionCharacter::wheelVelocity is its velocity), +0x40 or +0x64
    CollisionVec3 wheelCenter;  // +0x200 UpdateSuspensionProbe: world position of the scene node (0x004fc9a0); probes are offset from it by the radius
    CollisionVec3 wheelUpAxis;  // +0x20c UpdateSuspensionProbe: node-rotated direction kVec3ZAxis; fallback and frame axis passed to TireBuildFrame
    CollisionVec3 normalLeverCross;  // +0x218 UpdateContactPatch: cross(n*(axis.n), relativePos) with n = contact normal; fallback tangent source when the projected dot <= 0.001; ctor zero vector
    CollisionVec3 wheelVelocity;  // +0x224 UpdateSuspensionProbe: copy of its velocity argument (*velocity) at the end; ctor zero vector
    CollisionVec3 rollDirection;  // +0x230 normalise(cross(normal, side)); slot 1 and UpdateContactPatch dot it with the tangent
    CollisionVec3 sideAxis;  // +0x23c cross(up, node axis) normalised; the wheel axle direction used for the probes
    CollisionVec3 field_0x248;
    CollisionVec3 field_0x254;
    int inContact;  // +0x260 UpdateSuspensionProbe: depth >= CollisionPoint::field_0x9c ? 1 : 0
    int reportContactOutputs;  // +0x264 UpdateContactPatch writes *outValue and *outSign only when it is nonzero; ctor 0 (tier 3: meaning of the flag)
    int field_0x268;
    int field_0x26c;
    float rollFriction;  // +0x270 ctor arg a3; cos term factor in slot 1
    float wheelRadius;  // +0x274 ctor: y extent of the node; slot 8 builds the 8-point rim polyline with it; probes offset by it
    float rollAngle;  // +0x278 0x00513560: distance * invWheelRadius; the wheel node's x rotation
    float slipSpeed;                      // +0x27c written by UpdateContactPatch (float, tier 2)
    float field_0x280;  // 0x005143d0: drive factor
    float surfaceScaleB;                      // +0x284 1.0f default or a surface table entry (tier 2)
    float tangentSin;  // +0x288 slot 1: sqrt(1 - cos^2)
    float tangentCos;  // +0x28c slot 1: |tangent . rollDirection| clamped to 1
    float field_0x290;  // 0x005143d0: last drive distance
    float field_0x294;
    int field_0x298;
    float rampLevel;  // +0x29c 0x005143d0 unwinds the roll angle by (1 - rampLevel) while set
    float field_0x2a0;  // ctor arg a10; 0x00514170 drive strength
    float field_0x2a4;
    TireAux* field_0x2a8;  // 0x005143d0: its value scales the roll angle when in (0, 1)
    TireAttachB* field_0x2ac;               // released by the dtor when field_0x2b0 is null
    TireAttachA* field_0x2b0;               // released first (dtor 0x004f9f80)
    float field_0x2b4;  // 0x00513f90: slip threshold
    float field_0x2b8;  // 0x00513f90: drive share
    float field_0x2bc;  // 0x005143d0: drive scale
};

// Base offsets (CollisionObject @0, CollisionPoint @0xb8 = 184, MovingPart @0x17c) are proven by
// the dtor core 0x005133d0 / deleting dtor 0x005133a0 and the vtable record Tire+184 (tier 1/2).
// The polymorphic CollisionPoint is placed before the non-polymorphic MovingPart by the compiler
// (so the declaration order of the two bases above does not matter for the layout).
typedef char tire_assert_first_field[(offsetof(Tire, invWheelRadius) == 0x1c8) ? 1 : -1];
typedef char tire_assert_last_field[(offsetof(Tire, field_0x2bc) == 0x2bc) ? 1 : -1];
typedef char tire_assert_sizeof[(sizeof(Tire) == 0x2c0) ? 1 : -1];
typedef char tire_assert_collision_point[(sizeof(CollisionPoint) == 0xc4) ? 1 : -1];

#endif

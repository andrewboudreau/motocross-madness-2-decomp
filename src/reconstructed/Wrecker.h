#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"

// Reconstruction of part of D:\aardvark\VC\krusty2\wrecker.cpp. Evidence and
// the per-function table are in docs/WRECKER.md.
//
// Confirmed (RTTI): Wrecker : GraphicsTest : GameObject : BaseObject, all
// single non-virtual inheritance; primary vtable 0x00558e24 (27 slots,
// COL 0x0055fb70). Wrecker overrides slot 0 (deleting dtor 0x005300a0),
// slot 10 (0x005306e0), slot 14 (0x004de400) and slot 23 (0x00532890).
// Allocation size 0x53c (Bike slot 97, `new(bike.cpp) Wrecker(1)`).

// GraphicsTest (RTTI .?AVGraphicsTest@@, vtable 0x00553de4). Only what the
// Wrecker code reaches is declared; the canonical, fuller declaration is
// src/krusty2/core/GraphicsTest.h. Constructor 0x0047bc70 (thiscall, ret 4),
// destructor core 0x0047bd00. Wrecker's own fields start at +0x34: the first
// Wrecker method that sets it (0x005301c0) is the only writer seen here, so
// whether +0x34 belongs to GraphicsTest is open.
class GraphicsTest : public GameObject {
public:
    explicit GraphicsTest(int flags);
    virtual ~GraphicsTest();

    // Debug drawing (bikerace.cpp slot 14): 0x0047c690 packs the draw
    // colour, 0x0047c4f0 draws a line between two world points.
    void SetDrawColor(int r, int g, int b);
    void DrawLine(const Vector3* a, const Vector3* b);

protected:
    int field_0x2c;            // packed draw colour (0x0047c690)
    int field_0x30;            // zeroed by the GraphicsTest constructor
};

// Object at Wrecker+0x38, RTTI ConstraintMethodCollisionModel (constructor
// 0x0043b8d0 writes vtables 0x00551260 at +0 and 0x005511f0 (a GameObject-shaped
// table) at +0xc; new size 0x10c at wrecker.cpp line 0x68). The secondary
// GameObject subobject at +0xc is what UnknownVirtualSlot8/10 dispatch on.
class UnknownWreckerQuadTreeObject {
public:
    virtual void UnknownVirtualSlot0();
    int field_0x04;
    int field_0x08;
};

struct UnknownWreckerSkeleton;

// 32-byte probe record of the collision model (+0x104, count +0x100).
struct UnknownWreckerProbe {
    Vector3 field_0x00;        // point in the node's space
    UnknownWreckerSkeleton* field_0x0c;
    int field_0x10;            // nonzero while the probe touches something
    Vector3 field_0x14;        // contact normal
};

class UnknownWreckerCollisionObject : public UnknownWreckerQuadTreeObject, public GraphicsTest {
public:
    // 0x00439410 (thiscall, ret 4): appends `owner` to the ignore list.
    void UnknownFunction439410(UnknownWreckerCollisionObject* owner);
    void UnknownFunction435fe0();                      // 0x00435fe0

    unsigned char field_0x40[0x58 - 0x40];  // GraphicsTest subobject ends at +0x40
    int field_0x58;
    unsigned char field_0x5c[0x6c - 0x5c];
    int field_0x6c;
    int field_0x70;
    unsigned char field_0x74[0x84 - 0x74];
    int field_0x84;            // quadtree cell, passed to QuadTree::Remove
};

class UnknownWreckerRigidBody;

class UnknownWreckerCollisionModel : public UnknownWreckerCollisionObject {
public:
    explicit UnknownWreckerCollisionModel(int flags);              // 0x0043b8d0
    // 0x0043b9e0 (thiscall, ret 0xc): binds the model to `body`; `name`
    // is the collision file Bike passes ("rider.col").
    void UnknownFunction43b9e0(UnknownWreckerRigidBody* body, int value, const char* name);
    // 0x0043c7f0 (thiscall, ret 0x10): adds a probe at `offset` on `bone`.
    void UnknownFunction43c7f0(Vector3 offset, int bone);

    unsigned char field_0x88[0xe8 - 0x88];
    int field_0xe8;
    int field_0xec;
    float field_0xf0;
    float field_0xf4;
    float field_0xf8;
    unsigned char field_0xfc[0x100 - 0xfc];
    int field_0x100;           // probe count
    UnknownWreckerProbe* field_0x104;
    int field_0x108;
};

// Global collision quadtree (0x0068aba4, VisibilityQuadTree's constructor
// installs it). 0x004dcf20 (thiscall, ret 8) removes an object from a cell.
class UnknownWreckerQuadTree {
public:
    void UnknownFunction4dcf20(UnknownWreckerCollisionObject* object, int cell);
};
extern UnknownWreckerQuadTree* g_UnknownGlobal68aba4;

// A 4x4 transform read as four Vector3 rows (right, up, forward,
// position). Retail copies these rows with integer moves, which is how VC6
// copies a Vector3 struct (a Matrix4 element-wise copy would use the FPU).
struct UnknownWreckerFrame {
    Vector3 right;
    float field_0x0c;
    Vector3 up;
    float field_0x1c;
    Vector3 forward;
    float field_0x2c;
    Vector3 position;
    float field_0x3c;
};

// Scene-graph node (0x004fxxxx methods; the rider's +0x1a0, the rigid
// body's +0x234 and Wrecker+0xb8 all point at one). Names provisional.
struct UnknownWreckerSkeleton {
    void UnknownFunction4fc690(UnknownWreckerSkeleton* frame, Vector3 delta); // 0x004fc690 (ret 0x10)
    void UnknownFunction4fc740(UnknownWreckerSkeleton* frame, Vector3* position); // 0x004fc740 (ret 8)
    void UnknownFunction4fc9a0(UnknownWreckerSkeleton* frame, Vector3* position); // 0x004fc9a0 (ret 8)
    void UnknownFunction4fca80(UnknownWreckerSkeleton* frame, UnknownWreckerFrame* out);        // 0x004fca80 (ret 8)
    void UnknownFunction4fcc70();                                  // 0x004fcc70
    Vector3 UnknownFunction4fd660(const Vector3& point);           // 0x004fd660 (ret 8)
    Vector3 UnknownFunction4fd710(const Vector3& direction);       // 0x004fd710 (ret 8)
    Vector3 UnknownFunction4fd7f0(const Vector3& point);           // 0x004fd7f0 (ret 8)
    void UnknownFunction4fe850(Vector3* a, Vector3* b);            // 0x004fe850 (ret 8)
    unsigned char field_0x000[0x140];
    UnknownWreckerSkeleton* field_0x140;
    unsigned char field_0x144[0x18c - 0x144];
    int field_0x18c;
};

struct UnknownWreckerRaceState {
    unsigned char field_0x00[0xa8];
    int field_0xa8;
    unsigned char field_0xac[0xb4 - 0xac];
    int field_0xb4;
};

// Object at Wrecker+0x34 (Bike passes its rider character). Its +0x1a0 is
// the skeleton the probes and the wreck pose are attached to.
class UnknownWreckerCharacter {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7(float frameTime, int a, int b);

    void UnknownFunction4a8ac0(Vector3* position, int value);      // 0x004a8ac0 (ret 8)
    void UnknownFunction4a8b10(const char* animation);             // 0x004a8b10 (ret 4)
    void UnknownFunction4a8b40(int value);                         // 0x004a8b40 (ret 4)
    void UnknownFunction4a8b80(int value, float blend);            // 0x004a8b80 (ret 8)

    unsigned char field_0x004[0x18 - 0x4];
    UnknownWreckerRaceState* field_0x18;
    int* field_0x1c;
    unsigned char field_0x020[0x34 - 0x20];
    int field_0x34;
    unsigned char field_0x038[0x190 - 0x38];
    Vector3 field_0x190;
    unsigned char field_0x19c[0x1a0 - 0x19c];
    UnknownWreckerSkeleton* field_0x1a0;
};

// Object at Wrecker+0x40: RTTI PhysicsRigidBody (new 0x2a8 at line 0x5f,
// constructor 0x004cc120). Slots past GameObject's 27 are called by number.
class UnknownWreckerRigidBody : public GameObject {
public:
    explicit UnknownWreckerRigidBody(int flags);                   // 0x004cc120
    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28(UnknownWreckerSkeleton* skeleton);
    virtual void UnknownVirtualSlot29(float value);
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual void UnknownVirtualSlot32();
    virtual void UnknownVirtualSlot33(Vector3* value);
    virtual void UnknownVirtualSlot34(Vector3* value);
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

    unsigned char field_0x02c[0x13c - 0x2c];
    Vector3 field_0x13c;
    Vector3 field_0x148;
    Vector3 field_0x154;
    Vector3 field_0x160;
    Vector3 field_0x16c;
    float field_0x178;
    unsigned char field_0x17c[0x190 - 0x17c];
    Matrix4 field_0x190;
    Matrix4 field_0x1d0;
    unsigned char field_0x210[0x228 - 0x210];
    Vector3 field_0x228;
    UnknownWreckerSkeleton* field_0x234;
    unsigned char field_0x238[0x2a4 - 0x238];
    int field_0x2a4;
};

// Object at Wrecker+0x3c: RTTI SelectiveGravityModel (new 0x3c at line
// 0x5b, constructor 0x004f9760).
class UnknownWreckerGravityModel : public GameObject {
public:
    explicit UnknownWreckerGravityModel(int flags);                // 0x004f9760
    virtual void UnknownVirtualSlot27(UnknownWreckerRigidBody* body);
    unsigned char field_0x02c[0x38 - 0x2c];
    int field_0x38;
};

// Particle system at Wrecker+0x11c (Bike hands over its own; RTTI
// ParticleManager). 0x004baa50 (thiscall, ret 0x20) adds one particle.
class UnknownWreckerParticles {
public:
    int UnknownFunction4baa50(float age, const Vector3* position, int frame, float size,
                              float lifetime, float growth, unsigned int flags, int color);
};

// 0x00460b50 (cdecl): table-driven square root (FollowCamera.h declares the
// same function).
float UnknownFunction460b50(float value);

// 0x005015b0 (cdecl): the out-of-line copy of a vector scale (see
// src/krusty2/math/Math3D.h, Vec3ScaleCall).
Vector3 UnknownFunction5015b0(const Vector3& v, float scale);


class Wrecker : public GraphicsTest {
public:
    explicit Wrecker(int flags);                       // 0x0052ff90
    virtual ~Wrecker();                                // 0x005300a0 / 0x005300c0
    // Slot 10, 0x005306e0 (about 4.2 KB): the per-frame wreck update; not
    // reconstructed (see docs/WRECKER.md).
    virtual int UnknownVirtualSlot10(float frameTime);
    // Slot 14, 0x004de400: `jmp GameObject slot 14`. The linker folded
    // this body with identical ones of other classes, so the copy that
    // survives lies outside wrecker.cpp's code.
    virtual int UnknownVirtualSlot14();
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00532890

    // 0x00530190: slot 8 on the owner, then 0x005301c0; returns this.
    Wrecker* UnknownFunction530190(void* owner, UnknownWreckerCharacter* character, const char* name);
    void UnknownFunction5301c0(UnknownWreckerCharacter* character, const char* name);
    void UnknownFunction5305b0(int value);             // appends to field_0x58
    void UnknownFunction5305f0(int value);             // appends to field_0x50
    int UnknownFunction530630(int value);              // appends to field_0x48, returns its index
    void UnknownFunction530680(UnknownWreckerCollisionObject* object); // appends to field_0x60
    void UnknownFunction5327c0();
    void UnknownFunction5328b0(UnknownWreckerSkeleton* frame, Vector3 center, Vector3 halfExtents);
    void UnknownFunction5329e0(float frameTime);
    void UnknownFunction532900(Vector3 offset, int bone);
    void UnknownFunction531740(Vector3 a, Vector3 b, Vector3 normal, float frameTime);
    void UnknownFunction531da0(Vector3 a, Vector3 b, float frameTime);
    void UnknownFunction532020(float frameTime);
    void UnknownFunction532150();
    void UnknownFunction532220(int index);
    void UnknownFunction532310();
    void UnknownFunction532490();
    void UnknownFunction532580();

    UnknownWreckerCharacter* field_0x34;
    UnknownWreckerCollisionModel* field_0x38;
    UnknownWreckerGravityModel* field_0x3c;
    UnknownWreckerRigidBody* field_0x40;
    int field_0x44;            // wreck state (0 idle, 1..3)
    int* field_0x48;
    int field_0x4c;
    int* field_0x50;
    int field_0x54;
    int* field_0x58;
    int field_0x5c;
    UnknownWreckerCollisionObject** field_0x60;
    int field_0x64;
    float field_0x68;
    UnknownWreckerFrame field_0x6c; // last pose of field_0xb8
    int field_0xac;
    int field_0xb0;
    int field_0xb4;
    UnknownWreckerSkeleton* field_0xb8;
    Vector3 field_0xbc;
    Vector3 field_0xc8;
    int field_0xd4;
    int field_0xd8;
    Vector3 field_0xdc;
    Vector3 field_0xe8;
    Vector3 field_0xf4;
    UnknownWreckerSkeleton* field_0x100; // space of the box field_0xe8/field_0xf4
    Vector3* field_0x104;      // one per collision-model probe
    int* field_0x108;          // one per collision-model probe
    Vector3 field_0x10c;
    float field_0x118;
    UnknownWreckerParticles* field_0x11c;
    int field_0x120;           // particle frame, cycles 0..12
    int field_0x124;
    float field_0x128[256];    // random values in [0, 1)
    int field_0x528;
    int field_0x52c;
    int field_0x530;
    int field_0x534;
    int field_0x538;
};

typedef char wrecker_assert_sizeof[(sizeof(Wrecker) == 0x53c) ? 1 : -1];

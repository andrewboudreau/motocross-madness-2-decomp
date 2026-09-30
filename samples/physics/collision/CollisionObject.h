// CollisionObject -- reconstructed class layout (CollisionObject.cpp).
//
// Evidence (tier 1 unless noted), all from analysis/*.json and target bytes:
//  * RTTI direct bases: QuadTreeObject (offset 0) and GraphicsTest (offset 12);
//    GraphicsTest : GameObject with plain (non-virtual) inheritance
//    (BCD mdisp=12 pdisp=-1).  vtables: 0x005511b8 @0 (2 slots, QuadTreeObject's),
//    0x00551148 @12 (27 slots, GraphicsTest/GameObject shape).
//  * ctor 0x00431e70 writes vptr 0x5511c4 (QuadTreeObject), then the GraphicsTest
//    base ctor 0x0047bc70 on this+12, then 0x5511b8 / 0x551148.
//  * sizeof == 0xb8: operator new(0xb8, "CollisionCharacter.cpp", 27) at 0x00431a1d.
//  * Overrides of the secondary vtable are compiled with `this` == the GraphicsTest
//    subobject (this+12); VC6 does this natively for overrides of a second base
//    (verified with a scratch MI class), so the natural source shape matches.
//
// Member names are field_0xNN (object-relative offsets) unless the arithmetic
// clearly shows the meaning; semantic names are tier 3 (provisional).
#ifndef COLLISION_OBJECT_H
#define COLLISION_OBJECT_H

#include "CollisionTypes.h"
#include "../soultree_base/GraphicsTest.h"   // canonical BaseObject, GameObject, GraphicsTest

class CollisionModelSource;   // scene-graph node source used by shape setup 0x004324b0
struct CollisionHullBody;      // CollisionShapeTests.h
struct CollisionModelBody;
struct CollisionSweepQuery;
struct CollisionTreeNode;      // CollisionDebugDraw.cpp
struct Vec3;                   // ../common/Math3D.h
struct Matrix4;

// GraphicsTest (: GameObject, non-virtual) is declared once in ../soultree_base/GraphicsTest.h.
// No vbptr/vtordisp here; GameObject members are at their plain offsets from the GraphicsTest
// subobject at +12.

class QuadTreeObject {
public:
    virtual void UnknownVirtualSlot0();         // 0x004dc610 (ret)
    virtual int UnknownVirtualSlot1();          // 0x00434ce0 (returns 15)

    QuadTreeObject() { field_0x04 = 0; field_0x08 = (char)0xff; }
    short field_0x04;
    short field_0x06;                           // not touched by the inlined ctor; keeps field_0x08 at +8
    char field_0x08;
};

// Shape payloads hung off CollisionObject::field_0x54, discriminated by field_0x50.
// Sizes come from the allocation sizes in the shape setup functions; member names
// are provisional (tier 3).
struct CollisionHullShape {                // type 0, 0x198 bytes (0x004328b0, 0x00432720)
    int field_0x00;
    int field_0x04;
    char field_0x08[0x80];
    char field_0x88[0xc0];                 // filled by 0x0042cc60 / 0x0042c8c0
    CollisionMatrix4 field_0x148;          // copy of the matrix returned by 0x004a1410
    void* field_0x188;                     // freed with 0x0042a160
    void* field_0x18c;                     // freed with 0x0042a160
    CollisionVec3* field_0x190;            // vertex array (0x24-byte tagged block)
};

struct CollisionCapsuleShape {             // type 3, 0x68 bytes (0x00432a20)
    CollisionVec3 p0;                      // arguments 0..2
    CollisionVec3 p1;                      // arguments 3..5
    float radius;                          // argument 6
    float radiusSquared;
    float field_0x20;                      // 1.0f
    float field_0x24;                      // 1.0f
    CollisionMatrix4 field_0x28;           // identity
};

struct CollisionSphereShape {              // type 4, 0x5c bytes (0x004329a0)
    CollisionVec3 center;
    float radius;
    float radiusSquared;
    float field_0x14;                      // 1.0f
    float field_0x18;                      // 1.0f
    CollisionMatrix4 field_0x1c;           // identity
};

// Code pointers at CollisionObject+0x88/+0x8c (tier 1: 0x0043b9a0 stores the addresses
// 0x0043b800 and 0x00464e90 there).  The argument list comes from the one known target,
// 0x0043b800 (cdecl: this object, the other object); tier 3.
class CollisionObject;
typedef void (*CollisionCallback)(CollisionObject* self, CollisionObject* other);

class CollisionObject : public QuadTreeObject, public GraphicsTest {
public:
    CollisionObject(int a);                     // 0x00431e70
    virtual ~CollisionObject();                 // slot 0 @12: 0x00431fd0 -> core 0x00432000
    virtual int GameObjectVirtualSlot10(float dt);    // 0x00499ae0
    virtual int GameObjectVirtualSlot14();            // 0x00434540 (draw, then returns GameObject slot 14; ConstraintMethodCollisionModel overrides it again)
    virtual int GameObjectVirtualSlot23(int a, int b);  // 0x00434970 (ret 8, returns GameObject slot 23)

    // Non-virtual members (this == complete object).
    void FreeShape();                                        // 0x00432430
    void Fn_004320f0(int a, int b, int c, int d);            // 0x004320f0
    void Fn_00432120(int a);                                 // 0x00432120
    void SetSphereShape(CollisionVec3 center, float radius); // 0x004329a0 (type 4)
    void SetCapsuleShape(CollisionVec3 p0, CollisionVec3 p1, float radius);  // 0x00432a20 (type 3)
    void Fn_004324b0(void* node, int a, int b, int c, int d); // 0x004324b0, shape setup from a node
    void Fn_00432720(void* node, int a, int b, int c, int d); // 0x00432720 (ret 0x14)
    void Fn_00432800(void* node, const char* path);           // 0x00432800, shape from a .col file path
    void Fn_00432ab0(int count, void* points);                // 0x00432ab0 (ret 8), polyline shape setter
    void Fn_00435fb0();                                       // 0x00435fb0
    void Fn_00435fe0();                                       // 0x00435fe0
    int Fn_00438e70();                                        // 0x00438e70
    // 0x00435830, thiscall, ret 4 (tier 1): switches on the shape type field_0x50 (0..4,
    // jump table 0x435ea8), copies the 16 floats of *m into the shape payload at
    // field_0x54 and derives values from them. Name tier 3. Callers: ObjectPlacement
    // 0x004b0df0 and Tire 0x00514550.
    void SetTransform(const Matrix4* m);
    // 0x00439400, thiscall, ret 4: field_0x74 = v (tier 1).
    void SetField_0x74(int v);
    // 0x00439410, thiscall, ret 4 (tier 1 body): unique-add of owner into the growable
    // pointer array field_0x78 (data) / field_0x7c (count): returns if already present,
    // reuses a NULL slot, else reallocs through 0x004a2ec0 (line 0x843) and appends.
    // Name tier 3.
    void AddIgnoredOwner(void* owner);
    // 0x004394d0 (ret 8): stores field_0x60 and field_0x64 (tier 1); name tier 3.
    void SetOwner(void* owner, int tag);

    // Narrow phase (CollisionShapeTests.cpp, names tier 3).  These are thiscall members that
    // mostly ignore `this`: the 0x00438b90 dispatchers forward ecx unchanged down the chain.
    void GetWorldBounds(CollisionVec3* outMin, CollisionVec3* outMax);   // 0x00436100
    int TestAgainst(CollisionObject* other);                             // 0x00436430
    int HullVsHull(CollisionHullBody* a, CollisionHullBody* b, CollisionSweepQuery* q);        // 0x00436720
    int HullVsHullSwept(CollisionHullBody* a, CollisionHullBody* b, CollisionSweepQuery* q);   // 0x00436af0
    int HullVsModel(CollisionHullBody* a, CollisionModelBody* b, CollisionSweepQuery* q);      // 0x00436e50
    int HullVsModelSwept(CollisionHullBody* a, CollisionModelBody* b, CollisionSweepQuery* q); // 0x004376f0
    int HullVsSphere(CollisionHullBody* hull, CollisionSphereShape* sphere);                   // 0x004379c0
    int HullVsCapsule(CollisionHullBody* hull, CollisionCapsuleShape* capsule);                // 0x00437aa0
    int ModelVsHull(CollisionModelBody* a, CollisionHullBody* b, CollisionSweepQuery* q);      // 0x00437c20
    int ModelVsHullSwept(CollisionModelBody* a, CollisionHullBody* b, CollisionSweepQuery* q); // 0x00437ef0
    int ModelVsModel(CollisionModelBody* a, CollisionModelBody* b, CollisionSweepQuery* q);    // 0x00438280
    int ModelVsModelSwept(CollisionModelBody* a, CollisionModelBody* b, CollisionSweepQuery* q); // 0x00438550
    int ModelVsSphere(CollisionModelBody* model, CollisionSphereShape* sphere);                // 0x00438860
    int ModelVsCapsule(CollisionModelBody* model, CollisionCapsuleShape* capsule);             // 0x004389b0
    int TestHullAgainst(CollisionObject* other);                         // 0x00438b90 (this is type 0)
    int TestModelAgainst(CollisionObject* other);                        // 0x00438c10 (this is type 1)
    int Fn_004392c0(CollisionObject* other);                             // 0x004392c0 (bounds test used by TestMeshAgainst)
    int TestMeshAgainst(CollisionObject* other);                         // 0x00438c90 (this is type 2)

    // Debug drawing of the bounding-volume trees (CollisionDebugDraw.cpp, names tier 3).
    // DrawHull / DrawModel pass a hull's tree (+0x188 triangles or +0x18c points), its
    // transform (+0x48), the second matrix (+0x08) and its vertex array (+0x190).
    void DrawTree(CollisionTreeNode* node, int depth, const Matrix4* xf,
                  const Vec3* verts);                                    // 0x00433240
    void DrawTreeMotion(CollisionTreeNode* node, int depth, const Matrix4* xf,
                        const Matrix4* motion, const Vec3* verts);       // 0x004334c0
    void DrawPointTree(CollisionTreeNode* node, const Matrix4* xf,
                       const Matrix4* motion);                           // 0x00433930
    void DrawTreeNormals(CollisionTreeNode* node, const Vec3* verts,
                         const Matrix4* xf);                             // 0x00433be0
    void DrawBoxTree(CollisionTreeNode* node, int depth, const Matrix4* xf); // 0x00434040
    void DrawHull(CollisionHullBody* hull, int depth, int mode);         // 0x00432d30
    void DrawModel(CollisionModelBody* model, int depth, int mode);      // 0x00432b30

    char field_0x44[0xc];                       // own bytes 0x44..0x4f, not accessed by any target
    int field_0x50;                             // shape type 0..4
    void* field_0x54;                           // shape payload (see shape structs)
    int field_0x58;
    void* field_0x5c;                           // contact record pointer (constraint solver)
    void* field_0x60;                           // owner: SoulTreePhysics slot 40 loaders store their `this`,
                                                // ConstraintMethodCollisionModel stores itself (tier 2)
    int field_0x64;                             // type tag; 0x3ea (1002) = has a body at +0xc4
    int field_0x68;
    int field_0x6c;
    int field_0x70;
    int field_0x74;
    int field_0x78;
    int field_0x7c;
    int field_0x80;
    int field_0x84;
    CollisionCallback field_0x88;               // 0x0043b9a0 stores 0x0043b800
    CollisionCallback field_0x8c;               // 0x0043b9a0 stores 0x00464e90 (empty function)
    int field_0x90;
    int field_0x94;
    int field_0x98;
    int field_0x9c;
    CollisionVec3 field_0xa0;
    CollisionVec3 field_0xac;
};

typedef char collision_object_assert_sizeof[(sizeof(CollisionObject) == 0xb8) ? 1 : -1];

#endif

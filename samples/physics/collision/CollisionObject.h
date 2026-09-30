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

class CollisionModelSource;   // scene-graph node source used by shape setup 0x004324b0

// Provisional stub of BaseObject/GameObject: 27 virtual slots, exactly the vtable
// shape read from the GraphicsTest vtable 0x00553de4.  Signatures are tier 3
// (argument counts come from the `ret N` of the shared default implementations).
class GameObject {
public:
    virtual ~GameObject();                      // slot 0
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8(int a);
    virtual void UnknownVirtualSlot9(int a);
    virtual void UnknownVirtualSlot10(int a);
    virtual int UnknownVirtualSlot11(int a);
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();        // draw (CollisionObject/Tire override)
    virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16(int a);
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19(int a);
    virtual void UnknownVirtualSlot20(int a);
    virtual void UnknownVirtualSlot21(int a);
    virtual void UnknownVirtualSlot22(int a, int b);
    virtual void UnknownVirtualSlot23(int a, int b);
    virtual void UnknownVirtualSlot24(int a, int b, int c, int d, int e);
    virtual void UnknownVirtualSlot25(int a);
    virtual void UnknownVirtualSlot26();

    int field_0x04[5];
    void* field_0x18;                           // object with a vtable, called by slot 14
    char field_0x1c[0x28];
};

class GraphicsTest : public GameObject {
public:
    GraphicsTest(int a);
    virtual ~GraphicsTest();
    virtual void UnknownVirtualSlot10(int a);
    // Non-virtual GraphicsTest methods used by CollisionObject::UnknownVirtualSlot14.
    void Fn_0047c6c0(int a, int b, int c, int d);
    void Fn_0047c6f0();
    void Fn_0047c0b0(void* a, int b, int c);
    void Fn_00469ce0(GraphicsTest* owner);   // 0x00469ce0, registers the object (thiscall, 1 arg)
};

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

class CollisionObject : public QuadTreeObject, public GraphicsTest {
public:
    CollisionObject(int a);                     // 0x00431e70
    virtual ~CollisionObject();                 // slot 0 @12: 0x00431fd0 -> core 0x00432000
    virtual void UnknownVirtualSlot10(int a);   // 0x00499ae0
    virtual void UnknownVirtualSlot14();        // 0x00434540
    virtual void UnknownVirtualSlot23(int a, int b); // 0x00434970

    // Non-virtual members (this == complete object).
    void FreeShape();                                        // 0x00432430
    void Fn_004320f0(int a, int b, int c, int d);            // 0x004320f0
    void Fn_00432120(int a);                                 // 0x00432120
    void SetSphereShape(CollisionVec3 center, float radius); // 0x004329a0 (type 4)
    void SetCapsuleShape(CollisionVec3 p0, CollisionVec3 p1, float radius);  // 0x00432a20 (type 3)

    int field_0x50;                             // shape type 0..4
    void* field_0x54;                           // shape payload (see shape structs)
    int field_0x58;
    void* field_0x5c;
    int field_0x60;
    int field_0x64;
    int field_0x68;
    int field_0x6c;
    int field_0x70;
    int field_0x74;
    int field_0x78;
    int field_0x7c;
    int field_0x80;
    int field_0x84;
    int field_0x88;
    int field_0x8c;
    int field_0x90;
    int field_0x94;
    int field_0x98;
    int field_0x9c;
    CollisionVec3 field_0xa0;
    CollisionVec3 field_0xac;
};

#endif

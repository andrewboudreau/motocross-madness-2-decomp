// SoultreePhysicsBaseObject -- reconstructed class layout (SoulTreePhysics.cpp).
//
// Evidence (tier 1 unless noted):
//  * RTTI: direct base GameObject, which is a *virtual* base (BCD attributes 0x10,
//    pdisp=4/vdisp=4 in analysis/rtti_classes.json).  The class therefore owns a
//    vfptr at +0 and a vbptr at +4; the GameObject subobject lives at +0x220 (544).
//  * COL 0x0055f0d0 / vtable 0x00557d90 at object offset 0 (40 slots, all of them
//    declared by this class), COL 0x0055f098 / vtable 0x00557d20 at offset 544 is
//    the GameObject virtual-base vtable (slot 0 = vtordisp deleting-destructor
//    thunk 0x00504280, slot 10 = thunk 0x005042d0 -> 0x005036f0).
//  * ctor 0x00500aa0 writes vptr 0x00557d90 to +0, vbtable 0x00557e30 to +4 and
//    the 0x00557d20 vptr into the virtual base; sizeof the non-virtual part is
//    0x21c (derived classes place their next base at 540).
//  NOTE: analysis/vtable_overrides.json labels many of the 40 primary slots as
//  "overrides" of GameObject slots.  That is an artifact of comparing slot
//  numbers across unrelated vtables; slot 0 here is a float-taking method, not a
//  destructor.  We treat all 40 slots as introduced by this class.
//
// Member names are field_0xNN unless the arithmetic clearly shows the meaning;
// semantic names are tier 3 (provisional).
#ifndef SOULTREE_PHYSICS_BASE_OBJECT_H
#define SOULTREE_PHYSICS_BASE_OBJECT_H

#include "SoultreePhysicsTypes.h"

class SoultreeNode;       // pointed to by field_0x08 (scene node / transform owner)
class SoultreeContact;    // elements of the field_0x12c array
class SoultreeBody;       // pointed to by field_0x128
class SoultreeSlot1f0;
class SoultreeAttachTarget;
class SoultreeAttachment; // 40-byte records in the field_0x1d4 array

#include "GameObject.h"   // BaseObject, GameObject

class SoultreePhysicsBaseObject : public virtual GameObject {
public:
    virtual ~SoultreePhysicsBaseObject();       // deleting dtor 0x00504290 via vbase vtable slot 0
    virtual int GameObjectVirtualSlot10(float dt); // override, thunk 0x005042d0 -> 0x005036f0
    // --- vtable 0x00557d90 (offset 0), slots 0..39, all introduced here ---
    virtual void UnknownVirtualSlot0(float value);
    virtual void UnknownVirtualSlot1(float value);
    virtual void UnknownVirtualSlot2(
        int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9,
        int a10, int a11, int a12, int a13, int a14, int a15, int a16, int a17,
        int a18, int a19, int a20, int a21, int a22, int a23, int a24, int a25,
        int a26);
    // Slot 3 (ret 0x1c).  Pointer types are tier 2: Vehicle callers (slots 38, 49) pass
    // Vec3 addresses for a1..a4; a5 is the event code KrustyBike 0x0048dbf0 compares with
    // 1000 and 0x67.  a7 is the address of the same local whose value Vehicle slot 38
    // passes as slot 4's a5 (the other body's field_0x24 float), hence float* (tier 2).
    virtual void UnknownVirtualSlot3(const SoultreeVec3* a1, const SoultreeVec3* a2,
                                     const SoultreeVec3* a3, const SoultreeVec3* a4,
                                     int a5, int a6, float* a7);
    // Slot 4 (ret 0x3c).  a2..a4/a8..a10 are dereferenced as Vec3 by this body (tier 1);
    // a1, a11, a12 receive Vec3 addresses at the Vehicle slot 38 call site (tier 2).  That
    // caller passes the other body's field_0x24 (a float, 1/field_0x158) as a5 and the
    // address of the local holding it as a14; KrustyBike 0x0048dc60 reads *a14 (tier 2).
    virtual void UnknownVirtualSlot4(const SoultreeVec3* a1, SoultreeVec3* a2, const SoultreeVec3* a3,
                                     const SoultreeVec3* a4, float a5, int a6, int a7,
                                     const SoultreeVec3* a8, const SoultreeVec3* a9,
                                     const SoultreeVec3* a10, SoultreeVec3* a11, SoultreeVec3* a12,
                                     int a13, float* a14, float a15);
    virtual int UnknownVirtualSlot5(int value);
    virtual void UnknownVirtualSlot6(SoultreeVec3* a, float* b);
    virtual void UnknownVirtualSlot7(const SoultreeVec3* a);  // only read (0x005019e0, tier 2)
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9(float dt, int* steps);
    virtual int UnknownVirtualSlot10();
    // Slot 11 (ret 0x14): returns the 0x004b0df0 result (tier 1).  a2..a4 receive Vec3
    // addresses and a5 an int address at the Vehicle slot 49 call site (tier 2).
    virtual int UnknownVirtualSlot11(int a1, SoultreeVec3* a2, SoultreeVec3* a3, SoultreeVec3* a4,
                                     int* a5);
    virtual int UnknownVirtualSlot12(int value);
    virtual void UnknownVirtualSlot13(SoultreeVec3* a, SoultreeVec3* b, float c);
    // Slot 14 (ret 0xc): retail 0x00502080 passes a2 to slot 16, a3 to slot 15 and later
    // dereferences a1 (tier 1 decoded stack offsets); all three are Vec3 pointers.
    // a1 is not const: the KrustyBike override 0x004965e0 zeroes a1->x and a1->z (tier 1).
    virtual void UnknownVirtualSlot14(SoultreeVec3* a1, const SoultreeVec3* a2,
                                      const SoultreeVec3* a3);
    virtual void UnknownVirtualSlot15(const SoultreeVec3* a, SoultreeVec3* b);
    virtual SoultreeVec3 UnknownVirtualSlot16(const SoultreeVec3* a);
    virtual SoultreeVec3 UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18(SoultreeAttachment* a);
    virtual void UnknownVirtualSlot19(SoultreeAttachment* a);
    virtual void UnknownVirtualSlot20(SoultreeAttachment* a);
    virtual void UnknownVirtualSlot21();
    virtual int UnknownVirtualSlot22();
    virtual int UnknownVirtualSlot23();
    virtual int UnknownVirtualSlot24();
    virtual int UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
    virtual void UnknownVirtualSlot27();
    virtual int UnknownVirtualSlot28(int a);
    virtual void UnknownVirtualSlot29(int a);
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual float UnknownVirtualSlot32();
    virtual int UnknownVirtualSlot33(const SoultreeVec3* a1, const SoultreeVec3* a2,
                                     const SoultreeVec3* a3, const SoultreeVec3* a4, int a5,
                                     float a6);  // a4 unused here; Vehicle passes a Vec3 address (tier 2)
    // Slots 34/35 are declared void: 0x004aa1c0/0x004aa1e0 end in `call; ret` with no use
    // of eax, identical for either return type; the Vehicle overrides 0x0040c4c0 and
    // 0x0040c540 return nothing (tier 2).
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35(int a, int b);
    virtual void UnknownVirtualSlot36();
    virtual SoultreeAttachment* UnknownVirtualSlot37(int type, void* a2, SoultreeAttachTarget* a3, const SoultreeVec3* v);
    // Slot 38 (ret 0xc): 0x00501600 switches on a2 (0x66/0x68/0x69/0x6a/0x2711) and reads
    // a3->+0x60 as the other body (tier 1); a3's type is provisional.
    virtual void UnknownVirtualSlot38(int a1, int a2, void* a3);
    // Slot 39 (ret 4): argument unused here; Vehicle slot 49 0x0052a940 passes it the same
    // dword it passes to slot 9 (float dt), so float (tier 2).
    virtual int UnknownVirtualSlot39(float dt);

    // --- data members (offsets confirmed by decoded accesses; names provisional) ---
    // vfptr at +0, vbptr at +4 (compiler generated)
    SoultreeNode* field_0x08;           // callee of 0x4fc630/0x4fc970/... (node/transform owner)
    SoultreeVec3 field_0x0c;
    SoultreeVec3 field_0x18;            // position (tier 3): subtracted from contact points
    float field_0x24;                   // 1 / field_0x158 (slot 0)
    float field_0x28;                   // ctor: 1.0f
    float field_0x2c;                   // 0x2c..0x44: filled by 0x4b5a60(...), previous
    float field_0x30;                   //   copy at 0x48..0x60 (slots 1, 33, 36, 7 tail)
    float field_0x34;
    float field_0x38;
    float field_0x3c;
    float field_0x40;
    float field_0x44;
    float field_0x48;
    float field_0x4c;
    float field_0x50;
    float field_0x54;
    float field_0x58;
    float field_0x5c;
    float field_0x60;
    SoultreeVec3 field_0x64;
    SoultreeVec3 field_0x70;
    SoultreeVec3 field_0x7c;
    SoultreeVec3 field_0x88;
    SoultreeVec3 field_0x94;
    SoultreeVec3 field_0xa0;
    SoultreeVec3 field_0xac;
    float field_0xb8;
    float field_0xbc;                   // |field_0x64| (slots 3, 4, 14)
    SoultreeVec3 field_0xc0;
    SoultreeVec3 field_0xcc;
    SoultreeVec3 field_0xd8;
    SoultreeVec3 field_0xe4;
    SoultreeVec3 field_0xf0;
    char field_0xfc[12];
    char field_0x108;
    char field_0x109;
    char field_0x10a;
    SoultreeVec3 field_0x10c;
    SoultreeVec3 field_0x118;
    void* field_0x124;
    SoultreeBody* field_0x128;
    SoultreeContact** field_0x12c;      // array of field_0x130 pointers
    int field_0x130;
    int field_0x134;
    char field_0x138;
    float field_0x13c;
    float field_0x140;
    float field_0x144;
    float field_0x148;
    float field_0x14c;
    float field_0x150;
    float field_0x154;
    float field_0x158;
    float field_0x15c;
    float field_0x160;
    float field_0x164[9];
    SoultreeVec3 field_0x188;
    SoultreeVec3 field_0x194;
    SoultreeVec3 field_0x1a0;
    SoultreeVec3 field_0x1ac;
    SoultreeVec3 field_0x1b8;
    int field_0x1c4;
    int field_0x1c8;
    int field_0x1cc;
    char field_0x1d0;
    SoultreeAttachment* field_0x1d4;    // array of 40-byte records
    int field_0x1d8;                    // capacity
    int field_0x1dc;                    // count
    float field_0x1e0;
    float field_0x1e4;
    float field_0x1e8;
    int field_0x1ec;
    SoultreeSlot1f0* field_0x1f0;
    int field_0x1f4;
    float field_0x1f8;                  // fld/fmul in KrustyBike slot 12 (0x0048de20), tier 1
    char field_0x1fc;
    void* field_0x200;
    int field_0x204;
    int field_0x208;
    char field_0x20c;
    char field_0x20d;
    char field_0x20e;
    unsigned char field_0x20f;
    int field_0x210;
    int field_0x214;
    SoultreeNode* field_0x218;
};

#endif

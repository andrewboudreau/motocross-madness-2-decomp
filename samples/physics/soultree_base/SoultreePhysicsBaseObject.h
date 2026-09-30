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

// BaseObject: RTTI .?AVBaseObject@@ (COL 0x0055aa38), vtable 0x005507c0 with 4 slots
// (0x00405130 scalar deleting dtor, 0x00405160, 0x00405170 Release, 0x00401940 returns
// the int at +4).  Ctor 0x00405120.  Minimum size 8 (analysis/class_layout_hints.json).
class BaseObject {
public:
    virtual ~BaseObject();
    virtual void BaseObjectVirtualSlot1();
    virtual void BaseObjectVirtualSlot2();
    virtual int BaseObjectVirtualSlot3();
    int field_0x04;
};

// GameObject: RTTI .?AVGameObject@@ (COL 0x0055c248), direct base BaseObject, vtable
// 0x00552a2c with 27 slots (slots 1 and 3 are inherited from BaseObject; 0, 2 and 4..26
// are GameObject's own).  Ctor 0x00468ca0 (D:ardvark\VC\krusty2\gameobj.cpp) writes
// fields +0x08..+0x28 and ends the object at 0x2c (tier 2: ctor/dtor field extent;
// the 0x28 field is an allocation of 0x28 bytes released by the dtor 0x00468d60).
// Only the slots SoultreePhysicsBaseObject overrides (0 = dtor, 10) are named; the rest
// are placeholders whose signatures are unknown.  This is a PROVISIONAL stub that
// exists so the virtual-base layout (vbptr, vtordisp, +0x220 subobject) is real.
class GameObject : public BaseObject {
public:
    virtual ~GameObject();
    virtual void BaseObjectVirtualSlot2();
    virtual void GameObjectVirtualSlot4();
    virtual void GameObjectVirtualSlot5();
    virtual void GameObjectVirtualSlot6();
    virtual void GameObjectVirtualSlot7();
    virtual void GameObjectVirtualSlot8();
    virtual void GameObjectVirtualSlot9();
    virtual void GameObjectVirtualSlot10();
    virtual void GameObjectVirtualSlot11();
    virtual void GameObjectVirtualSlot12();
    virtual void GameObjectVirtualSlot13();
    virtual void GameObjectVirtualSlot14();
    virtual void GameObjectVirtualSlot15();
    virtual void GameObjectVirtualSlot16();
    virtual void GameObjectVirtualSlot17();
    virtual void GameObjectVirtualSlot18();
    virtual void GameObjectVirtualSlot19();
    virtual void GameObjectVirtualSlot20();
    virtual void GameObjectVirtualSlot21();
    virtual void GameObjectVirtualSlot22();
    virtual void GameObjectVirtualSlot23();
    virtual void GameObjectVirtualSlot24();
    virtual void GameObjectVirtualSlot25();
    virtual void GameObjectVirtualSlot26();
    char field_0x08[0x24];
};

class SoultreePhysicsBaseObject : public virtual GameObject {
public:
    virtual ~SoultreePhysicsBaseObject();       // deleting dtor 0x00504290 via vbase vtable slot 0
    virtual void GameObjectVirtualSlot10();     // override, thunk 0x005042d0 -> 0x005036f0
    // --- vtable 0x00557d90 (offset 0), slots 0..39, all introduced here ---
    virtual void UnknownVirtualSlot0(float value);
    virtual void UnknownVirtualSlot1(float value);
    virtual void UnknownVirtualSlot2(
        int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9,
        int a10, int a11, int a12, int a13, int a14, int a15, int a16, int a17,
        int a18, int a19, int a20, int a21, int a22, int a23, int a24, int a25,
        int a26);
    virtual void UnknownVirtualSlot3(int a1, int a2, int a3, int a4, int a5, int a6, int a7);
    virtual void UnknownVirtualSlot4(int a1, SoultreeVec3* a2, const SoultreeVec3* a3,
                                     const SoultreeVec3* a4, int a5, int a6, int a7,
                                     const SoultreeVec3* a8, const SoultreeVec3* a9,
                                     const SoultreeVec3* a10, int a11, int a12, int a13,
                                     int a14, float a15);
    virtual int UnknownVirtualSlot5(int value);
    virtual void UnknownVirtualSlot6(SoultreeVec3* a, float* b);
    virtual void UnknownVirtualSlot7(SoultreeVec3* a);
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9(float dt, int* steps);
    virtual int UnknownVirtualSlot10();
    virtual int UnknownVirtualSlot11(int a1, int a2, int a3, int a4, int a5);
    virtual int UnknownVirtualSlot12(int value);
    virtual void UnknownVirtualSlot13(SoultreeVec3* a, SoultreeVec3* b, float c);
    virtual void UnknownVirtualSlot14(const SoultreeVec3* a1, const SoultreeVec3* a2, int a3);
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
                                     const SoultreeVec3* a3, int a4, int a5, float a6);
    virtual int UnknownVirtualSlot34();
    virtual int UnknownVirtualSlot35(int a, int b);
    virtual void UnknownVirtualSlot36();
    virtual SoultreeAttachment* UnknownVirtualSlot37(int type, void* a2, SoultreeAttachTarget* a3, const SoultreeVec3* v);
    virtual void UnknownVirtualSlot38(int a1, int a2, int a3);
    virtual int UnknownVirtualSlot39(int a);

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
    int field_0x1f8;
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

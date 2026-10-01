// GameObject.h -- BaseObject and GameObject (gameobj.cpp), split out of
// SoultreePhysicsBaseObject.h unchanged so that non-Soultree hierarchies (collision/
// constraint: CollisionObject -> GraphicsTest -> GameObject) can use the canonical classes
// without pulling in Math3D.h, whose static const Vec3 objects add dynamic initializers
// ($E symbols) that the retail translation units do not have.
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

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
// Slot names are placeholders (tier 3); the signatures carry the decoded ABI.  This is a
// PROVISIONAL class body that
// exists so the virtual-base layout (vbptr, vtordisp, +0x220 subobject) is real.
class GameObject : public BaseObject {
public:
    // 0x00468ca0 (thiscall, ret 4; tier 1 decoded body).  Bit 0 of the argument goes to bit 0
    // of field_0x25 and to bit 1 (shl 1); it also zeroes +0x08..+0x20, sets +0x24 to 0xff,
    // allocates the 0x28 block and registers the object (0x00469ce0).  Terrain's ctor
    // (0x00505830) forwards its own argument; CollisionCharacter (0x004318d0) passes 1 as the
    // virtual-base initializer.  Parameter name tier 3.  Declared explicit and without a
    // default constructor: every retail constructor seen so far passes the argument.
    explicit GameObject(int flags);
    virtual ~GameObject();
    virtual void BaseObjectVirtualSlot2();
    // Slot signatures below are tier 1 for the ABI (callee pop count, eax result) from the
    // GameObject bodies in vtable 0x00552a2c; parameter types beyond "4-byte value" and every
    // name are tier 3.  Most slots forward the call to the child list (+0x10, next +0x0c)
    // and return 1 (0 when a child returns 0).
    virtual void GameObjectVirtualSlot4();       // 0x004690a0: clears bits 0/1 of field_0x25, tail-calls slot 6
    virtual void GameObjectVirtualSlot5();       // 0x004690b0: sets bits 0/1 of field_0x25, tail-calls slot 7
    virtual void GameObjectVirtualSlot6();       // 0x00469050: calls slot 6 on each child, ret
    virtual void GameObjectVirtualSlot7();       // 0x00469070: calls slot 7 on each enabled child, ret
    // Slot 8: 0x004692f0 `mov eax,ecx; mov ecx,[esp+4]; mov [eax+0x18],ecx; ret 4` ->
    // stores its argument in field_0x18 and returns this (tier 1 decoded body).
    virtual GameObject* GameObjectVirtualSlot8(int a);
    virtual int GameObjectVirtualSlot9(int a);   // 0x00469430: ret 4, returns 1
    // Slots 10/11 take one dword (`ret 4` in 0x004693d0, 0x004da540 and every override:
    // 0x005036f0, 0x00504210, 0x0052a830, 0x004977a0).  Float time step and int result
    // are tier 2 (constraint solver 0x0043ba70/0x0043bdb0 use the argument as a float;
    // 0x004da540 is `mov eax,1; ret 4`).
    virtual int GameObjectVirtualSlot10(float dt);
    virtual int GameObjectVirtualSlot11(float dt);
    virtual int GameObjectVirtualSlot12();       // 0x00469300: ret, returns 1
    virtual int GameObjectVirtualSlot13();       // 0x00469330: ret, returns 1
    // 0x00469360: ret, returns 1, or 0 as soon as a child's slot 14 returns 0.  PhysicsBody's
    // override 0x004cbf10 is `call 0x00469360; mov eax,1; ret` (tier 1).
    virtual int GameObjectVirtualSlot14();
    virtual int GameObjectVirtualSlot15();       // 0x004693a0: ret, returns 1
    virtual int GameObjectVirtualSlot16(int a);  // 0x00469480: ret 4, returns 1
    virtual int GameObjectVirtualSlot17();       // 0x00469500: ret, returns 1
    virtual int GameObjectVirtualSlot18();       // 0x004694d0: ret, returns 1
    virtual int GameObjectVirtualSlot19(int a);  // 0x00469530: ret 4 (Terrain 0x00507920 returns 0)
    virtual int GameObjectVirtualSlot20(int a);  // 0x00469c00: ret 4
    virtual int GameObjectVirtualSlot21(int a);  // 0x00469c40: ret 4
    virtual int GameObjectVirtualSlot22(int a, int b);  // 0x00469580: ret 8 (Terrain 0x004dc4c0 returns 0)
    // 0x004695d0: ret 8, returns 1 when a child handles it.  Overrides: CollisionObject
    // 0x00434970, Terrain 0x00508850 (input event handler, tier 3).
    virtual int GameObjectVirtualSlot23(int a, int b);
    virtual int GameObjectVirtualSlot24(int a, int b, int c, int d, int e);  // 0x00469620: ret 0x14
    virtual int GameObjectVirtualSlot25(int a);  // 0x00469720: ret 4, returns 1
    virtual void GameObjectVirtualSlot26();      // 0x004692c0: ret, sets bit 3 of field_0x25
    // Non-virtual, this == the GameObject subobject (Bike slot 97 0x00409420 computes
    // `lea ecx,[vbase]` before calling it).  Argument types tier 3.
    void Method_0x00469190(void* a, int b);

    char field_0x08[0x10];
    void* field_0x18;                   // written by slot 8 (0x004692f0)
    char field_0x1c[9];
    unsigned char field_0x25;           // bit 0 tested by SoultreePhysicsBaseObject slot 21 and Vehicle
                                        // slot 38 through field_0x124 (tier 3: "enabled" flag)
    char field_0x26[6];                 // GameObject ends at 0x2c (ctor 0x00468ca0 extent)
};

#endif

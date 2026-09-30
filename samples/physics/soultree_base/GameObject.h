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
    // Slot 8: 0x004692f0 `mov eax,ecx; mov ecx,[esp+4]; mov [eax+0x18],ecx; ret 4` ->
    // stores its argument in field_0x18 and returns this (tier 1 decoded body).
    virtual GameObject* GameObjectVirtualSlot8(int a);
    virtual void GameObjectVirtualSlot9();
    // Slots 10/11 take one dword (`ret 4` in 0x004693d0, 0x004da540 and every override:
    // 0x005036f0, 0x00504210, 0x0052a830, 0x004977a0).  Float time step and int result
    // are tier 2 (constraint solver 0x0043ba70/0x0043bdb0 use the argument as a float;
    // 0x004da540 is `mov eax,1; ret 4`).
    virtual int GameObjectVirtualSlot10(float dt);
    virtual int GameObjectVirtualSlot11(float dt);
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
    virtual void GameObjectVirtualSlot23(int a, int b);  // 0x004695d0 ret 8; CollisionObject overrides (0x00434970)
    virtual void GameObjectVirtualSlot24();
    virtual void GameObjectVirtualSlot25();
    virtual void GameObjectVirtualSlot26();
    // Non-virtual, this == the GameObject subobject (Bike slot 97 0x00409420 computes
    // `lea ecx,[vbase]` before calling it).  Argument types tier 3.
    void Method_0x00469190(void* a, int b);

    char field_0x08[0x10];
    void* field_0x18;                   // written by slot 8 (0x004692f0)
    char field_0x1c[0x10];              // GameObject ends at 0x2c (ctor 0x00468ca0 extent)
};

#endif

// ConstraintBase.h -- LOCAL, PROVISIONAL copy of the CollisionObject class chain
// (see ../collision/CollisionObject.h, owned by the collision agent) with the slot
// signatures this translation unit needs. Kept local because the collision header
// declares GameObject slot 8 as `void` and slots 10/11 as `int a`, while
// ConstraintMethodCollisionModel.cpp needs:
//   * slot 8 returning a GameObject* (retail 0x0043b9a0 returns this ? this : 0 in eax;
//     the default 0x004692f0 is `mov eax,ecx; ... mov [eax+0x18],ecx`, i.e. it returns this),
//   * slots 10/11 taking a float time step (0x0043ba70 stores the argument at +0xc0 and
//     the solver 0x0043bdb0 multiplies it as a float).
// Layout facts (tier 1 unless noted) are the ones in CollisionObject.h: QuadTreeObject at
// 0 (vptr 0x005511c4 -> 0x005511b8), GraphicsTest at 12 (vptr 0x00551148), sizeof == 0xb8.
// Only the members this TU touches are named; the rest are anonymous padding.
#ifndef CONSTRAINT_BASE_H
#define CONSTRAINT_BASE_H

class CollisionModelSource;

class GameObject {
public:
    virtual ~GameObject();                                 // slot 0
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual GameObject* UnknownVirtualSlot8(int a);        // 0x004692f0: field_0x18 = a; return this
    virtual void UnknownVirtualSlot9(int a);
    virtual int UnknownVirtualSlot10(float dt);            // 0x004693d0
    virtual int UnknownVirtualSlot11(float dt);            // 0x004da540: return 1
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
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
    void* field_0x18;
    char field_0x1c[0x28];
};

class GraphicsTest : public GameObject {
public:
    GraphicsTest(int a);
    virtual ~GraphicsTest();
    virtual int UnknownVirtualSlot10(float dt);
};

class QuadTreeObject {
public:
    virtual void UnknownVirtualSlot0();                    // 0x004dc610 (ret)
    virtual int UnknownVirtualSlot1();                     // 0x00434ce0 (returns 15)

    QuadTreeObject() { field_0x04 = 0; field_0x08 = (char)0xff; }
    short field_0x04;
    short field_0x06;
    char field_0x08;
};

class CollisionObject : public QuadTreeObject, public GraphicsTest {
public:
    CollisionObject(int a);                                // 0x00431e70
    virtual ~CollisionObject();                            // slot 0 @12: 0x00431fd0 -> core 0x00432000
    virtual int UnknownVirtualSlot10(float dt);            // 0x00499ae0
    virtual void UnknownVirtualSlot14();                   // 0x00434540
    virtual void UnknownVirtualSlot23(int a, int b);       // 0x00434970

    void Fn_004320f0(int a, int b, int c, int d);          // 0x004320f0
    void Fn_004324b0(void* node, int a, int b, int c, int d);  // 0x004324b0, shape setup from a node
    void Fn_00432800(void* node, int a);                   // 0x00432800
    void Fn_00435fb0();                                    // 0x00435fb0
    void Fn_00435fe0();                                    // 0x00435fe0
    int Fn_00438e70();                                     // 0x00438e70

    int field_0x50;                                        // shape type 0..4
    void* field_0x54;                                      // shape payload
    int field_0x58;
    void* field_0x5c;                                      // contact record pointer (see solver)
    int field_0x60;
    int field_0x64;                                        // type tag; 0x3ea (1002) = has a body at +0xc4
    int field_0x68;
    int field_0x6c;
    int field_0x70;
    int field_0x74;
    int field_0x78;
    int field_0x7c;
    int field_0x80;
    int field_0x84;
    int field_0x88;                                        // callback (0x0043b9a0 stores 0x0043b800)
    int field_0x8c;                                        // callback (0x0043b9a0 stores 0x00464e90)
    int field_0x90;
    int field_0x94;
    int field_0x98;
    int field_0x9c;
    float field_0xa0[3];
    float field_0xac[3];
};

#endif

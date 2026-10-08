#pragma once

// CarProcedural.h -- the reconstructed part of
// D:\aardvark\VC\krusty2\CarProcedural.cpp (code 0x0042f390..0x00430feb).
//
// Confirmed (tier 1): RTTI .?AVCarProcedural@@ (COL 0x0055ae98, vtable
// 0x00551000, 28 slots): CarProcedural : GraphicsTest : GameObject :
// BaseObject, all single non-virtual inheritance. It overrides slot 0
// (scalar deleting destructor 0x0042f570) and slot 10 (0x0042fd80) and adds
// slot 27 (0x0042f600). The constructor 0x0042f390 writes the vptr at
// 0x0042f3a0, the destructor 0x0042f590 at 0x0042f5ad. __FILE__
// (0x005683e4) is used by slot 27 (lines 0x63, 0x7e, 0x95, 0xa2) and by
// 0x0042fa00 (lines 0xd9, 0xf1).
//
// What the code does (tier 3): slot 27 loads a car model (a
// D3DIMSoultreeObject) with up to six named wheels, two CollisionObjects and
// a keyframed path ("FRAME" / "transform" records of a text file); slot 10
// moves the car along that path each frame (0x00430b10 evaluates the path,
// 0x004308e0 searches it for a point at a given distance). All member and
// method names are provisional.

#include "MatrixUtil.h"
#include "Wrecker.h"   // GraphicsTest

// Scene-graph node methods as CarProcedural calls them (the model, its
// "Body" node and the wheels). Tier 2 from the call sites; see
// src/krusty2/core/SoultreeObject.h for the canonical view.
class CarProceduralNode {
public:
    virtual void UnknownNodeVirtualSlot0();
    virtual void UnknownNodeVirtualSlot1();
    virtual void UnknownNodeVirtualSlot2();
    virtual void UnknownNodeVirtualSlot3();
    virtual void UnknownNodeVirtualSlot4();
    virtual void UnknownNodeVirtualSlot5();
    virtual void UnknownNodeVirtualSlot6();
    virtual void UnknownNodeVirtualSlot7();
    virtual void UnknownNodeVirtualSlot8();
    // Slot 9 (+0x24): loads the model (CarProcedural passes its slot 27
    // arguments 0, 1, 3 and 4, then 1).
    virtual void UnknownNodeVirtualSlot9(void* a0, const char* name, int a3, int a4, int a5);

    CarProceduralNode* UnknownFunction4fdae0(const char* name);       // 0x004fdae0 named child
    void UnknownFunction4fc970(Vector3* position);                   // 0x004fc970
    void UnknownFunction4fe0a0(Vector3* a, Vector3* b);              // 0x004fe0a0 (ret 8)
    // Frame-relative helpers slot 10 uses (src/krusty2/core/SoultreeObject.h
    // documents them): position in `frame` space, placement there, the
    // local<->world direction maps and the two-axis orientation setter.
    void UnknownFunction4fc9a0(CarProceduralNode* frame, Vector3* position);   // 0x004fc9a0 (ret 8)
    void UnknownFunction4fc740(CarProceduralNode* frame, Vector3* position);   // 0x004fc740 (ret 8)
    Vector3 UnknownFunction4fd710(const Vector3& direction);                  // 0x004fd710 (ret 8) world -> local
    Vector3 UnknownFunction4fd5c0(const Vector3& direction);                  // 0x004fd5c0 (ret 8) local -> world
    void UnknownFunction4fbd70(const Vector3* axisA, const Vector3* axisB, int frame, int flag); // 0x004fbd70 (ret 0x10)

    int field_0x04;
    int field_0x08;
};

// D3DIMSoultreeObject (constructor 0x0043f160, new 0x2d8 at line 0x63): a
// node with a GameObject base at +0xc.
class CarProceduralModel : public CarProceduralNode, public GameObject {
public:
    explicit CarProceduralModel(int flags);                          // 0x0043f160
    char field_0x38[0x2d8 - 0x38];            // starts with the model name (0x0042fa00)
};

// Rigid body reached through CarProceduralCollision::field_0x54.
struct CarProceduralBody {
    int field_0x00;
    unsigned char field_0x04[0xf8 - 0x04];
    Vector3 field_0xf8;
};

// Contact record reached through CarProceduralCollision::field_0x5c (the
// collision object's +0x5c, a void* in src/krusty2/collision). Slot 10
// reads +0x00 as the push-out vector of the body collider (scaled by 1.005
// into the body's velocity), and from the sensor collider the float at +0
// and the normal at +0x08 (tier 3 roles).
struct CarProceduralContact {
    Vector3 field_0x00;
    Vector3 field_0x0c;
    Vector3 field_0x18;
};

// Ground query provider at CarProcedural::field_0x20c (0x00507c10, thiscall,
// ret 0x10: snaps *position to the surface and writes its normal; other
// units declare the same function on their own views of the terrain).
class CarProceduralTerrain {
public:
    void UnknownFunction507c10(Vector3* position, Vector3* normal, int a, int b);
};

class CarProceduralQuadTreeObject {
public:
    virtual void UnknownNodeVirtualSlot0();
    int field_0x04;
    int field_0x08;
};

// CollisionObject (constructor 0x00431e70, new 0xb8 at lines 0x95 and
// 0xa2): a quadtree object with a GraphicsTest base at +0xc.
class CarProceduralCollision : public CarProceduralQuadTreeObject, public GraphicsTest {
public:
    explicit CarProceduralCollision(int flags);                      // 0x00431e70
    void UnknownFunction4320f0(void* a0, int a1, int a2, int a3);    // 0x004320f0
    void UnknownFunction432720(CarProceduralModel* model, int a, int b, int c, int d); // 0x00432720
    void UnknownFunction432800(CarProceduralModel* model, const char* path); // 0x00432800
    void UnknownFunction435fe0();                                    // 0x00435fe0
    void UnknownFunction439410(CarProceduralCollision* other);       // 0x00439410
    void UnknownFunction432ab0(int count, Vector3* points);         // 0x00432ab0 (ret 8): mesh shape
    void UnknownFunction435830(const Matrix4* transform);            // 0x00435830 (ret 4): sets the transform
    void UnknownFunction435fb0();                                    // 0x00435fb0: placement update
    int UnknownFunction438e70();                                     // 0x00438e70: runs the collision query

    unsigned char field_0x40[0x54 - 0x40];   // the GraphicsTest base ends at +0x40
    CarProceduralBody* field_0x54;
    int field_0x58;                            // has contact
    CarProceduralContact* field_0x5c;
    unsigned char field_0x60[0x68 - 0x60];
    int field_0x68;
    unsigned char field_0x6c[0xb8 - 0x6c];
};

// One path key (16 bytes): time, then position.
struct CarProceduralKey {
    float time;
    Vector3 position;
};

class CarProcedural : public GraphicsTest {
public:
    explicit CarProcedural(int flags);                               // 0x0042f390 (ret 4)
    virtual ~CarProcedural();                                        // 0x0042f590 (slot 0 0x0042f570)
    virtual int UnknownVirtualSlot10(float frameTime);               // 0x0042fd80
    // Slot 27 (0x0042f600, ret 0x40): sets the car up; returns this.
    virtual CarProcedural* UnknownVirtualSlot27(void* a0, const char* name, const char* collisionFile,
                                                int a3, int a4, void* a5, const char* pathFile,
                                                const Vector3* a7, float speed, int a9, float a10,
                                                int a11, unsigned int wheelCount, float a13, float a14,
                                                float a15);

    void UnknownFunction42fa00(const char* pathFile, const char* name); // 0x0042fa00 (ret 8)
    float UnknownFunction4308e0(float time, float distance);         // 0x004308e0 (ret 8)
    Vector3 UnknownFunction430b10(float time);                       // 0x00430b10 (ret 8)
    // 0x00430e60 (ret 0x14): the four cubic Hermite basis weights of t.
    void UnknownFunction430e60(float t, float* h0, float* h1, float* h2, float* h3);

    CarProceduralModel* field_0x34;
    CarProceduralCollision* field_0x38;
    CarProceduralCollision* field_0x3c;
    Vector3 field_0x40;
    float field_0x4c;
    int field_0x50;
    int field_0x54;                            // set once slot 10 has run its first update
    int field_0x58;
    int field_0x5c;
    unsigned int field_0x60;                   // wheel count
    unsigned int field_0x64;                   // current path key
    int field_0x68;
    unsigned int field_0x6c;                   // path key count
    char field_0x70[0x100];                    // name (at most 0xff characters)
    float field_0x170;                         // speed
    float field_0x174;                         // 1 / speed
    float field_0x178;
    float field_0x17c;
    float field_0x180;
    float field_0x184;                         // 1 / wheel radius
    float field_0x188;
    float field_0x18c;
    float field_0x190;
    float field_0x194;
    float field_0x198;
    float field_0x19c;
    float field_0x1a0;
    float field_0x1a4;
    float field_0x1a8;
    float field_0x1ac;
    float field_0x1b0;
    CarProceduralKey* field_0x1b4;             // path keys
    Vector3 field_0x1b8;
    Vector3 field_0x1c4;
    Vector3 field_0x1d0;
    Vector3 field_0x1dc;
    Vector3 field_0x1e8;
    Vector3 field_0x1f4;
    Vector3 field_0x200;
    void* field_0x20c;                         // the ground query provider (CarProceduralTerrain in slot 10)
    CarProceduralNode** field_0x210;           // wheels
    Vector3 field_0x214;
    Vector3 field_0x220;
};

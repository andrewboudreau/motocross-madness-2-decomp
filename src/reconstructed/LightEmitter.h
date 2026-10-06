#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"

// LightEmitter.cpp (literal __FILE__ at 0x0056ddf0, xrefs 0x0049e276..
// 0x0049e4fc; code 0x0049dd30..0x004a01c4). RTTI: LightEmitter : GameObject
// (vtable 0x005550cc) and LightManager : GameObject (vtable 0x00555144).
// Field and method names are provisional.

// RTTI D3DIMSoultreeObject (vtable 0x005513ec written by 0x0043f160), seen
// from this file only: its SoultreeObject/QuadTreeObject base at +0 owns the
// primary table and the two transform helpers, the GameObject base sits at
// +0x0c (samples/physics/collision/SoultreePhysicsObject.h has the bases).
class D3DIMSoultreeObject {
public:
    explicit D3DIMSoultreeObject(int flags);       // 0x0043f160 (ret 4)
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    // Slot 9 (0x0043f4b0, ret 0x14): loads the model; the result is handed
    // straight to GameObject 0x00469190.
    virtual GameObject* UnknownVirtualSlot9(void* owner, const char* name, int a, int b, int c);

    void UnknownFunction4fc630(Vector3 position);  // 0x004fc630: sets the local translation
    void UnknownFunction4fd340(float x, float y, float z); // 0x004fd340
    // Seen from ArcadeObject.cpp: 0x004fc660 (ret 4) sets the position from
    // a pointer, 0x004fe850 (ret 8) returns two vectors (the second is the
    // half extent ArcadeObject doubles), 0x004fbd70 (ret 0x10) takes four
    // pass-through arguments, 0x004fceb0 (ret 0x10) an axis and an angle,
    // 0x00444d80 (D3DIMSoulTree.CPP, ret 4) attaches a modifier.
    void UnknownFunction4fc660(const Vector3* position);
    void UnknownFunction4fe850(Vector3* a, Vector3* b);
    void UnknownFunction4fbd70(int a, int b, int c, int d);
    void UnknownFunction4fceb0(Vector3 axis, float angle);
    void UnknownFunction444d80(GameObject* modifier);
    // Seen from D3DIMSoultreeModifier.cpp: 0x00444de0 and 0x00444f10 (both
    // D3DIMSoulTree.CPP, ret 4) remove a modifier from the first (+0x264)
    // or second (+0x26c) modifier list.
    void UnknownFunction444de0(GameObject* modifier);
    void UnknownFunction444f10(GameObject* modifier);

    unsigned char field_0x004[0x2d8 - 4];          // operator new size 0x2d8
};

// The object at LightEmitter+0x9c; only these two helpers (both in this
// file) are known.
class UnknownLightEmitterTarget {
public:
    void UnknownFunction49de70(unsigned int color);           // 0x0049de70
    void UnknownFunction49de90(const Vector3* position);      // 0x0049de90

    unsigned char field_0x00[0xb8];
    Vector3 field_0xb8;
    unsigned char field_0xc4[0xd0 - 0xc4];
    unsigned int field_0xd0;                                   // colour without alpha
};

class LightManager;

class LightEmitter : public GameObject {
public:
    explicit LightEmitter(int flags);               // 0x0049deb0
    virtual ~LightEmitter();                        // 0x0049dfc0 (deleting wrapper 0x0049dfa0)
    // Slot 12 is the shared `return 1` body 0x00467ae0.
    virtual int UnknownVirtualSlot12();

    // 0x0049e230 (ret 0x2c): slot 8 of the base, then the type, colour,
    // range, optional position/direction and sphere model; arguments 8-10
    // are unused.
    LightEmitter* UnknownFunction49e230(void* value, int type, unsigned int color,
                                        const Vector3* position, const Vector3* direction,
                                        float range, int sphere, int a8, int a9, int a10,
                                        int index);
    unsigned int UnknownFunction49dfd0();                  // 0x0049dfd0: packed ARGB colour
    void UnknownFunction49e020(unsigned int color);        // 0x0049e020: sets the colours
    void UnknownFunction49e0f0(float range);               // 0x0049e0f0
    void UnknownFunction49e150(const Vector3* position);   // 0x0049e150
    void UnknownFunction49e1e0(const Vector3* direction);  // 0x0049e1e0

    int field_0x2c;                         // type (LightManager 0x004a0190 searches it)
    int field_0x30;                         // derived from the type, default 3
    float field_0x34[4];                    // colour (r, g, b, a)
    float field_0x44[4];
    float field_0x54[4];
    Vector3 field_0x64;                     // position
    Vector3 field_0x70;                     // direction
    float field_0x7c;                       // range, sqrt(FLT_MAX) when unbounded
    float field_0x80;
    float field_0x84;
    float field_0x88;
    float field_0x8c;
    float field_0x90;                       // pi / 4
    float field_0x94;                       // pi / 2
    D3DIMSoultreeObject* field_0x98;        // "sphere.slt" model
    UnknownLightEmitterTarget* field_0x9c;
    int field_0xa0;
    LightManager* field_0xa4;               // owner (set by 0x0049e470)
    unsigned int field_0xa8;                // compared, never written here
};

class LightManager : public GameObject {
public:
    explicit LightManager(int flags);               // 0x0049e390
    virtual ~LightManager();                        // 0x0049e400 (deleting wrapper 0x0049e3e0)
    // Slot 8 is the shared base-forwarding body 0x004452e0.
    virtual GameObject* UnknownVirtualSlot8(void* value);

    void UnknownFunction49e470(LightEmitter* light);      // 0x0049e470: adds a light
    void UnknownFunction49e490();                         // 0x0049e490: counts a change
    LightEmitter* UnknownFunction4a0190(int type);        // 0x004a0190: first light of `type`
    // 0x0049e4a0 lights `count` vertices (Griddraw.h's GridVertexSink is
    // this function seen from the terrain). Not reconstructed: it and its
    // three workers 0x0049e6b0, 0x0049f1c0 and 0x0049f8d0 round with an
    // inline `fld; fistp` sequence that VC6 only emits for inline assembly.
    void UnknownFunction49e4a0(void* matrix, int count, void* vertices, void* buffer,
                               int stride, void* colors);

    int field_0x2c;                         // light count
    LightEmitter* field_0x30[50];
    int* field_0xf8;                        // per-vertex red (one allocation of 3 * capacity)
    int* field_0xfc;                        // green
    int* field_0x100;                       // blue
    int field_0x104;
    int field_0x108;
    int field_0x10c;
    int field_0x110;                        // capacity in vertices
    int field_0x114;
    int field_0x118;                        // change counter
};

// 0x0049f180 (cdecl): out-of-line negation, also called from the terrain.
Vector3 operator-(const Vector3& v);

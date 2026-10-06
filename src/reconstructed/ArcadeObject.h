#pragma once

// ArcadeObject.h -- the reconstructed D:\aardvark\VC\krusty2\ArcadeObject.cpp
// (literal __FILE__ at 0x00566644, xrefs 0x00401338..0x00401646; code
// 0x00401260..0x00401944, after AgeManager.cpp and before Arrow.cpp).
//
// Confirmed (tier 1): RTTI .?AVArcadeObject@@ (COL 0x0055a778, vtable
// 0x00550414, 27 slots), single base GameObject at mdisp 0. The constructor
// 0x00401260 and the destructor 0x00401300 write the vptr. Overrides: slot 0
// (deleting destructor 0x004012e0), slot 10 (0x00401800) and slot 14
// (0x00401920). 0x00401940 (slot 3) is BaseObject::GetRefCount, emitted here.
// Member and method names are provisional (tier 3).

#include "GameObject.h"
#include "MatrixUtil.h"

class CollisionObject;
class D3DIMSoultreeObject;
typedef void (*UnknownPickCallback)(CollisionObject* self, CollisionObject* other);

// RTTI TransparencyMod : D3DIMSoultreeModifier : GraphicsTest : GameObject
// (vtable 0x005588c4, constructor 0x005206a0, 0x4c bytes). The intermediate
// bases are folded into GameObject here; only the alpha byte is used.
class TransparencyMod : public GameObject {
public:
    explicit TransparencyMod(int flags);       // 0x005206a0 (ret 4)

    unsigned char field_0x2c[0x48 - 0x2c];
    unsigned char field_0x48;                  // alpha, 0x80 by default
    unsigned char field_0x49[0x4c - 0x49];
};

// The object passed as the ninth argument of 0x00401310; only its float at
// +0x198 is read (it scales the model to a screen size).
struct UnknownArcadeView {
    unsigned char field_0x000[0x198];
    float field_0x198;
};

class ArcadeObject : public GameObject {
public:
    explicit ArcadeObject(int flags);          // 0x00401260
    virtual ~ArcadeObject();                   // 0x00401300 (deleting wrapper 0x004012e0)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00401800
    virtual int UnknownVirtualSlot14();        // 0x00401920

    // 0x00401310 (ret 0x34): slot 8 of the base, then loads the model `name`
    // (line 78), places it and optionally scales it to `pixels` and attaches
    // a TransparencyMod with `alpha` (line 104). 0 when the model fails.
    ArcadeObject* UnknownFunction401310(void* value, int a, int b, const char* name, Vector3 position,
                                        int pixels, UnknownArcadeView* view, float size, int c, int d,
                                        unsigned char alpha);
    void UnknownFunction4014f0(const Vector3* position);  // 0x004014f0
    void UnknownFunction401520(int a, int b, int c, int d); // 0x00401520: forwards to the model
    // 0x00401540 (ret 0x18): builds the collision object (line 154) from
    // "<name>.col" when the archive has it, under the "Collision" category.
    int UnknownFunction401540(const char* name, int value, UnknownPickCallback callback, void* context,
                              unsigned char a, unsigned char b);
    // 0x004017a0 (ret 0x10): spins around `axis` at `speed` degrees per unit time.
    void UnknownFunction4017a0(Vector3 axis, float speed);

    D3DIMSoultreeObject* field_0x2c;           // model
    int field_0x30;                            // visible
    float field_0x34;                          // scale, 1 by default
    float field_0x38;
    float field_0x3c;
    UnknownArcadeView* field_0x40;
    float field_0x44;
    int field_0x48;
    int field_0x4c;
    float field_0x50;                          // model width
    float field_0x54;                          // model height
    TransparencyMod* field_0x58;
    CollisionObject* field_0x5c;
    int field_0x60;                            // collision built
    int field_0x64;                            // lifetime enabled
    float field_0x68;                          // lifetime
    int field_0x6c;
    float field_0x70;                          // elapsed lifetime
    int field_0x74;
    int field_0x78;                            // delay enabled
    float field_0x7c;                          // delay
    float field_0x80;                          // elapsed delay
    int field_0x84;                            // repeat limit (999: unlimited)
    int field_0x88;                            // repeats
    int field_0x8c;
    int field_0x90;                            // spinning
    Vector3 field_0x94;                        // spin axis
    float field_0xa0;                          // spin angle
    float field_0xa4;                          // spin speed (radians)
};

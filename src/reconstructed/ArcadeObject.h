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
#include "TransparencyMod.h"

class CollisionObject;
class D3DIMSoultreeObject;
typedef void (*UnknownPickCallback)(CollisionObject* self, CollisionObject* other);

// The object passed as the ninth argument of 0x00401310; only its float at
// +0x198 is read here (it scales the model to a screen size). The other
// fields are the camera fields Krusty3DObjects.cpp's VisualCue reads.
struct UnknownArcadeView {
    unsigned char field_0x000[0xac];
    float cameraMatrix[4][4];                 // +0x0ac camera matrix (rows of 4)
    unsigned char field_0x0ec[0x16c - 0xec];
    float fieldOfView;                        // +0x16c field of view, degrees
    Vector3 cameraPosition;                   // +0x170 position
    unsigned char field_0x17c[0x198 - 0x17c];
    float field_0x198;
    unsigned char field_0x19c[0x1b8 - 0x19c];
    float aspectRatio;                        // +0x1b8 aspect ratio
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
    ArcadeObject* Create(void* value, int a, int b, const char* name, Vector3 position,
                                        int pixels, UnknownArcadeView* view, float size, float c, float d,
                                        unsigned char alpha);
    void SetPosition(const Vector3* position);  // 0x004014f0
    // 0x00401520: forwards two axes and two flags to the model (0x004fbd70).
    void SetAxes(const Vector3* a, const Vector3* b, int c, int d);
    // 0x00401540 (ret 0x18): builds the collision object (line 154) from
    // "<name>.col" when the archive has it, under the "Collision" category.
    int BuildCollision(const char* name, int value, UnknownPickCallback callback, void* context,
                              unsigned char a, unsigned char b);
    // 0x004017a0 (ret 0x10): spins around `axis` at `speed` degrees per unit time.
    void StartSpin(Vector3 axis, float speed);

    D3DIMSoultreeObject* arcadeModel;           // model
    int isVisible;                            // visible
    float modelScale;                          // scale, 1 by default
    float field_0x38;
    float field_0x3c;
    UnknownArcadeView* arcadeView;
    float field_0x44;
    float field_0x48;                          // screen fraction (0x00401310's c)
    float field_0x4c;                          // screen fraction (0x00401310's d)
    float modelWidth;                          // model width
    float modelHeight;                        // +0x54 model height
    TransparencyMod* transparency;            // +0x58
    CollisionObject* collisionObject;         // +0x5c
    int collisionBuilt;                       // +0x60 collision built
    int lifetimeEnabled;                      // +0x64 lifetime enabled
    float lifetime;                           // +0x68 lifetime
    int field_0x6c;
    float elapsedLifetime;                    // +0x70 elapsed lifetime
    int field_0x74;
    int delayEnabled;                         // +0x78 delay enabled
    float delay;                              // +0x7c delay
    float elapsedDelay;                       // +0x80 elapsed delay
    int repeatLimit;                          // +0x84 repeat limit (999: unlimited)
    int repeatCount;                          // +0x88 repeats
    int field_0x8c;
    int spinning;                             // +0x90 spinning
    Vector3 spinAxis;                         // +0x94 spin axis
    float spinAngle;                          // +0xa0 spin angle
    float spinSpeed;                          // +0xa4 spin speed (radians)
};

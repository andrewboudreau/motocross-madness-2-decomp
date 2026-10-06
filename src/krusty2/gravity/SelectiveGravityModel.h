// SelectiveGravityModel.h -- SelectiveGravityModel : GameObject
// (retail D:\aardvark\VC\krusty2\SelectiveGravityModel.cpp, __FILE__ string 0x00573f54).
//
// Evidence:
//  * RTTI .?AVSelectiveGravityModel@@ with the single plain base GameObject; vtable 0x00557b0c
//    (29 slots).  Slots 0, 10, 11, 27 and 28 are its own (analysis/vtables.json).
//  * The destructor core 0x004f97b0 and slot 27 (0x004f9820) pass this file's __FILE__ with
//    lines 0x12 and 0x18 to the debug delete and realloc (tier 1).
//  * The TU builds its own four constant vectors (0x00689e18 zero, 0x00689e28 +X,
//    0x00689e38 +Y, 0x00689e08 +Z) in the $E2/$E1 .. $E11/$E10 pairs at 0x004f9910..0x004f9a4b,
//    the math/Math3D.h static constants (tier 1 decoded stores, tier 2 for the header).
//  * Offsets are tier 1 (decoded loads and stores); names are tier 3.
#ifndef SELECTIVE_GRAVITY_MODEL_H
#define SELECTIVE_GRAVITY_MODEL_H

#include "core/GameObject.h"
#include "math/Math3D.h"   // Vec3 and the per-TU constant vectors (0x00689e08..0x00689e40)

// The rigid body the model pushes on, as seen through its vtable.  Slots 0..26 are the
// GameObject ones; slots 27..36 are placeholders and slot 37 is AddWorldForce (the PhysicsBody
// vtable 0x00556e0c, samples/physics/rigidbody/PhysicsBody.h).  Identity of the body class is
// provisional (tier 3); the slot offset 0x94 and the mass at +0x178 are decoded (tier 1).
class GravityBody : public GameObject {
public:
    virtual void Slot27(); virtual void Slot28(); virtual void Slot29(); virtual void Slot30();
    virtual void Slot31(); virtual void Slot32(); virtual void Slot33(); virtual void Slot34();
    virtual void Slot35(); virtual void Slot36();
    virtual void AddWorldForce(Vec3 force);                     // slot 37
    char field_0x2c[0x14c];
    float mass;                                                        // +0x178
};

class SelectiveGravityModel : public GameObject {
public:
    explicit SelectiveGravityModel(int flags);                 // 0x004f9760
    virtual ~SelectiveGravityModel();                          // slot 0, deleting 0x004f9790, core 0x004f97b0
    virtual int GameObjectVirtualSlot10(float dt);             // 0x004f9860
    virtual int GameObjectVirtualSlot11(float dt);             // 0x004f9890
    virtual void AddBody(GravityBody* body);                   // slot 27, 0x004f9820
    // slot 28, 0x004f9a50.  Emitted after the TU's $E initializers, at the end of the
    // object, as VC6 places an inline virtual's out-of-line copy (tier 2).
    virtual void SetGravity(float g) { gravity = g; }

    float gravity;          // +0x2c ctor 32.2f; slot 11 pushes mass * gravity along -Y
    GravityBody** bodies;   // +0x30 array grown by slot 27 (debug realloc, line 0x18); freed by the dtor (line 0x12)
    int bodyCount;          // +0x34 slot 27 appends here
    int field_0x38;         // +0x38 ctor 0; slot 10 runs slot 11 itself while it is 0
};

#endif

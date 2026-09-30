// ConstraintMethodCollisionModel -- header for the constraint (impulse) collision model.
//
// Confirmed (tier 1): primary vtable 0x00551260 at +0 written by 0x0043b980 (ecx-0xc)
// and the constructor 0x0043b8d0; secondary vtable 0x005511f0 at +12; RTTI base
// CollisionObject; sizeof == 0x10c (operator new(0x10c, "wrecker.cpp", 0x68) at 0x005302e8,
// which then calls the constructor 0x0043b8d0 with argument 1).
// Field semantics are tier 3 (justified in ConstraintMethodCollisionModel.cpp).
#ifndef CONSTRAINT_METHOD_COLLISION_MODEL_H
#define CONSTRAINT_METHOD_COLLISION_MODEL_H

#include "../collision/CollisionObject.h"   // canonical CollisionObject chain (MIGRATION.md)
#include "ConstraintTypes.h"

// One probe point (32 bytes), see AddProbePoint / GameObjectVirtualSlot11.
struct ConstraintProbe {
    ConVec3 localPoint;    // +0x00 point in the owner node's space
    ConNode* node;         // +0x0c owner node
    int hit;               // +0x10 set by the last slot 11 query
    ConVec3 normal;        // +0x14 surface normal recorded by that query
};

class ConstraintMethodCollisionModel : public CollisionObject {
public:
    ConstraintMethodCollisionModel(int a);                   // 0x0043b8d0
    virtual void UnknownVirtualSlot2();                      // 0x0044d710 (bare ret)

    // Secondary vtable (this == subobject at +12) overrides.
    virtual ~ConstraintMethodCollisionModel();               // 0x0043b950 -> core 0x0043b980
    virtual GameObject* GameObjectVirtualSlot8(int a);       // 0x0043b9a0
    virtual int GameObjectVirtualSlot10(float dt);           // 0x0043ba40
    virtual int GameObjectVirtualSlot11(float dt);           // 0x0043ba70
    virtual void GameObjectVirtualSlot14();                  // 0x0043c880 (tail jump)

    void SetBody(ConBody* b, int useNodeModel, int arg);     // 0x0043b9e0
    void AddProbePoint(ConVec3 point, ConNode* node);   // 0x0043c7f0
    // 0x0043bdb0: impulse response for a contact; see the header comment in the .cpp.
    void ApplyContactImpulse(ConBody* other, float t, ConVec3 offset, ConVec3 base, ConVec3 normal);

    float field_0xb8;                // restitution e in -(1+e) v.n
    float field_0xbc;                // 0.9f in the constructor
    float dt;                        // 0xc0, slot 11 argument
    ConBody* body;                   // 0xc4
    int field_0xc8;
    ConVec3 contactPoint;            // 0xcc, world contact point of the last impulse
    int field_0xd8;
    int field_0xdc;
    int field_0xe0;
    ConGroundQuery* groundQuery;     // 0xe4
    int field_0xe8;
    int field_0xec;                  // 1 after the constructor
    float damping;                   // 0xf0, velocity *= 1 - dt*damping
    float field_0xf4;                // impulse *= 1 - field_0xf4
    float maxImpulse;                // 0xf8, 0 = unlimited
    float maxSpeed;                  // 0xfc, 0 = unlimited
    int probeCount;                  // 0x100
    ConstraintProbe* probes;         // 0x104
    int field_0x108;                 // 1 after the constructor
};

#endif

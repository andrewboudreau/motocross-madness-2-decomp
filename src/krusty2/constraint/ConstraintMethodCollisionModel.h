// ConstraintMethodCollisionModel -- header for the constraint (impulse) collision model.
//
// Confirmed (tier 1): primary vtable 0x00551260 at +0 written by 0x0043b980 (ecx-0xc)
// and the constructor 0x0043b8d0; secondary vtable 0x005511f0 at +12; RTTI base
// CollisionObject; sizeof == 0x10c (operator new(0x10c, "wrecker.cpp", 0x68) at 0x005302e8,
// which then calls the constructor 0x0043b8d0 with argument 1).
// Field semantics are tier 3 (justified in ConstraintMethodCollisionModel.cpp and its near misses).
#ifndef CONSTRAINT_METHOD_COLLISION_MODEL_H
#define CONSTRAINT_METHOD_COLLISION_MODEL_H

#include "../collision/CollisionObject.h"   // canonical CollisionObject chain (MIGRATION.md)
#include "../math/Vec3.h"

class PhysicsBody;      // rigidbody/PhysicsBody.h
class SoultreeObject;   // core/SoultreeObject.h

class Terrain;

// One probe point (32 bytes), see AddProbePoint / Step. A genuine record, not a view:
// AddProbePoint 0x0043c7f0 DebugReallocs probes (+0x104) by 0x20 per point and stores node, point and hit.
struct ConstraintProbe {
    Vec3 localPoint;    // +0x00 point in the owner node's space
    SoultreeObject* node;  // +0x0c owner node
    int hit;               // +0x10 set by the last slot 11 query
    Vec3 normal;        // +0x14 surface normal recorded by that query
};

class ConstraintMethodCollisionModel : public CollisionObject {
public:
    ConstraintMethodCollisionModel(int flags);                   // 0x0043b8d0
    virtual void UnknownVirtualSlot2();                      // 0x0044d710 (bare ret)

    // Secondary vtable (this == subobject at +12) overrides.
    virtual ~ConstraintMethodCollisionModel();               // 0x0043b950 -> core 0x0043b980
    virtual GameObject* SetRenderContext(void* context);             // 0x0043b9a0
    virtual int Tick(float dt);                              // 0x0043ba40
    virtual int Step(float dt);           // 0x0043ba70
    virtual int Render3D();                   // 0x0043c880 (tail jump)

    void SetBody(PhysicsBody* b, int useNodeModel, const char* shapePath);     // 0x0043b9e0
    void AddProbePoint(Vec3 point, SoultreeObject* node);   // 0x0043c7f0
    // 0x0043bdb0: impulse response for a contact; see the header comment in the .cpp.
    void ApplyContactImpulse(PhysicsBody* other, float t, Vec3 offset, Vec3 point, Vec3 normal);

    float restitution;                // +0xb8 restitution e in -(1+e) v.n
    float field_0xbc;                // 0.9f in the constructor
    float dt;                        // 0xc0, slot 11 argument
    PhysicsBody* body;               // 0xc4
    int refreshPending;  // +0xc8 slot 11 0x0043ba70: when set it runs 0x00435fe0 (double refresh) instead of 0x00435fb0 and clears it
    Vec3 contactPoint;            // 0xcc, world contact point of the last impulse
    int field_0xd8;
    int field_0xdc;
    int field_0xe0;
    Terrain* terrain;  // +0xe4 the probes' ground (Terrain::QueryGround 0x00507c10); 0 skips them
    int manualStep;  // +0xe8 slot 10 0x0043ba40 only calls slot 11 itself when this is 0
    int shapeCollisionEnabled;                  // +0xec 1 after the constructor
    float damping;                   // 0xf0, velocity *= 1 - dt*damping
    float impulseLoss;                // +0xf4 impulse *= 1 - impulseLoss
    float maxImpulse;                // 0xf8, 0 = unlimited
    float maxSpeed;                  // 0xfc, 0 = unlimited
    int probeCount;                  // 0x100
    ConstraintProbe* probes;         // 0x104
    int applyImpulses;                 // +0x108 1 after the constructor
};

#endif

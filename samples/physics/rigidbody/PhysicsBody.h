// PhysicsBody.h -- PhysicsBody / PhysicsRigidBody layout (MCM2 retail, VC6 SP3).
//
// Owner: opus_rigidbody. Uses the shared math in ../common/Math3D.h.
//
// Class evidence:
//  * RTTI .?AVPhysicsBody@@ and .?AVPhysicsRigidBody@@ (tier 1). Complete-object
//    vtables: PhysicsBody 0x00556e0c (44 slots), PhysicsRigidBody 0x00556ec0
//    (45 slots), both at object offset 0. There is single inheritance
//    PhysicsRigidBody : PhysicsBody : GraphicsTest : GameObject (tier 1 from RTTI
//    and from the destructor chain: 0x004cc1e0 writes 0x556ec0 and jumps to
//    0x004cbde0, which writes 0x556e0c and jumps to the GraphicsTest destructor
//    0x0047bd00).
//  * PhysicsBody slots 36..41 are _purecall (0x00534cfe), so PhysicsBody is abstract.
//  * Member offsets are tier 1 (decoded loads and stores in the target functions
//    and the constructors 0x004cbc50 / 0x004cc120). Semantic names are tier 3 and
//    are justified next to each field by the arithmetic that uses it.
#ifndef MCM2_PHYSICS_RIGIDBODY_PHYSICSBODY_H
#define MCM2_PHYSICS_RIGIDBODY_PHYSICSBODY_H

#include "../common/Math3D.h"

// ---------------------------------------------------------------------------
// Provisional GameObject / GraphicsTest stand-ins: 27 virtual slots (the
// GraphicsTest vtable 0x00553de4 shape). Only the slots PhysicsBody overrides have
// meaningful signatures. Arguments of slots 10/11 are 4-byte values (ret 4). The
// rigid body forwards the slot 10 argument to its integrator as the float time
// step, so it is declared as float dt here (tier 3).
// ---------------------------------------------------------------------------
class GameObject {
public:
    virtual ~GameObject();                              // slot 0
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8(int a);
    virtual void UnknownVirtualSlot9(int a);
    // 0x004693d0: when (flags 0x20 & 2), calls slot 10 on each child whose flag
    // byte +0x25 has bit 0 set and bits 2/3 clear. Returns 1.
    virtual int UnknownVirtualSlot10(float dt);
    // 0x004da540: return 1.
    virtual int UnknownVirtualSlot11(float dt);
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    // 0x00469360.
    virtual int UnknownVirtualSlot14();
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

    int field_0x04[7];                // 0x04..0x1f (+0x10 first child, +0x0c next sibling)
    unsigned char field_0x20;         // bit 1: update children (GameObject slot 10)
    unsigned char field_0x21[4];
    unsigned char field_0x25;         // bit 0: object enabled (read by PhysicsBody slot 35)
    unsigned char field_0x26[10];     // 0x26..0x2f
};

class GraphicsTest : public GameObject {
public:
    GraphicsTest(SoultreeObject* node);                 // 0x0047bc70
    virtual ~GraphicsTest();                            // 0x0047bd00

    int field_0x30;                   // zeroed by the GraphicsTest constructor
    int field_0x34;                   // zeroed by the PhysicsRigidBody constructor
};

// ---------------------------------------------------------------------------
// PhysicsBody: vtable 0x00556e0c, object size >= 0x23c.
// ---------------------------------------------------------------------------
class PhysicsBody : public GraphicsTest {
public:
    PhysicsBody(SoultreeObject* node);                  // 0x004cbc50
    virtual ~PhysicsBody();                             // 0x004cbde0 (scalar deleting 0x004cbdc0)

    virtual int UnknownVirtualSlot14();                 // 0x004cbf10

    // slot 27, 0x004cc060: copies at most 255 chars into name and terminates it.
    virtual void SetName(const char* s);
    // slot 28, 0x004cbdf0: attaches the scene-graph node (ignored if null).
    virtual void SetNode(SoultreeObject* n);
    // slot 29, 0x004cbe10: mass and its reciprocal (ignored unless m > 0).
    virtual void SetMass(float m);
    // slot 30, 0x004cbe40: inertia tensor and its inverse.
    virtual void SetInertia(const Matrix4& I);
    // slot 31, 0x004cc0a0: stores a 4-byte value at +0x180 (see PhysicsBodyProbe.cpp).
    virtual void UnknownVirtualSlot31(int v);
    // slot 32, 0x004cc0b0: stores a Vec3 (passed by value, ret 0xc) at +0x184.
    virtual void UnknownVirtualSlot32(Vec3 v);
    // slots 33/34, 0x004cc0d0 / 0x004cc0f0: values copied into velocity /
    // angularVelocity by PhysicsRigidBody::ResetState.
    virtual void SetInitialVelocity(const Vec3& v);
    virtual void SetInitialAngularVelocity(const Vec3& w);
    // slot 35, 0x004cc110: active = enabled bit of GameObject::field_0x25. The
    // argument is unused (ret 4).
    virtual void UnknownVirtualSlot35(int unused);

    // slots 36..41: pure virtual here (_purecall), implemented by PhysicsRigidBody.
    // "World" means world space. "Local" means the node's body space.
    virtual void AddWorldForceAtWorldPoint(Vec3 force, Vec3 point) = 0;   // 36
    virtual void AddWorldForce(Vec3 force) = 0;                           // 37
    virtual void AddWorldTorque(Vec3 torque) = 0;                         // 38
    virtual void AddLocalForceAtLocalPoint(Vec3 force, Vec3 point) = 0;   // 39
    virtual void AddLocalForce(Vec3 force) = 0;                           // 40
    virtual void AddLocalTorque(Vec3 torque) = 0;                         // 41

    // slot 42, 0x004cbe90: velocity expressed in the body frame.
    virtual Vec3 GetLocalVelocity();
    // slot 43, 0x004cbed0: angular velocity expressed in the world frame.
    virtual Vec3 GetWorldAngularVelocity();

    char name[256];                   // 0x038, SetName clamps to 255 + NUL
    int active;                       // 0x138, 1 after construction; the rigid-body
                                      //        step integrates only while nonzero
    Vec3 initialVelocity;             // 0x13c, copied to velocity by ResetState
    Vec3 initialAngularVelocity;      // 0x148, copied to angularVelocity by ResetState
    Vec3 velocity;                    // 0x154, world; integrated with force*invMass
    Vec3 prevVelocity;                // 0x160, velocity saved at the start of a step
    Vec3 angularVelocity;             // 0x16c, body frame; feeds QuatDerivative and
                                      //        Euler's equations w x (I w)
    float mass;                       // 0x178
    float invMass;                    // 0x17c, 1/mass (SetMass)
    int field_0x180;                  // 0x180
    Vec3 field_0x184;                 // 0x184
    Matrix4 inertia;                  // 0x190, body-frame inertia tensor (I w in the step)
    Matrix4 invInertia;               // 0x1d0, MatrixInverse(inertia) (SetInertia)
    Vec3 force;                       // 0x210, world-frame force accumulator
    Vec3 torque;                      // 0x21c, body-frame torque accumulator
    Vec3 centerOfMass;                // 0x228, node-local; torque arm r = p - centerOfMass
    SoultreeObject* node;             // 0x234, scene-graph node carrying the transform
    IntegrateFn integrate;            // 0x238, IntegrateAdamsBashforth2 by default
};

// ---------------------------------------------------------------------------
// PhysicsRigidBody: vtable 0x00556ec0, constructor 0x004cc120.
// The state-derivative arrays hold [0] = current, [1] = previous step, the layout
// the two-step integrator (0x004a1d10) expects.
// ---------------------------------------------------------------------------
class PhysicsRigidBody : public PhysicsBody {
public:
    PhysicsRigidBody(SoultreeObject* node);             // 0x004cc120
    virtual ~PhysicsRigidBody();                        // 0x004cc1e0 (scalar deleting 0x004cc1c0)

    virtual int UnknownVirtualSlot10(float dt);         // 0x004cc600
    virtual int UnknownVirtualSlot11(float dt);         // 0x004cc630, the integration step
    virtual int UnknownVirtualSlot14();                 // 0x004cca80

    virtual void AddWorldForceAtWorldPoint(Vec3 force, Vec3 point);   // 0x004cc230
    virtual void AddWorldForce(Vec3 force);                           // 0x004cc1f0
    virtual void AddWorldTorque(Vec3 torque);                         // 0x004cc330
    virtual void AddLocalForceAtLocalPoint(Vec3 force, Vec3 point);   // 0x004cc3f0
    virtual void AddLocalForce(Vec3 force);                           // 0x004cc390
    virtual void AddLocalTorque(Vec3 torque);                         // 0x004cc4c0

    // slot 44, 0x004cc500: clears the accumulators and derivative history, and
    // restores the initial velocities.
    virtual void ResetState();

    Vec3 dPosition[2];                // 0x23c, d(position)/dt = velocity
    Vec3 dVelocity[2];                // 0x254, d(velocity)/dt = force * invMass
    Quat dOrientation[2];             // 0x26c, QuatDerivative(orientation, angularVelocity)
    Vec3 dAngularVelocity[2];         // 0x28c, invInertia * (torque - w x (I w))
    int field_0x2a4;                  // 0x2a4, nonzero: slot 10 skips the step
};

#endif

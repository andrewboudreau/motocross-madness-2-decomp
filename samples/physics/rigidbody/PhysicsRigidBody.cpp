// PhysicsRigidBody.cpp -- second translation unit of the physics body code.
//
// TU evidence (tier 2): 0x004cbf20..0x004cca7e. It opens with the four Math3D.h
// vector-constant initializers (0x004cbf20..0x004cc010). The remaining PhysicsBody
// setters (slots 27, 31..35) follow, then everything for PhysicsRigidBody: the
// constructor 0x004cc120, the scalar deleting destructor 0x004cc1c0, the
// destructor 0x004cc1e0, the force/torque accumulators, ResetState, and slots
// 10/11/14. VC6 emits $E initializers first and then functions in source order,
// which was verified with a probe compile. The retail file name is unknown. The
// PhysicsBody setters could equally live at the end of an unknown
// PhysicsBody-related file; they are kept here to preserve link order.
#include <string.h>
#include "PhysicsBody.h"

// ---------------------------------------------------------------------------
// PhysicsBody members placed in this TU
// ---------------------------------------------------------------------------

// slot 27, 0x004cc060.
void PhysicsBody::SetName(const char* s)
{
    int n = strlen(s);
    int len = n > 255 ? 255 : n;
    strncpy(name, s, len);          // 0x00534b10
    name[len] = 0;
}

// slot 31, 0x004cc0a0.
void PhysicsBody::UnknownVirtualSlot31(int v)
{
    field_0x180 = v;
}

// slot 32, 0x004cc0b0. Taking the Vec3 by value (3 stack floats, ret 0xc) gives
// the retail straight copy. Three separate floats would build a stack temporary.
void PhysicsBody::UnknownVirtualSlot32(Vec3 v)
{
    field_0x184 = v;
}

// slot 33, 0x004cc0d0.
void PhysicsBody::SetInitialVelocity(const Vec3& v)
{
    initialVelocity = v;
}

// slot 34, 0x004cc0f0.
void PhysicsBody::SetInitialAngularVelocity(const Vec3& w)
{
    initialAngularVelocity = w;
}

// slot 35, 0x004cc110.
void PhysicsBody::UnknownVirtualSlot35(int)
{
    active = field_0x25 & 1;
}

// ---------------------------------------------------------------------------
// PhysicsRigidBody
// ---------------------------------------------------------------------------

// 0x004cc120 (not a target; emits the vtable and the scalar deleting destructor).
PhysicsRigidBody::PhysicsRigidBody(SoultreeObject* n)
    : PhysicsBody(n)
{
    field_0x34 = 0;
    force = kVec3Zero;
    torque = kVec3Zero;
    ResetState();
    field_0x2a4 = 0;
}

// 0x004cc1e0: vptr = 0x00556ec0; tail-jumps to PhysicsBody::~PhysicsBody.
PhysicsRigidBody::~PhysicsRigidBody()
{
}

// slot 37, 0x004cc1f0.
void PhysicsRigidBody::AddWorldForce(Vec3 f)
{
    force += f;
}

// slot 36, 0x004cc230. A world-space force at a world-space point. The force is
// accumulated in world space. The torque arm r (from the centre of mass) and the
// force are both taken into body space, so the torque accumulates in body space.
void PhysicsRigidBody::AddWorldForceAtWorldPoint(Vec3 f, Vec3 p)
{
    if (node) {
        Vec3 localForce = node->WorldToLocalDirection(f);
        Vec3 r = node->WorldToLocalPoint(p) - centerOfMass;
        force += f;
        torque += CrossProduct(r, localForce);
    }
}

// slot 38, 0x004cc330.
void PhysicsRigidBody::AddWorldTorque(Vec3 t)
{
    if (node)
        torque += node->WorldToLocalDirection(t);
}

// slot 40, 0x004cc390.
void PhysicsRigidBody::AddLocalForce(Vec3 f)
{
    if (node)
        force += node->LocalToWorldDirection(f);
}

// slot 39, 0x004cc3f0.
void PhysicsRigidBody::AddLocalForceAtLocalPoint(Vec3 f, Vec3 p)
{
    if (node) {
        force += node->LocalToWorldDirection(f);
        torque += CrossProduct(p - centerOfMass, f);
    }
}

// slot 41, 0x004cc4c0.
void PhysicsRigidBody::AddLocalTorque(Vec3 t)
{
    torque += t;
}

// slot 44, 0x004cc500. The derivative history is cleared with memset. The VC6
// intrinsic gives the retail 'xor esi,esi' plus three dword stores per vector;
// Vec3(0,0,0) would build hoisted stack temporaries instead.
void PhysicsRigidBody::ResetState()
{
    force = kVec3Zero;
    torque = kVec3Zero;
    velocity = initialVelocity;
    prevVelocity = initialVelocity;
    angularVelocity = initialAngularVelocity;
    for (int i = 0; i < 2; i++) {
        memset(&dPosition[i], 0, sizeof(dPosition[i]));
        dOrientation[i] = g_QuatIdentity;
        memset(&dVelocity[i], 0, sizeof(dVelocity[i]));
        memset(&dAngularVelocity[i], 0, sizeof(dAngularVelocity[i]));
    }
}

// slot 10, 0x004cc600.
int PhysicsRigidBody::UnknownVirtualSlot10(float dt)
{
    if (!field_0x2a4)
        UnknownVirtualSlot11(dt);
    return GraphicsTest::UnknownVirtualSlot10(dt);
}

// slot 11, 0x004cc630: one rigid-body integration step of length dt.
// State: centre-of-mass world position, world velocity, orientation quaternion,
// and body angular velocity. Derivatives: velocity, force/m, 0.5*q*(0,w), and
// Euler's equations I^-1 (tau - w x Iw). Each pair goes through the integrator
// callback (two-step Adams-Bashforth by default). The node transform is then
// rebuilt from the new orientation and centre-of-mass position.
//
// Shape notes (VC6, no /G6; 983/1030 unmasked bytes, see targets.json):
//  * Retail writes the current-derivative slots component by component. Its stores
//    go straight to [this+0x23c..] etc. with no 'lea' of the element, and whole-Vec3
//    assignment does not reproduce that. The component form does. This hints that
//    the original derivative storage may have been plain float arrays (tier 3).
//  * The zero branch is three separate 0.0f stores: 'xor eax,eax' plus three direct
//    stores. memset would take the element address first.
//  * 'float m = mass;' gives the retail 'fld mass; fdivr 1.0f' operand order.
//  * The normalised quaternion is written back into 'orientation'. A separate named
//    Quat adds 16 bytes of frame (0xcc instead of 0xbc).
int PhysicsRigidBody::UnknownVirtualSlot11(float dt)
{
    GraphicsTest::UnknownVirtualSlot10(dt);
    if (node && active) {
        prevVelocity = velocity;

        Vec3 position = node->LocalToWorldPoint(centerOfMass);
        Matrix4 rotation;
        node->GetMatrixIn(0, &rotation);
        Quat orientation = QuatFromMatrix(rotation);

        dPosition[0].x = velocity.x;
        dPosition[0].y = velocity.y;
        dPosition[0].z = velocity.z;
        if (mass > 0.0) {
            float m = mass;
            Vec3 a = force * (1.0f / m);
            dVelocity[0].x = a.x;
            dVelocity[0].y = a.y;
            dVelocity[0].z = a.z;
        } else {
            dVelocity[0].x = 0.0f;
            dVelocity[0].y = 0.0f;
            dVelocity[0].z = 0.0f;
        }
        dOrientation[0] = QuatDerivative(orientation, angularVelocity);
        Vec3 L = RotateVector(angularVelocity, inertia);
        Vec3 alpha = RotateVector(torque - CrossProduct(angularVelocity, L), invInertia);
        dAngularVelocity[0].x = alpha.x;
        dAngularVelocity[0].y = alpha.y;
        dAngularVelocity[0].z = alpha.z;

        integrate(dt, 3, &position.x, &dPosition[0].x);
        integrate(dt, 3, &velocity.x, &dVelocity[0].x);
        integrate(dt, 4, &orientation.w, &dOrientation[0].w);
        integrate(dt, 3, &angularVelocity.x, &dAngularVelocity[0].x);

        orientation = QuatNormalize(orientation);
        rotation = QuatToMatrix(orientation);
        node->SetMatrixIn(0, &rotation);
        Vec3 offset = node->LocalToWorldPoint(centerOfMass);
        position = position - offset;
        node->SetPositionIn(0, &position);
    }
    force = kVec3Zero;
    torque = kVec3Zero;
    return 1;
}

// slot 14, 0x004cca80.
int PhysicsRigidBody::UnknownVirtualSlot14()
{
    return PhysicsBody::UnknownVirtualSlot14();
}

// PhysicsBody.cpp -- first PhysicsBody translation unit.
//
// TU evidence (tier 2): the retail code 0x004cbb10..0x004cbf1b is one TU. It
// starts with the four Math3D.h vector-constant initializers (0x004cbb10..0x004cbc00).
// Then come the constructor 0x004cbc50, the scalar deleting destructor 0x004cbdc0,
// the destructor 0x004cbde0, and slots 28, 29, 30, 42, 43 and 14. The next four
// initializers (0x004cbf20..) open the following TU. The remaining PhysicsBody
// setters (slots 27, 31..35) sit in that TU, ahead of the PhysicsRigidBody
// constructor (see PhysicsRigidBody.cpp). Neighbouring source strings are
// PCVideoCard.cpp (before) and Pixtrans.cpp (after), which fits alphabetical link order.
#include "PhysicsBody.h"

// 0x004cbc50 (not a target; kept to emit the vtable and document the defaults).
PhysicsBody::PhysicsBody(SoultreeObject* n)
    : GraphicsTest(n)
{
    active = 1;
    initialVelocity = kVec3Zero;
    initialAngularVelocity = kVec3Zero;
    velocity = kVec3Zero;
    angularVelocity = kVec3Zero;
    mass = 0.0f;
    invMass = 0.0f;
    field_0x180 = 0;
    inertia = MatrixZero();
    invInertia = MatrixZero();
    field_0x184 = kVec3Zero;
    integrate = IntegrateAdamsBashforth2;
    centerOfMass = kVec3Zero;
    node = 0;
}

// 0x004cbde0: vptr = 0x00556e0c; tail-jumps to GraphicsTest::~GraphicsTest.
// The scalar deleting destructor 0x004cbdc0 is compiler-generated from this.
PhysicsBody::~PhysicsBody()
{
}

// slot 28, 0x004cbdf0.
void PhysicsBody::SetNode(SoultreeObject* n)
{
    if (n)
        node = n;
}

// slot 29, 0x004cbe10. The comparison is against the double 0.0 at 0x005507e8.
void PhysicsBody::SetMass(float m)
{
    if (m > 0.0) {
        mass = m;
        invMass = 1.0f / m;
    }
}

// slot 30, 0x004cbe40.
void PhysicsBody::SetInertia(const Matrix4& I)
{
    inertia = I;
    invInertia = MatrixInverse(I);
}

// slot 42, 0x004cbe90.
Vec3 PhysicsBody::GetLocalVelocity()
{
    return node->WorldToLocalDirection(velocity);
}

// slot 43, 0x004cbed0.
Vec3 PhysicsBody::GetWorldAngularVelocity()
{
    return node->LocalToWorldDirection(angularVelocity);
}

// slot 14, 0x004cbf10.
int PhysicsBody::UnknownVirtualSlot14()
{
    GraphicsTest::UnknownVirtualSlot14();
    return 1;
}

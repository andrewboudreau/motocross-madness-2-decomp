// ContactImpulse.cpp -- physics contact/impulse core (SoulTreePhysics.cpp neighbourhood).
#include "ContactTypes.h"

// 0x005004a0 (1062 bytes).  Impulse between two bodies A and B at a contact.
//   restitution  first argument (SoultreePhysicsBaseObject::field_0x14c)
//   n            contact normal
//   invMassA     inverse mass of A (field_0x24)
//   nodeA/nodeB  scene-graph frames used to move directions between world and local space
//   dispA/dispB  displacement of the contact point of A / B over the step
//   rA/rB        lever arms from each body centre to the contact
//   invInertiaA/B  diagonal inverse inertia (field_0xe4 / slot 2's inertia vector)
//   angVelA/velA velocity accumulators updated for A (angular in local space, linear)
//   angVelB/velB same for B (only when velB != 0)
//   outJ         receives the impulse magnitude j.
// j = -(1 + e) * (dispA - dispB).n / (n.S + invMassA + invMassB) with
// S = ((I^-1 (rA x n)) x rA) + ((I^-1 (rB x n)) x rB).  Names are tier 3.
void ContactSolveImpulse(float restitution, const Vec3* n, float invMassA, SoultreeObject* nodeA,
                         const Vec3* dispA, const Vec3* rA, const Vec3* invInertiaA,
                         Vec3* angVelA, Vec3* velA, float invMassB, SoultreeObject* nodeB,
                         const Vec3* dispB, const Vec3* rB, const Vec3* invInertiaB,
                         Vec3* angVelB, Vec3* velB, float* outJ)
{
    Vec3 rel = *dispA - *dispB;
    float closingZ = rel.z * n->z;
    float closingXY = rel.y * n->y + rel.x * n->x;

    float sumN;
    Vec3 angA, angB, uA;
    {
        Vec3 torque = ContactCross(*n, *rA);
        Vec3 local = nodeA->WorldToLocalDirection(torque);
        angA.x = local.x * invInertiaA->x;
        angA.y = local.y * invInertiaA->y;
        angA.z = local.z * invInertiaA->z;
        Vec3 world = nodeA->LocalToWorldDirection(angA);
        uA = CrossProduct(world, *rA);
    }
    {
        Vec3 torque = ContactCross(*n, *rB);
        Vec3 local = nodeB->WorldToLocalDirection(torque);
        angB.x = local.x * invInertiaB->x;
        angB.y = local.y * invInertiaB->y;
        angB.z = local.z * invInertiaB->z;
        Vec3 world = nodeB->LocalToWorldDirection(angB);
        Vec3 uB = CrossProduct(world, *rB);
        sumN = DotProduct(ContactVec3(uA.x + uB.x, uA.y + uB.y, uA.z + uB.z), *n);
    }
    *outJ = -((closingXY + closingZ) * (restitution + 1.0f)) / (sumN + invMassA + invMassB);

    ContactVec3 J(*outJ * n->x, *outJ * n->y, *outJ * n->z);
    ContactVec3 dvA(J.x * invMassA, J.y * invMassA, J.z * invMassA);
    *velA += dvA;
    ((ContactVec3*)angVelA)->operator+=(ContactVec3(angA.x * *outJ, angA.y * *outJ, angA.z * *outJ));
    if (velB) {
        ContactVec3 dvB(-J.x * invMassB, -J.y * invMassB, -J.z * invMassB);
        *(ContactVec3*)velB += dvB;
        ((ContactVec3*)angVelB)->operator+=(ContactVec3(angB.x * *outJ, angB.y * *outJ, angB.z * *outJ));
    }
}

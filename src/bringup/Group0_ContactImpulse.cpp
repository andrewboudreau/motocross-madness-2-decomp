// Group0_ContactImpulse.cpp -- bring-up body for the contact impulse solver
// ContactSolveImpulse (0x005004a0), declared in contact/ContactImpulse.h.
// Follows samples/physics/contact/ContactImpulse.cpp (decoded structure, 940
// of 1062 retail bytes in that sample) with plain Math3D arithmetic; the tail
// (B's angular update scaled by +j, B's linear update by -j) was rechecked
// against retail 0x00500800..0x005008bf. Byte matching was not attempted.
#include "contact/ContactImpulse.h"

// j = -(1 + e) * ((dispA - dispB) . n) / (n . S + invMassA + invMassB) with
// S = ((I_A^-1 (rA x n)) x rA) + ((I_B^-1 (rB x n)) x rB), the inertia applied in
// each body's local frame. A's velocities get +j, B's linear velocity -j
// (only when velB is given); *outJ receives j.
void ContactSolveImpulse(float restitution, const Vec3* n, float invMassA, SoultreeObject* nodeA,
                         const Vec3* dispA, const Vec3* rA, const Vec3* invInertiaA,
                         Vec3* angVelA, Vec3* velA, float invMassB, SoultreeObject* nodeB,
                         const Vec3* dispB, const Vec3* rB, const Vec3* invInertiaB,
                         Vec3* angVelB, Vec3* velB, float* outJ)
{
    Vec3 rel = *dispA - *dispB;
    float closing = (rel.y * n->y + rel.x * n->x) + rel.z * n->z;

    Vec3 localA = nodeA->WorldToLocalDirection(CrossProduct(*rA, *n));
    Vec3 angA(localA.x * invInertiaA->x, localA.y * invInertiaA->y, localA.z * invInertiaA->z);
    Vec3 uA = CrossProduct(nodeA->LocalToWorldDirection(angA), *rA);

    Vec3 localB = nodeB->WorldToLocalDirection(CrossProduct(*rB, *n));
    Vec3 angB(localB.x * invInertiaB->x, localB.y * invInertiaB->y, localB.z * invInertiaB->z);
    Vec3 uB = CrossProduct(nodeB->LocalToWorldDirection(angB), *rB);

    Vec3 sum = uA + uB;
    float num = -(closing * (restitution + 1.0f));
    float j = num / (DotProduct(sum, *n) + invMassA + invMassB);
    *outJ = j;

    Vec3 impulse = *n * j;
    *velA += impulse * invMassA;
    *angVelA += angA * *outJ;
    if (velB) {
        Vec3 impulseB = *n * -*outJ;
        *velB += impulseB * invMassB;
        *angVelB += angB * *outJ;
    }
}

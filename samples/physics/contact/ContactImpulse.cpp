// ContactImpulse.cpp -- physics contact/impulse core (SoulTreePhysics.cpp neighbourhood).
#include "ContactTypes.h"

// Dot product with the z term added last; VC6 then sums y, x, z as retail does.
inline float ContactDot(const Vec3& a, const Vec3& b) { return a.z * b.z + (a.x * b.x + a.y * b.y); }
// Out-of-line Vec3 * float (0x005015b0) at the sites where retail calls it.
ContactVec3 ContactScaleCall(const Vec3& v, float s);
inline ContactVec3 ContactScaled(const Vec3& v, float s) { return ContactVec3(v.x * s, v.y * s, v.z * s); }
inline ContactVec3 ContactSum(const Vec3& a, const Vec3& b) { return ContactVec3(a.x + b.x, a.y + b.y, a.z + b.z); }

// Torque B's cross in the Vec3(x, y, z) constructor form with the products parenthesised
// except x's first one (931 strict against 874 for Math3D CrossProduct there).  Retail
// spelling unknown (provisional): only the grouping moves VC6's frame slots; product
// operand order is inert (all 64 forms measured).
static inline Vec3 ContactCrossCtor(const Vec3& a, const Vec3& b)
{
    return Vec3(a.y * b.z - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x));
}

// uB's cross with every product parenthesised (940 strict against 931 for Math3D
// CrossProduct and 933 for ContactCrossCtor there; provisional spelling).  The fully
// parenthesised form at torque B as well gives 919, and either helper at uA loses.
static inline Vec3 ContactCrossParen(const Vec3& a, const Vec3& b)
{
    return Vec3((a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x));
}

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
// Retail tail (decoded): J = *n * j through Vec3(x,y,z) 0x404e60 into a named copy,
// *velA += J * invMassA inline, then the out-of-line operator*(Vec3,float) 0x5015b0 and
// Vec3::operator+= 0x428060 for angVelA and the whole B branch (J = *n * -j), so retail had
// spent its inline budget by then.  ContactScaleCall models those 0x5015b0 sites (bound in
// the bindings, docs/VC6_INLINE_BUDGET.md); this tail lines up call for call (0.94 ratio).
// The two torque blocks are written at function scope with distinct names; with
// sibling { } scopes VC6 shares their slots and the frame shrinks to 0x54 (retail 0x60).
// closingXY is written (y*y') + x*x' so the y product is loaded first, as retail does.
// 940/1062 strict.  Remaining: the torque cross products load r before n where retail
// loads n first, and most temporaries sit in other frame slots.  The product order
// follows leaf age, so neither operand order nor parentheses in the cross move it (all
// 2^6 operand orders measured in the member-store and constructor forms, also written
// as n x r with the subtraction swapped), nor do pointer/reference aliases of n, a Vec3 copy of n (worse), placing
// the closing dot after the first cross, or naming the tail temporaries.  The
// constructor helper at torque A as well scores 896 (910 at A alone).  The tail also
// schedules `mov edi, [esp+0xb0]` before a push.
void ContactSolveImpulse(float restitution, const Vec3* n, float invMassA, SoultreeObject* nodeA,
                         const Vec3* dispA, const Vec3* rA, const Vec3* invInertiaA,
                         Vec3* angVelA, Vec3* velA, float invMassB, SoultreeObject* nodeB,
                         const Vec3* dispB, const Vec3* rB, const Vec3* invInertiaB,
                         Vec3* angVelB, Vec3* velB, float* outJ)
{
    Vec3 rel = *dispA - *dispB;
    float closingZ = rel.z * n->z;
    float closingXY = (rel.y * n->y) + rel.x * n->x;

    ContactVec3 sum;
    Vec3 angA, angB, uA;
    Vec3 torqueA = CrossProduct(*rA, *n);
    Vec3 localA = nodeA->WorldToLocalDirection(torqueA);
    angA.x = localA.x * invInertiaA->x;
    angA.y = localA.y * invInertiaA->y;
    angA.z = localA.z * invInertiaA->z;
    Vec3 worldA = nodeA->LocalToWorldDirection(angA);
    uA = CrossProduct(worldA, *rA);
    Vec3 torqueB = ContactCrossCtor(*rB, *n);
    Vec3 localB = nodeB->WorldToLocalDirection(torqueB);
    angB.x = localB.x * invInertiaB->x;
    angB.y = localB.y * invInertiaB->y;
    angB.z = localB.z * invInertiaB->z;
    Vec3 worldB = nodeB->LocalToWorldDirection(angB);
    Vec3 uB = ContactCrossParen(worldB, *rB);
    sum = ContactSum(uA, uB);
    float num = -((closingXY + closingZ) * (restitution + 1.0f));
    float j = num / (ContactDot(sum, *n) + invMassA + invMassB);
    *outJ = j;
    ContactVec3 J = ContactScaled(*n, j);
    ContactVec3 dvA = ContactScaled(J, invMassA);
    *velA += dvA;
    ContactVec3 dA = ContactScaleCall(angA, *outJ);
    ((ContactVec3*)angVelA)->operator+=(dA);
    if (velB) {
        ContactVec3 Jb = ContactScaleCall(*n, -*outJ);
        ((ContactVec3*)velB)->operator+=(ContactScaleCall(Jb, invMassB));
        ContactVec3 dB = ContactScaleCall(angB, *outJ);
        ((ContactVec3*)angVelB)->operator+=(dB);
    }
}

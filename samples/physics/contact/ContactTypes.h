// ContactTypes.h -- shared declarations for the contact/impulse translation unit
// (PROVISIONAL: tier 3 names; offsets and calling conventions are decoded from the
// target bytes).  Reuses Vec3 (../common/Math3D.h) and SoultreeNode
// (../soultree_base/SoultreePhysicsCallees.h); nothing about their layout is duplicated.
#ifndef CONTACT_TYPES_H
#define CONTACT_TYPES_H

#include "../soultree_base/SoultreePhysicsCallees.h"

// Vec3 whose (x, y, z) constructor and operator+= are kept out of line in retail
// (0x00404e60 thiscall ret 0xc; 0x00428060 thiscall ret 4).  Declared only, so VC6 emits
// the calls instead of expanding them.  Derives from Vec3 so that node helpers taking
// or returning Vec3 work unchanged.
struct ContactVec3 : public Vec3 {
    ContactVec3() {}
    ContactVec3(float x_, float y_, float z_);      // 0x00404e60
    ContactVec3& operator+=(const ContactVec3& v);  // 0x00428060
};

// Cross product written the way the retail solver expands it: every fmul takes the
// second operand first (same shape as SoultreeCross in SoulTreePhysics.cpp).
static inline Vec3 ContactCross(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.x = a.z * b.y - a.y * b.z;
    r.y = a.x * b.z - a.z * b.x;
    r.z = a.y * b.x - a.x * b.y;
    return r;
}

// 0x005004a0 with pointer-typed nodeB (arg 11) and velB (arg 16).  The shared header
// SoultreePhysicsCallees.h declares those two as int (Fn_5004a0); the unification pass
// should switch them to SoultreeNode* / SoultreeVec3*.
void ContactSolveImpulse(float restitution, const Vec3* n, float invMassA, SoultreeNode* nodeA,
                         const Vec3* dispA, const Vec3* rA, const Vec3* invInertiaA,
                         ContactVec3* angVelA, Vec3* velA, float invMassB, SoultreeNode* nodeB,
                         const Vec3* dispB, const Vec3* rB, const Vec3* invInertiaB,
                         ContactVec3* angVelB, ContactVec3* velB, float* outJ);

#endif

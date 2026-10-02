// ContactImpulse.h -- the single declaration of the contact impulse solver 0x005004a0
// (defined in ContactImpulse.cpp; callers: SoultreePhysicsBaseObject slot 4 0x005013d0
// and Vehicle slot 4).  Kept free of other area headers so soultree_base and vehicle can
// include it without a cycle.
#ifndef CONTACT_IMPULSE_H
#define CONTACT_IMPULSE_H

#include "math/Math3D.h"

class SoultreeObject;

// 0x005004a0, cdecl, 17 stack args (tier 1: arg count and caller cleanup).  nodeA/nodeB
// are scene-graph nodes (both are `this` of the thiscalls 0x4fd710/0x4fd5c0, tier 1);
// velB is tested for NULL before B is updated (tier 1).  Parameter names are tier 3.
// The slot 4 callers carry nodeB and velB as int arguments and cast at the call.
void ContactSolveImpulse(float restitution, const Vec3* n, float invMassA, SoultreeObject* nodeA,
                         const Vec3* dispA, const Vec3* rA, const Vec3* invInertiaA,
                         Vec3* angVelA, Vec3* velA, float invMassB, SoultreeObject* nodeB,
                         const Vec3* dispB, const Vec3* rB, const Vec3* invInertiaB,
                         Vec3* angVelB, Vec3* velB, float* outJ);

#endif

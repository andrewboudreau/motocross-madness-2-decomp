// Declarations of the non-virtual callees used by SoulTreePhysics.cpp.
// Only calling convention and argument shapes matter (call targets are
// relocation-masked).  Names carry the retail address (tier 3).
#ifndef SOULTREE_PHYSICS_CALLEES_H
#define SOULTREE_PHYSICS_CALLEES_H

#include "soultree/SoultreePhysicsTypes.h"
#include "contact/ContactImpulse.h"
#include "contact/ObjectPlacement.h"

// The node at SoultreePhysicsBaseObject::field_0x08 / field_0x218 is SoultreeObject
// (../common/SoultreeObject.h, included through Math3D.h).

// cdecl helpers outside this class.
extern "C++" {
// 0x004cb6e0 is UnknownAxisSettle_4cb6e0 (../common/Math3D.h).
}


// TU-local solver helper (cdecl), 0x00500220 (12 args).  0x005004a0 is declared in
// ../contact/ContactImpulse.h and 0x004b0df0 in ../contact/ObjectPlacement.h.
void ResolveContactImpulse(float restitution, float invMass, SoultreeObject* node,
                           const Vec3* normal, const Vec3* relVel, const Vec3* arm,
                           Vec3* invInertia, const Vec3* angScale, Vec3* angVel,
                           Vec3* velocity, float* impulseOut, int scaleBits);


// 0x005015b0 is Vec3ScaleCall (../common/Math3D.h out-of-line call views).

#endif

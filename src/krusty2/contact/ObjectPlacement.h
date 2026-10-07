// ObjectPlacement.h -- the single declaration of the placement query 0x004b0df0 (defined
// in ObjectPlacement.cpp; callers: SoultreePhysicsBaseObject slot 11 0x00501da0 and the
// KrustyBike slot 11 override).  Kept free of other area headers so soultree_base and
// krustybike can include it without a cycle.
#ifndef OBJECT_PLACEMENT_H
#define OBJECT_PLACEMENT_H

#include "math/Math3D.h"

class CollisionObject;
struct SoultreeProbe;

// 0x004b0df0, cdecl, 17 stack args (tier 1).  j is an unsigned char (the base caller
// zero-extends it with mov al, tier 1); k and l are nullable Vec3 pointers (0x4b17f1 and
// 0x4b1c72 dereference l, tier 1).  Parameter roles: see ObjectPlacement.cpp (tier 3).
int FindObjectPlacement(CollisionObject* a, SoultreeProbe* b, Vec3* c, float d, int e, int f, int g, int h,
              float i, unsigned char j, const Vec3* k, const Vec3* l, int m, Vec3* n, Vec3* o,
              Vec3* p, int* q);

#endif

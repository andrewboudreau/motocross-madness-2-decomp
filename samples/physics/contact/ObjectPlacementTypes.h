// ObjectPlacementTypes.h -- declarations used by ObjectPlacement.cpp (0x004b0df0).
// PROVISIONAL (tier 3 names).  Offsets, argument counts and calling conventions are
// decoded from the target bytes (tier 1); the names are guesses.
//
// The CollisionObject members 0x00435830 SetTransform, 0x00439400 SetField_0x74 and
// 0x00439410 AddIgnoredOwner are declared in ../collision/CollisionObject.h.
// Virtual: slot 2 of the GraphicsTest subobject at +0xc (BaseObjectVirtualSlot2, the
// Release-style slot, called through [ [obj+0xc] + 8 ]) is invoked after use.
#ifndef OBJECT_PLACEMENT_TYPES_H
#define OBJECT_PLACEMENT_TYPES_H

#include "ContactTypes.h"
#include "../collision/CollisionObject.h"

// Ground/probe query object (SoultreeProbe in SoultreePhysicsCallees.h is only forward
// declared).  0x00507c10 is thiscall, ret 0x10: (point, out point, 1, out flag byte).
struct PlacementProbe {
    void Fn_507c10(const Vec3* point, Vec3* out, int one, unsigned char* flag);
};

// cdecl helpers (call targets are relocation-masked; only shapes matter).
extern "C++" {
// 0x004b0ac0: three radii + a float; returns 0 when blocked (tier 3).  9 stack args.
int Fn_4b0ac0(const Vec3* p, float r0, float r1, float r2, float d, int a, int b, int c, int d2);
// 0x004b08f0: 7 stack args (point, margin, body, f, g, 1, 0); nonzero = blocked (tier 3).
int Fn_4b08f0(const Vec3* p, float margin, CollisionObject* a, int f, int g, int one, int zero);
// 0x004b0b80: 10 stack args, the tail call of 0x004b0df0; its result is the caller's result.
int Fn_4b0b80(CollisionObject* a, SoultreeProbe* b, Vec3* c, float d, int e, int f, int g,
              const Vec3* l, Vec3* n, Vec3* o);
// 0x00515600 / 0x0040ae30 / 0x005015b0: CrossProductCall / Vec3DotCall / Vec3ScaleCall
// (../common/Math3D.h out-of-line call views).
}

#endif

// ObjectPlacementTypes.h -- declarations used by ObjectPlacement.cpp (0x004b0df0).
// PROVISIONAL (tier 3 names).  Offsets, argument counts and calling conventions are
// decoded from the target bytes (tier 1); the names are guesses.
//
// Header note for the collision area: these CollisionObject members are called by
// 0x004b0df0 with this == the complete object (ecx = the pointer returned by the
// ctor 0x00431e70).  They should be added to CollisionObject.h; until then they are
// reached through PlacementBodyOps, a data-less struct declaring the same thiscalls.
//   0x00435830  void SetTransform(const Matrix4* m)   thiscall, ret 4 (switch on
//               field_0x50 shape type 0..4, jump table at 0x435ea8; copies the 16 floats
//               of the argument into the shape payload at field_0x54 and derives values)
//   0x00439400  void SetField_0x74(int v)             thiscall, ret 4: field_0x74 = v
//   0x00439410  void AddIgnoredOwner(void* owner)     thiscall, ret 4: unique-add into the
//               growable pointer array field_0x78 (data) / field_0x7c (count): returns if
//               already present, reuses a NULL slot, else reallocs through 0x004a2ec0
//               (__FILE__ ..., line 0x843) and appends
//   0x00432ab0  Fn_00432ab0(count, points)            already on CollisionObject
//   0x00438e70  Fn_00438e70()                         already on CollisionObject
// Virtual: slot 2 of the GraphicsTest subobject at +0xc (BaseObjectVirtualSlot2, the
// Release-style slot, called through [ [obj+0xc] + 8 ]) is invoked after use.
#ifndef OBJECT_PLACEMENT_TYPES_H
#define OBJECT_PLACEMENT_TYPES_H

#include "ContactTypes.h"
#include "../collision/CollisionObject.h"

struct PlacementBodyOps {
    void SetTransform(const Matrix4* m);       // 0x00435830
    void SetField_0x74(int v);                 // 0x00439400
    void AddIgnoredOwner(void* owner);         // 0x00439410
};

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
int Fn_4b08f0(const Vec3* p, float margin, SoultreeBody* a, int f, int g, int one, int zero);
// 0x004b0b80: 10 stack args, the tail call of 0x004b0df0; its result is the caller's result.
int Fn_4b0b80(SoultreeBody* a, SoultreeProbe* b, Vec3* c, float d, int e, int f, int g,
              const Vec3* l, Vec3* n, Vec3* o);
// 0x00515600: cross product, hidden result pointer first (out-of-line COMDAT copy).
Vec3 PlacementCross(const Vec3& a, const Vec3& b);
// 0x0040ae30: dot product, out of line.
float PlacementDot(const Vec3* a, const Vec3* b);
}

#endif

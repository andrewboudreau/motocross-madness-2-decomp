// Declarations of the non-virtual callees used by SoulTreePhysics.cpp.
// Only calling convention and argument shapes matter (call targets are
// relocation-masked).  Names carry the retail address (tier 3).
#ifndef SOULTREE_PHYSICS_CALLEES_H
#define SOULTREE_PHYSICS_CALLEES_H

#include "SoultreePhysicsTypes.h"

// Object pointed to by SoultreePhysicsBaseObject::field_0x08 / field_0x218.  This is the
// scene-graph node documented in ../common/SoultreeObject.h (SoultreeObject, tier 2:
// 0x4fc630 SetPosition, 0x4fc660 SetPosition(Vec3), 0x4fc970 GetPosition, 0x4fc9a0
// GetPositionIn, 0x4fd5c0 LocalToWorldDirection, 0x4fd710 WorldToLocalDirection,
// 0x4fd7f0 WorldToLocalPoint).  Declared here with only the members this TU needs.
class SoultreeNode {
public:
    char pad_0x00[0x1a4];                                // sizeof, from the slot 2 allocation (0x1a4)
    SoultreeNode(int a);                                 // 0x004fb2b0 (thiscall, callee pops 4)
    void Fn_4fe850(SoultreeVec3* center, SoultreeVec3* extents); // bounds query (tier 3)
    void Fn_4fbd10(float a, float b, float c, float d, float e, float f, float g, int h);
    void Fn_4fd910(SoultreeNode* child);
    int Fn_4fc540(int a, SoultreeVec3& b, SoultreeVec3& c);
    int Fn_4fc050(int a, SoultreeVec3* b, SoultreeVec3* c, int d, int e);
    void Fn_4fc630(float x, float y, float z);           // SetPosition(x,y,z)
    void Fn_4fc660(SoultreeVec3* p);                     // SetPosition(const Vec3&)
    void Fn_4fc970(SoultreeVec3* out);                   // GetPosition (hidden-return pointer written in place; retail passes &field_0x0c)
    void Fn_4fc9a0(SoultreeNode* frame, SoultreeVec3* out); // GetPositionIn
    SoultreeVec3 Fn_4fd7f0(const SoultreeVec3* v);       // WorldToLocalPoint
    SoultreeVec3 Fn_4fd5c0(const SoultreeVec3* v);       // LocalToWorldDirection
    SoultreeVec3 Fn_4fd710(const SoultreeVec3* v);       // WorldToLocalDirection
    SoultreeVec3 Fn_4fd660(const SoultreeVec3* v);       // LocalToWorldPoint (tier 3, sibling of 4fd5c0)
};

// cdecl helpers outside this class.
extern "C++" {
// 0x004cb6e0
void Fn_4cb6e0(SoultreeVec3* a, SoultreeVec3* b, SoultreeVec3* c, float d, int e);

class SoultreeBody;
// 0x004b0df0 (17 args); j is an unsigned char (the caller zero-extends it with mov al)
int Fn_4b0df0(SoultreeBody* a, int b, SoultreeVec3* c, float d, int e, int f, int g, int h,
              float i, unsigned char j, SoultreeVec3* k, int l, int m, SoultreeVec3* n, SoultreeVec3* o,
              SoultreeVec3* p, int* q);
}


// TU-local solver helpers (cdecl), 0x00500220 (12 args) and 0x005004a0 (17 args).
void Fn_500220(float a, float b, SoultreeNode* node, const SoultreeVec3* a1, const SoultreeVec3* a2,
               const SoultreeVec3* a3, SoultreeVec3* e4, const SoultreeVec3* a4, SoultreeVec3* d8,
               SoultreeVec3* v64, float* a7, int a6);
void Fn_5004a0(float a, const SoultreeVec3* a1, float b, SoultreeNode* node, SoultreeVec3* l1,
               const SoultreeVec3* a4, SoultreeVec3* e4, SoultreeVec3* d8, SoultreeVec3* a2,
               int a6, int a7, SoultreeVec3* l2, const SoultreeVec3* a10, SoultreeVec3* a11,
               SoultreeVec3* a12, int a13, float* a14);


// 0x005015b0: out-of-line TU-local helper, cdecl: *out = *v * s, returns out.  Defined at the
// end of SoulTreePhysics.cpp so callers are not inlined against it (VC6 only inlines
// functions that are already defined at the call site).
SoultreeVec3* SoultreeScaleVec3(SoultreeVec3* out, const SoultreeVec3* v, float s);

#endif

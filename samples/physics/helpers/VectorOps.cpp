// VectorOps.cpp -- out-of-line instances of the Vec3 operators declared inline in
// ../common/Math3D.h. Retail emitted them as separate COMDAT functions (tier 2:
// decoded bodies equal the inline formulas; callers use a hidden result pointer).
#include "../common/Math3D.h"

// Suppress expansion so the inline members/operators get out-of-line bodies.
#pragma inline_depth(0)

// 0x00404e60. Vec3::Vec3(float, float, float), __thiscall ret 0xc.
Vec3 g_ProbeVec3Ctor(float x, float y, float z) { return Vec3(x, y, z); }

// 0x00421cb0 / 0x00421d00. operator+ / operator- (cdecl, hidden result first).
Vec3 g_ProbeVec3Add(const Vec3& a, const Vec3& b) { return a + b; }
Vec3 g_ProbeVec3Sub(const Vec3& a, const Vec3& b) { return a - b; }

// 0x005015b0 / 0x00428090. operator*(const Vec3&, float) / operator*(float, const Vec3&).
Vec3 g_ProbeVec3MulVS(const Vec3& v, float s) { return v * s; }
Vec3 g_ProbeVec3MulSV(float s, const Vec3& v) { return s * v; }

// 0x00428060. Vec3::operator+=(const Vec3&), __thiscall ret 4, returns this.
void g_ProbeVec3AddAssign(Vec3& a, const Vec3& b) { a += b; }

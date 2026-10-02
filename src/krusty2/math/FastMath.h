// FastMath.h -- the single declarations of the table-driven float helpers (defined in
// ../helpers/FastMath.cpp).  Kept apart from Math3D.h so translation units that model
// their vectors with their own types (and must not pick up Math3D.h's per-TU static
// Vec3 constants and their $E initializers) can still share these declarations.
#ifndef MCM2_PHYSICS_COMMON_FASTMATH_H
#define MCM2_PHYSICS_COMMON_FASTMATH_H

// 0x00460b50, 85 bytes, __cdecl, float result in st(0). Table-driven square root:
// returns 0 for x == 0, otherwise builds the result from the halved exponent and a
// 256-entry mantissa table at 0x005dafe0 (top 7 mantissa bits + exponent parity). No
// Newton step. Tier 2 semantics (decoded bit arithmetic), tier 3 name.
float FastSqrt(float x);

// 0x00460c00, 107 bytes, __cdecl, float result in st(0). Reciprocal square root: table
// estimate (0x005daf5c, 128 entries, exponent 0x5f000000 - (e << 22)) refined by two
// Newton steps y = 0.5 * y * (3 - x*y*y). Tier 2 semantics, tier 3 name.
float FastInvSqrt(float x);

#endif

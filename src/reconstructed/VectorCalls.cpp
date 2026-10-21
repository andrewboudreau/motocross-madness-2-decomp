// Complete explicit-output call views of the retail vector operators.
// The names and this reconstruction grouping are provisional, not recovered
// retail exports or translation-unit ownership. See docs/VECTOR_CALLS.md.
#include "../krusty2/math/Vec3.h"

// 0x00421cb0, 69 bytes. The temporary also preserves overlapping input/output
// behavior: each component is computed before the corresponding output copy.
Vec3* Vec3AddCall(Vec3* out, const Vec3* a, const Vec3* b)
{
    Vec3 value(a->x + b->x, a->y + b->y, a->z + b->z);
    *out = value;
    return out;
}

// 0x00421d00, 69 bytes; cdecl, explicit output followed by both input pointers.
Vec3* Vec3SubtractCall(Vec3* out, const Vec3* a, const Vec3* b)
{
    Vec3 value(a->x - b->x, a->y - b->y, a->z - b->z);
    *out = value;
    return out;
}

// 0x005015b0, 69 bytes. Each scalar product is rounded into the local vector
// before its bits are copied to the output, matching the out-of-line operator.
Vec3* Vec3ScaleCall(Vec3* out, const Vec3* v, float s)
{
    Vec3 value(s * v->x, s * v->y, s * v->z);
    *out = value;
    return out;
}

// 0x0040ae30, 29 bytes. Retail sums the y and x products, then adds z.
// Parentheses preserve that grouping under VC6 /O2 without rounding the
// intermediate x87 result to a float. Math3D.h's inline DotProduct is separate.
float Vec3DotCall(const Vec3* a, const Vec3* b)
{
    return a->z * b->z + (a->x * b->x + a->y * b->y);
}

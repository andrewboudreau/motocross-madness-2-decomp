// Group0_Math.cpp -- bring-up bodies for two small math helpers that other
// reconstructed code calls but nobody had written yet. Behavior follows the
// retail disassembly; byte matching was not attempted.
//
//   TransformPoints  0x004a1b00 (cdecl)
//   Vec3Normalize    0x005087b0, CollisionVec3 pointer form
#include "Matrix4.h"
#include "collision/CollisionPoint.h"
#include "math/FastMath.h"

// 0x004a1b00: transforms `count` points by `matrix` (row-vector convention:
// p' = p * M, translation row 3). Retail WRITES through the first argument
// and READS the second: its own caller 0x00447091 passes a fresh
// count * 12 byte buffer first with stride 0xc and the vertex array second
// with stride 0x20, so the first pair is (destination, destinationStride)
// and the last stride belongs to the source. A zero source stride means
// "same as the destination stride". Strides are in bytes. The first
// parameter keeps the declared `const void*` type so the symbol matches.
// Retail is inline assembly (EBP frame, both pointers biased to the end and
// indexed by a negated offset that counts up to zero). Audited for the linked
// game 2026-11-11: as there, the loop is a do-while that stops when the
// source offset reaches the end (so a zero stride on both sides transforms
// one point); each sum adds the x, y and z products then the translation in
// that order (written as compound adds: VC6 /O2 reassociates the single
// expression `x*a + y*b + z*c + t` into `(y*b + z*c) + x*a + t`); every
// coordinate is re-read from the source point; and all three results are
// computed before the first store (an in-place transform reads the old point).
void TransformPoints(const void* destinationPoints, void* sourcePoints, const Matrix4* matrix,
                     int count, int destinationStride, int sourceStride)
{
    if (count == 0)
        return;
    if (sourceStride == 0)
        sourceStride = destinationStride;
    unsigned char* to = (unsigned char*)destinationPoints;
    const unsigned char* from = (const unsigned char*)sourcePoints;
    const unsigned char* end = from + count * sourceStride;
    do {
        const float* p = (const float*)from;
        float* out = (float*)to;
        float x = p[0] * matrix->_11;
        x += p[1] * matrix->_21;
        x += p[2] * matrix->_31;
        float y = p[0] * matrix->_12;
        y += p[1] * matrix->_22;
        y += p[2] * matrix->_32;
        float z = p[0] * matrix->_13;
        z += p[1] * matrix->_23;
        z += p[2] * matrix->_33;
        x += matrix->_41;
        y += matrix->_42;
        z += matrix->_43;
        out[0] = x;
        out[1] = y;
        out[2] = z;
        to += destinationStride;
        from += sourceStride;
    } while (from != end);
}

// 0x005087b0 (the pointer form the collision code declares): out = src / |src|
// through the table reciprocal square root; a vector whose squared length is
// exactly 1.0f is copied unchanged. Returns `out`.
CollisionVec3* Vec3Normalize(CollisionVec3* out, const CollisionVec3* src)
{
    float y = src->y, x = src->x, z = src->z;
    float squared = z * z + (x * x + y * y);
    if (squared == 1.0f) {
        out->x = src->x;
        out->y = src->y;
        out->z = src->z;
        return out;
    }
    float inverse = FastInvSqrt(squared);
    float nx = inverse * src->x;
    float ny = inverse * src->y;
    float nz = inverse * src->z;
    out->x = nx;
    out->y = ny;
    out->z = nz;
    return out;
}

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
void TransformPoints(const void* destinationPoints, void* sourcePoints, const Matrix4* matrix,
                     int count, int destinationStride, int sourceStride)
{
    if (count == 0)
        return;
    if (sourceStride == 0)
        sourceStride = destinationStride;
    unsigned char* to = (unsigned char*)destinationPoints;
    const unsigned char* from = (const unsigned char*)sourcePoints;
    for (int i = 0; i < count; i++) {
        const float* p = (const float*)from;
        float x = p[0], y = p[1], z = p[2];
        float* out = (float*)to;
        out[0] = x * matrix->_11 + y * matrix->_21 + z * matrix->_31 + matrix->_41;
        out[1] = x * matrix->_12 + y * matrix->_22 + z * matrix->_32 + matrix->_42;
        out[2] = x * matrix->_13 + y * matrix->_23 + z * matrix->_33 + matrix->_43;
        to += destinationStride;
        from += sourceStride;
    }
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

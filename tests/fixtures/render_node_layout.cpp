#include <stddef.h>
#include "../../src/reconstructed/D3DIMSoulTree.h"
#include "../../src/krusty2/math/Vec3.h"

typedef char vector_size[sizeof(Vector3) == 12 && sizeof(Vec3) == 12 ? 1 : -1];
typedef char vector_x[offsetof(Vector3, x) == offsetof(Vec3, x) ? 1 : -1];
typedef char vector_y[offsetof(Vector3, y) == offsetof(Vec3, y) ? 1 : -1];
typedef char vector_z[offsetof(Vector3, z) == offsetof(Vec3, z) ? 1 : -1];
typedef char matrix_offset[offsetof(D3DIMSoultreeObject, localMatrix) == 0xb8 ? 1 : -1];
typedef char bounds_offset[offsetof(D3DIMSoultreeObject, subtreeBoundsCenter) == 0x174 ? 1 : -1];
typedef char extents_offset[offsetof(D3DIMSoultreeObject, subtreeBoundsHalfExtent) == 0x180 ? 1 : -1];
typedef char dirty_offset[offsetof(D3DIMSoultreeObject, subtreeDirty) == 0x18c ? 1 : -1];

// These inherited members belong to the complete SoultreeObject subobject.
void (SoultreeObject::*PositionMethod)(const Vec3&) = &D3DIMSoultreeObject::SetPosition;
void (SoultreeObject::*AxesMethod)(const Vec3*, const Vec3*, int, int) = &D3DIMSoultreeObject::SetAxesPtr;
void (SoultreeObject::*RotateMethod)(Vec3, float) = &D3DIMSoultreeObject::Rotate;
void (SoultreeObject::*BoundsMethod)(Vec3*, Vec3*) = &D3DIMSoultreeObject::GetSubtreeBounds;

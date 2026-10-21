// Complete in-place addition for the existing BVH call view.
// The provisional view is retained to preserve its out-of-line caller boundary.
// Original TU and class identity are unproven; see docs/VECTOR_ADD_ASSIGN.md.
#include "VectorAddAssign.h"

// 0x00428060, 33 bytes, thiscall ret4. Each component is read and written
// before reading the next component, including for overlapping vector storage.
Vec3AddAssignCall& Vec3AddAssignCall::operator+=(const Vec3& vector)
{
    x += vector.x;
    y += vector.y;
    z += vector.z;
    return *this;
}

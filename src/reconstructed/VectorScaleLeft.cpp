// Complete scalar-first vector helper reached by CapsuleTreeQuery.
// The helper name and this reconstruction grouping are provisional; the retail
// translation unit is unproven. See docs/VECTOR_SCALE_LEFT.md.
#include "../krusty2/math/Vec3.h"

// 0x00428090, 69 bytes. Cdecl hidden 12-byte output, then scalar and vector.
// All components are rounded into the temporary before the output is copied.
Vec3 Vec3ScaleLeft(float scale, const Vec3& vector)
{
    return Vec3(scale * vector.x, scale * vector.y, scale * vector.z);
}

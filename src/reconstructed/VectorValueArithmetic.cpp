// Existing collision and wrecker call views of the complete vector-value
// operators at 0x00421cb0 and 0x00421d00. Names and original TU are provisional;
// the three-float layouts, hidden output, and complete bodies are verified.
#include "math/Vec3.h"
#include "MatrixUtil.h"

Vec3 SegmentAdd(const Vec3& a, const Vec3& b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 SegmentSubtract(const Vec3& a, const Vec3& b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 WreckerSubtractValue(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

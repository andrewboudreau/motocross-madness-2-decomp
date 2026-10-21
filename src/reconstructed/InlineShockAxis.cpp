#include "Suspension.h"
#include "../krusty2/math/Vec3.h"
#include "Matrix4.h"
#include "../krusty2/core/SoultreeObject.h"
#include <math.h>

void InlineShock::UpdateAxis(ShockNode* a, ShockNode* b)
{
    ShockVec3 pa;
    ShockVec3 pb;
    reinterpret_cast<SoultreeObject*>(a)->GetPositionIn(0, reinterpret_cast<Vec3*>(&pa));
    reinterpret_cast<SoultreeObject*>(b)->GetPositionIn(0, reinterpret_cast<Vec3*>(&pb));
    axis = ShockVec3(pa.x - pb.x, pa.y - pb.y, pa.z - pb.z);
    float len = axis.LengthSquared();
    len = (len == 1.0f) ? 1.0f : (float)sqrt(len);
    axis = axis * (1.0f / (axisLength = len));
    if (len < minLength)
        minLength = len;
}

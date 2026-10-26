#include "Suspension.h"
#include "../krusty2/math/Vec3.h"
#include "Matrix4.h"
#include "../krusty2/core/SoultreeObject.h"
#include <math.h>

void InlineShock::UpdateAxis(ShockNode* wheelNode, ShockNode* mountNode)
{
    ShockVec3 wheelPoint;
    ShockVec3 mountPoint;
    reinterpret_cast<SoultreeObject*>(wheelNode)->GetPositionIn(0, reinterpret_cast<Vec3*>(&wheelPoint));
    reinterpret_cast<SoultreeObject*>(mountNode)->GetPositionIn(0, reinterpret_cast<Vec3*>(&mountPoint));
    axis = ShockVec3(wheelPoint.x - mountPoint.x, wheelPoint.y - mountPoint.y, wheelPoint.z - mountPoint.z);
    float len = axis.LengthSquared();
    len = (len == 1.0f) ? 1.0f : (float)sqrt(len);
    axis = axis * (1.0f / (axisLength = len));
    if (len < minLength)
        minLength = len;
}

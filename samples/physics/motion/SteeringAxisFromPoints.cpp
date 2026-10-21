// Complete steering-axis candidate, retail 0x00504d30..0x00504e19.
// Nonexact reconstruction; this standalone grouping does not prove original TU ownership.
#include "math/Math3D.h"
#include "motion/SteeringControl.h"
#include <math.h>

int SteeringControl::SetAxisFromPoints(SoultreeObject* from, SoultreeObject* to, SoultreeObject* frame)
{
    Vec3 fromPos;
    Vec3 toPos;
    from->GetPositionIn(0, &fromPos);
    to->GetPositionIn(0, &toPos);
    Vec3 d = fromPos - toPos;
    float lenSq = d.x * d.x + d.y * d.y;
    lenSq += d.z * d.z;
    float len = lenSq == 1.0f ? 1.0f : (float)sqrt(lenSq);
    float inv = 1.0f / len;
    Vec3 dir = Vec3(d.x * inv, d.y * inv, d.z * inv);
    if (frame) {
        axis = frame->WorldToLocalDirection(dir);
        return 1;
    }
    return 0;
}

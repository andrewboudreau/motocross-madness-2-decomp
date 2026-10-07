// Near miss for src/krusty2/motion/SteeringControl.cpp (TU SteeringControl.cpp).
// SteeringControl::SetAxisFromPoints (0x00504d30): retail spills y to a stack slot
// (60.7% relocation-masked); the flow and calls match. The promoted file supplies the
// rest of the unit.
#include "motion/SteeringControl.cpp"

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


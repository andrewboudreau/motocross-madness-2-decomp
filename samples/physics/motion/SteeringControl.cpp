// Near miss for src/krusty2/motion/SteeringControl.cpp (TU SteeringControl.cpp).
// SteeringControl::SetAxisFromPoints (0x00504d30, 145/234 strict): retail spills y to
// a stack slot and keeps d.x on the x87 stack (`fld st(0); fmul st(3)`), popping it
// at the end; the flow and calls match. Tried without gain: d * inv, d * (1 / len),
// inv * d.x per component (all 64 operand-order combinations), d *= inv, d / len, a
// component-wise d, other lenSq term orders, SquareMagnitude/DotProduct helpers and
// separate dir stores, a scale helper (with or without component copies, products
// first, 1 / len argument), copies of d or of d.x/d.z for the length or the scale
// (all 145; component stores 109). The promoted file supplies the rest of the unit.
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


#include "CollisionPoint.h"

inline CollisionPoint::CollisionPoint(float a, int b)
    : field_0x88(a), field_0xc0(b) {
    field_0x08 = g_CollisionZeroVec3;
    field_0x04 = 0;
    field_0x14 = g_CollisionZeroVec3;
    field_0x20 = g_CollisionZeroVec3;
    field_0x2c = g_CollisionVec3_579810;
    field_0x38 = g_CollisionZeroVec3;
    field_0x44 = g_CollisionZeroVec3;
    field_0x50 = g_CollisionZeroVec3;
    field_0x5c = g_CollisionZeroVec3;
    field_0x90 = 0;
    field_0x94 = 0;
    field_0xb8 = 1.0f;
    field_0x8c = 1.0f;
    field_0x98 = -999.0f;               // 0xc479c000 at 0x0043a48c
    field_0x9c = 0;
    field_0xa4 = 0;
    field_0xa8 = 0;
    field_0xac = 0;
    field_0xb0 = 0;
    field_0xb4 = 0;
    field_0xa0 = 0.5f;
    field_0xbc = 0;
    field_0x84 = 0;
    field_0x78 = g_CollisionZeroVec3;
    field_0x74 = 0;
    field_0x68 = g_CollisionZeroVec3;
}

void CollisionPoint::CollisionPointVirtualSlot1() {
    if (!(field_0x90 > 0.001f) && !(field_0x94 > 0.001f)) {
        field_0x84 = 0.0f;
        field_0x78 = g_CollisionZeroVec3;
    } else {
        field_0x84 = (-field_0x88) * field_0x8c * field_0x74;
        field_0x78 = field_0x50 * field_0x84;
        if (field_0x84 < 0.0f)
            field_0x84 = -field_0x84;
    }
}

CollisionPoint* AddCollisionPoint(int capacity, CollisionPoint** points, const CollisionVec3* position,
                                  CollisionContactOwner* owner, float a4, int* count, float a6,
                                  int a7) {
    if (*count < capacity) {
        points[*count] = new(__FILE__, 27) CollisionPoint(a6, a7);
        CollisionPoint* p = points[*count];
        p->field_0x08 = *position;
        p->field_0x98 = a4 - 999.0f;
        p->field_0x04 = owner;
        p->field_0x9c = a4;
        (*count)++;
        return p;
    }
    return 0;
}

float FastSqrt(float x);      // 0x00460b50
float FastInvSqrt(float x);   // 0x00460c00

// 0x0043a640.  Tier 3 semantics.  Given the contact normal n = field_0x2c:
//   field_0x38 = field_0x14 - a1                      (position relative to a1)
//   c          = (n * (n . a2)) x a4                   (normal component of a2 crossed with a4)
//   a5 > 0.001: t = a3 with its component along n removed and renormalised; field_0x90 = t . a3;
//               if that is positive the tangent is normalise(t * field_0x90 + c),
//               otherwise normalise(c); field_0x94 = |c|
//   otherwise:  field_0x90 = 0, tangent = normalise(c), field_0x94 = |c|
// The tangent is stored in field_0x50; a degenerate (|c| <= 0.001) tangent is zero.
void CollisionPoint::Fn_0043a640(const CollisionVec3* a1, const CollisionVec3* a2, const CollisionVec3* a3,
                                 const CollisionVec3* a4, float a5)
{
    field_0x38 = field_0x14 - *a1;

    float s = field_0x2c.y * a2->y + field_0x2c.x * a2->x + field_0x2c.z * a2->z;
    CollisionVec3 u = field_0x2c * s;
    CollisionVec3 c = CollisionCross(u, *a4);

    if (a5 > 0.001f) {
        CollisionVec3 t;
        if (CollisionRejectFrom(&t, a3, &field_0x2c)) {
            CollisionVec3 n;
            t = *Fn_005087b0(&n, &t);
        }
        float f = Fn_0040ae30(&t, a3);
        field_0x90 = f;
        if (f > 0.001f) {
            CollisionVec3 v1 = CollisionVec3(t.x * f, t.y * f, t.z * f);
            CollisionVec3 w = CollisionVec3(v1.x + c.x, v1.y + c.y, v1.z + c.z);
            float m = Fn_0040ae30(&w, &w);
            float len;
            if (m == 0.0f)
                len = 0.0f;
            else if (m == 1.0f)
                len = 1.0f;
            else
                len = 1.0f / FastInvSqrt(m);
            float k = 1.0f / len;
            field_0x50 = CollisionVec3(w.x * k, w.y * k, w.z * k);
            float cm = Fn_0040ae30(&c, &c);
            if (cm == 1.0f)
                field_0x94 = 1.0f;
            else
                field_0x94 = FastSqrt(cm);
        } else {
            field_0x90 = 0.0f;
            float cm = Fn_0040ae30(&c, &c);
            float len;
            if (cm == 1.0f)
                len = 1.0f;
            else
                len = FastSqrt(cm);
            field_0x94 = len;
            if (len > 0.001f) {
                float k = 1.0f / len;
                field_0x50 = CollisionVec3(c.x * k, c.y * k, c.z * k);
            } else {
                field_0x50 = g_CollisionZeroVec3;
                field_0x94 = 0.0f;
            }
        }
    } else {
        field_0x90 = 0.0f;
        float len = CollisionLength(&c);
        field_0x94 = len;
        if (len > 0.001f) {
            CollisionVec3 d;
            field_0x50 = *CollisionDivide(&d, &c, len);
        } else {
            field_0x50 = g_CollisionZeroVec3;
            field_0x94 = 0.0f;
        }
    }
}

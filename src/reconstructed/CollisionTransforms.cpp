// Collision transform helpers, grouped by use; original TU ownership is unknown.
// Complete bodies at 0x00428db0 (789 bytes) and 0x0042e2b0 (144 bytes).
#include "bvh/BoundingBoxTreeQuery.h"
#include "CollisionTransforms.h"

// MoveBox's transform: the x and z rows sum z, y, x, which also gives retail's slot order for
// d and half; the x row's first two products are grouped (with the point through a local
// pointer, below, this makes both rows load the point's component before the matrix element).
inline void TransformPointPtrInline(Vec3* out, const Vec3* v, const Matrix4* m)
{
    out->x = (v->z * m->_31 + v->y * m->_21) + v->x * m->_11 + m->_41;
    out->y = v->x * m->_12 + v->y * m->_22 + v->z * m->_32 + m->_42;
    out->z = v->z * m->_33 + v->y * m->_23 + v->x * m->_13 + m->_43;
}
// 0x00428db0.  Grows the box (center, halfExtents) in place to also contain itself moved by m.
void MoveBox(Vec3* center, Vec3* halfExtents, const Matrix4* m)
{
    Vec3 hx(halfExtents->x, 0.0f, 0.0f);
    Vec3 hy(0.0f, halfExtents->y, 0.0f);
    Vec3 hz(0.0f, 0.0f, halfExtents->z);
    Vec3 axes[3];
    RotateVectorInline(&axes[0], hx, m);
    RotateVectorInline(&axes[1], hy, m);
    RotateVectorInline(&axes[2], hz, m);
    // Assigned after the last rotation: VC6 hoists the x and y copies but keeps the z copy
    // after the last read of hz.z, whose slot it shares.
    Vec3 grown;
    grown.x = halfExtents->x;
    grown.y = halfExtents->y;
    grown.z = halfExtents->z;
    Vec3& ax = axes[0];
    Vec3& ay = axes[1];
    Vec3& az = axes[2];
    Vec3 half;
    half.x = QueryAbs(ax.x) + QueryAbs(ay.x) + QueryAbs(az.x);
    half.y = QueryAbs(ax.y) + QueryAbs(ay.y) + QueryAbs(az.y);
    half.z = QueryAbs(ax.z) + QueryAbs(ay.z) + QueryAbs(az.z);
    Vec3 moved;
    const Vec3* point = center;
    TransformPointPtrInline(&moved, point, m);
    Vec3 d;
    d.x = moved.x - center->x;
    d.y = moved.y - center->y;
    d.z = moved.z - center->z;
    Vec3 mid;
    for (int i = 0; i < 3; i++) {
        float lo = d[i] - half[i];
        float hi = half[i] + d[i];
        if (-grown[i] < lo)
            lo = -grown[i];
        hi = QueryMax(grown[i], hi);
        mid[i] = (hi + lo) * 0.5f;
        grown[i] = (hi - lo) * 0.5f;
    }
    *center += mid;
    *halfExtents = grown;
}

// 0x0042e2b0. Transpose rotation before computing all three translated
// components, then store them; the input/output matrix is the same object.
void InvertRigidTransform(Matrix4* matrix)
{
    InvertRigid(matrix);
}

// Near-miss GraphicsTest.cpp candidates (src/krusty2/core/GraphicsTest.cpp), kept out of
// src/krusty2 until they match.
//
// 0x0047bd10 (circle, 917 bytes): same calls (two inlined and two out-of-line cross
// products, two Vec3Normalize calls), loop and vertex fill. The frame is 0xb84 against
// retail's 0xb78 (one more Vec3 temporary), and VC6 here picks the other operand order in
// several products of the basis and of the point transform; the source term order does
// not change it.
//
// 0x0047c270 (DrawBox, 626 bytes): 622 of 626 positions; the z sum of `corners[i] +=
// *center` loads center.z first in retail (x and y match).
#include <math.h>

#include "core/GraphicsTest.h"
#include "math/Math3D.h"

struct GraphicsTestVertex {
    float x;
    float y;
    float z;
    int reserved;
    int color;
    int specular;
    float tu;
    float tv;
};

class GraphicsTestRenderer {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6(); virtual void Slot7();
    virtual void Slot8(); virtual void Slot9(); virtual void Slot10(); virtual void Slot11();
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    virtual int DrawPrimitive(int type, int fvf, void* vertices, int vertexCount, int flags);
};

#define RENDERER() ((GraphicsTestRenderer*)field_0x18)

// Row-vector transform with translation (row 3).
inline void GraphicsTestTransform(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// 0x0047bd10: a circle of `segments` line segments (at most 64) around `axis` (the
// circle's normal; null draws it in the xz plane).
void GraphicsTest::Fn_0047bd10(const Vec3* center, float radius, const Vec3* axis, int segments)
{
    Matrix4 m;
    Vec3 points[64];
    GraphicsTestVertex vertices[64];
    int i;

    if (segments > 64) {
        segments = 64;
    }
    if (axis) {
        Vec3 u = CrossProduct(*axis, Vec3(axis->z, axis->x, axis->y));
        Vec3 n = *axis;
        Vec3 v = CrossProduct(n, u);
        n = CrossProductCall(u, v);
        n = Vec3Normalize(n);
        u = Vec3Normalize(u);
        v = CrossProductCall(n, u);
        m._11 = v.x;
        m._12 = v.y;
        m._13 = v.z;
        m._21 = n.x;
        m._22 = n.y;
        m._23 = n.z;
        m._31 = u.x;
        m._32 = u.y;
        m._33 = u.z;
        m._41 = 0;
        m._42 = 0;
        m._43 = 0;
    }
    for (i = 0; i < segments; i++) {
        float angle = i * (6.2831855f / segments);
        points[i].x = cos(angle) * radius;
        points[i].y = 0;
        points[i].z = sin(angle) * radius;
        if (axis) {
            GraphicsTestTransform(&points[i], points[i], &m);
        }
        points[i] += *center;
    }
    points[segments] = points[0];
    for (i = 0; i < segments * 2; i++) {
        vertices[i].color = drawColor;
        vertices[i].specular = field_0x30;
        vertices[i].tu = 0;
        vertices[i].tv = 0;
    }
    for (i = 0; i < segments; i++) {
        vertices[i * 2].x = points[i].x;
        vertices[i * 2].y = points[i].y;
        vertices[i * 2].z = points[i].z;
        vertices[i * 2 + 1].x = points[i + 1].x;
        vertices[i * 2 + 1].y = points[i + 1].y;
        vertices[i * 2 + 1].z = points[i + 1].z;
    }
    RENDERER()->DrawPrimitive(2, 0x1e2, vertices, segments * 2, 0);
}

// 0x0047c270
void GraphicsTest::DrawBox(const Vec3* center, const Vec3* halfExtents, const Matrix4* xf)
{
    unsigned short edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7},
    };
    Vec3 corners[8];
    int i;

    corners[0].x = -halfExtents->x;
    corners[0].y = halfExtents->y;
    corners[0].z = halfExtents->z;
    corners[1].x = halfExtents->x;
    corners[1].y = halfExtents->y;
    corners[1].z = halfExtents->z;
    corners[2].x = halfExtents->x;
    corners[2].y = halfExtents->y;
    corners[2].z = -halfExtents->z;
    corners[3].x = -halfExtents->x;
    corners[3].y = halfExtents->y;
    corners[3].z = -halfExtents->z;
    corners[4].x = -halfExtents->x;
    corners[4].y = -halfExtents->y;
    corners[4].z = halfExtents->z;
    corners[5].x = halfExtents->x;
    corners[5].y = -halfExtents->y;
    corners[5].z = halfExtents->z;
    corners[6].x = halfExtents->x;
    corners[6].y = -halfExtents->y;
    corners[6].z = -halfExtents->z;
    corners[7].x = -halfExtents->x;
    corners[7].y = -halfExtents->y;
    corners[7].z = -halfExtents->z;
    for (i = 0; i < 8; i++) {
        corners[i] += *center;
        if (xf) {
            GraphicsTestTransform(&corners[i], corners[i], xf);
        }
    }
    for (i = 0; i < 12; i++) {
        DrawLine(&corners[edges[i][0]], &corners[edges[i][1]]);
    }
}

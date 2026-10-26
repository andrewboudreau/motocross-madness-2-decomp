// Near-miss GraphicsTest.cpp candidates (src/krusty2/core/GraphicsTest.cpp), kept out of
// src/krusty2 until they match.
//
// 0x0047bd10 (circle, 917 bytes, 818 strict): same calls (two inlined and two out-of-line
// cross products, two Vec3Normalize calls), loop and vertex fill. With Math3D.h's
// parenthesised CrossProduct the basis products match retail's operand order and schedule
// except one load of the first inline cross product (issued one instruction early). Left:
// the frame (0xb84 against retail's 0xb78, one more Vec3 temporary), the `[eax - 4]` store of
// the vertex fill, one x87 load scheduled later in the point loop and the operand order of
// `points[i] += *center` (retail loads the centre first).
//
// A `Vec3* point = &points[i]` local for the transform and the centre sum gives 818 (the same
// pointer local made DrawBox 0x0047c270 exact, now in src/krusty2/core/GraphicsTest.cpp);
// using it for the cos/sin stores as well gives 799.
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

#define RENDERER() ((GraphicsTestRenderer*)renderContext)

// Row-vector transform with translation (row 3).
inline void GraphicsTestTransform(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// 0x0047bd10: a circle of `segments` line segments (at most 64) around `axis` (the
// circle's normal; null draws it in the xz plane).
void GraphicsTest::DrawCircle(const Vec3* center, float radius, const Vec3* axis, int segments)
{
    Matrix4 basis;
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
        basis._11 = v.x;
        basis._12 = v.y;
        basis._13 = v.z;
        basis._21 = n.x;
        basis._22 = n.y;
        basis._23 = n.z;
        basis._31 = u.x;
        basis._32 = u.y;
        basis._33 = u.z;
        basis._41 = 0;
        basis._42 = 0;
        basis._43 = 0;
    }
    for (i = 0; i < segments; i++) {
        float angle = i * (6.2831855f / segments);
        points[i].x = cos(angle) * radius;
        points[i].y = 0;
        points[i].z = sin(angle) * radius;
        Vec3* point = &points[i];
        if (axis) {
            GraphicsTestTransform(point, *point, &basis);
        }
        *point += *center;
    }
    points[segments] = points[0];
    for (i = 0; i < segments * 2; i++) {
        vertices[i].color = drawColor;
        vertices[i].specular = specularColor;
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

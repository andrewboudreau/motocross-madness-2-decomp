// GraphicsTest.cpp -- GraphicsTest : GameObject (see GraphicsTest.h), the debug drawing base of
// CollisionObject, PhysicsBody, the soultree modifiers and SoultreeQuadTreeRenderer.
//
// Code 0x0047bc70..0x0047c87c: the constructor, the destructor, the line/circle/sphere/box
// drawing helpers, then the Math3D.h vector set (.CRT$XCU 132-135, globals 0x0065b620..
// 0x0065b658, no readers) that closes the unit.  It sits after the GhostMod1 code and before
// Grid1.cpp (first xref 0x0047c97a).  No __FILE__ literal: the file name is ours (tier 3);
// the class is RTTI-confirmed (tier 1) and the helpers are tier 2 by position and callers.
#include <math.h>

#include "core/GraphicsTest.h"
#include "core/DebugAlloc.h"
#include "math/Math3D.h"

// D3DLVERTEX layout (FVF 0x1e2): position, reserved, colour, specular, texture coordinates.
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

// The render target at GameObject::renderContext (RTTI PCRenderTarget; the
// src/reconstructed RenderTarget.h slot numbers).  Local view, tier 3.
class GraphicsTestRenderer {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6();
    virtual long SetTextureStageState(int stage, int type, int value);  // slot 7
    virtual void SetRenderState(int state, int value, int force);       // slot 8
    virtual void Slot9(); virtual void Slot10();
    virtual long SetTexture(int stage);                                  // slot 11
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    // slot 16: (primitive type, FVF, vertices, vertex count, flags).
    virtual int DrawPrimitive(int type, int fvf, void* vertices, int vertexCount, int flags);
};

#define RENDERER() ((GraphicsTestRenderer*)renderContext)
#define RGBA_MAKE(r, g, b, a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

// Row-vector transform with translation (row 3).
inline void GraphicsTestTransform(Vec3* out, Vec3 v, const Matrix4* m)
{
    out->x = v.x * m->_11 + v.y * m->_21 + v.z * m->_31 + m->_41;
    out->y = v.x * m->_12 + v.y * m->_22 + v.z * m->_32 + m->_42;
    out->z = v.x * m->_13 + v.y * m->_23 + v.z * m->_33 + m->_43;
}

// 0x0047bc70
GraphicsTest::GraphicsTest(int flags)
    : GameObject(flags)
{
    SetDrawColor(0xff, 0xff, 0xff);
    specularColor = 0;
}

// 0x0047bd00 (deleting wrapper 0x0047bce0)
GraphicsTest::~GraphicsTest()
{
}

// 0x0047bd10. Not exact (836 of 917 bytes, 268 of 273 instructions: five x87
// operand orders; the x component of the first inline cross product, the x row of
// the transform, where retail loads the matrix element first, and the x centre sum,
// where retail loads the point first. Rows y and z match).
// A circle of `segments` line segments (at most 64) around `axis` (the circle's
// normal; null draws it in the xz plane). Storing the point as x, z, y (the
// constant y last) puts the `[eax - 4]` store where retail has it, and passing
// `points[i]` rather than `*point` as the transform's by-value argument gives
// retail's z centre sum.
// Forms tried, all 836 unless noted: matrix by reference, the three other
// parameter orders, a caller-side Vec3 copy, parenthesised or regrouped row sums,
// matrix-first products in the source (`m->_11 * v.x`), a transform that copies
// `*out` itself (pointer or reference), a component-wise centre sum. Worse:
// `const Vec3* c = center; *point += *c` 834 (fixes the x sum, flips the y sum),
// `const Vec3& in` copied inside the helper 834, float locals for the components
// 832, a `const Matrix4* m = &basis` local 828, `points[i] += *center` 828,
// `&points[i]` as the output argument 828, a Vec3-returning helper 528,
// `*point = *point + *center` 500. Before: y stored second with `*point` as the
// argument 818; the pointer local also used for the cos/sin stores 799 (the same
// pointer local made DrawBox exact).
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
        points[i].z = sin(angle) * radius;
        points[i].y = 0;
        Vec3* point = &points[i];
        if (axis) {
            GraphicsTestTransform(point, points[i], &basis);
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

// 0x0047c0b0: a wire sphere: `segments` circles about the y axis, then about the z axis.
void GraphicsTest::DrawSphere(const Vec3* center, float radius, int segments)
{
    int i;
    Vec3 axis = Vec3(0.0f, 1.0f, 0.0f);
    Vec3 ringCenter = *center;
    ringCenter.y -= radius;
    for (i = 0; i < segments; i++) {
        float height = radius - i * (radius / (segments * 0.5f));
        float ringRadius = FastSqrt(radius * radius - height * height);
        DrawCircle(&ringCenter, ringRadius, &axis, segments);
        ringCenter.y += radius / (segments * 0.5f);
    }
    axis = Vec3(0.0f, 0.0f, 1.0f);
    ringCenter = *center;
    ringCenter.z -= radius;
    for (i = 0; i < segments; i++) {
        float height = radius - i * (radius / (segments * 0.5f));
        float ringRadius = FastSqrt(radius * radius - height * height);
        DrawCircle(&ringCenter, ringRadius, &axis, segments);
        ringCenter.z += radius / (segments * 0.5f);
    }
}

// 0x0047c270: a wire box of half extents `halfExtents` about `center`, optionally transformed
// by `xf` (12 edges of the 8 corners).
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
        // the corner pointer local gives retail's center-first z sum
        // (corners[i] += *center loads corner.z first, 622/626)
        Vec3* corner = &corners[i];
        *corner += *center;
        if (xf) {
            GraphicsTestTransform(corner, *corner, xf);
        }
    }
    for (i = 0; i < 12; i++) {
        DrawLine(&corners[edges[i][0]], &corners[edges[i][1]]);
    }
}

// 0x0047c4f0
void GraphicsTest::DrawLine(const Vec3* from, const Vec3* to)
{
    GraphicsTestVertex vertices[2];
    for (int i = 0; i < 2; i++) {
        vertices[i].color = drawColor;
        vertices[i].specular = specularColor;
        vertices[i].tu = 0;
        vertices[i].tv = 0;
    }
    vertices[0].x = from->x;
    vertices[0].y = from->y;
    vertices[0].z = from->z;
    vertices[1].x = to->x;
    vertices[1].y = to->y;
    vertices[1].z = to->z;
    RENDERER()->DrawPrimitive(2, 0x1e2, vertices, 2, 0);
}

// 0x0047c570: three axis-aligned lines of length `size` through p.
void GraphicsTest::DrawMarker(const Vec3* p, float size)
{
    GraphicsTestVertex vertices[2];
    for (int i = 0; i < 2; i++) {
        vertices[i].color = drawColor;
        vertices[i].specular = specularColor;
        vertices[i].tu = 0;
        vertices[i].tv = 0;
    }
    size *= 0.5f;
    vertices[0].x = p->x - size;
    vertices[0].y = p->y;
    vertices[0].z = p->z;
    vertices[1].x = p->x + size;
    vertices[1].y = p->y;
    vertices[1].z = p->z;
    RENDERER()->DrawPrimitive(2, 0x1e2, vertices, 2, 0);
    vertices[0].x = p->x;
    vertices[0].y = p->y - size;
    vertices[0].z = p->z;
    vertices[1].x = p->x;
    vertices[1].y = p->y + size;
    vertices[1].z = p->z;
    RENDERER()->DrawPrimitive(2, 0x1e2, vertices, 2, 0);
    vertices[0].x = p->x;
    vertices[0].y = p->y;
    vertices[0].z = p->z - size;
    vertices[1].x = p->x;
    vertices[1].y = p->y;
    vertices[1].z = p->z + size;
    RENDERER()->DrawPrimitive(2, 0x1e2, vertices, 2, 0);
}

// 0x0047c690
void GraphicsTest::SetDrawColor(int r, int g, int b)
{
    drawColor = RGBA_MAKE(r, g, b, 0xff);
}

// 0x0047c6c0
void GraphicsTest::SetDrawRGBA(int r, int g, int b, int a)
{
    drawColor = RGBA_MAKE(r, g, b, a);
}

// 0x0047c6f0: untextured, diffuse alpha, alpha blending on.
void GraphicsTest::BeginAlphaBlend()
{
    RENDERER()->SetTexture(0);
    RENDERER()->SetTextureStageState(0, 1, 1);
    RENDERER()->SetTextureStageState(0, 4, 2);
    RENDERER()->SetTextureStageState(0, 5, 0);
    RENDERER()->SetRenderState(0x1b, 1, 0);
}

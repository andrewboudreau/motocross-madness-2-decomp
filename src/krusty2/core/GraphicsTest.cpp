// GraphicsTest.cpp -- GraphicsTest : GameObject (see GraphicsTest.h), the debug drawing base of
// CollisionObject, PhysicsBody, the soultree modifiers and SoultreeQuadTreeRenderer.
//
// Code 0x0047bc70..0x0047c87c: the constructor, the destructor, the line/circle/sphere/box
// drawing helpers, then the Math3D.h vector set (.CRT$XCU 132-135, globals 0x0065b620..
// 0x0065b658, no readers) that closes the unit.  It sits after the GhostMod1 code and before
// Grid1.cpp (first xref 0x0047c97a).  No __FILE__ literal: the file name is ours (tier 3);
// the class is RTTI-confirmed (tier 1) and the helpers are tier 2 by position and callers.
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

// The render target at GameObject::field_0x18 (RTTI PCRenderTarget; the
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

#define RENDERER() ((GraphicsTestRenderer*)field_0x18)
#define RGBA_MAKE(r, g, b, a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

// The circle 0x0047bd10 and DrawBox 0x0047c270 are near misses:
// samples/physics/helpers/GraphicsTestNearMisses.cpp.

// 0x0047bc70
GraphicsTest::GraphicsTest(int flags)
    : GameObject(flags)
{
    SetDrawColor(0xff, 0xff, 0xff);
    field_0x30 = 0;
}

// 0x0047bd00 (deleting wrapper 0x0047bce0)
GraphicsTest::~GraphicsTest()
{
}

// 0x0047c0b0: a wire sphere: `segments` circles about the y axis, then about the z axis.
void GraphicsTest::DrawSphere(const Vec3* center, float radius, int segments)
{
    int i;
    Vec3 axis = Vec3(0.0f, 1.0f, 0.0f);
    Vec3 p = *center;
    p.y -= radius;
    for (i = 0; i < segments; i++) {
        float h = radius - i * (radius / (segments * 0.5f));
        float r = FastSqrt(radius * radius - h * h);
        DrawCircle(&p, r, &axis, segments);
        p.y += radius / (segments * 0.5f);
    }
    axis = Vec3(0.0f, 0.0f, 1.0f);
    p = *center;
    p.z -= radius;
    for (i = 0; i < segments; i++) {
        float h = radius - i * (radius / (segments * 0.5f));
        float r = FastSqrt(radius * radius - h * h);
        DrawCircle(&p, r, &axis, segments);
        p.z += radius / (segments * 0.5f);
    }
}

// 0x0047c4f0
void GraphicsTest::DrawLine(const Vec3* a, const Vec3* b)
{
    GraphicsTestVertex vertices[2];
    for (int i = 0; i < 2; i++) {
        vertices[i].color = drawColor;
        vertices[i].specular = field_0x30;
        vertices[i].tu = 0;
        vertices[i].tv = 0;
    }
    vertices[0].x = a->x;
    vertices[0].y = a->y;
    vertices[0].z = a->z;
    vertices[1].x = b->x;
    vertices[1].y = b->y;
    vertices[1].z = b->z;
    RENDERER()->DrawPrimitive(2, 0x1e2, vertices, 2, 0);
}

// 0x0047c570: three axis-aligned lines of length `size` through p.
void GraphicsTest::DrawMarker(const Vec3* p, float size)
{
    GraphicsTestVertex vertices[2];
    for (int i = 0; i < 2; i++) {
        vertices[i].color = drawColor;
        vertices[i].specular = field_0x30;
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

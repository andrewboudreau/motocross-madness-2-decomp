// D3DIMSoultreeShadowNearMisses.cpp -- near misses of D:\aardvark\VC\krusty2\D3DIMSoultreeShadow.cpp
// (canonical source src/krusty2/shadow/D3DIMSoultreeShadow.cpp; slots 27, 29 and 30 of
// D3DIMSoultreeShadow, vtable 0x0055156c, vtable_overrides.json).  Kept here, not registered.
//
// Slot 27 (0x004468f0, 767 B) and slot 29 (0x00446c30, 771 B) are the same visibility test with
// a different camera: the caster's matrix (0x004fca80 with a null frame) times the camera matrix
// at camera+0xec, written out as 16 four-term sums into a local, then the visibility clipper's
// box test (0x0052f570) with the caster's bounds (0x004fe850).  Everything but the x87 operand
// order of the product matches retail.  The order is chosen by VC6 itself: the source term order
// of a plain in-function product has no effect, while the matrix type of the destination and an
// inline product helper do.  Forms tried for slot 27 (elements whose term order matches retail,
// of 16): float m[4][4] everywhere 8; named _11.._44 destination 11; union (_11.. and m[4][4],
// as Math3D.h's Matrix4) destination and camera 12; the inline helper below with union types
// 13 (row 2, elements _21.._23, differ); local declaration order, b*a operand order, the
// soultree.cpp MatrixProduct argument convention and dummy declarations before the types change
// nothing.  A non-natural term order in _21 reaches 15 of 16, no pair of reorders reaches 16.
// Slot 30 (0x00446f40, 1524 B) builds the receiver's shadow vertices from the caster's current
// LOD meshes (TerrainShadow slot 30 0x00509aa0 is its terrain twin).  Inherent near miss: retail
// rounds the clipped points with an inline __asm fld/fistp helper (float temp at ebp-8 / ebp-0x14,
// pointer at ebp-0x10, `fld; mov eax,[ptr]; fistp [eax]`), which forces its ebp frame; the
// (int) casts here call __ftol instead.
#include "shadow/D3DIMSoultreeShadow.h"
#include "../../../src/reconstructed/ClipRectangle.h"

// Row-major 4x4 matrix with the D3DMATRIX member names and the index view (Math3D.h's
// Matrix4 shape; the shadow TU's ShadowMatrix only has m[4][4]).  Local view, tier 3.
struct D3DIMShadowMatrix4 {
    D3DIMShadowMatrix4() {}
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};

// Triangle mesh of a render-model LOD, 0x38 bytes (BoundingBoxTreeBuild.h TreeModelMesh).
struct D3DIMShadowMesh {
    int partCount;                  // +0x00
    void* parts;                    // +0x04
    int vertexCount;                // +0x08
    int triangleCount;              // +0x0c
    float* vertices;                // +0x10 0x20-byte vertices, position first
    void* field_0x14;
    int field_0x18;
    unsigned short* triangles;      // +0x1c three indices per triangle
    char pad_0x20[0x38 - 0x20];
};
struct D3DIMShadowLod {
    int meshCount;                  // +0x00
    D3DIMShadowMesh* meshes;        // +0x04
};

// The caster as slots 27, 29 and 30 use it (a D3DIMSoultreeObject; BoundingBoxTreeBuild.h
// TreeModelSource).  Local view, tier 3.
class D3DIMShadowCasterView {
public:
    void GetMatrixIn(void* frame, D3DIMShadowMatrix4* out);           // 0x004fca80
    void GetBounds(ShadowVec3* center, ShadowVec3* halfExtents);      // 0x004fe850
    void SelectLod(int lod);                                          // 0x00440d40
    char pad_0x00[0x14c];
    int field_0x14c;                // +0x14c nonzero skips the visibility test
    char pad_0x150[0x18c - 0x150];
    int field_0x18c;                // +0x18c nonzero: slot 30 calls SelectLod(-1) first
    char pad_0x190[0x27c - 0x190];
    int currentLod;                 // +0x27c
    char pad_0x280[0x28c - 0x280];
    D3DIMShadowLod* lods;           // +0x28c
};
class D3DIMShadowCameraView {
public:
    char pad_0x00[0xec];
    D3DIMShadowMatrix4 matrix_0xec; // +0xec
};
// The render target at GameObject::field_0x18 (slot 29 tests against its camera at +8).
class D3DIMShadowHostView {
public:
    void* vtable;                   // +0x00
    void* display;                  // +0x04
    D3DIMShadowCameraView* camera;  // +0x08
};
// The object at 0x00575a98 (src/krusty2/visibility/VisibilityQuadTree.h VisibilityClipper).
class D3DIMShadowClipper {
public:
    int TestBox(D3DIMShadowCameraView* camera, const float* matrix, const float* center,
                const float* extent, int* screenRect, int* cornersInside, int unused);  // 0x0052f570
};
extern D3DIMShadowClipper* g_visibilityClipper;   // 0x00575a98

static inline void D3DIMShadowMatrixProduct(D3DIMShadowMatrix4* out, const D3DIMShadowMatrix4& a,
                                            const D3DIMShadowMatrix4& b)
{
    out->_11 = a._11 * b._11 + a._12 * b._21 + a._13 * b._31 + a._14 * b._41;
    out->_12 = a._11 * b._12 + a._12 * b._22 + a._13 * b._32 + a._14 * b._42;
    out->_13 = a._11 * b._13 + a._12 * b._23 + a._13 * b._33 + a._14 * b._43;
    out->_14 = a._11 * b._14 + a._12 * b._24 + a._13 * b._34 + a._14 * b._44;
    out->_21 = a._21 * b._11 + a._22 * b._21 + a._23 * b._31 + a._24 * b._41;
    out->_22 = a._21 * b._12 + a._22 * b._22 + a._23 * b._32 + a._24 * b._42;
    out->_23 = a._21 * b._13 + a._22 * b._23 + a._23 * b._33 + a._24 * b._43;
    out->_24 = a._21 * b._14 + a._22 * b._24 + a._23 * b._34 + a._24 * b._44;
    out->_31 = a._31 * b._11 + a._32 * b._21 + a._33 * b._31 + a._34 * b._41;
    out->_32 = a._31 * b._12 + a._32 * b._22 + a._33 * b._32 + a._34 * b._42;
    out->_33 = a._31 * b._13 + a._32 * b._23 + a._33 * b._33 + a._34 * b._43;
    out->_34 = a._31 * b._14 + a._32 * b._24 + a._33 * b._34 + a._34 * b._44;
    out->_41 = a._41 * b._11 + a._42 * b._21 + a._43 * b._31 + a._44 * b._41;
    out->_42 = a._41 * b._12 + a._42 * b._22 + a._43 * b._32 + a._44 * b._42;
    out->_43 = a._41 * b._13 + a._42 * b._23 + a._43 * b._33 + a._44 * b._43;
    out->_44 = a._41 * b._14 + a._42 * b._24 + a._43 * b._34 + a._44 * b._44;
}

// 0x004468f0 (slot 27): clears field_0x38 and field_0x34, then sets field_0x34 when the caster's
// bounds pass the shadow camera's box test; returns field_0x34.
int D3DIMSoultreeShadow::UnknownVirtualSlot27()
{
    D3DIMShadowCasterView* c = (D3DIMShadowCasterView*)caster;
    field_0x38 = 0;
    field_0x34 = 0;
    if (!c->field_0x14c) {
        D3DIMShadowMatrix4 local;
        ShadowVec3 center, extent;
        D3DIMShadowMatrix4 m;
        c->GetMatrixIn(0, &local);
        ((D3DIMShadowCasterView*)caster)->GetBounds(&center, &extent);
        const D3DIMShadowMatrix4& view = ((D3DIMShadowCameraView*)shadow->camera)->matrix_0xec;
        D3DIMShadowMatrixProduct(&m, local, view);
        if (g_visibilityClipper->TestBox((D3DIMShadowCameraView*)shadow->camera, &m._11, &center.x,
                                         &extent.x, 0, 0, 0))
            field_0x34 = 1;
    }
    return field_0x34;
}

// 0x00446c30 (slot 29): with field_0x34 set, the same test against the render target's camera
// decides field_0x38 (the flag slot 14 draws with); returns field_0x38.
int D3DIMSoultreeShadow::UnknownVirtualSlot29()
{
    D3DIMShadowCasterView* c = (D3DIMShadowCasterView*)caster;
    field_0x38 = 0;
    if (!c->field_0x14c && field_0x34) {
        D3DIMShadowMatrix4 local;
        ShadowVec3 center, extent;
        D3DIMShadowMatrix4 m;
        c->GetMatrixIn(0, &local);
        ((D3DIMShadowCasterView*)caster)->GetBounds(&center, &extent);
        const D3DIMShadowMatrix4& view = ((D3DIMShadowHostView*)field_0x18)->camera->matrix_0xec;
        D3DIMShadowMatrixProduct(&m, local, view);
        if (g_visibilityClipper->TestBox(((D3DIMShadowHostView*)field_0x18)->camera, &m._11, &center.x,
                                         &extent.x, 0, 0, 0))
            field_0x38 = 1;
    }
    return field_0x38;
}

// Slot 30 scratch (tier 2 from the code): the light-space positions of the current mesh, grown
// on demand (DebugMalloc line 0xad, DebugFree line 0xaa; capacity 0x0059ad28, buffer 0x0059ad2c),
// and the per-vertex outcodes (0x0057efd4).  The outcode array's size is unknown; it is followed
// by other data in .bss.  PROVISIONAL names.
static int s_shadowPointCapacity;
static ShadowVec3* s_shadowPoints;
int g_d3dimShadowOutcodes[1];
extern float g_d3dimShadowTexel;      // 0x0057efa4 (slot 28)

struct D3DIMShadowVertexView {
    float x, y, z;
    int reserved;
    unsigned int diffuse;
    unsigned int specular;
    float tu, tv;
};
extern D3DIMShadowVertexView g_d3dimShadowVertexArray[3000];   // 0x00581eb8

// 0x00446f40 (slot 30): when the shadow is visible (field_0x34), rasterises the caster's
// triangles that lie in front of the shadow's depth into the shadow texture (update frames
// only) and turns the others, when field_0x38 is set, into textured vertices of the receiver
// (field_0x44 of at most 3000).
void D3DIMSoultreeShadow::UnknownVirtualSlot30()
{
    if (!field_0x34)
        return;
    ProjectedShadow* s = shadow;
    g_clipRectangle->Set((float)s->clipRect.left, (float)s->clipRect.top, (float)s->clipRect.right,
                         (float)s->clipRect.bottom);
    int pitch;
    short* pixels = shadow->texture->LockPixels(0, &pitch, 0);
    unsigned int color;
    if (shadow->surfaceFormat == 0x613)
        color = 0x7fffffff;
    else
        color = ((0xff - (shadow->shadowColor & 0xff)) << 24) | 0xffffff;
    field_0x44 = 0;
    D3DIMShadowCasterView* c = (D3DIMShadowCasterView*)caster;
    if (c->field_0x18c)
        c->SelectLod(-1);
    for (int i = 0; i < ((D3DIMShadowCasterView*)caster)->lods[((D3DIMShadowCasterView*)caster)->currentLod].meshCount; i++) {
        D3DIMShadowMesh* mesh = &((D3DIMShadowCasterView*)caster)->lods[((D3DIMShadowCasterView*)caster)->currentLod].meshes[i];
        if (mesh->vertexCount > s_shadowPointCapacity) {
            if (s_shadowPoints) {
                DebugFree(s_shadowPoints, __FILE__, 0xaa);
                s_shadowPoints = 0;
            }
            s_shadowPoints = (ShadowVec3*)DebugMalloc(mesh->vertexCount * sizeof(ShadowVec3), __FILE__, 0xad);
            s_shadowPointCapacity = mesh->vertexCount;
        }
        if (shadow->mode == 3)
            ShadowTransformPointsOrtho(s_shadowPoints, mesh->vertices, &shadow->lightMatrix, mesh->vertexCount, 0xc, 0x20);
        else
            ShadowTransformPointsProjective(s_shadowPoints, mesh->vertices, &shadow->lightMatrix, mesh->vertexCount, 0xc, 0x20);

        ShadowVec3* v = s_shadowPoints;
        int* code = g_d3dimShadowOutcodes;
        for (int n = 0; n < mesh->vertexCount; n++) {
            *code = 0;
            if (shadow->updateThisFrame) {
                if ((float)shadow->clipRect.left > v->x)
                    *code = 1;
                if ((float)shadow->clipRect.right < v->x)
                    *code |= 2;
                if ((float)shadow->clipRect.top > v->y)
                    *code |= 4;
                if ((float)shadow->clipRect.bottom < v->y)
                    *code |= 8;
            } else {
                if ((float)(shadow->clipRect.left * 2) > v->x)
                    *code = 1;
                if ((float)(shadow->clipRect.right * 2) < v->x)
                    *code |= 2;
                if ((float)(shadow->clipRect.top * 2) > v->y)
                    *code |= 4;
                if ((float)(shadow->clipRect.bottom * 2) < v->y)
                    *code |= 8;
            }
            v++;
            code++;
        }

        for (int t = 0; t < mesh->triangleCount; t++) {
            short corner[3];
            const unsigned short* index = mesh->triangles + t * 3;
            corner[0] = index[0];
            corner[1] = index[1];
            corner[2] = index[2];
            int i0 = (unsigned short)corner[0];
            int i1 = (unsigned short)corner[1];
            int i2 = (unsigned short)corner[2];
            if (g_d3dimShadowOutcodes[i0] & g_d3dimShadowOutcodes[i1] & g_d3dimShadowOutcodes[i2])
                continue;
            ClipPoint tri[3];
            tri[0] = *(ClipPoint*)&s_shadowPoints[i0];
            tri[1] = *(ClipPoint*)&s_shadowPoints[i1];
            tri[2] = *(ClipPoint*)&s_shadowPoints[i2];
            if ((tri[1].x - tri[0].x) * (tri[2].y - tri[0].y) - (tri[2].x - tri[0].x) * (tri[1].y - tri[0].y) <= 0.0f)
                continue;
            ClipPoint clipped[8];
            int count = g_clipRectangle->ClipPolygon(tri, clipped, 3);
            if (!count)
                continue;
            if (tri[0].z < shadow->minDepth && tri[1].z < shadow->minDepth && tri[2].z < shadow->minDepth) {
                if (!shadow->updateThisFrame)
                    continue;
                int points[6];
                points[0] = (int)(clipped[0].x - 0.5f);
                points[1] = (int)(clipped[0].y - 0.5f);
                for (int k = 0; k < count - 2; k++) {
                    points[2] = (int)(clipped[k + 1].x - 0.5f);
                    points[3] = (int)(clipped[k + 1].y - 0.5f);
                    points[4] = (int)(clipped[k + 2].x - 0.5f);
                    points[5] = (int)(clipped[k + 2].y - 0.5f);
                    ShadowFillTriangle(shadow->sizeShift, pixels, shadow->colorKeyLow, points, 0);
                }
            } else if (field_0x38) {
                for (int k = 0; k < 3; k++) {
                    int idx = (unsigned short)corner[k];
                    const float* src = mesh->vertices + idx * 8;
                    g_d3dimShadowVertexArray[field_0x44].x = src[0];
                    g_d3dimShadowVertexArray[field_0x44].y = src[1];
                    g_d3dimShadowVertexArray[field_0x44].z = src[2];
                    g_d3dimShadowVertexArray[field_0x44].diffuse = color;
                    g_d3dimShadowVertexArray[field_0x44].reserved = 0;
                    g_d3dimShadowVertexArray[field_0x44].specular = 0;
                    g_d3dimShadowVertexArray[field_0x44].tu = s_shadowPoints[idx].x * g_d3dimShadowTexel;
                    g_d3dimShadowVertexArray[field_0x44].tv = s_shadowPoints[idx].y * g_d3dimShadowTexel;
                    if (++field_0x44 == 3000)
                        goto done;
                }
            }
        }
    }
done:
    shadow->texture->UnlockPixels(0);
}

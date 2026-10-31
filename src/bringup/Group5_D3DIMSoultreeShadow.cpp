// Group5 bring-up definitions (best-guess, link frontier). Not strict matches.
//
// D3DIMSoultreeShadow slots 29 (0x00446c30) and 30 (0x00446f40), extracted from
// samples/physics/shadow/D3DIMSoultreeShadowNearMisses.cpp (slot 29 differs from
// retail only in x87 operand order; slot 30 uses (int) casts where retail
// rounds with an inline fld/fistp helper). Changes for the bring-up link:
//  * the caster's TransformVertexGroups (0x00440d40) is called as
//    D3DIMSoultreeObject's (D3DIMSoulTree.cpp defines it);
//  * the receiver vertices are D3DIMSoultreeShadow.cpp's g_d3dimShadowVertices
//    (0x00581eb8), declared here with that file's vertex type;
//  * the outcodes (0x0057efd4) are a file-local array of 3000 entries.
//    best guess: retail's array runs up to the vertex array, 0x2ee4 bytes
//    (3001 ints), and the meshes drawn here stay within it;
//  * __FILE__ is the retail path string (0x00568cac).
#include "shadow/D3DIMSoultreeShadow.h"
#include "math/Vec3.h"
#include "Matrix4.h"
#include "core/SoultreeObject.h"
#include "ClipRectangle.h"

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
    char pad_0x00[0x14c];
    int hidden;                     // +0x14c (D3DIMSoulTree.h): nonzero skips the visibility test
    char pad_0x150[0x18c - 0x150];
    int subtreeDirty;               // +0x18c nonzero: slot 30 calls TransformVertexGroups(-1) first
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
// The render target at GameObject::renderContext (slot 29 tests against its camera at +8).
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
#include "visibility/VisibilityClipperGlobal.h"   // 0x00575a98

static inline void D3DIMShadowMatrixProduct(D3DIMShadowMatrix4* out, const D3DIMShadowMatrix4& a,
                                            const D3DIMShadowMatrix4& b)
{
    // Reading b through a second name in _24 only makes VC6 order the row-2 sums as retail
    // does in slot 27 (docs/VC6_OPERAND_ORDER.md section 1.2: the alias gives b's row-2
    // column leaves a different age).
    const D3DIMShadowMatrix4& right = b;
    out->_11 = a._11 * b._11 + a._12 * b._21 + a._13 * b._31 + a._14 * b._41;
    out->_12 = a._11 * b._12 + a._12 * b._22 + a._13 * b._32 + a._14 * b._42;
    out->_13 = a._11 * b._13 + a._12 * b._23 + a._13 * b._33 + a._14 * b._43;
    out->_14 = a._11 * b._14 + a._12 * b._24 + a._13 * b._34 + a._14 * b._44;
    out->_21 = a._21 * b._11 + a._22 * b._21 + a._23 * b._31 + a._24 * b._41;
    out->_22 = a._21 * b._12 + a._22 * b._22 + a._23 * b._32 + a._24 * b._42;
    out->_23 = a._21 * b._13 + a._22 * b._23 + a._23 * b._33 + a._24 * b._43;
    out->_24 = a._21 * right._14 + a._22 * right._24 + a._23 * right._34 + a._24 * right._44;
    out->_31 = a._31 * b._11 + a._32 * b._21 + a._33 * b._31 + a._34 * b._41;
    out->_32 = a._31 * b._12 + a._32 * b._22 + a._33 * b._32 + a._34 * b._42;
    out->_33 = a._31 * b._13 + a._32 * b._23 + a._33 * b._33 + a._34 * b._43;
    out->_34 = a._31 * b._14 + a._32 * b._24 + a._33 * b._34 + a._34 * b._44;
    out->_41 = a._41 * b._11 + a._42 * b._21 + a._43 * b._31 + a._44 * b._41;
    out->_42 = a._41 * b._12 + a._42 * b._22 + a._43 * b._32 + a._44 * b._42;
    out->_43 = a._41 * b._13 + a._42 * b._23 + a._43 * b._33 + a._44 * b._43;
    out->_44 = a._41 * b._14 + a._42 * b._24 + a._43 * b._34 + a._44 * b._44;
}

// 0x00446c30 (slot 29): with inShadowView set, the same test against the render target's camera
// decides inCameraView (the flag slot 14 draws with); returns inCameraView.
int D3DIMSoultreeShadow::UnknownVirtualSlot29()
{
    D3DIMShadowCasterView* casterView = (D3DIMShadowCasterView*)caster;
    inCameraView = 0;
    if (!casterView->hidden && inShadowView) {
        D3DIMShadowMatrix4 local;
        ShadowVec3 center, extent;
        D3DIMShadowMatrix4 localToView;
        reinterpret_cast<SoultreeObject*>(casterView)->GetMatrixIn(0, reinterpret_cast<Matrix4*>(&local));
        reinterpret_cast<SoultreeObject*>(caster)->GetSubtreeBounds(reinterpret_cast<Vec3*>(&center), reinterpret_cast<Vec3*>(&extent));
        D3DIMShadowCameraView* camera = ((D3DIMShadowHostView*)renderContext)->camera;
        const D3DIMShadowMatrix4& view = camera->matrix_0xec;
        D3DIMShadowMatrixProduct(&localToView, local, view);
        if (reinterpret_cast<D3DIMShadowClipper*>(g_visibilityClipper)->TestBox(
                ((D3DIMShadowHostView*)renderContext)->camera, &localToView._11, &center.x, &extent.x,
                0, 0, 0))
            inCameraView = 1;
    }
    return inCameraView;
}

// Slot 30 scratch (tier 2 from the code): the light-space positions of the current mesh, grown
// on demand (DebugMalloc line 0xad, DebugFree line 0xaa; capacity 0x0059ad28, buffer 0x0059ad2c),
// and the per-vertex outcodes (0x0057efd4).  The outcode array's size is unknown; it is followed
// by other data in .bss.  PROVISIONAL names.
static int s_shadowPointCapacity;
static ShadowVec3* s_shadowPoints;
static int g_d3dimShadowOutcodes[3000];
extern float g_d3dimShadowTexel;      // 0x0057efa4 (slot 28)

// D3DIMSoultreeShadow.cpp's vertex type and array (0x00581eb8, 3000 entries).
struct D3DIMShadowVertex {
    float x, y, z;
    int reserved;
    unsigned int diffuse;
    unsigned int specular;
    float tu, tv;
    D3DIMShadowVertex() {}
};
extern D3DIMShadowVertex g_d3dimShadowVertices[3000];
#define g_d3dimShadowVertexArray g_d3dimShadowVertices

// D3DIMSoulTree.cpp's provider, declared only as far as slot 30 needs.
class D3DIMSoultreeObject {
public:
    void TransformVertexGroups(int lod);  // 0x00440d40
};

static const char s_d3dimShadowFile[] = "D:\\aardvark\\VC\\krusty2\\D3DIMSoultreeShadow.cpp";

// 0x00446f40 (slot 30): when the shadow is visible (inShadowView), rasterises the caster's
// triangles that lie in front of the shadow's depth into the shadow texture (update frames
// only) and turns the others, when inCameraView is set, into textured vertices of the receiver
// (vertexCount of at most 3000).
void D3DIMSoultreeShadow::UnknownVirtualSlot30()
{
    if (!inShadowView)
        return;
    ProjectedShadow* projected = shadow;
    g_clipRectangle->Set((float)projected->clipRect.left, (float)projected->clipRect.top,
                         (float)projected->clipRect.right, (float)projected->clipRect.bottom);
    int pitch;
    short* pixels = shadow->texture->LockPixels(0, &pitch, 0);
    unsigned int color;
    if (shadow->surfaceFormat == 0x613)
        color = 0x7fffffff;
    else
        color = ((0xff - (shadow->shadowColor & 0xff)) << 24) | 0xffffff;
    vertexCount = 0;
    D3DIMShadowCasterView* casterView = (D3DIMShadowCasterView*)caster;
    if (casterView->subtreeDirty)
        reinterpret_cast<D3DIMSoultreeObject*>(casterView)->TransformVertexGroups(-1);
    for (int i = 0; i < ((D3DIMShadowCasterView*)caster)->lods[((D3DIMShadowCasterView*)caster)->currentLod].meshCount; i++) {
        D3DIMShadowMesh* mesh = &((D3DIMShadowCasterView*)caster)->lods[((D3DIMShadowCasterView*)caster)->currentLod].meshes[i];
        if (mesh->vertexCount > s_shadowPointCapacity) {
            if (s_shadowPoints) {
                DebugFree(s_shadowPoints, s_d3dimShadowFile, 0xaa);
                s_shadowPoints = 0;
            }
            s_shadowPoints = (ShadowVec3*)DebugMalloc(mesh->vertexCount * sizeof(ShadowVec3), s_d3dimShadowFile, 0xad);
            s_shadowPointCapacity = mesh->vertexCount;
        }
        if (shadow->mode == 3)
            ShadowTransformPointsOrtho(s_shadowPoints, mesh->vertices, &shadow->lightMatrix, mesh->vertexCount, 0xc, 0x20);
        else
            ShadowTransformPointsProjective(s_shadowPoints, mesh->vertices, &shadow->lightMatrix, mesh->vertexCount, 0xc, 0x20);

        ShadowVec3* point = s_shadowPoints;
        int* code = g_d3dimShadowOutcodes;
        for (int n = 0; n < mesh->vertexCount; n++) {
            *code = 0;
            if (shadow->updateThisFrame) {
                if ((float)shadow->clipRect.left > point->x)
                    *code = 1;
                if ((float)shadow->clipRect.right < point->x)
                    *code |= 2;
                if ((float)shadow->clipRect.top > point->y)
                    *code |= 4;
                if ((float)shadow->clipRect.bottom < point->y)
                    *code |= 8;
            } else {
                if ((float)(shadow->clipRect.left * 2) > point->x)
                    *code = 1;
                if ((float)(shadow->clipRect.right * 2) < point->x)
                    *code |= 2;
                if ((float)(shadow->clipRect.top * 2) > point->y)
                    *code |= 4;
                if ((float)(shadow->clipRect.bottom * 2) < point->y)
                    *code |= 8;
            }
            point++;
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
            ClipPoint tri[4];  // ClipPolygon stores points[count] = points[0], so a triangle needs a fourth slot
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
            } else if (inCameraView) {
                for (int k = 0; k < 3; k++) {
                    int idx = (unsigned short)corner[k];
                    const float* src = mesh->vertices + idx * 8;
                    g_d3dimShadowVertexArray[vertexCount].x = src[0];
                    g_d3dimShadowVertexArray[vertexCount].y = src[1];
                    g_d3dimShadowVertexArray[vertexCount].z = src[2];
                    g_d3dimShadowVertexArray[vertexCount].diffuse = color;
                    g_d3dimShadowVertexArray[vertexCount].reserved = 0;
                    g_d3dimShadowVertexArray[vertexCount].specular = 0;
                    g_d3dimShadowVertexArray[vertexCount].tu = s_shadowPoints[idx].x * g_d3dimShadowTexel;
                    g_d3dimShadowVertexArray[vertexCount].tv = s_shadowPoints[idx].y * g_d3dimShadowTexel;
                    if (++vertexCount == 3000)
                        goto done;
                }
            }
        }
    }
done:
    shadow->texture->UnlockPixels(0);
}

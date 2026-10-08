// TerrainShadow.cpp -- TerrainShadow (vtable 0x005582d4).
// Ownership is uncertain, so this lives in samples/: no __FILE__ xref lies in 0x00508ae0..0x0050a58f
// (Terrain.cpp's last is 0x00507b38, Texmap.cpp's first 0x0050a6bc). The vector $E block at
// 0x005089a0 just before it closes Terrain.cpp (docs/INITIALIZERS.md), so this code starts a unit
// of its own or continues Terrain.cpp (tier 3).
#include "TerrainShadow.h"
#include "../../../src/reconstructed/ClipRectangle.h"

// ---------------------------------------------------------------------------------------------
// TerrainShadow (vtable 0x005582d4).  owner: bracket only.  No __FILE__ xref lies in
// 0x00508ae0..0x0050a58f (Terrain.cpp's last is 0x00507b38; Texmap.cpp's first is 0x0050a6bc).
// Terrain.cpp's vector $E block at 0x005089a0 precedes it (tier 3).
#include <math.h>

// 0x00508ae0 (ret 4).
TerrainShadow::TerrainShadow(int flags)
    : ShadowReceiver(flags)
{
    caster = 0;
    vertexCount = 0;
    for (int i = 0; i < 0x300; i++)
        indexTable[i] = (short)i;
    minX = 0x7fffffff;
    maxX = 0x80000000;
    minZ = 0x7fffffff;
    maxZ = 0x80000000;
    field_0x6668 = 3.4028235e38f;
    field_0x666c = -3.4028235e38f;
    field_0x6670 = 3.4028235e38f;
    field_0x6674 = -3.4028235e38f;
    cellCount = 0;
    field_0x677c = 0.0078125f;
}

// 0x00508b80 (ret 0xc).
TerrainShadow* TerrainShadow::Attach(int host, Terrain* c, ProjectedShadow* s)
{
    if (s) {
        GameObject::GameObjectVirtualSlot8(host);
        if (this) {
            caster = c;
            shadow = s;
            s->AddReceiver(this);
            return this;
        }
    }
    return 0;
}

// 0x005099c0 (slot 29): collects the distinct cell keys the shadow's grid rectangle
// (minX..maxX, minZ..maxZ in steps of 16) touches in the terrain's height field.  Returns whether
// any were found; when 0x40 are gathered it resets (tier 3 semantics).
int TerrainShadow::UnknownVirtualSlot29()
{
    cellCount = 0;
    for (int z = minZ; z <= maxZ; z += 16) {
        for (int x = minX; x <= maxX; x += 16) {
            TerrainCell* cell = caster->heightField->LookupCell(x, z);
            if (cell) {
                if (cell->owner->vertexCount == 0)
                    cell = (TerrainCell*)cell->key;
                void* key = cell;
                int i = 0;
                while (i < cellCount && cellKeys[i] != key)
                    i++;
                if (i == cellCount) {
                    cellKeys[cellCount] = key;
                    cellCount++;
                }
                if (cellCount == 0x40) {
                    cellCount = 0;
                    vertexCount = 0;
                    return 0;
                }
            }
        }
    }
    return cellCount > 0;
}

// 0x005097d0 (slot 28): sizes the shadow's light matrix to the terrain rectangle (tier 3).  Same
// two-mode matrix build as ProjectedShadow::ComputeBounds: mode 1 is a perspective camera whose
// field of view comes from the texture area over the larger rectangle edge, otherwise an
// orthographic scale by lensScale.  Returns 0 when the rectangle does not fit the texture.
// Explicit-destination forms of 0x004a1410 / 0x004a1860 (cdecl, the destination is the first
// argument and is returned), as Math3D.h's MatrixMultiply(Matrix4* out, a, b) declares them.
ShadowMatrix* TerrainMatrixIdentityInto(ShadowMatrix* out);                       // 0x004a1410
ShadowMatrix* TerrainMatrixMultiplyInto(ShadowMatrix* out, ShadowMatrix a, ShadowMatrix b);  // 0x004a1860

int TerrainShadow::UnknownVirtualSlot28()
{
    float k;
    if (shadow->field_0x98)
        k = 0.5f;
    else
        k = 1.0f;
    field_0x677c = k / shadow->texture->size;
    float extentX = field_0x666c - field_0x6668;
    float extent = field_0x6674 - field_0x6670;
    if (extentX > extent)
        extent = extentX;
    if (shadow->texture->size > extent) {
        float fov;
        ShadowMatrix m;
        if (shadow->mode == 1) {
            fov = (float)atan2((float)(shadow->texture->size * shadow->texture->size) / extent * 0.5,
                               shadow->camera->field_0x198) * 114.59156f;
            shadow->camera->SetLookAt(&shadow->camera->eyePosition, 0, 0, 0, &fov);
            shadow->camera->UnknownVirtualSlot28();
            m = *TerrainMatrixIdentityInto(&m);
            m.m[0][0] = (float)shadow->texture->size;
            m.m[1][1] = (float)shadow->texture->size;
            shadow->lightMatrix = *TerrainMatrixMultiplyInto(&m, m, shadow->camera->matrix_0xec);
        } else {
            m = *TerrainMatrixIdentityInto(&m);
            m.m[0][0] = 0.5f / shadow->lensScale * fov;
            m.m[1][1] = -m.m[0][0];
            m.m[3][0] = (float)shadow->texture->size * 0.5f;
            m.m[3][1] = (float)shadow->texture->size * 0.5f;
            shadow->lightMatrix = *TerrainMatrixMultiplyInto(&m, m, shadow->camera->matrix_0xac);
        }
        return 1;
    }
    return 0;
}

// The render target at GameObject::field_0x18 as slot 14 uses it (RTTI PCRenderTarget; the
// same local view D3DIMSoultreeShadow.cpp keeps).  Tier 3.
class TerrainShadowRenderTarget {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5();
    virtual long GetTextureStageState(int stage, int type, int* value);  // slot 6
    virtual long SetTextureStageState(int stage, int type, int value);   // slot 7
    virtual void SetRenderState(int state, int value, int force);        // slot 8
    virtual long GetRenderState(int state, int* value);                  // slot 9
    virtual void Slot10(); virtual void Slot11(); virtual void Slot12(); virtual void Slot13();
    virtual void Slot14();
    // slot 15: indexed draw (type, FVF, vertices, vertex count, indices, index count, flags).
    virtual int DrawIndexed(int type, int fvf, void* vertices, int vertexCount, short* indices,
                            int indexCount, int flags);
    virtual void Slot16(); virtual void Slot17();
    virtual void Slot18(int value);
    char pad_0x04[0x1c8 - 0x04];
    unsigned char field_0x1c8;                   // bit 2 tested by slot 14
};

// The shadow texture's slot 19 (0x4c) binds it to texture stage 0 (PCTextureMap slot 19).  Tier 3.
class TerrainShadowTextureView {
public:
    virtual void Slot0(); virtual void Slot1(); virtual void Slot2(); virtual void Slot3();
    virtual void Slot4(); virtual void Slot5(); virtual void Slot6(); virtual void Slot7();
    virtual void Slot8(); virtual void Slot9(); virtual void Slot10(); virtual void Slot11();
    virtual void Slot12(); virtual void Slot13(); virtual void Slot14(); virtual void Slot15();
    virtual void Slot16(); virtual void Slot17(); virtual void Slot18();
    virtual void Bind();                                          // slot 19
};

// 0x0050a1a0 (slot 14): draws the collected terrain vertices (+0x34, vertexCount of them) with the
// shadow texture and modulating texture stages; D3DIMSoultreeShadow slot 14 without the world
// matrix.  The address mode and render state 4 are restored afterwards; format 0x613 textures
// also switch render state 0x21 on around the draw.
int TerrainShadow::GameObjectVirtualSlot14()
{
    if (vertexCount) {
        ((TerrainShadowTextureView*)shadow->texture)->Bind();
        int address;
        ((TerrainShadowRenderTarget*)field_0x18)->GetTextureStageState(0, 0xc, &address);
        if (address != 3)
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 0xc, 3);
        if (shadow->surfaceFormat == 0x613) {
            ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x21, 1, 0);
            ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x1b, 1, 0);
            if (((TerrainShadowRenderTarget*)field_0x18)->field_0x1c8 & 4) {
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 4);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 3, 0);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 4);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 5, 2);
                ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 6, 0);
            }
        } else {
            ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x1b, 1, 0);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 1, 4);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 2, 2);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 3, 0);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 4, 4);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 5, 2);
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 6, 0);
        }
        int blend;
        ((TerrainShadowRenderTarget*)field_0x18)->GetRenderState(4, &blend);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(4, 1, 0);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0xe, 0, 0);
        ((TerrainShadowRenderTarget*)field_0x18)->Slot18(0);
        ((TerrainShadowRenderTarget*)field_0x18)->DrawIndexed(4, 0x1e2, vertices, vertexCount, indexTable,
                                                              vertexCount, 0);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0xe, 1, 0);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x1b, 0, 0);
        if (address != 3)
            ((TerrainShadowRenderTarget*)field_0x18)->SetTextureStageState(0, 0xc, address);
        ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(4, blend, 0);
        if (shadow->surfaceFormat == 0x613)
            ((TerrainShadowRenderTarget*)field_0x18)->SetRenderState(0x21, 0, 0);
    }
    return 1;
}

// 0x00509aa0 (slot 30): builds the shadowed terrain vertices.  Every cell key slot 29 gathered has
// its mesh transformed by the shadow's light matrix (perspective in mode 3), each triangle is
// outcode-rejected against the clip rectangle (doubled while the shadow is not being updated),
// back-face culled, clipped, and then either rasterised into the shadow texture's height map
// (all three corners nearer than minDepth) or copied into the vertex array with the shadow colour
// and texture coordinates scaled by field_0x677c.  Cells that were drawn into are marked used.
// Tier 3 semantics; the retail body converts with an inline __asm fistp helper, so this stays a
// documented partial (see targets.json).
void TerrainShadow::UnknownVirtualSlot30()
{
    enum { kMaxMeshVertices = 867 };
    ShadowVec3 transformed[kMaxMeshVertices];
    int outcodes[kMaxMeshVertices];
    ClipPoint clipped[8];
    ClipPoint tri[3];
    int points[6];
    short corner[3];
    int pitch;

    if (cellCount <= 0) {
        vertexCount = 0;
        return;
    }
    ProjectedShadow* s = shadow;
    if (s->updateThisFrame)
        g_clipRectangle->Set((float)s->clipRect.left, (float)s->clipRect.top, (float)s->clipRect.right,
                             (float)s->clipRect.bottom);
    else
        g_clipRectangle->Set((float)s->clipRect.left * 2.0f, (float)s->clipRect.top * 2.0f,
                             (float)s->clipRect.right * 2.0f, (float)s->clipRect.bottom * 2.0f);
    short* pixels = s->texture->LockPixels(0, &pitch, 0);
    unsigned int color;
    if (shadow->surfaceFormat == 0x613)
        color = 0x7fffffff;
    else
        color = ((0xff - (shadow->shadowColor & 0xff)) << 24) | 0xffffff;

    vertexCount = 0;
    for (int i = 0; i < cellCount; i++) {
        int used = 0;
        TerrainCell* cell = (TerrainCell*)cellKeys[i];
        TerrainCellOwner* mesh = cell->owner;
        if (!mesh->vertices)
            continue;
        if (shadow->mode == 3)
            ShadowTransformPointsOrtho(transformed, mesh->vertices, &shadow->lightMatrix, mesh->vertexCount, 0xc, 0x20);
        else
            ShadowTransformPointsProjective(transformed, mesh->vertices, &shadow->lightMatrix, mesh->vertexCount, 0xc, 0x20);

        int n = mesh->vertexCount;
        if (n > 0) {
            ProjectedShadow* ps = shadow;
            int* code = outcodes;
            ShadowVec3* v = transformed;
            do {
                *code = 0;
                if (ps->updateThisFrame) {
                    if ((float)ps->clipRect.left > v->x)
                        *code |= 1;
                    if ((float)ps->clipRect.right < v->x)
                        *code |= 2;
                    if ((float)ps->clipRect.top > v->y)
                        *code |= 4;
                    if ((float)ps->clipRect.bottom < v->y)
                        *code |= 8;
                } else {
                    if ((float)(ps->clipRect.left * 2) > v->x)
                        *code |= 1;
                    if ((float)(ps->clipRect.right * 2) < v->x)
                        *code |= 2;
                    if ((float)(ps->clipRect.top * 2) > v->y)
                        *code |= 4;
                    if ((float)(ps->clipRect.bottom * 2) < v->y)
                        *code |= 8;
                }
                v++;
                code++;
            } while (--n);
        }

        int sectionCount = 1;
        if (mesh->flags_0x121 & 8)
            sectionCount = 0x10;
        for (int sec = 0; sec < sectionCount; sec++) {
            int indexStart;
            int indexCount;
            int vertexBase;
            if (sectionCount == 1) {
                indexStart = 0;
                vertexBase = 0;
                indexCount = mesh->indexCount;
            } else if (sec == 0) {
                indexStart = 0;
                vertexBase = 0;
                indexCount = mesh->sections[0].indexEnd;
            } else {
                indexStart = mesh->sections[sec - 1].indexEnd;
                indexCount = mesh->sections[sec].indexEnd - indexStart;
                vertexBase = mesh->sections[sec - 1].vertexBase;
            }
            for (int t = 0; t < indexCount; t += 3) {
                const short* index = mesh->indices + indexStart + t;
                corner[0] = index[0];
                corner[1] = index[1];
                corner[2] = index[2];
                int i0 = (unsigned short)corner[0] + vertexBase;
                int i1 = (unsigned short)corner[1] + vertexBase;
                int i2 = (unsigned short)corner[2] + vertexBase;
                if (outcodes[i0] & outcodes[i1] & outcodes[i2])
                    continue;
                tri[0] = *(ClipPoint*)&transformed[i0];
                tri[1] = *(ClipPoint*)&transformed[i1];
                tri[2] = *(ClipPoint*)&transformed[i2];
                if ((tri[1].x - tri[0].x) * (tri[2].y - tri[0].y) - (tri[2].x - tri[0].x) * (tri[1].y - tri[0].y) <= 0.0f)
                    continue;
                int count = g_clipRectangle->ClipPolygon(tri, clipped, 3);
                if (!count)
                    continue;
                if (tri[0].z < shadow->minDepth && tri[1].z < shadow->minDepth && tri[2].z < shadow->minDepth) {
                    used = 1;
                    if (!shadow->updateThisFrame)
                        continue;
                    points[0] = (int)(clipped[0].x - 0.5f);
                    points[1] = (int)(clipped[0].y - 0.5f);
                    for (int k = 0; k < count - 2; k++) {
                        points[2] = (int)(clipped[k + 1].x - 0.5f);
                        points[3] = (int)(clipped[k + 1].y - 0.5f);
                        points[4] = (int)(clipped[k + 2].x - 0.5f);
                        points[5] = (int)(clipped[k + 2].y - 0.5f);
                        ShadowFillTriangle(shadow->sizeShift, pixels, shadow->colorKeyLow, points, 0);
                    }
                } else if (cell->field_0x40) {
                    for (int k = 0; k < 3; k++) {
                        int idx = (unsigned short)corner[k] + vertexBase;
                        const float* src = (const float*)((char*)mesh->vertices + idx * 0x20);
                        TerrainShadowVertex* out = &vertices[vertexCount];
                        out->x = src[0];
                        out->y = src[1];
                        out->z = src[2];
                        out->diffuse = color;
                        out->reserved = 0;
                        out->specular = 0;
                        out->tu = transformed[idx].x * field_0x677c;
                        out->tv = transformed[idx].y * field_0x677c;
                        vertexCount++;
                        if (vertexCount == 0x300)
                            goto full;
                    }
                }
            }
        }
full:
        if (used)
            caster->field_0xc88->MarkUsed(mesh->ageEntry);
        if (vertexCount == 0x300)
            break;
    }
    shadow->texture->UnlockPixels(0);
}

// ---------------------------------------------------------------------------------------------
// 0x00508bc0 (slot 27, 3087 bytes): the shadow's footprint on the terrain.  Casts the light
// frustum's corner rays at the height field and accumulates the hit cells' grid rectangle
// (minX..maxZ) and the hits' screen rectangle (field_0x6668..field_0x6674); returns whether
// the footprint is on screen and large enough (tier 3 semantics).
// ---------------------------------------------------------------------------------------------

// The unit's vector with the d3dvec.inl constructors (ProjectedShadow.h's ShadowVec3 has none;
// slot 27's expanded operators build their results through the three-argument constructor,
// and the out-of-line sites call its COMDAT copy 0x00404e60).
struct TerrainShadowVec {
    TerrainShadowVec() {}
    TerrainShadowVec(float x_, float y_, float z_) { x = x_; y = y_; z = z_; }
    float x, y, z;
};

// Views of the objects slot 27 reaches (tier 2 from the call sites; the canonical
// declarations are src/krusty2/visibility/VisibilityQuadTree.h and broadphase/Terrain.h):
// the light camera's eye, look and up vectors behind ProjectedShadow::camera, the terrain's
// segment cast with the hit cell as its fourth argument, and the visibility clipper's
// vertex projection.
struct TerrainShadowCamera {
    char pad_0x00[0x170];
    TerrainShadowVec eye;          // +0x170
    TerrainShadowVec look;         // +0x17c
    TerrainShadowVec up;           // +0x188
    float pad_0x194;
    float field_0x198;             // +0x198 look distance in mode 1 (tier 3)
};
struct TerrainShadowViewer {       // GameObject::field_0x18 of the receiver
    char pad_0x00[8];
    void* camera;                  // +0x08 the visibility camera (matrix at +0xec)
    int width;                     // +0x0c
    int height;                    // +0x10
};
class TerrainShadowCaster {
public:
    int UnknownFunction506e90(const TerrainShadowVec* from, const TerrainShadowVec* to, TerrainShadowVec* hit,
                              TerrainCell** cell, int a, int b);              // 0x00506e90 (ret 0x18)
};
class TerrainShadowClipper {
public:
    void UnknownFunction52f190(const void* camera, const void* matrix, int count,
                               const TerrainShadowVec* vertices, TerrainShadowVec* screen, int* codes); // 0x0052f190
};
extern TerrainShadowClipper* g_terrainShadowClipper;                               // 0x00575a98

// Out-of-line call views (docs/VC6_INLINE_BUDGET.md): the far corners of mode 2 are built
// with the COMDAT constructor, difference and sum.
struct TerrainShadowVecCall : TerrainShadowVec {
    TerrainShadowVecCall(float x_, float y_, float z_);                            // 0x00404e60
};
TerrainShadowVec* TerrainShadowAddCall(TerrainShadowVec* out, const TerrainShadowVec* a, const TerrainShadowVec* b);      // 0x00421cb0
TerrainShadowVec* TerrainShadowSubtractCall(TerrainShadowVec* out, const TerrainShadowVec* a, const TerrainShadowVec* b); // 0x00421d00

static inline TerrainShadowVec operator+(const TerrainShadowVec& a, const TerrainShadowVec& b)
{
    return TerrainShadowVec(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline TerrainShadowVec operator-(const TerrainShadowVec& a, const TerrainShadowVec& b)
{
    return TerrainShadowVec(a.x - b.x, a.y - b.y, a.z - b.z);
}
static inline TerrainShadowVec operator*(const TerrainShadowVec& v, float s)
{
    return TerrainShadowVec(s * v.x, s * v.y, s * v.z);
}
static inline TerrainShadowVec& operator+=(TerrainShadowVec& a, const TerrainShadowVec& b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}
static inline TerrainShadowVec TerrainShadowCross(const TerrainShadowVec& a, const TerrainShadowVec& b)
{
    TerrainShadowVec r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}
static inline TerrainShadowVec TerrainShadowSubtractCtorCall(const TerrainShadowVec& a, const TerrainShadowVec& b)
{
    return TerrainShadowVecCall(a.x - b.x, a.y - b.y, a.z - b.z);
}
static inline TerrainShadowVec TerrainShadowAddCtorCall(const TerrainShadowVec& a, const TerrainShadowVec& b)
{
    return TerrainShadowVecCall(a.x + b.x, a.y + b.y, a.z + b.z);
}
// The light camera, re-read through the shadow at every use as retail does.
#define TERRAIN_SHADOW_CAMERA() ((TerrainShadowCamera*)shadow->camera)
#define TERRAIN_SHADOW_MIN(a, b) ((a) < (b) ? (a) : (b))
#define TERRAIN_SHADOW_MAX(a, b) ((a) > (b) ? (a) : (b))

// Grows the grid and screen rectangles by one terrain hit. A macro, not an inline helper:
// the two modes repeat this text and the final test, and that size is what gives the
// function the expansion budget retail shows (docs/VC6_INLINE_BUDGET.md); as inline helpers
// the budget is used up before the first corner.
#define TERRAIN_SHADOW_ACCUMULATE(cell, hit)                                                         \
    minX = TERRAIN_SHADOW_MIN(minX, (cell)->gridX);                                                   \
    maxX = TERRAIN_SHADOW_MAX(maxX, (cell)->gridX);                                                   \
    minZ = TERRAIN_SHADOW_MIN(minZ, (cell)->gridZ);                                                   \
    maxZ = TERRAIN_SHADOW_MAX(maxZ, (cell)->gridZ);                                                   \
    g_terrainShadowClipper->UnknownFunction52f190(TERRAIN_SHADOW_VIEWER()->camera,                   \
                                                  (char*)TERRAIN_SHADOW_VIEWER()->camera + 0xec, 1,  \
                                                  &(hit), &screen, &code);                           \
    field_0x6668 = TERRAIN_SHADOW_MIN(field_0x6668, screen.x);                                       \
    field_0x666c = TERRAIN_SHADOW_MAX(field_0x666c, screen.x);                                       \
    field_0x6670 = TERRAIN_SHADOW_MIN(field_0x6670, screen.y);                                       \
    field_0x6674 = TERRAIN_SHADOW_MAX(field_0x6674, screen.y)
#define TERRAIN_SHADOW_VIEWER() ((TerrainShadowViewer*)field_0x18)

// The footprint is usable when its screen rectangle overlaps the viewport and is at least
// 10 x 5 pixels; otherwise the cell list and the vertices are dropped.
#define TERRAIN_SHADOW_FINISH()                                                                      \
    if ((float)TERRAIN_SHADOW_VIEWER()->width > field_0x6668 && field_0x666c > 0.0f                   \
        && (float)TERRAIN_SHADOW_VIEWER()->height > field_0x6670 && field_0x6674 > 0.0f               \
        && field_0x666c - field_0x6668 > 10.0f && field_0x6674 - field_0x6670 > 5.0f)                \
        return 1;                                                                                    \
    cellCount = 0;                                                                                   \
    vertexCount = 0;                                                                                 \
    return 0

int TerrainShadow::UnknownVirtualSlot27()
{
    minX = 0x7fffffff;
    minZ = 0x7fffffff;
    maxX = 0x80000000;
    maxZ = 0x80000000;
    field_0x6668 = 3.4028235e38f;
    field_0x6670 = 3.4028235e38f;
    field_0x666c = -3.4028235e38f;
    field_0x6674 = -3.4028235e38f;
    TerrainShadowVec eye = TERRAIN_SHADOW_CAMERA()->eye;
    TerrainShadowVec screen;
    int code;
    if (shadow->mode == 1) {
        eye += TERRAIN_SHADOW_CAMERA()->look * TERRAIN_SHADOW_CAMERA()->field_0x198;
        float half = (float)shadow->texture->size * 0.5f;
        TerrainShadowVec right = TerrainShadowCross(TERRAIN_SHADOW_CAMERA()->look, TERRAIN_SHADOW_CAMERA()->up) * half;
        TerrainShadowVec up = TERRAIN_SHADOW_CAMERA()->up * half;
        TerrainShadowVec corners[4];
        corners[0] = (eye - right) - up;
        corners[1] = (right + eye) + up;
        corners[2] = (right + eye) - up;
        corners[3] = (eye - right) + up;
        for (int i = 0; i < 4; i++) {
            TerrainShadowVec hit;
            TerrainCell* cell;
            if (((TerrainShadowCaster*)caster)->UnknownFunction506e90(&TERRAIN_SHADOW_CAMERA()->eye, &corners[i], &hit, &cell, 0, 0))
                TERRAIN_SHADOW_ACCUMULATE(cell, hit);
        }
        TERRAIN_SHADOW_FINISH();
    } else {
        eye += TERRAIN_SHADOW_CAMERA()->look * 10000.0f;
        TerrainShadowVec right = TerrainShadowCross(TERRAIN_SHADOW_CAMERA()->look, TERRAIN_SHADOW_CAMERA()->up) * shadow->lensScale;
        TerrainShadowVec up = TERRAIN_SHADOW_CAMERA()->up * shadow->lensScale;
        TerrainShadowVec nearCorners[4];
        nearCorners[0] = (TERRAIN_SHADOW_CAMERA()->eye - right) - up;
        nearCorners[1] = (right + TERRAIN_SHADOW_CAMERA()->eye) + up;
        nearCorners[2] = (right + TERRAIN_SHADOW_CAMERA()->eye) - up;
        nearCorners[3] = TerrainShadowAddCtorCall(TERRAIN_SHADOW_CAMERA()->eye - right, up);
        TerrainShadowVec farCorners[4];
        farCorners[0] = TerrainShadowSubtractCtorCall(TerrainShadowSubtractCtorCall(eye, right), up);
        farCorners[1] = TerrainShadowAddCtorCall(TerrainShadowAddCtorCall(right, eye), up);
        farCorners[2] = TerrainShadowSubtractCtorCall(TerrainShadowAddCtorCall(right, eye), up);
        TerrainShadowVec difference;
        TerrainShadowVec sum;
        farCorners[3] = *TerrainShadowAddCall(&sum, TerrainShadowSubtractCall(&difference, &eye, &right), &up);
        for (int i = 0; i < 4; i++) {
            TerrainShadowVec hit;
            TerrainCell* cell;
            while (((TerrainShadowCaster*)caster)->UnknownFunction506e90(&nearCorners[i], &farCorners[i], &hit, &cell, 0, 0)) {
                TERRAIN_SHADOW_ACCUMULATE(cell, hit);
                TerrainShadowVec step = TerrainShadowVecCall(TERRAIN_SHADOW_CAMERA()->look.x * 3.0f, TERRAIN_SHADOW_CAMERA()->look.y * 3.0f, TERRAIN_SHADOW_CAMERA()->look.z * 3.0f);
                nearCorners[i] = *TerrainShadowAddCall(&sum, &hit, &step);
            }
        }
        TERRAIN_SHADOW_FINISH();
    }
}

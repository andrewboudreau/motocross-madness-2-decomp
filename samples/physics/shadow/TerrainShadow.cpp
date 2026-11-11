// TerrainShadow.cpp -- the TerrainShadow (vtable 0x005582d4) near misses: slots 27, 28 and 30.
// The constructor, Attach, slot 29, the slot 14 render and the ~ShadowReceiver COMDAT are strict
// exact in src/reconstructed/TerrainShadow.cpp, which also declares the class (TerrainShadow.h).
// Ownership is uncertain: no __FILE__ xref lies in 0x00508ae0..0x0050a58f (Terrain.cpp's last is
// 0x00507b38, Texmap.cpp's first 0x0050a6bc). The vector $E block at 0x005089a0 just before it
// closes Terrain.cpp (docs/INITIALIZERS.md), so this code starts a unit of its own or continues
// Terrain.cpp (tier 3).
#include "../../../src/reconstructed/TerrainShadow.h"
#include "../../../src/reconstructed/AgeManager.h"
#include "../../../src/reconstructed/ClipRectangle.h"
#include "../../common/FistpFloatToInt.h"
#include <math.h>

// 0x005097d0 (slot 28): sizes the shadow's light matrix to the terrain rectangle (tier 3).  Same
// two-mode matrix build as ProjectedShadow::ComputeBounds: mode 1 is a perspective camera whose
// field of view comes from the texture area over the larger rectangle edge, otherwise an
// orthographic scale by lensScale.  Returns 0 when the rectangle does not fit the texture.
// Each branch owns its matrix: with one shared `projection` VC6 cross-jumps the whole Multiply argument
// copy, retail only merges from the final push.  The chained stores give retail's fst order.
// Larger-extent dead end (about 18 forms): retail's else arm stores the extent back over
// itself (`mov edx, [S]; fstp st0; mov [S], edx`) and reloads texture; if/else, ternary,
// max-by-reference helpers and pointer-select forms all drop the self-copy.
// The orthographic arm reads `fov` unset, in retail too (provisional reading: a source bug):
// retail's `fmul [esp+0x4c]` there addresses fov's home [esp+0x10] (the slot SetLookAt gets
// as &fov on the mode-1 path), and nothing on the orthographic path stores to it first.
// Slot homes (work/s14/symmap.py on /FAs): ours packs fov with extent at [esp+0xc]; retail
// keeps extent alone at 0xc and puts fov at 0x10 with the size*size temporary.
// Explicit-destination forms of 0x004a1410 / 0x004a1860 (cdecl, the destination is the first
// argument and is returned), as Math3D.h's MatrixMultiply(Matrix4* out, a, b) declares them.
ShadowMatrix* TerrainMatrixIdentityInto(ShadowMatrix* out);                       // 0x004a1410
ShadowMatrix* TerrainMatrixMultiplyInto(ShadowMatrix* out, ShadowMatrix a, ShadowMatrix b);  // 0x004a1860

int TerrainShadow::UnknownVirtualSlot28()
{
    float texelScale;
    if (shadow->field_0x98)
        texelScale = 0.5f;
    else
        texelScale = 1.0f;
    field_0x677c = texelScale / shadow->texture->size;
    float extentX = field_0x666c - field_0x6668;
    float extent = field_0x6674 - field_0x6670;
    if (extentX > extent)
        extent = extentX;
    if (shadow->texture->size > extent) {
        float fov;
        if (shadow->mode == 1) {
            ShadowMatrix projection;
            fov = (float)atan2((float)(shadow->texture->size * shadow->texture->size) / extent * 0.5,
                               shadow->camera->field_0x198) * 114.59156f;
            shadow->camera->SetLookAt(&shadow->camera->eyePosition, 0, 0, 0, &fov);
            shadow->camera->UnknownVirtualSlot28();
            projection = *TerrainMatrixIdentityInto(&projection);
            projection.m[0][0] = projection.m[1][1] = (float)shadow->texture->size;
            shadow->lightMatrix = *TerrainMatrixMultiplyInto(&projection, projection,
                                                             shadow->camera->matrix_0xec);
        } else {
            ShadowMatrix projection;
            projection = *TerrainMatrixIdentityInto(&projection);
            float half = (float)shadow->texture->size * 0.5f;
            projection.m[0][0] = 0.5f / shadow->lensScale * fov;
            projection.m[1][1] = -projection.m[0][0];
            projection.m[3][1] = projection.m[3][0] = half;
            shadow->lightMatrix = *TerrainMatrixMultiplyInto(&projection, projection,
                                                             shadow->camera->matrix_0xac);
        }
        return 1;
    }
    return 0;
}

// 0x00509aa0 (slot 30): builds the shadowed terrain vertices.  Every cell key slot 29 gathered has
// its mesh transformed by the shadow's light matrix (perspective in mode 3), each triangle is
// outcode-rejected against the clip rectangle (doubled while the shadow is not being updated),
// back-face culled, clipped, and then either rasterised into the shadow texture's height map
// (all three corners nearer than minDepth) or copied into the vertex array with the shadow colour
// and texture coordinates scaled by field_0x677c.  Cells that were drawn into are marked used.
// Tier 3 semantics; the retail body converts with an inline __asm fistp helper, so this stays a
// documented partial (see targets.json); FistpFloatToInt (samples/common/FistpFloatToInt.h) gives its rounding.
// The section records are reached through the mesh's pointer at +0x17c (retail `mov edx,
// [mesh + 0x17c]; movsx edx, word ptr [edx + 2]`).  An earlier Terrain.h laid them out inline
// at +0x17c, so sectioned meshes read the pointer and the following bytes as index ranges and
// produced indices past the transformed array; the bounds guard added for that is gone, as
// retail has none.  Checked against retail and equal apart from the rounding helper: the
// clip-rectangle doubling, the outcode tests, the cull sign, the minDepth test, the 0x300
// vertex limit, the colour constants and the MarkUsed address (mesh + 0x144).
void TerrainShadow::UnknownVirtualSlot30()
{
    enum { kMaxMeshVertices = 867 };
    ShadowVec3 transformed[kMaxMeshVertices];
    int outcodes[kMaxMeshVertices];
    ClipPoint clipped[8];
    ClipPoint tri[4];  // ClipPolygon stores points[count] = points[0], so a triangle needs a fourth slot
    int points[6];
    short corner[3];
    int pitch;

    if (cellCount <= 0) {
        vertexCount = 0;
        return;
    }
    ProjectedShadow* projected = shadow;
    if (projected->updateThisFrame)
        g_clipRectangle->Set((float)projected->clipRect.left, (float)projected->clipRect.top,
                             (float)projected->clipRect.right, (float)projected->clipRect.bottom);
    else
        g_clipRectangle->Set((float)projected->clipRect.left * 2.0f, (float)projected->clipRect.top * 2.0f,
                             (float)projected->clipRect.right * 2.0f,
                             (float)projected->clipRect.bottom * 2.0f);
    short* pixels = projected->texture->LockPixels(0, &pitch, 0);
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

        int remaining = mesh->vertexCount;
        if (remaining > 0) {
            ProjectedShadow* ps = shadow;
            int* code = outcodes;
            ShadowVec3* point = transformed;
            do {
                *code = 0;
                if (ps->updateThisFrame) {
                    if ((float)ps->clipRect.left > point->x)
                        *code |= 1;
                    if ((float)ps->clipRect.right < point->x)
                        *code |= 2;
                    if ((float)ps->clipRect.top > point->y)
                        *code |= 4;
                    if ((float)ps->clipRect.bottom < point->y)
                        *code |= 8;
                } else {
                    if ((float)(ps->clipRect.left * 2) > point->x)
                        *code |= 1;
                    if ((float)(ps->clipRect.right * 2) < point->x)
                        *code |= 2;
                    if ((float)(ps->clipRect.top * 2) > point->y)
                        *code |= 4;
                    if ((float)(ps->clipRect.bottom * 2) < point->y)
                        *code |= 8;
                }
                point++;
                code++;
            } while (--remaining);
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
            for (int triStart = 0; triStart < indexCount; triStart += 3) {
                const short* index = mesh->indices + indexStart + triStart;
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
                    // ClipPolygon output lies in the clip rectangle, so every value is in int range.
                    FistpFloatToInt(&points[0], clipped[0].x - 0.5f);
                    FistpFloatToInt(&points[1], clipped[0].y - 0.5f);
                    for (int k = 0; k < count - 2; k++) {
                        FistpFloatToInt(&points[2], clipped[k + 1].x - 0.5f);
                        FistpFloatToInt(&points[3], clipped[k + 1].y - 0.5f);
                        FistpFloatToInt(&points[4], clipped[k + 2].x - 0.5f);
                        FistpFloatToInt(&points[5], clipped[k + 2].y - 0.5f);
                        ShadowFillTriangle(shadow->sizeShift, pixels, shadow->colorKeyLow, points, 0);
                    }
                } else if (cell->field_0x40) {
                    for (int k = 0; k < 3; k++) {
                        TerrainShadowVertex* out = &vertices[vertexCount];
                        int idx = (unsigned short)corner[k] + vertexBase;
                        const float* src = (const float*)((char*)mesh->vertices + idx * 0x20);
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
            caster->meshAgeManager->MarkUsed(reinterpret_cast<AgeEntry*>(mesh->ageEntry));
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
// The accumulate macro is braced under each CastSegment test: unbraced, only its first
// statement was conditional.  hit, cell, right and up are declared once at function scope and
// both modes assign right and up, which gives retail's temporary-then-copy stores and its
// `add ecx, 0x188` for up; mode 2 reads lensScale once into a local, as mode 1 does its half
// size, so the scale stays on the x87 stack.  Retail's frame, from the call arguments
// (offsets from esp after the prologue): screen 0xc, the mode 2 step 0x18, hit 0x24, eye 0x30,
// cell 0x3c, up 0x40, right 0x4c, code 0x58, the corners 0x5c, the near corners 0x94; the
// mode 2 sum/difference results sit at 0 and 0xc4 and 8 bytes of select temporaries at 0x8c.
// This form has all of them.
// The cross product parenthesises the x term's second product and both y products
// (TerrainShadowCross; best of its 64 groupings, at both call sites): 1563/3099 -> 2749/3092,
// aligned instructions 791 -> 810 of 824.  Provisional: the grouping is a codegen lever.
// Remaining (18 differing instructions in 9 hunks): the scheduling of the mode 1 right/up
// component copies and corner loads, push order around ProjectVertices and two
// `add eax, 0x170/0x17c` row pointers.  One footprint test
// after the if/else shrinks the function to 2938 bytes (496): the inline budget needs both
// copies.
// Retail's `add eax, 0x17c` before the mode 2 step is a reference to look: a scale helper
// taking `const TerrainShadowVec&` gives it (3094 bytes, 51 differing instructions) but 1557
// strict here (before the cross grouping).  The 16 operand orders of the cross product's x
// and y terms compile the same.
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
struct TerrainShadowViewer {       // GameObject::renderContext of the receiver
    char pad_0x00[8];
    void* camera;                  // +0x08 the visibility camera (matrix at +0xec)
    int width;                     // +0x0c
    int height;                    // +0x10
};
class TerrainShadowCaster {
public:
    int CastSegment(const TerrainShadowVec* from, const TerrainShadowVec* to, TerrainShadowVec* hit,
                              TerrainCell** cell, int a, int b);              // 0x00506e90 (ret 0x18)
};
class TerrainShadowClipper {
public:
    // 0x0052f190: VisibilityClipper::ProjectVertices (src/krusty2/visibility).
    void ProjectVertices(const void* camera, const void* matrix, int count,
                         const TerrainShadowVec* vertices, TerrainShadowVec* screen, int* codes);
};
extern TerrainShadowClipper* g_terrainShadowClipper;                               // 0x00575a98

// Out-of-line call views (docs/VC6_INLINE_BUDGET.md): the far corners of mode 2 are built
// with the COMDAT constructor, difference and sum.  The sum and difference return by value:
// their results are statement temporaries, which VC6 lays over the loop's hit slots, and that
// gives retail's 0xd0 frame (named result locals add 0xc).
struct TerrainShadowVecCall : TerrainShadowVec {
    TerrainShadowVecCall(float x_, float y_, float z_);                            // 0x00404e60
};
TerrainShadowVec TerrainShadowSum(const TerrainShadowVec& a, const TerrainShadowVec& b);         // 0x00421cb0
TerrainShadowVec TerrainShadowDifference(const TerrainShadowVec& a, const TerrainShadowVec& b);  // 0x00421d00

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
    return TerrainShadowVec(a.y * b.z - (a.z * b.y), (a.z * b.x) - (a.x * b.z), a.x * b.y - a.y * b.x);
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
    g_terrainShadowClipper->ProjectVertices(TERRAIN_SHADOW_VIEWER()->camera,                   \
                                                  (char*)TERRAIN_SHADOW_VIEWER()->camera + 0xec, 1,  \
                                                  &(hit), &screen, &code);                           \
    field_0x6668 = TERRAIN_SHADOW_MIN(field_0x6668, screen.x);                                       \
    field_0x666c = TERRAIN_SHADOW_MAX(field_0x666c, screen.x);                                       \
    field_0x6670 = TERRAIN_SHADOW_MIN(field_0x6670, screen.y);                                       \
    field_0x6674 = TERRAIN_SHADOW_MAX(field_0x6674, screen.y)
#define TERRAIN_SHADOW_VIEWER() ((TerrainShadowViewer*)renderContext)

// The footprint is usable when its screen rectangle overlaps the viewport and is at least
// 10 x 5 pixels.  Each mode repeats the test (the inline budget needs both copies); both fall
// through to one shared exit that drops the cell list and the vertices.
#define TERRAIN_SHADOW_RETURN_IF_VISIBLE()                                                                      \
    if ((float)TERRAIN_SHADOW_VIEWER()->width > field_0x6668 && field_0x666c > 0.0f                   \
        && (float)TERRAIN_SHADOW_VIEWER()->height > field_0x6670 && field_0x6674 > 0.0f               \
        && field_0x666c - field_0x6668 > 10.0f && field_0x6674 - field_0x6670 > 5.0f)                \
        return 1

int TerrainShadow::UnknownVirtualSlot27()
{
    minX = 0x7fffffff;
    maxX = 0x80000000;
    minZ = 0x7fffffff;
    maxZ = 0x80000000;
    field_0x6668 = 3.4028235e38f;
    field_0x666c = -3.4028235e38f;
    field_0x6670 = 3.4028235e38f;
    field_0x6674 = -3.4028235e38f;
    TerrainShadowVec eye = TERRAIN_SHADOW_CAMERA()->eye;
    TerrainShadowVec hit;
    TerrainShadowVec screen;
    int code;
    TerrainCell* cell;
    TerrainShadowVec right;
    TerrainShadowVec up;
    if (shadow->mode == 1) {
        eye += TERRAIN_SHADOW_CAMERA()->look * TERRAIN_SHADOW_CAMERA()->field_0x198;
        float half = (float)shadow->texture->size * 0.5f;
        right = TerrainShadowCross(TERRAIN_SHADOW_CAMERA()->look, TERRAIN_SHADOW_CAMERA()->up) * half;
        up = TERRAIN_SHADOW_CAMERA()->up * half;
        TerrainShadowVec corners[4];
        corners[0] = (eye - right) - up;
        corners[1] = (right + eye) + up;
        corners[2] = (right + eye) - up;
        corners[3] = (eye - right) + up;
        for (int i = 0; i < 4; i++) {
            if (((TerrainShadowCaster*)caster)->CastSegment(&TERRAIN_SHADOW_CAMERA()->eye, &corners[i], &hit, &cell, 0, 0)) {
                TERRAIN_SHADOW_ACCUMULATE(cell, hit);
            }
        }
        TERRAIN_SHADOW_RETURN_IF_VISIBLE();
    } else {
        eye += TERRAIN_SHADOW_CAMERA()->look * 10000.0f;
        float lens = shadow->lensScale;
        right = TerrainShadowCross(TERRAIN_SHADOW_CAMERA()->look, TERRAIN_SHADOW_CAMERA()->up) * lens;
        up = TERRAIN_SHADOW_CAMERA()->up * lens;
        TerrainShadowVec nearCorners[4];
        nearCorners[0] = (TERRAIN_SHADOW_CAMERA()->eye - right) - up;
        nearCorners[1] = (right + TERRAIN_SHADOW_CAMERA()->eye) + up;
        nearCorners[2] = (right + TERRAIN_SHADOW_CAMERA()->eye) - up;
        nearCorners[3] = TerrainShadowAddCtorCall(TERRAIN_SHADOW_CAMERA()->eye - right, up);
        TerrainShadowVec farCorners[4];
        farCorners[0] = TerrainShadowSubtractCtorCall(TerrainShadowSubtractCtorCall(eye, right), up);
        farCorners[1] = TerrainShadowAddCtorCall(TerrainShadowAddCtorCall(right, eye), up);
        farCorners[2] = TerrainShadowSubtractCtorCall(TerrainShadowAddCtorCall(right, eye), up);
        farCorners[3] = TerrainShadowSum(TerrainShadowDifference(eye, right), up);
        for (int i = 0; i < 4; i++) {
            while (((TerrainShadowCaster*)caster)->CastSegment(&nearCorners[i], &farCorners[i], &hit, &cell, 0, 0)) {
                TERRAIN_SHADOW_ACCUMULATE(cell, hit);
                TerrainShadowVec step = TerrainShadowVecCall(TERRAIN_SHADOW_CAMERA()->look.x * 3.0f, TERRAIN_SHADOW_CAMERA()->look.y * 3.0f, TERRAIN_SHADOW_CAMERA()->look.z * 3.0f);
                nearCorners[i] = TerrainShadowSum(hit, step);
            }
        }
        TERRAIN_SHADOW_RETURN_IF_VISIBLE();
    }
    cellCount = 0;
    vertexCount = 0;
    return 0;
}

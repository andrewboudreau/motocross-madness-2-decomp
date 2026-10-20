// Near-miss D3DIMSoulTree.CPP candidates (D3DIMSoultreeObject), kept out of
// src/reconstructed/D3DIMSoulTree.cpp until they match. The class, its
// evidence and the exact functions are in src/reconstructed/D3DIMSoulTree.h
// and .cpp. Bindings: D3DIMSoulTreeNearMisses.bindings.json.
//
// UnknownVirtualSlot12 (0x00443aa0, 832 bytes): 799/832. The inline view
// matrix product only gets retail's `add eax, 0xec` base when the camera
// is not held in a local; named result fields (_11.._44) fix the term
// order of rows 0, 2 and 3. Row 1 still adds its four products in a
// different order. VC6 canonicalises each sum (the source term order and
// the statement order of the rows do not matter); the order changes with
// the kind of the leaves and of the destination (a row pointer for row 1:
// 808 of 832; operator(): 787; `view.m[r][c]` stores: 768; a `float*`
// or pointer/reference to `view`: 739), not with symbol numbering (dummy
// locals), a plain struct instead of the union, projection pointers or
// flattened indices (unchanged), and no uniform form gives retail's row 1.
// The product as an inline helper (reference or pointer result, by-value
// left operand, or returning the matrix) is not inlined by VC6. Redundant
// parentheses (docs/VC6_OPERAND_ORDER.md section 3): one bracketing for the
// whole row gives 806; a different one per element of row 1 reaches 826
// (_21..._23 retail's order, _24 still t1, t3, t2, t0 where retail sums t3,
// t2, t1, t0, unchanged by its 264 orders); not kept, since nothing ties
// those per-element forms to the source. Any extra local, even an unused
// `const Matrix4& p = projection;` or `Matrix4* w = &world;`, reorders the
// sums globally (737); reference aliases of world/projection on one or all
// row 1 leaves score the same as the bare declaration (references to locals
// fold), and a second reference to the camera's view matrix gives 160.
// Dead initialised int locals (2026-10-11): 0..33 at the top and 0..31 just before row 1
// give the same scores by count (799 at 0 and 29..33, otherwise 706..776), so the count
// moves all rows together and no count gives retail's row 1.
// One term per statement (2026-10-20, 625 forms: row 1 summed into the
// field or into a float local, terms t0 up or t3 down): only `_24` summed
// into a local from t3 down moves it (805); the same form on _21.._23 gives
// 739..741, so no uniform form exists and none is kept.
//
// DrawCurrentLod (0x00440f30..0x00442dd8, 7848 bytes): the drawing
// function. A nine-way switch on the material mapping type (+0xc8; source
// order 0, 7, 1, 3, 4, 8, 2, 5, 6 from the case layout) rewrites the
// source texture coordinates, then the modifiers, the texture
// coordinate transform 0x00510910, the draw (slot 15) and the debug
// overlays run, and the coordinates are restored from +0x28. The .data
// ints 0x00568930..0x0056893c (all 1) and the float 0x00568940 (20.0) are
// function-local statics, as are the three vectors 0x0057eef8,
// 0x0057ef38 and 0x0057eee8 (guard 0x0057ef24 bits 1, 2, 4) whose type has
// an empty destructor: their atexit entries are the `ret` stubs
// 0x00442f60, 0x00442f50 and 0x00442f40. Same structure (1895 of 1893
// instructions, 484 differing, 7832 bytes): the three mode 8 and mode 2
// normalisations return through the out-of-line Vector3 constructor
// 0x00404e60 as retail does (Vector3Call; the inline budget, with the
// constructor expanded in the mode 5 and 6 normalisations). Mode 5's
// `lines` has 8 entries (4 used). This is a strong inference from the
// frame, not confirmed: retail leaves 0x80 bytes after it that nothing
// touches, and with 8 entries circle, ring, identity and line all sit
// 0x40 below their retail slots (tools/frame_layout.py). Sweeping 13
// choice points (8192 forms; insn diffs 507 to 495, aligned 1386 to 1398)
// kept identity(r, c) stores, a reference to the light position and a
// separate normalisation in mode 3. Accumulating the squared lengths and
// the mode 5 and 6 dot products one term per statement (512 forms; the
// MPBikeRiderDlg lever in docs/VC6_OPERAND_ORDER.md) gives 484 differing
// and 1409 aligned; Length adds y first, the normalisations x first. Left: retail's frame is 0x9e8 (ours
// 0x9a8) with temporaries the source does not have yet (mode 2 reads the
// vertex's y and z through a stack copy, x straight from the vertex; a
// named copy, a copy constructor, a by-value difference and three-float
// constructor forms all copy x too; x subtracted before a named copy keeps
// retail's loads but adds a dead x store (713); y and z in float locals,
// alone or with x read in place, are folded away (744)), `this` is in esi where ours is in
// ebx, and the identity diagonal is stored from m[3][3] down (every order
// and spelling of the four stores is emitted from m[0][0] up).
// The mode 5 source keeps two retail bugs: `circle[i + 1]` reads past the
// 16 points, and the group loop indexes with the restore loop's `k`.
//
#include "../../src/reconstructed/D3DIMSoulTree.h"
#include "../../src/reconstructed/Camera.h"
#include "../../src/reconstructed/D3DIMSoultreeModifier.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/LightEmitter.h"
#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/ResourceManager.h"
#include "../../src/reconstructed/SoultreeMaterial.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/D3DConstants.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// soultree.cpp node calls only these near misses use. They are kept out of
// D3DIMSoulTree.h: more declarations there change VC6's operand order in
// SceneManager.cpp's exact 0x004eb570.
class UnknownSoultreeNodeView : public SoultreeObject {
public:
    void GetMatrixIn(SoultreeObject* frame, Matrix4* out); // 0x004fca80: matrix in `frame` (0: world)
    void GetPositionIn(SoultreeObject* frame, Vector3* out); // 0x004fc9a0: position in `frame` (0: world)
    Vector3 WorldToLocalDirection(const Vector3* v);                 // 0x004fd710: world to local direction
    Vector3 WorldToLocalPoint(const Vector3* v);                 // 0x004fd7f0: world to local point
    Vector3 LocalToWorldDirection(const Vector3* v);                 // 0x004fd5c0: local to world direction
};

// MatrixUtil.h's Vector3 has no binary + and -; these are the
// D3D_OVERLOADS shapes (component-wise temporaries).
static inline Vector3 operator+(const Vector3& a, const Vector3& b)
{
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

float FastInvSqrt(float x);                         // 0x00460c00

// |v| through the table-driven reciprocal square root, exact for 0 and 1.
static inline float Length(const Vector3& v)
{
    float squared = v.y * v.y;
    squared += v.x * v.x;
    squared += v.z * v.z;
    if (squared == 0.0f)
        return 0.0f;
    if (squared == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(squared);
}

// The box test at 0x00575a98 (VisibilityClipper, 0x0052f570, ret 0x1c):
// camera, view matrix, box centre and half extent, screen rectangle,
// corner count, and a last pointer argument.
class UnknownSoultreeClipper {
public:
    int TestBox(UnknownSoultreeCameraView* camera, Matrix4* matrix, Vector3* center,
                              Vector3* extents, UnknownSoultreeScreenRect* rect, int* corners, int* a7);
};
extern UnknownSoultreeClipper* g_visibilityClipper;   // 0x00575a98 (VisibilityClipper, src/krusty2/visibility)

struct UnknownSoultreeMatrix {
    union {
        struct {
            float _11;
            float _12;
            float _13;
            float _14;
            float _21;
            float _22;
            float _23;
            float _24;
            float _31;
            float _32;
            float _33;
            float _34;
            float _41;
            float _42;
            float _43;
            float _44;
        };
        float m[4][4];
    };
};

// 0x00443aa0
int D3DIMSoultreeObject::PrepareGeometry()
{
    visible = 0;
    if (!field_0x14c && field_0x190) {
        if (subtreeDirty)
            UpdateSubtreeBounds();
        Matrix4 world;
        ((UnknownSoultreeNodeView*)this)->GetMatrixIn(0, &world);
        Vector3 center;
        Vector3 extents;
        GetSubtreeBounds(&center, &extents);
        const Matrix4& projection = ((UnknownSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->viewMatrix;
        UnknownSoultreeMatrix view;
        view._11 = world.m[0][0] * projection.m[0][0] + world.m[0][1] * projection.m[1][0] + world.m[0][2] * projection.m[2][0] + world.m[0][3] * projection.m[3][0];
        view._12 = world.m[0][0] * projection.m[0][1] + world.m[0][1] * projection.m[1][1] + world.m[0][2] * projection.m[2][1] + world.m[0][3] * projection.m[3][1];
        view._13 = world.m[0][0] * projection.m[0][2] + world.m[0][1] * projection.m[1][2] + world.m[0][2] * projection.m[2][2] + world.m[0][3] * projection.m[3][2];
        view._14 = world.m[0][0] * projection.m[0][3] + world.m[0][1] * projection.m[1][3] + world.m[0][2] * projection.m[2][3] + world.m[0][3] * projection.m[3][3];
        view._21 = world.m[1][0] * projection.m[0][0] + world.m[1][1] * projection.m[1][0] + world.m[1][2] * projection.m[2][0] + world.m[1][3] * projection.m[3][0];
        view._22 = world.m[1][0] * projection.m[0][1] + world.m[1][1] * projection.m[1][1] + world.m[1][2] * projection.m[2][1] + world.m[1][3] * projection.m[3][1];
        view._23 = world.m[1][0] * projection.m[0][2] + world.m[1][1] * projection.m[1][2] + world.m[1][2] * projection.m[2][2] + world.m[1][3] * projection.m[3][2];
        view._24 = world.m[1][0] * projection.m[0][3] + world.m[1][1] * projection.m[1][3] + world.m[1][2] * projection.m[2][3] + world.m[1][3] * projection.m[3][3];
        view._31 = world.m[2][0] * projection.m[0][0] + world.m[2][1] * projection.m[1][0] + world.m[2][2] * projection.m[2][0] + world.m[2][3] * projection.m[3][0];
        view._32 = world.m[2][0] * projection.m[0][1] + world.m[2][1] * projection.m[1][1] + world.m[2][2] * projection.m[2][1] + world.m[2][3] * projection.m[3][1];
        view._33 = world.m[2][0] * projection.m[0][2] + world.m[2][1] * projection.m[1][2] + world.m[2][2] * projection.m[2][2] + world.m[2][3] * projection.m[3][2];
        view._34 = world.m[2][0] * projection.m[0][3] + world.m[2][1] * projection.m[1][3] + world.m[2][2] * projection.m[2][3] + world.m[2][3] * projection.m[3][3];
        view._41 = world.m[3][0] * projection.m[0][0] + world.m[3][1] * projection.m[1][0] + world.m[3][2] * projection.m[2][0] + world.m[3][3] * projection.m[3][0];
        view._42 = world.m[3][0] * projection.m[0][1] + world.m[3][1] * projection.m[1][1] + world.m[3][2] * projection.m[2][1] + world.m[3][3] * projection.m[3][1];
        view._43 = world.m[3][0] * projection.m[0][2] + world.m[3][1] * projection.m[1][2] + world.m[3][2] * projection.m[2][2] + world.m[3][3] * projection.m[3][2];
        view._44 = world.m[3][0] * projection.m[0][3] + world.m[3][1] * projection.m[1][3] + world.m[3][2] * projection.m[2][3] + world.m[3][3] * projection.m[3][3];
        if (g_visibilityClipper->TestBox((UnknownSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera, (Matrix4*)&view, &center, &extents, &screenRect, 0,
                                                                  &field_0x260)) {
            visible = 1;
            SelectLod(&center, &extents);
        }
    }
    return GameObject::PrepareGeometry();
}


// ---------------------------------------------------------------------------
// 0x00440f30..0x00442dd8: the drawing function. Not matched; see the notes
// at the top of this file.

// 0x0040ae30 (cdecl): the out-of-line dot product.
float Vec3Dot(const Vector3* a, const Vector3* b);
// 0x004a10e0 (cdecl): *out = v - 2 (v.n) n.
void ReflectAndNormalize(const Vector3* v, const Vector3* n, Vector3* out);

// The vector type of the drawing function's local statics: their atexit
// entries are the `ret` stubs 0x00442f40/0x00442f50/0x00442f60, so the type
// has an (empty) destructor.
struct UnknownSoultreeStaticVector : Vector3 {
    ~UnknownSoultreeStaticVector() {}
};

static inline Vector3 NormalizedByCall(const Vector3& v)
{
    float squared = Vec3Dot(&v, &v);
    if (squared == 1.0f)
        return v;
    float scale = FastInvSqrt(squared);
    return Vector3(v.x * scale, v.y * scale, v.z * scale);
}

// Vector3::Vector3(float, float, float), 0x00404e60: the out-of-line copy
// retail calls in the mode 8 and mode 2 normalisations (VC6's inline budget,
// docs/VC6_INLINE_BUDGET.md). Declared only; the bindings map it.
struct Vector3Call : Vector3 {
    Vector3Call(float x_, float y_, float z_);
};

static inline Vector3 NormalizedCallingConstructor(const Vector3& v)
{
    float squared = v.x * v.x;
    squared += v.y * v.y;
    squared += v.z * v.z;
    if (squared == 1.0f)
        return v;
    float scale = FastInvSqrt(squared);
    return Vector3Call(v.x * scale, v.y * scale, v.z * scale);
}

static inline Vector3 Normalized(const Vector3& v)
{
    float squared = v.x * v.x;
    squared += v.y * v.y;
    squared += v.z * v.z;
    if (squared == 1.0f)
        return v;
    float scale = FastInvSqrt(squared);
    return Vector3(v.x * scale, v.y * scale, v.z * scale);
}

// Mode 7's planar mapping: value * scale + offset.
static inline float Remap(float value, float scale, float offset)
{
    float scaled = value * scale;
    return scaled + offset;
}

static inline float RemapDown(float value, float scale, float offset)
{
    float scaled = value * scale;
    return scaled - offset;
}

// The light the shading modes use: the last of the LightManager's lights
// of type 1 or 2.
static inline LightEmitter* FindLight(LightManager* lights)
{
    LightEmitter* light = 0;
    for (int i = 0; i < lights->lightCount; i++) {
        if (lights->lights[i]->lightType == 1 || lights->lights[i]->lightType == 2)
            light = lights->lights[i];
    }
    return light;
}

// 0x00440f30: draws the current level of detail. Per surface and material
// the material's mapping type (+0xc8) rewrites the texture coordinates of
// the source vertices first; afterwards they are restored from +0x28.
void D3DIMSoultreeObject::DrawCurrentLod()
{
    if (!lodTable)
        return;
    if (subtreeDirty)
        TransformVertexGroups(-1);
    Matrix4 identity;
    memset(&identity, 0, sizeof(identity));
    identity(0, 0) = 1.0f;
    identity(1, 1) = 1.0f;
    identity(2, 2) = 1.0f;
    identity(3, 3) = 1.0f;
    ((RenderTarget*)renderContext)->currentCamera->SetWorldMatrix(&identity);
    int savedCull;
    int savedShade;
    ((RenderTarget*)renderContext)->GetRenderState(4, &savedCull);
    ((RenderTarget*)renderContext)->GetRenderState(9, &savedShade);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, texturePerspective, 0);
    if (gouraudShading)
        ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD, 0);
    else
        ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT, 0);
    int k;
    for (int s = 0; s < lodTable[currentLod].surfaceCount; s++) {
        UnknownSoultreeSurface* surface = &lodTable[currentLod].surfaces[s];
        int count = surface->vertexCount;
        int modified = 0;
        int restore = 0;
        for (int m = 0; m < surface->materialCount; m++) {
            SoultreeMaterial* material = materialTable[surface->materialIndices[m]];
            if (useLighting)
                ((RenderTarget*)renderContext)->SetTextureBlend(2, 0);
            material->ApplyRenderStates();
            switch (material->mappingType) {
            case 0:
                modified = 1;
                break;
            case 7: {
                modified = 1;
                restore = 1;
                for (int v = 0; v < count; v++) {
                    if (surface->drawnVertices[v].position.x > 0.0f) {
                        surface->vertices[v].tv = Remap(surface->drawnVertices[v].position.y, -1.0f, 3.0f);
                        surface->vertices[v].tu = RemapDown(surface->drawnVertices[v].position.z, 1.0f, -2.4f);
                    } else {
                        surface->vertices[v].tv = Remap(surface->drawnVertices[v].position.y, -1.0f, 3.0f);
                        surface->vertices[v].tu = RemapDown(surface->drawnVertices[v].position.z, -1.0f, 1.4f);
                    }
                }
                break;
            }
            case 1: {
                textureScroll += lastFrameTime * material->textureSpeed;
                while (textureScroll > 1.0f)
                    textureScroll -= 1.0f;
                for (int v = 0; v < count; v++)
                    surface->vertices[v].tv = surface->vertices[v].tv + textureScroll;
                modified = 1;
                restore = 1;
                break;
            }
            case 3: {
                LightManager* lights = lightManager;
                if (!lights)
                    break;
                LightEmitter* light = FindLight(lights);
                if (!light)
                    break;
                const Vector3& lightPosition = light->lightPosition;
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[j];
                    int vertices = group->vertexCount;
                    Vector3 position;
                    ((UnknownSoultreeNodeView*)group->node)->GetPositionIn(0, &position);
                    Vector3 direction = position - lightPosition;
                    direction = NormalizedByCall(direction);
                    direction = ((UnknownSoultreeNodeView*)group->node)->WorldToLocalDirection(&direction);
                    for (int v = 0; v < vertices; v++) {
                        group->sourceVertices[v].tu =
                            1.0f - (direction.y * group->normals[v].y + direction.x * group->normals[v].x +
                                    direction.z * group->normals[v].z + 1.0f) * 0.5f;
                        group->sourceVertices[v].tv = 0;
                    }
                }
                modified = 1;
                restore = 1;
                break;
            }
            case 4: {
                Vector3 eye = ((UnknownSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position;
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[j];
                    int vertices = group->vertexCount;
                    Vector3 position;
                    ((UnknownSoultreeNodeView*)group->node)->GetPositionIn(0, &position);
                    Vector3 direction = NormalizedByCall(position - eye);
                    direction = ((UnknownSoultreeNodeView*)group->node)->WorldToLocalDirection(&direction);
                    for (int v = 0; v < vertices; v++) {
                        group->sourceVertices[v].tu =
                            1.0f - (direction.y * group->normals[v].y + direction.x * group->normals[v].x +
                                    direction.z * group->normals[v].z + 1.0f) * 0.5f;
                        group->sourceVertices[v].tv = 0;
                    }
                }
                modified = 1;
                restore = 1;
                break;
            }
            case 8: {
                static UnknownSoultreeStaticVector eye;    // 0x0057eef8 (guard bit 1)
                static int updateEye = 1;                  // 0x00568930
                static int drawReflections = 1;            // 0x00568934
                if (drawReflections)
                    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
                UnknownSoultreeVertex line[2];
                line[0].diffuse = 0xffff0000;
                line[1].diffuse = 0xffffffff;
                if (updateEye)
                    (Vector3&)eye = ((UnknownSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position;
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[j];
                    Vector3 localEye = ((UnknownSoultreeNodeView*)group->node)->WorldToLocalPoint(&eye);
                    if (!count)
                        continue;
                    for (int v = 0; v < group->vertexCount; v++) {
                        Vector3 direction;
                        direction.x = group->transformedVertices[v].position.x - localEye.x;
                        direction.y = group->transformedVertices[v].position.y - localEye.y;
                        direction.z = group->transformedVertices[v].position.z - localEye.z;
                        direction = NormalizedCallingConstructor(direction);
                        Vector3 reflected;
                        ReflectAndNormalize(&direction, &group->normals[v], &reflected);
                        if (drawReflections) {
                            Vector3 world = ((UnknownSoultreeNodeView*)group->node)->LocalToWorldDirection(&reflected);
                            line[0].position.x = group->sourceVertices[v].position.x;
                            line[0].position.y = group->sourceVertices[v].position.y;
                            line[0].position.z = group->sourceVertices[v].position.z;
                            line[1].position.x = world.x + group->sourceVertices[v].position.x;
                            line[1].position.y = world.y + group->sourceVertices[v].position.y;
                            line[1].position.z = world.z + group->sourceVertices[v].position.z;
                            ((RenderTarget*)renderContext)->DrawPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)line, 2, 0);
                        }
                        group->sourceVertices[v].tv = ((float)asin(reflected.y) + 1.5707964f) * -0.31830987f;
                        group->sourceVertices[v].tv = -((reflected.y + 1.0f) * 0.5f);
                        group->sourceVertices[v].tv = 0.25f;
                        reflected.y = 0.0f;
                        reflected = NormalizedCallingConstructor(reflected);
                        group->sourceVertices[v].tu =
                            ((float)atan(reflected.x / reflected.z) + 1.5707964f) * 0.31830987f;
                    }
                }
                restore = 1;
                if (drawReflections)
                    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                break;
            }
            case 2: {
                Vector3 position = *(Vector3*)&worldMatrix.m[3][0];
                textureScroll += Length(lastScrollPosition - position) * 0.0075f;
                lastScrollPosition = position;
                if (textureScroll > 100.0f)
                    textureScroll -= 100.0f;
                float scroll = textureScroll;
                Vector3 eye = ((UnknownSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position;
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[j];
                    count = group->vertexCount;
                    if (!count)
                        continue;
                    group->node->UpdateWorldMatrix();
                    Matrix4* world = &group->node->worldMatrix;
                    for (int v = 0; v < count; v++) {
                        Vector3 direction = NormalizedCallingConstructor(group->sourceVertices[v].position - eye);
                        Vector3 normal = group->normals[v];
                        Vector3 n(normal.x * world->m[0][0] + normal.y * world->m[1][0] + normal.z * world->m[2][0],
                                  normal.x * world->m[0][1] + normal.y * world->m[1][1] + normal.z * world->m[2][1],
                                  normal.x * world->m[0][2] + normal.y * world->m[1][2] + normal.z * world->m[2][2]);
                        Vector3 twice = n + n;
                        float d = direction.x * n.x + direction.y * n.y + direction.z * n.z;
                        Vector3 reflected = twice * d - direction;
                        group->sourceVertices[v].tu = reflected.x * 0.5f + 0.5f;
                        group->sourceVertices[v].tv = reflected.y * 0.5f + 0.5f - scroll;
                    }
                }
                restore = 1;
                break;
            }
            case 5: {
                Vector3 center(0.0f, 15.8278f, 0.0f);
                ((UnknownSoultreeNodeView*)this)->GetPositionIn(0, &center);
                center.y = 15.8278f;
                Vector3 circle[16];
                int i;
                for (i = 0; i < 16; i++) {
                    float angle = i * 0.3926991f;
                    circle[i].x = (float)cos(angle) * 128.0f + center.x;
                    circle[i].z = (float)sin(angle) * 128.0f + center.z;
                    circle[i].y = 15.8278f;
                }
                UnknownSoultreeVertex ring[32];
                for (i = 0; i < 32; i++) {
                    ring[i].diffuse = 0xff00ff00;
                    ring[i].specular = 0xff000000;
                    ring[i].tu = 0;
                    ring[i].tv = 0;
                }
                for (i = 0; i < 16; i++) {
                    ring[i * 2].position = circle[i];
                    ring[i * 2 + 1].position = circle[i + 1];
                }
                static UnknownSoultreeStaticVector viewDirection;  // 0x0057ef38 (guard bit 2)
                static UnknownSoultreeStaticVector viewPosition;   // 0x0057eee8 (guard bit 4)
                static int updateView = 1;                         // 0x00568938
                static int drawSphere = 1;                         // 0x0056893c
                if (drawSphere) {
                    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
                    ((RenderTarget*)renderContext)->DrawPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)ring, 32, 0);
                }
                if (updateView) {
                    (Vector3&)viewDirection = ((UnknownSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->viewDirection;
                    (Vector3&)viewPosition = ((UnknownSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position;
                }
                UnknownSoultreeVertex lines[8];          // 4 used; 8 inferred from the frame
                for (i = 0; i < 4; i++) {
                    lines[i].diffuse = 0xffff0000;
                    lines[i].specular = 0xff000000;
                    lines[i].tu = 0;
                    lines[i].tv = 0;
                }
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[k];
                    count = group->vertexCount;
                    if (!count)
                        continue;
                    group->node->UpdateWorldMatrix();
                    Matrix4* world = &group->node->worldMatrix;
                    for (int v = 0; v < count; v++) {
                        Vector3 position = group->sourceVertices[v].position;
                        Vector3 segment[2];
                        segment[0] = position;
                        Vector3 direction = Normalized(position - viewPosition);
                        Vector3 normal = group->normals[v];
                        Vector3 n(normal.x * world->m[0][0] + normal.z * world->m[2][0] + normal.y * world->m[1][0],
                                  normal.x * world->m[0][1] + normal.y * world->m[1][1] + normal.z * world->m[2][1],
                                  normal.x * world->m[0][2] + normal.y * world->m[1][2] + normal.z * world->m[2][2]);
                        float facing = direction.x * n.x;
                        facing += direction.y * n.y;
                        facing += direction.z * n.z;
                        Vector3 offset = n * facing;
                        Vector3 reflected = direction - (offset + offset);
                        Vector3 target = position + reflected;
                        // Where the reflected ray meets the vertical cylinder of
                        // radius 128 about `center`.
                        float dx = target.x - position.x;
                        float dy = target.y - position.y;
                        float dz = target.z - position.z;
                        float a = dx * dx + dz * dz;
                        float b = ((position.x - center.x) * dx + (position.z - center.z) * dz) * 2.0f;
                        float c = position.x * position.x + position.z * position.z + center.x * center.x +
                                  center.z * center.z - (position.x * center.x + position.z * center.z) * 2.0f -
                                  16384.0f;
                        float discriminant = b * b - a * c * 4.0f;
                        Vector3 hit = position;
                        if (discriminant > 0.0f) {
                            float root = (float)sqrt(discriminant);
                            float t = (root - b) / (a + a);
                            if (t > 0.0f)
                                hit = position + Vector3(dx * t, dy * t, dz * t);
                            else if ((-b - root) / (a + a) > 0.0f)
                                hit = position + Vector3(dx * t, dy * t, dz * t);
                        }
                        float fresnel = 1.0f - (float)fabs(facing);
                        segment[1] = hit;
                        group->sourceVertices[v].tv = hit.y * 0.01f;
                        if (group->sourceVertices[v].tv > 1.0f)
                            group->sourceVertices[v].tv = 1.0f;
                        group->sourceVertices[v].tv = 1.0f - group->sourceVertices[v].tv;
                        Vector3 around = hit - center;
                        around.y = 0.0f;
                        around = Normalized(around);
                        float angle = (float)acos(around.z);
                        if (around.x < 0.0f)
                            angle = -angle;
                        group->sourceVertices[v].tu = angle * 0.15915494f;
                        group->sourceVertices[v].diffuse = ((long)(fresnel * 255.0f) << 24) | 0xffffff;
                        for (i = 0; i < 2; i++) {
                            lines[i * 2].position = segment[i];
                            lines[i * 2 + 1].position = segment[i + 1];
                        }
                        if (drawSphere) {
                            ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
                            ((RenderTarget*)renderContext)->DrawPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)lines, 2, 0);
                        }
                    }
                }
                restore = 1;
                break;
            }
            case 6: {
                LightManager* lights = lightManager;
                if (!lights)
                    break;
                LightEmitter* light = FindLight(lights);
                if (!light)
                    break;
                static float exponent = 20.0f;             // 0x00568940
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[j];
                    Vector3 lightPosition = ((UnknownSoultreeNodeView*)group->node)->WorldToLocalDirection(&light->lightPosition);
                    Vector3 eye = ((UnknownSoultreeNodeView*)group->node)->WorldToLocalDirection(
                        &((UnknownSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position);
                    for (int v = 0; v < group->vertexCount; v++) {
                        Vector3 position = group->sourceVertices[v].position;
                        Vector3 toLight = Normalized(position - lightPosition);
                        Vector3 toEye = Normalized(eye - position);
                        Vector3& normal = group->normals[v];
                        float d = (toLight.x * normal.x + toLight.y * normal.y + toLight.z * normal.z) * 2.0f;
                        Vector3 reflected = toLight - normal * d;
                        float specular = toEye.x * reflected.x;
                        specular += toEye.y * reflected.y;
                        specular += toEye.z * reflected.z;
                        if (specular < 0.0f)
                            specular = 0.0f;
                        group->sourceVertices[v].diffuse = ((long)(pow(specular, exponent) * 255.0) << 24) | 0xffffff;
                    }
                }
                modified = 1;
                restore = 0;
                break;
            }
            }
            UnknownSoultreeSurface* drawn;
            if (modifierCount > 0) {
                for (int i = 0; i < modifierCount; i++)
                    ((D3DIMSoultreeModifier*)modifierList[i])->UnknownVirtualSlot27(
                        this, (UnknownSoultreeMesh*)surface, (UnknownSoultreeMesh**)&drawn);
            } else {
                drawn = surface;
            }
            if (drawn->vertexCount) {
                if (material->surfaceMap) {
                    ManagedTexture* texture = (ManagedTexture*)material->surfaceMap->texture;
                    if (texture->mapFlags & 1)
                        texture->RemapTexCoordsIfMoved(&drawn->field_0x2c, (float*)&drawn->field_0x30,
                                                       (float*)&drawn->field_0x34, &drawn->vertices->tu,
                                                       &drawn->vertices->tv, drawn->vertexCount, 0x20);
                }
                ((RenderTarget*)renderContext)->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, (int)drawn->vertices, drawn->vertexCount,
                                                                  (int)drawn->indices, drawn->faceCount * 3, 0);
                if (showWireframe)
                    DrawWireframe(drawn);
                if (showNormals)
                    DrawNormals(drawn);
                if (showVertexCrosses)
                    DrawVertexCrosses(drawn);
            }
            if (modified && restore) {
                for (k = 0; k < count; k++) {
                    surface->vertices[k].tu = surface->uvs[k].u;
                    surface->vertices[k].tv = surface->uvs[k].v;
                }
                surface->field_0x2c = 1.0f;
                surface->field_0x30 = 0;
                surface->field_0x34 = 0;
            }
            material->RestoreRenderStates();
        }
    }
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, savedCull, 0);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_SHADEMODE, savedShade, 0);
}

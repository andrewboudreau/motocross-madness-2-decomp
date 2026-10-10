// Near-miss D3DIMSoulTree.CPP candidates (D3DIMSoultreeObject), kept out of
// src/reconstructed/D3DIMSoulTree.cpp until they match. The class, its
// evidence and the exact functions are in src/reconstructed/D3DIMSoulTree.h
// and .cpp. Bindings: D3DIMSoulTreeNearMisses.bindings.json.
//
// ReadLods (0x00440060, 1964 bytes): 1958/1964. The .slt LOD
// reader. The colour packed from named long components (r, g, b) gives
// retail's `[faces + offset]` face stores; the inline D3DRGB-style
// expression gives `[offset + faces]`. The remaining operand order is not
// spelling- or declaration-sensitive: it flips with the amount of IR in the
// vertex loop (emptying it or dropping half its statements gives retail's
// order, no single deletion does), every neutral rewrite of the s-loop
// surface pointer, the loop declarations and the group loops was tried, and
// a 0..63 typedef prefix scan leaves it unchanged. Before that change:
// retail forms the surface address as
// `offset + surfaces` (mov ebx, offset; mov edi, surfaces; add ebx, edi)
// and stores the face indices as [faces + offset]; VC6 emits
// `surfaces + offset` and [offset + faces] for every indexing form tried
// (&a[i], a + i, i + a, a pointer local, a (*)[3] cast). Same pattern as
// samples/camera/FollowCameraNearMisses.cpp's 0x00463140. A small probe
// with only the face loop (3*f, f*3, pointer forms, unsigned f, a k += 3
// loop) emits retail's `[faces + offset]`: the swap is a whole-function
// effect, as samples/track/SceneManagerNearMisses.cpp's 0x004ecd60.
//
// TransformVertexGroups (0x00440d40, 484 bytes): 241/484, 2 bytes short.
// The vertex-group transform; the same `offset + groups` operand order for
// the source group (retail mov ecx, groups; mov ebp, offset; add ebp, ecx).
// Everything after that only shifts.
//
// UnknownFunction4435b0 (0x004435b0, 400 bytes): the axis gizmo. Retail
// stores the index words and the four points with plain immediates in
// declaration order and keeps the vertex array at esp+0x5c. Member stores
// (135/404) and aggregate initialisers both group the zero stores first;
// Vector3 temporaries are worse. Retail also copies each transformed point
// back out of `points` instead of from the call result.
//
// UnknownFunction443740 (0x00443740, 640 bytes): the box outline. The corner
// table is built from negated extents kept on the FPU stack (fst chains)
// and integer copies; the Vector3 form here evaluates each corner apart.
//
// SoultreeVirtualSlot5 (0x00444140, 768 bytes): 610/764. The bounds use
// by-value Raise/Lower inlines and no face-group pointer; the loops match.
// VC6 adds the y and z of `high + low` as low + high, and keeps the
// +0x164 address in ebp where retail uses edx. Unchanged by `high + low`,
// `0.5f * (...)`, named center/extent locals, operator+ with by-value
// operands or swapped component sums; explicit component constructors are
// worse.
//
// SelectLod (0x00443de0, 592 bytes): the level-of-detail and
// mip chooser. Writing the camera expression without a local reproduces
// retail's `add esi, 0x170`. Retail keeps `center`/`extents` in ebx/ebp
// from the prologue and multiplies scale * +0x280[i] * +0x284 and
// scale * +0xcc * width in that order; VC6 reorders both products.
// `*(float*)&field_0xcc`: SoultreeMaterial.h types +0xcc as int.
//
// SoultreeVirtualSlot7 (0x00444560, 1233 bytes): the node search reuses
// the CollectDescendants counter `index` (retail keeps it in that slot,
// frame 0x2c now matches), the surface array size is read back from
// `to->surfaceCount`, the final UnknownFunction444440 call is shared after
// the if/else (retail pops edi/ebp before it), and `to` declared before
// `from` gives retail's loop-head schedule. The instruction stream then
// equals retail's up to a register rotation: retail keeps the source in ebx,
// `from` in edi and `to` in ebp (ours ebp, ebx, edi), whose disp8 on [ebp]
// makes VC6 here 2 bytes shorter (355/1231 by position). Swapping or
// dropping locals, the loop declarations and a typedef-prefix scan do not
// rotate it.
//
// UnknownFunction440810 (0x00440810, 1328 bytes): texture density per
// material. Same 388 instructions; the uv delta is a copy of a zeroed
// Vector3 (`Vector3 delta = zero`) taken once, which reproduces retail's
// single z*z. Retail spills `this` and keeps the face offset in ebx; VC6
// keeps a separate face counter, so the frame is 4 bytes larger.
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
// those per-element forms to the source.
//
// DrawNormals (normals, 528 bytes) and DrawVertexCrosses
// (vertex crosses, 512 bytes): both scale by function-local statics in
// .data (0x00568944 = 0.5, 0x00568948 = 0.025), which is what puts the
// variable before the vertex operand in retail's fmul/fadd. Remaining:
// retail materializes the face-group address with `lea` and keeps
// different induction registers.
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
// 0x00442f60, 0x00442f50 and 0x00442f40. Same structure (normalised
// ratio 0.87, 1882 of 1893 instructions); retail's frame is 0x9e8 (ours
// 0x940), it calls the out-of-line Vector3 constructor 0x00404e60 in the
// mode 2 and 8 normalisations, and register allocation differs.
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
    void UnknownFunction4fca80(SoultreeObject* frame, Matrix4* out); // 0x004fca80: matrix in `frame` (0: world)
    void UnknownFunction4fc9a0(SoultreeObject* frame, Vector3* out); // 0x004fc9a0: position in `frame` (0: world)
    Vector3 UnknownFunction4fd710(const Vector3* v);                 // 0x004fd710: world to local direction
    Vector3 UnknownFunction4fd7f0(const Vector3* v);                 // 0x004fd7f0: world to local point
    Vector3 UnknownFunction4fd5c0(const Vector3* v);                 // 0x004fd5c0: local to world direction
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

// 0x00440060: reads the levels of detail from the .slt keys; returns 1
// when a surface has no material (slot 3 then adds an untextured one).
int D3DIMSoultreeObject::ReadLods()
{
    int untextured = 0;
    int count = UnknownFunction4fda30();
    SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x1dd);
    int index = 1;
    nodes[0] = this;
    CollectDescendants(&index, nodes);
    char section[0x80];
    sprintf(section, "LOD Information");
    parameterBlock->UnknownFunction4b78f0(section);
    parameterBlock->UnknownFunction4b7f10("NumberOfLOD", 0, &lodCount);
    parameterBlock->UnknownFunction4b7f10("UseAutoLOD", 0, &field_0x288);
    int i;
    if (field_0x288) {
        autoLodDistances = (float*)DebugMalloc(lodCount * 4 - 1, __FILE__, 0x1ec);
        for (i = 0; i < lodCount - 1; i++) {
            sprintf(section, "AutoLOD#%i", i);
            parameterBlock->UnknownFunction4b7f40(section, -1.0f, &autoLodDistances[i]);
        }
    }
    lodTable = (UnknownSoultreeLod*)DebugMalloc(lodCount * 8, __FILE__, 0x1f5);
    UnknownSoultreeFaceGroup* groups = (UnknownSoultreeFaceGroup*)DebugMalloc(count * 0x14, __FILE__, 0x1f9);
    for (i = 0; i < count; i++)
        groups[i].node = nodes[i];
    for (int lod = 0; lod < lodCount; lod++) {
        UnknownSoultreeLod* level = &lodTable[lod];
        sprintf(section, "LOD %i", lod);
        parameterBlock->UnknownFunction4b78f0(section);
        parameterBlock->UnknownFunction4b7f10("NumberOfSurfaces", 0, &level->surfaceCount);
        level->surfaces = (UnknownSoultreeSurface*)DebugMalloc(level->surfaceCount * 0x38, __FILE__, 0x206);
        for (int s = 0; s < level->surfaceCount; s++) {
            UnknownSoultreeSurface* surface = &level->surfaces[s];
            surface->field_0x2c = 1.0f;
            surface->field_0x30 = 0;
            surface->field_0x34 = 0;
            sprintf(section, "LOD %i - Surface %i", lod, s);
            parameterBlock->UnknownFunction4b78f0(section);
            parameterBlock->UnknownFunction4b7f10("NumberOfVertices", 0, &surface->vertexCount);
            parameterBlock->UnknownFunction4b7f10("NumberOfFaces", 0, &surface->faceCount);
            parameterBlock->UnknownFunction4b7f10("NumberOfMaterials", 0, &surface->materialCount);
            if (surface->materialCount > 0) {
                surface->materialIndices = (int*)DebugMalloc(surface->materialCount * 4, __FILE__, 0x216);
                for (int m = 0; m < surface->materialCount; m++) {
                    char key[0x80];
                    sprintf(key, "Material#%i", m);
                    parameterBlock->UnknownFunction4b7f10(key, 0, &surface->materialIndices[m]);
                }
            } else {
                untextured = 1;
                surface->materialIndices = (int*)DebugMalloc(4, __FILE__, 0x21e);
                surface->materialIndices[0] = 0;
                surface->materialCount = 1;
            }
            surface->vertices = (UnknownSoultreeVertex*)DebugMalloc(surface->vertexCount * 0x20, __FILE__, 0x223);
            surface->drawnVertices = (UnknownSoultreeVertex*)DebugMalloc(surface->vertexCount * 0x20, __FILE__, 0x224);
            surface->normals = (Vector3*)DebugMalloc(surface->vertexCount * 0xc, __FILE__, 0x225);
            surface->indices = (unsigned short*)DebugMalloc(surface->faceCount * 6, __FILE__, 0x226);
            surface->uvs = (UnknownSoultreeUV*)DebugMalloc(surface->vertexCount * 8, __FILE__, 0x227);
            sprintf(section, "LOD %i - Surface %i - Vertices", lod, s);
            parameterBlock->UnknownFunction4b7f70(section);
            int v;
            for (v = 0; v < surface->vertexCount; v++) {
                float red;
                float green;
                float blue;
                parameterBlock->UnknownFunction4b8010(0);
                parameterBlock->UnknownFunction4b81c0(0, &surface->drawnVertices[v].position.x);
                parameterBlock->UnknownFunction4b81c0(1, &surface->drawnVertices[v].position.y);
                parameterBlock->UnknownFunction4b81c0(2, &surface->drawnVertices[v].position.z);
                parameterBlock->UnknownFunction4b81c0(3, &surface->normals[v].x);
                parameterBlock->UnknownFunction4b81c0(4, &surface->normals[v].y);
                parameterBlock->UnknownFunction4b81c0(5, &surface->normals[v].z);
                parameterBlock->UnknownFunction4b81c0(6, &surface->drawnVertices[v].tu);
                parameterBlock->UnknownFunction4b81c0(7, &surface->drawnVertices[v].tv);
                parameterBlock->UnknownFunction4b81c0(12, &red);
                parameterBlock->UnknownFunction4b81c0(13, &green);
                parameterBlock->UnknownFunction4b81c0(14, &blue);
                long r = (long)(red * 255.0);
                long g = (long)(green * 255.0);
                long b = (long)(blue * 255.0);
                surface->drawnVertices[v].diffuse = 0xff000000 | (r << 16) | (g << 8) | b;
                surface->drawnVertices[v].specular = 0;
                surface->vertices[v] = surface->drawnVertices[v];
                surface->uvs[v].u = surface->vertices[v].tu;
                surface->uvs[v].v = surface->vertices[v].tv;
            }
            sprintf(section, "LOD %i - Surface %i - Faces", lod, s);
            parameterBlock->UnknownFunction4b7f70(section);
            for (int f = 0; f < surface->faceCount; f++) {
                int a;
                int b;
                int c;
                parameterBlock->UnknownFunction4b8010(0);
                parameterBlock->UnknownFunction4b8180(0, &a);
                parameterBlock->UnknownFunction4b8180(1, &b);
                parameterBlock->UnknownFunction4b8180(2, &c);
                surface->indices[f * 3] = a;
                surface->indices[f * 3 + 1] = b;
                surface->indices[f * 3 + 2] = c;
            }
            sprintf(section, "LOD %i - Surface %i - Object Pointer List", lod, s);
            parameterBlock->UnknownFunction4b7f70(section);
            int used = 0;
            for (i = 0; i < count; i++) {
                int first;
                int vertices;
                parameterBlock->UnknownFunction4b8010(0);
                parameterBlock->UnknownFunction4b8180(0, &first);
                parameterBlock->UnknownFunction4b8180(1, &vertices);
                groups[i].vertexCount = vertices;
                groups[i].sourceVertices = &surface->vertices[first];
                groups[i].transformedVertices = &surface->drawnVertices[first];
                groups[i].normals = &surface->normals[first];
                if (vertices > 0)
                    used++;
            }
            surface->groupCount = used;
            surface->groups = (UnknownSoultreeFaceGroup*)DebugMalloc(used * 0x14, __FILE__, 0x283);
            int k = 0;
            for (i = 0; i < count; i++) {
                if (groups[i].vertexCount > 0)
                    surface->groups[k++] = groups[i];
            }
        }
    }
    UnknownFunction444440();
    DebugFree(groups, __FILE__, 0x291);
    DebugFree(nodes, __FILE__, 0x292);
    return untextured;
}

// 0x00440d40
void D3DIMSoultreeObject::TransformVertexGroups(int lod)
{
    int saved = field_0x27c;
    if (lod != -1)
        field_0x27c = lod;
    for (int i = 0; i < lodTable[field_0x27c].surfaceCount; i++) {
        UnknownSoultreeSurface* surface = &lodTable[field_0x27c].surfaces[i];
        int colored = 0;
        int k;
        for (k = 0; k < surface->materialCount; k++) {
            SoultreeMaterial* material = materialTable[surface->materialIndices[k]];
            if (!material->hasTextureName || material->useVertexColor)
                colored = 1;
        }
        UnknownSoultreeSurface* drawn;
        if (secondModifierCount > 0) {
            for (k = 0; k < secondModifierCount; k++)
                ((D3DIMSoultreeModifier*)secondModifierList[k])->UnknownVirtualSlot27(
                    this, (UnknownSoultreeMesh*)surface, (UnknownSoultreeMesh**)&drawn);
        } else {
            drawn = surface;
        }
        for (int j = 0; j < drawn->groupCount; j++) {
            UnknownSoultreeFaceGroup* group = &drawn->groups[j];
            UnknownSoultreeFaceGroup* source = &surface->groups[j];
            int count = group->vertexCount;
            if (count) {
                group->node->UpdateWorldMatrix();
                void* target = group->transformedVertices;
                void* vertices = source->sourceVertices;
                if (field_0x1a4) {
                    if (colored)
                        lightManager->UnknownFunction49e4a0(&group->node->worldMatrix, count,
                                                           group->normals, vertices, 0xc, target);
                    else
                        lightManager->UnknownFunction49e4a0(&group->node->worldMatrix, count,
                                                           group->normals, vertices, 0xc, 0);
                }
                TransformPoints(vertices, target, &group->node->worldMatrix, count, 0x20, 0x20);
            }
        }
    }
    field_0x18c = 0;
    field_0x27c = saved;
}

// 0x004435b0
void D3DIMSoultreeObject::UnknownFunction4435b0()
{
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
    unsigned short indices[6] = { 0, 1, 0, 2, 0, 3 };
    Vector3 points[4];
    points[0].x = 0.0f;
    points[0].y = 0.0f;
    points[0].z = 0.0f;
    points[1].x = 0.25f;
    points[1].y = 0.0f;
    points[1].z = 0.0f;
    points[2].x = 0.0f;
    points[2].y = 0.25f;
    points[2].z = 0.0f;
    points[3].x = 0.0f;
    points[3].y = 0.0f;
    points[3].z = 0.25f;
    UnknownSoultreeVertex vertices[4];
    for (int i = 0; i < 4; i++) {
        points[i] = UnknownFunction4fd660(points[i]);
        vertices[i].position = points[i];
        vertices[i].tu = 0;
        vertices[i].tv = 0;
        vertices[i].specular = 0;
    }
    vertices[0].diffuse = 0xffffff;
    vertices[1].diffuse = 0xff0000;
    vertices[2].diffuse = 0xff;
    vertices[3].diffuse = 0xff00;
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot15(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)vertices, 4, (int)indices, 6, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
    if (nextSibling)
        ((D3DIMSoultreeObject*)nextSibling)->UnknownFunction4435b0();
    if (firstChild)
        ((D3DIMSoultreeObject*)firstChild)->UnknownFunction4435b0();
}

// 0x00443740
void D3DIMSoultreeObject::UnknownFunction443740(Vector3 center, Vector3 extents, int flag)
{
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
    unsigned short indices[24] = {
        0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7,
    };
    Vector3 corners[8];
    corners[0] = Vector3(-extents.x, extents.y, extents.z);
    corners[1] = Vector3(extents.x, extents.y, extents.z);
    corners[2] = Vector3(extents.x, extents.y, -extents.z);
    corners[3] = Vector3(-extents.x, extents.y, -extents.z);
    corners[4] = Vector3(-extents.x, -extents.y, extents.z);
    corners[5] = Vector3(extents.x, -extents.y, extents.z);
    corners[6] = Vector3(extents.x, -extents.y, -extents.z);
    corners[7] = Vector3(-extents.x, -extents.y, -extents.z);
    UnknownSoultreeVertex vertices[8];
    for (int i = 0; i < 8; i++) {
        corners[i].x = center.x + corners[i].x;
        corners[i].y = center.y + corners[i].y;
        corners[i].z = center.z + corners[i].z;
        corners[i] = UnknownFunction4fd660(corners[i]);
        vertices[i].position = corners[i];
        vertices[i].tu = 0;
        vertices[i].tv = 0;
        if (flag)
            vertices[i].specular = 0;
        else
            vertices[i].specular = 0xff0000;
        vertices[i].diffuse = 0;
    }
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot15(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)vertices, 8, (int)indices, 24, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
}

static inline void Raise(float& bound, float value)
{
    if (value > bound)
        bound = value;
}

static inline void Lower(float& bound, float value)
{
    if (value < bound)
        bound = value;
}

// 0x00444140
void D3DIMSoultreeObject::SoultreeVirtualSlot5()
{
    D3DIMSoultreeObject* model = this;
    while (!model->lodTable)
        model = (D3DIMSoultreeObject*)model->parent;
    boundsFound = 0;
    Vector3 low(64000.0f, 64000.0f, 64000.0f);
    Vector3 high(-64000.0f, -64000.0f, -64000.0f);
    for (int lod = 0; lod < model->lodCount; lod++) {
        for (int i = 0; i < model->lodTable[lod].surfaceCount; i++) {
            for (int j = 0; j < model->lodTable[lod].surfaces[i].groupCount; j++) {
                if (model->lodTable[lod].surfaces[i].groups[j].node == this) {
                    UnknownSoultreeVertex* vertex = model->lodTable[lod].surfaces[i].groups[j].transformedVertices;
                    int count = model->lodTable[lod].surfaces[i].groups[j].vertexCount;
                    for (int k = 0; k < count; k++) {
                        Raise(high.x, vertex[k].position.x);
                        Raise(high.y, vertex[k].position.y);
                        Raise(high.z, vertex[k].position.z);
                        Lower(low.x, vertex[k].position.x);
                        Lower(low.y, vertex[k].position.y);
                        Lower(low.z, vertex[k].position.z);
                        boundsFound = 1;
                    }
                }
            }
        }
    }
    if (!boundsFound) {
        low = Vector3(0.0f, 0.0f, 0.0f);
        high = Vector3(0.0f, 0.0f, 0.0f);
    }
    field_0x154 = 1;
    field_0x158 = (low + high) * 0.5f;
    field_0x164 = (high - low) * 0.5f;
}

// 0x00444560
void D3DIMSoultreeObject::SoultreeVirtualSlot7(SoultreeObject* sourceNode)
{
    D3DIMSoultreeObject* source = (D3DIMSoultreeObject*)sourceNode;
    GameObject::UnknownVirtualSlot8(source->field_0x18);
    textureFormat = source->textureFormat;
    textureManager = source->textureManager;
    lightManager = source->lightManager;
    SoultreeObject::SoultreeVirtualSlot7(source);
    UnknownFunction444a40(source);
    lodCount = source->lodCount;
    field_0x27c = source->field_0x27c;
    lowestLod = source->lowestLod;
    field_0x288 = source->field_0x288;
    if (field_0x288) {
        autoLodDistances = (float*)DebugMalloc(lodCount * 4, __FILE__, 0x88e);
        for (int i = 0; i < lodCount; i++)
            autoLodDistances[i] = source->autoLodDistances[i];
    }
    if (source->lodTable) {
        int count = UnknownFunction4fda30();
        SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x897);
        int index = 1;
        nodes[0] = this;
        CollectDescendants(&index, nodes);
        SoultreeObject** sourceNodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x89d);
        index = 1;
        sourceNodes[0] = source;
        source->CollectDescendants(&index, sourceNodes);
        lodTable = (UnknownSoultreeLod*)DebugMalloc(lodCount * 8, __FILE__, 0x8a2);
        for (int lod = 0; lod < lodCount; lod++) {
            UnknownSoultreeLod* to = &lodTable[lod];
            UnknownSoultreeLod* from = &source->lodTable[lod];
            to->surfaceCount = from->surfaceCount;
            to->surfaces = (UnknownSoultreeSurface*)DebugMalloc(to->surfaceCount * 0x38, __FILE__, 0x8aa);
            for (int i = 0; i < to->surfaceCount; i++) {
                UnknownSoultreeSurface* s = &from->surfaces[i];
                UnknownSoultreeSurface* d = &to->surfaces[i];
                d->vertexCount = s->vertexCount;
                d->faceCount = s->faceCount;
                d->materialCount = s->materialCount;
                d->materialIndices = (int*)DebugMalloc(d->materialCount * 4, __FILE__, 0x8b3);
                int k;
                for (k = 0; k < d->materialCount; k++)
                    d->materialIndices[k] = s->materialIndices[k];
                d->field_0x2c = s->field_0x2c;
                d->field_0x30 = s->field_0x30;
                d->field_0x34 = s->field_0x34;
                d->vertices = (UnknownSoultreeVertex*)DebugMalloc(d->vertexCount * 0x20, __FILE__, 0x8bd);
                d->drawnVertices = (UnknownSoultreeVertex*)DebugMalloc(d->vertexCount * 0x20, __FILE__, 0x8be);
                d->normals = (Vector3*)DebugMalloc(d->vertexCount * 0xc, __FILE__, 0x8bf);
                d->indices = (unsigned short*)DebugMalloc(d->faceCount * 6, __FILE__, 0x8c0);
                d->uvs = (UnknownSoultreeUV*)DebugMalloc(d->vertexCount * 8, __FILE__, 0x8c1);
                memcpy(d->vertices, s->vertices, d->vertexCount * 0x20);
                memcpy(d->drawnVertices, s->drawnVertices, d->vertexCount * 0x20);
                memcpy(d->normals, s->normals, d->vertexCount * 0xc);
                memcpy(d->indices, s->indices, d->faceCount * 6);
                memcpy(d->uvs, s->uvs, d->vertexCount * 8);
                d->groupCount = s->groupCount;
                d->groups = (UnknownSoultreeFaceGroup*)DebugMalloc(d->groupCount * 0x14, __FILE__, 0x8cb);
                int first = 0;
                for (int j = 0; j < d->groupCount; j++) {
                    d->groups[j].node = 0;
                    for (index = 0; !d->groups[j].node; index++) {
                        if (sourceNodes[index] == s->groups[j].node)
                            d->groups[j].node = nodes[index];
                    }
                    d->groups[j].vertexCount = s->groups[j].vertexCount;
                    d->groups[j].sourceVertices = &d->vertices[first];
                    d->groups[j].transformedVertices = &d->drawnVertices[first];
                    d->groups[j].normals = &d->normals[first];
                    first += d->groups[j].vertexCount;
                }
            }
        }
        DebugFree(nodes, __FILE__, 0x8e1);
        DebugFree(sourceNodes, __FILE__, 0x8e2);
    } else {
        lodTable = 0;
    }
    UnknownFunction444440();
}

// 0x00460b50 (cdecl): table-driven square root (FollowCamera.h and
// Wrecker.h declare it too).
float UnknownFunction460b50(float value);

static inline float Magnitude(const Vector3& v)
{
    float squared = v.x * v.x + v.y * v.y + v.z * v.z;
    if (squared == 1.0f)
        return 1.0f;
    return UnknownFunction460b50(squared);
}

// 0x00443de0
void D3DIMSoultreeObject::SelectLod(Vector3* center, Vector3* extents)
{
    Vector3 boundsCenter;
    Vector3 boundsExtents;
    if (!center || !extents) {
        UnknownFunction4fe850(&boundsCenter, &boundsExtents);
        center = &boundsCenter;
        extents = &boundsExtents;
    }
    float distance = Magnitude(UnknownFunction4fd660(*center) -
                               ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->position);
    float range = distance - Magnitude(*extents);
    if (0.1f > range)
        range = 0.1f;
    float scale = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->imagePlaneDistance / range;
    if (field_0x288) {
        int lod = 0;
        for (int i = 0; i < lodCount - 1; i++) {
            if (scale * autoLodDistances[i] * field_0x284 < 1.0f)
                lod = i + 1;
        }
        if (field_0x27c != lod)
            UnknownFunction444d00(lod);
    }
    for (int i = 0; i < materialCount; i++) {
        SoultreeMaterial* material = materialTable[i];
        if (material->field_0x70) {
            ManagedTexture* texture = (ManagedTexture*)material->field_0x70->texture;
            if (texture->field_0x68 & 1) {
                float width = (float)texture->field_0x14;
                float size = scale * *(float*)&material->field_0xcc * width;
                if (size > width + width)
                    size = width + width;
                int power = 1;
                int level = 0;
                for (int n = (int)size >> 1; n; n >>= 1) {
                    power <<= 1;
                    level++;
                }
                float fraction = (size - power) / power;
                if (fraction < 0.0f)
                    fraction = 0.0f;
                texture->UnknownFunction510820(level + 1 + fraction);
            }
        }
    }
}

float FastInvSqrt(float x);                         // 0x00460c00

// |v| through the table-driven reciprocal square root, exact for 0 and 1.
static inline float Length(const Vector3& v)
{
    float squared = v.x * v.x + v.y * v.y + v.z * v.z;
    if (squared == 0.0f)
        return 0.0f;
    if (squared == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(squared);
}

// 0x00440810: for every surface of the current level of detail, the
// largest ratio of edge length to texture-coordinate length over its
// faces, divided by the texture height, raises each textured material's
// +0xcc.
void D3DIMSoultreeObject::UnknownFunction440810()
{
    TransformVertexGroups(-1);
    for (int i = 0; i < lodTable[field_0x27c].surfaceCount; i++) {
        UnknownSoultreeSurface* surface = &lodTable[field_0x27c].surfaces[i];
        float best = 0.0f;
        for (int f = 0; f < surface->faceCount; f++) {
            Vector3 zero(0.0f, 0.0f, 0.0f);
            unsigned short* face = &surface->indices[f * 3];
            Vector3 p0 = surface->vertices[face[0]].position;
            Vector3 p1 = surface->vertices[face[1]].position;
            Vector3 p2 = surface->vertices[face[2]].position;

            Vector3 delta = zero;
            delta.x = surface->vertices[face[0]].tu - surface->vertices[face[1]].tu;
            delta.y = surface->vertices[face[0]].tv - surface->vertices[face[1]].tv;
            Vector3 edge = p0 - p1;
            float uvLength = Length(delta);
            if (uvLength <= 0.01f)
                uvLength = 10000.0f;
            float ratio = Length(edge) / uvLength;
            if (best <= ratio)
                best = ratio;

            delta.x = surface->vertices[face[1]].tu - surface->vertices[face[2]].tu;
            delta.y = surface->vertices[face[1]].tv - surface->vertices[face[2]].tv;
            edge = p1 - p2;
            uvLength = Length(delta);
            if (uvLength <= 0.01f)
                uvLength = 10000.0f;
            ratio = Length(edge) / uvLength;
            if (best <= ratio)
                best = ratio;

            delta.x = surface->vertices[face[2]].tu - surface->vertices[face[0]].tu;
            delta.y = surface->vertices[face[2]].tv - surface->vertices[face[0]].tv;
            edge = p2 - p0;
            uvLength = Length(delta);
            if (uvLength <= 0.01f)
                uvLength = 10000.0f;
            ratio = Length(edge) / uvLength;
            if (best <= ratio)
                best = ratio;
        }
        for (int k = 0; k < surface->materialCount; k++) {
            SoultreeMaterial* material = materialTable[surface->materialIndices[k]];
            if (material->field_0x70) {
                float scale = best / material->field_0x70->texture->field_0x18;
                float current = *(float*)&material->field_0xcc;
                *(float*)&material->field_0xcc = current > scale ? current : scale;
            }
        }
    }
}

// 0x00442fe0
void D3DIMSoultreeObject::DrawNormals(UnknownSoultreeSurface* surface)
{
    static float length = 0.5f;                    // 0x00568944
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
    for (int i = 0; i < surface->groupCount; i++) {
        for (int j = 0; j < surface->groups[i].vertexCount; j++) {
            UnknownSoultreeFaceGroup* group = &surface->groups[i];
            Vector3 from;
            from.x = group->transformedVertices[j].position.x;
            from.y = group->transformedVertices[j].position.y;
            from.z = group->transformedVertices[j].position.z;
            Vector3 to = from + length * group->normals[j];
            from = group->node->UnknownFunction4fd660(from);
            to = surface->groups[i].node->UnknownFunction4fd660(to);
            UnknownSoultreeVertex line[2];
            line[0].position = from;
            line[0].tu = 0;
            line[0].tv = 0;
            line[0].specular = 0xff;
            line[0].diffuse = 0;
            line[1].position = to;
            line[1].tu = 0;
            line[1].tv = 0;
            line[1].diffuse = 0;
            line[1].specular = 0xffffff;
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)line, 2, 0);
        }
    }
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
}

// 0x004431f0
void D3DIMSoultreeObject::DrawVertexCrosses(UnknownSoultreeSurface* surface)
{
    static float size = 0.025f;                    // 0x00568948
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
    Vector3 offsets[6];
    UnknownSoultreeVertex cross[6];
    int k;
    for (k = 0; k < 6; k++) {
        offsets[k].x = 0.0f;
        offsets[k].y = 0.0f;
        offsets[k].z = 0.0f;
        cross[k].tu = 0;
        cross[k].tv = 0;
        cross[k].specular = 0xff0000;
        cross[k].diffuse = 0;
    }
    offsets[0].x += size;
    offsets[1].x -= size;
    offsets[2].y += size;
    offsets[3].y -= size;
    offsets[4].z += size;
    offsets[5].z -= size;
    for (int i = 0; i < surface->groupCount; i++) {
        for (int j = 0; j < surface->groups[i].vertexCount; j++) {
            for (k = 0; k < 6; k++) {
                UnknownSoultreeFaceGroup* group = &surface->groups[i];
                Vector3 point;
                point.x = group->transformedVertices[j].position.x;
                point.y = group->transformedVertices[j].position.y;
                point.z = group->transformedVertices[j].position.z;
                point += offsets[k];
                point = group->node->UnknownFunction4fd660(point);
                cross[k].position = point;
            }
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)cross, 6, 0);
        }
    }
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
}

// The box test at 0x00575a98 (VisibilityClipper, 0x0052f570, ret 0x1c):
// camera, view matrix, box centre and half extent, screen rectangle,
// corner count, and a last pointer argument.
class UnknownSoultreeClipper {
public:
    int UnknownFunction52f570(UnknownSoultreeCameraView* camera, Matrix4* matrix, Vector3* center,
                              Vector3* extents, UnknownSoultreeCounters* rect, int* corners, int* a7);
};
extern UnknownSoultreeClipper* g_UnknownSoultreeClipper575a98;

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
int D3DIMSoultreeObject::UnknownVirtualSlot12()
{
    field_0x2d4 = 0;
    if (!field_0x14c && field_0x190) {
        if (field_0x18c)
            UnknownFunction4fe0f0();
        Matrix4 world;
        ((UnknownSoultreeNodeView*)this)->UnknownFunction4fca80(0, &world);
        Vector3 center;
        Vector3 extents;
        UnknownFunction4fe850(&center, &extents);
        const Matrix4& projection = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->viewMatrix;
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
        if (g_UnknownSoultreeClipper575a98->UnknownFunction52f570((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08, (Matrix4*)&view, &center, &extents, &field_0x298, 0,
                                                                  &field_0x260)) {
            field_0x2d4 = 1;
            SelectLod(&center, &extents);
        }
    }
    return GameObject::UnknownVirtualSlot12();
}


// ---------------------------------------------------------------------------
// 0x00440f30..0x00442dd8: the drawing function. Not matched; see the notes
// at the top of this file.

// 0x0040ae30 (cdecl): the out-of-line dot product.
float UnknownFunction40ae30(const Vector3* a, const Vector3* b);
// 0x004a10e0 (cdecl): *out = v - 2 (v.n) n.
void UnknownFunction4a10e0(const Vector3* v, const Vector3* n, Vector3* out);

// The vector type of the drawing function's local statics: their atexit
// entries are the `ret` stubs 0x00442f40/0x00442f50/0x00442f60, so the type
// has an (empty) destructor.
struct UnknownSoultreeStaticVector : Vector3 {
    ~UnknownSoultreeStaticVector() {}
};

static inline Vector3 NormalizedByCall(const Vector3& v)
{
    float squared = UnknownFunction40ae30(&v, &v);
    if (squared == 1.0f)
        return v;
    float scale = FastInvSqrt(squared);
    return Vector3(v.x * scale, v.y * scale, v.z * scale);
}

static inline Vector3 Normalized(const Vector3& v)
{
    float squared = v.x * v.x + v.y * v.y + v.z * v.z;
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
        if (lights->field_0x30[i]->lightType == 1 || lights->field_0x30[i]->lightType == 2)
            light = lights->field_0x30[i];
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
    if (field_0x18c)
        TransformVertexGroups(-1);
    Matrix4 identity;
    memset(&identity, 0, sizeof(identity));
    identity.m[0][0] = 1.0f;
    identity.m[1][1] = 1.0f;
    identity.m[2][2] = 1.0f;
    identity.m[3][3] = 1.0f;
    ((RenderTarget*)field_0x18)->field_0x08->UnknownVirtualSlot30(&identity);
    int savedCull;
    int savedShade;
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot9(4, &savedCull);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot9(9, &savedShade);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, field_0x1b4, 0);
    if (field_0x1b8)
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD, 0);
    else
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT, 0);
    int k;
    for (int s = 0; s < lodTable[field_0x27c].surfaceCount; s++) {
        UnknownSoultreeSurface* surface = &lodTable[field_0x27c].surfaces[s];
        int count = surface->vertexCount;
        int modified = 0;
        int restore = 0;
        for (int m = 0; m < surface->materialCount; m++) {
            SoultreeMaterial* material = materialTable[surface->materialIndices[m]];
            if (field_0x1a4)
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
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
                Vector3 lightPosition = light->lightPosition;
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[j];
                    int vertices = group->vertexCount;
                    Vector3 position;
                    ((UnknownSoultreeNodeView*)group->node)->UnknownFunction4fc9a0(0, &position);
                    Vector3 direction = NormalizedByCall(position - lightPosition);
                    direction = ((UnknownSoultreeNodeView*)group->node)->UnknownFunction4fd710(&direction);
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
                Vector3 eye = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->position;
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[j];
                    int vertices = group->vertexCount;
                    Vector3 position;
                    ((UnknownSoultreeNodeView*)group->node)->UnknownFunction4fc9a0(0, &position);
                    Vector3 direction = NormalizedByCall(position - eye);
                    direction = ((UnknownSoultreeNodeView*)group->node)->UnknownFunction4fd710(&direction);
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
                    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
                UnknownSoultreeVertex line[2];
                line[0].diffuse = 0xffff0000;
                line[1].diffuse = 0xffffffff;
                if (updateEye)
                    (Vector3&)eye = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->position;
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[j];
                    Vector3 localEye = ((UnknownSoultreeNodeView*)group->node)->UnknownFunction4fd7f0(&eye);
                    if (!count)
                        continue;
                    for (int v = 0; v < group->vertexCount; v++) {
                        Vector3 direction;
                        direction.x = group->transformedVertices[v].position.x - localEye.x;
                        direction.y = group->transformedVertices[v].position.y - localEye.y;
                        direction.z = group->transformedVertices[v].position.z - localEye.z;
                        direction = Normalized(direction);
                        Vector3 reflected;
                        UnknownFunction4a10e0(&direction, &group->normals[v], &reflected);
                        if (drawReflections) {
                            Vector3 world = ((UnknownSoultreeNodeView*)group->node)->UnknownFunction4fd5c0(&reflected);
                            line[0].position.x = group->sourceVertices[v].position.x;
                            line[0].position.y = group->sourceVertices[v].position.y;
                            line[0].position.z = group->sourceVertices[v].position.z;
                            line[1].position.x = world.x + group->sourceVertices[v].position.x;
                            line[1].position.y = world.y + group->sourceVertices[v].position.y;
                            line[1].position.z = world.z + group->sourceVertices[v].position.z;
                            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)line, 2, 0);
                        }
                        group->sourceVertices[v].tv = ((float)asin(reflected.y) + 1.5707964f) * -0.31830987f;
                        group->sourceVertices[v].tv = -((reflected.y + 1.0f) * 0.5f);
                        group->sourceVertices[v].tv = 0.25f;
                        reflected.y = 0.0f;
                        reflected = Normalized(reflected);
                        group->sourceVertices[v].tu =
                            ((float)atan(reflected.x / reflected.z) + 1.5707964f) * 0.31830987f;
                    }
                }
                restore = 1;
                if (drawReflections)
                    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                break;
            }
            case 2: {
                Vector3 position = *(Vector3*)&worldMatrix.m[3][0];
                textureScroll += Length(field_0x2bc - position) * 0.0075f;
                field_0x2bc = position;
                if (textureScroll > 100.0f)
                    textureScroll -= 100.0f;
                float scroll = textureScroll;
                Vector3 eye = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->position;
                for (int j = 0; j < surface->groupCount; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->groups[j];
                    count = group->vertexCount;
                    if (!count)
                        continue;
                    group->node->UpdateWorldMatrix();
                    Matrix4* world = &group->node->worldMatrix;
                    for (int v = 0; v < count; v++) {
                        Vector3 direction = Normalized(group->sourceVertices[v].position - eye);
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
                ((UnknownSoultreeNodeView*)this)->UnknownFunction4fc9a0(0, &center);
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
                    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
                    ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)ring, 32, 0);
                }
                if (updateView) {
                    (Vector3&)viewDirection = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->viewDirection;
                    (Vector3&)viewPosition = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->position;
                }
                UnknownSoultreeVertex lines[4];
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
                        float facing = direction.x * n.x + direction.y * n.y + direction.z * n.z;
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
                            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
                            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)lines, 2, 0);
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
                    Vector3 lightPosition = ((UnknownSoultreeNodeView*)group->node)->UnknownFunction4fd710(&light->lightPosition);
                    Vector3 eye = ((UnknownSoultreeNodeView*)group->node)->UnknownFunction4fd710(
                        &((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->position);
                    for (int v = 0; v < group->vertexCount; v++) {
                        Vector3 position = group->sourceVertices[v].position;
                        Vector3 toLight = Normalized(position - lightPosition);
                        Vector3 toEye = Normalized(eye - position);
                        Vector3& normal = group->normals[v];
                        float d = (toLight.x * normal.x + toLight.y * normal.y + toLight.z * normal.z) * 2.0f;
                        Vector3 reflected = toLight - normal * d;
                        float specular = toEye.x * reflected.x + toEye.y * reflected.y + toEye.z * reflected.z;
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
                if (material->field_0x70) {
                    ManagedTexture* texture = (ManagedTexture*)material->field_0x70->texture;
                    if (texture->field_0x68 & 1)
                        texture->UnknownFunction510910(&drawn->field_0x2c, (float*)&drawn->field_0x30,
                                                       (float*)&drawn->field_0x34, &drawn->vertices->tu,
                                                       &drawn->vertices->tv, drawn->vertexCount, 0x20);
                }
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot15(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, (int)drawn->vertices, drawn->vertexCount,
                                                                  (int)drawn->indices, drawn->faceCount * 3, 0);
                if (field_0x1b0)
                    DrawWireframe(drawn);
                if (field_0x1a8)
                    DrawNormals(drawn);
                if (field_0x1ac)
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
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, savedCull, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_SHADEMODE, savedShade, 0);
}

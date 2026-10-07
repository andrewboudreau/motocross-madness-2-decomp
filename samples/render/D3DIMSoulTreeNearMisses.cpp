// Near-miss D3DIMSoulTree.CPP candidates (D3DIMSoultreeObject), kept out of
// src/reconstructed/D3DIMSoulTree.cpp until they match. The class, its
// evidence and the exact functions are in src/reconstructed/D3DIMSoulTree.h
// and .cpp. Bindings: D3DIMSoulTreeNearMisses.bindings.json.
//
// UnknownFunction440060 (0x00440060, 1964 bytes): 1955/1964. The .slt LOD
// reader. Two operand orders: retail forms the surface address as
// `offset + surfaces` (mov ebx, offset; mov edi, surfaces; add ebx, edi)
// and stores the face indices as [faces + offset]; VC6 emits
// `surfaces + offset` and [offset + faces] for every indexing form tried
// (&a[i], a + i, i + a, a pointer local, a (*)[3] cast). Same pattern as
// samples/camera/FollowCameraNearMisses.cpp's 0x00463140.
//
// UnknownFunction440d40 (0x00440d40, 484 bytes): 241/484, 2 bytes short.
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
// +0x164 address in ebp where retail uses edx.
//
// UnknownFunction443de0 (0x00443de0, 592 bytes): the level-of-detail and
// mip chooser. Writing the camera expression without a local reproduces
// retail's `add esi, 0x170`. Retail keeps `center`/`extents` in ebx/ebp
// from the prologue and multiplies scale * +0x280[i] * +0x284 and
// scale * +0xcc * width in that order; VC6 reorders both products.
// `*(float*)&field_0xcc`: SoultreeMaterial.h types +0xcc as int.
//
// SoultreeVirtualSlot7 (0x00444560, 1008 bytes): 680/1233 with the source
// in ebp instead of ebx and the two node lists in swapped frame slots
// (retail frame 0x2c, ours 0x30).
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
// different order.
//
// UnknownFunction442fe0 (normals, 528 bytes) and UnknownFunction4431f0
// (vertex crosses, 512 bytes): both scale by function-local statics in
// .data (0x00568944 = 0.5, 0x00568948 = 0.025), which is what puts the
// variable before the vertex operand in retail's fmul/fadd. Remaining:
// retail materializes the face-group address with `lea` and keeps
// different induction registers.
//
// UnknownFunction440f30 (0x00440f30..0x00442dd8, 7848 bytes): the drawing
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
int D3DIMSoultreeObject::UnknownFunction440060()
{
    int untextured = 0;
    int count = UnknownFunction4fda30();
    SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x1dd);
    int index = 1;
    nodes[0] = this;
    UnknownFunction4fda60(&index, nodes);
    char section[0x80];
    sprintf(section, "LOD Information");
    field_0x1a0->UnknownFunction4b78f0(section);
    field_0x1a0->UnknownFunction4b7f10("NumberOfLOD", 0, &field_0x274);
    field_0x1a0->UnknownFunction4b7f10("UseAutoLOD", 0, &field_0x288);
    int i;
    if (field_0x288) {
        field_0x280 = (float*)DebugMalloc(field_0x274 * 4 - 1, __FILE__, 0x1ec);
        for (i = 0; i < field_0x274 - 1; i++) {
            sprintf(section, "AutoLOD#%i", i);
            field_0x1a0->UnknownFunction4b7f40(section, -1.0f, &field_0x280[i]);
        }
    }
    field_0x28c = (UnknownSoultreeLod*)DebugMalloc(field_0x274 * 8, __FILE__, 0x1f5);
    UnknownSoultreeFaceGroup* groups = (UnknownSoultreeFaceGroup*)DebugMalloc(count * 0x14, __FILE__, 0x1f9);
    for (i = 0; i < count; i++)
        groups[i].field_0x00 = nodes[i];
    for (int lod = 0; lod < field_0x274; lod++) {
        UnknownSoultreeLod* level = &field_0x28c[lod];
        sprintf(section, "LOD %i", lod);
        field_0x1a0->UnknownFunction4b78f0(section);
        field_0x1a0->UnknownFunction4b7f10("NumberOfSurfaces", 0, &level->field_0x00);
        level->field_0x04 = (UnknownSoultreeSurface*)DebugMalloc(level->field_0x00 * 0x38, __FILE__, 0x206);
        for (int s = 0; s < level->field_0x00; s++) {
            UnknownSoultreeSurface* surface = &level->field_0x04[s];
            surface->field_0x2c = 1.0f;
            surface->field_0x30 = 0;
            surface->field_0x34 = 0;
            sprintf(section, "LOD %i - Surface %i", lod, s);
            field_0x1a0->UnknownFunction4b78f0(section);
            field_0x1a0->UnknownFunction4b7f10("NumberOfVertices", 0, &surface->field_0x08);
            field_0x1a0->UnknownFunction4b7f10("NumberOfFaces", 0, &surface->field_0x0c);
            field_0x1a0->UnknownFunction4b7f10("NumberOfMaterials", 0, &surface->field_0x20);
            if (surface->field_0x20 > 0) {
                surface->field_0x24 = (int*)DebugMalloc(surface->field_0x20 * 4, __FILE__, 0x216);
                for (int m = 0; m < surface->field_0x20; m++) {
                    char key[0x80];
                    sprintf(key, "Material#%i", m);
                    field_0x1a0->UnknownFunction4b7f10(key, 0, &surface->field_0x24[m]);
                }
            } else {
                untextured = 1;
                surface->field_0x24 = (int*)DebugMalloc(4, __FILE__, 0x21e);
                surface->field_0x24[0] = 0;
                surface->field_0x20 = 1;
            }
            surface->field_0x10 = (UnknownSoultreeVertex*)DebugMalloc(surface->field_0x08 * 0x20, __FILE__, 0x223);
            surface->field_0x14 = (UnknownSoultreeVertex*)DebugMalloc(surface->field_0x08 * 0x20, __FILE__, 0x224);
            surface->field_0x18 = (Vector3*)DebugMalloc(surface->field_0x08 * 0xc, __FILE__, 0x225);
            surface->field_0x1c = (unsigned short*)DebugMalloc(surface->field_0x0c * 6, __FILE__, 0x226);
            surface->field_0x28 = (UnknownSoultreeUV*)DebugMalloc(surface->field_0x08 * 8, __FILE__, 0x227);
            sprintf(section, "LOD %i - Surface %i - Vertices", lod, s);
            field_0x1a0->UnknownFunction4b7f70(section);
            int v;
            for (v = 0; v < surface->field_0x08; v++) {
                float red;
                float green;
                float blue;
                field_0x1a0->UnknownFunction4b8010(0);
                field_0x1a0->UnknownFunction4b81c0(0, &surface->field_0x14[v].field_0x00.x);
                field_0x1a0->UnknownFunction4b81c0(1, &surface->field_0x14[v].field_0x00.y);
                field_0x1a0->UnknownFunction4b81c0(2, &surface->field_0x14[v].field_0x00.z);
                field_0x1a0->UnknownFunction4b81c0(3, &surface->field_0x18[v].x);
                field_0x1a0->UnknownFunction4b81c0(4, &surface->field_0x18[v].y);
                field_0x1a0->UnknownFunction4b81c0(5, &surface->field_0x18[v].z);
                field_0x1a0->UnknownFunction4b81c0(6, &surface->field_0x14[v].field_0x18);
                field_0x1a0->UnknownFunction4b81c0(7, &surface->field_0x14[v].field_0x1c);
                field_0x1a0->UnknownFunction4b81c0(12, &red);
                field_0x1a0->UnknownFunction4b81c0(13, &green);
                field_0x1a0->UnknownFunction4b81c0(14, &blue);
                surface->field_0x14[v].field_0x10 = 0xff000000 | ((long)(red * 255.0) << 16) |
                                                    ((long)(green * 255.0) << 8) | (long)(blue * 255.0);
                surface->field_0x14[v].field_0x14 = 0;
                surface->field_0x10[v] = surface->field_0x14[v];
                surface->field_0x28[v].field_0x00 = surface->field_0x10[v].field_0x18;
                surface->field_0x28[v].field_0x04 = surface->field_0x10[v].field_0x1c;
            }
            sprintf(section, "LOD %i - Surface %i - Faces", lod, s);
            field_0x1a0->UnknownFunction4b7f70(section);
            for (int f = 0; f < surface->field_0x0c; f++) {
                int a;
                int b;
                int c;
                field_0x1a0->UnknownFunction4b8010(0);
                field_0x1a0->UnknownFunction4b8180(0, &a);
                field_0x1a0->UnknownFunction4b8180(1, &b);
                field_0x1a0->UnknownFunction4b8180(2, &c);
                surface->field_0x1c[f * 3] = a;
                surface->field_0x1c[f * 3 + 1] = b;
                surface->field_0x1c[f * 3 + 2] = c;
            }
            sprintf(section, "LOD %i - Surface %i - Object Pointer List", lod, s);
            field_0x1a0->UnknownFunction4b7f70(section);
            int used = 0;
            for (i = 0; i < count; i++) {
                int first;
                int vertices;
                field_0x1a0->UnknownFunction4b8010(0);
                field_0x1a0->UnknownFunction4b8180(0, &first);
                field_0x1a0->UnknownFunction4b8180(1, &vertices);
                groups[i].field_0x04 = vertices;
                groups[i].field_0x08 = &surface->field_0x10[first];
                groups[i].field_0x0c = &surface->field_0x14[first];
                groups[i].field_0x10 = &surface->field_0x18[first];
                if (vertices > 0)
                    used++;
            }
            surface->field_0x00 = used;
            surface->field_0x04 = (UnknownSoultreeFaceGroup*)DebugMalloc(used * 0x14, __FILE__, 0x283);
            int k = 0;
            for (i = 0; i < count; i++) {
                if (groups[i].field_0x04 > 0)
                    surface->field_0x04[k++] = groups[i];
            }
        }
    }
    UnknownFunction444440();
    operator delete(groups, __FILE__, 0x291);
    operator delete(nodes, __FILE__, 0x292);
    return untextured;
}

// 0x00440d40
void D3DIMSoultreeObject::UnknownFunction440d40(int lod)
{
    int saved = field_0x27c;
    if (lod != -1)
        field_0x27c = lod;
    for (int i = 0; i < field_0x28c[field_0x27c].field_0x00; i++) {
        UnknownSoultreeSurface* surface = &field_0x28c[field_0x27c].field_0x04[i];
        int colored = 0;
        int k;
        for (k = 0; k < surface->field_0x20; k++) {
            SoultreeMaterial* material = field_0x290[surface->field_0x24[k]];
            if (!material->field_0x9c || material->field_0xa0)
                colored = 1;
        }
        UnknownSoultreeSurface* drawn;
        if (field_0x26c > 0) {
            for (k = 0; k < field_0x26c; k++)
                ((D3DIMSoultreeModifier*)field_0x270[k])->UnknownVirtualSlot27(
                    this, (UnknownSoultreeMesh*)surface, (UnknownSoultreeMesh**)&drawn);
        } else {
            drawn = surface;
        }
        for (int j = 0; j < drawn->field_0x00; j++) {
            UnknownSoultreeFaceGroup* group = &drawn->field_0x04[j];
            UnknownSoultreeFaceGroup* source = &surface->field_0x04[j];
            int count = group->field_0x04;
            if (count) {
                group->field_0x00->UnknownFunction4fb4f0();
                void* target = group->field_0x0c;
                void* vertices = source->field_0x08;
                if (field_0x1a4) {
                    if (colored)
                        field_0x1bc->UnknownFunction49e4a0(&group->field_0x00->field_0x0f8, count,
                                                           group->field_0x10, vertices, 0xc, target);
                    else
                        field_0x1bc->UnknownFunction49e4a0(&group->field_0x00->field_0x0f8, count,
                                                           group->field_0x10, vertices, 0xc, 0);
                }
                UnknownFunction4a1b00(vertices, target, &group->field_0x00->field_0x0f8, count, 0x20, 0x20);
            }
        }
    }
    field_0x18c = 0;
    field_0x27c = saved;
}

// 0x004435b0
void D3DIMSoultreeObject::UnknownFunction4435b0()
{
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 0, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 1);
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
        vertices[i].field_0x00 = points[i];
        vertices[i].field_0x18 = 0;
        vertices[i].field_0x1c = 0;
        vertices[i].field_0x14 = 0;
    }
    vertices[0].field_0x10 = 0xffffff;
    vertices[1].field_0x10 = 0xff0000;
    vertices[2].field_0x10 = 0xff;
    vertices[3].field_0x10 = 0xff00;
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot15(2, 0x1e2, (int)vertices, 4, (int)indices, 6, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
    if (field_0x144)
        ((D3DIMSoultreeObject*)field_0x144)->UnknownFunction4435b0();
    if (field_0x140)
        ((D3DIMSoultreeObject*)field_0x140)->UnknownFunction4435b0();
}

// 0x00443740
void D3DIMSoultreeObject::UnknownFunction443740(Vector3 center, Vector3 extents, int flag)
{
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 1);
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
        vertices[i].field_0x00 = corners[i];
        vertices[i].field_0x18 = 0;
        vertices[i].field_0x1c = 0;
        if (flag)
            vertices[i].field_0x14 = 0;
        else
            vertices[i].field_0x14 = 0xff0000;
        vertices[i].field_0x10 = 0;
    }
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot15(2, 0x1e2, (int)vertices, 8, (int)indices, 24, 0);
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
    while (!model->field_0x28c)
        model = (D3DIMSoultreeObject*)model->field_0x13c;
    field_0x150 = 0;
    Vector3 low(64000.0f, 64000.0f, 64000.0f);
    Vector3 high(-64000.0f, -64000.0f, -64000.0f);
    for (int lod = 0; lod < model->field_0x274; lod++) {
        for (int i = 0; i < model->field_0x28c[lod].field_0x00; i++) {
            for (int j = 0; j < model->field_0x28c[lod].field_0x04[i].field_0x00; j++) {
                if (model->field_0x28c[lod].field_0x04[i].field_0x04[j].field_0x00 == this) {
                    UnknownSoultreeVertex* vertex = model->field_0x28c[lod].field_0x04[i].field_0x04[j].field_0x0c;
                    int count = model->field_0x28c[lod].field_0x04[i].field_0x04[j].field_0x04;
                    for (int k = 0; k < count; k++) {
                        Raise(high.x, vertex[k].field_0x00.x);
                        Raise(high.y, vertex[k].field_0x00.y);
                        Raise(high.z, vertex[k].field_0x00.z);
                        Lower(low.x, vertex[k].field_0x00.x);
                        Lower(low.y, vertex[k].field_0x00.y);
                        Lower(low.z, vertex[k].field_0x00.z);
                        field_0x150 = 1;
                    }
                }
            }
        }
    }
    if (!field_0x150) {
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
    field_0x244 = source->field_0x244;
    field_0x240 = source->field_0x240;
    field_0x1bc = source->field_0x1bc;
    SoultreeObject::SoultreeVirtualSlot7(source);
    UnknownFunction444a40(source);
    field_0x274 = source->field_0x274;
    field_0x27c = source->field_0x27c;
    field_0x278 = source->field_0x278;
    field_0x288 = source->field_0x288;
    if (field_0x288) {
        field_0x280 = (float*)DebugMalloc(field_0x274 * 4, __FILE__, 0x88e);
        for (int i = 0; i < field_0x274; i++)
            field_0x280[i] = source->field_0x280[i];
    }
    if (source->field_0x28c) {
        int count = UnknownFunction4fda30();
        SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x897);
        int index = 1;
        nodes[0] = this;
        UnknownFunction4fda60(&index, nodes);
        SoultreeObject** sourceNodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x89d);
        index = 1;
        sourceNodes[0] = source;
        source->UnknownFunction4fda60(&index, sourceNodes);
        field_0x28c = (UnknownSoultreeLod*)DebugMalloc(field_0x274 * 8, __FILE__, 0x8a2);
        for (int lod = 0; lod < field_0x274; lod++) {
            UnknownSoultreeLod* from = &source->field_0x28c[lod];
            UnknownSoultreeLod* to = &field_0x28c[lod];
            to->field_0x00 = from->field_0x00;
            to->field_0x04 = (UnknownSoultreeSurface*)DebugMalloc(from->field_0x00 * 0x38, __FILE__, 0x8aa);
            for (int i = 0; i < to->field_0x00; i++) {
                UnknownSoultreeSurface* s = &from->field_0x04[i];
                UnknownSoultreeSurface* d = &to->field_0x04[i];
                d->field_0x08 = s->field_0x08;
                d->field_0x0c = s->field_0x0c;
                d->field_0x20 = s->field_0x20;
                d->field_0x24 = (int*)DebugMalloc(d->field_0x20 * 4, __FILE__, 0x8b3);
                int k;
                for (k = 0; k < d->field_0x20; k++)
                    d->field_0x24[k] = s->field_0x24[k];
                d->field_0x2c = s->field_0x2c;
                d->field_0x30 = s->field_0x30;
                d->field_0x34 = s->field_0x34;
                d->field_0x10 = (UnknownSoultreeVertex*)DebugMalloc(d->field_0x08 * 0x20, __FILE__, 0x8bd);
                d->field_0x14 = (UnknownSoultreeVertex*)DebugMalloc(d->field_0x08 * 0x20, __FILE__, 0x8be);
                d->field_0x18 = (Vector3*)DebugMalloc(d->field_0x08 * 0xc, __FILE__, 0x8bf);
                d->field_0x1c = (unsigned short*)DebugMalloc(d->field_0x0c * 6, __FILE__, 0x8c0);
                d->field_0x28 = (UnknownSoultreeUV*)DebugMalloc(d->field_0x08 * 8, __FILE__, 0x8c1);
                memcpy(d->field_0x10, s->field_0x10, d->field_0x08 * 0x20);
                memcpy(d->field_0x14, s->field_0x14, d->field_0x08 * 0x20);
                memcpy(d->field_0x18, s->field_0x18, d->field_0x08 * 0xc);
                memcpy(d->field_0x1c, s->field_0x1c, d->field_0x0c * 6);
                memcpy(d->field_0x28, s->field_0x28, d->field_0x08 * 8);
                d->field_0x00 = s->field_0x00;
                d->field_0x04 = (UnknownSoultreeFaceGroup*)DebugMalloc(d->field_0x00 * 0x14, __FILE__, 0x8cb);
                int first = 0;
                for (int j = 0; j < d->field_0x00; j++) {
                    d->field_0x04[j].field_0x00 = 0;
                    for (k = 0; !d->field_0x04[j].field_0x00; k++) {
                        if (sourceNodes[k] == s->field_0x04[j].field_0x00)
                            d->field_0x04[j].field_0x00 = nodes[k];
                    }
                    d->field_0x04[j].field_0x04 = s->field_0x04[j].field_0x04;
                    d->field_0x04[j].field_0x08 = &d->field_0x10[first];
                    d->field_0x04[j].field_0x0c = &d->field_0x14[first];
                    d->field_0x04[j].field_0x10 = &d->field_0x18[first];
                    first += d->field_0x04[j].field_0x04;
                }
            }
        }
        operator delete(nodes, __FILE__, 0x8e1);
        operator delete(sourceNodes, __FILE__, 0x8e2);
        UnknownFunction444440();
    } else {
        field_0x28c = 0;
        UnknownFunction444440();
    }
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
void D3DIMSoultreeObject::UnknownFunction443de0(Vector3* center, Vector3* extents)
{
    Vector3 boundsCenter;
    Vector3 boundsExtents;
    if (!center || !extents) {
        UnknownFunction4fe850(&boundsCenter, &boundsExtents);
        center = &boundsCenter;
        extents = &boundsExtents;
    }
    float distance = Magnitude(UnknownFunction4fd660(*center) -
                               ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->field_0x170);
    float range = distance - Magnitude(*extents);
    if (0.1f > range)
        range = 0.1f;
    float scale = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->field_0x198 / range;
    if (field_0x288) {
        int lod = 0;
        for (int i = 0; i < field_0x274 - 1; i++) {
            if (scale * field_0x280[i] * field_0x284 < 1.0f)
                lod = i + 1;
        }
        if (field_0x27c != lod)
            UnknownFunction444d00(lod);
    }
    for (int i = 0; i < field_0x294; i++) {
        SoultreeMaterial* material = field_0x290[i];
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
    UnknownFunction440d40(-1);
    for (int i = 0; i < field_0x28c[field_0x27c].field_0x00; i++) {
        UnknownSoultreeSurface* surface = &field_0x28c[field_0x27c].field_0x04[i];
        float best = 0.0f;
        for (int f = 0; f < surface->field_0x0c; f++) {
            Vector3 zero(0.0f, 0.0f, 0.0f);
            unsigned short* face = &surface->field_0x1c[f * 3];
            Vector3 p0 = surface->field_0x10[face[0]].field_0x00;
            Vector3 p1 = surface->field_0x10[face[1]].field_0x00;
            Vector3 p2 = surface->field_0x10[face[2]].field_0x00;

            Vector3 delta = zero;
            delta.x = surface->field_0x10[face[0]].field_0x18 - surface->field_0x10[face[1]].field_0x18;
            delta.y = surface->field_0x10[face[0]].field_0x1c - surface->field_0x10[face[1]].field_0x1c;
            Vector3 edge = p0 - p1;
            float uvLength = Length(delta);
            if (uvLength <= 0.01f)
                uvLength = 10000.0f;
            float ratio = Length(edge) / uvLength;
            if (best <= ratio)
                best = ratio;

            delta.x = surface->field_0x10[face[1]].field_0x18 - surface->field_0x10[face[2]].field_0x18;
            delta.y = surface->field_0x10[face[1]].field_0x1c - surface->field_0x10[face[2]].field_0x1c;
            edge = p1 - p2;
            uvLength = Length(delta);
            if (uvLength <= 0.01f)
                uvLength = 10000.0f;
            ratio = Length(edge) / uvLength;
            if (best <= ratio)
                best = ratio;

            delta.x = surface->field_0x10[face[2]].field_0x18 - surface->field_0x10[face[0]].field_0x18;
            delta.y = surface->field_0x10[face[2]].field_0x1c - surface->field_0x10[face[0]].field_0x1c;
            edge = p2 - p0;
            uvLength = Length(delta);
            if (uvLength <= 0.01f)
                uvLength = 10000.0f;
            ratio = Length(edge) / uvLength;
            if (best <= ratio)
                best = ratio;
        }
        for (int k = 0; k < surface->field_0x20; k++) {
            SoultreeMaterial* material = field_0x290[surface->field_0x24[k]];
            if (material->field_0x70) {
                float scale = best / material->field_0x70->texture->field_0x18;
                float current = *(float*)&material->field_0xcc;
                *(float*)&material->field_0xcc = current > scale ? current : scale;
            }
        }
    }
}

// 0x00442fe0
void D3DIMSoultreeObject::UnknownFunction442fe0(UnknownSoultreeSurface* surface)
{
    static float length = 0.5f;                    // 0x00568944
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 1);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
    for (int i = 0; i < surface->field_0x00; i++) {
        for (int j = 0; j < surface->field_0x04[i].field_0x04; j++) {
            UnknownSoultreeFaceGroup* group = &surface->field_0x04[i];
            Vector3 from;
            from.x = group->field_0x0c[j].field_0x00.x;
            from.y = group->field_0x0c[j].field_0x00.y;
            from.z = group->field_0x0c[j].field_0x00.z;
            Vector3 to = from + length * group->field_0x10[j];
            from = group->field_0x00->UnknownFunction4fd660(from);
            to = surface->field_0x04[i].field_0x00->UnknownFunction4fd660(to);
            UnknownSoultreeVertex line[2];
            line[0].field_0x00 = from;
            line[0].field_0x18 = 0;
            line[0].field_0x1c = 0;
            line[0].field_0x14 = 0xff;
            line[0].field_0x10 = 0;
            line[1].field_0x00 = to;
            line[1].field_0x18 = 0;
            line[1].field_0x1c = 0;
            line[1].field_0x10 = 0;
            line[1].field_0x14 = 0xffffff;
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(2, 0x1e2, (int)line, 2, 0);
        }
    }
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
}

// 0x004431f0
void D3DIMSoultreeObject::UnknownFunction4431f0(UnknownSoultreeSurface* surface)
{
    static float size = 0.025f;                    // 0x00568948
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 1);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
    Vector3 offsets[6];
    UnknownSoultreeVertex cross[6];
    int k;
    for (k = 0; k < 6; k++) {
        offsets[k].x = 0.0f;
        offsets[k].y = 0.0f;
        offsets[k].z = 0.0f;
        cross[k].field_0x18 = 0;
        cross[k].field_0x1c = 0;
        cross[k].field_0x14 = 0xff0000;
        cross[k].field_0x10 = 0;
    }
    offsets[0].x += size;
    offsets[1].x -= size;
    offsets[2].y += size;
    offsets[3].y -= size;
    offsets[4].z += size;
    offsets[5].z -= size;
    for (int i = 0; i < surface->field_0x00; i++) {
        for (int j = 0; j < surface->field_0x04[i].field_0x04; j++) {
            for (k = 0; k < 6; k++) {
                UnknownSoultreeFaceGroup* group = &surface->field_0x04[i];
                Vector3 point;
                point.x = group->field_0x0c[j].field_0x00.x;
                point.y = group->field_0x0c[j].field_0x00.y;
                point.z = group->field_0x0c[j].field_0x00.z;
                point += offsets[k];
                point = group->field_0x00->UnknownFunction4fd660(point);
                cross[k].field_0x00 = point;
            }
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(2, 0x1e2, (int)cross, 6, 0);
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
        const Matrix4& projection = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->field_0x0ec;
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
            UnknownFunction443de0(&center, &extents);
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
    for (int i = 0; i < lights->field_0x2c; i++) {
        if (lights->field_0x30[i]->field_0x2c == 1 || lights->field_0x30[i]->field_0x2c == 2)
            light = lights->field_0x30[i];
    }
    return light;
}

// 0x00440f30: draws the current level of detail. Per surface and material
// the material's mapping type (+0xc8) rewrites the texture coordinates of
// the source vertices first; afterwards they are restored from +0x28.
void D3DIMSoultreeObject::UnknownFunction440f30()
{
    if (!field_0x28c)
        return;
    if (field_0x18c)
        UnknownFunction440d40(-1);
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
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(4, field_0x1b4, 0);
    if (field_0x1b8)
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(9, 2, 0);
    else
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(9, 1, 0);
    int k;
    for (int s = 0; s < field_0x28c[field_0x27c].field_0x00; s++) {
        UnknownSoultreeSurface* surface = &field_0x28c[field_0x27c].field_0x04[s];
        int count = surface->field_0x08;
        int modified = 0;
        int restore = 0;
        for (int m = 0; m < surface->field_0x20; m++) {
            SoultreeMaterial* material = field_0x290[surface->field_0x24[m]];
            if (field_0x1a4)
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
            material->UnknownFunction4ff180();
            switch (material->field_0xc8) {
            case 0:
                modified = 1;
                break;
            case 7: {
                modified = 1;
                restore = 1;
                for (int v = 0; v < count; v++) {
                    if (surface->field_0x14[v].field_0x00.x > 0.0f) {
                        surface->field_0x10[v].field_0x1c = Remap(surface->field_0x14[v].field_0x00.y, -1.0f, 3.0f);
                        surface->field_0x10[v].field_0x18 = RemapDown(surface->field_0x14[v].field_0x00.z, 1.0f, -2.4f);
                    } else {
                        surface->field_0x10[v].field_0x1c = Remap(surface->field_0x14[v].field_0x00.y, -1.0f, 3.0f);
                        surface->field_0x10[v].field_0x18 = RemapDown(surface->field_0x14[v].field_0x00.z, -1.0f, 1.4f);
                    }
                }
                break;
            }
            case 1: {
                field_0x2c8 += field_0x2b8 * material->field_0xc4;
                while (field_0x2c8 > 1.0f)
                    field_0x2c8 -= 1.0f;
                for (int v = 0; v < count; v++)
                    surface->field_0x10[v].field_0x1c = surface->field_0x10[v].field_0x1c + field_0x2c8;
                modified = 1;
                restore = 1;
                break;
            }
            case 3: {
                LightManager* lights = field_0x1bc;
                if (!lights)
                    break;
                LightEmitter* light = FindLight(lights);
                if (!light)
                    break;
                Vector3 lightPosition = light->field_0x64;
                for (int j = 0; j < surface->field_0x00; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->field_0x04[j];
                    int vertices = group->field_0x04;
                    Vector3 position;
                    ((UnknownSoultreeNodeView*)group->field_0x00)->UnknownFunction4fc9a0(0, &position);
                    Vector3 direction = NormalizedByCall(position - lightPosition);
                    direction = ((UnknownSoultreeNodeView*)group->field_0x00)->UnknownFunction4fd710(&direction);
                    for (int v = 0; v < vertices; v++) {
                        group->field_0x08[v].field_0x18 =
                            1.0f - (direction.y * group->field_0x10[v].y + direction.x * group->field_0x10[v].x +
                                    direction.z * group->field_0x10[v].z + 1.0f) * 0.5f;
                        group->field_0x08[v].field_0x1c = 0;
                    }
                }
                modified = 1;
                restore = 1;
                break;
            }
            case 4: {
                Vector3 eye = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->field_0x170;
                for (int j = 0; j < surface->field_0x00; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->field_0x04[j];
                    int vertices = group->field_0x04;
                    Vector3 position;
                    ((UnknownSoultreeNodeView*)group->field_0x00)->UnknownFunction4fc9a0(0, &position);
                    Vector3 direction = NormalizedByCall(position - eye);
                    direction = ((UnknownSoultreeNodeView*)group->field_0x00)->UnknownFunction4fd710(&direction);
                    for (int v = 0; v < vertices; v++) {
                        group->field_0x08[v].field_0x18 =
                            1.0f - (direction.y * group->field_0x10[v].y + direction.x * group->field_0x10[v].x +
                                    direction.z * group->field_0x10[v].z + 1.0f) * 0.5f;
                        group->field_0x08[v].field_0x1c = 0;
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
                    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
                UnknownSoultreeVertex line[2];
                line[0].field_0x10 = 0xffff0000;
                line[1].field_0x10 = 0xffffffff;
                if (updateEye)
                    (Vector3&)eye = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->field_0x170;
                for (int j = 0; j < surface->field_0x00; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->field_0x04[j];
                    Vector3 localEye = ((UnknownSoultreeNodeView*)group->field_0x00)->UnknownFunction4fd7f0(&eye);
                    if (!count)
                        continue;
                    for (int v = 0; v < group->field_0x04; v++) {
                        Vector3 direction;
                        direction.x = group->field_0x0c[v].field_0x00.x - localEye.x;
                        direction.y = group->field_0x0c[v].field_0x00.y - localEye.y;
                        direction.z = group->field_0x0c[v].field_0x00.z - localEye.z;
                        direction = Normalized(direction);
                        Vector3 reflected;
                        UnknownFunction4a10e0(&direction, &group->field_0x10[v], &reflected);
                        if (drawReflections) {
                            Vector3 world = ((UnknownSoultreeNodeView*)group->field_0x00)->UnknownFunction4fd5c0(&reflected);
                            line[0].field_0x00.x = group->field_0x08[v].field_0x00.x;
                            line[0].field_0x00.y = group->field_0x08[v].field_0x00.y;
                            line[0].field_0x00.z = group->field_0x08[v].field_0x00.z;
                            line[1].field_0x00.x = world.x + group->field_0x08[v].field_0x00.x;
                            line[1].field_0x00.y = world.y + group->field_0x08[v].field_0x00.y;
                            line[1].field_0x00.z = world.z + group->field_0x08[v].field_0x00.z;
                            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(2, 0x1e2, (int)line, 2, 0);
                        }
                        group->field_0x08[v].field_0x1c = ((float)asin(reflected.y) + 1.5707964f) * -0.31830987f;
                        group->field_0x08[v].field_0x1c = -((reflected.y + 1.0f) * 0.5f);
                        group->field_0x08[v].field_0x1c = 0.25f;
                        reflected.y = 0.0f;
                        reflected = Normalized(reflected);
                        group->field_0x08[v].field_0x18 =
                            ((float)atan(reflected.x / reflected.z) + 1.5707964f) * 0.31830987f;
                    }
                }
                restore = 1;
                if (drawReflections)
                    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 4);
                break;
            }
            case 2: {
                Vector3 position = *(Vector3*)&field_0x0f8.m[3][0];
                field_0x2c8 += Length(field_0x2bc - position) * 0.0075f;
                field_0x2bc = position;
                if (field_0x2c8 > 100.0f)
                    field_0x2c8 -= 100.0f;
                float scroll = field_0x2c8;
                Vector3 eye = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->field_0x170;
                for (int j = 0; j < surface->field_0x00; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->field_0x04[j];
                    count = group->field_0x04;
                    if (!count)
                        continue;
                    group->field_0x00->UnknownFunction4fb4f0();
                    Matrix4* world = &group->field_0x00->field_0x0f8;
                    for (int v = 0; v < count; v++) {
                        Vector3 direction = Normalized(group->field_0x08[v].field_0x00 - eye);
                        Vector3 normal = group->field_0x10[v];
                        Vector3 n(normal.x * world->m[0][0] + normal.y * world->m[1][0] + normal.z * world->m[2][0],
                                  normal.x * world->m[0][1] + normal.y * world->m[1][1] + normal.z * world->m[2][1],
                                  normal.x * world->m[0][2] + normal.y * world->m[1][2] + normal.z * world->m[2][2]);
                        Vector3 twice = n + n;
                        float d = direction.x * n.x + direction.y * n.y + direction.z * n.z;
                        Vector3 reflected = twice * d - direction;
                        group->field_0x08[v].field_0x18 = reflected.x * 0.5f + 0.5f;
                        group->field_0x08[v].field_0x1c = reflected.y * 0.5f + 0.5f - scroll;
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
                    ring[i].field_0x10 = 0xff00ff00;
                    ring[i].field_0x14 = 0xff000000;
                    ring[i].field_0x18 = 0;
                    ring[i].field_0x1c = 0;
                }
                for (i = 0; i < 16; i++) {
                    ring[i * 2].field_0x00 = circle[i];
                    ring[i * 2 + 1].field_0x00 = circle[i + 1];
                }
                static UnknownSoultreeStaticVector viewDirection;  // 0x0057ef38 (guard bit 2)
                static UnknownSoultreeStaticVector viewPosition;   // 0x0057eee8 (guard bit 4)
                static int updateView = 1;                         // 0x00568938
                static int drawSphere = 1;                         // 0x0056893c
                if (drawSphere) {
                    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
                    ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(2, 0x1e2, (int)ring, 32, 0);
                }
                if (updateView) {
                    (Vector3&)viewDirection = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->field_0x17c;
                    (Vector3&)viewPosition = ((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->field_0x170;
                }
                UnknownSoultreeVertex lines[4];
                for (i = 0; i < 4; i++) {
                    lines[i].field_0x10 = 0xffff0000;
                    lines[i].field_0x14 = 0xff000000;
                    lines[i].field_0x18 = 0;
                    lines[i].field_0x1c = 0;
                }
                for (int j = 0; j < surface->field_0x00; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->field_0x04[k];
                    count = group->field_0x04;
                    if (!count)
                        continue;
                    group->field_0x00->UnknownFunction4fb4f0();
                    Matrix4* world = &group->field_0x00->field_0x0f8;
                    for (int v = 0; v < count; v++) {
                        Vector3 position = group->field_0x08[v].field_0x00;
                        Vector3 segment[2];
                        segment[0] = position;
                        Vector3 direction = Normalized(position - viewPosition);
                        Vector3 normal = group->field_0x10[v];
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
                        group->field_0x08[v].field_0x1c = hit.y * 0.01f;
                        if (group->field_0x08[v].field_0x1c > 1.0f)
                            group->field_0x08[v].field_0x1c = 1.0f;
                        group->field_0x08[v].field_0x1c = 1.0f - group->field_0x08[v].field_0x1c;
                        Vector3 around = hit - center;
                        around.y = 0.0f;
                        around = Normalized(around);
                        float angle = (float)acos(around.z);
                        if (around.x < 0.0f)
                            angle = -angle;
                        group->field_0x08[v].field_0x18 = angle * 0.15915494f;
                        group->field_0x08[v].field_0x10 = ((long)(fresnel * 255.0f) << 24) | 0xffffff;
                        for (i = 0; i < 2; i++) {
                            lines[i * 2].field_0x00 = segment[i];
                            lines[i * 2 + 1].field_0x00 = segment[i + 1];
                        }
                        if (drawSphere) {
                            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
                            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(2, 0x1e2, (int)lines, 2, 0);
                        }
                    }
                }
                restore = 1;
                break;
            }
            case 6: {
                LightManager* lights = field_0x1bc;
                if (!lights)
                    break;
                LightEmitter* light = FindLight(lights);
                if (!light)
                    break;
                static float exponent = 20.0f;             // 0x00568940
                for (int j = 0; j < surface->field_0x00; j++) {
                    UnknownSoultreeFaceGroup* group = &surface->field_0x04[j];
                    Vector3 lightPosition = ((UnknownSoultreeNodeView*)group->field_0x00)->UnknownFunction4fd710(&light->field_0x64);
                    Vector3 eye = ((UnknownSoultreeNodeView*)group->field_0x00)->UnknownFunction4fd710(
                        &((UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08)->field_0x170);
                    for (int v = 0; v < group->field_0x04; v++) {
                        Vector3 position = group->field_0x08[v].field_0x00;
                        Vector3 toLight = Normalized(position - lightPosition);
                        Vector3 toEye = Normalized(eye - position);
                        Vector3& normal = group->field_0x10[v];
                        float d = (toLight.x * normal.x + toLight.y * normal.y + toLight.z * normal.z) * 2.0f;
                        Vector3 reflected = toLight - normal * d;
                        float specular = toEye.x * reflected.x + toEye.y * reflected.y + toEye.z * reflected.z;
                        if (specular < 0.0f)
                            specular = 0.0f;
                        group->field_0x08[v].field_0x10 = ((long)(pow(specular, exponent) * 255.0) << 24) | 0xffffff;
                    }
                }
                modified = 1;
                restore = 0;
                break;
            }
            }
            UnknownSoultreeSurface* drawn;
            if (field_0x264 > 0) {
                for (int i = 0; i < field_0x264; i++)
                    ((D3DIMSoultreeModifier*)field_0x268[i])->UnknownVirtualSlot27(
                        this, (UnknownSoultreeMesh*)surface, (UnknownSoultreeMesh**)&drawn);
            } else {
                drawn = surface;
            }
            if (drawn->field_0x08) {
                if (material->field_0x70) {
                    ManagedTexture* texture = (ManagedTexture*)material->field_0x70->texture;
                    if (texture->field_0x68 & 1)
                        texture->UnknownFunction510910(&drawn->field_0x2c, (float*)&drawn->field_0x30,
                                                       (float*)&drawn->field_0x34, &drawn->field_0x10->field_0x18,
                                                       &drawn->field_0x10->field_0x1c, drawn->field_0x08, 0x20);
                }
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot15(4, 0x1e2, (int)drawn->field_0x10, drawn->field_0x08,
                                                                  (int)drawn->field_0x1c, drawn->field_0x0c * 3, 0);
                if (field_0x1b0)
                    UnknownFunction442f70(drawn);
                if (field_0x1a8)
                    UnknownFunction442fe0(drawn);
                if (field_0x1ac)
                    UnknownFunction4431f0(drawn);
            }
            if (modified && restore) {
                for (k = 0; k < count; k++) {
                    surface->field_0x10[k].field_0x18 = surface->field_0x28[k].field_0x00;
                    surface->field_0x10[k].field_0x1c = surface->field_0x28[k].field_0x04;
                }
                surface->field_0x2c = 1.0f;
                surface->field_0x30 = 0;
                surface->field_0x34 = 0;
            }
            material->UnknownFunction4ff410();
        }
    }
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(4, savedCull, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(9, savedShade, 0);
}

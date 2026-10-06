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
// declaration order and keeps the vertex array at esp+0x5c; a Vector3 array
// goes through temporaries, a POD array is grouped zeros first.
//
// UnknownFunction443740 (0x00443740, 640 bytes): the box outline. The corner
// table is built from negated extents kept on the FPU stack (fst chains)
// and integer copies; the Vector3 form here evaluates each corner apart.
//
// SoultreeVirtualSlot5 (0x00444140, 768 bytes): the node bounds. Retail
// keeps all six bounds in memory and `this`/the model in ebp/ebx; VC6 keeps
// low.z on the FPU stack and allocates differently.
//
// UnknownFunction443de0 (0x00443de0, 592 bytes): 44/579, the level-of-detail
// and mip chooser. Retail keeps `center` in ebx and `extents` in ebp, takes
// the camera position as a pointer (add esi, 0x170) and sums the squares as
// y*y + x*x + z*z for the difference vector; VC6 keeps `center` in eax and
// reverses the sum here. `*(float*)&field_0xcc`: SoultreeMaterial.h types
// +0xcc as int, retail multiplies it as a float.
//
// SoultreeVirtualSlot7 (0x00444560, 1008 bytes): 680/1233 with the source
// in ebp instead of ebx and the two node lists in swapped frame slots
// (retail frame 0x2c, ours 0x30).
//
// Not attempted: 0x00440810 (1328 bytes), the drawing function
// 0x00440f30..0x00442dd8 (about 7.8 KB with a jump table at 0x00442ddc and
// a function-local static vector at 0x0057eef8, guard 0x0057ef24), slot 12
// 0x00443aa0 (inline matrix product, VisibilityClipper 0x0052f570) and its
// the three `ret` stubs
// 0x00442f40/0x00442f50/0x00442f60.

#include "../../src/reconstructed/D3DIMSoulTree.h"
#include "../../src/reconstructed/D3DIMSoultreeModifier.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/LightEmitter.h"
#include "../../src/reconstructed/ManagedTexture.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/ResourceManager.h"
#include "../../src/reconstructed/SoultreeMaterial.h"
#include "../../src/reconstructed/TextureMap.h"

#include <stdio.h>
#include <string.h>

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
    points[0] = Vector3(0.0f, 0.0f, 0.0f);
    points[1] = Vector3(0.25f, 0.0f, 0.0f);
    points[2] = Vector3(0.0f, 0.25f, 0.0f);
    points[3] = Vector3(0.0f, 0.0f, 0.25f);
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
                UnknownSoultreeFaceGroup* group = &model->field_0x28c[lod].field_0x04[i].field_0x04[j];
                if (group->field_0x00 == this) {
                    UnknownSoultreeVertex* vertex = (UnknownSoultreeVertex*)group->field_0x0c;
                    for (int k = 0; k < group->field_0x04; k++) {
                        if (vertex[k].field_0x00.x > high.x)
                            high.x = vertex[k].field_0x00.x;
                        if (vertex[k].field_0x00.y > high.y)
                            high.y = vertex[k].field_0x00.y;
                        if (vertex[k].field_0x00.z > high.z)
                            high.z = vertex[k].field_0x00.z;
                        if (vertex[k].field_0x00.x < low.x)
                            low.x = vertex[k].field_0x00.x;
                        if (vertex[k].field_0x00.y < low.y)
                            low.y = vertex[k].field_0x00.y;
                        if (vertex[k].field_0x00.z < low.z)
                            low.z = vertex[k].field_0x00.z;
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
    field_0x158 = (high + low) * 0.5f;
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
    UnknownSoultreeCameraView* camera = (UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08;
    float distance = Magnitude(UnknownFunction4fd660(*center) - camera->field_0x170);
    float range = distance - Magnitude(*extents);
    if (range < 0.1f)
        range = 0.1f;
    float scale = camera->field_0x198 / range;
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


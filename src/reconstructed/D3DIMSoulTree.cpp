// D3DIMSoulTree.cpp -- see D3DIMSoulTree.h for the evidence.

#include "D3DIMSoulTree.h"
#include "D3DIMSoultreeModifier.h"
#include "Parameterblocks.h"
#include "PCRenderTarget.h"
#include "DebugAlloc.h"
#include "ResourceManager.h"
#include "SoultreeMaterial.h"
#include "TextureMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 0x0043f160
D3DIMSoultreeObject::D3DIMSoultreeObject(int flags)
    : SoultreeObject(flags)
{
    field_0x1a4 = 0;
    field_0x1a8 = 0;
    field_0x1ac = 0;
    field_0x1b4 = 1;
    field_0x1b8 = 1;
    field_0x240 = 0;
    field_0x244 = 0x22b;
    field_0x290 = 0;
    field_0x294 = 0;
    field_0x1c0[0] = 0;
    field_0x274 = 0;
    field_0x27c = 0;
    field_0x278 = 0;
    field_0x28c = 0;
    field_0x280 = 0;
    field_0x284 = 1.0f;
    field_0x288 = 0;
    field_0x1bc = 0;
    field_0x268 = 0;
    field_0x264 = 0;
    field_0x270 = 0;
    field_0x26c = 0;
    field_0x298.field_0x00 = 0;
    field_0x298.field_0x04 = 0;
    field_0x298.field_0x08 = 0;
    field_0x298.field_0x0c = 0;
    field_0x2c8 = 0;
    // Retail fills the temporary z first (a plain Vector3(0, 0, 0) stores
    // it x first).
    Vector3 zero;
    zero.z = 0.0f;
    zero.y = 0.0f;
    zero.x = 0.0f;
    field_0x2bc = zero;
    field_0x2d0 = 0;
    field_0x154 = 0;
    field_0x1b0 = 0;
    field_0x260 = 0;
    field_0x2d4 = 1;
}

// 0x0043f2b0
D3DIMSoultreeObject::~D3DIMSoultreeObject()
{
    if (field_0x290)
        DebugFree(field_0x290, __FILE__, 0x5d);
    if (field_0x280)
        DebugFree(field_0x280, __FILE__, 0x60);
    if (field_0x28c) {
        for (int i = 0; i < field_0x274; i++) {
            for (int j = 0; j < field_0x28c[i].field_0x00; j++) {
                DebugFree(field_0x28c[i].field_0x04[j].field_0x1c, __FILE__, 0x67);
                DebugFree(field_0x28c[i].field_0x04[j].field_0x04, __FILE__, 0x68);
                DebugFree(field_0x28c[i].field_0x04[j].field_0x10, __FILE__, 0x69);
                DebugFree(field_0x28c[i].field_0x04[j].field_0x14, __FILE__, 0x6a);
                DebugFree(field_0x28c[i].field_0x04[j].field_0x18, __FILE__, 0x6b);
                DebugFree(field_0x28c[i].field_0x04[j].field_0x28, __FILE__, 0x6c);
                DebugFree(field_0x28c[i].field_0x04[j].field_0x24, __FILE__, 0x6d);
            }
            DebugFree(field_0x28c[i].field_0x04, __FILE__, 0x70);
        }
        DebugFree(field_0x28c, __FILE__, 0x72);
    }
    UnknownFunction444e80();
    UnknownFunction444fb0();
    void* item = g_UnknownResourceManager572b44->UnknownFunction4e93f0(this);
    if (item)
        g_UnknownResourceManager572b44->UnknownFunction4e9010(item, 0);
}

// 0x0043f4b0
GameObject* D3DIMSoultreeObject::UnknownVirtualSlot9(void* owner, const char* path, int a, int b, int c)
{
    GameObject::UnknownVirtualSlot8(owner);
    if (c) {
        memcpy(field_0x248, (const int*)b, sizeof(field_0x248));
        field_0x240 = *(TextureMapManager**)b;
    }
    field_0x2cc = c;
    field_0x1bc = (LightManager*)a;
    if (*path) {
        char drive[_MAX_DRIVE + 1];
        char dir[_MAX_DIR];
        char fname[_MAX_FNAME];
        char ext[_MAX_EXT];
        char name[0x104];
        char fullPath[0x104];
        _splitpath(path, drive, dir, fname, ext);
        int fnameLength = strlen(fname);
        int length = fnameLength > 0x103 ? 0x103 : fnameLength;
        strncpy(name, fname, length);
        name[length] = 0;
        strcat(name, ".slb");
        int driveLength = strlen(drive);
        length = driveLength > 0x103 ? 0x103 : driveLength;
        strncpy(fullPath, drive, length);
        fullPath[length] = 0;
        strcat(fullPath, dir);
        strcat(fullPath, fname);
        strcat(fullPath, ".slb");
        ResourceItem* item = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
        if (!item) {
            g_UnknownResourceManager572b44->UnknownFunction4e9430(name, fullPath);
            item = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
        }
        if (item) {
            if (item->field_0x10) {
                item->AddRef();
                SoultreeVirtualSlot7((SoultreeObject*)item->field_0x10);
                UnknownFunction4fedb0();
                return this;
            }
            ((UnknownTextureStream*)item->field_0x14)->UnknownFunction461340(item->field_0x18, 0, 0);
            SoultreeVirtualSlot2((UnknownTextureStream*)item->field_0x14);
        } else {
            fnameLength = strlen(fname);
            length = fnameLength > 0x103 ? 0x103 : fnameLength;
            strncpy(name, fname, length);
            name[length] = 0;
            strcat(name, ".slt");
            driveLength = strlen(drive);
            length = driveLength > 0x103 ? 0x103 : driveLength;
            strncpy(fullPath, drive, length);
            fullPath[length] = 0;
            strcat(fullPath, dir);
            strcat(fullPath, fname);
            strcat(fullPath, ".slt");
            item = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
            if (!item) {
                g_UnknownResourceManager572b44->UnknownFunction4e9430(name, fullPath);
                item = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
                if (!item) {
                    UnknownFunction4fedb0();
                    return this;
                }
            }
            if (item->field_0x10) {
                item->AddRef();
                SoultreeVirtualSlot7((SoultreeObject*)item->field_0x10);
                UnknownFunction4fedb0();
                return this;
            }
            UnknownTextureStream* stream = new (__FILE__, 0xe1) UnknownTextureStream((int)g_UnknownResourceManager572b44);
            stream->UnknownFunction460f50(name, "rb", 0);
            UnknownFunction4fdb60(stream, 0);
            delete stream;
        }
        if (item)
            g_UnknownResourceManager572b44->UnknownFunction4e9010(item, this);
        UnknownFunction4fedb0();
    }
    return this;
}

// 0x0043f950
void D3DIMSoultreeObject::SoultreeVirtualSlot2(UnknownTextureStream* stream)
{
    SoultreeObject::SoultreeVirtualSlot2(stream);
    int count = UnknownFunction4fda30();
    SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x14e);
    int index = 1;
    nodes[0] = this;
    UnknownFunction4fda60(&index, nodes);
    stream->UnknownFunction461640(&field_0x294, 4, 1);
    if (field_0x294 > 0) {
        field_0x290 = (SoultreeMaterial**)DebugMalloc(field_0x294 * 4, __FILE__, 0x156);
        for (int i = 0; i < field_0x294; i++) {
            field_0x290[i] = new (__FILE__, 0x158) SoultreeMaterial(1);
            field_0x290[i]->UnknownFunction4ff0b0((RenderTarget*)field_0x18, (const SoultreeTextureOptions*)field_0x248,
                                                  field_0x240, field_0x244);
            UnknownFunction469190(field_0x290[i], -1);
            field_0x290[i]->UnknownFunction4ff450((UnknownParameterStream*)stream);
        }
    } else {
        field_0x290 = 0;
    }
    stream->UnknownFunction461640(&field_0x274, 4, 1);
    stream->UnknownFunction461640(&field_0x288, 4, 1);
    if (field_0x288) {
        field_0x280 = (float*)DebugMalloc(field_0x274 * 4, __FILE__, 0x166);
        for (int i = 0; i < field_0x274 - 1; i++)
            stream->UnknownFunction461640(&field_0x280[i], 4, 1);
    }
    if (field_0x274 > 0) {
        field_0x28c = (UnknownSoultreeLod*)DebugMalloc(field_0x274 * 8, __FILE__, 0x16f);
        for (int lod = 0; lod < field_0x274; lod++) {
            stream->UnknownFunction461640(&field_0x28c[lod].field_0x00, 4, 1);
            field_0x28c[lod].field_0x04 =
                (UnknownSoultreeSurface*)DebugMalloc(field_0x28c[lod].field_0x00 * 0x38, __FILE__, 0x173);
            for (int i = 0; i < field_0x28c[lod].field_0x00; i++) {
                UnknownSoultreeSurface* surface = &field_0x28c[lod].field_0x04[i];
                stream->UnknownFunction461640(&surface->field_0x00, 4, 1);
                stream->UnknownFunction461640(&surface->field_0x08, 4, 1);
                stream->UnknownFunction461640(&surface->field_0x0c, 4, 1);
                stream->UnknownFunction461640(&surface->field_0x20, 4, 1);
                surface->field_0x30 = 0;
                surface->field_0x34 = 0;
                surface->field_0x2c = 1.0f;
                surface->field_0x10 = (UnknownSoultreeVertex*)DebugMalloc(surface->field_0x08 * 0x20, __FILE__, 0x181);
                stream->UnknownFunction461640(surface->field_0x10, 0x20, surface->field_0x08);
                surface->field_0x14 = (UnknownSoultreeVertex*)DebugMalloc(surface->field_0x08 * 0x20, __FILE__, 0x184);
                stream->UnknownFunction461640(surface->field_0x14, 0x20, surface->field_0x08);
                surface->field_0x18 = (Vector3*)DebugMalloc(surface->field_0x08 * 0xc, __FILE__, 0x187);
                stream->UnknownFunction461640(surface->field_0x18, 0xc, surface->field_0x08);
                surface->field_0x1c = (unsigned short*)DebugMalloc(surface->field_0x0c * 6, __FILE__, 0x18a);
                stream->UnknownFunction461640(surface->field_0x1c, 6, surface->field_0x0c);
                surface->field_0x28 = (UnknownSoultreeUV*)DebugMalloc(surface->field_0x08 * 8, __FILE__, 0x18d);
                stream->UnknownFunction461640(surface->field_0x28, 4, surface->field_0x08 * 2);
                surface->field_0x24 = (int*)DebugMalloc(surface->field_0x20 * 4, __FILE__, 0x191);
                stream->UnknownFunction461640(surface->field_0x24, 4, surface->field_0x20);
                surface->field_0x04 = (UnknownSoultreeFaceGroup*)DebugMalloc(surface->field_0x00 * 0x14, __FILE__, 0x194);
                stream->UnknownFunction461640(surface->field_0x04, 0x14, surface->field_0x00);
                for (int j = 0; j < surface->field_0x00; j++) {
                    int node;
                    int first;
                    stream->UnknownFunction461640(&node, 4, 1);
                    stream->UnknownFunction461640(&first, 4, 1);
                    surface->field_0x04[j].field_0x00 = nodes[node];
                    surface->field_0x04[j].field_0x08 = &surface->field_0x10[first];
                    surface->field_0x04[j].field_0x0c = &surface->field_0x14[first];
                    surface->field_0x04[j].field_0x10 = &surface->field_0x18[first];
                }
            }
        }
    } else {
        field_0x28c = 0;
    }
    DebugFree(nodes, __FILE__, 0x1a7);
    UnknownFunction444440();
}

// 0x0043fe40
void D3DIMSoultreeObject::SoultreeVirtualSlot3()
{
    field_0x1a0->UnknownFunction4b78f0("Materials");
    field_0x1a0->UnknownFunction4b7f10("NumberOfMaterials", 0, &field_0x294);
    SoultreeObject::SoultreeVirtualSlot3();
    int untextured = UnknownFunction440060();
    int count = field_0x294;
    if (untextured)
        field_0x294 = count + 1;
    field_0x290 = (SoultreeMaterial**)DebugMalloc(field_0x294 * 4, __FILE__, 0x1c0);
    for (int i = 0; i < count; i++) {
        field_0x290[i] = new (__FILE__, 0x1c5) SoultreeMaterial(1);
        field_0x290[i]->UnknownFunction4ff0b0((RenderTarget*)field_0x18, (const SoultreeTextureOptions*)field_0x248,
                                              field_0x240, field_0x244);
        UnknownFunction469190(field_0x290[i], -1);
        char section[0x80];
        sprintf(section, "Material - %d", i);
        field_0x1a0->UnknownFunction4b78f0(section);
        field_0x290[i]->UnknownFunction4ff9e0(field_0x1a0, field_0x2cc);
    }
    if (untextured) {
        field_0x290[count] = new (__FILE__, 0x1ce) SoultreeMaterial(1);
        field_0x290[count]->UnknownFunction4ff0b0((RenderTarget*)field_0x18, (const SoultreeTextureOptions*)field_0x248,
                                                  field_0x240, field_0x244);
        UnknownFunction469190(field_0x290[count], -1);
        field_0x290[count]->UnknownFunction5000b0();
    }
    UnknownFunction440810();
}

// The four vector constants that open about 73 retail files (see
// src/krusty2/math/Math3D.h). Here they sit mid-file: 0x0057ef08,
// 0x0057ef18, 0x0057ef28 and 0x0057eed8, initialised by
// 0x00442e00..0x00442f3b (.CRT$XCU entries 82-85), after the drawing code
// and before the debug helpers.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x00442f70
void D3DIMSoultreeObject::UnknownFunction442f70(UnknownSoultreeSurface* surface)
{
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(8, 2, 1);
    for (int i = 0; i < surface->field_0x08; i++)
        surface->field_0x10[i].field_0x10 = 0;
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot15(4, 0x1e2, (int)surface->field_0x10, surface->field_0x08,
                                                (int)surface->field_0x1c, surface->field_0x0c * 3, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(8, 3, 1);
}

// 0x004433f0
void D3DIMSoultreeObject::UnknownFunction4433f0(int surface, UnknownSoultreeVertex** vertices,
                                                int* vertexCount, unsigned short** indices, int* indexCount,
                                                int a6, int lod)
{
    if (lod == -1)
        lod = field_0x27c;
    if (field_0x18c || lod != field_0x27c)
        UnknownFunction440d40(lod);
    *vertices = field_0x28c[lod].field_0x04[surface].field_0x10;
    *vertexCount = field_0x28c[lod].field_0x04[surface].field_0x08;
    *indexCount = field_0x28c[lod].field_0x04[surface].field_0x0c * 3;
    *indices = field_0x28c[lod].field_0x04[surface].field_0x1c;
}

// 0x00443490
int D3DIMSoultreeObject::UnknownVirtualSlot10(float frameTime)
{
    field_0x2b8 = frameTime;
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x004434b0
void D3DIMSoultreeObject::UnknownFunction4434b0()
{
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 0, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
    Vector3 center;
    Vector3 extents;
    if (!field_0x13c) {
        UnknownFunction4fe850(&center, &extents);
        UnknownFunction443740(center, extents, 1);
    }
    UnknownFunction4fe0a0(&center, &extents);
    UnknownFunction443740(center, extents, 0);
    if (field_0x144)
        ((D3DIMSoultreeObject*)field_0x144)->UnknownFunction4434b0();
    if (field_0x140)
        ((D3DIMSoultreeObject*)field_0x140)->UnknownFunction4434b0();
}

// 0x004439c0
void D3DIMSoultreeObject::UnknownFunction4439c0(UnknownSoultreeCounters* rect)
{
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    ((PCRenderTarget*)field_0x18)->field_0x48->UnknownMethod25(0, &desc, 1, 0);
    UnknownSoultreeCameraView* camera = (UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08;
    char* bits = (char*)desc.surface + camera->field_0x1a4 * desc.pitch + camera->field_0x1a0;
    int i;
    for (i = rect->field_0x00; i < rect->field_0x08; i++) {
        bits[rect->field_0x04 * desc.pitch + i * 2] = 0;
        bits[rect->field_0x0c * desc.pitch + i * 2] = 0;
    }
    for (i = rect->field_0x04; i < rect->field_0x0c; i++) {
        bits[i * desc.pitch + rect->field_0x08 * 2] = 0;
        bits[i * desc.pitch + rect->field_0x00 * 2] = 0;
    }
    ((PCRenderTarget*)field_0x18)->field_0x48->UnknownMethod32(0);
}

// 0x00444030
int D3DIMSoultreeObject::UnknownVirtualSlot14()
{
    if (!field_0x2d4)
        return 1;
    UnknownFunction440f30();
    if (field_0x2d0) {
        UnknownFunction4439c0(&field_0x298);
        UnknownFunction4434b0();
        UnknownFunction4435b0();
    }
    GameObject::UnknownVirtualSlot14();
    field_0x298.field_0x08++;
    field_0x298.field_0x0c++;
    return 1;
}

// 0x004440a0
void D3DIMSoultreeObject::SoultreeVirtualSlot4(SoultreeObject** out)
{
    D3DIMSoultreeObject* copy = new (__FILE__, 0x7d5) D3DIMSoultreeObject(field_0x25_bit0);
    *out = copy;
    copy->UnknownVirtualSlot9(field_0x18, "", (int)field_0x1bc, (int)field_0x248, 1);
}

// 0x00444440
void D3DIMSoultreeObject::UnknownFunction444440()
{
    int count = UnknownFunction4fda30();
    SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x80b);
    int index = 1;
    nodes[0] = this;
    UnknownFunction4fda60(&index, nodes);
    for (int i = 0; i < count; i++)
        nodes[i]->SoultreeVirtualSlot5();
    DebugFree(nodes, __FILE__, 0x812);
    UnknownFunction4fe0f0();
}

// 0x004444c0
void D3DIMSoultreeObject::UnknownFunction4444c0(int value)
{
    field_0x18c = 1;
    field_0x1a4 = value;
}

// 0x004444e0
void D3DIMSoultreeObject::UnknownFunction4444e0()
{
    field_0x298.field_0x00 = 0;
    field_0x298.field_0x08 = 0;
    field_0x298.field_0x04 = 0;
    field_0x298.field_0x0c = 0;
    field_0x2a8.field_0x00 = 0;
    field_0x2a8.field_0x08 = 0;
    field_0x2a8.field_0x04 = 0;
    field_0x2a8.field_0x0c = 0;
    UnknownFunction4fdb40();
}

// 0x00444520
void D3DIMSoultreeObject::UnknownVirtualSlot5()
{
    GameObject::UnknownVirtualSlot5();
    UnknownFunction4fdb50();
}

// 0x00444540
void D3DIMSoultreeObject::UnknownVirtualSlot4()
{
    GameObject::UnknownVirtualSlot4();
    UnknownFunction4444e0();
}

// 0x00444a40
void D3DIMSoultreeObject::UnknownFunction444a40(D3DIMSoultreeObject* source)
{
    field_0x294 = source->field_0x294;
    field_0x290 = (SoultreeMaterial**)DebugMalloc(field_0x294 * 4, __FILE__, 0x8ed);
    for (int i = 0; i < field_0x294; i++) {
        field_0x290[i] = new (__FILE__, 0x8ef) SoultreeMaterial(1);
        field_0x290[i]->UnknownFunction4ff0b0((RenderTarget*)field_0x18, (const SoultreeTextureOptions*)field_0x248,
                                              field_0x240, field_0x244);
        UnknownFunction469190(field_0x290[i], -1);
        field_0x290[i]->UnknownFunction5000f0(source->field_0x290[i]);
    }
}

// 0x00444b60
void D3DIMSoultreeObject::SoultreeVirtualSlot8(SoultreeObject* sourceNode, SoultreeObject* targetNode)
{
    SoultreeObject::SoultreeVirtualSlot8(sourceNode, targetNode);
    D3DIMSoultreeObject* source = (D3DIMSoultreeObject*)sourceNode;
    D3DIMSoultreeObject* target = (D3DIMSoultreeObject*)targetNode;
    target->field_0x18c = 1;
    target->field_0x1a4 = source->field_0x1a4;
    target->field_0x1a8 = source->field_0x1a8;
    target->field_0x1ac = source->field_0x1ac;
    target->field_0x1bc = source->field_0x1bc;
    int nameLength = strlen(source->field_0x1c0);
    int length = nameLength > 0x7f ? 0x7f : nameLength;
    strncpy(target->field_0x1c0, source->field_0x1c0, length);
    target->field_0x1c0[length] = 0;
    target->field_0x1bc = source->field_0x1bc;
    target->field_0x240 = source->field_0x240;
    target->field_0x244 = source->field_0x244;
    memcpy(target->field_0x248, source->field_0x248, sizeof(target->field_0x248));
    target->field_0x298 = source->field_0x298;
    target->field_0x2a8 = source->field_0x2a8;
}

// 0x00444c70
int D3DIMSoultreeObject::UnknownFunction444c70(int index, const char* name, int a)
{
    if (index > field_0x294)
        return 0;
    if (!field_0x290[index]->field_0x9c)
        return 0;
    int nameLength = strlen(name);
    int length = nameLength > 0x3f ? 0x3f : nameLength;
    strncpy(field_0x290[index]->field_0x2c, name, length);
    field_0x290[index]->field_0x2c[length] = 0;
    field_0x290[index]->UnknownFunction4ff620();
    return 1;
}

// 0x00444d00
void D3DIMSoultreeObject::UnknownFunction444d00(int lod)
{
    if (lod < field_0x278)
        lod = field_0x278;
    if (lod >= field_0x274)
        lod = field_0x274 - 1;
    if (field_0x27c != lod) {
        field_0x18c = 1;
        field_0x27c = lod;
    }
}

// 0x00444d40
void D3DIMSoultreeObject::UnknownFunction444d40(int lod)
{
    if (lod < 0)
        lod = 0;
    if (lod >= field_0x274)
        lod = field_0x274 - 1;
    field_0x278 = lod;
}

// 0x00444d60
void D3DIMSoultreeObject::UnknownFunction444d60(float value)
{
    field_0x284 = 1.0f / value;
}

// 0x00444d80
void D3DIMSoultreeObject::UnknownFunction444d80(GameObject* modifier)
{
    field_0x268 = (GameObject**)DebugRealloc(field_0x268, field_0x264 * 4 + 4, __FILE__, 0x958);
    field_0x268[field_0x264] = modifier;
    field_0x264++;
    ((D3DIMSoultreeModifier*)modifier)->UnknownFunction4452f0(this);
}

// 0x00444de0
void D3DIMSoultreeObject::UnknownFunction444de0(GameObject* modifier)
{
    int found = 0;
    int i;
    for (i = 0; i < field_0x264; i++) {
        if (field_0x268[i] == modifier)
            found = i;
    }
    for (i = found; i < field_0x264 - 1; i++)
        field_0x268[i] = field_0x268[i + 1];
    field_0x268 = (GameObject**)DebugRealloc(field_0x268, field_0x264 * 4 - 4, __FILE__, 0x96d);
    field_0x264--;
    ((D3DIMSoultreeModifier*)modifier)->UnknownFunction445360(this);
}

// 0x00444e80
void D3DIMSoultreeObject::UnknownFunction444e80()
{
    while (field_0x264)
        UnknownFunction444de0(field_0x268[0]);
}

// 0x00444eb0
void D3DIMSoultreeObject::UnknownFunction444eb0(GameObject* modifier)
{
    field_0x270 = (GameObject**)DebugRealloc(field_0x270, field_0x26c * 4 + 4, __FILE__, 0x97f);
    field_0x270[field_0x26c] = modifier;
    field_0x26c++;
    ((D3DIMSoultreeModifier*)modifier)->UnknownFunction4452f0(this);
}

// 0x00444f10
void D3DIMSoultreeObject::UnknownFunction444f10(GameObject* modifier)
{
    int found = 0;
    int i;
    for (i = 0; i < field_0x26c; i++) {
        if (field_0x270[i] == modifier)
            found = i;
    }
    for (i = found; i < field_0x26c - 1; i++)
        field_0x270[i] = field_0x270[i + 1];
    field_0x270 = (GameObject**)DebugRealloc(field_0x270, field_0x26c * 4 - 4, __FILE__, 0x994);
    field_0x26c--;
    ((D3DIMSoultreeModifier*)modifier)->UnknownFunction445360(this);
}

// 0x00444fb0
void D3DIMSoultreeObject::UnknownFunction444fb0()
{
    while (field_0x26c)
        UnknownFunction444f10(field_0x270[0]);
}

// 0x00444fe0
int D3DIMSoultreeObject::UnknownFunction444fe0()
{
    int most = 0;
    if (field_0x28c) {
        for (int i = 0; i < field_0x274; i++) {
            int total = 0;
            for (int j = 0; j < field_0x28c[i].field_0x00; j++)
                total += field_0x28c[i].field_0x04[j].field_0x08;
            if (total > most)
                most = total;
        }
    }
    return most;
}

// 0x00445030
int D3DIMSoultreeObject::UnknownFunction445030(int lod)
{
    if (lod == -1)
        lod = field_0x27c;
    if (field_0x28c)
        return field_0x28c[lod].field_0x00;
    return 0;
}

// 0x00445060
int D3DIMSoultreeObject::UnknownFunction445060(SoultreeObject* node, int lod)
{
    for (int i = 0; i < field_0x28c[lod].field_0x00; i++) {
        UnknownSoultreeSurface* surface = &field_0x28c[lod].field_0x04[i];
        for (int j = 0; j < surface->field_0x00; j++) {
            if (surface->field_0x04[j].field_0x00 == node && surface->field_0x04[j].field_0x04 > 0)
                return 1;
        }
    }
    return 0;
}

// 0x004450c0
void D3DIMSoultreeObject::SoultreeVirtualSlot6()
{
    int saved = field_0x27c;
    for (int lod = 0; lod < field_0x274; lod++) {
        field_0x27c = lod;
        UnknownFunction440d40(-1);
        for (int i = 0; i < field_0x28c[lod].field_0x00; i++) {
            for (int j = 0; j < field_0x28c[lod].field_0x04[i].field_0x08; j++) {
                field_0x28c[lod].field_0x04[i].field_0x14[j].field_0x00.x =
                    field_0x28c[lod].field_0x04[i].field_0x10[j].field_0x00.x;
                field_0x28c[lod].field_0x04[i].field_0x14[j].field_0x00.y =
                    field_0x28c[lod].field_0x04[i].field_0x10[j].field_0x00.y;
                field_0x28c[lod].field_0x04[i].field_0x14[j].field_0x00.z =
                    field_0x28c[lod].field_0x04[i].field_0x10[j].field_0x00.z;
            }
        }
    }
    field_0x27c = saved;
    SoultreeObject::SoultreeVirtualSlot6();
    if (!field_0x13c)
        UnknownFunction444440();
}

// 0x004451e0
void D3DIMSoultreeObject::UnknownFunction4451e0(int index)
{
    UnknownFunction444d60(g_UnknownSoultreeLodSettings689f18[index].field_0x00);
    field_0x1b4 = g_UnknownSoultreeLodSettings689f18[index].field_0x04;
    field_0x1b8 = g_UnknownSoultreeLodSettings689f18[index].field_0x08;
    UnknownFunction4444c0(g_UnknownSoultreeLodSettings689f18[index].field_0x0c);
}

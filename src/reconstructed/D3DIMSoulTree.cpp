// D3DIMSoulTree.cpp -- see D3DIMSoulTree.h for the evidence.

#include "D3DIMSoulTree.h"
#include "D3DIMSoultreeModifier.h"
#include "Parameterblocks.h"
#include "PCRenderTarget.h"
#include "DebugAlloc.h"
#include "ResourceManager.h"
#include "SoultreeMaterial.h"
#include "TextureMap.h"
#include "D3DConstants.h"

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
    textureManager = 0;
    textureFormat = 555;
    materialTable = 0;
    materialCount = 0;
    modelName[0] = 0;
    lodCount = 0;
    field_0x27c = 0;
    lowestLod = 0;
    lodTable = 0;
    autoLodDistances = 0;
    field_0x284 = 1.0f;
    field_0x288 = 0;
    lightManager = 0;
    modifierList = 0;
    modifierCount = 0;
    secondModifierList = 0;
    secondModifierCount = 0;
    field_0x298.field_0x00 = 0;
    field_0x298.field_0x04 = 0;
    field_0x298.field_0x08 = 0;
    field_0x298.field_0x0c = 0;
    textureScroll = 0;
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
    if (materialTable)
        DebugFree(materialTable, __FILE__, 0x5d);
    if (autoLodDistances)
        DebugFree(autoLodDistances, __FILE__, 0x60);
    if (lodTable) {
        for (int i = 0; i < lodCount; i++) {
            for (int j = 0; j < lodTable[i].surfaceCount; j++) {
                DebugFree(lodTable[i].surfaces[j].indices, __FILE__, 0x67);
                DebugFree(lodTable[i].surfaces[j].groups, __FILE__, 0x68);
                DebugFree(lodTable[i].surfaces[j].vertices, __FILE__, 0x69);
                DebugFree(lodTable[i].surfaces[j].drawnVertices, __FILE__, 0x6a);
                DebugFree(lodTable[i].surfaces[j].normals, __FILE__, 0x6b);
                DebugFree(lodTable[i].surfaces[j].uvs, __FILE__, 0x6c);
                DebugFree(lodTable[i].surfaces[j].materialIndices, __FILE__, 0x6d);
            }
            DebugFree(lodTable[i].surfaces, __FILE__, 0x70);
        }
        DebugFree(lodTable, __FILE__, 0x72);
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
        memcpy(textureOptions, (const int*)b, sizeof(textureOptions));
        textureManager = *(TextureMapManager**)b;
    }
    loadFlag = c;
    lightManager = (LightManager*)a;
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
                RegisterNode();
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
                    RegisterNode();
                    return this;
                }
            }
            if (item->field_0x10) {
                item->AddRef();
                SoultreeVirtualSlot7((SoultreeObject*)item->field_0x10);
                RegisterNode();
                return this;
            }
            UnknownTextureStream* stream = new (__FILE__, 0xe1) UnknownTextureStream((int)g_UnknownResourceManager572b44);
            stream->UnknownFunction460f50(name, "rb", 0);
            LoadFromParameters(stream, 0);
            delete stream;
        }
        if (item)
            g_UnknownResourceManager572b44->UnknownFunction4e9010(item, this);
        RegisterNode();
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
    CollectDescendants(&index, nodes);
    stream->UnknownFunction461640(&materialCount, 4, 1);
    if (materialCount > 0) {
        materialTable = (SoultreeMaterial**)DebugMalloc(materialCount * 4, __FILE__, 0x156);
        for (int i = 0; i < materialCount; i++) {
            materialTable[i] = new (__FILE__, 0x158) SoultreeMaterial(1);
            materialTable[i]->Attach((RenderTarget*)field_0x18, (const SoultreeTextureOptions*)textureOptions,
                                                  textureManager, textureFormat);
            UnknownFunction469190(materialTable[i], -1);
            materialTable[i]->ReadSaved((UnknownParameterStream*)stream);
        }
    } else {
        materialTable = 0;
    }
    stream->UnknownFunction461640(&lodCount, 4, 1);
    stream->UnknownFunction461640(&field_0x288, 4, 1);
    if (field_0x288) {
        autoLodDistances = (float*)DebugMalloc(lodCount * 4, __FILE__, 0x166);
        for (int i = 0; i < lodCount - 1; i++)
            stream->UnknownFunction461640(&autoLodDistances[i], 4, 1);
    }
    if (lodCount > 0) {
        lodTable = (UnknownSoultreeLod*)DebugMalloc(lodCount * 8, __FILE__, 0x16f);
        for (int lod = 0; lod < lodCount; lod++) {
            stream->UnknownFunction461640(&lodTable[lod].surfaceCount, 4, 1);
            lodTable[lod].surfaces =
                (UnknownSoultreeSurface*)DebugMalloc(lodTable[lod].surfaceCount * 0x38, __FILE__, 0x173);
            for (int i = 0; i < lodTable[lod].surfaceCount; i++) {
                UnknownSoultreeSurface* surface = &lodTable[lod].surfaces[i];
                stream->UnknownFunction461640(&surface->groupCount, 4, 1);
                stream->UnknownFunction461640(&surface->vertexCount, 4, 1);
                stream->UnknownFunction461640(&surface->faceCount, 4, 1);
                stream->UnknownFunction461640(&surface->materialCount, 4, 1);
                surface->field_0x30 = 0;
                surface->field_0x34 = 0;
                surface->field_0x2c = 1.0f;
                surface->vertices = (UnknownSoultreeVertex*)DebugMalloc(surface->vertexCount * 0x20, __FILE__, 0x181);
                stream->UnknownFunction461640(surface->vertices, 0x20, surface->vertexCount);
                surface->drawnVertices = (UnknownSoultreeVertex*)DebugMalloc(surface->vertexCount * 0x20, __FILE__, 0x184);
                stream->UnknownFunction461640(surface->drawnVertices, 0x20, surface->vertexCount);
                surface->normals = (Vector3*)DebugMalloc(surface->vertexCount * 0xc, __FILE__, 0x187);
                stream->UnknownFunction461640(surface->normals, 0xc, surface->vertexCount);
                surface->indices = (unsigned short*)DebugMalloc(surface->faceCount * 6, __FILE__, 0x18a);
                stream->UnknownFunction461640(surface->indices, 6, surface->faceCount);
                surface->uvs = (UnknownSoultreeUV*)DebugMalloc(surface->vertexCount * 8, __FILE__, 0x18d);
                stream->UnknownFunction461640(surface->uvs, 4, surface->vertexCount * 2);
                surface->materialIndices = (int*)DebugMalloc(surface->materialCount * 4, __FILE__, 0x191);
                stream->UnknownFunction461640(surface->materialIndices, 4, surface->materialCount);
                surface->groups = (UnknownSoultreeFaceGroup*)DebugMalloc(surface->groupCount * 0x14, __FILE__, 0x194);
                stream->UnknownFunction461640(surface->groups, 0x14, surface->groupCount);
                for (int j = 0; j < surface->groupCount; j++) {
                    int node;
                    int first;
                    stream->UnknownFunction461640(&node, 4, 1);
                    stream->UnknownFunction461640(&first, 4, 1);
                    surface->groups[j].node = nodes[node];
                    surface->groups[j].sourceVertices = &surface->vertices[first];
                    surface->groups[j].transformedVertices = &surface->drawnVertices[first];
                    surface->groups[j].normals = &surface->normals[first];
                }
            }
        }
    } else {
        lodTable = 0;
    }
    DebugFree(nodes, __FILE__, 0x1a7);
    UnknownFunction444440();
}

// 0x0043fe40
void D3DIMSoultreeObject::SoultreeVirtualSlot3()
{
    parameterBlock->UnknownFunction4b78f0("Materials");
    parameterBlock->UnknownFunction4b7f10("NumberOfMaterials", 0, &materialCount);
    SoultreeObject::SoultreeVirtualSlot3();
    int untextured = ReadLods();
    int count = materialCount;
    if (untextured)
        materialCount = count + 1;
    materialTable = (SoultreeMaterial**)DebugMalloc(materialCount * 4, __FILE__, 0x1c0);
    for (int i = 0; i < count; i++) {
        materialTable[i] = new (__FILE__, 0x1c5) SoultreeMaterial(1);
        materialTable[i]->Attach((RenderTarget*)field_0x18, (const SoultreeTextureOptions*)textureOptions,
                                              textureManager, textureFormat);
        UnknownFunction469190(materialTable[i], -1);
        char section[0x80];
        sprintf(section, "Material - %d", i);
        parameterBlock->UnknownFunction4b78f0(section);
        materialTable[i]->ReadKeys(parameterBlock, loadFlag);
    }
    if (untextured) {
        materialTable[count] = new (__FILE__, 0x1ce) SoultreeMaterial(1);
        materialTable[count]->Attach((RenderTarget*)field_0x18, (const SoultreeTextureOptions*)textureOptions,
                                                  textureManager, textureFormat);
        UnknownFunction469190(materialTable[count], -1);
        materialTable[count]->MakeUntextured();
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
void D3DIMSoultreeObject::DrawWireframe(UnknownSoultreeSurface* surface)
{
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME, 1);
    for (int i = 0; i < surface->vertexCount; i++)
        surface->vertices[i].diffuse = 0;
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot15(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, (int)surface->vertices, surface->vertexCount,
                                                (int)surface->indices, surface->faceCount * 3, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID, 1);
}

// 0x004433f0
void D3DIMSoultreeObject::UnknownFunction4433f0(int surface, UnknownSoultreeVertex** vertices,
                                                int* vertexCount, unsigned short** indices, int* indexCount,
                                                int a6, int lod)
{
    if (lod == -1)
        lod = field_0x27c;
    if (field_0x18c || lod != field_0x27c)
        TransformVertexGroups(lod);
    *vertices = lodTable[lod].surfaces[surface].vertices;
    *vertexCount = lodTable[lod].surfaces[surface].vertexCount;
    *indexCount = lodTable[lod].surfaces[surface].faceCount * 3;
    *indices = lodTable[lod].surfaces[surface].indices;
}

// 0x00443490
int D3DIMSoultreeObject::UnknownVirtualSlot10(float frameTime)
{
    lastFrameTime = frameTime;
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x004434b0
void D3DIMSoultreeObject::ComputeSubtreeBounds()
{
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    Vector3 center;
    Vector3 extents;
    if (!parent) {
        UnknownFunction4fe850(&center, &extents);
        UnknownFunction443740(center, extents, 1);
    }
    UnknownFunction4fe0a0(&center, &extents);
    UnknownFunction443740(center, extents, 0);
    if (nextSibling)
        ((D3DIMSoultreeObject*)nextSibling)->ComputeSubtreeBounds();
    if (firstChild)
        ((D3DIMSoultreeObject*)firstChild)->ComputeSubtreeBounds();
}

// 0x004439c0
void D3DIMSoultreeObject::UnknownFunction4439c0(UnknownSoultreeCounters* rect)
{
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    ((PCRenderTarget*)field_0x18)->field_0x48->UnknownMethod25(0, &desc, 1, 0);
    UnknownSoultreeCameraView* camera = (UnknownSoultreeCameraView*)((RenderTarget*)field_0x18)->field_0x08;
    char* bits = (char*)desc.surface + camera->viewportY * desc.pitch + camera->viewportX;
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
    DrawCurrentLod();
    if (field_0x2d0) {
        UnknownFunction4439c0(&field_0x298);
        ComputeSubtreeBounds();
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
    copy->UnknownVirtualSlot9(field_0x18, "", (int)lightManager, (int)textureOptions, 1);
}

// 0x00444440
void D3DIMSoultreeObject::UnknownFunction444440()
{
    int count = UnknownFunction4fda30();
    SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x80b);
    int index = 1;
    nodes[0] = this;
    CollectDescendants(&index, nodes);
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
    materialCount = source->materialCount;
    materialTable = (SoultreeMaterial**)DebugMalloc(materialCount * 4, __FILE__, 0x8ed);
    for (int i = 0; i < materialCount; i++) {
        materialTable[i] = new (__FILE__, 0x8ef) SoultreeMaterial(1);
        materialTable[i]->Attach((RenderTarget*)field_0x18, (const SoultreeTextureOptions*)textureOptions,
                                              textureManager, textureFormat);
        UnknownFunction469190(materialTable[i], -1);
        materialTable[i]->CopyFrom(source->materialTable[i]);
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
    target->lightManager = source->lightManager;
    int nameLength = strlen(source->modelName);
    int length = nameLength > 0x7f ? 0x7f : nameLength;
    strncpy(target->modelName, source->modelName, length);
    target->modelName[length] = 0;
    target->lightManager = source->lightManager;
    target->textureManager = source->textureManager;
    target->textureFormat = source->textureFormat;
    memcpy(target->textureOptions, source->textureOptions, sizeof(target->textureOptions));
    target->field_0x298 = source->field_0x298;
    target->field_0x2a8 = source->field_0x2a8;
}

// 0x00444c70
int D3DIMSoultreeObject::UnknownFunction444c70(int index, const char* name, int a)
{
    if (index > materialCount)
        return 0;
    if (!materialTable[index]->hasTextureName)
        return 0;
    int nameLength = strlen(name);
    int length = nameLength > 0x3f ? 0x3f : nameLength;
    strncpy(materialTable[index]->textureName, name, length);
    materialTable[index]->textureName[length] = 0;
    materialTable[index]->LoadTexture();
    return 1;
}

// 0x00444d00
void D3DIMSoultreeObject::UnknownFunction444d00(int lod)
{
    if (lod < lowestLod)
        lod = lowestLod;
    if (lod >= lodCount)
        lod = lodCount - 1;
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
    if (lod >= lodCount)
        lod = lodCount - 1;
    lowestLod = lod;
}

// 0x00444d60
void D3DIMSoultreeObject::UnknownFunction444d60(float value)
{
    field_0x284 = 1.0f / value;
}

// 0x00444d80
void D3DIMSoultreeObject::UnknownFunction444d80(GameObject* modifier)
{
    modifierList = (GameObject**)DebugRealloc(modifierList, modifierCount * 4 + 4, __FILE__, 0x958);
    modifierList[modifierCount] = modifier;
    modifierCount++;
    ((D3DIMSoultreeModifier*)modifier)->AddObject(this);
}

// 0x00444de0
void D3DIMSoultreeObject::UnknownFunction444de0(GameObject* modifier)
{
    int found = 0;
    int i;
    for (i = 0; i < modifierCount; i++) {
        if (modifierList[i] == modifier)
            found = i;
    }
    for (i = found; i < modifierCount - 1; i++)
        modifierList[i] = modifierList[i + 1];
    modifierList = (GameObject**)DebugRealloc(modifierList, modifierCount * 4 - 4, __FILE__, 0x96d);
    modifierCount--;
    ((D3DIMSoultreeModifier*)modifier)->RemoveObject(this);
}

// 0x00444e80
void D3DIMSoultreeObject::UnknownFunction444e80()
{
    while (modifierCount)
        UnknownFunction444de0(modifierList[0]);
}

// 0x00444eb0
void D3DIMSoultreeObject::UnknownFunction444eb0(GameObject* modifier)
{
    secondModifierList = (GameObject**)DebugRealloc(secondModifierList, secondModifierCount * 4 + 4, __FILE__, 0x97f);
    secondModifierList[secondModifierCount] = modifier;
    secondModifierCount++;
    ((D3DIMSoultreeModifier*)modifier)->AddObject(this);
}

// 0x00444f10
void D3DIMSoultreeObject::UnknownFunction444f10(GameObject* modifier)
{
    int found = 0;
    int i;
    for (i = 0; i < secondModifierCount; i++) {
        if (secondModifierList[i] == modifier)
            found = i;
    }
    for (i = found; i < secondModifierCount - 1; i++)
        secondModifierList[i] = secondModifierList[i + 1];
    secondModifierList = (GameObject**)DebugRealloc(secondModifierList, secondModifierCount * 4 - 4, __FILE__, 0x994);
    secondModifierCount--;
    ((D3DIMSoultreeModifier*)modifier)->RemoveObject(this);
}

// 0x00444fb0
void D3DIMSoultreeObject::UnknownFunction444fb0()
{
    while (secondModifierCount)
        UnknownFunction444f10(secondModifierList[0]);
}

// 0x00444fe0
int D3DIMSoultreeObject::UnknownFunction444fe0()
{
    int most = 0;
    if (lodTable) {
        for (int i = 0; i < lodCount; i++) {
            int total = 0;
            for (int j = 0; j < lodTable[i].surfaceCount; j++)
                total += lodTable[i].surfaces[j].vertexCount;
            if (total > most)
                most = total;
        }
    }
    return most;
}

// 0x00445030
int D3DIMSoultreeObject::GetSurfaceCount(int lod)
{
    if (lod == -1)
        lod = field_0x27c;
    if (lodTable)
        return lodTable[lod].surfaceCount;
    return 0;
}

// 0x00445060
int D3DIMSoultreeObject::NodeMovesVertices(SoultreeObject* node, int lod)
{
    for (int i = 0; i < lodTable[lod].surfaceCount; i++) {
        UnknownSoultreeSurface* surface = &lodTable[lod].surfaces[i];
        for (int j = 0; j < surface->groupCount; j++) {
            if (surface->groups[j].node == node && surface->groups[j].vertexCount > 0)
                return 1;
        }
    }
    return 0;
}

// 0x004450c0
void D3DIMSoultreeObject::SoultreeVirtualSlot6()
{
    int saved = field_0x27c;
    for (int lod = 0; lod < lodCount; lod++) {
        field_0x27c = lod;
        TransformVertexGroups(-1);
        for (int i = 0; i < lodTable[lod].surfaceCount; i++) {
            for (int j = 0; j < lodTable[lod].surfaces[i].vertexCount; j++) {
                lodTable[lod].surfaces[i].drawnVertices[j].position.x =
                    lodTable[lod].surfaces[i].vertices[j].position.x;
                lodTable[lod].surfaces[i].drawnVertices[j].position.y =
                    lodTable[lod].surfaces[i].vertices[j].position.y;
                lodTable[lod].surfaces[i].drawnVertices[j].position.z =
                    lodTable[lod].surfaces[i].vertices[j].position.z;
            }
        }
    }
    field_0x27c = saved;
    SoultreeObject::SoultreeVirtualSlot6();
    if (!parent)
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

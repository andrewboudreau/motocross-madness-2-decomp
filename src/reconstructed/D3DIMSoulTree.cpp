// D3DIMSoulTree.cpp -- see D3DIMSoulTree.h for the evidence.

#include "D3DIMSoulTree.h"
#include "D3DIMSoultreeModifier.h"
#include "Parameterblocks.h"
#include "PCRenderTarget.h"
#include "DebugAlloc.h"
#include "ResourceManager.h"
#include "SoultreeMaterial.h"
#include "TextureMap.h"
#include "ManagedTexture.h"
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
    localBoundsValid = 0;
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
        g_UnknownResourceManager572b44->SetItemObject(item, 0);
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
            if (item->object) {
                item->AddRef();
                SoultreeVirtualSlot7((SoultreeObject*)item->object);
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
            if (item->object) {
                item->AddRef();
                SoultreeVirtualSlot7((SoultreeObject*)item->object);
                RegisterNode();
                return this;
            }
            UnknownTextureStream* stream = new (__FILE__, 0xe1) UnknownTextureStream((int)g_UnknownResourceManager572b44);
            stream->UnknownFunction460f50(name, "rb", 0);
            LoadFromParameters(stream, 0);
            delete stream;
        }
        if (item)
            g_UnknownResourceManager572b44->SetItemObject(item, this);
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
            AppendChild(materialTable[i], -1);
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
        AppendChild(materialTable[i], -1);
        char section[0x80];
        sprintf(section, "Material - %d", i);
        parameterBlock->UnknownFunction4b78f0(section);
        materialTable[i]->ReadKeys(parameterBlock, loadFlag);
    }
    if (untextured) {
        materialTable[count] = new (__FILE__, 0x1ce) SoultreeMaterial(1);
        materialTable[count]->Attach((RenderTarget*)field_0x18, (const SoultreeTextureOptions*)textureOptions,
                                                  textureManager, textureFormat);
        AppendChild(materialTable[count], -1);
        materialTable[count]->MakeUntextured();
    }
    UpdateMaterialTexelRatios();
}

// 0x00440060: reads the levels of detail from the .slt keys; returns 1
// when a surface has no material (slot 3 then adds an untextured one).
// The surface array is allocated through a second name for the level
// (`levelRef`); with `level->` there VC6 forms the s-loop surface address
// as `surfaces + offset`, retail `offset + surfaces`.
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
        UnknownSoultreeLod& levelRef = *level;
        sprintf(section, "LOD %i", lod);
        parameterBlock->UnknownFunction4b78f0(section);
        parameterBlock->UnknownFunction4b7f10("NumberOfSurfaces", 0, &level->surfaceCount);
        levelRef.surfaces = (UnknownSoultreeSurface*)DebugMalloc(levelRef.surfaceCount * 0x38, __FILE__, 0x206);
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

// 0x00440d40: the source group is read through a second name for the
// surface (`surfaceRef`); with `surface->` VC6 forms its address as
// `groups + offset`, retail `offset + groups`.
void D3DIMSoultreeObject::TransformVertexGroups(int lod)
{
    int saved = field_0x27c;
    if (lod != -1)
        field_0x27c = lod;
    for (int i = 0; i < lodTable[field_0x27c].surfaceCount; i++) {
        UnknownSoultreeSurface* surface = &lodTable[field_0x27c].surfaces[i];
        UnknownSoultreeSurface& surfaceRef = *surface;
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
            UnknownSoultreeFaceGroup* source = &surfaceRef.groups[j];
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
    subtreeDirty = 0;
    field_0x27c = saved;
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

// MatrixUtil.h's Vector3 has no binary + and -; these are the D3D_OVERLOADS
// shapes (component-wise temporaries).
static inline Vector3 operator+(const Vector3& a, const Vector3& b)
{
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// 0x00442fe0: a line from every vertex of the surface's vertex groups
// along its normal, scaled by the function-local static 0x00568944 (0.5),
// which puts the scale before the normal in each fmul. As in
// DrawVertexCrosses the group is re-indexed at each use and the points are
// copied per component; `from` and `line` are declared at function scope
// (inside the loops VC6 keeps the counter j in a register and spills the
// normal offset, retail the other way round).
void D3DIMSoultreeObject::DrawNormals(UnknownSoultreeSurface* surface)
{
    static float length = 0.5f;                    // 0x00568944
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
    Vector3 from;
    UnknownSoultreeVertex line[2];
    for (int i = 0; i < surface->groupCount; i++) {
        for (int j = 0; j < surface->groups[i].vertexCount; j++) {
            from.x = surface->groups[i].transformedVertices[j].position.x;
            from.y = surface->groups[i].transformedVertices[j].position.y;
            from.z = surface->groups[i].transformedVertices[j].position.z;
            Vector3 to = from + length * surface->groups[i].normals[j];
            from = surface->groups[i].node->UnknownFunction4fd660(from);
            to = surface->groups[i].node->UnknownFunction4fd660(to);
            line[0].position.x = from.x;
            line[0].position.y = from.y;
            line[0].position.z = from.z;
            line[0].tu = 0;
            line[0].tv = 0;
            line[0].specular = 0xff;
            line[0].diffuse = 0;
            line[1].position.x = to.x;
            line[1].position.y = to.y;
            line[1].position.z = to.z;
            line[1].tu = 0;
            line[1].tv = 0;
            line[1].diffuse = 0;
            line[1].specular = 0xffffff;
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)line, 2, 0);
        }
    }
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
}

// 0x004431f0: a six-point cross (+-size on each axis, red lines) at every
// vertex of the surface's vertex groups. The group is read through
// surface->groups[i] at each use (a group pointer local gives VC6's
// `offset + groups` add, retail adds `groups + offset`) and the transformed
// point is copied per component (a Vector3 assignment gives another frame
// slot and register allocation).
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
                Vector3 point;
                point.x = surface->groups[i].transformedVertices[j].position.x;
                point.y = surface->groups[i].transformedVertices[j].position.y;
                point.z = surface->groups[i].transformedVertices[j].position.z;
                point += offsets[k];
                point = surface->groups[i].node->UnknownFunction4fd660(point);
                cross[k].position.x = point.x;
                cross[k].position.y = point.y;
                cross[k].position.z = point.z;
            }
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot16(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)cross, 6, 0);
        }
    }
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(7, 0);
}

// 0x004433f0
void D3DIMSoultreeObject::UnknownFunction4433f0(int surface, UnknownSoultreeVertex** vertices,
                                                int* vertexCount, unsigned short** indices, int* indexCount,
                                                int a6, int lod)
{
    if (lod == -1)
        lod = field_0x27c;
    if (subtreeDirty || lod != field_0x27c)
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
        DrawBoxOutline(center, extents, 1);
    }
    UnknownFunction4fe0a0(&center, &extents);
    DrawBoxOutline(center, extents, 0);
    if (nextSibling)
        ((D3DIMSoultreeObject*)nextSibling)->ComputeSubtreeBounds();
    if (firstChild)
        ((D3DIMSoultreeObject*)firstChild)->ComputeSubtreeBounds();
}

// 0x004435b0: the axis gizmo (three lines from the node's origin, drawn in
// world space). Each transformed point is copied into the vertex per
// component; a Vector3 assignment lets VC6 copy from the call result and
// schedule the index and point stores in another order.
void D3DIMSoultreeObject::DrawAxisGizmo()
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
        vertices[i].position.x = points[i].x;
        vertices[i].position.y = points[i].y;
        vertices[i].position.z = points[i].z;
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
        ((D3DIMSoultreeObject*)nextSibling)->DrawAxisGizmo();
    if (firstChild)
        ((D3DIMSoultreeObject*)firstChild)->DrawAxisGizmo();
}

// 0x00443740: the outline of the box center +- extents (specular 0 when
// `flag` is set, red otherwise). The corners are stored per
// component, which lets VC6 share each negated extent across corners
// (retail's fchs/fst chains); Vector3 temporaries evaluate each corner apart.
void D3DIMSoultreeObject::DrawBoxOutline(Vector3 center, Vector3 extents, int flag)
{
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)field_0x18)->UnknownVirtualSlot10(2, 0);
    unsigned short indices[24] = {
        0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7,
    };
    Vector3 corners[8];
    corners[0].x = -extents.x;
    corners[0].y = extents.y;
    corners[0].z = extents.z;
    corners[1].x = extents.x;
    corners[1].y = extents.y;
    corners[1].z = extents.z;
    corners[2].x = extents.x;
    corners[2].y = extents.y;
    corners[2].z = -extents.z;
    corners[3].x = -extents.x;
    corners[3].y = extents.y;
    corners[3].z = -extents.z;
    corners[4].x = -extents.x;
    corners[4].y = -extents.y;
    corners[4].z = extents.z;
    corners[5].x = extents.x;
    corners[5].y = -extents.y;
    corners[5].z = extents.z;
    corners[6].x = extents.x;
    corners[6].y = -extents.y;
    corners[6].z = -extents.z;
    corners[7].x = -extents.x;
    corners[7].y = -extents.y;
    corners[7].z = -extents.z;
    UnknownSoultreeVertex vertices[8];
    for (int i = 0; i < 8; i++) {
        corners[i].x = center.x + corners[i].x;
        corners[i].y = center.y + corners[i].y;
        corners[i].z = center.z + corners[i].z;
        corners[i] = UnknownFunction4fd660(corners[i]);
        vertices[i].position.x = corners[i].x;
        vertices[i].position.y = corners[i].y;
        vertices[i].position.z = corners[i].z;
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

// 0x004439c0
void D3DIMSoultreeObject::UnknownFunction4439c0(UnknownSoultreeCounters* rect)
{
    UnknownSurfaceDesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    ((PCRenderTarget*)field_0x18)->renderSurface->Lock(0, &desc, 1, 0);
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
    ((PCRenderTarget*)field_0x18)->renderSurface->Unlock(0);
}

// 0x00460b50 (cdecl): table-driven square root (FollowCamera.h and
// Wrecker.h declare it too).
float UnknownFunction460b50(float value);

static inline float Magnitude(const Vector3& v)
{
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return 1.0f;
    return UnknownFunction460b50(squared);
}

// 0x00443de0: picks the level of detail from the projected size of the
// bounds (the node's own when either pointer is null) and sets each
// mipmapped texture's level. The squared lengths are grouped
// z*z + (x*x + y*y) and the scale products (scale * a) * b, as retail
// evaluates them; the bounds pointers are set before the bounds call, which
// keeps them in ebx/ebp from the prologue on.
void D3DIMSoultreeObject::SelectLod(Vector3* center, Vector3* extents)
{
    Vector3 boundsCenter;
    Vector3 boundsExtents;
    if (!center || !extents) {
        center = &boundsCenter;
        extents = &boundsExtents;
        UnknownFunction4fe850(center, extents);
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
            if ((scale * autoLodDistances[i]) * field_0x284 < 1.0f)
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
                float size = (scale * *(float*)&material->field_0xcc) * width;
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
                texture->RecordUse(level + 1 + fraction);
            }
        }
    }
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
        DrawAxisGizmo();
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
    subtreeDirty = 1;
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
        AppendChild(materialTable[i], -1);
        materialTable[i]->CopyFrom(source->materialTable[i]);
    }
}

// 0x00444b60
void D3DIMSoultreeObject::SoultreeVirtualSlot8(SoultreeObject* sourceNode, SoultreeObject* targetNode)
{
    SoultreeObject::SoultreeVirtualSlot8(sourceNode, targetNode);
    D3DIMSoultreeObject* source = (D3DIMSoultreeObject*)sourceNode;
    D3DIMSoultreeObject* target = (D3DIMSoultreeObject*)targetNode;
    target->subtreeDirty = 1;
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
        subtreeDirty = 1;
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

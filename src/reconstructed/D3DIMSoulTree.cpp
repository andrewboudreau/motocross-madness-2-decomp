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
#include "Camera.h"
#include "LightEmitter.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 0x0043f160
D3DIMSoultreeObject::D3DIMSoultreeObject(int flags)
    : SoultreeObject(flags)
{
    useLighting = 0;
    showNormals = 0;
    showVertexCrosses = 0;
    texturePerspective = 1;
    gouraudShading = 1;
    textureManager = 0;
    textureFormat = 555;
    materialTable = 0;
    materialCount = 0;
    modelName[0] = 0;
    lodCount = 0;
    currentLod = 0;
    lowestLod = 0;
    lodTable = 0;
    autoLodDistances = 0;
    inverseLodThreshold = 1.0f;
    useAutoLod = 0;
    lightManager = 0;
    modifierList = 0;
    modifierCount = 0;
    secondModifierList = 0;
    secondModifierCount = 0;
    screenRect.left = 0;
    screenRect.top = 0;
    screenRect.right = 0;
    screenRect.bottom = 0;
    textureScroll = 0;
    // Retail fills the temporary z first (a plain Vector3(0, 0, 0) stores
    // it x first).
    Vector3 zero;
    zero.z = 0.0f;
    zero.y = 0.0f;
    zero.x = 0.0f;
    lastScrollPosition = zero;
    showBounds = 0;
    localBoundsValid = 0;
    showWireframe = 0;
    field_0x260 = 0;
    visible = 1;
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
                DebugFree(lodTable[i].surfaces[j].localVertices, __FILE__, 0x6a);
                DebugFree(lodTable[i].surfaces[j].normals, __FILE__, 0x6b);
                DebugFree(lodTable[i].surfaces[j].uvs, __FILE__, 0x6c);
                DebugFree(lodTable[i].surfaces[j].materialIndices, __FILE__, 0x6d);
            }
            DebugFree(lodTable[i].surfaces, __FILE__, 0x70);
        }
        DebugFree(lodTable, __FILE__, 0x72);
    }
    RemoveAllModifiers();
    RemoveAllSecondModifiers();
    void* item = g_ResourceManager->FindItemByObject(this);
    if (item)
        g_ResourceManager->SetItemObject(item, 0);
}

// 0x0043f4b0
GameObject* D3DIMSoultreeObject::LoadModel(void* owner, const char* path, const LightManager* lights, const TextureContext* textures,
                                           int useOptions)
{
    GameObject::SetRenderContext(owner);
    if (useOptions) {
        textureOptions = *textures;
        textureManager = textures->manager;
    }
    loadFlag = useOptions;
    lightManager = (LightManager*)lights;
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
        ResourceItem* item = g_ResourceManager->FindItem(name, 0);
        if (!item) {
            g_ResourceManager->AddLooseFile(name, fullPath);
            item = g_ResourceManager->FindItem(name, 0);
        }
        if (item) {
            if (item->object) {
                item->AddRef();
                CloneFrom((SoultreeObject*)item->object);
                RegisterNode();
                return this;
            }
            ((FileStream*)item->archive)->Seek(item->offset, 0, 0);
            LoadHierarchy((FileStream*)item->archive);
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
            item = g_ResourceManager->FindItem(name, 0);
            if (!item) {
                g_ResourceManager->AddLooseFile(name, fullPath);
                item = g_ResourceManager->FindItem(name, 0);
                if (!item) {
                    RegisterNode();
                    return this;
                }
            }
            if (item->object) {
                item->AddRef();
                CloneFrom((SoultreeObject*)item->object);
                RegisterNode();
                return this;
            }
            FileStream* stream = new (__FILE__, 0xe1) FileStream(g_ResourceManager);
            stream->Open(name, "rb", 0);
            LoadFromParameters(stream, 0);
            delete stream;
        }
        if (item)
            g_ResourceManager->SetItemObject(item, this);
        RegisterNode();
    }
    return this;
}

// 0x0043f950
void D3DIMSoultreeObject::LoadHierarchy(FileStream* stream)
{
    SoultreeObject::LoadHierarchy(stream);
    int count = CountNodes();
    SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x14e);
    int index = 1;
    nodes[0] = this;
    CollectDescendants(&index, nodes);
    stream->Read(&materialCount, 4, 1);
    if (materialCount > 0) {
        materialTable = (SoultreeMaterial**)DebugMalloc(materialCount * 4, __FILE__, 0x156);
        for (int i = 0; i < materialCount; i++) {
            materialTable[i] = new (__FILE__, 0x158) SoultreeMaterial(1);
            materialTable[i]->Attach((RenderTarget*)renderContext, &textureOptions,
                                                  textureManager, textureFormat);
            AppendChild(materialTable[i], -1);
            materialTable[i]->ReadSaved((FileStream*)stream);
        }
    } else {
        materialTable = 0;
    }
    stream->Read(&lodCount, 4, 1);
    stream->Read(&useAutoLod, 4, 1);
    if (useAutoLod) {
        autoLodDistances = (float*)DebugMalloc(lodCount * 4, __FILE__, 0x166);
        for (int i = 0; i < lodCount - 1; i++)
            stream->Read(&autoLodDistances[i], 4, 1);
    }
    if (lodCount > 0) {
        lodTable = (SoultreeLod*)DebugMalloc(lodCount * 8, __FILE__, 0x16f);
        for (int lod = 0; lod < lodCount; lod++) {
            stream->Read(&lodTable[lod].surfaceCount, 4, 1);
            lodTable[lod].surfaces =
                (SoultreeSurface*)DebugMalloc(lodTable[lod].surfaceCount * 0x38, __FILE__, 0x173);
            for (int i = 0; i < lodTable[lod].surfaceCount; i++) {
                SoultreeSurface* surface = &lodTable[lod].surfaces[i];
                stream->Read(&surface->groupCount, 4, 1);
                stream->Read(&surface->vertexCount, 4, 1);
                stream->Read(&surface->faceCount, 4, 1);
                stream->Read(&surface->materialCount, 4, 1);
                surface->mappingOffsetU = 0;
                surface->mappingOffsetV = 0;
                surface->mappingScale = 1.0f;
                surface->vertices = (SoultreeVertex*)DebugMalloc(surface->vertexCount * 0x20, __FILE__, 0x181);
                stream->Read(surface->vertices, 0x20, surface->vertexCount);
                surface->localVertices = (SoultreeVertex*)DebugMalloc(surface->vertexCount * 0x20, __FILE__, 0x184);
                stream->Read(surface->localVertices, 0x20, surface->vertexCount);
                surface->normals = (Vector3*)DebugMalloc(surface->vertexCount * 0xc, __FILE__, 0x187);
                stream->Read(surface->normals, 0xc, surface->vertexCount);
                surface->indices = (unsigned short*)DebugMalloc(surface->faceCount * 6, __FILE__, 0x18a);
                stream->Read(surface->indices, 6, surface->faceCount);
                surface->uvs = (SoultreeUV*)DebugMalloc(surface->vertexCount * 8, __FILE__, 0x18d);
                stream->Read(surface->uvs, 4, surface->vertexCount * 2);
                surface->materialIndices = (int*)DebugMalloc(surface->materialCount * 4, __FILE__, 0x191);
                stream->Read(surface->materialIndices, 4, surface->materialCount);
                surface->groups = (SoultreeVertexGroup*)DebugMalloc(surface->groupCount * 0x14, __FILE__, 0x194);
                stream->Read(surface->groups, 0x14, surface->groupCount);
                for (int j = 0; j < surface->groupCount; j++) {
                    int node;
                    int first;
                    stream->Read(&node, 4, 1);
                    stream->Read(&first, 4, 1);
                    surface->groups[j].node = nodes[node];
                    surface->groups[j].vertices = &surface->vertices[first];
                    surface->groups[j].localVertices = &surface->localVertices[first];
                    surface->groups[j].normals = &surface->normals[first];
                }
            }
        }
    } else {
        lodTable = 0;
    }
    DebugFree(nodes, __FILE__, 0x1a7);
    ComputeAllBounds();
}

// 0x0043fe40
void D3DIMSoultreeObject::LoadHierarchyFromParameters()
{
    parameterBlock->SelectSection("Materials");
    parameterBlock->GetInt("NumberOfMaterials", 0, &materialCount);
    SoultreeObject::LoadHierarchyFromParameters();
    int untextured = ReadLods();
    int count = materialCount;
    if (untextured)
        materialCount = count + 1;
    materialTable = (SoultreeMaterial**)DebugMalloc(materialCount * 4, __FILE__, 0x1c0);
    for (int i = 0; i < count; i++) {
        materialTable[i] = new (__FILE__, 0x1c5) SoultreeMaterial(1);
        materialTable[i]->Attach((RenderTarget*)renderContext, &textureOptions,
                                              textureManager, textureFormat);
        AppendChild(materialTable[i], -1);
        char section[0x80];
        sprintf(section, "Material - %d", i);
        parameterBlock->SelectSection(section);
        materialTable[i]->ReadKeys(parameterBlock, loadFlag);
    }
    if (untextured) {
        materialTable[count] = new (__FILE__, 0x1ce) SoultreeMaterial(1);
        materialTable[count]->Attach((RenderTarget*)renderContext, &textureOptions,
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
    int count = CountNodes();
    SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x1dd);
    int index = 1;
    nodes[0] = this;
    CollectDescendants(&index, nodes);
    char section[0x80];
    sprintf(section, "LOD Information");
    parameterBlock->SelectSection(section);
    parameterBlock->GetInt("NumberOfLOD", 0, &lodCount);
    parameterBlock->GetInt("UseAutoLOD", 0, &useAutoLod);
    int i;
    if (useAutoLod) {
        autoLodDistances = (float*)DebugMalloc(lodCount * 4 - 1, __FILE__, 0x1ec);
        for (i = 0; i < lodCount - 1; i++) {
            sprintf(section, "AutoLOD#%i", i);
            parameterBlock->GetFloat(section, -1.0f, &autoLodDistances[i]);
        }
    }
    lodTable = (SoultreeLod*)DebugMalloc(lodCount * 8, __FILE__, 0x1f5);
    SoultreeVertexGroup* groups = (SoultreeVertexGroup*)DebugMalloc(count * 0x14, __FILE__, 0x1f9);
    for (i = 0; i < count; i++)
        groups[i].node = nodes[i];
    for (int lod = 0; lod < lodCount; lod++) {
        SoultreeLod* level = &lodTable[lod];
        SoultreeLod& levelRef = *level;
        sprintf(section, "LOD %i", lod);
        parameterBlock->SelectSection(section);
        parameterBlock->GetInt("NumberOfSurfaces", 0, &level->surfaceCount);
        levelRef.surfaces = (SoultreeSurface*)DebugMalloc(levelRef.surfaceCount * 0x38, __FILE__, 0x206);
        for (int s = 0; s < level->surfaceCount; s++) {
            SoultreeSurface* surface = &level->surfaces[s];
            surface->mappingScale = 1.0f;
            surface->mappingOffsetU = 0;
            surface->mappingOffsetV = 0;
            sprintf(section, "LOD %i - Surface %i", lod, s);
            parameterBlock->SelectSection(section);
            parameterBlock->GetInt("NumberOfVertices", 0, &surface->vertexCount);
            parameterBlock->GetInt("NumberOfFaces", 0, &surface->faceCount);
            parameterBlock->GetInt("NumberOfMaterials", 0, &surface->materialCount);
            if (surface->materialCount > 0) {
                surface->materialIndices = (int*)DebugMalloc(surface->materialCount * 4, __FILE__, 0x216);
                for (int m = 0; m < surface->materialCount; m++) {
                    char key[0x80];
                    sprintf(key, "Material#%i", m);
                    parameterBlock->GetInt(key, 0, &surface->materialIndices[m]);
                }
            } else {
                untextured = 1;
                surface->materialIndices = (int*)DebugMalloc(4, __FILE__, 0x21e);
                surface->materialIndices[0] = 0;
                surface->materialCount = 1;
            }
            surface->vertices = (SoultreeVertex*)DebugMalloc(surface->vertexCount * 0x20, __FILE__, 0x223);
            surface->localVertices = (SoultreeVertex*)DebugMalloc(surface->vertexCount * 0x20, __FILE__, 0x224);
            surface->normals = (Vector3*)DebugMalloc(surface->vertexCount * 0xc, __FILE__, 0x225);
            surface->indices = (unsigned short*)DebugMalloc(surface->faceCount * 6, __FILE__, 0x226);
            surface->uvs = (SoultreeUV*)DebugMalloc(surface->vertexCount * 8, __FILE__, 0x227);
            sprintf(section, "LOD %i - Surface %i - Vertices", lod, s);
            parameterBlock->BeginRows(section);
            int v;
            for (v = 0; v < surface->vertexCount; v++) {
                float red;
                float green;
                float blue;
                parameterBlock->ReadRow(0);
                parameterBlock->GetRowFloat(0, &surface->localVertices[v].position.x);
                parameterBlock->GetRowFloat(1, &surface->localVertices[v].position.y);
                parameterBlock->GetRowFloat(2, &surface->localVertices[v].position.z);
                parameterBlock->GetRowFloat(3, &surface->normals[v].x);
                parameterBlock->GetRowFloat(4, &surface->normals[v].y);
                parameterBlock->GetRowFloat(5, &surface->normals[v].z);
                parameterBlock->GetRowFloat(6, &surface->localVertices[v].tu);
                parameterBlock->GetRowFloat(7, &surface->localVertices[v].tv);
                parameterBlock->GetRowFloat(12, &red);
                parameterBlock->GetRowFloat(13, &green);
                parameterBlock->GetRowFloat(14, &blue);
                long r = (long)(red * 255.0);
                long g = (long)(green * 255.0);
                long b = (long)(blue * 255.0);
                surface->localVertices[v].diffuse = 0xff000000 | (r << 16) | (g << 8) | b;
                surface->localVertices[v].specular = 0;
                surface->vertices[v] = surface->localVertices[v];
                surface->uvs[v].u = surface->vertices[v].tu;
                surface->uvs[v].v = surface->vertices[v].tv;
            }
            sprintf(section, "LOD %i - Surface %i - Faces", lod, s);
            parameterBlock->BeginRows(section);
            for (int f = 0; f < surface->faceCount; f++) {
                int corner0;
                int corner1;
                int corner2;
                parameterBlock->ReadRow(0);
                parameterBlock->GetRowInt(0, &corner0);
                parameterBlock->GetRowInt(1, &corner1);
                parameterBlock->GetRowInt(2, &corner2);
                surface->indices[f * 3] = corner0;
                surface->indices[f * 3 + 1] = corner1;
                surface->indices[f * 3 + 2] = corner2;
            }
            sprintf(section, "LOD %i - Surface %i - Object Pointer List", lod, s);
            parameterBlock->BeginRows(section);
            int used = 0;
            for (i = 0; i < count; i++) {
                int first;
                int vertices;
                parameterBlock->ReadRow(0);
                parameterBlock->GetRowInt(0, &first);
                parameterBlock->GetRowInt(1, &vertices);
                groups[i].vertexCount = vertices;
                groups[i].vertices = &surface->vertices[first];
                groups[i].localVertices = &surface->localVertices[first];
                groups[i].normals = &surface->normals[first];
                if (vertices > 0)
                    used++;
            }
            surface->groupCount = used;
            surface->groups = (SoultreeVertexGroup*)DebugMalloc(used * 0x14, __FILE__, 0x283);
            int k = 0;
            for (i = 0; i < count; i++) {
                if (groups[i].vertexCount > 0)
                    surface->groups[k++] = groups[i];
            }
        }
    }
    ComputeAllBounds();
    DebugFree(groups, __FILE__, 0x291);
    DebugFree(nodes, __FILE__, 0x292);
    return untextured;
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

// 0x00460c00 (cdecl): table-driven reciprocal square root.
float FastInvSqrt(float x);

// |v| through the reciprocal square root, exact for 0 and 1. The squared
// length is grouped z*z + (x*x + y*y), as in Magnitude below.
static inline float Length(const Vector3& v)
{
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 0.0f)
        return 0.0f;
    if (squared == 1.0f)
        return 1.0f;
    return 1.0f / FastInvSqrt(squared);
}

// 0x00440810: for every surface of the current level of detail, the
// largest ratio of edge length to texture-coordinate length over its
// faces, divided by the texture height, raises each textured material's
// +0xcc. The corner positions are copied per component (as from a
// D3DTLVERTEX's sx/sy/sz): as whole-Vector3 copies VC6 keeps a second
// pointer to each vertex for the copy and spills the face offset, which
// retail keeps in ebx. The uv delta is a copy of a zeroed Vector3 taken
// once per face, which gives retail's single z*z.
void D3DIMSoultreeObject::UpdateMaterialTexelRatios()
{
    TransformVertexGroups(-1);
    for (int i = 0; i < lodTable[currentLod].surfaceCount; i++) {
        SoultreeSurface* surface = &lodTable[currentLod].surfaces[i];
        float best = 0.0f;
        for (int f = 0; f < surface->faceCount; f++) {
            Vector3 zero(0.0f, 0.0f, 0.0f);
            unsigned short* face = &surface->indices[f * 3];
            Vector3 p0;
            p0.x = surface->vertices[face[0]].position.x;
            p0.y = surface->vertices[face[0]].position.y;
            p0.z = surface->vertices[face[0]].position.z;
            Vector3 p1;
            p1.x = surface->vertices[face[1]].position.x;
            p1.y = surface->vertices[face[1]].position.y;
            p1.z = surface->vertices[face[1]].position.z;
            Vector3 p2;
            p2.x = surface->vertices[face[2]].position.x;
            p2.y = surface->vertices[face[2]].position.y;
            p2.z = surface->vertices[face[2]].position.z;

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
            if (material->surfaceMap) {
                float scale = best / material->surfaceMap->texture->textureHeight;
                float current = *(float*)&material->texelRatio;
                *(float*)&material->texelRatio = current > scale ? current : scale;
            }
        }
    }
}

// 0x00440d40: the source group is read through a second name for the
// surface (`surfaceRef`); with `surface->` VC6 forms its address as
// `groups + offset`, retail `offset + groups`.
void D3DIMSoultreeObject::TransformVertexGroups(int lod)
{
    int saved = currentLod;
    if (lod != -1)
        currentLod = lod;
    for (int i = 0; i < lodTable[currentLod].surfaceCount; i++) {
        SoultreeSurface* surface = &lodTable[currentLod].surfaces[i];
        SoultreeSurface& surfaceRef = *surface;
        int colored = 0;
        int k;
        for (k = 0; k < surface->materialCount; k++) {
            SoultreeMaterial* material = materialTable[surface->materialIndices[k]];
            if (!material->hasTextureName || material->useVertexColor)
                colored = 1;
        }
        SoultreeSurface* drawn;
        if (secondModifierCount > 0) {
            for (k = 0; k < secondModifierCount; k++)
                ((D3DIMSoultreeModifier*)secondModifierList[k])->ModifyMesh(
                    this, (ModifierSoultreeSurfaceView*)surface, (ModifierSoultreeSurfaceView**)&drawn);
        } else {
            drawn = surface;
        }
        for (int j = 0; j < drawn->groupCount; j++) {
            SoultreeVertexGroup* group = &drawn->groups[j];
            SoultreeVertexGroup* source = &surfaceRef.groups[j];
            int count = group->vertexCount;
            if (count) {
                group->node->UpdateWorldMatrix();
                void* localVertices = group->localVertices;
                void* worldVertices = source->vertices;
                if (useLighting) {
                    if (colored)
                        lightManager->LightVertices(&group->node->worldMatrix, count,
                                                   group->normals, worldVertices, 0xc, localVertices);
                    else
                        lightManager->LightVertices(&group->node->worldMatrix, count,
                                                   group->normals, worldVertices, 0xc, 0);
                }
                TransformPoints(worldVertices, localVertices, &group->node->worldMatrix, count, 0x20, 0x20);
            }
        }
    }
    subtreeDirty = 0;
    currentLod = saved;
}

// soultree.cpp node calls only these near misses use. They are kept out of
// D3DIMSoulTree.h: more declarations there change VC6's operand order in
// SceneManager.cpp's exact 0x004eb570.
class DrawSoultreeObjectView : public SoultreeObject {
public:
    void GetMatrixIn(SoultreeObject* frame, Matrix4* out); // 0x004fca80: matrix in `frame` (0: world)
    void GetPositionIn(SoultreeObject* frame, Vector3* out); // 0x004fc9a0: position in `frame` (0: world)
    Vector3 WorldToLocalDirection(const Vector3* v);                 // 0x004fd710: world to local direction
    Vector3 WorldToLocalPoint(const Vector3* v);                 // 0x004fd7f0: world to local point
    Vector3 LocalToWorldDirection(const Vector3* v);                 // 0x004fd5c0: local to world direction
};

// The box test at 0x00575a98 (VisibilityClipper, 0x0052f570, ret 0x1c):
// camera, view matrix, box centre and half extent, screen rectangle,
// corner count, and an unused last argument. Declared as
// src/krusty2/visibility/VisibilityClipper.h does (the provider's names).
struct VisibilityCamera;
class VisibilityClipper {
public:
    int TestBox(VisibilityCamera* camera, const float* matrix, const float* center,
                const float* extent, int* screenRect, int* cornersInside, int* unused);
};
extern VisibilityClipper* g_visibilityClipper;   // 0x00575a98

struct SoultreeD3DMatrix {
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

// 0x0040ae30 (cdecl): the out-of-line dot product (defined as Vec3DotCall).
float Vec3DotCall(const Vec3* a, const Vec3* b);
static inline float Vec3Dot(const Vector3* a, const Vector3* b)
{
    return Vec3DotCall(reinterpret_cast<const Vec3*>(a), reinterpret_cast<const Vec3*>(b));
}
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
// retail calls in the mode 8 and mode 2 normalisations. Declared only;
// config/game_build.json links it to Vec3Call's constructor
// (src/krusty2/bvh/BoundingBoxTreeQuery.cpp), the same body.
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

// Mode 2's scroll distance: |v| summed y*y, x*x, z*z. With Length's
// z*z + (x*x + y*y) grouping (0x00440810) the result differs from retail at
// 24-bit precision (tools/emu_cases/soultree_draw_current_lod.py).
static inline float ScrollLength(const Vector3& v)
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

// 0x00440f30. Not exact (DRAWSCORE of 7851 bytes: operand order and frame
// layout differ; docs/D3DIMSOULTREE.md forms tried).
// Draws the current level of detail. Per surface and material
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
    int k = 0;  // best guess: retail leaves it uninitialised (mode 5 below)
    for (int surfaceIndex = 0; surfaceIndex < lodTable[currentLod].surfaceCount; surfaceIndex++) {
        SoultreeSurface* surface = &lodTable[currentLod].surfaces[surfaceIndex];
        int count = surface->vertexCount;
        int modified = 0;
        int restore = 0;
        for (int materialIndex = 0; materialIndex < surface->materialCount; materialIndex++) {
            SoultreeMaterial* material = materialTable[surface->materialIndices[materialIndex]];
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
                    if (surface->localVertices[v].position.x > 0.0f) {
                        surface->vertices[v].tv = Remap(surface->localVertices[v].position.y, -1.0f, 3.0f);
                        surface->vertices[v].tu = RemapDown(surface->localVertices[v].position.z, 1.0f, -2.4f);
                    } else {
                        surface->vertices[v].tv = Remap(surface->localVertices[v].position.y, -1.0f, 3.0f);
                        surface->vertices[v].tu = RemapDown(surface->localVertices[v].position.z, -1.0f, 1.4f);
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
                    SoultreeVertexGroup* group = &surface->groups[j];
                    int vertices = group->vertexCount;
                    Vector3 position;
                    ((DrawSoultreeObjectView*)group->node)->GetPositionIn(0, &position);
                    Vector3 direction = position - lightPosition;
                    direction = NormalizedByCall(direction);
                    direction = ((DrawSoultreeObjectView*)group->node)->WorldToLocalDirection(&direction);
                    for (int v = 0; v < vertices; v++) {
                        const Vector3& normal = group->normals[v];
                        group->vertices[v].tu =
                            1.0f - ((direction.y * normal.y + direction.x * normal.x) +
                                    direction.z * normal.z + 1.0f) * 0.5f;
                        group->vertices[v].tv = 0;
                    }
                }
                modified = 1;
                restore = 1;
                break;
            }
            case 4: {
                Vector3 eye = ((D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position;
                for (int j = 0; j < surface->groupCount; j++) {
                    SoultreeVertexGroup* group = &surface->groups[j];
                    int vertices = group->vertexCount;
                    Vector3 position;
                    ((DrawSoultreeObjectView*)group->node)->GetPositionIn(0, &position);
                    Vector3 direction = NormalizedByCall(position - eye);
                    direction = ((DrawSoultreeObjectView*)group->node)->WorldToLocalDirection(&direction);
                    for (int v = 0; v < vertices; v++) {
                        const Vector3& normal = group->normals[v];
                        group->vertices[v].tu =
                            1.0f - ((direction.y * normal.y + direction.x * normal.x) +
                                    direction.z * normal.z + 1.0f) * 0.5f;
                        group->vertices[v].tv = 0;
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
                SoultreeVertex line[2];
                line[0].diffuse = 0xffff0000;
                line[1].diffuse = 0xffffffff;
                if (updateEye)
                    (Vector3&)eye = ((D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position;
                for (int j = 0; j < surface->groupCount; j++) {
                    SoultreeVertexGroup* group = &surface->groups[j];
                    Vector3 localEye = ((DrawSoultreeObjectView*)group->node)->WorldToLocalPoint(&eye);
                    if (!count)
                        continue;
                    for (int v = 0; v < group->vertexCount; v++) {
                        Vector3 direction;
                        direction.x = group->localVertices[v].position.x - localEye.x;
                        direction.y = group->localVertices[v].position.y - localEye.y;
                        direction.z = group->localVertices[v].position.z - localEye.z;
                        direction = NormalizedCallingConstructor(direction);
                        Vector3 reflected;
                        ReflectAndNormalize(&direction, &group->normals[v], &reflected);
                        if (drawReflections) {
                            Vector3 world = ((DrawSoultreeObjectView*)group->node)->LocalToWorldDirection(&reflected);
                            line[0].position.x = group->vertices[v].position.x;
                            line[0].position.y = group->vertices[v].position.y;
                            line[0].position.z = group->vertices[v].position.z;
                            line[1].position.x = world.x + group->vertices[v].position.x;
                            line[1].position.y = world.y + group->vertices[v].position.y;
                            line[1].position.z = world.z + group->vertices[v].position.z;
                            ((RenderTarget*)renderContext)->DrawPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)line, 2, 0);
                        }
                        group->vertices[v].tv = ((float)asin(reflected.y) + 1.5707964f) * -0.31830987f;
                        group->vertices[v].tv = -((reflected.y + 1.0f) * 0.5f);
                        group->vertices[v].tv = 0.25f;
                        reflected.y = 0.0f;
                        reflected = NormalizedCallingConstructor(reflected);
                        group->vertices[v].tu =
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
                float moved = ScrollLength(lastScrollPosition - position) * 0.0075f;
                lastScrollPosition = position;
                textureScroll += moved;
                if (textureScroll > 100.0f)
                    textureScroll -= 100.0f;
                float scroll = textureScroll;
                Vector3 eye = ((D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position;
                for (int j = 0; j < surface->groupCount; j++) {
                    SoultreeVertexGroup* group = &surface->groups[j];
                    count = group->vertexCount;
                    if (!count)
                        continue;
                    group->node->UpdateWorldMatrix();
                    Matrix4* world = &group->node->worldMatrix;
                    for (int v = 0; v < count; v++) {
                        Vector3 direction = NormalizedCallingConstructor(group->vertices[v].position - eye);
                        Vector3 normal = group->normals[v];
                        // One statement per component: VC6 then adds the y row as retail does,
                        // (x m01 + z m21) + y m11 (0x00441cf8..0x00441d18); the constructor
                        // form groups it (z m21 + y m11) + x m01, an ulp off at 24 bits.
                        Vector3 worldNormal;
                        worldNormal.x = normal.x * world->m[0][0] + normal.y * world->m[1][0] + normal.z * world->m[2][0];
                        worldNormal.y = normal.x * world->m[0][1] + normal.y * world->m[1][1] + normal.z * world->m[2][1];
                        worldNormal.z = normal.x * world->m[0][2] + normal.y * world->m[1][2] + normal.z * world->m[2][2];
                        Vector3 twice = worldNormal + worldNormal;
                        // (y + x) + z as retail sums it (0x00441d52..0x00441d6f).
                        float facing = (direction.x * worldNormal.x + direction.y * worldNormal.y)
                                       + direction.z * worldNormal.z;
                        Vector3 reflected = twice * facing - direction;
                        float halfX = reflected.x * 0.5f;
                        float halfY = reflected.y * 0.5f;
                        group->vertices[v].tu = halfX + 0.5f;
                        group->vertices[v].tv = halfY + 0.5f - scroll;
                    }
                }
                restore = 1;
                break;
            }
            case 5: {
                Vector3 center(0.0f, 15.8278f, 0.0f);
                ((DrawSoultreeObjectView*)this)->GetPositionIn(0, &center);
                center.y = 15.8278f;
                Vector3 circle[16];
                int i;
                for (i = 0; i < 16; i++) {
                    float angle = i * 0.3926991f;
                    circle[i].x = (float)cos(angle) * 128.0f + center.x;
                    circle[i].z = (float)sin(angle) * 128.0f + center.z;
                    circle[i].y = 15.8278f;
                }
                SoultreeVertex ring[32];
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
                    (Vector3&)viewDirection = ((D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->viewDirection;
                    (Vector3&)viewPosition = ((D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position;
                }
                SoultreeVertex lines[8];                 // 4 used; 8 inferred from the frame
                for (i = 0; i < 4; i++) {
                    lines[i].diffuse = 0xffff0000;
                    lines[i].specular = 0xff000000;
                    lines[i].tu = 0;
                    lines[i].tv = 0;
                }
                for (int j = 0; j < surface->groupCount; j++) {
                    SoultreeVertexGroup* group = &surface->groups[k];
                    count = group->vertexCount;
                    if (!count)
                        continue;
                    group->node->UpdateWorldMatrix();
                    Matrix4* world = &group->node->worldMatrix;
                    for (int v = 0; v < count; v++) {
                        Vector3 position = group->vertices[v].position;
                        Vector3 segment[2];
                        segment[0] = position;
                        Vector3 direction = Normalized(position - viewPosition);
                        Vector3 normal = group->normals[v];
                        // Retail adds (x m00 + z m20) + y m10, then (z m21 + y m11) + x m01
                        // and (z m22 + y m12) + x m02 (0x004421b3..0x0044221c).
                        Vector3 worldNormal(
                            (normal.x * world->m[0][0] + normal.z * world->m[2][0]) + normal.y * world->m[1][0],
                            (normal.y * world->m[1][1] + normal.z * world->m[2][1]) + normal.x * world->m[0][1],
                            (normal.y * world->m[1][2] + normal.z * world->m[2][2]) + normal.x * world->m[0][2]);
                        float facing = direction.x * worldNormal.x;
                        facing += direction.y * worldNormal.y;
                        facing += direction.z * worldNormal.z;
                        Vector3 offset = worldNormal * facing;
                        Vector3 reflected = direction - (offset + offset);
                        Vector3 target = position + reflected;
                        // Where the reflected ray meets the vertical cylinder of
                        // radius 128 about `center`.
                        // No dy: retail forms target.y - position.y only inside the hit
                        // branch (0x004423d2) and keeps dx/dz on the x87 stack until then.
                        float dx = target.x - position.x;
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
                                hit = position + (target - position) * t;
                            else if ((-b - root) / (a + a) > 0.0f)
                                hit = position + (target - position) * t;
                        }
                        float fresnel = 1.0f - (float)fabs(facing);
                        segment[1] = hit;
                        group->vertices[v].tv = hit.y * 0.01f;
                        if (group->vertices[v].tv > 1.0f)
                            group->vertices[v].tv = 1.0f;
                        group->vertices[v].tv = 1.0f - group->vertices[v].tv;
                        Vector3 around = hit - center;
                        around.y = 0.0f;
                        around = Normalized(around);
                        float angle = (float)acos(around.z);
                        if (around.x < 0.0f)
                            angle = -angle;
                        group->vertices[v].tu = angle * 0.15915494f;
                        group->vertices[v].diffuse = ((long)(fresnel * 255.0f) << 24) | 0xffffff;
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
                    SoultreeVertexGroup* group = &surface->groups[j];
                    Vector3 lightPosition = ((DrawSoultreeObjectView*)group->node)->WorldToLocalDirection(&light->lightPosition);
                    Vector3 eye = ((DrawSoultreeObjectView*)group->node)->WorldToLocalDirection(
                        &((D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position);
                    for (int v = 0; v < group->vertexCount; v++) {
                        Vector3 position = group->vertices[v].position;
                        Vector3 toLight = Normalized(position - lightPosition);
                        Vector3 toEye = Normalized(eye - position);
                        Vector3& normal = group->normals[v];
                        float twiceDot =
                            (toLight.x * normal.x + toLight.y * normal.y + toLight.z * normal.z) * 2.0f;
                        Vector3 reflected = toLight - normal * twiceDot;
                        float specular = toEye.x * reflected.x;
                        specular += toEye.y * reflected.y;
                        specular += toEye.z * reflected.z;
                        if (specular < 0.0f)
                            specular = 0.0f;
                        group->vertices[v].diffuse = ((long)(pow(specular, exponent) * 255.0) << 24) | 0xffffff;
                    }
                }
                modified = 1;
                restore = 0;
                break;
            }
            }
            SoultreeSurface* drawn;
            if (modifierCount > 0) {
                for (int i = 0; i < modifierCount; i++)
                    ((D3DIMSoultreeModifier*)modifierList[i])->ModifyMesh(
                        this, (ModifierSoultreeSurfaceView*)surface, (ModifierSoultreeSurfaceView**)&drawn);
            } else {
                drawn = surface;
            }
            if (drawn->vertexCount) {
                if (material->surfaceMap) {
                    ManagedTexture* texture = (ManagedTexture*)material->surfaceMap->texture;
                    if (texture->mapFlags & 1)
                        texture->RemapTexCoordsIfMoved(&drawn->mappingScale, (float*)&drawn->mappingOffsetU,
                                                       (float*)&drawn->mappingOffsetV, &drawn->vertices->tu,
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
                surface->mappingScale = 1.0f;
                surface->mappingOffsetU = 0;
                surface->mappingOffsetV = 0;
            }
            material->RestoreRenderStates();
        }
    }
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, savedCull, 0);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_SHADEMODE, savedShade, 0);
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
void D3DIMSoultreeObject::DrawWireframe(SoultreeSurface* surface)
{
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME, 1);
    for (int i = 0; i < surface->vertexCount; i++)
        surface->vertices[i].diffuse = 0;
    ((RenderTarget*)renderContext)->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, (int)surface->vertices, surface->vertexCount,
                                                (int)surface->indices, surface->faceCount * 3, 0);
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID, 1);
}

// 0x00442fe0: a line from every vertex of the surface's vertex groups
// along its normal, scaled by the function-local static 0x00568944 (0.5),
// which puts the scale before the normal in each fmul. As in
// DrawVertexCrosses the group is re-indexed at each use and the points are
// copied per component; `from` and `line` are declared at function scope
// (inside the loops VC6 keeps the counter j in a register and spills the
// normal offset, retail the other way round).
void D3DIMSoultreeObject::DrawNormals(SoultreeSurface* surface)
{
    static float length = 0.5f;                    // 0x00568944
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)renderContext)->SetTextureBlend(2, 0);
    Vector3 from;
    SoultreeVertex line[2];
    for (int i = 0; i < surface->groupCount; i++) {
        for (int j = 0; j < surface->groups[i].vertexCount; j++) {
            from.x = surface->groups[i].localVertices[j].position.x;
            from.y = surface->groups[i].localVertices[j].position.y;
            from.z = surface->groups[i].localVertices[j].position.z;
            Vector3 to = from + length * surface->groups[i].normals[j];
            from = surface->groups[i].node->LocalToWorldPoint(from);
            to = surface->groups[i].node->LocalToWorldPoint(to);
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
            ((RenderTarget*)renderContext)->DrawPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)line, 2, 0);
        }
    }
    ((RenderTarget*)renderContext)->SetTextureBlend(7, 0);
}

// 0x004431f0: a six-point cross (+-size on each axis, red lines) at every
// vertex of the surface's vertex groups. The group is read through
// surface->groups[i] at each use (a group pointer local gives VC6's
// `offset + groups` add, retail adds `groups + offset`) and the transformed
// point is copied per component (a Vector3 assignment gives another frame
// slot and register allocation).
void D3DIMSoultreeObject::DrawVertexCrosses(SoultreeSurface* surface)
{
    static float size = 0.025f;                    // 0x00568948
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)renderContext)->SetTextureBlend(2, 0);
    Vector3 offsets[6];
    SoultreeVertex cross[6];
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
                point.x = surface->groups[i].localVertices[j].position.x;
                point.y = surface->groups[i].localVertices[j].position.y;
                point.z = surface->groups[i].localVertices[j].position.z;
                point += offsets[k];
                point = surface->groups[i].node->LocalToWorldPoint(point);
                cross[k].position.x = point.x;
                cross[k].position.y = point.y;
                cross[k].position.z = point.z;
            }
            ((RenderTarget*)renderContext)->DrawPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)cross, 6, 0);
        }
    }
    ((RenderTarget*)renderContext)->SetTextureBlend(7, 0);
}

// 0x004433f0
void D3DIMSoultreeObject::GetPart(int surface, SoultreeVertex** vertices,
                                                int* vertexCount, unsigned short** indices, int* indexCount,
                                                int unused, int lod)
{
    if (lod == -1)
        lod = currentLod;
    if (subtreeDirty || lod != currentLod)
        TransformVertexGroups(lod);
    *vertices = lodTable[lod].surfaces[surface].vertices;
    *vertexCount = lodTable[lod].surfaces[surface].vertexCount;
    *indexCount = lodTable[lod].surfaces[surface].faceCount * 3;
    *indices = lodTable[lod].surfaces[surface].indices;
}

// 0x00443490
int D3DIMSoultreeObject::Tick(float frameTime)
{
    lastFrameTime = frameTime;
    return GameObject::Tick(frameTime);
}

// 0x004434b0
void D3DIMSoultreeObject::ComputeSubtreeBounds()
{
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
    ((RenderTarget*)renderContext)->SetTextureBlend(7, 0);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    Vector3 center;
    Vector3 extents;
    if (!parent) {
        GetSubtreeBounds(reinterpret_cast<Vec3*>(&center), reinterpret_cast<Vec3*>(&extents));
        DrawBoxOutline(center, extents, 1);
    }
    GetLocalBounds(reinterpret_cast<Vec3*>(&center), reinterpret_cast<Vec3*>(&extents));
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
    ((RenderTarget*)renderContext)->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
    ((RenderTarget*)renderContext)->SetTextureBlend(7, 0);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)renderContext)->SetTextureBlend(2, 0);
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
    SoultreeVertex vertices[4];
    for (int i = 0; i < 4; i++) {
        points[i] = LocalToWorldPoint(points[i]);
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
    ((RenderTarget*)renderContext)->DrawIndexedPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)vertices, 4, (int)indices, 6, 0);
    ((RenderTarget*)renderContext)->SetTextureBlend(7, 0);
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
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    ((RenderTarget*)renderContext)->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    ((RenderTarget*)renderContext)->SetTextureBlend(2, 0);
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
    SoultreeVertex vertices[8];
    for (int i = 0; i < 8; i++) {
        corners[i].x = center.x + corners[i].x;
        corners[i].y = center.y + corners[i].y;
        corners[i].z = center.z + corners[i].z;
        corners[i] = LocalToWorldPoint(corners[i]);
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
    ((RenderTarget*)renderContext)->DrawIndexedPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, (int)vertices, 8, (int)indices, 24, 0);
    ((RenderTarget*)renderContext)->SetTextureBlend(7, 0);
}

// 0x004439c0
void D3DIMSoultreeObject::DrawScreenRect(SoultreeScreenRect* rect)
{
    DDSurfaceDesc2 desc;
    memset(&desc, 0, sizeof(desc));
    desc.size = sizeof(desc);
    ((PCRenderTarget*)renderContext)->renderSurface->Lock(0, &desc, 1, 0);
    D3DIMSoultreeCameraView* camera = (D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera;
    char* bits = (char*)desc.surface + camera->viewportY * desc.pitch + camera->viewportX;
    int i;
    for (i = rect->left; i < rect->right; i++) {
        bits[rect->top * desc.pitch + i * 2] = 0;
        bits[rect->bottom * desc.pitch + i * 2] = 0;
    }
    for (i = rect->top; i < rect->bottom; i++) {
        bits[i * desc.pitch + rect->right * 2] = 0;
        bits[i * desc.pitch + rect->left * 2] = 0;
    }
    ((PCRenderTarget*)renderContext)->renderSurface->Unlock(0);
}

// 0x00443aa0. Not exact (PREPSCORE of 832 bytes: row 1 of the inline view
// matrix is bracketed in retail's addition order, which VC6 schedules
// differently; docs/D3DIMSOULTREE.md forms tried).
int D3DIMSoultreeObject::PrepareGeometry()
{
    visible = 0;
    if (!hidden && drawsItself) {
        if (subtreeDirty)
            UpdateSubtreeBounds();
        Matrix4 world;
        ((DrawSoultreeObjectView*)this)->GetMatrixIn(0, &world);
        Vector3 center;
        Vector3 extents;
        GetSubtreeBounds(reinterpret_cast<Vec3*>(&center), reinterpret_cast<Vec3*>(&extents));
        const Matrix4& projection = ((D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->viewMatrix;
        SoultreeD3DMatrix view;
        view._11 = world.m[0][0] * projection.m[0][0] + world.m[0][1] * projection.m[1][0] + world.m[0][2] * projection.m[2][0] + world.m[0][3] * projection.m[3][0];
        view._12 = world.m[0][0] * projection.m[0][1] + world.m[0][1] * projection.m[1][1] + world.m[0][2] * projection.m[2][1] + world.m[0][3] * projection.m[3][1];
        view._13 = world.m[0][0] * projection.m[0][2] + world.m[0][1] * projection.m[1][2] + world.m[0][2] * projection.m[2][2] + world.m[0][3] * projection.m[3][2];
        view._14 = world.m[0][0] * projection.m[0][3] + world.m[0][1] * projection.m[1][3] + world.m[0][2] * projection.m[2][3] + world.m[0][3] * projection.m[3][3];
        // Row 1 sums in retail's order (0x00443bac..0x00443c3a): the grouping shows at 24-bit
        // precision, so _21, _23 and _24 are bracketed as retail adds them.
        view._21 = ((world.m[1][2] * projection.m[2][0] + world.m[1][3] * projection.m[3][0]) + world.m[1][0] * projection.m[0][0]) + world.m[1][1] * projection.m[1][0];
        view._22 = world.m[1][0] * projection.m[0][1] + world.m[1][1] * projection.m[1][1] + world.m[1][2] * projection.m[2][1] + world.m[1][3] * projection.m[3][1];
        view._23 = ((world.m[1][0] * projection.m[0][2] + world.m[1][1] * projection.m[1][2]) + world.m[1][3] * projection.m[3][2]) + world.m[1][2] * projection.m[2][2];
        view._24 = ((world.m[1][2] * projection.m[2][3] + world.m[1][3] * projection.m[3][3]) + world.m[1][1] * projection.m[1][3]) + world.m[1][0] * projection.m[0][3];
        view._31 = world.m[2][0] * projection.m[0][0] + world.m[2][1] * projection.m[1][0] + world.m[2][2] * projection.m[2][0] + world.m[2][3] * projection.m[3][0];
        view._32 = world.m[2][0] * projection.m[0][1] + world.m[2][1] * projection.m[1][1] + world.m[2][2] * projection.m[2][1] + world.m[2][3] * projection.m[3][1];
        view._33 = world.m[2][0] * projection.m[0][2] + world.m[2][1] * projection.m[1][2] + world.m[2][2] * projection.m[2][2] + world.m[2][3] * projection.m[3][2];
        view._34 = world.m[2][0] * projection.m[0][3] + world.m[2][1] * projection.m[1][3] + world.m[2][2] * projection.m[2][3] + world.m[2][3] * projection.m[3][3];
        view._41 = world.m[3][0] * projection.m[0][0] + world.m[3][1] * projection.m[1][0] + world.m[3][2] * projection.m[2][0] + world.m[3][3] * projection.m[3][0];
        view._42 = world.m[3][0] * projection.m[0][1] + world.m[3][1] * projection.m[1][1] + world.m[3][2] * projection.m[2][1] + world.m[3][3] * projection.m[3][1];
        view._43 = world.m[3][0] * projection.m[0][2] + world.m[3][1] * projection.m[1][2] + world.m[3][2] * projection.m[2][2] + world.m[3][3] * projection.m[3][2];
        view._44 = world.m[3][0] * projection.m[0][3] + world.m[3][1] * projection.m[1][3] + world.m[3][2] * projection.m[2][3] + world.m[3][3] * projection.m[3][3];
        if (g_visibilityClipper->TestBox((VisibilityCamera*)((RenderTarget*)renderContext)->currentCamera,
                                         view.m[0], &center.x, &extents.x, (int*)&screenRect, 0,
                                         &field_0x260)) {
            visible = 1;
            SelectLod(&center, &extents);
        }
    }
    return GameObject::PrepareGeometry();
}

// 0x00460b50 (cdecl): table-driven square root (FollowCamera.h and
// Wrecker.h declare it too).
float FastSqrt(float value);

static inline float Magnitude(const Vector3& v)
{
    float squared = v.z * v.z + (v.x * v.x + v.y * v.y);
    if (squared == 1.0f)
        return 1.0f;
    return FastSqrt(squared);
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
        GetSubtreeBounds(reinterpret_cast<Vec3*>(center), reinterpret_cast<Vec3*>(extents));
    }
    float distance = Magnitude(LocalToWorldPoint(*center) -
                               ((D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->position);
    float range = distance - Magnitude(*extents);
    if (0.1f > range)
        range = 0.1f;
    float scale = ((D3DIMSoultreeCameraView*)((RenderTarget*)renderContext)->currentCamera)->imagePlaneDistance / range;
    if (useAutoLod) {
        int lod = 0;
        for (int i = 0; i < lodCount - 1; i++) {
            if ((scale * autoLodDistances[i]) * inverseLodThreshold < 1.0f)
                lod = i + 1;
        }
        if (currentLod != lod)
            SetLod(lod);
    }
    for (int i = 0; i < materialCount; i++) {
        SoultreeMaterial* material = materialTable[i];
        if (material->surfaceMap) {
            ManagedTexture* texture = (ManagedTexture*)material->surfaceMap->texture;
            if (texture->mapFlags & 1) {
                float width = (float)texture->textureWidth;
                float size = (scale * *(float*)&material->texelRatio) * width;
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
int D3DIMSoultreeObject::Render3D()
{
    if (!visible)
        return 1;
    DrawCurrentLod();
    if (showBounds) {
        DrawScreenRect(&screenRect);
        ComputeSubtreeBounds();
        DrawAxisGizmo();
    }
    GameObject::Render3D();
    screenRect.right++;
    screenRect.bottom++;
    return 1;
}

// 0x004440a0
void D3DIMSoultreeObject::CreateNode(SoultreeObject** out)
{
    D3DIMSoultreeObject* copy = new (__FILE__, 0x7d5) D3DIMSoultreeObject(isEnabled);
    *out = copy;
    copy->LoadModel(renderContext, "", lightManager, &textureOptions, 1);
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

// 0x00444140: the local bounds of the vertices this node owns in every
// level of detail (zero when it owns none). The centre sum is written per
// component: through operator+ VC6 adds y and z high-first, retail
// low-first; the valid flag is stored last.
void D3DIMSoultreeObject::UpdateLocalBounds()
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
                    SoultreeVertex* vertex = model->lodTable[lod].surfaces[i].groups[j].localVertices;
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
    reinterpret_cast<Vector3&>(localBoundsCenter) = Vector3(low.x + high.x, low.y + high.y, low.z + high.z) * 0.5f;
    reinterpret_cast<Vector3&>(localBoundsHalfExtent) = (high - low) * 0.5f;
    localBoundsValid = 1;
}

// 0x00444440
void D3DIMSoultreeObject::ComputeAllBounds()
{
    int count = CountNodes();
    SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x80b);
    int index = 1;
    nodes[0] = this;
    CollectDescendants(&index, nodes);
    for (int i = 0; i < count; i++)
        nodes[i]->UpdateLocalBounds();
    DebugFree(nodes, __FILE__, 0x812);
    UpdateSubtreeBounds();
}

// 0x004444c0
void D3DIMSoultreeObject::SetUseLighting(int value)
{
    subtreeDirty = 1;
    useLighting = value;
}

// 0x004444e0
void D3DIMSoultreeObject::ClearScreenRects()
{
    screenRect.left = 0;
    screenRect.right = 0;
    screenRect.top = 0;
    screenRect.bottom = 0;
    field_0x2a8.left = 0;
    field_0x2a8.right = 0;
    field_0x2a8.top = 0;
    field_0x2a8.bottom = 0;
    SetFlag14c();
}

// 0x00444520
void D3DIMSoultreeObject::Enable()
{
    GameObject::Enable();
    ClearFlag14c();
}

// 0x00444540
void D3DIMSoultreeObject::Disable()
{
    GameObject::Disable();
    ClearScreenRects();
}

// 0x00444560: copies the source node. The node search reuses the
// CollectDescendants counter `index`; `to` is declared before `from`, and
// `d` before `s`, which gives retail's register rotation (source ebx,
// `from` edi, `to` ebp).
void D3DIMSoultreeObject::CloneFrom(SoultreeObject* sourceNode)
{
    D3DIMSoultreeObject* source = (D3DIMSoultreeObject*)sourceNode;
    GameObject::SetRenderContext(source->renderContext);
    textureFormat = source->textureFormat;
    textureManager = source->textureManager;
    lightManager = source->lightManager;
    SoultreeObject::CloneFrom(source);
    CopyMaterials(source);
    lodCount = source->lodCount;
    currentLod = source->currentLod;
    lowestLod = source->lowestLod;
    useAutoLod = source->useAutoLod;
    if (useAutoLod) {
        autoLodDistances = (float*)DebugMalloc(lodCount * 4, __FILE__, 0x88e);
        for (int i = 0; i < lodCount; i++)
            autoLodDistances[i] = source->autoLodDistances[i];
    }
    if (source->lodTable) {
        int count = CountNodes();
        SoultreeObject** nodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x897);
        int index = 1;
        nodes[0] = this;
        CollectDescendants(&index, nodes);
        SoultreeObject** sourceNodes = (SoultreeObject**)DebugMalloc(count * 4, __FILE__, 0x89d);
        index = 1;
        sourceNodes[0] = source;
        source->CollectDescendants(&index, sourceNodes);
        lodTable = (SoultreeLod*)DebugMalloc(lodCount * 8, __FILE__, 0x8a2);
        for (int lod = 0; lod < lodCount; lod++) {
            SoultreeLod* to = &lodTable[lod];
            SoultreeLod* from = &source->lodTable[lod];
            to->surfaceCount = from->surfaceCount;
            to->surfaces = (SoultreeSurface*)DebugMalloc(to->surfaceCount * 0x38, __FILE__, 0x8aa);
            for (int i = 0; i < to->surfaceCount; i++) {
                SoultreeSurface* toSurface = &to->surfaces[i];
                SoultreeSurface* fromSurface = &from->surfaces[i];
                toSurface->vertexCount = fromSurface->vertexCount;
                toSurface->faceCount = fromSurface->faceCount;
                toSurface->materialCount = fromSurface->materialCount;
                toSurface->materialIndices = (int*)DebugMalloc(toSurface->materialCount * 4, __FILE__, 0x8b3);
                int k;
                for (k = 0; k < toSurface->materialCount; k++)
                    toSurface->materialIndices[k] = fromSurface->materialIndices[k];
                toSurface->mappingScale = fromSurface->mappingScale;
                toSurface->mappingOffsetU = fromSurface->mappingOffsetU;
                toSurface->mappingOffsetV = fromSurface->mappingOffsetV;
                toSurface->vertices =
                    (SoultreeVertex*)DebugMalloc(toSurface->vertexCount * 0x20, __FILE__, 0x8bd);
                toSurface->localVertices =
                    (SoultreeVertex*)DebugMalloc(toSurface->vertexCount * 0x20, __FILE__, 0x8be);
                toSurface->normals = (Vector3*)DebugMalloc(toSurface->vertexCount * 0xc, __FILE__, 0x8bf);
                toSurface->indices = (unsigned short*)DebugMalloc(toSurface->faceCount * 6, __FILE__, 0x8c0);
                toSurface->uvs = (SoultreeUV*)DebugMalloc(toSurface->vertexCount * 8, __FILE__, 0x8c1);
                memcpy(toSurface->vertices, fromSurface->vertices, toSurface->vertexCount * 0x20);
                memcpy(toSurface->localVertices, fromSurface->localVertices, toSurface->vertexCount * 0x20);
                memcpy(toSurface->normals, fromSurface->normals, toSurface->vertexCount * 0xc);
                memcpy(toSurface->indices, fromSurface->indices, toSurface->faceCount * 6);
                memcpy(toSurface->uvs, fromSurface->uvs, toSurface->vertexCount * 8);
                toSurface->groupCount = fromSurface->groupCount;
                toSurface->groups =
                    (SoultreeVertexGroup*)DebugMalloc(toSurface->groupCount * 0x14, __FILE__, 0x8cb);
                int first = 0;
                for (int j = 0; j < toSurface->groupCount; j++) {
                    toSurface->groups[j].node = 0;
                    for (index = 0; !toSurface->groups[j].node; index++) {
                        if (sourceNodes[index] == fromSurface->groups[j].node)
                            toSurface->groups[j].node = nodes[index];
                    }
                    toSurface->groups[j].vertexCount = fromSurface->groups[j].vertexCount;
                    toSurface->groups[j].vertices = &toSurface->vertices[first];
                    toSurface->groups[j].localVertices = &toSurface->localVertices[first];
                    toSurface->groups[j].normals = &toSurface->normals[first];
                    first += toSurface->groups[j].vertexCount;
                }
            }
        }
        DebugFree(nodes, __FILE__, 0x8e1);
        DebugFree(sourceNodes, __FILE__, 0x8e2);
    } else {
        lodTable = 0;
    }
    ComputeAllBounds();
}

// 0x00444a40
void D3DIMSoultreeObject::CopyMaterials(D3DIMSoultreeObject* source)
{
    materialCount = source->materialCount;
    materialTable = (SoultreeMaterial**)DebugMalloc(materialCount * 4, __FILE__, 0x8ed);
    for (int i = 0; i < materialCount; i++) {
        materialTable[i] = new (__FILE__, 0x8ef) SoultreeMaterial(1);
        materialTable[i]->Attach((RenderTarget*)renderContext, &textureOptions,
                                              textureManager, textureFormat);
        AppendChild(materialTable[i], -1);
        materialTable[i]->CopyFrom(source->materialTable[i]);
    }
}

// 0x00444b60
void D3DIMSoultreeObject::CopySubtree(SoultreeObject* sourceNode, SoultreeObject* targetNode)
{
    SoultreeObject::CopySubtree(sourceNode, targetNode);
    D3DIMSoultreeObject* source = (D3DIMSoultreeObject*)sourceNode;
    D3DIMSoultreeObject* target = (D3DIMSoultreeObject*)targetNode;
    target->subtreeDirty = 1;
    target->useLighting = source->useLighting;
    target->showNormals = source->showNormals;
    target->showVertexCrosses = source->showVertexCrosses;
    target->lightManager = source->lightManager;
    int nameLength = strlen(source->modelName);
    int length = nameLength > 0x7f ? 0x7f : nameLength;
    strncpy(target->modelName, source->modelName, length);
    target->modelName[length] = 0;
    target->lightManager = source->lightManager;
    target->textureManager = source->textureManager;
    target->textureFormat = source->textureFormat;
    target->textureOptions = source->textureOptions;
    target->screenRect = source->screenRect;
    target->field_0x2a8 = source->field_0x2a8;
}

// 0x00444c70
int D3DIMSoultreeObject::ReplaceMaterialTexture(int index, const char* name, TextureContext* textures)
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
void D3DIMSoultreeObject::SetLod(int lod)
{
    if (lod < lowestLod)
        lod = lowestLod;
    if (lod >= lodCount)
        lod = lodCount - 1;
    if (currentLod != lod) {
        subtreeDirty = 1;
        currentLod = lod;
    }
}

// 0x00444d40
void D3DIMSoultreeObject::SetLowestLod(int lod)
{
    if (lod < 0)
        lod = 0;
    if (lod >= lodCount)
        lod = lodCount - 1;
    lowestLod = lod;
}

// 0x00444d60
void D3DIMSoultreeObject::SetLodThreshold(float value)
{
    inverseLodThreshold = 1.0f / value;
}

// 0x00444d80
void D3DIMSoultreeObject::AddModifier(GameObject* modifier)
{
    modifierList = (GameObject**)DebugRealloc(modifierList, modifierCount * 4 + 4, __FILE__, 0x958);
    modifierList[modifierCount] = modifier;
    modifierCount++;
    ((D3DIMSoultreeModifier*)modifier)->AddObject(this);
}

// 0x00444de0
void D3DIMSoultreeObject::RemoveModifier(GameObject* modifier)
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
void D3DIMSoultreeObject::RemoveAllModifiers()
{
    while (modifierCount)
        RemoveModifier(modifierList[0]);
}

// 0x00444eb0: appends to the second list (+0x270, count +0x26c; D3DIMSoulTree.CPP line 0x97f)
void D3DIMSoultreeObject::AddSecondModifier(GameObject* modifier)
{
    secondModifierList = (GameObject**)DebugRealloc(secondModifierList, secondModifierCount * 4 + 4, __FILE__, 0x97f);
    secondModifierList[secondModifierCount] = modifier;
    secondModifierCount++;
    ((D3DIMSoultreeModifier*)modifier)->AddObject(this);
}

// 0x00444f10
void D3DIMSoultreeObject::RemoveSecondModifier(GameObject* modifier)
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
void D3DIMSoultreeObject::RemoveAllSecondModifiers()
{
    while (secondModifierCount)
        RemoveSecondModifier(secondModifierList[0]);
}

// 0x00444fe0
int D3DIMSoultreeObject::GetVertexCount()
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
        lod = currentLod;
    if (lodTable)
        return lodTable[lod].surfaceCount;
    return 0;
}

// 0x00445060
int D3DIMSoultreeObject::NodeMovesVertices(SoultreeObject* node, int lod)
{
    for (int i = 0; i < lodTable[lod].surfaceCount; i++) {
        SoultreeSurface* surface = &lodTable[lod].surfaces[i];
        for (int j = 0; j < surface->groupCount; j++) {
            if (surface->groups[j].node == node && surface->groups[j].vertexCount > 0)
                return 1;
        }
    }
    return 0;
}

// 0x004450c0
void D3DIMSoultreeObject::ResetLocalMatrices()
{
    int saved = currentLod;
    for (int lod = 0; lod < lodCount; lod++) {
        currentLod = lod;
        TransformVertexGroups(-1);
        for (int i = 0; i < lodTable[lod].surfaceCount; i++) {
            for (int j = 0; j < lodTable[lod].surfaces[i].vertexCount; j++) {
                lodTable[lod].surfaces[i].localVertices[j].position.x =
                    lodTable[lod].surfaces[i].vertices[j].position.x;
                lodTable[lod].surfaces[i].localVertices[j].position.y =
                    lodTable[lod].surfaces[i].vertices[j].position.y;
                lodTable[lod].surfaces[i].localVertices[j].position.z =
                    lodTable[lod].surfaces[i].vertices[j].position.z;
            }
        }
    }
    currentLod = saved;
    SoultreeObject::ResetLocalMatrices();
    if (!parent)
        ComputeAllBounds();
}

// 0x004451e0
void D3DIMSoultreeObject::ApplyLodSettings(int index)
{
    SetLodThreshold(g_ActiveLodSettings[index].lodThreshold);
    texturePerspective = g_ActiveLodSettings[index].texturePerspective;
    gouraudShading = g_ActiveLodSettings[index].gouraudShading;
    SetUseLighting(g_ActiveLodSettings[index].useLighting);
}

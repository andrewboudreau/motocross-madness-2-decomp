// EcoSystem.cpp -- D:\aardvark\VC\krusty2\EcoSystem.cpp, 0x00455da0..0x0045c830.
// See EcoSystem.h for the class evidence and docs/ECOSYSTEM.md for the
// match state. Line numbers in the allocation calls are the retail
// __LINE__ values.

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "EcoSystem.h"

#include "MemTag.h"
#include "Parameterblocks.h"
#include "PeakHold.h"
#include "TypeRegistry.h"
#include "UnknownResourceManager.h"
#include "bmpfile.h"
#include "D3DConstants.h"

// The four vector constants that open about 73 retail files
// (0x0059aea0, 0x0059aeb0, 0x0059aed8, 0x0059ae90; $E 0x00459830..0x00459b3b).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

EcoSystem* g_UnknownGlobal59aebc;             // 0x0059aebc
static int g_UnknownGlobal59aec0;             // 0x0059aec0: billboard draw time
static UnknownPeakHold g_UnknownGlobal59aef0(5000); // 0x0059aef0 ($E 0x00455db0, XCU 103)
static UnknownPeakHold g_UnknownGlobal59aec8(5000); // 0x0059aec8 ($E 0x00455dd0, XCU 104)
static int g_UnknownGlobal59aee4;             // 0x0059aee4: geometry draw time
static int g_UnknownGlobal59aee8;             // 0x0059aee8: classification time
static int g_UnknownGlobal59aefc;             // 0x0059aefc: textures were preloaded
static Matrix4* g_UnknownGlobal59af00;        // 0x0059af00: the view matrix
static int g_UnknownGlobal59af04;             // 0x0059af04: geometry objects drawn
static int g_UnknownGlobal59af08;             // 0x0059af08: billboards drawn
static int g_UnknownGlobal59af0c;             // 0x0059af0c: geometry blocks alive
static int g_UnknownGlobal59af10;             // 0x0059af10: textures preloaded
static UnknownEcoDetailBand* g_UnknownGlobal59af14; // 0x0059af14: the detail band table

// The detail bands, by level: 0x0056a600 for the 1555 (software) path,
// 0x0056a740 for 4444.
static UnknownEcoDetailBand g_UnknownGlobal56a600[10] = {
    {0, 0, 1555, 256, 0, 0, 0, 0},   {0, 0, 1555, 256, 0, 0, 0, 0},   {0, 0, 1555, 256, 0, 0, 0, 0},
    {10, 10, 1555, 384, 0, 0, 0, 0}, {20, 20, 1555, 384, 0, 1, 0, 0}, {20, 20, 1555, 448, 0, 1, 1, 0},
    {30, 30, 1555, 512, 0, 1, 1, 0}, {40, 40, 1555, 512, 0, 1, 1, 0}, {75, 75, 1555, 640, 1, 1, 1, 0},
    {150, 75, 1555, 768, 1, 1, 1, 1},
};
static UnknownEcoDetailBand g_UnknownGlobal56a740[10] = {
    {20, 0, 4444, 256, 0, 1, 1, 1},   {35, 5, 4444, 256, 0, 1, 1, 1},   {45, 10, 4444, 320, 0, 1, 1, 1},
    {60, 20, 4444, 384, 0, 1, 1, 1},  {75, 30, 4444, 448, 1, 1, 1, 1},  {90, 40, 4444, 512, 1, 1, 1, 1},
    {105, 50, 4444, 576, 1, 1, 1, 1}, {120, 60, 4444, 640, 1, 1, 1, 1}, {135, 67, 4444, 768, 1, 1, 1, 1},
    {150, 75, 4444, 768, 1, 1, 1, 1},
};

static int g_UnknownGlobal56a128 = -1;        // 0x0056a128
static int g_UnknownGlobal56a12c = 1;         // 0x0056a12c: drawing enabled (slot 23 toggles it)

// Inline helper (shape only; the retail header is not attested): the
// squared length, summed as z + (x + y) (the only order VC6 compiles to
// the retail load sequence).
static inline float UnknownSquareMagnitude(const Vector3* v) {
    return v->z * v->z + (v->x * v->x + v->y * v->y);
}

// The view (RenderTarget.h) the object was attached to, GameObject+0x18.

#define ECO_VIEW ((UnknownEcoRenderTarget*)field_0x18)

#define ECO_RGBA(r, g, b, a) ((unsigned int)(((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

// 0x00455de0
UnknownEcoDefinition::UnknownEcoDefinition() {
    name[0] = 0;
    billboardName[0] = 0;
    probabilityTga[0] = 0;
    meanHeight = 0.0f;
    minHeight = 0.0f;
    maxHeight = 0.0f;
    meanRadius = 0.0f;
    minRadius = 0.0f;
    maxRadius = 0.0f;
    meanSlope = 0.0f;
    standardDeviationSlope = 0.0f;
    meanAspect = 0.0f;
    standardDeviationAspect = 0.0f;
    meanDrainage = 0.0f;
    standardDeviationDrainage = 0.0f;
    meanAltitude = 0.0f;
    standardDeviationAltitude = 0.0f;
    uLeft = 0.0f;
    uRight = 1.0f;
    uCenter = 1.0f;
    vBottom = 0.0f;
    vTop = 0.5f;
    collisionCount = 0;
    collisionObjects = 0;
    collisionDefinitions = 0;
}

// 0x00455e80
UnknownEcoDefinition::~UnknownEcoDefinition() {
    int i;
    if (billboardTexture) {
        billboardTexture->UnknownVirtualSlot2();
        if (g_UnknownGlobal59aefc)
            billboardTexture->UnknownVirtualSlot2();
    }
    if (modelTexture) {
        modelTexture->UnknownVirtualSlot2();
        if (g_UnknownGlobal59aefc)
            modelTexture->UnknownVirtualSlot2();
    }
    for (i = 0; i < lodCount; i++) {
        if (modelVertices[i])
            DebugFree(modelVertices[i], __FILE__, 0x66);
    }
    if (collisionDefinitions)
        DebugFree(collisionDefinitions, __FILE__, 0x6a);
    if (collisionObjects)
        DebugFree(collisionObjects, __FILE__, 0x6e);
    g_UnknownGlobal59af10 = 0;
}

// 0x00455f50
int UnknownEcoDefinition::RandomParameter() {
    return rand() >> 8;
}

// 0x00455f60
int UnknownEcoDefinition::ParameterForHeight(float height) {
    return (int)(rand() * height / maxHeight) >> 8;
}

// 0x00455f90
float UnknownEcoDefinition::HeightForParameter(unsigned char parameter) {
    if (parameter < 0x80)
        return meanHeight - (meanHeight - minHeight) * parameter * (1.0f / 128.0f);
    return meanHeight + (maxHeight - meanHeight) * (255 - parameter) * (1.0f / 128.0f);
}

// 0x00455ff0
float UnknownEcoDefinition::RadiusForParameter(unsigned char parameter) {
    if (parameter < 0x80)
        return meanRadius - (meanRadius - minRadius) * parameter * (1.0f / 128.0f);
    return meanRadius + (maxRadius - meanRadius) * (255 - parameter) * (1.0f / 128.0f);
}

// 0x00456650
Vegetation::Vegetation() {
    quantizedPosition.x = 0;
    quantizedPosition.y = 0;
    quantizedPosition.z = 0;
    heightParam = 0;
    radiusParam = 0;
    definitionIndex = 0;
    geometryBlock = 0;
    field_0x08 = g_UnknownGlobal59aebc->vegetationTypeId;
}

// 0x00456690
unsigned short Vegetation::UnknownVirtualSlot0() {
    Matrix4* m = g_UnknownGlobal59af00;
    float depth = ((quantizedPosition.z * (*m)(2, 2) + quantizedPosition.x * (*m)(0, 2)) * g_UnknownGlobal59aebc->unitsPerCoordinate
                   + (*m)(3, 2)) * g_UnknownGlobal59aebc->depthScale;
    if (depth < 0.0)
        field_0x06 = 0;
    else if ((int)depth >= 0xffff)
        field_0x06 = (short)0xffff;
    else
        field_0x06 = (short)(int)depth;
    return field_0x06;
}

// 0x00456720
unsigned int Vegetation::UnknownVirtualSlot1(UnknownEcoQuadTree* tree) {
    float radius = g_UnknownGlobal59aebc->definitionTable[definitionIndex]->RadiusForParameter(radiusParam);
    float x = quantizedPosition.x * g_UnknownGlobal59aebc->unitsPerCoordinate;
    float z = quantizedPosition.z * g_UnknownGlobal59aebc->unitsPerCoordinate;
    return tree->ComputeCode(x - radius, z - radius, x + radius, z + radius);
}

// 0x004567a0
void Vegetation::PlaceQuantized(TextureMapManager* textures, unsigned char definition,
                                       const UnknownEcoCoordinates* coordinates, unsigned char heightParameter,
                                       unsigned char radiusParameter) {
    definitionIndex = definition;
    quantizedPosition = *coordinates;
    radiusParam = radiusParameter;
    heightParam = heightParameter;
    isBillboard = 1;
}

// By-value component reads. Retail's 0x004567e0 multiplies with the
// position component loaded first (`fld [position]; fmul [scale]`); VC6 SP3
// produces that order only when the component is read through a by-value
// accessor like these. A plain `position->x` operand is loaded second
// whichever way the product is written.
static inline float VectorX(const Vector3& v) { return v.x; }
static inline float VectorY(const Vector3& v) { return v.y; }
static inline float VectorZ(const Vector3& v) { return v.z; }

// 0x004567e0: places the object at the world position `position`.
void Vegetation::Place(TextureMapManager* textures, unsigned char definition,
                                       const Vector3* position, unsigned char heightParameter,
                                       unsigned char radiusParameter) {
    definitionIndex = definition;
    quantizedPosition.x = (unsigned short)(int)(g_UnknownGlobal59aebc->coordinatesPerUnit * VectorX(*position));
    quantizedPosition.y = (unsigned short)(int)(g_UnknownGlobal59aebc->coordinatesPerUnit * VectorY(*position));
    quantizedPosition.z = (unsigned short)(int)(g_UnknownGlobal59aebc->coordinatesPerUnit * VectorZ(*position));
    radiusParam = radiusParameter;
    heightParam = heightParameter;
    isBillboard = 1;
}

// 0x00456850
int EvictGeometry(void* owner, int context) {
    Vegetation* object = (Vegetation*)owner;
    DebugFree(object->geometryBlock, __FILE__, 0x1a1);
    object->geometryBlock = 0;
    object->isBillboard = 1;
    return 1;
}

// 0x00456a10: builds the lit geometry for `billboard` (a LOD index, or the
// billboard when it is past the LOD count) at the quantized position, rotated
// to face the camera's horizontal direction. The vertex block (vertices, the
// 4-byte-padded indices, then the AgeEntry) is registered with the
// AgeManager for eviction. The min/max of x are computed but unused.
void Vegetation::SetBillboard(int billboard, int fade) {
    UnknownEcoDefinition* definition = g_UnknownGlobal59aebc->definitionTable[definitionIndex];
    UnknownEcoCamera* camera = ((UnknownEcoRenderTarget*)g_UnknownGlobal59aebc->field_0x18)->field_0x08;
    fadeLevel = (unsigned char)fade;
    if (billboard == isBillboard)
        return;
    if (isBillboard < definition->lodCount && geometryBlock)
        g_UnknownGlobal59af0c--;
    float maxX = -FLT_MAX;
    float minX = FLT_MAX;
    if (billboard < definition->lodCount && !geometryBlock) {
        int vertexCount = definition->modelVertexCount[billboard];
        int indexCount = definition->modelIndexCount[billboard];
        int dwords = vertexCount * 8 + (indexCount + 1) / 2;
        geometryBlock = DebugMalloc(dwords * 4 + sizeof(AgeEntry), __FILE__, 0x1fb);
        AgeEntry* entry = (AgeEntry*)((int*)geometryBlock + dwords);
        g_UnknownGlobal59aebc->ageManager->Register(entry, EvictGeometry, this, 0,
                                                                  dwords * 4 + sizeof(AgeEntry));
        g_UnknownGlobal59af0c++;
        memcpy((UnknownEcoVertex*)geometryBlock + vertexCount, definition->modelIndices[billboard], indexCount * 2);
        float radiusScale = definition->RadiusForParameter(radiusParam) * definition->modelRadiusScale;
        float heightScale = definition->HeightForParameter(heightParam) * definition->modelHeightScale;
        float c;
        float s;
        if (camera->field_0x17c.z != 0.0f) {
            float inverse = UnknownFunction460c70(camera->field_0x17c.x * camera->field_0x17c.x
                                                  + camera->field_0x17c.z * camera->field_0x17c.z) * radiusScale;
            c = inverse * camera->field_0x17c.z;
            s = -(inverse * camera->field_0x17c.x);
        } else {
            c = radiusScale;
            s = 0.0f;
        }
        // Six scalars, not two Vector3s: retail interleaves the colour and
        // ambient components with the other scalar slots of the frame.
        float colorR = g_UnknownGlobal59aebc->lightColor.x;
        float colorG = g_UnknownGlobal59aebc->lightColor.y;
        float colorB = g_UnknownGlobal59aebc->lightColor.z;
        float ambientR = g_UnknownGlobal59aebc->ambientLight.x;
        float ambientG = g_UnknownGlobal59aebc->ambientLight.y;
        float ambientB = g_UnknownGlobal59aebc->ambientLight.z;
        unsigned int ambientColor = ECO_RGBA((int)(ambientR * 255.0f), (int)(ambientG * 255.0f),
                                             (int)(ambientB * 255.0f), 255);
        UnknownEcoModelVertex* source = definition->modelVertices[billboard];
        int i;
        // Retail indexes the output vertex off `geometryBlock` and re-reads
        // the member before every store (the void* member aliases the
        // stores); a walking `UnknownEcoVertex*` local compiles a different
        // loop with a 0x4c frame.
#define VERTEX ((UnknownEcoVertex*)geometryBlock)[i]
        for (i = 0; i < vertexCount; i++) {
            int unlit = 0;
            Vector3 normal;
            float x = quantizedPosition.x;
            VERTEX.position.x = c * source->position.x - s * source->position.z + x * g_UnknownGlobal59aebc->unitsPerCoordinate;
            VERTEX.position.y = quantizedPosition.y * g_UnknownGlobal59aebc->unitsPerCoordinate + heightScale * source->position.y;
            VERTEX.position.z = quantizedPosition.z * g_UnknownGlobal59aebc->unitsPerCoordinate + c * source->position.z
                                + s * source->position.x;
            if (definition->usePlanarLighting) {
                if (source->normal.y < 0.0f) {
                    unlit = 1;
                } else {
                    // The planar normal is the rotated model position (its
                    // radial direction), not the model normal; the explicit
                    // `unlit = 0` here reproduces retail's branch bodies.
                    float length;
                    unlit = 0;
                    normal.x = c * source->position.x - s * source->position.z;
                    normal.z = c * source->position.z + s * source->position.x;
                    length = normal.x * normal.x;
                    length += normal.z * normal.z;
                    if (length == 0.0f) {
                        normal = kVec3Zero;
                    } else {
                        length = FastInvSqrt(length);
                        normal.x = length * normal.x;
                        normal.y = 0.0f;
                        normal.z = length * normal.z;
                    }
                }
            } else {
                float length;
                normal.x = c * source->normal.x - s * source->normal.z;
                normal.y = source->normal.y;
                normal.z = s * source->normal.x + c * source->normal.z;
                length = UnknownSquareMagnitude(&normal);
                if (length == 0.0f) {
                    normal = kVec3Zero;
                } else {
                    length = FastInvSqrt(length);
                    normal.x = length * normal.x;
                    normal.y = length * normal.y;
                    normal.z = length * normal.z;
                }
            }
            if (VERTEX.position.x > maxX)
                maxX = VERTEX.position.x;
            if (VERTEX.position.x < minX)
                minX = VERTEX.position.x;
            float intensity = -(normal.z * g_UnknownGlobal59aebc->lightDirection.z
                                + (normal.x * g_UnknownGlobal59aebc->lightDirection.x
                                   + normal.y * g_UnknownGlobal59aebc->lightDirection.y));
            if (!unlit && source->normal.y > -0.95f && intensity > 0.0f) {
                float r = intensity * colorR + ambientR;
                float g = intensity * colorG + ambientG;
                float b = intensity * colorB + ambientB;
                if (r > 1.0f)
                    r = 1.0f;
                if (g > 1.0f)
                    g = 1.0f;
                if (b > 1.0f)
                    b = 1.0f;
                VERTEX.diffuse = ECO_RGBA((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), 255);
            } else {
                VERTEX.diffuse = ambientColor;
            }
            VERTEX.reserved = 0;
            VERTEX.specular = 0;
            VERTEX.tu = source->tu;
            VERTEX.tv = source->tv;
            source++;
        }
#undef VERTEX
    }
    isBillboard = billboard;
}

// 0x00457000
void Vegetation::DrawGeometry(UnknownEcoRenderTarget* target) {
    UnknownEcoDefinition* definition = g_UnknownGlobal59aebc->definitionTable[definitionIndex];
    definition->modelTexture->UnknownVirtualSlot19();
    int vertexCount = definition->modelVertexCount[isBillboard];
    int indexCount = definition->modelIndexCount[isBillboard];
    target->UnknownVirtualSlot15(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, geometryBlock, vertexCount, (UnknownEcoVertex*)geometryBlock + vertexCount,
                                 indexCount, 0);
    g_UnknownGlobal59aebc->ageManager->MarkUsed(
        (AgeEntry*)((int*)geometryBlock + vertexCount * 8 + (indexCount + 1) / 2));
}

// 0x00457080
int Vegetation::GetCollisionCount() {
    return g_UnknownGlobal59aebc->definitionTable[definitionIndex]->collisionCount;
}

// 0x00457230
float Vegetation::GetRadius() {
    return g_UnknownGlobal59aebc->definitionTable[definitionIndex]->RadiusForParameter(radiusParam);
}

// 0x00457250
EcoSystem::EcoSystem(int flags) : GameObject(flags) {
    totalObjects = 0;
    vegetation = 0;
    billboardList = 0;
    geometryList = 0;
    groundTerrain = 0;
    textureManager = 0;
    billboardVertices = 0;
    billboardIndices = 0;
    ageManager = 0;
    method = 0;
    placedCount = 0;
    billboardFormat = 555;
    memset(definitionTable, 0, sizeof(definitionTable));
    esbStream = 0;
    field_0x598 = 0xcd;
    g_UnknownGlobal59aebc = this;
    unitsPerCoordinate = 1.0f;
    coordinatesPerUnit = 1.0f;
}

// 0x00457330
EcoSystem::~EcoSystem() {
    int i;
    for (i = 0; i < 256; i++) {
        if (definitionTable[i])
            delete definitionTable[i];
    }
    if (vegetation) {
        for (i = 0; i < placedCount; i++) {
            if (vegetation[i].geometryBlock)
                DebugFree(vegetation[i].geometryBlock, __FILE__, 0x2cd);
        }
        delete vegetation;
    }
    if (billboardVertices)
        DebugFree(billboardVertices, __FILE__, 0x2d2);
    if (billboardIndices)
        DebugFree(billboardIndices, __FILE__, 0x2d3);
    if (billboardList)
        DebugFree(billboardList, __FILE__, 0x2d4);
    if (geometryList)
        DebugFree(geometryList, __FILE__, 0x2d5);
    if (ageManager)
        delete ageManager;
    g_UnknownGlobal59aebc = 0;
    g_UnknownGlobal59aefc = 0;
}

// 0x00458360: reads a definition's "CollisionObject%i" entries. The keys
// count from 1 over a zero-based loop (`i + 1`), and the profile defaults
// are the empty string (the shared "" literal 0x00577738), not "NONE".
void EcoSystem::ReadCollisionObjects(const char* path, int index, UnknownEcoDefinition* definition) {
    char key[0x80];
    char value[0x80];
    char section[0x100];
    char kind[0x80];
    int i;
    sprintf(section, "Vegetation_%d", index);
    int count = GetPrivateProfileInt(section, "NumCollisionObjects", 0, path);
    definition->collisionCount = count;
    if (count > 0) {
        definition->collisionDefinitions = (UnknownEcoCollisionDefinition*)DebugMalloc(
            count * sizeof(UnknownEcoCollisionDefinition), __FILE__, 0x40c);
        for (i = 0; i < definition->collisionCount; i++) {
            UnknownEcoCollisionDefinition* shape = &definition->collisionDefinitions[i];
            sprintf(key, "CollisionObject%i", i + 1);
            GetPrivateProfileString(section, key, "", kind, 0x80, path);
            if (!_stricmp(kind, "GEOMETRY")) {
                shape->type = 0;
            } else if (!_stricmp(kind, "SPHERE")) {
                shape->type = 2;
                sprintf(key, "CollisionObject%iCenter", i + 1);
                GetPrivateProfileString(section, key, "", value, 0x80, path);
                shape->start.x = (float)atof(strtok(value, ","));
                shape->start.y = (float)atof(strtok(0, ","));
                shape->start.z = (float)atof(strtok(0, "\n"));
                sprintf(key, "CollisionObject%iRadius", i + 1);
                shape->radius = UnknownFunction47b8a0(section, key, 0.0, path);
            } else if (!_stricmp(kind, "RADIUSEDLINE")) {
                shape->type = 3;
                sprintf(key, "CollisionObject%iStart", i + 1);
                GetPrivateProfileString(section, key, "", value, 0x80, path);
                shape->start.x = (float)atof(strtok(value, ","));
                shape->start.y = (float)atof(strtok(0, ","));
                shape->start.z = (float)atof(strtok(0, "\n"));
                sprintf(key, "CollisionObject%iEnd", i + 1);
                GetPrivateProfileString(section, key, "", value, 0x80, path);
                shape->end.x = (float)atof(strtok(value, ","));
                shape->end.y = (float)atof(strtok(0, ","));
                shape->end.z = (float)atof(strtok(0, "\n"));
                sprintf(key, "CollisionObject%iRadius", i + 1);
                shape->radius = UnknownFunction47b8a0(section, key, 0.0, path);
            } else if (!_stricmp(kind, "CYLINDER")) {
                shape->type = 1;
                sprintf(key, "CollisionObject%iBottom", i + 1);
                GetPrivateProfileString(section, key, "", value, 0x80, path);
                shape->start.x = (float)atof(strtok(value, ","));
                shape->start.y = (float)atof(strtok(0, ","));
                shape->start.z = (float)atof(strtok(0, "\n"));
                sprintf(key, "CollisionObject%iRadius", i + 1);
                shape->radius = UnknownFunction47b8a0(section, key, 0.0, path);
                sprintf(key, "CollisionObject%iHeight", i + 1);
                shape->height = UnknownFunction47b8a0(section, key, 0.0, path);
            }
        }
    } else {
        definition->collisionDefinitions = 0;
    }
}

// 0x004587b0: writes the .esb next to the .est.
int EcoSystem::WriteEsb(const char* path) {
    char name[0x104];
    unsigned char present;
    unsigned char length;
    int i;
    int pathLength = strlen(path);
    int count = pathLength > 0x103 ? 0x103 : pathLength;
    strncpy(name, path, count);
    name[count] = 0;
    strcpy(strrchr(name, '.'), ".esb");
    FILE* file = fopen(name, "wb");
    if (!file)
        return 0;
    fwrite(&method, 4, 1, file);
    for (i = 0; i < 256; i++) {
        present = definitionTable[i] != 0;
        fwrite(&present, 1, 1, file);
        if (present) {
            int j;
            length = strlen(definitionTable[i]->name) + 1;
            fwrite(&length, 1, 1, file);
            fwrite(definitionTable[i]->name, length, 1, file);
            length = strlen(definitionTable[i]->billboardName) + 1;
            fwrite(&length, 1, 1, file);
            fwrite(definitionTable[i]->billboardName, length, 1, file);
            fwrite(&definitionTable[i]->meanHeight, 4, 1, file);
            fwrite(&definitionTable[i]->minHeight, 4, 1, file);
            fwrite(&definitionTable[i]->maxHeight, 4, 1, file);
            fwrite(&definitionTable[i]->meanRadius, 4, 1, file);
            fwrite(&definitionTable[i]->minRadius, 4, 1, file);
            fwrite(&definitionTable[i]->maxRadius, 4, 1, file);
            fwrite(&definitionTable[i]->uLeft, 4, 1, file);
            fwrite(&definitionTable[i]->uRight, 4, 1, file);
            fwrite(&definitionTable[i]->vBottom, 4, 1, file);
            fwrite(&definitionTable[i]->uCenter, 4, 1, file);
            fwrite(&definitionTable[i]->vTop, 4, 1, file);
            fwrite(&definitionTable[i]->usePlanarLighting, 4, 1, file);
            fwrite(&definitionTable[i]->blendLods, 4, 1, file);
            fwrite(&definitionTable[i]->keyColor, 4, 1, file);
            if (method == 2) {
                fwrite(&definitionTable[i]->percentBias, 4, 1, file);
                fwrite(&definitionTable[i]->meanSlope, 4, 1, file);
                fwrite(&definitionTable[i]->standardDeviationSlope, 4, 1, file);
                fwrite(&definitionTable[i]->meanAspect, 4, 1, file);
                fwrite(&definitionTable[i]->standardDeviationAspect, 4, 1, file);
                fwrite(&definitionTable[i]->meanDrainage, 4, 1, file);
                fwrite(&definitionTable[i]->standardDeviationDrainage, 4, 1, file);
                fwrite(&definitionTable[i]->meanAltitude, 4, 1, file);
                fwrite(&definitionTable[i]->standardDeviationAltitude, 4, 1, file);
                present = definitionTable[i]->probabilityTga[0] != 0;
                fwrite(&present, 1, 1, file);
                if (present) {
                    length = strlen(definitionTable[i]->probabilityTga) + 1;
                    fwrite(&length, 1, 1, file);
                    fwrite(definitionTable[i]->probabilityTga, length, 1, file);
                }
            }
            fwrite(&definitionTable[i]->collisionCount, 4, 1, file);
            for (j = 0; j < definitionTable[i]->collisionCount; j++) {
                fwrite(&definitionTable[i]->collisionDefinitions[j].type, 4, 1, file);
                fwrite(&definitionTable[i]->collisionDefinitions[j].start, 0xc, 1, file);
                fwrite(&definitionTable[i]->collisionDefinitions[j].end, 0xc, 1, file);
                fwrite(&definitionTable[i]->collisionDefinitions[j].radius, 4, 1, file);
                fwrite(&definitionTable[i]->collisionDefinitions[j].height, 4, 1, file);
            }
        }
    }
    present = placementBmp[0] != 0;
    fwrite(&present, 1, 1, file);
    if (present) {
        length = strlen(placementBmp) + 1;
        fwrite(&length, 1, 1, file);
        fwrite(placementBmp, length, 1, file);
    }
    if (method == 2) {
        fwrite(&northAngle, 4, 1, file);
        present = probabilityTga[0] != 0;
        fwrite(&present, 1, 1, file);
        if (present) {
            length = strlen(probabilityTga) + 1;
            fwrite(&length, 1, 1, file);
            fwrite(probabilityTga, length, 1, file);
        }
    }
    fwrite(&placedCount, 4, 1, file);
    for (i = 0; i < placedCount; i++) {
        unsigned char definition = vegetation[i].definitionIndex;
        fwrite(&definition, 1, 1, file);
        fwrite(&vegetation[i].quantizedPosition, 6, 1, file);
        fwrite(&vegetation[i].heightParam, 1, 1, file);
        fwrite(&vegetation[i].radiusParam, 1, 1, file);
    }
    fclose(file);
    WriteListing(path);
    return 1;
}

// 0x00458f70: reads the .esb (the stream's own, the archive's, or a new one).
int EcoSystem::ReadEsb(const char* path, UnknownTextureStream* stream) {
    unsigned char present;
    unsigned char length;
    int i;
    esbStreamInArchive = 0;
    if (!stream) {
        UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(path, 1);
        if (!entry) {
            stream = new(__FILE__, 0x520) UnknownTextureStream(g_UnknownResourceManager572b44);
            if (!stream->UnknownFunction460f50(path, "rb", 0)) {
                if (stream)
                    delete stream;
                return 0;
            }
        } else {
            stream = entry->field_0x14;
            esbStreamInArchive = 1;
        }
    } else {
        esbStreamInArchive = 1;
    }
    esbStream = stream;
    stream->UnknownFunction461640(&method, 4, 1);
    for (i = 0; i < 256; i++) {
        stream->UnknownFunction461640(&present, 1, 1);
        if (present) {
            int j;
            stream->UnknownFunction461640(&length, 1, 1);
            definitionTable[i] = new(__FILE__, 0x543) UnknownEcoDefinition;
            stream->UnknownFunction461640(definitionTable[i]->name, length, 1);
            stream->UnknownFunction461640(&length, 1, 1);
            stream->UnknownFunction461640(definitionTable[i]->billboardName, length, 1);
            stream->UnknownFunction461640(&definitionTable[i]->meanHeight, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->minHeight, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->maxHeight, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->meanRadius, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->minRadius, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->maxRadius, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->uLeft, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->uRight, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->vBottom, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->uCenter, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->vTop, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->usePlanarLighting, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->blendLods, 4, 1);
            stream->UnknownFunction461640(&definitionTable[i]->keyColor, 4, 1);
            if (method == 2) {
                stream->UnknownFunction461640(&definitionTable[i]->percentBias, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->meanSlope, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->standardDeviationSlope, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->meanAspect, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->standardDeviationAspect, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->meanDrainage, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->standardDeviationDrainage, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->meanAltitude, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->standardDeviationAltitude, 4, 1);
                stream->UnknownFunction461640(&present, 1, 1);
                if (present) {
                    stream->UnknownFunction461640(&length, 1, 1);
                    stream->UnknownFunction461640(definitionTable[i]->probabilityTga, length, 1);
                }
            }
            stream->UnknownFunction461640(&definitionTable[i]->collisionCount, 4, 1);
            definitionTable[i]->collisionDefinitions = (UnknownEcoCollisionDefinition*)DebugMalloc(
                definitionTable[i]->collisionCount * sizeof(UnknownEcoCollisionDefinition), __FILE__, 0x56d);
            for (j = 0; j < definitionTable[i]->collisionCount; j++) {
                stream->UnknownFunction461640(&definitionTable[i]->collisionDefinitions[j].type, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->collisionDefinitions[j].start, 0xc, 1);
                stream->UnknownFunction461640(&definitionTable[i]->collisionDefinitions[j].end, 0xc, 1);
                stream->UnknownFunction461640(&definitionTable[i]->collisionDefinitions[j].radius, 4, 1);
                stream->UnknownFunction461640(&definitionTable[i]->collisionDefinitions[j].height, 4, 1);
            }
        }
    }
    stream->UnknownFunction461640(&present, 1, 1);
    if (present) {
        stream->UnknownFunction461640(&length, 1, 1);
        stream->UnknownFunction461640(placementBmp, length, 1);
    }
    if (method == 2) {
        stream->UnknownFunction461640(&northAngle, 4, 1);
        stream->UnknownFunction461640(&present, 1, 1);
        if (present) {
            stream->UnknownFunction461640(&length, 1, 1);
            stream->UnknownFunction461640(probabilityTga, length, 1);
        }
    }
    stream->UnknownFunction461640(&placedCount, 4, 1);
    totalObjects = placedCount;
    return 1;
}

// 0x004594c0
void EcoSystem::UnknownFunction4594c0(int level) {
    detailLevel = level;
}

// 0x004594d0
EcoSystem* EcoSystem::UnknownFunction4594d0(void* view, TextureMapManager* textures, UnknownEcoTerrain* terrain,
                                            LightManager* lights, char* path, UnknownTextureStream* stream,
                                            int textureFormat, int collisions, int level) {
    int i;
    GameObject::UnknownVirtualSlot8(view);
    g_UnknownGlobal59af14 = g_TrackGame->field_0x2d0 ? g_UnknownGlobal56a600 : g_UnknownGlobal56a740;
    UnknownFunction4594c0(level);
    groundTerrain = terrain;
    textureManager = textures;
    lightManager = lights;
    billboardFormat = textureFormat;
    float size = g_collisionQuadTree->field_0x54;
    float width = g_collisionQuadTree->field_0x50;
    if (width > size)
        size = width;
    coordinatesPerUnit = 65536.0f / size;
    unitsPerCoordinate = size * (1.0f / 65536.0f);
    ageManager = new(__FILE__, 0x5ca) AgeManager;
    UnknownFunction45a9a0();
    if (!ReadEst(path, stream)) {
        Release();
        return 0;
    }
    for (i = 0; i < 256; i++) {
        if (definitionTable[i]) {
            definitionTable[i]->LoadModel(textures, g_UnknownGlobal59af14[detailLevel].modelFlags);
            if (collisions)
                BuildCollisionObjects(definitionTable[i]);
            else
                definitionTable[i]->collisionCount = 0;
        }
    }
    vegetationTypeId = g_TypeRegistry->FindTypeId("Vegetation");
    vegetation = new(__FILE__, 0x5eb) Vegetation[totalObjects];
    billboardList = (Vegetation**)DebugMalloc(0x1f40, __FILE__, 0x5ec);
    geometryList = (Vegetation**)DebugMalloc(0x190, __FILE__, 0x5ed);
    billboardCapacity = 2000;
    geometryCapacity = 100;
    billboardVertices = (UnknownEcoVertex*)DebugMalloc(0x5a00, __FILE__, 0x5f1);
    billboardIndices = (unsigned short*)DebugMalloc(0xb40, __FILE__, 0x5f2);
    for (i = 0; i < 720; i++) {
        billboardVertices[i].reserved = 0;
        billboardVertices[i].specular = 0;
    }
    int index = 0;
    for (i = 0; i < 120; i++) {
        billboardIndices[index++] = i * 6;
        billboardIndices[index++] = i * 6 + 4;
        billboardIndices[index++] = i * 6 + 5;
        billboardIndices[index++] = i * 6;
        billboardIndices[index++] = i * 6 + 5;
        billboardIndices[index++] = i * 6 + 3;
        billboardIndices[index++] = i * 6 + 4;
        billboardIndices[index++] = i * 6 + 1;
        billboardIndices[index++] = i * 6 + 2;
        billboardIndices[index++] = i * 6 + 4;
        billboardIndices[index++] = i * 6 + 2;
        billboardIndices[index++] = i * 6 + 5;
    }
    if (strstr(path, ".esb")) {
        PlaceStoredObjects();
    } else {
        if (method == 1)
            PlaceAuthoredObjects();
        else
            GenerateObjects(UnknownFunction511ad0(textureFormat));
        WriteEsb(path);
    }
    return this;
}

// 0x00459b40: places the objects the PlacementBmp paints (one pixel per
// terrain cell, the palette index selecting the definition). The cell
// sizes are `extent; /= width` (that gives retail's `fld; fidiv` over the
// shared conversion slot), the pixel pointer is taken before them, and the
// loop bounds re-read the bitmap header each iteration.
int EcoSystem::PlaceAuthoredObjects() {
    if (!placementBmp[0])
        return 0;
    UnknownBitmapFile* bitmap = UnknownFunction424140(placementBmp, 0);
    if (!bitmap)
        return 0;
    int row;
    int column;
    unsigned char* pixel = (unsigned char*)bitmap->bits;
    float cellX = g_collisionQuadTree->field_0x50;
    cellX /= bitmap->infoHeader.width;
    float cellZ = g_collisionQuadTree->field_0x54;
    cellZ /= bitmap->infoHeader.height;
    for (row = 0; row < bitmap->infoHeader.height; row++) {
        for (column = 0; column < bitmap->infoHeader.width; column++) {
            int index = *pixel++;
            if (index && definitionTable[index]) {
                Vector3 position;
                if (placedCount == totalObjects)
                    goto done;
                position.x = (column + 0.5f) * cellX;
                position.y = 0.0f;
                position.z = (row + 0.5f) * cellZ;
                groundTerrain->QueryGround(&position, 0, 0, 0);
                unsigned char heightParameter = definitionTable[index]->RandomParameter();
                unsigned char radiusParameter = definitionTable[index]->ParameterForHeight(
                    definitionTable[index]->HeightForParameter(heightParameter));
                vegetation[placedCount].Place(textureManager, index, &position, heightParameter, radiusParameter);
                placedCount++;
            }
        }
    }
done:
    UnknownFunction4245b0(bitmap);
    return 1;
}

// 0x0045a9a0
void EcoSystem::UnknownFunction45a9a0() {
    UnknownEcoLight* light = lightManager->UnknownFunction4a0190(6);
    if (light) {
        ambientLight.x = light->field_0x54[0];
        ambientLight.y = light->field_0x54[1];
        ambientLight.z = light->field_0x54[2];
    } else {
        ambientLight.z = 0.0f;
        ambientLight.y = 0.0f;
        ambientLight.x = 0.0f;
    }
    light = lightManager->UnknownFunction4a0190(4);
    if (!light) {
        light = lightManager->UnknownFunction4a0190(2);
        if (!light) {
            lightColor.z = 0.0f;
            lightColor.y = 0.0f;
            lightColor.x = 0.0f;
            return;
        }
    }
    lightColor.x = light->field_0x54[0];
    lightColor.y = light->field_0x54[1];
    lightColor.z = light->field_0x54[2];
    lightDirection.x = light->field_0x70.x;
    lightDirection.y = 0.0f;
    lightDirection.z = light->field_0x70.z;
    float length = UnknownSquareMagnitude(&lightDirection);
    if (length == 0.0f) {
        lightDirection = kVec3Zero;
        return;
    }
    length = FastInvSqrt(length);
    lightDirection.x = length * lightDirection.x;
    lightDirection.y = length * lightDirection.y;
    lightDirection.z = length * lightDirection.z;
}

// 0x0045aad0
int EcoSystem::UnknownVirtualSlot12() {
    if (!g_UnknownGlobal56a12c)
        return 1;
    int category = g_MemTagStack->Push("EcoSystem");
    unsigned int start = ReadClock();
    if (!totalObjects)
        return 1;
    if (ageManager) {
        int stale;
        int total = ageManager->TotalSize(&stale);
        if (stale > 0x40000)
            ageManager->EvictStale(total - stale);
        ageManager->AdvanceAge();
    }
    g_UnknownGlobal59af00 = &ECO_VIEW->field_0x08->field_0xac;
    depthScale = 65535.0f / ECO_VIEW->field_0x08->field_0x1c0;
    depthUnit = ECO_VIEW->field_0x08->field_0x1c0 * (1.0f / 65535.0f);
    billboardCount = 0;
    geometryCount = 0;
    g_collisionQuadTree->RestartQuery();
    Vegetation* object = (Vegetation*)g_collisionQuadTree->NextObjectSorted();
    while (object) {
        if (object->field_0x08 == vegetationTypeId) {
            int billboard = object->isBillboard;
            int fade = object->fadeLevel;
            object->TestDistance(&billboard, &fade);
            UnknownEcoDefinition* definition = definitionTable[object->definitionIndex];
            int visible = 1;
            if (!billboard) {
                float radius = definition->RadiusForParameter(object->radiusParam);
                float height = definition->HeightForParameter(object->heightParam) * 0.5;
                Vector3 center;
                if (radius <= height)
                    radius = height;
                center.x = object->quantizedPosition.x * g_UnknownGlobal59aebc->unitsPerCoordinate;
                center.y = object->quantizedPosition.y * g_UnknownGlobal59aebc->unitsPerCoordinate + height;
                center.z = object->quantizedPosition.z * g_UnknownGlobal59aebc->unitsPerCoordinate;
                visible = g_visibilityClipper->SphereInFrustum(ECO_VIEW->field_0x08, &ECO_VIEW->field_0x08->field_0xec,
                                                               &center, radius, 0);
            }
            if (visible) {
                object->SetBillboard(billboard, fade);
                if (object->fadeLevel) {
                    if (billboardCount == billboardCapacity) {
                        Vegetation** old = billboardList;
                        billboardList = (Vegetation**)DebugRealloc(old, (billboardCapacity + 100) * sizeof(Vegetation*),
                                                                __FILE__, 0x8dd);
                        if (billboardList != old)
                            billboardCapacity += 100;
                    }
                    billboardList[billboardCount] = object;
                    billboardCount++;
                }
                if (object->isBillboard < definition->lodCount) {
                    if (geometryCount == geometryCapacity) {
                        Vegetation** old = geometryList;
                        geometryList = (Vegetation**)DebugRealloc(old, (geometryCapacity + 20) * sizeof(Vegetation*),
                                                                __FILE__, 0x8ea);
                        if (geometryList != old)
                            geometryCapacity += 20;
                    }
                    geometryList[geometryCount] = object;
                    geometryCount++;
                }
            }
        }
        object = (Vegetation*)g_collisionQuadTree->NextObjectSorted();
    }
    g_UnknownGlobal59aee8 = ReadClock() - start;
    g_MemTagStack->Pop(category);
    return 1;
}

// 0x0045ade0: the render and texture-stage states for `format`.
void EcoSystem::SetRenderStates(int format) {
    float alphaReference;
    if (format == 1555) {
        alphaReference = (float)(g_TrackGame->field_0x2d0 ? 0 : 0xc0);
        ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
        ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_COLORKEYENABLE, 0, 0);
    } else if (format == 4444 || format == 8888) {
        alphaReference = 0.0f;
        ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_COLORKEYENABLE, 0, 0);
    } else {
        alphaReference = 0.0f;
        ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
        ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_COLORKEYENABLE, 1, 0);
    }
    ECO_VIEW->UnknownVirtualSlot18((int)alphaReference);
    if (g_UnknownGlobal59af14[detailLevel].stageLighting) {
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        if (g_UnknownGlobal59af14[detailLevel].specular)
            ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD, 0);
        else
            ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT, 0);
        if (format == 4444 || format == 8888) {
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        } else {
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        }
    } else {
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        ECO_VIEW->UnknownVirtualSlot8(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT, 0);
        // Both arms are the same in retail (the second is reached by jump
        // threading from the format test below).
        if (format == 4444) {
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        } else {
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        }
    }
    if (format == 1555) {
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
        if (g_TrackGame->field_0x2d0) {
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        } else {
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
            ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
        }
    } else if (format == 4444) {
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
    } else {
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
        ECO_VIEW->UnknownVirtualSlot7(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
    }
}

// 0x0045bfe0: the key toggling the drawing.
int EcoSystem::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (GameObject::UnknownVirtualSlot23(event, entry))
        return 1;
    if (UnknownFunction43caa0(4, 0, event, 0x80)) {
        g_UnknownGlobal56a12c = 1 - g_UnknownGlobal56a12c;
        return 1;
    }
    return 0;
}

// 0x0045c040 (cdecl): loads every texture the .esb names before the
// ecosystem itself is created (the loader then finds them in the manager).
// `path` is rewritten to the .esb name when one exists next to the .est.
// The index read is its own block: `collisionCount` must not be live in the
// probe block (its slot holds the probe's `new` temporary) while it is live
// across the stream's `new` (which takes `ownsStream`'s dead slot) and the
// second loop (whose induction spill gets a slot of its own).
int UnknownFunction45c040(TextureMapManager* textures, char* path, UnknownTextureStream* stream, int modelFlags) {
    char names[256][0x80];
    char billboardNames[256][0x80];
    unsigned int keyColors[256];
    char esbName[0x104];
    char sltName[0x104];
    unsigned char present;
    unsigned char length;
    int method;
    int ownsStream = 0;
    int i;
    memset(billboardNames, 0, sizeof(billboardNames));
    memset(names, 0, sizeof(names));
    g_UnknownGlobal59af10 = 0;
    if (!stream && strstr(path, ".est")) {
        int pathLength = strlen(path);
        int count = pathLength > 0x103 ? 0x103 : pathLength;
        strncpy(esbName, path, count);
        esbName[count] = 0;
        strcpy(strstr(esbName, ".est"), ".esb");
        UnknownTextureStream* probe = new(__FILE__, 0xad7) UnknownTextureStream(g_UnknownResourceManager572b44);
        if (probe->UnknownFunction460f50(esbName, "rb", 0))
            strcpy(strstr(path, ".est"), ".esb");
        if (probe)
            delete probe;
    }
    if (!strstr(path, ".esb"))
        return 0;
    {
        int collisionCount;
        UnknownTextureStream* esb = stream;
        if (!esb) {
            UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(path, 1);
            if (!entry) {
                esb = new(__FILE__, 0xaf8) UnknownTextureStream(g_UnknownResourceManager572b44);
                if (!esb->UnknownFunction460f50(path, "rb", 0)) {
                    if (esb)
                        delete esb;
                    return 0;
                }
                ownsStream = 1;
            } else {
                esb = entry->field_0x14;
            }
        }
        esb->UnknownFunction461640(&method, 4, 1);
        for (i = 0; i < 256; i++) {
            esb->UnknownFunction461640(&present, 1, 1);
            if (present) {
                esb->UnknownFunction461640(&length, 1, 1);
                esb->UnknownFunction461640(names[i], length, 1);
                esb->UnknownFunction461640(&length, 1, 1);
                esb->UnknownFunction461640(billboardNames[i], length, 1);
                esb->UnknownFunction461340(0x34, 1, 1);
                esb->UnknownFunction461640(&keyColors[i], 4, 1);
                if (method == 2) {
                    esb->UnknownFunction461340(0x24, 1, 1);
                    esb->UnknownFunction461640(&present, 1, 1);
                    if (present) {
                        esb->UnknownFunction461640(&length, 1, 1);
                        esb->UnknownFunction461340(length, 1, 1);
                    }
                }
                esb->UnknownFunction461640(&collisionCount, 4, 1);
                esb->UnknownFunction461340(collisionCount * sizeof(UnknownEcoCollisionDefinition), 1, 1);
            }
        }
        if (ownsStream) {
            if (esb)
                delete esb;
        }
        for (i = 0; i < 256; i++) {
            if (billboardNames[i][0]) {
                UnknownEcoTexture* texture = UnknownFunction50a590(textures, billboardNames[i], modelFlags, 0, 2, 5, 6, 0, 0x80,
                                                                   0xff00ff, 1, 1);
                if (!texture->UnknownVirtualSlot7()) {
                    texture->UnknownVirtualSlot8(1, 0, 0);
                    g_UnknownGlobal59af10++;
                }
            }
            if (names[i][0]) {
                strcpy(sltName, names[i]);
                strcat(sltName, ".slt");
                UnknownTextureStream* sltStream = new(__FILE__, 0xb50) UnknownTextureStream(g_UnknownResourceManager572b44);
                if (sltStream->UnknownFunction460f50(sltName, "r", 0)) {
                    UnknownParameterBlock* block = new(__FILE__, 0xb52) UnknownParameterBlock;
                    block->UnknownFunction4b77a0((UnknownParameterStream*)sltStream, 0, 1);
                    block->UnknownFunction4b78f0("Material - 0");
                    block->UnknownFunction4b7b30("TextureMap", sltName, -1);
                    int format = 1555;
                    if (!g_TrackGame->field_0x2d0 && (g_TrackGame->field_0x10->field_0x1c0 & 8)
                        && g_TrackGame->GetRegistryFlag("KeyColorTrees", 0))
                        format = g_TrackGame->field_0x10->field_0x28;
                    UnknownEcoTexture* texture = UnknownFunction50a590(textures, sltName, format, 0, 2, 5, 6, 0, 0x80, 0xff00ff, 1, 1);
                    if (!texture->UnknownVirtualSlot7()) {
                        g_UnknownGlobal59af10++;
                        int loaded = texture->field_0x20;
                        if (loaded == 555 || loaded == 565 || loaded == 888 || loaded == 1555)
                            texture->UnknownVirtualSlot18(keyColors[i]);
                        texture->UnknownVirtualSlot8(1, 0, 0);
                    }
                    if (block)
                        delete block;
                }
                if (sltStream)
                    delete sltStream;
            }
        }
    }
    g_UnknownGlobal59aefc = 1;
    return 1;
}

// 0x0045c6a0 (cdecl): fwrite through a running xor key, 1 KB at a time.
// The byte is read, then xored with the key; the key accumulates the
// encoded byte. The block count is the loop bound expression (it stays in
// a frame slot) and the tail loop counts down.
int UnknownFunction45c6a0(const unsigned char* data, int size, int count, FILE* file, unsigned char* key) {
    unsigned char buffer[0x400];
    int total = size * count;
    int block;
    int i;
    for (block = 0; block < total / 0x400; block++) {
        unsigned char* out = buffer;
        for (i = 0; i < 0x400; i++) {
            unsigned char c = *data++;
            c ^= *key;
            *key += c;
            *out++ = c;
        }
        if (fwrite(buffer, 0x400, 1, file) != 1)
            return 0;
    }
    total %= 0x400;
    if (total == 0)
        return 1;
    unsigned char* out = buffer;
    for (i = total; i > 0; i--) {
        unsigned char c = *data++;
        c ^= *key;
        *key += c;
        *out++ = c;
    }
    return fwrite(buffer, total, 1, file) == 1;
}

// 0x0045c7b0 (cdecl): fread through the running xor key. A short read is
// fatal on error, and on EOF only the bytes read are decoded. The decoded
// byte is computed before the key accumulates the encoded byte.
int UnknownFunction45c7b0(unsigned char* data, int size, int count, FILE* file, unsigned char* key) {
    int total = size * count;
    int read = fread(data, size, count, file);
    int i;
    if (read != count) {
        if (file->_flag & 0x10) {
            if (!read)
                return 0;
            total = read * size;
        }
        if (file->_flag & 0x20)
            return 0;
    }
    for (i = 0; i < total; i++) {
        unsigned char value = *data;
        unsigned char decoded = *key ^ value;
        *key += value;
        *data = decoded;
        data++;
    }
    return 1;
}

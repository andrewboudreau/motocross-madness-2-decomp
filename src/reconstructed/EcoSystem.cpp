#include "../krusty2/broadphase/Terrain.h"
// EcoSystem.cpp -- D:\aardvark\VC\krusty2\EcoSystem.cpp, 0x00455da0..0x0045c830.
// See EcoSystem.h for the class evidence and docs/ECOSYSTEM.md for the
// match state. Line numbers in the allocation calls are the retail
// __LINE__ values.

#include "TrackGameGlobal.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "EcoSystem.h"
#include "../krusty2/visibility/VisibilityClipper.h"

#include "MemTag.h"
#include "Parameterblocks.h"
#include "PeakHold.h"
#include "TypeRegistry.h"
#include "ResourceEntry.h"
#include "bmpfile.h"
#include "Tgafile.h"
#include "D3DConstants.h"

// The four vector constants that open about 73 retail files
// (0x0059aea0, 0x0059aeb0, 0x0059aed8, 0x0059ae90; $E 0x00459830..0x00459b3b).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

EcoSystem* g_EcoSystem;             // 0x0059aebc
static int s_geometryRenderTicks;            // 0x0059aec0: slot 14 geometry draw ticks ("Render %d(3d:%d)")
static PeakHold s_prepareGeometryPeak(5000); // 0x0059aef0 ($E 0x00455db0, XCU 103): "PrepareGeometry %d %d" peak
static PeakHold s_renderPeak(5000);   // 0x0059aec8 ($E 0x00455dd0, XCU 104): "Render" peak
static int s_renderTicks;                    // 0x0059aee4: slot 14 draw ticks ("Render %d")
static int s_prepareGeometryTicks;           // 0x0059aee8: classification ticks ("PrepareGeometry %d")
static int g_UnknownGlobal59aefc;             // 0x0059aefc: textures were preloaded
static Matrix4* g_UnknownGlobal59af00;        // 0x0059af00: the view matrix
static int s_geometryDrawn;                  // 0x0059af04: geometry objects drawn ("Rendered ... 3D %d")
static int s_billboardsDrawn;                // 0x0059af08: billboards drawn ("Rendered 2D %d")
static int g_UnknownGlobal59af0c;             // 0x0059af0c: geometry blocks alive
int g_UnknownGlobal59af10;                    // 0x0059af10: textures preloaded (BaseQuarryEvent::LoadEventObjects reads it)
UnknownEcoDetailBand* g_UnknownGlobal59af14; // 0x0059af14: the detail band table (Render3D and TestDistance read it)

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

static int s_debugPage = -1;                 // 0x0056a128: the "Ecosystem:%d objects" debug-overlay page
static int s_drawEnabled = 1;                // 0x0056a12c: drawing enabled (slot 23 toggles it)

// Inline helper (shape only; the retail header is not attested): the
// squared length, summed as z + (x + y) (the only order VC6 compiles to
// the retail load sequence).
static inline float EcoSquareMagnitude(const Vector3* v) {
    return v->z * v->z + (v->x * v->x + v->y * v->y);
}

// The view (RenderTarget.h) the object was attached to, GameObject+0x18.

#define ECO_VIEW ((RenderTarget*)renderContext)

#define ECO_RGBA(r, g, b, a) ((unsigned int)(((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

// 0x00455de0
VegetationDefinition::VegetationDefinition() {
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
VegetationDefinition::~VegetationDefinition() {
    int i;
    if (billboardTexture) {
        billboardTexture->Release();
        if (g_UnknownGlobal59aefc)
            billboardTexture->Release();
    }
    if (modelTexture) {
        modelTexture->Release();
        if (g_UnknownGlobal59aefc)
            modelTexture->Release();
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
int VegetationDefinition::RandomParameter() {
    return rand() >> 8;
}

// 0x00455f60
int VegetationDefinition::ParameterForHeight(float height) {
    return (int)(rand() * height / maxHeight) >> 8;
}

// 0x00455f90
float VegetationDefinition::HeightForParameter(unsigned char parameter) {
    if (parameter < 0x80)
        return meanHeight - (meanHeight - minHeight) * parameter * (1.0f / 128.0f);
    return meanHeight + (maxHeight - meanHeight) * (255 - parameter) * (1.0f / 128.0f);
}

// 0x00455ff0
float VegetationDefinition::RadiusForParameter(unsigned char parameter) {
    if (parameter < 0x80)
        return meanRadius - (meanRadius - minRadius) * parameter * (1.0f / 128.0f);
    return meanRadius + (maxRadius - meanRadius) * (255 - parameter) * (1.0f / 128.0f);
}

// 0x00456050: loads the billboard texture and the .slt model (vertices,
// faces and its texture); `modelFlags` is the detail band's model flag.
int VegetationDefinition::LoadModel(TextureMapManager* textures, int modelFlags) {
    char path[0x104];
    char section[0x80];
    if (billboardName[0]) {
        billboardTexture = TerrainLoadTexture(textures, billboardName, modelFlags, 0, 2, 5, 6, 0, 0x80, 0xff00ff, 1, 1);
        if (!billboardTexture->IsResident())
            billboardTexture->CreateTextureSurface(1, 0, 0);
    }
    if (name[0]) {
        int nameLength = strlen(name);
        int length = nameLength > 0x103 ? 0x103 : nameLength;
        strncpy(path, name, length);
        path[length] = 0;
        strcat(path, ".slt");
        FileStream* stream = new(__FILE__, 0xc0) FileStream(g_ResourceManager);
        if (stream->Open(path, "r", 0)) {
            int lod;
            ParameterBlock* block = new(__FILE__, 0xc2) ParameterBlock;
            block->Attach((FileStream*)stream, 0, 1);
            block->SelectSection("Material - 0");
            block->ReadString("TextureMap", path, -1);
            int format = 1555;
            if (!reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->bladeRenderer && (reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->display->driverColorKeyCaps & 8)
                && reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->GetRegistryFlag("KeyColorTrees", 0))
                format = reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->display->colorKeyFormat;
            modelTexture = TerrainLoadTexture(textures, path, format, 0, 2, 5, 6, 0, 0x80, 0xff00ff, 1, 1);
            if (!modelTexture->IsResident()) {
                int loaded = modelTexture->mapFormat;
                if (loaded == 555 || loaded == 565 || loaded == 888 || loaded == 1555)
                    modelTexture->ApplyColorKey(keyColor);
                modelTexture->CreateTextureSurface(1, 0, 0);
            }
            block->SelectSection("LOD Information");
            block->GetInt("NumberOfLOD", 0, &lodCount);
            if (lodCount > kMaxLods)
                lodCount = kMaxLods;
            float maxX = -FLT_MAX;
            float minX = FLT_MAX;
            float maxY = -FLT_MAX;
            float minY = FLT_MAX;
            for (lod = 0; lod < lodCount; lod++) {
                int faces;
                int i;
                int corner0;
                int corner1;
                int corner2;
                sprintf(section, "LOD %d - Surface 0", lod);
                block->SelectSection(section);
                block->GetInt("NumberOfVertices", 0, &modelVertexCount[lod]);
                block->GetInt("NumberOfFaces", 0, &faces);
                modelIndexCount[lod] = faces * 3;
                modelVertices[lod] = (EcoModelVertex*)DebugMalloc(
                    modelVertexCount[lod] * sizeof(EcoModelVertex) + faces * 3 * sizeof(unsigned short), __FILE__, 0x106);
                modelIndices[lod] = (unsigned short*)(modelVertices[lod] + modelVertexCount[lod]);
                sprintf(section, "LOD %d - Surface 0 - Vertices", lod);
                block->BeginRows(section);
                for (i = 0; i < modelVertexCount[lod]; i++) {
                    block->ReadRow(0);
                    block->GetRowFloat(0, &modelVertices[lod][i].position.x);
                    block->GetRowFloat(1, &modelVertices[lod][i].position.y);
                    block->GetRowFloat(2, &modelVertices[lod][i].position.z);
                    if (modelVertices[lod][i].position.y > maxY)
                        maxY = modelVertices[lod][i].position.y;
                    if (modelVertices[lod][i].position.y < minY)
                        minY = modelVertices[lod][i].position.y;
                    if (modelVertices[lod][i].position.x > maxX)
                        maxX = modelVertices[lod][i].position.x;
                    if (modelVertices[lod][i].position.x < minX)
                        minX = modelVertices[lod][i].position.x;
                    block->GetRowFloat(3, &modelVertices[lod][i].normal.x);
                    block->GetRowFloat(4, &modelVertices[lod][i].normal.y);
                    block->GetRowFloat(5, &modelVertices[lod][i].normal.z);
                    block->GetRowFloat(6, &modelVertices[lod][i].tu);
                    block->GetRowFloat(7, &modelVertices[lod][i].tv);
                    if (modelVertices[lod][i].tv < 0.0f)
                        modelVertices[lod][i].tv = modelVertices[lod][i].tv + 1.0f;
                }
                sprintf(section, "LOD %i - Surface 0 - Faces", lod);
                block->BeginRows(section);
                unsigned short* index = modelIndices[lod];
                for (i = 0; i < modelIndexCount[lod] / 3; i++) {
                    block->ReadRow(0);
                    block->GetRowInt(0, &corner0);
                    block->GetRowInt(1, &corner1);
                    block->GetRowInt(2, &corner2);
                    *index++ = (unsigned short)corner0;
                    *index++ = (unsigned short)corner1;
                    *index++ = (unsigned short)corner2;
                }
            }
            if (block)
                delete block;
            modelHeightScale = 1.0f / (maxY - minY);
            modelRadiusScale = 2.0f / (maxX - minX);
            normalizedMinY = modelHeightScale * minY;
        }
        if (stream)
            delete stream;
    }
    return 1;
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
    objectTypeId = g_EcoSystem->vegetationTypeId;
}

// 0x00456690
short Vegetation::ComputeSortKey() {
    Matrix4* view = g_UnknownGlobal59af00;
    float depth = ((quantizedPosition.z * (*view)(2, 2) + quantizedPosition.x * (*view)(0, 2)) *
                       g_EcoSystem->unitsPerCoordinate +
                   (*view)(3, 2)) * g_EcoSystem->depthScale;
    if (depth < 0.0)
        sortKey = 0;
    else if ((int)depth >= 0xffff)
        sortKey = (short)0xffff;
    else
        sortKey = (short)(int)depth;
    return sortKey;
}

// 0x00456720
unsigned int Vegetation::ComputeQuadTreeCode(QuadTree* tree) {
    float radius = g_EcoSystem->definitionTable[definitionIndex]->RadiusForParameter(radiusParam);
    float x = quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
    float z = quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
    return tree->ComputeCode(x - radius, z - radius, x + radius, z + radius);
}

// 0x004567a0
void Vegetation::PlaceQuantized(TextureMapManager* textures, unsigned char definition,
                                       const EcoQuantizedPosition* coordinates, unsigned char heightParameter,
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
    quantizedPosition.x = (unsigned short)(int)(g_EcoSystem->coordinatesPerUnit * VectorX(*position));
    quantizedPosition.y = (unsigned short)(int)(g_EcoSystem->coordinatesPerUnit * VectorY(*position));
    quantizedPosition.z = (unsigned short)(int)(g_EcoSystem->coordinatesPerUnit * VectorZ(*position));
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

float FastSqrt(float value);  // 0x00460b50

// 0x00456890: whether the object is beyond the detail band's 3D distance
// (then `fade` is 255), else the fade between the band's two distances.
// Not exact (356 of 369 bytes in place; docs/ECOSYSTEM.md).
void Vegetation::TestDistance(int* billboard, int* fade) {
    VegetationDefinition* definition = g_EcoSystem->definitionTable[definitionIndex];
    Camera* camera = ((RenderTarget*)g_EcoSystem->renderContext)->currentCamera;
    float dx = camera->eyePosition.x - quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
    float dz = camera->eyePosition.z - quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
    *billboard = 0;
    float outer = (float)g_UnknownGlobal59af14[g_EcoSystem->detailLevel].geometryDistance;
    float inner = (float)g_UnknownGlobal59af14[g_EcoSystem->detailLevel].fadeStartDistance;
    if (dx > outer || dz > outer) {
        *billboard = 1;
        *fade = 0xff;
        return;
    }
    float distance = FastSqrt(dz * dz + dx * dx);
    if (distance > outer) {
        *billboard = 1;
        *fade = 0xff;
        return;
    }
    if (definition->blendLods) {
        if (distance < inner)
            *fade = 0;
        else
            *fade = (int)((distance - inner) * 255.0f / (outer - inner));
    } else {
        *fade = 0;
    }
}

// 0x00456a10: builds the lit geometry for `billboard` (a LOD index, or the
// billboard when it is past the LOD count) at the quantized position, rotated
// to face the camera's horizontal direction. The vertex block (vertices, the
// 4-byte-padded indices, then the AgeEntry) is registered with the
// AgeManager for eviction. The min/max of x are computed but unused.
void Vegetation::SetBillboard(int billboard, int fade) {
    VegetationDefinition* definition = g_EcoSystem->definitionTable[definitionIndex];
    Camera* camera = ((RenderTarget*)g_EcoSystem->renderContext)->currentCamera;
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
        g_EcoSystem->ageManager->Register(entry, EvictGeometry, this, 0,
                                                                  dwords * 4 + sizeof(AgeEntry));
        g_UnknownGlobal59af0c++;
        memcpy((EcoLitVertex*)geometryBlock + vertexCount, definition->modelIndices[billboard], indexCount * 2);
        float radiusScale = definition->RadiusForParameter(radiusParam) * definition->modelRadiusScale;
        float heightScale = definition->HeightForParameter(heightParam) * definition->modelHeightScale;
        float c;
        float s;
        if (camera->viewDirection.z != 0.0f) {
            float inverse = FastInvSqrtEstimate(camera->viewDirection.x * camera->viewDirection.x
                                                  + camera->viewDirection.z * camera->viewDirection.z) * radiusScale;
            c = inverse * camera->viewDirection.z;
            s = -(inverse * camera->viewDirection.x);
        } else {
            c = radiusScale;
            s = 0.0f;
        }
        // Six scalars, not two Vector3s: retail interleaves the colour and
        // ambient components with the other scalar slots of the frame.
        float colorR = g_EcoSystem->lightColor.x;
        float colorG = g_EcoSystem->lightColor.y;
        float colorB = g_EcoSystem->lightColor.z;
        float ambientR = g_EcoSystem->ambientLight.x;
        float ambientG = g_EcoSystem->ambientLight.y;
        float ambientB = g_EcoSystem->ambientLight.z;
        unsigned int ambientColor = ECO_RGBA((int)(ambientR * 255.0f), (int)(ambientG * 255.0f),
                                             (int)(ambientB * 255.0f), 255);
        EcoModelVertex* source = definition->modelVertices[billboard];
        int i;
        // Retail indexes the output vertex off `geometryBlock` and re-reads
        // the member before every store (the void* member aliases the
        // stores); a walking `EcoLitVertex*` local compiles a different
        // loop with a 0x4c frame.
#define VERTEX ((EcoLitVertex*)geometryBlock)[i]
        for (i = 0; i < vertexCount; i++) {
            int unlit = 0;
            Vector3 normal;
            float x = quantizedPosition.x;
            VERTEX.position.x = c * source->position.x - s * source->position.z + x * g_EcoSystem->unitsPerCoordinate;
            VERTEX.position.y = quantizedPosition.y * g_EcoSystem->unitsPerCoordinate + heightScale * source->position.y;
            VERTEX.position.z = quantizedPosition.z * g_EcoSystem->unitsPerCoordinate + c * source->position.z
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
                length = EcoSquareMagnitude(&normal);
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
            float intensity = -(normal.z * g_EcoSystem->lightDirection.z
                                + (normal.x * g_EcoSystem->lightDirection.x
                                   + normal.y * g_EcoSystem->lightDirection.y));
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
void Vegetation::DrawGeometry(RenderTarget* target) {
    VegetationDefinition* definition = g_EcoSystem->definitionTable[definitionIndex];
    definition->modelTexture->Bind();
    int vertexCount = definition->modelVertexCount[isBillboard];
    int indexCount = definition->modelIndexCount[isBillboard];
    target->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, (int)geometryBlock, vertexCount, (int)((EcoLitVertex*)geometryBlock + vertexCount),
                                 indexCount, 0);
    g_EcoSystem->ageManager->MarkUsed(
        (AgeEntry*)((int*)geometryBlock + vertexCount * 8 + (indexCount + 1) / 2));
}

// 0x00457080
int Vegetation::GetCollisionCount() {
    return g_EcoSystem->definitionTable[definitionIndex]->collisionCount;
}

// 0x00457230
float Vegetation::GetRadius() {
    return g_EcoSystem->definitionTable[definitionIndex]->RadiusForParameter(radiusParam);
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
    g_EcoSystem = this;
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
    g_EcoSystem = 0;
    g_UnknownGlobal59aefc = 0;
}

// The identity matrix as the collision builders store it (inline shape
// only; the out-of-line IdentityMatrix 0x004a1410 is not what retail calls).
static inline void EcoSetIdentity(Matrix4* m) {
    memset(m, 0, sizeof(Matrix4));
    (*m)(3, 3) = 1.0f;
    (*m)(2, 2) = 1.0f;
    (*m)(1, 1) = 1.0f;
    (*m)(0, 0) = 1.0f;
}

// By-value component read: it makes VC6 load the other operand first
// (0x004567e0, now exact in the src unit, needs the same).
static inline float EcoComponentZ(const Vector3& v) {
    return v.z;
}

// The vector sum as 0x00458246 computes it (a value-returning inline in
// operator+ form). Retail sums z vertex-first (`fld [vertex.z]; fadd
// [start.z]`); only the by-value read of the vertex's z gives that order.
static inline Vector3 EcoSum(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + EcoComponentZ(b));
}

// 0x00457480: reads the .est placement file.
// Not exact (561 of 2629 bytes in place, 2626 compiled; docs/ECOSYSTEM.md).
int EcoSystem::ReadEst(char* path, FileStream* stream) {
    char name[0x104];
    char value[0x80];
    char section[0x80];
    int i;
    if (!stream && strstr(path, ".est")) {
        int pathLength = strlen(path);
        int length = pathLength > 0x103 ? 0x103 : pathLength;
        strncpy(name, path, length);
        name[length] = 0;
        strcpy(strstr(name, ".est"), ".esb");
        FileStream* probe = new(__FILE__, 0x2eb) FileStream(g_ResourceManager);
        if (probe->Open(name, "rb", 0))
            strcpy(strstr(path, ".est"), ".esb");
        delete probe;
    }
    if (strstr(path, ".est")) {
        float total;
        GetPrivateProfileString("EcoSystem", "Method", "NONE", value, 0x80, path);
        if (!strcmp(value, "NONE"))
            return 0;
        if (!_stricmp(value, "Authored"))
            method = 1;
        else if (!_stricmp(value, "Auto"))
            method = 2;
        else
            return 0;
        totalObjects = GetPrivateProfileInt("EcoSystem", "TotalObjects", 40000, path);
        total = 0.0f;
        for (i = 1; i < 256; i++) {
            sprintf(section, "Vegetation_%d", i);
            GetPrivateProfileString(section, "Name", "NONE", value, 0x80, path);
            if (strcmp(value, "NONE")) {
                int red;
                int green;
                int blue;
                definitionTable[i] = new(__FILE__, 0x31f) VegetationDefinition;
                strcpy(definitionTable[i]->name, value);
                GetPrivateProfileString(section, "BillboardName", "NONE", value, 0x80, path);
                strcpy(definitionTable[i]->billboardName, value);
                definitionTable[i]->meanHeight = ReadIniFloat(section, "MeanHeight", 10.0, path);
                definitionTable[i]->minHeight = ReadIniFloat(section, "MinHeight", 5.0, path);
                definitionTable[i]->maxHeight = ReadIniFloat(section, "MaxHeight", 15.0, path);
                definitionTable[i]->meanRadius = ReadIniFloat(section, "MeanRadius", 10.0, path);
                definitionTable[i]->minRadius = ReadIniFloat(section, "MinRadius", 5.0, path);
                definitionTable[i]->maxRadius = ReadIniFloat(section, "MaxRadius", 15.0, path);
                definitionTable[i]->uLeft = ReadIniFloat(section, "ULeft", 0.0, path) * (1.0f / 256.0f);
                definitionTable[i]->uRight = ReadIniFloat(section, "URight", 1.0, path) * (1.0f / 256.0f);
                definitionTable[i]->vBottom = ReadIniFloat(section, "VBottom", 0.0, path) * (1.0f / 256.0f);
                // Retail crosses these two keys: "VTop" lands in the field the
                // ctor and WriteEsb treat as uCenter, "UCenter" in vTop.
                definitionTable[i]->uCenter = ReadIniFloat(section, "VTop", 1.0, path) * (1.0f / 256.0f);
                definitionTable[i]->vTop = ReadIniFloat(section, "UCenter", 1.0, path) * (1.0f / 256.0f);
                definitionTable[i]->usePlanarLighting = GetPrivateProfileInt(section, "UsePlanarLighting", 1, path) != 0;
                definitionTable[i]->blendLods = GetPrivateProfileInt(section, "BlendLODs", 1, path) != 0;
                definitionTable[i]->percentBias = ReadIniFloat(section, "PercentBias", 1.0, path);
                red = GetPrivateProfileInt(section, "KeyColorRed", 0x5a, path);
                green = GetPrivateProfileInt(section, "KeyColorGreen", 0x5b, path);
                blue = GetPrivateProfileInt(section, "KeyColorBlue", 0xb, path);
                definitionTable[i]->keyColor = (red << 16) | (green << 8) | blue;
                total = total + definitionTable[i]->percentBias;
                ReadCollisionObjects(path, i, definitionTable[i]);
            }
        }
        if (total != 100.0f) {
            for (i = 1; i < 256; i++) {
                if (definitionTable[i])
                    definitionTable[i]->percentBias = 100.0f / total * definitionTable[i]->percentBias;
            }
        }
        GetPrivateProfileString("EcoSystem", "PlacementBmp", "NONE", value, 0x80, path);
        if (!strcmp(value, "NONE")) {
            placementBmp[0] = 0;
        } else {
            int valueLength = strlen(value);
            int length = valueLength > 0x7f ? 0x7f : valueLength;
            strncpy(placementBmp, value, length);
            placementBmp[length] = 0;
        }
        if (method == 2) {
            for (i = 0; i < 256; i++) {
                if (definitionTable[i]) {
                    sprintf(section, "Vegetation_%d", i);
                    definitionTable[i]->meanSlope = ReadIniFloat(section, "MeanSlope", 45.0, path) * (1.0f / 90.0f);
                    definitionTable[i]->standardDeviationSlope =
                        ReadIniFloat(section, "StandardDeviationSlope", 45.0, path) * (1.0f / 90.0f);
                    definitionTable[i]->meanAspect =
                        (ReadIniFloat(section, "MeanAspect", 180.0, path) - 180.0f) * (1.0f / 90.0f);
                    definitionTable[i]->standardDeviationAspect =
                        (ReadIniFloat(section, "StandardDeviationAspect", 180.0, path) - 180.0f) * (1.0f / 90.0f);
                    definitionTable[i]->meanDrainage = ReadIniFloat(section, "MeanDrainage", 0.5, path);
                    definitionTable[i]->standardDeviationDrainage = ReadIniFloat(section, "StandardDeviationDrainage", 0.5, path);
                    definitionTable[i]->meanAltitude = ReadIniFloat(section, "MeanAltitude", 0.5, path);
                    definitionTable[i]->standardDeviationAltitude = ReadIniFloat(section, "StandardDeviationAltitude", 0.5, path);
                    GetPrivateProfileString(section, "ProbabilityTga", "NONE", value, 0x80, path);
                    if (!strcmp(value, "NONE"))
                        definitionTable[i]->probabilityTga[0] = 0;
                    else
                        strcpy(definitionTable[i]->probabilityTga, value);
                }
            }
            northAngle = ReadIniFloat("EcoSystem", "NorthAngle", 0.0, path);
            GetPrivateProfileString("EcoSystem", "ProbabilityTga", "NONE", value, 0x80, path);
            if (!strcmp(value, "NONE")) {
                probabilityTga[0] = 0;
            } else {
                int valueLength = strlen(value);
                int length = valueLength > 0x7f ? 0x7f : valueLength;
                strncpy(probabilityTga, value, length);
                probabilityTga[length] = 0;
            }
        }
        return 1;
    }
    if (strstr(path, ".esb")) {
        ReadEsb(path, stream);
        return 1;
    }
    return 0;
}

// 0x00457ed0: builds the definition's collision objects as children.
// Not exact (1105 of 1153 bytes in place; docs/ECOSYSTEM.md).
void EcoSystem::BuildCollisionObjects(VegetationDefinition* definition) {
    Vector3 vertices[32];
    int indices[96];
    int i;
    int j;
    if (definition->collisionCount == 0) {
        definition->collisionObjects = 0;
        return;
    }
    int category = g_MemTagStack->Push("Collision");
    definition->collisionObjects = (CollisionObject**)DebugMalloc(definition->collisionCount * sizeof(CollisionObject*),
                                                             __FILE__, 0x3a5);
    for (i = 0; i < definition->collisionCount; i++) {
        VegetationCollisionDefinition* shape = &definition->collisionDefinitions[i];
        definition->collisionObjects[i] = new(__FILE__, 0x3ad) CollisionObject(1);
        definition->collisionObjects[i]->Configure(reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->display, 0, 0, 1);
        AppendChild(definition->collisionObjects[i], -1);
        definition->collisionObjects[i]->ownerType = 0x3e8;
        if (shape->type == 0) {
            definition->collisionObjects[i]->ownerType = 0x3e9;
            Vector3* hull = new(__FILE__, 0x3b8) Vector3[definition->modelVertexCount[0]];
            int* hullIndices = new(__FILE__, 0x3b9) int[definition->modelIndexCount[0]];
            for (j = 0; j < definition->modelIndexCount[0]; j++)
                hullIndices[j] = definition->modelIndices[0][j];
            for (j = 0; j < definition->modelVertexCount[0]; j++) {
                hull[j].x = definition->modelVertices[0][j].position.x;
                hull[j].y = definition->modelVertices[0][j].position.y;
                hull[j].z = definition->modelVertices[0][j].position.z;
            }
            definition->collisionObjects[i]->ownerType = 0x3e9;
            definition->collisionObjects[i]->SetTriangleMeshHullShape((CollisionVec3*)hull, hullIndices, definition->modelIndexCount[0] / 3,
                                                               definition->modelVertexCount[0]);
            EcoSetIdentity(&((EcoHullShape*)definition->collisionObjects[i]->shapeData)->transform);
            delete hull;
            delete hullIndices;
        } else if (shape->type == 2) {
            definition->collisionObjects[i]->SetSphereShape(*(CollisionVec3*)&shape->start, shape->radius);
        } else if (shape->type == 3) {
            definition->collisionObjects[i]->SetCapsuleShape(*(CollisionVec3*)&shape->start, *(CollisionVec3*)&shape->end, shape->radius);
        } else if (shape->type == 1) {
            definition->collisionObjects[i]->ownerType = 0x3e9;
            for (j = 0; j < 16; j++) {
                float angle = j * 0.39269909f;
                vertices[j].x = vertices[j + 16].x = (float)cos(angle) * shape->radius;
                vertices[j].z = vertices[j + 16].z = (float)sin(angle) * shape->radius;
                vertices[j].y = shape->height;
                vertices[j + 16].y = 0.0f;
            }
            for (j = 0; j < 32; j++)
                vertices[j] = EcoSum(shape->start, vertices[j]);
            for (j = 0; j < 16; j++) {
                int next = j + 1;
                if (next >= 16)
                    next = 0;
                indices[j * 6 + 0] = j;
                indices[j * 6 + 1] = next;
                indices[j * 6 + 2] = next + 16;
                indices[j * 6 + 3] = j;
                indices[j * 6 + 4] = next + 16;
                indices[j * 6 + 5] = j + 16;
            }
            definition->collisionObjects[i]->SetTriangleMeshHullShape((CollisionVec3*)vertices, indices, 32, 32);
            EcoSetIdentity(&((EcoHullShape*)definition->collisionObjects[i]->shapeData)->transform);
        }
    }
    g_MemTagStack->Pop(category);
}

// 0x00458360: reads a definition's "CollisionObject%i" entries. The keys
// count from 1 over a zero-based loop (`i + 1`), and the profile defaults
// are the empty string (the shared "" literal 0x00577738), not "NONE".
void EcoSystem::ReadCollisionObjects(const char* path, int index, VegetationDefinition* definition) {
    char key[0x80];
    char value[0x80];
    char section[0x100];
    char kind[0x80];
    int i;
    sprintf(section, "Vegetation_%d", index);
    int count = GetPrivateProfileInt(section, "NumCollisionObjects", 0, path);
    definition->collisionCount = count;
    if (count > 0) {
        definition->collisionDefinitions = (VegetationCollisionDefinition*)DebugMalloc(
            count * sizeof(VegetationCollisionDefinition), __FILE__, 0x40c);
        for (i = 0; i < definition->collisionCount; i++) {
            VegetationCollisionDefinition* shape = &definition->collisionDefinitions[i];
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
                shape->radius = ReadIniFloat(section, key, 0.0, path);
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
                shape->radius = ReadIniFloat(section, key, 0.0, path);
            } else if (!_stricmp(kind, "CYLINDER")) {
                shape->type = 1;
                sprintf(key, "CollisionObject%iBottom", i + 1);
                GetPrivateProfileString(section, key, "", value, 0x80, path);
                shape->start.x = (float)atof(strtok(value, ","));
                shape->start.y = (float)atof(strtok(0, ","));
                shape->start.z = (float)atof(strtok(0, "\n"));
                sprintf(key, "CollisionObject%iRadius", i + 1);
                shape->radius = ReadIniFloat(section, key, 0.0, path);
                sprintf(key, "CollisionObject%iHeight", i + 1);
                shape->height = ReadIniFloat(section, key, 0.0, path);
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

// 0x00458da0: writes the placed objects as text next to the .est.
// Not exact (457 of 461 bytes in place; docs/ECOSYSTEM.md).
void EcoSystem::WriteListing(const char* path) {
    char name[0x104];
    int i;
    int pathLength = strlen(path);
    int length = pathLength > 0x103 ? 0x103 : pathLength;
    strncpy(name, path, length);
    name[length] = 0;
    strcpy(strrchr(name, '.'), ".txt");
    FILE* file = fopen(name, "w");
    if (!file)
        return;
    FilePrintf(file, "%i\n", placedCount);
    for (i = 0; i < placedCount; i++) {
        VegetationDefinition* definition = definitionTable[vegetation[i].definitionIndex];
        float radius = definition->RadiusForParameter(vegetation[i].radiusParam);
        float height = definition->HeightForParameter(vegetation[i].heightParam);
        float z = vegetation[i].quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
        float y = vegetation[i].quantizedPosition.y * g_EcoSystem->unitsPerCoordinate;
        float x = vegetation[i].quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
        FilePrintf(file, "%i,%f,%f,%f,%f,%f\n", vegetation[i].definitionIndex, x, y, z,
                              definition->RadiusForParameter(vegetation[i].radiusParam),
                              definition->HeightForParameter(vegetation[i].heightParam));
    }
    fclose(file);
}

// 0x00458f70: reads the .esb (the stream's own, the archive's, or a new one).
int EcoSystem::ReadEsb(const char* path, FileStream* stream) {
    unsigned char present;
    unsigned char length;
    int i;
    esbStreamInArchive = 0;
    if (!stream) {
        UnknownResourceEntry* entry = g_ResourceManager->FindItem(path, 1);
        if (!entry) {
            stream = new(__FILE__, 0x520) FileStream(g_ResourceManager);
            if (!stream->Open(path, "rb", 0)) {
                if (stream)
                    delete stream;
                return 0;
            }
        } else {
            stream = entry->archive;
            esbStreamInArchive = 1;
        }
    } else {
        esbStreamInArchive = 1;
    }
    esbStream = stream;
    stream->Read(&method, 4, 1);
    for (i = 0; i < 256; i++) {
        stream->Read(&present, 1, 1);
        if (present) {
            int j;
            stream->Read(&length, 1, 1);
            definitionTable[i] = new(__FILE__, 0x543) VegetationDefinition;
            stream->Read(definitionTable[i]->name, length, 1);
            stream->Read(&length, 1, 1);
            stream->Read(definitionTable[i]->billboardName, length, 1);
            stream->Read(&definitionTable[i]->meanHeight, 4, 1);
            stream->Read(&definitionTable[i]->minHeight, 4, 1);
            stream->Read(&definitionTable[i]->maxHeight, 4, 1);
            stream->Read(&definitionTable[i]->meanRadius, 4, 1);
            stream->Read(&definitionTable[i]->minRadius, 4, 1);
            stream->Read(&definitionTable[i]->maxRadius, 4, 1);
            stream->Read(&definitionTable[i]->uLeft, 4, 1);
            stream->Read(&definitionTable[i]->uRight, 4, 1);
            stream->Read(&definitionTable[i]->vBottom, 4, 1);
            stream->Read(&definitionTable[i]->uCenter, 4, 1);
            stream->Read(&definitionTable[i]->vTop, 4, 1);
            stream->Read(&definitionTable[i]->usePlanarLighting, 4, 1);
            stream->Read(&definitionTable[i]->blendLods, 4, 1);
            stream->Read(&definitionTable[i]->keyColor, 4, 1);
            if (method == 2) {
                stream->Read(&definitionTable[i]->percentBias, 4, 1);
                stream->Read(&definitionTable[i]->meanSlope, 4, 1);
                stream->Read(&definitionTable[i]->standardDeviationSlope, 4, 1);
                stream->Read(&definitionTable[i]->meanAspect, 4, 1);
                stream->Read(&definitionTable[i]->standardDeviationAspect, 4, 1);
                stream->Read(&definitionTable[i]->meanDrainage, 4, 1);
                stream->Read(&definitionTable[i]->standardDeviationDrainage, 4, 1);
                stream->Read(&definitionTable[i]->meanAltitude, 4, 1);
                stream->Read(&definitionTable[i]->standardDeviationAltitude, 4, 1);
                stream->Read(&present, 1, 1);
                if (present) {
                    stream->Read(&length, 1, 1);
                    stream->Read(definitionTable[i]->probabilityTga, length, 1);
                }
            }
            stream->Read(&definitionTable[i]->collisionCount, 4, 1);
            definitionTable[i]->collisionDefinitions = (VegetationCollisionDefinition*)DebugMalloc(
                definitionTable[i]->collisionCount * sizeof(VegetationCollisionDefinition), __FILE__, 0x56d);
            for (j = 0; j < definitionTable[i]->collisionCount; j++) {
                stream->Read(&definitionTable[i]->collisionDefinitions[j].type, 4, 1);
                stream->Read(&definitionTable[i]->collisionDefinitions[j].start, 0xc, 1);
                stream->Read(&definitionTable[i]->collisionDefinitions[j].end, 0xc, 1);
                stream->Read(&definitionTable[i]->collisionDefinitions[j].radius, 4, 1);
                stream->Read(&definitionTable[i]->collisionDefinitions[j].height, 4, 1);
            }
        }
    }
    stream->Read(&present, 1, 1);
    if (present) {
        stream->Read(&length, 1, 1);
        stream->Read(placementBmp, length, 1);
    }
    if (method == 2) {
        stream->Read(&northAngle, 4, 1);
        stream->Read(&present, 1, 1);
        if (present) {
            stream->Read(&length, 1, 1);
            stream->Read(probabilityTga, length, 1);
        }
    }
    stream->Read(&placedCount, 4, 1);
    totalObjects = placedCount;
    return 1;
}

// 0x004594c0
void EcoSystem::SetDetailLevel(int level) {
    detailLevel = level;
}

// 0x004594d0
EcoSystem* EcoSystem::Create(void* view, TextureMapManager* textures, Terrain* terrain,
                                            LightManager* lights, char* path, FileStream* stream,
                                            int textureFormat, int collisions, int level) {
    int i;
    GameObject::SetRenderContext(view);
    g_UnknownGlobal59af14 = reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->bladeRenderer ? g_UnknownGlobal56a600 : g_UnknownGlobal56a740;
    SetDetailLevel(level);
    groundTerrain = terrain;
    textureManager = textures;
    lightManager = lights;
    billboardFormat = textureFormat;
    float size = g_collisionQuadTree->extentZ;
    float width = g_collisionQuadTree->extentX;
    if (width > size)
        size = width;
    coordinatesPerUnit = 65536.0f / size;
    unitsPerCoordinate = size * (1.0f / 65536.0f);
    ageManager = new(__FILE__, 0x5ca) AgeManager;
    LightingChanged();
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
    billboardVertices = (EcoLitVertex*)DebugMalloc(0x5a00, __FILE__, 0x5f1);
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
            GenerateObjects(IsFormat4444Or8888(textureFormat));
        WriteEsb(path);
    }
    return this;
}

// 0x004598d0: places the objects the .esb lists. The definition byte is
// widened to `int` right after its read (that hoists the zero-extension
// into a register and spills the loop counter as retail does).
int EcoSystem::PlaceStoredObjects() {
    FileStream* stream = esbStream;
    int i;
    for (i = 0; i < placedCount; i++) {
        unsigned char definitionByte;
        EcoQuantizedPosition coordinates;
        unsigned char heightParameter;
        unsigned char radiusParameter;
        stream->Read(&definitionByte, 1, 1);
        int definition = definitionByte;
        stream->Read(&coordinates, 6, 1);
        stream->Read(&heightParameter, 1, 1);
        stream->Read(&radiusParameter, 1, 1);
        vegetation[i].PlaceQuantized(textureManager, definition, &coordinates, heightParameter, radiusParameter);
        float radius = definitionTable[definition]->RadiusForParameter(radiusParameter);
        float height = definitionTable[definition]->HeightForParameter(heightParameter);
        float x = vegetation[i].quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
        float y = vegetation[i].quantizedPosition.y * g_EcoSystem->unitsPerCoordinate;
        float z = vegetation[i].quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
        unsigned int code = g_collisionQuadTree->ComputeCode((x - radius), (z - radius), (x + radius), (z + radius));
        g_MemTagStack->Push("QuadTree");
        g_collisionQuadTree->Insert(&vegetation[i], code, y, y + height);
        g_MemTagStack->Push("EcoSystem");
    }
    if (!esbStreamInArchive) {
        if (esbStream)
            delete esbStream;
        esbStream = 0;
    }
    return 1;
}

// 0x00459b40: places the objects the PlacementBmp paints (one pixel per
// terrain cell, the palette index selecting the definition). The cell
// sizes are `extent; /= width` (that gives retail's `fld; fidiv` over the
// shared conversion slot), the pixel pointer is taken before them, and the
// loop bounds re-read the bitmap header each iteration.
int EcoSystem::PlaceAuthoredObjects() {
    if (!placementBmp[0])
        return 0;
    BitmapFile* bitmap = ReadBitmap(placementBmp, 0);
    if (!bitmap)
        return 0;
    int row;
    int column;
    unsigned char* pixel = (unsigned char*)bitmap->bits;
    float cellX = g_collisionQuadTree->extentX;
    cellX /= bitmap->infoHeader.width;
    float cellZ = g_collisionQuadTree->extentZ;
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
                reinterpret_cast<Terrain*>(groundTerrain)->QueryGround(reinterpret_cast<TerrainVec3*>(&position), 0, 0, 0);
                unsigned char heightParameter = definitionTable[index]->RandomParameter();
                unsigned char radiusParameter = definitionTable[index]->ParameterForHeight(
                    definitionTable[index]->HeightForParameter(heightParameter));
                vegetation[placedCount].Place(textureManager, index, &position, heightParameter, radiusParameter);
                placedCount++;
            }
        }
    }
done:
    FreeBitmap(bitmap);
    return 1;
}

// A random number in 0..1 (inline shape only: the helper boundary is what
// keeps VC6 from folding the scale into the generator's other constants).
static inline float EcoRandomUnit() {
    return rand() * (1.0f / 32768.0f);
}

// The value a probability TGA holds at the terrain position (`x`, `z`):
// the green channel of a 32- or 24-bit image, the green field of a 16-bit
// one, scaled to 0..1. Inline shape only (the three branches are expanded
// at every sample site of the generator).
static inline float SampleEcoProbabilityTga(TgaFile* tga, float x, float z) {
    float worldX = g_collisionQuadTree->extentX;
    int width = tga->width;
    int column = (int)(x / worldX * width);
    float worldZ = g_collisionQuadTree->extentZ;
    int height = tga->height;
    int row = (int)((1.0f - z / worldZ) * height);
    if (column >= width)
        column = width - 1;
    if (row >= height)
        row = height - 1;
    int index = row * width + column;
    if (tga->bitsPerPixel == 32)
        return ((unsigned char*)tga->bits)[index * 4 + 1] * (1.0f / 255.0f);
    if (tga->bitsPerPixel == 24)
        return ((unsigned char*)tga->bits)[index * 3 + 1] * (1.0f / 255.0f);
    return ((((unsigned short*)tga->bits)[index] >> 5) & 0x1f) * (1.0f / 31.0f);
}

// SampleEcoProbabilityTga times `weight`, multiplied in retail's order: the
// channel times the weight first, then the 1/255 (1/31) scale. At 24-bit
// precision this rounds differently from scaling the channel first.
static inline float SampleEcoProbabilityTgaWeighted(TgaFile* tga, float x, float z, float weight) {
    float worldX = g_collisionQuadTree->extentX;
    int width = tga->width;
    int column = (int)(x / worldX * width);
    float worldZ = g_collisionQuadTree->extentZ;
    int height = tga->height;
    int row = (int)((1.0f - z / worldZ) * height);
    if (column >= width)
        column = width - 1;
    if (row >= height)
        row = height - 1;
    int index = row * width + column;
    if (tga->bitsPerPixel == 32)
        return ((unsigned char*)tga->bits)[index * 4 + 1] * weight * (1.0f / 255.0f);
    if (tga->bitsPerPixel == 24)
        return ((unsigned char*)tga->bits)[index * 3 + 1] * weight * (1.0f / 255.0f);
    return ((((unsigned short*)tga->bits)[index] >> 5) & 0x1f) * weight * (1.0f / 31.0f);
}

// Whether the probability TGA is nonzero at the terrain position.
static inline int EcoTgaIsSet(TgaFile* tga, float x, float z) {
    float worldX = g_collisionQuadTree->extentX;
    int width = tga->width;
    int column = (int)(x / worldX * width);
    float worldZ = g_collisionQuadTree->extentZ;
    int height = tga->height;
    int row = (int)((1.0f - z / worldZ) * height);
    if (column >= width)
        column = width - 1;
    if (row >= height)
        row = height - 1;
    int index = row * width + column;
    if (tga->bitsPerPixel == 32)
        return ((unsigned char*)tga->bits)[index * 4 + 1] != 0;
    if (tga->bitsPerPixel == 24)
        return ((unsigned char*)tga->bits)[index * 3 + 1] != 0;
    return (((unsigned short*)tga->bits)[index] & 0x3e0) != 0;
}

// 0x00459ce0: the Auto method. Places the authored objects, then draws
// random positions (in 5x5 / 6x6 "EcoGen" rings when the registry asks
// for them) until TotalObjects are placed or 10000 draws in a row failed.
// A draw picks a definition from the PercentBias table and must pass the
// probability TGAs, the slope and altitude bell curves and, with
// `checkCodes`, the QuadTree code test.
// Audited against retail (2026-11-11): rings, cluster pick, ComputeCode
// argument order, threshold, TGA clamps, slope/altitude terms, placement and
// free loops agree. Retail weights the definition TGA as
// (channel * probability) * 1/255 inside each format branch; the old source did
// (channel * 1/255) * probability. At 53-bit precision the first product is
// exact either way (8-bit channel times a 24-bit float), so both round the same
// exact triple product once and give identical floats. At 24-bit precision each
// product rounds: 20333 of the 65025 channel/probability pairs then differ by
// one ulp (exhaustive check, third emu_diff sweep); whether that ever flips a
// placement draw is open. The definition TGA is therefore weighted in retail's
// order (SampleEcoProbabilityTgaWeighted), at a small cost in linked score (VC6
// reallocates ebx/ebp across the body): behaviour over code shape.
// Not exact (366 of 3257 bytes in place; docs/ECOSYSTEM.md).
int EcoSystem::GenerateObjects(int checkCodes) {
    TgaFile* tgas[256];
    float definitionIndex[256];
    float cumulative[256];
    Vector3 outerRing[20];
    Vector3 innerRing[12];
    Vector3 normal;
    float minHeight;
    float maxHeight;
    float spacing;
    float rangeX;
    float rangeZ;
    int i;
    int j;
    PlaceAuthoredObjects();
    TgaFile* tga = 0;
    memset(tgas, 0, sizeof(tgas));
    if (probabilityTga[0])
        tga = LoadFile(probabilityTga, 0, (int)g_ResourceManager);
    for (i = 0; i < 256; i++) {
        if (definitionTable[i] && definitionTable[i]->probabilityTga[0])
            tgas[i] = LoadFile(definitionTable[i]->probabilityTga, 0, (int)g_ResourceManager);
    }
    groundTerrain->GetHeightRange(&minHeight, &maxHeight);
    unsigned int attempt = 0;
    float total = 0.0f;
    int count = 0;
    int failures = 0;
    for (i = 0; i < 256; i++) {
        if (definitionTable[i]) {
            definitionIndex[count] = (float)i;
            total += definitionTable[i]->percentBias;
            cumulative[count] = total;
            count++;
        }
    }
    int mode = reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->GetRegistryInt("EcoGen", 0);
    rangeX = g_collisionQuadTree->extentX;
    rangeZ = g_collisionQuadTree->extentZ;
    if (mode == 5) {
        spacing = g_collisionQuadTree->extentX * (1.0f / 5.0f);
        rangeX = rangeZ = spacing;
    } else if (mode == 6) {
        spacing = g_collisionQuadTree->extentX * (1.0f / 6.0f);
        rangeX = rangeZ = spacing;
    }
    if (mode) {
        int k = 0;
        for (i = 0; i < mode; i++) {
            outerRing[k++] = Vector3(i * spacing, 0.0f, 0.0f);
            outerRing[k++] = Vector3(i * spacing, 0.0f, (mode - 1) * spacing);
        }
        for (i = 1; i < mode - 1; i++) {
            outerRing[k++] = Vector3(0.0f, 0.0f, i * spacing);
            outerRing[k++] = Vector3((mode - 1) * spacing, 0.0f, i * spacing);
        }
        k = 0;
        for (i = 1; i < mode - 1; i++) {
            innerRing[k++] = Vector3(i * spacing - spacing, 0.0f, 0.0f);
            innerRing[k++] = Vector3(i * spacing - spacing, 0.0f, (mode - 2) * spacing - spacing);
        }
        for (i = 2; i < mode - 2; i++) {
            innerRing[k++] = Vector3(0.0f, 0.0f, i * spacing - spacing);
            innerRing[k++] = Vector3((mode - 2) * spacing - spacing, 0.0f, i * spacing - spacing);
        }
    }
    while (placedCount < totalObjects && failures < 10000) {
        Vector3 position;
        Vector3 probe;
        failures++;
        attempt++;
        int selected = (int)definitionIndex[count - 1];
        float draw = rand() * (100.0f / 32767.0f);
        for (i = 0; i < count; i++) {
            if (draw <= cumulative[i]) {
                selected = (int)definitionIndex[i];
                break;
            }
        }
        position.x = EcoRandomUnit() * rangeX;
        int clusterCount = 1;
        Vector3* cluster = 0;
        position.y = 0.0f;
        position.z = EcoRandomUnit() * rangeZ;
        if (mode == 5) {
            if (attempt % 3 == 0) {
                clusterCount = 16;
                cluster = outerRing;
            } else if (attempt % 3 == 1) {
                clusterCount = 8;
                position.x += spacing;
                cluster = innerRing;
                position.z += spacing;
            } else {
                clusterCount = 1;
                cluster = 0;
                position.x = spacing * 2.0f + position.x;
                position.z = spacing * 2.0f + position.z;
            }
        } else if (mode == 6) {
            if (attempt % 6 == 0) {
                clusterCount = 20;
                cluster = outerRing;
            } else if (attempt % 6 == 1) {
                clusterCount = 12;
                position.x += spacing;
                cluster = innerRing;
                position.z += spacing;
            } else {
                clusterCount = 1;
                cluster = 0;
                position.x = (position.x + spacing) * 2.0f;
                position.z = (position.z + spacing) * 2.0f;
            }
        }
        if (placedCount + clusterCount > totalObjects)
            continue;
        unsigned char heightParameter = definitionTable[selected]->RandomParameter();
        unsigned char radiusParameter = definitionTable[selected]->ParameterForHeight(
            definitionTable[selected]->HeightForParameter(heightParameter));
        float radius = definitionTable[selected]->RadiusForParameter(radiusParameter);
        float height = definitionTable[selected]->HeightForParameter(heightParameter);
        probe = position;
        unsigned int code = g_collisionQuadTree->ComputeCode(position.x - radius, position.z - radius,
                                                             radius + position.x, radius + position.z);
        if (checkCodes && !g_collisionQuadTree->IsValidCode(code))
            continue;
        // The named local keeps VC6 from folding the two scales into one.
        float random = EcoRandomUnit();
        float threshold = random * 3.0f;
        float probability;
        if (tga)
            probability = SampleEcoProbabilityTga(tga, probe.x, probe.z);
        else
            probability = 1.0f;
        if (probability <= threshold)
            continue;
        if (tgas[selected])
            probability = SampleEcoProbabilityTgaWeighted(tgas[selected], probe.x, probe.z, probability);
        if (probability <= threshold)
            continue;
        reinterpret_cast<Terrain*>(groundTerrain)->QueryGround(reinterpret_cast<TerrainVec3*>(&probe), reinterpret_cast<TerrainVec3*>(&normal), 0, 0);
        float slope = (float)acos(normal.y);
        float altitude = probe.y;
        if (slope < 0.0f)
            slope = 0.0f;
        slope *= (float)(2.0 / 3.14159265358979);
        probability *= g_SharedNormalDistribution->Lookup(slope, definitionTable[selected]->meanSlope,
                                                      definitionTable[selected]->standardDeviationSlope);
        if (probability <= threshold)
            continue;
        altitude = (altitude - minHeight) / (maxHeight - minHeight);
        probability *= g_SharedNormalDistribution->Lookup(altitude, definitionTable[selected]->meanAltitude,
                                                      definitionTable[selected]->standardDeviationAltitude);
        if (probability <= threshold)
            continue;
        for (j = 1; j < clusterCount; j++) {
            probe = Vector3(position.x + cluster[j].x, cluster[j].y, position.z + cluster[j].z);
            if (tga && !EcoTgaIsSet(tga, probe.x, probe.z))
                break;
            if (tgas[selected] && !EcoTgaIsSet(tgas[selected], probe.x, probe.z))
                break;
        }
        if (j != clusterCount)
            continue;
        for (j = 0; j < clusterCount; j++) {
            if (clusterCount == 1)
                probe = position;
            else
                probe = Vector3(position.x + cluster[j].x, cluster[j].y, position.z + cluster[j].z);
            reinterpret_cast<Terrain*>(groundTerrain)->QueryGround(reinterpret_cast<TerrainVec3*>(&probe), 0, 0, 0);
            code = g_collisionQuadTree->ComputeCode(probe.x - radius, probe.z - radius, probe.x + radius,
                                                    probe.z + radius);
            vegetation[placedCount].Place(textureManager, selected, &probe, heightParameter, radiusParameter);
            g_MemTagStack->Push("QuadTree");
            g_collisionQuadTree->Insert(&vegetation[placedCount], code, probe.y, probe.y + height);
            g_MemTagStack->Push("EcoSystem");
            placedCount++;
        }
        failures = 0;
    }
    if (tga)
        FreeFile(tga);
    for (i = 0; i < 256; i++) {
        if (tgas[i])
            FreeFile(tgas[i]);
    }
    return 1;
}

// 0x0045a9a0
void EcoSystem::LightingChanged() {
    LightEmitter* light = lightManager->FindLightOfType(6);
    if (light) {
        ambientLight.x = light->ambientRGBA[0];
        ambientLight.y = light->ambientRGBA[1];
        ambientLight.z = light->ambientRGBA[2];
    } else {
        ambientLight.z = 0.0f;
        ambientLight.y = 0.0f;
        ambientLight.x = 0.0f;
    }
    light = lightManager->FindLightOfType(4);
    if (!light) {
        light = lightManager->FindLightOfType(2);
        if (!light) {
            lightColor.z = 0.0f;
            lightColor.y = 0.0f;
            lightColor.x = 0.0f;
            return;
        }
    }
    lightColor.x = light->ambientRGBA[0];
    lightColor.y = light->ambientRGBA[1];
    lightColor.z = light->ambientRGBA[2];
    lightDirection.x = light->lightDirection.x;
    lightDirection.y = 0.0f;
    lightDirection.z = light->lightDirection.z;
    float length = EcoSquareMagnitude(&lightDirection);
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
int EcoSystem::PrepareGeometry() {
    if (!s_drawEnabled)
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
    g_UnknownGlobal59af00 = &ECO_VIEW->currentCamera->viewMatrix;
    depthScale = 65535.0f / ECO_VIEW->currentCamera->farPlane;
    depthUnit = ECO_VIEW->currentCamera->farPlane * (1.0f / 65535.0f);
    billboardCount = 0;
    geometryCount = 0;
    g_collisionQuadTree->RestartQuery();
    Vegetation* object = (Vegetation*)g_collisionQuadTree->NextObjectSorted();
    while (object) {
        if (object->objectTypeId == vegetationTypeId) {
            int billboard = object->isBillboard;
            int fade = object->fadeLevel;
            object->TestDistance(&billboard, &fade);
            VegetationDefinition* definition = definitionTable[object->definitionIndex];
            int visible = 1;
            if (!billboard) {
                float radius = definition->RadiusForParameter(object->radiusParam);
                float height = definition->HeightForParameter(object->heightParam) * 0.5;
                Vector3 center;
                if (radius <= height)
                    radius = height;
                center.x = object->quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
                center.y = object->quantizedPosition.y * g_EcoSystem->unitsPerCoordinate + height;
                center.z = object->quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
                visible = g_visibilityClipper->SphereInFrustum(
                    reinterpret_cast<const VisibilityCamera*>(ECO_VIEW->currentCamera),
                    reinterpret_cast<const VisibilityMatrix*>(&ECO_VIEW->currentCamera->screenMatrix),
                    reinterpret_cast<const VisibilityBoxVec*>(&center), radius, 0);
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
    s_prepareGeometryTicks = ReadClock() - start;
    g_MemTagStack->Pop(category);
    return 1;
}

// 0x0045ade0: the render and texture-stage states for `format`.
void EcoSystem::SetRenderStates(int format) {
    float alphaReference;
    if (format == 1555) {
        alphaReference = (float)(reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->bladeRenderer ? 0 : 0xc0);
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, 0, 0);
    } else if (format == 4444 || format == 8888) {
        alphaReference = 0.0f;
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, 0, 0);
    } else {
        alphaReference = 0.0f;
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, 1, 0);
    }
    ECO_VIEW->SetAlphaTest((int)alphaReference);
    if (g_UnknownGlobal59af14[detailLevel].stageLighting) {
        ECO_VIEW->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        ECO_VIEW->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        ECO_VIEW->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        if (g_UnknownGlobal59af14[detailLevel].specular)
            ECO_VIEW->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD, 0);
        else
            ECO_VIEW->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT, 0);
        if (format == 4444 || format == 8888) {
            ECO_VIEW->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            ECO_VIEW->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            ECO_VIEW->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        } else {
            ECO_VIEW->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ECO_VIEW->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        }
    } else {
        ECO_VIEW->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        ECO_VIEW->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT, 0);
        // Both arms are the same in retail (the second is reached by jump
        // threading from the format test below).
        if (format == 4444) {
            ECO_VIEW->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ECO_VIEW->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        } else {
            ECO_VIEW->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ECO_VIEW->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        }
    }
    if (format == 1555) {
        ECO_VIEW->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
        if (reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->bladeRenderer) {
            ECO_VIEW->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
            ECO_VIEW->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        } else {
            ECO_VIEW->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
            ECO_VIEW->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
        }
    } else if (format == 4444) {
        ECO_VIEW->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
        ECO_VIEW->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
        ECO_VIEW->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
    } else {
        ECO_VIEW->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
        ECO_VIEW->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
        ECO_VIEW->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
    }
}

// An xz direction normalised in place (the y component is cleared), zero
// when it has no length. Inline shape only.
static inline void EcoNormalizeXZ(Vector3* v) {
    float length = v->x * v->x + v->z * v->z;
    if (length == 0.0f) {
        *v = kVec3Zero;
    } else {
        length = FastInvSqrt(length);
        v->x = length * v->x;
        v->y = 0.0f;
        v->z = length * v->z;
    }
}

// The lit colour (no alpha) of a billboard face whose normal has the light
// intensity `intensity`. Inline shape only.
static inline float EcoMinOne(float v) {
    return v < 1.0f ? v : 1.0f;
}
static inline unsigned int EcoLitColor(const EcoSystem* eco, float intensity) {
    float r = EcoMinOne(intensity * eco->lightColor.x + eco->ambientLight.x);
    float g = EcoMinOne(intensity * eco->lightColor.y + eco->ambientLight.y);
    float b = EcoMinOne(intensity * eco->lightColor.z + eco->ambientLight.z);
    return ((int)(r * 255.0f) << 16) | ((int)(g * 255.0f) << 8) | (int)(b * 255.0f);
}

// The xz part of `v` scaled, returned by value (y is left unset; the
// normalise below clears it). Inline shape only: retail's lean keeps the
// x product on the x87 stack and spills the z product to a by-value
// temporary that is copied once more, which is what a returned vector
// passed by value to EcoLean produces.
static inline Vector3 EcoScaleXZ(const Vector3& v, float s) {
    Vector3 r;
    r.x = v.x * s;
    r.z = v.z * s;
    return r;
}

// `v` moved back by `lean` in the xz plane. Inline shape only.
static inline void EcoLean(Vector3* v, Vector3 lean) {
    v->x = v->x - lean.x;
    v->z = v->z - lean.z;
}

// The light falling on a face with normal `n` (the dot product is summed
// as z + (x + y), the order VC6 emits for retail; see EcoSquareMagnitude).
static inline float EcoLightIntensity(const EcoSystem* eco, const Vector3* n) {
    return -(n->z * eco->lightDirection.z + (n->x * eco->lightDirection.x + n->y * eco->lightDirection.y));
}

// 0x0045b060: slot 14, the draw. The geometry list is drawn object by
// object; the billboards are built 120 at a time into the vertex buffer as
// view-aligned quads (or, when the band lights them, as three lit faces
// folded towards the camera) and drawn in one call. The timings and counts
// go to the "EcoSystem" debug overlay page.
// Not exact (3714 of 3967 bytes in place; docs/ECOSYSTEM.md).
int EcoSystem::Render3D() {
    Matrix4 identity;
    int savedPerspective;
    int savedShadeMode;
    Vector3 right;
    Vector3 left;
    Vector3 back;
    int i;
    if (!s_drawEnabled)
        return 1;
    unsigned int start = ReadClock();
    int category = g_MemTagStack->Push("EcoSystem");
    int verticesBefore = ECO_VIEW->verticesDrawn;
    Camera* camera = ECO_VIEW->currentCamera;
    int trianglesBefore = ECO_VIEW->trianglesDrawn;
    float projectionScale = camera->focalLength;
    identity(0, 0) = 1.0f;
    identity(0, 1) = 0.0f;
    identity(0, 2) = 0.0f;
    identity(0, 3) = 0.0f;
    identity(1, 0) = 0.0f;
    identity(1, 1) = 1.0f;
    identity(1, 2) = 0.0f;
    identity(1, 3) = 0.0f;
    identity(2, 0) = 0.0f;
    identity(2, 1) = 0.0f;
    identity(2, 2) = 1.0f;
    identity(2, 3) = 0.0f;
    identity(3, 0) = 0.0f;
    identity(3, 1) = 0.0f;
    identity(3, 2) = 0.0f;
    identity(3, 3) = 1.0f;
    ECO_VIEW->currentCamera->SetWorldMatrix(&identity);
    ECO_VIEW->GetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, &savedPerspective);
    ECO_VIEW->GetRenderState(D3DRENDERSTATE_SHADEMODE, &savedShadeMode);
    // The three face normals: the quad's two sides and the face towards
    // the camera, the sides leaned back by a tenth of it.
    right.x = -camera->viewDirection.z;
    right.z = camera->viewDirection.x;
    left.x = camera->viewDirection.z;
    left.z = -camera->viewDirection.x;
    back.x = -camera->viewDirection.x;
    back.z = -camera->viewDirection.z;
    EcoLean(&right, EcoScaleXZ(back, 0.1f));
    EcoLean(&left, EcoScaleXZ(back, 0.1f));
    EcoNormalizeXZ(&right);
    EcoNormalizeXZ(&left);
    EcoNormalizeXZ(&back);
    // The light on each face.
    float rightLight = EcoLightIntensity(this, &right);
    if (rightLight < 0.0f)
        rightLight = 0.0f;
    float leftLight = EcoLightIntensity(this, &left);
    if (leftLight < 0.0f)
        leftLight = 0.0f;
    float backLight = EcoLightIntensity(this, &back);
    if (backLight < 0.0f)
        backLight = 0.0f;
    unsigned int backColor = EcoLitColor(this, backLight);
    unsigned int rightColor = EcoLitColor(this, rightLight);
    unsigned int leftColor = EcoLitColor(this, leftLight);
    float inverse = FastInvSqrtEstimate(camera->viewDirection.x * camera->viewDirection.x
                                          + camera->viewDirection.z * camera->viewDirection.z);
    float sine = inverse * camera->viewDirection.x;
    float cosine = inverse * camera->viewDirection.z;
    s_billboardsDrawn = 0;
    s_geometryDrawn = 0;
    if (reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->bladeRenderer)
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, g_UnknownGlobal59af14[detailLevel].fog, 0);
    if (geometryCount) {
        SetRenderStates(definitionTable[vegetation[0].definitionIndex]->modelTexture->mapFormat);
        for (i = 0; i < geometryCount; i++) {
            if (s_drawEnabled)
                geometryList[i]->DrawGeometry(ECO_VIEW);
            s_geometryDrawn++;
        }
    }
    s_geometryRenderTicks = ReadClock() - start;
    if (reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->bladeRenderer)
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0, 0);
    if (billboardCount) {
        definitionTable[vegetation[0].definitionIndex]->billboardTexture->Bind();
        SetRenderStates(definitionTable[vegetation[0].definitionIndex]->billboardTexture->mapFormat);
        int next = 0;
        int remaining = billboardCount;
        float widthScale = (float)(15.0 / ECO_VIEW->targetWidth);
        int stageLighting = g_UnknownGlobal59af14[detailLevel].stageLighting;
        float billboardRange = (float)g_UnknownGlobal59af14[detailLevel].billboardRange;
        int billboardLimit = g_UnknownGlobal59af14[detailLevel].billboardLimit;
        while (remaining) {
            int vertexCount = 0;
            int indexCount = 0;
            int offset0 = 0;
            int offset5 = 0xa0;
            int offset2 = 0x40;
            int offset1 = 0x20;
            int offset3 = 0x60;
            for (i = 0; i < remaining && i < 120; i++, next++) {
                Vegetation* object = billboardList[next];
                VegetationDefinition* definition = definitionTable[object->definitionIndex];
                float radius = definition->RadiusForParameter(object->radiusParam);
                int depthKey = (unsigned short)object->sortKey;
                float depth = depthKey * depthUnit;
                if (depth < billboardRange && radius * projectionScale / depth > 2.0f) {
                    float height = definition->HeightForParameter(object->heightParam);
                    float vTop = definition->vTop;
                    float uLeft = definition->uLeft;
                    float uRight = definition->uRight;
                    float uScale = (radius + radius) / (uRight - uLeft);
                    float uLeftWidth = (vTop - uLeft) * uScale;
                    float uRightWidth = (uRight - vTop) * uScale;
                    float x = object->quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
                    float y = object->quantizedPosition.y * g_EcoSystem->unitsPerCoordinate;
                    float z = object->quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
#define VERTEX(o) (*(EcoLitVertex*)((char*)billboardVertices + (o)))
                    VERTEX(offset0).position.x = uRightWidth * cosine + x;
                    VERTEX(offset3).position.x = VERTEX(offset0).position.x;
                    VERTEX(offset1).position.x = x - uLeftWidth * cosine;
                    VERTEX(offset2).position.x = VERTEX(offset1).position.x;
                    VERTEX(offset0).position.z = z - uRightWidth * sine;
                    VERTEX(offset0 + 0x60).position.z = VERTEX(offset0).position.z;
                    VERTEX(offset0 + 0x20).position.z = z + uLeftWidth * sine;
                    VERTEX(offset0 + 0x40).position.z = VERTEX(offset0 + 0x20).position.z;
                    VERTEX(offset0).position.y = height * definition->normalizedMinY + y;
                    VERTEX(offset0 + 0x20).position.y = VERTEX(offset0).position.y;
                    VERTEX(offset0 + 0x40).position.y = height + VERTEX(offset0).position.y;
                    VERTEX(offset0 + 0x60).position.y = VERTEX(offset0 + 0x40).position.y;
                    unsigned int alpha;
                    if (object->isBillboard >= definition->lodCount)
                        alpha = 0xff000000;
                    else
                        alpha = object->fadeLevel << 24;
                    int stride;
                    if (billboardLimit && ((stageLighting && depth < 20.0f) || depth * widthScale < radius)) {
                        VERTEX(offset5).position.x = x;
                        VERTEX(offset0 + 0x80).position.x = x;
                        VERTEX(offset0 + 0xa0).position.z = z;
                        VERTEX(offset0 + 0x80).position.z = z;
                        VERTEX(offset0 + 0x80).position.y = VERTEX(offset0).position.y;
                        VERTEX(offset0 + 0xa0).position.y = VERTEX(offset0 + 0x60).position.y;
                        VERTEX(offset0 + 0x40).tu = definition->uLeft;
                        VERTEX(offset0 + 0x20).tu = VERTEX(offset0 + 0x40).tu;
                        VERTEX(offset0 + 0xa0).tu = definition->vTop;
                        VERTEX(offset0 + 0x80).tu = VERTEX(offset0 + 0xa0).tu;
                        VERTEX(offset0 + 0x60).tu = definition->uRight;
                        VERTEX(offset0).tu = VERTEX(offset0 + 0x60).tu;
                        VERTEX(offset0 + 0xa0).tv = definition->uCenter;
                        VERTEX(offset0 + 0x60).tv = VERTEX(offset0 + 0xa0).tv;
                        VERTEX(offset0 + 0x40).tv = VERTEX(offset0 + 0x60).tv;
                        VERTEX(offset0 + 0x80).tv = definition->vBottom;
                        VERTEX(offset0 + 0x20).tv = VERTEX(offset0 + 0x80).tv;
                        VERTEX(offset0).tv = VERTEX(offset0 + 0x20).tv;
                        VERTEX(offset0 + 0x40).diffuse = alpha | rightColor;
                        VERTEX(offset0 + 0x20).diffuse = VERTEX(offset0 + 0x40).diffuse;
                        VERTEX(offset0 + 0x60).diffuse = alpha | leftColor;
                        VERTEX(offset0).diffuse = VERTEX(offset0 + 0x60).diffuse;
                        VERTEX(offset0 + 0xa0).diffuse = alpha | backColor;
                        VERTEX(offset0 + 0x80).diffuse = VERTEX(offset0 + 0xa0).diffuse;
                        billboardIndices[indexCount++] = vertexCount;
                        billboardIndices[indexCount++] = vertexCount + 4;
                        billboardIndices[indexCount++] = vertexCount + 5;
                        billboardIndices[indexCount++] = vertexCount;
                        billboardIndices[indexCount++] = vertexCount + 5;
                        billboardIndices[indexCount++] = vertexCount + 3;
                        billboardIndices[indexCount++] = vertexCount + 4;
                        billboardIndices[indexCount++] = vertexCount + 1;
                        billboardIndices[indexCount++] = vertexCount + 2;
                        billboardIndices[indexCount++] = vertexCount + 4;
                        billboardIndices[indexCount++] = vertexCount + 2;
                        billboardIndices[indexCount++] = vertexCount + 5;
                        vertexCount += 6;
                        stride = 0xc0;
                    } else {
                        VERTEX(offset0 + 0x60).tu = definition->uRight;
                        VERTEX(offset0).tu = VERTEX(offset0 + 0x60).tu;
                        VERTEX(offset0 + 0x40).tu = definition->uLeft;
                        VERTEX(offset0 + 0x20).tu = VERTEX(offset0 + 0x40).tu;
                        VERTEX(offset0 + 0x60).tv = definition->uCenter;
                        VERTEX(offset0 + 0x40).tv = VERTEX(offset0 + 0x60).tv;
                        VERTEX(offset0 + 0x20).tv = definition->vBottom;
                        VERTEX(offset0).tv = VERTEX(offset0 + 0x20).tv;
                        VERTEX(offset0 + 0x60).diffuse = alpha | backColor;
                        VERTEX(offset0).diffuse = VERTEX(offset0 + 0x60).diffuse;
                        VERTEX(offset0 + 0x40).diffuse = VERTEX(offset0).diffuse;
                        VERTEX(offset0 + 0x20).diffuse = VERTEX(offset0 + 0x40).diffuse;
                        billboardIndices[indexCount++] = vertexCount;
                        billboardIndices[indexCount++] = vertexCount + 1;
                        billboardIndices[indexCount++] = vertexCount + 2;
                        billboardIndices[indexCount++] = vertexCount;
                        billboardIndices[indexCount++] = vertexCount + 2;
                        billboardIndices[indexCount++] = vertexCount + 3;
                        vertexCount += 4;
                        stride = 0x80;
                    }
#undef VERTEX
                    offset3 += stride;
                    offset1 += stride;
                    offset2 += stride;
                    offset5 += stride;
                    offset0 += stride;
                    s_billboardsDrawn++;
                }
            }
            if (s_drawEnabled && vertexCount)
                ECO_VIEW->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, (int)billboardVertices, vertexCount,
                                               (int)billboardIndices, indexCount, 0);
            remaining -= i;
        }
    }
    s_renderTicks = ReadClock() - start;
    DebugOverlay* overlay = reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->debugOverlay;
    if (overlay) {
        if (s_debugPage < 0)
            { int page = overlay->nextPage++; s_debugPage = page; }
        s_prepareGeometryPeak.Record(s_prepareGeometryTicks);
        s_renderPeak.Record(s_renderTicks);
        reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->debugOverlay->Title(s_debugPage, "Ecosystem:%d objects", totalObjects);
        reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->debugOverlay->Print(s_debugPage, "Visible  2D %d 3D %d", billboardCount,
                                                       geometryCount);
        reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->debugOverlay->Print(s_debugPage, "Rendered 2D %d 3D %d",
                                                       s_billboardsDrawn, s_geometryDrawn);
        reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->debugOverlay->Print(s_debugPage, "Memory %d",
                                                       g_MemTagStack->GetBytes("EcoSystem"));
        reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->debugOverlay->Print(s_debugPage, "PrepareGeometry %d %d",
                                                       s_prepareGeometryTicks, s_prepareGeometryPeak.Peak());
        reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->debugOverlay->Print(s_debugPage, "Render %d(3d:%d) %d",
                                                       s_renderTicks, s_geometryRenderTicks,
                                                       s_renderPeak.Peak());
        reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->debugOverlay->Print(s_debugPage, "Total Transforms %d",
                                                       ECO_VIEW->verticesDrawn - verticesBefore);
        reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->debugOverlay->Print(s_debugPage, "Total Triangles  %d",
                                                       ECO_VIEW->trianglesDrawn - trianglesBefore);
    }
    g_MemTagStack->Pop(category);
    ECO_VIEW->SetRenderState(D3DRENDERSTATE_SHADEMODE, savedShadeMode, 0);
    ECO_VIEW->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, savedPerspective, 0);
    ECO_VIEW->SetTextureStageState(0, D3DTSS_MAGFILTER, reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->magFilter);
    ECO_VIEW->SetTextureStageState(0, D3DTSS_MINFILTER, reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->minFilter);
    ECO_VIEW->SetTextureStageState(0, D3DTSS_MIPFILTER, reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->mipFilter);
    ECO_VIEW->DisableAlphaTest();
    return GameObject::Render3D();
}

// 0x0045bfe0: the key toggling the drawing.
int EcoSystem::OnControlPress(ControlEvent* event, InputButtonState* entry) {
    if (GameObject::OnControlPress(event, entry))
        return 1;
    if (TestInputEvent(4, 0, event, 0x80)) {
        s_drawEnabled = 1 - s_drawEnabled;
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
int PreloadEcosystem(TextureMapManager* textures, char* path, FileStream* stream, int modelFlags) {
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
        FileStream* probe = new(__FILE__, 0xad7) FileStream(g_ResourceManager);
        if (probe->Open(esbName, "rb", 0))
            strcpy(strstr(path, ".est"), ".esb");
        if (probe)
            delete probe;
    }
    if (!strstr(path, ".esb"))
        return 0;
    {
        int collisionCount;
        FileStream* esb = stream;
        if (!esb) {
            UnknownResourceEntry* entry = g_ResourceManager->FindItem(path, 1);
            if (!entry) {
                esb = new(__FILE__, 0xaf8) FileStream(g_ResourceManager);
                if (!esb->Open(path, "rb", 0)) {
                    if (esb)
                        delete esb;
                    return 0;
                }
                ownsStream = 1;
            } else {
                esb = entry->archive;
            }
        }
        esb->Read(&method, 4, 1);
        for (i = 0; i < 256; i++) {
            esb->Read(&present, 1, 1);
            if (present) {
                esb->Read(&length, 1, 1);
                esb->Read(names[i], length, 1);
                esb->Read(&length, 1, 1);
                esb->Read(billboardNames[i], length, 1);
                esb->Seek(0x34, 1, 1);
                esb->Read(&keyColors[i], 4, 1);
                if (method == 2) {
                    esb->Seek(0x24, 1, 1);
                    esb->Read(&present, 1, 1);
                    if (present) {
                        esb->Read(&length, 1, 1);
                        esb->Seek(length, 1, 1);
                    }
                }
                esb->Read(&collisionCount, 4, 1);
                esb->Seek(collisionCount * sizeof(VegetationCollisionDefinition), 1, 1);
            }
        }
        if (ownsStream) {
            if (esb)
                delete esb;
        }
        for (i = 0; i < 256; i++) {
            if (billboardNames[i][0]) {
                TextureMap* texture = TerrainLoadTexture(textures, billboardNames[i], modelFlags, 0, 2, 5, 6, 0, 0x80,
                                                                   0xff00ff, 1, 1);
                if (!texture->IsResident()) {
                    texture->CreateTextureSurface(1, 0, 0);
                    g_UnknownGlobal59af10++;
                }
            }
            if (names[i][0]) {
                strcpy(sltName, names[i]);
                strcat(sltName, ".slt");
                FileStream* sltStream = new(__FILE__, 0xb50) FileStream(g_ResourceManager);
                if (sltStream->Open(sltName, "r", 0)) {
                    ParameterBlock* block = new(__FILE__, 0xb52) ParameterBlock;
                    block->Attach((FileStream*)sltStream, 0, 1);
                    block->SelectSection("Material - 0");
                    block->ReadString("TextureMap", sltName, -1);
                    int format = 1555;
                    if (!reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->bladeRenderer && (reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->display->driverColorKeyCaps & 8)
                        && reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->GetRegistryFlag("KeyColorTrees", 0))
                        format = reinterpret_cast<EcoTrackGameView*>(g_TrackGame)->display->colorKeyFormat;
                    TextureMap* texture = TerrainLoadTexture(textures, sltName, format, 0, 2, 5, 6, 0, 0x80, 0xff00ff, 1, 1);
                    if (!texture->IsResident()) {
                        g_UnknownGlobal59af10++;
                        int loaded = texture->mapFormat;
                        if (loaded == 555 || loaded == 565 || loaded == 888 || loaded == 1555)
                            texture->ApplyColorKey(keyColors[i]);
                        texture->CreateTextureSurface(1, 0, 0);
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
int WriteXorEncoded(const unsigned char* data, int size, int count, FILE* file, unsigned char* key) {
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
int ReadXorDecoded(unsigned char* data, int size, int count, FILE* file, unsigned char* key) {
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

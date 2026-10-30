// Bring-up definitions (best-guess-unreviewed) for EcoSystem methods that
// the linked game references but src/reconstructed/EcoSystem.cpp does not
// yet provide. Bodies are extracted from the near-miss candidates in
// samples/ecosystem/EcoSystemNearMisses.cpp (retail VAs noted per method);
// they are behaviorally faithful to the decoded retail functions but are
// not byte matches.
//
// best guess: EcoSystem.cpp keeps its debug counters, peaks and the detail
// band table file-static. This unit keeps its own copies: the detail band
// tables are constant and the selection (bladeRenderer ? 1555 : 4444)
// mirrors EcoSystem.cpp:801, the counters/peaks only feed the debug
// overlay. The slot-23 draw toggle (s_drawEnabled) is not shared.
#include "../krusty2/broadphase/Terrain.h"
#include "../reconstructed/TrackGameGlobal.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "../reconstructed/EcoSystem.h"

#include "../reconstructed/MemTag.h"
#include "../reconstructed/PeakHold.h"
#include "../reconstructed/D3DConstants.h"
#include "../reconstructed/Parameterblocks.h"
#include "../reconstructed/TypeRegistry.h"
#include "../reconstructed/UnknownResourceManager.h"
#include "../reconstructed/bmpfile.h"
#include "../reconstructed/Tgafile.h"

static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static int s_debugPage = -1;                 // 0x0056a128 (local copy)
static int s_drawEnabled = 1;                // 0x0056a12c (local copy)
static int s_geometryRenderTicks;            // 0x0059aec0 (local copy)
static UnknownPeakHold s_prepareGeometryPeak(5000); // 0x0059aef0 (local copy)
static UnknownPeakHold s_renderPeak(5000);   // 0x0059aec8 (local copy)
static int s_renderTicks;                    // 0x0059aee4 (local copy)
static int s_prepareGeometryTicks;           // 0x0059aee8 (local copy, never set here)
static int s_geometryDrawn;                  // 0x0059af04 (local copy)
static int s_billboardsDrawn;                // 0x0059af08 (local copy)
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

// best guess: EcoSystem.cpp:801 assigns 0x0059af14 from these two tables
// on bladeRenderer; recompute the same choice here.
static UnknownEcoDetailBand* UnknownEcoDetailBands() {
    return reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->bladeRenderer ? g_UnknownGlobal56a600
                                                                              : g_UnknownGlobal56a740;
}
#define g_UnknownGlobal59af14 (UnknownEcoDetailBands())

#define ECO_VIEW ((UnknownEcoRenderTarget*)renderContext)

// Bridges to the real providers (src/bringup/Group4_EcoBridges.cpp); the
// proxies EcoSystem.h declares mangle differently from them.
void Group4Eco_SetTriangleMeshHullShape(void* object, const void* vertices, const int* indices,
                                        int triangleCount, int vertexCount);
void Group4Eco_SetSphereShape(void* object, const void* center, float radius);
void Group4Eco_SetCapsuleShape(void* object, const void* p0, const void* p1, float radius);
void Group4Eco_QueryGround(void* terrain, void* position, void* normal, int flatShaded,
                           unsigned char* surface);
void Group4Eco_GetHeightRange(void* terrain, float* outMin, float* outMax);

// DebugOverlay.h's class, declared here with only the two cdecl formatters
// (0x00447f40 Print, 0x00447fa0 Title); EcoSystem.h's UnknownEcoDebugOverlay
// proxy names the same object.
class DebugOverlay {
public:
    void Print(int page, const char* format, ...);
    void Title(int page, const char* format, ...);
};
static inline DebugOverlay* Group4RealOverlay(UnknownEcoDebugOverlay* overlay) {
    return reinterpret_cast<DebugOverlay*>(overlay);
}

static inline void UnknownSetIdentity(Matrix4* m) {
    memset(m, 0, sizeof(Matrix4));
    (*m)(3, 3) = 1.0f;
    (*m)(2, 2) = 1.0f;
    (*m)(1, 1) = 1.0f;
    (*m)(0, 0) = 1.0f;
}

static inline float UnknownEcoZ(const Vector3& v) {
    return v.z;
}

static inline Vector3 UnknownEcoOffset(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + UnknownEcoZ(b));
}

// 0x00457480
int EcoSystem::ReadEst(char* path, UnknownTextureStream* stream) {
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
        UnknownTextureStream* probe = new(__FILE__, 0x2eb) UnknownTextureStream(g_ResourceManager);
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
                definitionTable[i] = new(__FILE__, 0x31f) UnknownEcoDefinition;
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
void EcoSystem::BuildCollisionObjects(UnknownEcoDefinition* definition) {
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
        UnknownEcoCollisionDefinition* shape = &definition->collisionDefinitions[i];
        definition->collisionObjects[i] = new(__FILE__, 0x3ad) CollisionObject(1);
        definition->collisionObjects[i]->Configure(reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->display, 0, 0, 1);
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
            Group4Eco_SetTriangleMeshHullShape(definition->collisionObjects[i], hull, hullIndices,
                                               definition->modelIndexCount[0] / 3, definition->modelVertexCount[0]);
            UnknownSetIdentity(&((UnknownEcoHullShape*)definition->collisionObjects[i]->shapeData)->transform);
            delete hull;
            delete hullIndices;
        } else if (shape->type == 2) {
            Group4Eco_SetSphereShape(definition->collisionObjects[i], &shape->start, shape->radius);
        } else if (shape->type == 3) {
            Group4Eco_SetCapsuleShape(definition->collisionObjects[i], &shape->start, &shape->end, shape->radius);
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
                vertices[j] = UnknownEcoOffset(shape->start, vertices[j]);
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
            Group4Eco_SetTriangleMeshHullShape(definition->collisionObjects[i], vertices, indices, 32, 32);
            UnknownSetIdentity(&((UnknownEcoHullShape*)definition->collisionObjects[i]->shapeData)->transform);
        }
    }
    g_MemTagStack->Pop(category);
}

// 0x00458da0: writes the placed objects as text next to the .est.
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
        UnknownEcoDefinition* definition = definitionTable[vegetation[i].definitionIndex];
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

// A random number in 0..1 (inline shape only: the helper boundary is what
// keeps VC6 from folding the scale into the generator's other constants).
static inline float UnknownEcoRandom() {
    return rand() * (1.0f / 32768.0f);
}

// The value a probability TGA holds at the terrain position (`x`, `z`):
// the green channel of a 32- or 24-bit image, the green field of a 16-bit
// one, scaled to 0..1. Inline shape only (the three branches are expanded
// at every sample site of the generator).
static inline float UnknownEcoSampleTga(UnknownTgaFile* tga, float x, float z) {
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

// Whether the probability TGA is nonzero at the terrain position.
static inline int UnknownEcoTgaIsSet(UnknownTgaFile* tga, float x, float z) {
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
int EcoSystem::GenerateObjects(int checkCodes) {
    UnknownTgaFile* tgas[256];
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
    UnknownTgaFile* tga = 0;
    memset(tgas, 0, sizeof(tgas));
    if (probabilityTga[0])
        tga = LoadFile(probabilityTga, 0, (int)g_ResourceManager);
    for (i = 0; i < 256; i++) {
        if (definitionTable[i] && definitionTable[i]->probabilityTga[0])
            tgas[i] = LoadFile(definitionTable[i]->probabilityTga, 0, (int)g_ResourceManager);
    }
    Group4Eco_GetHeightRange(groundTerrain, &minHeight, &maxHeight);
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
    int mode = reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->GetRegistryInt("EcoGen", 0);
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
        position.x = UnknownEcoRandom() * rangeX;
        int clusterCount = 1;
        Vector3* cluster = 0;
        position.y = 0.0f;
        position.z = UnknownEcoRandom() * rangeZ;
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
        float random = UnknownEcoRandom();
        float threshold = random * 3.0f;
        float probability;
        if (tga)
            probability = UnknownEcoSampleTga(tga, probe.x, probe.z);
        else
            probability = 1.0f;
        if (probability <= threshold)
            continue;
        if (tgas[selected])
            probability = UnknownEcoSampleTga(tgas[selected], probe.x, probe.z) * probability;
        if (probability <= threshold)
            continue;
        Group4Eco_QueryGround(groundTerrain, &probe, &normal, 0, 0);
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
            if (tga && !UnknownEcoTgaIsSet(tga, probe.x, probe.z))
                break;
            if (tgas[selected] && !UnknownEcoTgaIsSet(tgas[selected], probe.x, probe.z))
                break;
        }
        if (j != clusterCount)
            continue;
        for (j = 0; j < clusterCount; j++) {
            if (clusterCount == 1)
                probe = position;
            else
                probe = Vector3(position.x + cluster[j].x, cluster[j].y, position.z + cluster[j].z);
            Group4Eco_QueryGround(groundTerrain, &probe, 0, 0, 0);
            code = g_collisionQuadTree->ComputeCode(probe.x - radius, probe.z - radius, probe.x + radius,
                                                    probe.z + radius);
            vegetation[placedCount].Place(textureManager, selected, &probe, heightParameter, radiusParameter);
            g_MemTagStack->Push("QuadTree");
            g_collisionQuadTree->Insert(reinterpret_cast<QuadTreeObject*>(&vegetation[placedCount]), code, probe.y, probe.y + height);
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

// An xz direction normalised in place (the y component is cleared), zero
// when it has no length. Inline shape only.
static inline void UnknownEcoNormalizeXZ(Vector3* v) {
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
static inline float UnknownEcoMin1(float v) {
    return v < 1.0f ? v : 1.0f;
}
static inline unsigned int UnknownEcoLitColor(const EcoSystem* eco, float intensity) {
    float r = UnknownEcoMin1(intensity * eco->lightColor.x + eco->ambientLight.x);
    float g = UnknownEcoMin1(intensity * eco->lightColor.y + eco->ambientLight.y);
    float b = UnknownEcoMin1(intensity * eco->lightColor.z + eco->ambientLight.z);
    return ((int)(r * 255.0f) << 16) | ((int)(g * 255.0f) << 8) | (int)(b * 255.0f);
}

// The light falling on a face with normal `n` (the dot product is summed
// as z + (x + y), the order VC6 emits for retail; see UnknownSquareMagnitude).
static inline float UnknownEcoLightOn(const EcoSystem* eco, const Vector3* n) {
    return -(n->z * eco->lightDirection.z + (n->x * eco->lightDirection.x + n->y * eco->lightDirection.y));
}

// 0x0045b060: slot 14, the draw. The geometry list is drawn object by
// object; the billboards are built 120 at a time into the vertex buffer as
// view-aligned quads (or, when the band lights them, as three lit faces
// folded towards the camera) and drawn in one call. The timings and counts
// go to the "EcoSystem" debug overlay page.
int EcoSystem::UnknownVirtualSlot14() {
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
    UnknownEcoCamera* camera = ECO_VIEW->camera;
    int verticesBefore = ECO_VIEW->verticesDrawn;
    int trianglesBefore = ECO_VIEW->trianglesDrawn;
    float projectionScale = camera->projectionScale;
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
    ECO_VIEW->camera->SetWorldMatrix(&identity);
    ECO_VIEW->GetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, &savedPerspective);
    ECO_VIEW->GetRenderState(D3DRENDERSTATE_SHADEMODE, &savedShadeMode);
    // The three face normals: the quad's two sides and the face towards
    // the camera, the sides leaned back by a tenth of it.
    right.x = -camera->direction.z;
    right.z = camera->direction.x;
    left.x = camera->direction.z;
    left.z = -camera->direction.x;
    back.x = -camera->direction.x;
    back.z = -camera->direction.z;
    right.x = right.x - back.x * 0.1f;
    right.z = right.z - back.z * 0.1f;
    left.x = left.x - back.x * 0.1f;
    left.z = left.z - back.z * 0.1f;
    UnknownEcoNormalizeXZ(&right);
    UnknownEcoNormalizeXZ(&left);
    UnknownEcoNormalizeXZ(&back);
    // The light on each face.
    float rightLight = UnknownEcoLightOn(this, &right);
    if (rightLight < 0.0f)
        rightLight = 0.0f;
    float leftLight = UnknownEcoLightOn(this, &left);
    if (leftLight < 0.0f)
        leftLight = 0.0f;
    float backLight = UnknownEcoLightOn(this, &back);
    if (backLight < 0.0f)
        backLight = 0.0f;
    unsigned int backColor = UnknownEcoLitColor(this, backLight);
    unsigned int rightColor = UnknownEcoLitColor(this, rightLight);
    unsigned int leftColor = UnknownEcoLitColor(this, leftLight);
    float inverse = FastInvSqrtEstimate(camera->direction.x * camera->direction.x
                                          + camera->direction.z * camera->direction.z);
    float sine = inverse * camera->direction.x;
    float cosine = inverse * camera->direction.z;
    s_billboardsDrawn = 0;
    s_geometryDrawn = 0;
    if (reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->bladeRenderer)
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, g_UnknownGlobal59af14[detailLevel].fog, 0);
    if (geometryCount) {
        SetRenderStates(definitionTable[vegetation[0].definitionIndex]->modelTexture->pixelFormat);
        for (i = 0; i < geometryCount; i++) {
            if (s_drawEnabled)
                geometryList[i]->DrawGeometry(ECO_VIEW);
            s_geometryDrawn++;
        }
    }
    s_geometryRenderTicks = ReadClock() - start;
    if (reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->bladeRenderer)
        ECO_VIEW->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0, 0);
    if (billboardCount) {
        definitionTable[vegetation[0].definitionIndex]->billboardTexture->Bind();
        SetRenderStates(definitionTable[vegetation[0].definitionIndex]->billboardTexture->pixelFormat);
        int next = 0;
        int remaining = billboardCount;
        float widthScale = (float)(15.0 / ECO_VIEW->width);
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
            for (i = 0; i < remaining && i < 120; i++) {
                Vegetation* object = billboardList[next];
                UnknownEcoDefinition* definition = definitionTable[object->definitionIndex];
                float radius = definition->RadiusForParameter(object->radiusParam);
                int depthKey = (unsigned short)object->sortKey;
                float depth = depthKey * depthUnit;
                if (depth < billboardRange && radius * projectionScale / depth > 2.0f) {
                    float height = definition->HeightForParameter(object->heightParam);
                    float uScale = (definition->uRight - definition->uLeft) / (radius + radius);
                    float uLeftWidth = (definition->vTop - definition->uLeft) * uScale;
                    float uRightWidth = (definition->uRight - definition->vTop) * uScale;
                    float x = object->quantizedPosition.x * g_EcoSystem->unitsPerCoordinate;
                    float y = object->quantizedPosition.y * g_EcoSystem->unitsPerCoordinate;
                    float z = object->quantizedPosition.z * g_EcoSystem->unitsPerCoordinate;
#define VERTEX(o) (*(UnknownEcoVertex*)((char*)billboardVertices + (o)))
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
                    if (object->isBillboard < definition->lodCount)
                        alpha = object->fadeLevel << 24;
                    else
                        alpha = 0xff000000;
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
                next++;
            }
            if (s_drawEnabled && vertexCount)
                ECO_VIEW->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, billboardVertices, vertexCount,
                                               billboardIndices, indexCount, 0);
            remaining -= i;
        }
    }
    s_renderTicks = ReadClock() - start;
    UnknownEcoDebugOverlay* overlay = reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->debugOverlay;
    if (overlay) {
        if (s_debugPage < 0)
            s_debugPage = overlay->nextPage++;
        s_prepareGeometryPeak.Record(s_prepareGeometryTicks);
        s_renderPeak.Record(s_renderTicks);
        Group4RealOverlay(overlay)->Title(s_debugPage, "Ecosystem:%d objects", totalObjects);
        Group4RealOverlay(overlay)->Print(s_debugPage, "Visible  2D %d 3D %d", billboardCount,
                                                       geometryCount);
        Group4RealOverlay(overlay)->Print(s_debugPage, "Rendered 2D %d 3D %d",
                                                       s_billboardsDrawn, s_geometryDrawn);
        Group4RealOverlay(overlay)->Print(s_debugPage, "Memory %d",
                                                       g_MemTagStack->GetBytes("EcoSystem"));
        Group4RealOverlay(overlay)->Print(s_debugPage, "PrepareGeometry %d %d",
                                                       s_prepareGeometryTicks, s_prepareGeometryPeak.Peak());
        Group4RealOverlay(overlay)->Print(s_debugPage, "Render %d(3d:%d) %d",
                                                       s_renderTicks, s_geometryRenderTicks,
                                                       s_renderPeak.Peak());
        Group4RealOverlay(overlay)->Print(s_debugPage, "Total Transforms %d",
                                                       ECO_VIEW->verticesDrawn - verticesBefore);
        Group4RealOverlay(overlay)->Print(s_debugPage, "Total Triangles  %d",
                                                       ECO_VIEW->trianglesDrawn - trianglesBefore);
    }
    g_MemTagStack->Pop(category);
    ECO_VIEW->SetRenderState(D3DRENDERSTATE_SHADEMODE, savedShadeMode, 0);
    ECO_VIEW->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, savedPerspective, 0);
    ECO_VIEW->SetTextureStageState(0, D3DTSS_MAGFILTER, reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->magFilter);
    ECO_VIEW->SetTextureStageState(0, D3DTSS_MINFILTER, reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->minFilter);
    ECO_VIEW->SetTextureStageState(0, D3DTSS_MIPFILTER, reinterpret_cast<UnknownEcoTrackGame*>(g_TrackGame)->mipFilter);
    ECO_VIEW->DisableAlphaTest();
    return GameObject::UnknownVirtualSlot14();
}

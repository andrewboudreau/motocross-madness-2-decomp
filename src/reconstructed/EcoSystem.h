#pragma once

// EcoSystem.h -- the reconstructed D:\aardvark\VC\krusty2\EcoSystem.cpp
// (literal __FILE__ at 0x0056a130, 35 .text xrefs 0x00455ee6..0x0045c509;
// code 0x00455da0..0x0045c830 between EventManager.cpp's neighbours; the
// unit's EH funclets lie at 0x0054a880..0x0054aa32). See docs/ECOSYSTEM.md.
//
// RTTI (tier 1): EcoSystem : GameObject (+0), GroundFogableObject (+0x2c),
// vtable 0x00552508 (27 slots, overrides 0/12/14/23); Vegetation :
// QuadTreeObject, vtable 0x005524f8 (2 slots). The vegetation definition
// class has no RTTI; its 0x210-byte layout comes from the constructor
// 0x00455de0 and the loaders. Member names are provisional (tier 3) unless
// a literal names them ("NumberOfLOD", "MeanHeight", ...).

#include <stdio.h>
#include <float.h>

#include "GameObject.h"
#include "MatrixUtil.h"
#include "AgeEntry.h"
#include "AgeManager.h"
#include "DebugAlloc.h"

class EcoSystem;
class TextureMapManager;
#include "FileStream.h"

struct BitmapFile;
struct TgaFile;

#include "GroundFogableObject.h"

// Stand-ins for classes reconstructed elsewhere, limited to what this
// file touches (the canonical declarations live in src/krusty2 and would
// drag in the collision header tree).

#include "../krusty2/broadphase/QuadTreeObject.h"

#include "../krusty2/broadphase/QuadTreeDeclarations.h"

class Terrain;

// src/krusty2/collision/CollisionObject.h: CollisionObject (0xb8 bytes,
// QuadTreeObject at +0, GraphicsTest : GameObject at +0xc).
struct EcoHullShape {                // type 0
    unsigned char field_0x00[0xc8];
    Matrix4 transform;                        // +0xc8: the shape's own transform
};
struct EcoCapsuleShape {             // type 3
    unsigned char field_0x00[0x20];
    float radiusScale;                        // +0x20: radius scale
    float endpointScale;                      // +0x24: endpoint scale
    Matrix4 transform;                        // +0x28
};
struct EcoSphereShape {              // type 4
    unsigned char field_0x00[0x14];
    float radiusScale;                        // +0x14: radius scale
    float centreScale;                        // +0x18: centre scale
    Matrix4 transform;                        // +0x1c
};
#include "../krusty2/collision/CollisionObject.h"   // 0xb8 bytes (operator new at 0x00457f74)

#include "LightEmitter.h"

#include "Camera.h"

#include "TextureMap.h"

// 0x00460b50 / 0x00460c00 / 0x00460c70 (src/krusty2/math/FastMath.h).
float FastSqrt(float x);
float FastInvSqrt(float x);
float FastInvSqrtEstimate(float x);        // table estimate of 1 / sqrt(x), no Newton step

// A definition's model vertex as the .slt loader stores it (8 floats).
struct EcoModelVertex {
    Vector3 position;
    Vector3 normal;
    float tu;
    float tv;
};

// D3DLVERTEX (FVF 0x1e2) as the geometry blocks and the billboard buffer
// hold it.
struct EcoLitVertex {
    Vector3 position;
    int reserved;
    unsigned int diffuse;
    unsigned int specular;
    float tu;
    float tv;
};

// A placed object's quantised position (Vegetation+0x0c; the .esb stores
// the six bytes as they are).
#include "EcoCoordinates.h"

// One "CollisionObject%i" entry of a definition (0x24 bytes).
struct VegetationCollisionDefinition {
    int type;                                 // 0 GEOMETRY, 1 CYLINDER, 2 SPHERE, 3 RADIUSEDLINE
    Vector3 start;                            // centre / start
    Vector3 end;                              // end
    float radius;                             // radius
    float height;                             // height
};

// One vegetation kind ("Vegetation_%d" section of the .est file), 0x210
// bytes (operator new at 0x00457771 and 0x0045908b); no vtable.
class VegetationDefinition {
public:
    enum { kMaxLods = 1 };

    VegetationDefinition();                   // 0x00455de0
    ~VegetationDefinition();                  // 0x00455e80
    int RandomParameter();                    // 0x00455f50: a random parameter byte
    int ParameterForHeight(float height);     // 0x00455f60: a random parameter for `height`
    float HeightForParameter(unsigned char parameter);    // 0x00455f90: height for a parameter
    float RadiusForParameter(unsigned char parameter);    // 0x00455ff0: radius for a parameter
    // 0x00456050: loads the billboard texture and the .slt model.
    int LoadModel(TextureMapManager* textures, int modelFlags);

    char name[0x80];                          // name
    char billboardName[0x80];                 // BillboardName
    char probabilityTga[0x80];                // ProbabilityTga
    float meanHeight;                         // MeanHeight
    float minHeight;                          // MinHeight
    float maxHeight;                          // MaxHeight
    float meanRadius;                         // MeanRadius
    float minRadius;                          // MinRadius
    float maxRadius;                          // MaxRadius
    float meanSlope;                          // MeanSlope
    float standardDeviationSlope;             // StandardDeviationSlope
    float meanAspect;                         // MeanAspect
    float standardDeviationAspect;            // StandardDeviationAspect
    float meanDrainage;                       // MeanDrainage
    float standardDeviationDrainage;          // StandardDeviationDrainage
    float meanAltitude;                       // MeanAltitude
    float standardDeviationAltitude;          // StandardDeviationAltitude
    float uLeft;                              // ULeft
    float uRight;                             // URight
    float uCenter;                            // UCenter
    float vBottom;                            // VBottom
    float vTop;                               // VTop
    float modelHeightScale;                   // model height scale
    float modelRadiusScale;                   // model radius scale
    float normalizedMinY;                     // +0x1D4  the model's lowest y over its height (modelHeightScale * minY); the billboards start that far below the object
    int lodCount;                             // NumberOfLOD (at most kMaxLods)
    unsigned int keyColor;                    // key colour
    TextureMap* billboardTexture;             // billboard texture
    TextureMap* modelTexture;                 // model texture
    unsigned short* modelIndices[kMaxLods];   // model indices (inside the vertex block)
    EcoModelVertex* modelVertices[kMaxLods]; // model vertices
    int modelVertexCount[kMaxLods];           // vertex count
    int modelIndexCount[kMaxLods];            // index count
    int usePlanarLighting;                    // UsePlanarLighting
    int blendLods;                            // BlendLODs
    float percentBias;                        // PercentBias
    int collisionCount;                       // NumCollisionObjects
    VegetationCollisionDefinition* collisionDefinitions;
    CollisionObject** collisionObjects;
};

// RTTI: Vegetation : QuadTreeObject (vtable 0x005524f8), 0x1c bytes
// (0x00459656 allocates count * 0x1c).
#include "Vegetation.h"

// 0x00456850 (cdecl): the AgeManager eviction callback for a geometry block.
int EvictGeometry(void* object, int context);

// 0x0045c6a0 / 0x0045c7b0 (cdecl): fwrite / fread through a running
// one-byte xor key.
int WriteXorEncoded(const unsigned char* data, int size, int count, FILE* file, unsigned char* key);
int ReadXorDecoded(unsigned char* data, int size, int count, FILE* file, unsigned char* key);

#include "../krusty2/visibility/VisibilityClipperGlobal.h"        // 0x00575a98

#include "DebugOverlay.h"
unsigned int ReadClock();                     // 0x004bfa80
int TestInputEvent(int control, int kind, const ControlEvent* event, int modifier); // 0x0043caa0

// A detail band (0x20 bytes) of the two tables 0x0056a600 / 0x0056a740.
struct EcoDetailLevel {
    int geometryDistance;                     // 3D distance
    int fadeStartDistance;                    // fade start distance
    int modelFlags;                           // model flags
    int billboardRange;                       // billboard range
    int billboardLimit;                       // billboard limit
    int stageLighting;                        // texture-stage lighting
    int specular;                             // specular
    int fog;                                  // fog
};

class EcoSystem : public GameObject, public GroundFogableObject {
public:
    friend class Vegetation;
    explicit EcoSystem(int flags);            // 0x00457250
    virtual ~EcoSystem();                     // 0x00457330 (deleting wrapper 0x00457310)
    virtual int PrepareGeometry();            // 0x0045aad0: classifies the visible objects
    // 0x0045b060 (3967 bytes, one function; 0x0045b136 is not a start):
    // draws the geometry list, then the billboards 120 at a time.
    virtual int Render3D();
    virtual int OnControlPress(ControlEvent* event, InputButtonState* entry);       // 0x0045bfe0

    // 0x00457480: reads the .est file (or a .esb through 0x00458f70);
    // `path` is rewritten to the .esb name when one exists.
    int ReadEst(char* path, FileStream* stream);
    // 0x00457ed0: builds the collision objects of a definition.
    void BuildCollisionObjects(VegetationDefinition* definition);
    // 0x00458360: reads a definition's "CollisionObject%i" entries.
    void ReadCollisionObjects(const char* path, int index, VegetationDefinition* definition);
    int WriteEsb(const char* path);              // 0x004587b0: writes the .esb
    void WriteListing(const char* path);          // 0x00458da0: writes the .txt listing
    int ReadEsb(const char* path, FileStream* stream);               // 0x00458f70: reads the .esb
    void SetDetailLevel(int level);    // 0x004594c0: detail level
    // 0x004594d0: creates the ecosystem.
    EcoSystem* Create(void* view, TextureMapManager* textures, Terrain* terrain,
                                     LightManager* lights, char* path, FileStream* stream,
                                     int textureFormat, int collisions, int level);
    int PlaceStoredObjects();                 // 0x004598d0: places the objects read from the .esb
    int PlaceAuthoredObjects();               // 0x00459b40: places the authored objects
    int GenerateObjects(int seed);            // 0x00459ce0: generates the objects
    void LightingChanged();             // 0x0045a9a0: lighting changed
    void SetRenderStates(int format);         // 0x0045ade0: render states for a texture format

    int method;                               // Method: 1 Authored, 2 Auto
    int totalObjects;                         // TotalObjects
    Vegetation* vegetation;                   // the objects
    Vegetation** billboardList;               // objects drawn as billboards
    Vegetation** geometryList;                // objects drawn as geometry
    Terrain* groundTerrain;
    TextureMapManager* textureManager;
    LightManager* lightManager;
    EcoLitVertex* billboardVertices;      // billboard vertices (120 * 6)
    unsigned short* billboardIndices;         // billboard indices (120 * 12)
    VegetationDefinition* definitionTable[256];
    float northAngle;                         // NorthAngle
    char placementBmp[0x80];                  // PlacementBmp
    char probabilityTga[0x80];                // ProbabilityTga
    int placedCount;                          // placed object count
    int billboardFormat;                      // texture format
    Vector3 lightColor;                       // directional light colour
    Vector3 ambientLight;                     // ambient colour
    Vector3 lightDirection;                   // light direction
    unsigned char field_0x588[8];
    FileStream* esbStream;          // the .esb stream
    int esbStreamInArchive;                   // nonzero: esbStream belongs to the archive
    unsigned char field_0x598;
    char vegetationTypeId;                    // the Vegetation type id (TypeRegistry)
    AgeManager* ageManager;
    int billboardCount;                       // billboard count
    int geometryCount;                        // geometry count
    float unitsPerCoordinate;                 // world units per coordinate unit
    float coordinatesPerUnit;                 // coordinate units per world unit
    int billboardCapacity;                    // billboard list capacity
    int geometryCapacity;                     // geometry list capacity
    float depthScale;                         // 65535 / far distance
    float depthUnit;                          // far distance / 65536
    int detailLevel;                          // detail level
};

// 0x0059aebc: the EcoSystem instance; the constructor 0x00457250
// stores `this` and the destructor 0x00457330 clears it.
#include "EcoSystemGlobal.h"

// NormalDistribution.cpp (src/krusty2/effects): the tabulated bell curve
// the generator weighs the slope and altitude with (Lookup 0x004b0070).
#include "../krusty2/effects/NormalDistribution.h"

// 0x0045c040 (cdecl): loads every texture a .esb names ahead of the
// ecosystem itself.
int PreloadEcosystem(TextureMapManager* textures, char* path, FileStream* stream, int modelFlags);

// Helpers reconstructed elsewhere.
// 0x0047b8a0 (gameui.cpp tail): a float profile value, `defaultValue` when the key is "NONE".
float ReadIniFloat(const char* section, const char* key, double defaultValue, const char* path);
#include "FilePrintf.h"
BitmapFile* ReadBitmap(const char* path, BitmapFile* into); // 0x00424140
void FreeBitmap(BitmapFile* bitmap);                  // 0x004245b0
TgaFile* LoadFile(const char* path, TgaFile* into, int resources); // 0x005125c0
void FreeFile(TgaFile* tga);                        // 0x00512dd0
int IsFormat4444Or8888(int format);                                  // 0x00511ad0

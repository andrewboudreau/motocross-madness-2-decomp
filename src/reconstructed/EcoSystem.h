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

#include "TrackGameGlobal.h"
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

struct UnknownBitmapFile;
struct UnknownTgaFile;

#include "GroundFogableObject.h"

// Stand-ins for classes reconstructed elsewhere, limited to what this
// file touches (the canonical declarations live in src/krusty2 and would
// drag in the collision header tree).

// src/krusty2/collision/CollisionObject.h: QuadTreeObject.
class UnknownEcoQuadTreeObject {
public:
    virtual unsigned short UnknownVirtualSlot0();
    virtual unsigned int UnknownVirtualSlot1(class UnknownEcoQuadTree* tree);
    UnknownEcoQuadTreeObject() { queryStamp = 0; typeId = (char)0xff; }
    short queryStamp;                         // +0x04: query stamp (ctor 0)
    short sortKey;                            // +0x06: sort key, slot 0's result
    char typeId;                              // +0x08: object type id (ctor -1)
};

// src/krusty2/broadphase/Quadtree.h: QuadTree (the global 0x0068aba4).
class UnknownEcoQuadTree {
public:
    unsigned char field_0x00[0x50];
    float extentX;                            // +0x50: world x extent
    float extentZ;                            // +0x54: world z extent

    unsigned int ComputeCode(float x0, float z0, float x1, float z1); // 0x004dc8c0
    void Insert(UnknownEcoQuadTreeObject* object, unsigned int code, float y0, float y1); // 0x004dcac0
    UnknownEcoQuadTreeObject* NextObjectSorted(); // 0x004dd600
    void RestartQuery();                      // 0x004dd750
    int IsValidCode(unsigned int code);       // 0x004ddd90
};
extern UnknownEcoQuadTree* g_collisionQuadTree; // 0x0068aba4

// src/krusty2/broadphase/Terrain.h: Terrain.
class UnknownEcoTerrain {
public:
    // 0x00507c10: snaps position->y to the ground under (x, z).
    void QueryGround(Vector3* position, Vector3* normal, int flatShaded, unsigned char* surface);
    void GetHeightRange(float* outMin, float* outMax); // 0x00508970
};

// src/krusty2/collision/CollisionObject.h: CollisionObject (0xb8 bytes,
// QuadTreeObject at +0, GraphicsTest : GameObject at +0xc).
struct UnknownEcoHullShape {                // type 0
    unsigned char field_0x00[0xc8];
    Matrix4 transform;                        // +0xc8: the shape's own transform
};
struct UnknownEcoCapsuleShape {             // type 3
    unsigned char field_0x00[0x20];
    float radiusScale;                        // +0x20: radius scale
    float endpointScale;                      // +0x24: endpoint scale
    Matrix4 transform;                        // +0x28
};
struct UnknownEcoSphereShape {              // type 4
    unsigned char field_0x00[0x14];
    float radiusScale;                        // +0x14: radius scale
    float centreScale;                        // +0x18: centre scale
    Matrix4 transform;                        // +0x1c
};
class CollisionObject : public UnknownEcoQuadTreeObject, public GameObject {
public:
    explicit CollisionObject(int flags);      // 0x00431e70
    void Configure(void* a, int b, int c, int d);          // 0x004320f0
    void SetTriangleMeshHullShape(const Vector3* vertices, const int* indices, int triangleCount,
                               int vertexCount);                       // 0x004328b0: hull
    void SetSphereShape(Vector3 center, float radius);          // 0x004329a0: sphere
    void SetCapsuleShape(Vector3 p0, Vector3 p1, float radius);  // 0x00432a20: capsule
    void SetTransform(const Matrix4* m);                      // 0x00435830: SetTransform

    unsigned char field_0x38[0x50 - 0x38];
    int shapeType;                            // +0x50: 0 hull, 3 capsule, 4 sphere
    void* shapeData;                          // +0x54: the shape (by shapeType)
    unsigned char field_0x58[0x64 - 0x58];
    int ownerType;                            // +0x64
    unsigned char field_0x68[0xb8 - 0x68];    // 0xb8 bytes (operator new at 0x00457f74)
};

// LightEmitter.h: LightEmitter and LightManager, as far as the lighting
// update reads them.
class UnknownEcoLight {
public:
    unsigned char field_0x00[0x54];
    float colour[4];                          // +0x54
    unsigned char field_0x64[0x70 - 0x64];
    Vector3 direction;                        // +0x70
};
class LightManager {
public:
    UnknownEcoLight* FindLightOfType(int type); // 0x004a0190: first light of `type`
};

// RenderTarget.h: the view at GameObject+0x18 and its Camera (+0x08).
// Slots 27-32 carry Camera.h's names (Camera introduces slots 27-32).
class UnknownEcoCamera {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16();
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual void UnknownVirtualSlot22();
    virtual void UnknownVirtualSlot23();
    virtual void UnknownVirtualSlot24();
    virtual void UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
    virtual void TakeScreenshot();
    virtual void UpdateMatrices();
    virtual void LookAt();
    virtual int SetWorldMatrix(const Matrix4* value);
    virtual int SetViewMatrix(const Matrix4* value);
    virtual int SetProjectionMatrix(const Matrix4* value);

    unsigned char field_0x004[0xac - 4];
    Matrix4 viewMatrix;                       // +0xac
    unsigned char field_0xec[0x170 - 0xec];   // the clipper's matrix
    Vector3 position;                         // +0x170
    Vector3 direction;                        // +0x17c
    unsigned char field_0x188[0x198 - 0x188];
    float projectionScale;                    // +0x198: the billboard size test
    unsigned char field_0x19c[0x1c0 - 0x19c];
    float farDistance;                        // +0x1c0
};
// Slots 6-9, 15 and 18 are named from the D3D arguments their callers pass
// (D3DTSS_*/D3DRENDERSTATE_* states, D3DPT_/D3DFVF_ index draws, the alpha
// reference); the names follow the IDirect3DDevice7 calls they forward to.
class UnknownEcoRenderTarget {
public:
    virtual void UnknownVirtualSlot0();
    virtual int BeginScene();
    virtual int EndScene();
    virtual int Blit(void* a, void* b, void* c, int d);
    virtual void* LockSurface(void* rect, long* pitch, int flags);
    virtual int UnlockSurface(void* rect);
    virtual long GetTextureStageState(int stage, int type, int* value);
    virtual long SetTextureStageState(int stage, int type, int value);
    virtual void SetRenderState(int state, int value, int force);
    virtual long GetRenderState(int state, int* value);
    virtual void SetTextureBlend(int mode, int flag);
    virtual long ClearTexture(int stage);
    virtual int Clear(const void* rect, int flags);
    virtual int SupportsTextureFormat(int a);
    virtual int SetViewport(void* viewport);
    virtual int DrawIndexedPrimitive(int primitive, int format, void* vertices, int vertexCount,
                                     void* indices, int indexCount, int flags);
    virtual int DrawPrimitive(int a, int b, int c, int d, int e);
    virtual int DrawIndexedPrimitiveVB(int a, int b, int c, int d, int e, int f, int g);
    virtual void SetAlphaReference(int value);
    virtual void DisableAlphaTest();

    unsigned char field_0x04[4];
    UnknownEcoCamera* camera;                 // +0x08
    int width;                                // +0x0c
    unsigned char field_0x10[0x38 - 0x10];
    int verticesDrawn;                        // +0x38
    unsigned char field_0x3c[0x44 - 0x3c];
    int trianglesDrawn;                       // +0x44
};

// TextureMap.h: the texture objects the definitions hold (0x0050a590, Texmap.cpp,
// returns a TextureMap). Slots 1-3 are BaseObject's and 4-19 TextureMap.h's.
class UnknownEcoTexture {
public:
    virtual void UnknownVirtualSlot0();
    virtual int AddRef();
    virtual int Release();
    virtual int GetRefCount();
    virtual int CreateSurface();
    virtual int LoadSurface();
    virtual int Clone();
    virtual int IsResident();
    virtual int CreateTextureSurface(int a, int b, int c);
    virtual int Upload();
    virtual int ReleaseTextureSurface();
    virtual int SetAsTexture();
    virtual int RecreateIfLost();
    virtual int LockSurface();
    virtual int UnlockSurface();
    virtual int BuildMipLevels();
    virtual int LockLevel();
    virtual int UnlockLevel();
    virtual int ApplyColorKey(unsigned int key);
    virtual int Bind();
    unsigned char field_0x04[0x20 - 4];
    int pixelFormat;                          // +0x20
};
// 0x0050a590 (TextureMap.h): loads a texture through the manager.
#include "TextureLoader.h"

// 0x00460b50 / 0x00460c00 / 0x00460c70 (src/krusty2/math/FastMath.h).
float FastSqrt(float x);
float FastInvSqrt(float x);
float FastInvSqrtEstimate(float x);        // table estimate of 1 / sqrt(x), no Newton step

// A definition's model vertex as the .slt loader stores it (8 floats).
struct UnknownEcoModelVertex {
    Vector3 position;
    Vector3 normal;
    float tu;
    float tv;
};

// D3DLVERTEX (FVF 0x1e2) as the geometry blocks and the billboard buffer
// hold it.
struct UnknownEcoVertex {
    Vector3 position;
    int reserved;
    unsigned int diffuse;
    unsigned int specular;
    float tu;
    float tv;
};

// A placed object's quantised position (Vegetation+0x0c; the .esb stores
// the six bytes as they are).
struct UnknownEcoCoordinates {
    unsigned short x;
    unsigned short y;
    unsigned short z;
};

// One "CollisionObject%i" entry of a definition (0x24 bytes).
struct UnknownEcoCollisionDefinition {
    int type;                                 // 0 GEOMETRY, 1 CYLINDER, 2 SPHERE, 3 RADIUSEDLINE
    Vector3 start;                            // centre / start
    Vector3 end;                              // end
    float radius;                             // radius
    float height;                             // height
};

// One vegetation kind ("Vegetation_%d" section of the .est file), 0x210
// bytes (operator new at 0x00457771 and 0x0045908b); no vtable.
class UnknownEcoDefinition {
public:
    enum { kMaxLods = 1 };

    UnknownEcoDefinition();                   // 0x00455de0
    ~UnknownEcoDefinition();                  // 0x00455e80
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
    UnknownEcoTexture* billboardTexture;      // billboard texture
    UnknownEcoTexture* modelTexture;          // model texture
    unsigned short* modelIndices[kMaxLods];   // model indices (inside the vertex block)
    UnknownEcoModelVertex* modelVertices[kMaxLods]; // model vertices
    int modelVertexCount[kMaxLods];           // vertex count
    int modelIndexCount[kMaxLods];            // index count
    int usePlanarLighting;                    // UsePlanarLighting
    int blendLods;                            // BlendLODs
    float percentBias;                        // PercentBias
    int collisionCount;                       // NumCollisionObjects
    UnknownEcoCollisionDefinition* collisionDefinitions;
    CollisionObject** collisionObjects;
};

// RTTI: Vegetation : QuadTreeObject (vtable 0x005524f8), 0x1c bytes
// (0x00459656 allocates count * 0x1c).
class Vegetation : public UnknownEcoQuadTreeObject {
public:
    Vegetation();                             // 0x00456650
    virtual unsigned short UnknownVirtualSlot0(); // 0x00456690: view depth as a sort key
    virtual unsigned int UnknownVirtualSlot1(UnknownEcoQuadTree* tree); // 0x00456720: cell code

    // 0x004567a0 / 0x004567e0: place the object (quantised or world
    // coordinates); the first argument (EcoSystem::textureManager) is not used.
    void PlaceQuantized(TextureMapManager* textures, unsigned char definition,
                               const UnknownEcoCoordinates* coordinates, unsigned char heightParameter,
                               unsigned char radiusParameter);
    void Place(TextureMapManager* textures, unsigned char definition,
                               const Vector3* position, unsigned char heightParameter,
                               unsigned char radiusParameter);
    // 0x00456890: distance test against the current detail band.
    void TestDistance(int* billboard, int* fade);
    // 0x00456a10: switches between the 3D geometry and the billboard.
    void SetBillboard(int billboard, int fade);
    void DrawGeometry(UnknownEcoRenderTarget* target);          // 0x00457000: draws the geometry
    int GetCollisionCount();                  // 0x00457080: the definition's collision count
    // 0x004570a0: the definition's collision object `index`, placed here.
    CollisionObject* GetCollisionObject(int index);
    float GetRadius();                        // 0x00457230: the radius

    UnknownEcoCoordinates quantizedPosition;  // in EcoSystem::unitsPerCoordinate units
    unsigned char definitionIndex;            // definition index
    unsigned char fadeLevel;                  // fade (0..255)
    unsigned char heightParam;                // height parameter
    unsigned char radiusParam;                // radius parameter
    unsigned char isBillboard : 1;            // drawn as a billboard
    void* geometryBlock;                      // geometry block (vertices, indices, AgeEntry)
};

// 0x00456850 (cdecl): the AgeManager eviction callback for a geometry block.
int EvictGeometry(void* object, int context);

// 0x0045c6a0 / 0x0045c7b0 (cdecl): fwrite / fread through a running
// one-byte xor key.
int WriteXorEncoded(const unsigned char* data, int size, int count, FILE* file, unsigned char* key);
int ReadXorDecoded(unsigned char* data, int size, int count, FILE* file, unsigned char* key);

// Griddraw.h / VisibilityQuadTree.h: the visibility clipper 0x00575a98.
class UnknownEcoVisibilityClipper {
public:
    int SphereInFrustum(const UnknownEcoCamera* camera, const void* matrix, const Vector3* center,
                        float radius, int* fullyInside);       // 0x0052fbb0
};
#include "../krusty2/visibility/VisibilityClipperGlobal.h"        // 0x00575a98

// TrackGame.h: the game object 0x0056e26c, as far as this file reads it
// (slot 22 is the registry query).
struct UnknownEcoDisplay {
    unsigned char field_0x000[0x28];
    int colorKeyFormat;                       // +0x28: colour-key texture format
    unsigned char field_0x02c[0x1c0 - 0x2c];
    int capabilityBits;                       // +0x1c0
};
// DebugOverlay.h: the overlay the draw prints its statistics on.
class UnknownEcoDebugOverlay {
public:
    // 0x00447f40 / 0x00447fa0: format a row onto page `page` (cdecl, `this`
    // on the stack).
    void Print(int page, const char* format, ...);
    void Title(int page, const char* format, ...);
    unsigned char field_0x0000[0x26c0];
    int nextPage;                             // +0x26c0 next free page (DebugOverlay.h)
};
class UnknownEcoTrackGame {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16();
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual int GetRegistryInt(const char* name, int defaultValue);       // Game.h slot 20
    virtual float GetRegistryFloat(const char* name, float defaultValue); // Game.h slot 21
    virtual int GetRegistryFlag(const char* name, int defaultValue); // registry value (Game slot 22)

    unsigned char field_0x004[0x10 - 4];
    UnknownEcoDisplay* display;               // +0x10
    unsigned char field_0x014[0x38 - 0x14];
    UnknownEcoDebugOverlay* debugOverlay;     // +0x38 the debug overlay (DebugOverlay.h), or 0
    unsigned char field_0x03c[0x2d0 - 0x3c];
    int bladeRenderer;                        // +0x2d0 the "Blade" software renderer (Game.h)
    unsigned char field_0x2d4[0x54c - 0x2d4];
    int magFilter;                            // +0x54C  PCGame magFilter; PCGame 0x004c0960 picks it from the device caps
    int minFilter;                            // +0x550  PCGame minFilter
    int mipFilter;                            // +0x554  PCGame mipFilter
};
unsigned int ReadClock();                     // 0x004bfa80
int TestInputEvent(int control, int kind, const UnknownControlEvent* event, int modifier); // 0x0043caa0

// A detail band (0x20 bytes) of the two tables 0x0056a600 / 0x0056a740.
struct UnknownEcoDetailBand {
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
    virtual int UnknownVirtualSlot14();
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);       // 0x0045bfe0

    // 0x00457480: reads the .est file (or a .esb through 0x00458f70);
    // `path` is rewritten to the .esb name when one exists.
    int ReadEst(char* path, UnknownTextureStream* stream);
    // 0x00457ed0: builds the collision objects of a definition.
    void BuildCollisionObjects(UnknownEcoDefinition* definition);
    // 0x00458360: reads a definition's "CollisionObject%i" entries.
    void ReadCollisionObjects(const char* path, int index, UnknownEcoDefinition* definition);
    int WriteEsb(const char* path);              // 0x004587b0: writes the .esb
    void WriteListing(const char* path);          // 0x00458da0: writes the .txt listing
    int ReadEsb(const char* path, UnknownTextureStream* stream);               // 0x00458f70: reads the .esb
    void SetDetailLevel(int level);    // 0x004594c0: detail level
    // 0x004594d0: creates the ecosystem.
    EcoSystem* Create(void* view, TextureMapManager* textures, UnknownEcoTerrain* terrain,
                                     LightManager* lights, char* path, UnknownTextureStream* stream,
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
    UnknownEcoTerrain* groundTerrain;
    TextureMapManager* textureManager;
    LightManager* lightManager;
    UnknownEcoVertex* billboardVertices;      // billboard vertices (120 * 6)
    unsigned short* billboardIndices;         // billboard indices (120 * 12)
    UnknownEcoDefinition* definitionTable[256];
    float northAngle;                         // NorthAngle
    char placementBmp[0x80];                  // PlacementBmp
    char probabilityTga[0x80];                // ProbabilityTga
    int placedCount;                          // placed object count
    int billboardFormat;                      // texture format
    Vector3 lightColor;                       // directional light colour
    Vector3 ambientLight;                     // ambient colour
    Vector3 lightDirection;                   // light direction
    unsigned char field_0x588[8];
    UnknownTextureStream* esbStream;          // the .esb stream
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
extern EcoSystem* g_EcoSystem;

// NormalDistribution.cpp (src/krusty2/effects): the tabulated bell curve
// the generator weighs the slope and altitude with. The pointer at
// 0x0056ece0 holds 0x006886e8, that file's static object, and 0x004b0070
// is its Lookup; only the method EcoSystem calls is declared here.
class NormalDistribution {
public:
    float Lookup(float value, float mean, float sigma); // 0x004b0070
};
extern NormalDistribution* g_SharedNormalDistribution;

// 0x0045c040 (cdecl): loads every texture a .esb names ahead of the
// ecosystem itself.
int PreloadEcosystem(TextureMapManager* textures, char* path, UnknownTextureStream* stream, int modelFlags);

// Helpers reconstructed elsewhere.
// 0x0047b8a0 (gameui.cpp tail): a float profile value, `defaultValue` when the key is "NONE".
float ReadIniFloat(const char* section, const char* key, double defaultValue, const char* path);
void FilePrintf(FILE* file, const char* format, ...); // 0x00461d40: fprintf
UnknownBitmapFile* ReadBitmap(const char* path, UnknownBitmapFile* into); // 0x00424140
void FreeBitmap(UnknownBitmapFile* bitmap);                  // 0x004245b0
UnknownTgaFile* LoadFile(const char* path, UnknownTgaFile* into, int a); // 0x005125c0
void FreeFile(UnknownTgaFile* tga);                        // 0x00512dd0
int IsFormat4444Or8888(int format);                                  // 0x00511ad0

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
#include "AgeManager.h"
#include "DebugAlloc.h"

class EcoSystem;
class TextureMapManager;
class UnknownParameterStream;

// TextureMap.h: UnknownTextureStream, as far as this file uses it.
class UnknownTextureStream {
public:
    explicit UnknownTextureStream(class UnknownResourceManager* manager); // 0x00460d10
    ~UnknownTextureStream();                  // 0x00460d60
    int UnknownFunction460f50(const char* path, const char* mode, int a); // 0x00460f50: opens
    int UnknownFunction461340(int offset, int a, int origin);            // 0x00461340: seeks
    int UnknownFunction461640(void* buffer, int size, int count);        // 0x00461640: reads

    unsigned char field_0x000[0x134];         // 0x134 bytes (operator new at 0x00458fcb)
};
struct UnknownBitmapFile;
struct UnknownTgaFile;

// Second direct base at +0x2c (RTTI .?AVGroundFogableObject@@, no vfptr);
// the shared ctor body 0x004aae20 stores one zero dword
// (src/krusty2/broadphase/Terrain.h declares the same class).
class GroundFogableObject {
public:
    GroundFogableObject();                    // 0x004aae20
    int field_0x00;
};

// Stand-ins for classes reconstructed elsewhere, limited to what this
// file touches (the canonical declarations live in src/krusty2 and would
// drag in the collision header tree).

// src/krusty2/collision/CollisionObject.h: QuadTreeObject.
class UnknownEcoQuadTreeObject {
public:
    virtual unsigned short UnknownVirtualSlot0();
    virtual unsigned int UnknownVirtualSlot1(class UnknownEcoQuadTree* tree);
    UnknownEcoQuadTreeObject() { field_0x04 = 0; field_0x08 = (char)0xff; }
    short field_0x04;                         // query stamp (ctor 0)
    short field_0x06;                         // sort key, slot 0's result
    char field_0x08;                          // object type id (ctor -1)
};

// src/krusty2/broadphase/Quadtree.h: QuadTree (the global 0x0068aba4).
class UnknownEcoQuadTree {
public:
    unsigned char field_0x00[0x50];
    float field_0x50;                         // world x extent
    float field_0x54;                         // world z extent

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
    Matrix4 field_0xc8;                       // the shape's own transform
};
struct UnknownEcoCapsuleShape {             // type 3
    unsigned char field_0x00[0x20];
    float field_0x20;                         // radius scale
    float field_0x24;                         // endpoint scale
    Matrix4 field_0x28;
};
struct UnknownEcoSphereShape {              // type 4
    unsigned char field_0x00[0x14];
    float field_0x14;                         // radius scale
    float field_0x18;                         // centre scale
    Matrix4 field_0x1c;
};
class CollisionObject : public UnknownEcoQuadTreeObject, public GameObject {
public:
    explicit CollisionObject(int flags);      // 0x00431e70
    void UnknownFunction4320f0(void* a, int b, int c, int d);          // 0x004320f0
    void UnknownFunction4328b0(const Vector3* vertices, const int* indices, int triangleCount,
                               int vertexCount);                       // 0x004328b0: hull
    void UnknownFunction4329a0(Vector3 center, float radius);          // 0x004329a0: sphere
    void UnknownFunction432a20(Vector3 p0, Vector3 p1, float radius);  // 0x00432a20: capsule
    void UnknownFunction435830(const Matrix4* m);                      // 0x00435830: SetTransform

    unsigned char field_0x38[0x50 - 0x38];
    int field_0x50;                           // shape type (0 hull, 3 capsule, 4 sphere)
    void* field_0x54;                         // the shape (by field_0x50)
    unsigned char field_0x58[0x64 - 0x58];
    int field_0x64;
    unsigned char field_0x68[0xb8 - 0x68];    // 0xb8 bytes (operator new at 0x00457f74)
};

// LightEmitter.h: LightEmitter and LightManager, as far as the lighting
// update reads them.
class UnknownEcoLight {
public:
    unsigned char field_0x00[0x54];
    float field_0x54[4];                      // colour
    unsigned char field_0x64[0x70 - 0x64];
    Vector3 field_0x70;                       // direction
};
class LightManager {
public:
    UnknownEcoLight* UnknownFunction4a0190(int type); // 0x004a0190: first light of `type`
};

// RenderTarget.h: the view at GameObject+0x18 and its Camera (+0x08).
struct UnknownEcoCamera {
    unsigned char field_0x000[0xac];
    Matrix4 field_0xac;                       // view matrix
    unsigned char field_0xec[0x170 - 0xec];   // the clipper's matrix
    Vector3 field_0x170;                      // position
    Vector3 field_0x17c;                      // direction
    unsigned char field_0x188[0x1c0 - 0x188];
    float field_0x1c0;                        // far distance
};
class UnknownEcoRenderTarget {
public:
    virtual void UnknownVirtualSlot0();
    virtual int UnknownVirtualSlot1();
    virtual int UnknownVirtualSlot2();
    virtual int UnknownVirtualSlot3(void* a, void* b, void* c, int d);
    virtual void* UnknownVirtualSlot4(void* rect, long* pitch, int flags);
    virtual int UnknownVirtualSlot5(void* rect);
    virtual long UnknownVirtualSlot6(int stage, int type, int* value);
    virtual long UnknownVirtualSlot7(int stage, int type, int value);
    virtual void UnknownVirtualSlot8(int state, int value, int force);
    virtual long UnknownVirtualSlot9(int state, int* value);
    virtual void UnknownVirtualSlot10(int mode, int flag);
    virtual long UnknownVirtualSlot11(int stage);
    virtual int UnknownVirtualSlot12(const void* rect, int flags);
    virtual int UnknownVirtualSlot13(int a);
    virtual int UnknownVirtualSlot14(void* viewport);
    virtual int UnknownVirtualSlot15(int primitive, int format, void* vertices, int vertexCount,
                                     void* indices, int indexCount, int flags);
    virtual int UnknownVirtualSlot16(int a, int b, int c, int d, int e);
    virtual int UnknownVirtualSlot17(int a, int b, int c, int d, int e, int f, int g);
    virtual void UnknownVirtualSlot18(int value);
    virtual void UnknownVirtualSlot19();

    unsigned char field_0x04[4];
    UnknownEcoCamera* field_0x08;
    int field_0x0c;                           // width
    unsigned char field_0x10[0x38 - 0x10];
    int field_0x38;                           // vertices drawn
    unsigned char field_0x3c[0x44 - 0x3c];
    int field_0x44;                           // triangles drawn
};

// TextureMap.h: the texture objects the definitions hold.
class UnknownEcoTexture {
public:
    virtual void UnknownVirtualSlot0();
    virtual int UnknownVirtualSlot1();
    virtual int UnknownVirtualSlot2();        // release
    virtual int UnknownVirtualSlot3();
    virtual int UnknownVirtualSlot4();
    virtual int UnknownVirtualSlot5();
    virtual int UnknownVirtualSlot6();
    virtual int UnknownVirtualSlot7();        // whether the load failed
    virtual int UnknownVirtualSlot8(int a, int b, int c);
    virtual int UnknownVirtualSlot9();
    virtual int UnknownVirtualSlot10();
    virtual int UnknownVirtualSlot11();
    virtual int UnknownVirtualSlot12();
    virtual int UnknownVirtualSlot13();
    virtual int UnknownVirtualSlot14();
    virtual int UnknownVirtualSlot15();
    virtual int UnknownVirtualSlot16();
    virtual int UnknownVirtualSlot17();
    virtual int UnknownVirtualSlot18(unsigned int key);
    virtual int UnknownVirtualSlot19();       // selects the texture
    unsigned char field_0x04[0x20 - 4];
    int field_0x20;                           // pixel format
};
// 0x0050a590 (TextureMap.h): loads a texture through the manager.
UnknownEcoTexture* UnknownFunction50a590(TextureMapManager* manager, const char* name, int format,
                                         void* palette, int flags, int addressU, int addressV,
                                         void* choice, int alphaThreshold, unsigned int key,
                                         int addRef, int fromArchive);

// 0x00460b50 / 0x00460c00 / 0x00460c70 (src/krusty2/math/FastMath.h).
float FastSqrt(float x);
float FastInvSqrt(float x);
float UnknownFunction460c70(float x);        // table estimate of 1 / sqrt(x), no Newton step

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
    int field_0x00;                           // 0 GEOMETRY, 1 CYLINDER, 2 SPHERE, 3 RADIUSEDLINE
    Vector3 field_0x04;                       // centre / start
    Vector3 field_0x10;                       // end
    float field_0x1c;                         // radius
    float field_0x20;                         // height
};

// One vegetation kind ("Vegetation_%d" section of the .est file), 0x210
// bytes (operator new at 0x00457771 and 0x0045908b); no vtable.
class UnknownEcoDefinition {
public:
    enum { kMaxLods = 1 };

    UnknownEcoDefinition();                   // 0x00455de0
    ~UnknownEcoDefinition();                  // 0x00455e80
    int UnknownFunction455f50();              // 0x00455f50: a random parameter byte
    int UnknownFunction455f60(float height);  // 0x00455f60: a random parameter for `height`
    float UnknownFunction455f90(unsigned char parameter); // 0x00455f90: height for a parameter
    float UnknownFunction455ff0(unsigned char parameter); // 0x00455ff0: radius for a parameter
    // 0x00456050: loads the billboard texture and the .slt model.
    int UnknownFunction456050(TextureMapManager* textures, int modelFlags);

    char field_0x000[0x80];                   // name
    char field_0x080[0x80];                   // BillboardName
    char field_0x100[0x80];                   // ProbabilityTga
    float field_0x180;                        // MeanHeight
    float field_0x184;                        // MinHeight
    float field_0x188;                        // MaxHeight
    float field_0x18c;                        // MeanRadius
    float field_0x190;                        // MinRadius
    float field_0x194;                        // MaxRadius
    float field_0x198;                        // MeanSlope
    float field_0x19c;                        // StandardDeviationSlope
    float field_0x1a0;                        // MeanAspect
    float field_0x1a4;                        // StandardDeviationAspect
    float field_0x1a8;                        // MeanDrainage
    float field_0x1ac;                        // StandardDeviationDrainage
    float field_0x1b0;                        // MeanAltitude
    float field_0x1b4;                        // StandardDeviationAltitude
    float field_0x1b8;                        // ULeft
    float field_0x1bc;                        // URight
    float field_0x1c0;                        // UCenter
    float field_0x1c4;                        // VBottom
    float field_0x1c8;                        // VTop
    float field_0x1cc;                        // model height scale
    float field_0x1d0;                        // model radius scale
    float field_0x1d4;
    int field_0x1d8;                          // NumberOfLOD (at most kMaxLods)
    unsigned int field_0x1dc;                 // key colour
    UnknownEcoTexture* field_0x1e0;           // billboard texture
    UnknownEcoTexture* field_0x1e4;           // model texture
    unsigned short* field_0x1e8[kMaxLods];    // model indices (inside the vertex block)
    UnknownEcoModelVertex* field_0x1ec[kMaxLods]; // model vertices
    int field_0x1f0[kMaxLods];                // vertex count
    int field_0x1f4[kMaxLods];                // index count
    int field_0x1f8;                          // UsePlanarLighting
    int field_0x1fc;                          // BlendLODs
    float field_0x200;                        // PercentBias
    int field_0x204;                          // NumCollisionObjects
    UnknownEcoCollisionDefinition* field_0x208;
    CollisionObject** field_0x20c;
};

// RTTI: Vegetation : QuadTreeObject (vtable 0x005524f8), 0x1c bytes
// (0x00459656 allocates count * 0x1c).
class Vegetation : public UnknownEcoQuadTreeObject {
public:
    Vegetation();                             // 0x00456650
    virtual unsigned short UnknownVirtualSlot0(); // 0x00456690: view depth as a sort key
    virtual unsigned int UnknownVirtualSlot1(UnknownEcoQuadTree* tree); // 0x00456720: cell code

    // 0x004567a0 / 0x004567e0: place the object (quantised or world
    // coordinates); the first argument (EcoSystem::field_0x48) is not used.
    void UnknownFunction4567a0(TextureMapManager* textures, unsigned char definition,
                               const UnknownEcoCoordinates* coordinates, unsigned char heightParameter,
                               unsigned char radiusParameter);
    void UnknownFunction4567e0(TextureMapManager* textures, unsigned char definition,
                               const Vector3* position, unsigned char heightParameter,
                               unsigned char radiusParameter);
    // 0x00456890: distance test against the current detail band.
    void UnknownFunction456890(int* billboard, int* fade);
    // 0x00456a10: switches between the 3D geometry and the billboard.
    void UnknownFunction456a10(int billboard, int fade);
    void UnknownFunction457000(UnknownEcoRenderTarget* target); // 0x00457000: draws the geometry
    int UnknownFunction457080();              // 0x00457080: the definition's collision count
    // 0x004570a0: the definition's collision object `index`, placed here.
    CollisionObject* UnknownFunction4570a0(int index);
    float UnknownFunction457230();            // 0x00457230: the radius

    UnknownEcoCoordinates field_0x0c;         // in EcoSystem::field_0x5a8 units
    unsigned char field_0x12;                 // definition index
    unsigned char field_0x13;                 // fade (0..255)
    unsigned char field_0x14;                 // height parameter
    unsigned char field_0x15;                 // radius parameter
    unsigned char field_0x16_bit0 : 1;        // drawn as a billboard
    void* field_0x18;                         // geometry block (vertices, indices, AgeEntry)
};

// 0x00456850 (cdecl): the AgeManager eviction callback for a geometry block.
int UnknownFunction456850(void* object, int context);

// 0x0045c6a0 / 0x0045c7b0 (cdecl): fwrite / fread through a running
// one-byte xor key.
int UnknownFunction45c6a0(const unsigned char* data, int size, int count, FILE* file, unsigned char* key);
int UnknownFunction45c7b0(unsigned char* data, int size, int count, FILE* file, unsigned char* key);

// Griddraw.h / VisibilityQuadTree.h: the visibility clipper 0x00575a98.
class UnknownEcoVisibilityClipper {
public:
    int SphereInFrustum(const UnknownEcoCamera* camera, const void* matrix, const Vector3* center,
                        float radius, int* fullyInside);       // 0x0052fbb0
};
extern UnknownEcoVisibilityClipper* g_visibilityClipper;        // 0x00575a98

// TrackGame.h: the game object 0x0056e26c, as far as this file reads it
// (slot 22 is the registry query).
struct UnknownEcoDisplay {
    unsigned char field_0x000[0x28];
    int field_0x28;                           // colour-key texture format
    unsigned char field_0x02c[0x1c0 - 0x2c];
    int field_0x1c0;                          // capability bits
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
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual int UnknownVirtualSlot22(const char* name, int defaultValue); // registry value

    unsigned char field_0x004[0x10 - 4];
    UnknownEcoDisplay* field_0x10;
    unsigned char field_0x014[0x2d0 - 0x14];
    int field_0x2d0;                          // software rendering
};
extern UnknownEcoTrackGame* g_UnknownGlobal56e26c;
unsigned int ReadClock();                     // 0x004bfa80
int UnknownFunction43caa0(int control, int kind, const UnknownControlEvent* event, int modifier); // 0x0043caa0

// A detail band (0x20 bytes) of the two tables 0x0056a600 / 0x0056a740.
struct UnknownEcoDetailBand {
    int field_0x00;                           // 3D distance
    int field_0x04;                           // fade start distance
    int field_0x08;                           // model flags
    int field_0x0c;                           // billboard range
    int field_0x10;                           // billboard limit
    int field_0x14;                           // texture-stage lighting
    int field_0x18;                           // specular
    int field_0x1c;                           // fog
};

class EcoSystem : public GameObject, public GroundFogableObject {
public:
    friend class Vegetation;
    explicit EcoSystem(int flags);            // 0x00457250
    virtual ~EcoSystem();                     // 0x00457330 (deleting wrapper 0x00457310)
    virtual int UnknownVirtualSlot12();       // 0x0045aad0: classifies the visible objects
    virtual int UnknownVirtualSlot14();       // 0x0045b060: draws them
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0045bfe0

    // 0x00457480: reads the .est file (or a .esb through 0x00458f70);
    // `path` is rewritten to the .esb name when one exists.
    int UnknownFunction457480(char* path, UnknownTextureStream* stream);
    // 0x00457ed0: builds the collision objects of a definition.
    void UnknownFunction457ed0(UnknownEcoDefinition* definition);
    // 0x00458360: reads a definition's "CollisionObject%i" entries.
    void UnknownFunction458360(const char* path, int index, UnknownEcoDefinition* definition);
    int UnknownFunction4587b0(const char* path); // 0x004587b0: writes the .esb
    void UnknownFunction458da0(const char* path); // 0x00458da0: writes the .txt listing
    int UnknownFunction458f70(const char* path, UnknownTextureStream* stream); // 0x00458f70: reads the .esb
    void UnknownFunction4594c0(int level);    // 0x004594c0: detail level
    // 0x004594d0: creates the ecosystem.
    EcoSystem* UnknownFunction4594d0(void* view, TextureMapManager* textures, UnknownEcoTerrain* terrain,
                                     LightManager* lights, char* path, UnknownTextureStream* stream,
                                     int textureFormat, int collisions, int level);
    int UnknownFunction4598d0();              // 0x004598d0: places the objects read from the .esb
    int UnknownFunction459b40();              // 0x00459b40: places the authored objects
    int UnknownFunction459ce0(int seed);      // 0x00459ce0: generates the objects
    void UnknownFunction45a9a0();             // 0x0045a9a0: lighting changed
    void UnknownFunction45ade0(int format);   // 0x0045ade0: render states for a texture format
    int UnknownFunction45b136();              // 0x0045b136: draws the billboards

    int field_0x30;                           // Method: 1 Authored, 2 Auto
    int field_0x34;                           // TotalObjects
    Vegetation* field_0x38;                   // the objects
    Vegetation** field_0x3c;                  // objects drawn as billboards
    Vegetation** field_0x40;                  // objects drawn as geometry
    UnknownEcoTerrain* field_0x44;
    TextureMapManager* field_0x48;
    LightManager* field_0x4c;
    UnknownEcoVertex* field_0x50;             // billboard vertices (120 * 6)
    unsigned short* field_0x54;               // billboard indices (120 * 12)
    UnknownEcoDefinition* field_0x58[256];
    float field_0x458;                        // NorthAngle
    char field_0x45c[0x80];                   // PlacementBmp
    char field_0x4dc[0x80];                   // ProbabilityTga
    int field_0x55c;                          // placed object count
    int field_0x560;                          // texture format
    Vector3 field_0x564;                      // directional light colour
    Vector3 field_0x570;                      // ambient colour
    Vector3 field_0x57c;                      // light direction
    unsigned char field_0x588[8];
    UnknownTextureStream* field_0x590;        // the .esb stream
    int field_0x594;                          // nonzero: field_0x590 belongs to the archive
    unsigned char field_0x598;
    char field_0x599;                         // the Vegetation type id (TypeRegistry)
    AgeManager* field_0x59c;
    int field_0x5a0;                          // billboard count
    int field_0x5a4;                          // geometry count
    float field_0x5a8;                        // world units per coordinate unit
    float field_0x5ac;                        // coordinate units per world unit
    int field_0x5b0;                          // billboard list capacity
    int field_0x5b4;                          // geometry list capacity
    float field_0x5b8;                        // 65535 / far distance
    float field_0x5bc;                        // far distance / 65536
    int field_0x5c0;                          // detail level
};

extern EcoSystem* g_UnknownGlobal59aebc;      // the one instance

// 0x0045c040 (cdecl): loads every texture a .esb names ahead of the
// ecosystem itself.
int UnknownFunction45c040(TextureMapManager* textures, char* path, UnknownTextureStream* stream, int modelFlags);

// Helpers reconstructed elsewhere.
// 0x0047b8a0 (gameui.cpp tail): a float profile value, `defaultValue` when the key is "NONE".
float UnknownFunction47b8a0(const char* section, const char* key, double defaultValue, const char* path);
void UnknownFunction461d40(FILE* file, const char* format, ...); // 0x00461d40: fprintf
UnknownBitmapFile* UnknownFunction424140(const char* path, UnknownBitmapFile* into); // 0x00424140
void UnknownFunction4245b0(UnknownBitmapFile* bitmap);                  // 0x004245b0
UnknownTgaFile* UnknownFunction5125c0(const char* path, UnknownTgaFile* into, int a); // 0x005125c0
void UnknownFunction512dd0(UnknownTgaFile* tga);                        // 0x00512dd0
int UnknownFunction511ad0(int format);                                  // 0x00511ad0

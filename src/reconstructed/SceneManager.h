#pragma once

#include <stdio.h>

#include "ContainerList.h"
#include "GameObject.h"
#include "MatrixUtil.h"
#include "Parameterblocks.h"
#include "TrackGame.h"

class LightEmitter;
class LightManager;
class Sound;
class SoundGroup;
struct UnknownSound3DParameters;

// The Soultree object whose detail level 0x004eff30 sets (0x004451e0, the
// D3DIMSoultreeObject methods of D3DIMSoulTree.h). A view: the scene reaches
// it through character +0x1a0, object +0x34 and each shadow caster.
struct UnknownSceneLodObject {
    void UnknownFunction4444c0(int value);           // 0x004444c0
    void UnknownFunction4451e0(int level);           // 0x004451e0
};

// The object a scene entry holds. Tier 2: 0x004eb040 calls 0x004a8b40
// (D3DIMSoultreeCharacter::SetMotion in src/krusty2/motion) on it, whose
// +0x1a4 buffer 0x004eaec0 compares as the name. Only that is declared here.
struct UnknownSceneObject {
    void UnknownFunction4a8b40(void* motion);          // 0x004a8b40 (SetMotion)
    int UnknownFunction4a6bb0(float time, int a, int b); // 0x004a6bb0: advances the motion

    unsigned char field_0x000[0x0c];
    int field_0x0c;                       // cleared by 0x004a8b40
    int field_0x10;                       // the motion time (Character +0x10)
    unsigned char field_0x014[0x1a0 - 0x14];
    UnknownSceneLodObject* field_0x1a0;
    char field_0x1a4[0x6c];
};

// What UnknownSceneEntry::field_0x08 holds for entries without bit 3: a
// GameObject whose slot 10 0x004eb040 calls. Only those fields are known.
class UnknownSceneAnimatedObject : public GameObject {
public:
    unsigned char field_0x2c[0x34 - 0x2c];
    UnknownSceneLodObject* field_0x34;
    unsigned char field_0x38[0x5c - 0x38];
    int field_0x5c;
    unsigned char field_0x60[0x194 - 0x60];
    float field_0x194;
};

// 0x44-byte element of UnknownSceneTable::field_0x04. The flag byte at +0
// is read and written as single bits (0x004ea880 tests bit 3, 0x004eafd0
// sets bit 2).
struct UnknownSceneEntry {
    unsigned char field_0x00_bit0 : 1;
    unsigned char field_0x00_bit1 : 1;
    unsigned char field_0x00_bit2 : 1;
    unsigned char field_0x00_bit3 : 1;
    unsigned char field_0x00_bit4 : 1;              // skipped by bikerace.cpp's camera (0x0041f1d0)
    unsigned char field_0x01[3];
    UnknownSceneObject* field_0x04;
    void* field_0x08;
    unsigned char field_0x0c[0x18 - 0x0c];
    int field_0x18;                       // 0x004eb040: the motion advance result
    int field_0x1c;
    void** field_0x20;                    // motions
    char field_0x24;                      // current motion index
    signed char field_0x25;               // field_0x28 count
    signed char field_0x26;               // index into field_0x28
    unsigned char field_0x27;
    char* field_0x28;                     // motion numbers (1-based)
    unsigned char field_0x2c[0x44 - 0x2c];
};

// 0x10-byte element of UnknownSceneTable::field_0x0c.
struct UnknownSceneEntry2 {
    signed char field_0x00;
    unsigned char field_0x01[3];
    UnknownSceneEntry* field_0x04;        // 0x004eff30 reads it as an entry
    void* field_0x08;
    void* field_0x0c;
};

// Owned by Scene+0xb4; freed with its arrays by the destructor.
struct UnknownSceneTable {
    int field_0x00;                       // entry count
    UnknownSceneEntry* field_0x04;
    int field_0x08;                       // entry2 count
    UnknownSceneEntry2* field_0x0c;
};

class ShadowCaster;
class ProjectedShadow;

// Views declared in samples/track/SceneManagerNearMisses.cpp (0x004ecd60).
class UnknownScenePhysicsObject;
class UnknownSceneCollisionObject;

// 0x28-byte element of UnknownSceneBuffer::field_0x04: a static model
// ("Model<n>", 0x004ecd60), also a shadow caster (0x004edf20 attaches a
// D3DIMSoultreeShadow to each).
struct UnknownSceneCaster {
    unsigned char physics : 1;            // bit 0: has a physics object
    unsigned char useLighting : 1;        // bit 1: "UseLighting"
    unsigned char field_0x01[3];
    ShadowCaster* field_0x04;             // the model (a D3DIMSoultreeObject)
    UnknownScenePhysicsObject* field_0x08;
    UnknownSceneCollisionObject* field_0x0c;
    char field_0x10[0x18];                // the model's file name, at most 0x14 characters
};

// Owned by Scene+0xb8: the shadow casters.
struct UnknownSceneBuffer {
    int field_0x00;                       // caster count
    UnknownSceneCaster* field_0x04;
};

// Minimal view of RTTI class D3DIMSoultreeShadow (0x48 bytes), declared in
// full in src/krusty2/shadow/D3DIMSoultreeShadow.h, whose header tree cannot
// be mixed with this one. Only what Scene 0x004edf20 calls is declared.
class D3DIMSoultreeShadow : public GameObject {
public:
    explicit D3DIMSoultreeShadow(int flags);                       // 0x00446840
    D3DIMSoultreeShadow* Attach(int host, ShadowCaster* caster, ProjectedShadow* shadow); // 0x004468b0

    unsigned char field_0x2c[0x48 - 0x2c];
};

// What 0x004f00e0 counts: a record with a texture name at +0x2c, built by
// its two callers (0x004f0390, 0x004f0ec0) on the stack. Only the name is
// known; its extent is provisional.
struct UnknownSceneTextureInfo {
    unsigned char field_0x00[0x2c];
    char field_0x2c[0x40];
};

// 0x8c-byte element of Scene::field_0xac, filled by 0x004eb570 from the
// "Light<n>" sections. Names follow the section keys.
struct UnknownSceneLight {
    LightEmitter* field_0x00;             // the emitter 0x004efb20 creates
    int type;                             // "Type" keyword (0x00572c28 table: Undefined..AmbientNoise)
    unsigned long color;                  // "ColorRGB"
    Vector3 position;                     // "Position"
    Vector3* field_0x18;                  // &position, &direction or 0
    Vector3 direction;                    // position scaled to 80000 units (at infinity)
    Vector3 look;                         // "LookVector" or towards "TargetPosition"
    Vector3* field_0x34;                  // &look or 0
    float range;                          // "Range"
    unsigned char showDebugSphere : 1;    // +0x3c bit 0
    unsigned char atInfinity : 1;         // bit 1 "PlaceLightAtInfinity"
    unsigned char emitsLight : 1;         // bit 2
    unsigned char castsShadows : 1;       // bit 3
    unsigned char hasLensFlare : 1;       // bit 4
    char layersVisible[5];                // +0x3d "LensFlareLayersVisible"
    unsigned char field_0x42[2];
    unsigned long lensFlareColor;         // +0x44
    float lensFlareBrightness;            // +0x48
    char lensFlareTexture[0x40];          // +0x4c
};

// Scene+0xa4 (0x420 bytes, SceneManager.cpp line 1456), filled by
// 0x004ebfc0 from the "Environment", "TerrainZone<n>" and "ReverbZone<n>"
// sections. Names follow the keys.
struct UnknownSceneEnvironment {
    char ecosystemFile[0x104];            // +0x000
    char terrainFile[0x104];              // +0x104
    char cubeFile[0x104];                 // +0x208
    char particleTexture[0x40];           // +0x30c
    char detailTexture[0x40];             // +0x34c
    float terrainWidthScale;              // +0x38c
    float terrainBreadthScale;
    float terrainWidth;
    float terrainBreadth;
    float startGateScale;                 // +0x39c
    float surfaceFriction[8];             // +0x3a0, per terrain zone
    float surfaceDrag[8];                 // +0x3c0
    float surfaceTraction[8];             // +0x3e0
    unsigned char generateDust[8];        // +0x400
    unsigned char generateDirt[8];        // +0x408
    int enviroID[4];                      // +0x410, per reverb zone
};

// Scene+0xb0, allocated by 0x004ebdb0 from the "Fog" section.
struct UnknownSceneFog {
    unsigned long color;                  // "ColorRGB", default 0x8080c0
    float visibility;                     // "Visibility", default 1.0
    float haziness;                       // "Haziness", default 0.5
};

// The 0x18-byte resource manager 0x004f0d20 keeps (unused) on its stack.
// Tier 2: the same retail class as UnknownResourceManager. Its constructor
// 0x004e8e80 zeroes the six fields; its destructor 0x004e8ea0 frees both
// arrays (ResourceManager.cpp lines 135 and 139). A separate view because
// declaring the destructor and fields on the shared class flips one register
// choice in TrackGame 0x00520ab0 (see docs/SCENEMANAGER.md).
class UnknownSceneResourceManager {
public:
    UnknownSceneResourceManager();       // 0x004e8e80
    ~UnknownSceneResourceManager();      // 0x004e8ea0

    int field_0x00;
    int field_0x04;
    int field_0x08;                       // field_0x14 count
    int field_0x0c;                       // field_0x10 count
    void** field_0x10;
    void** field_0x14;
};

// Name/value pair scanned by 0x004e9980.
struct UnknownSceneKeyword {
    const char* name;
    int value;
};

// RTTI: Scene : GameObject : BaseObject (vtable 0x0055786c), 0x8cc bytes
// (operator new(0x8cc) at 0x004e9a30 and 0x004e9aea). Scene overrides
// slots 0, 5, 7, 10, 14, 22 and 23. Field names are provisional.
class Scene : public GameObject {
public:
    explicit Scene(int flags);                       // 0x004ea660
    virtual ~Scene();                                // 0x004ea880 (deleting 0x004ea7c0)

    virtual void UnknownVirtualSlot5();              // 0x004eaa60
    virtual void UnknownVirtualSlot7();              // 0x004eaab0
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004eab00, not reconstructed
    virtual int UnknownVirtualSlot14();              // 0x004f0040
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry);

    // 0x004ea7e0: stores the owner and settings, adds the sound group child.
    Scene* UnknownFunction4ea7e0(UnknownTrackGameObject574* manager, int value, int unused,
                                 char flag);
    // 0x004eaec0: the entry object named like `path`'s file name.
    UnknownSceneObject* UnknownFunction4eaec0(const char* path, int* index);
    void UnknownFunction4eafd0(int index);
    void UnknownFunction4eff30(int level);           // 0x004eff30: detail level (QuarryStuntEvent.cpp)
    void UnknownFunction4eb000(float time);
    int UnknownFunction4eb040(int index, float time, int force, int motion);
    // 0x004eb160/0x004eb300/0x004eb480: read "x,y,z", "r,g,b" and a Y/T/1
    // flag list from `block` (default: field_0xdc). Parameter names provisional.
    int UnknownFunction4eb160(Vector3* out, const char* section, const char* key,
                              const char* def, int useDefault, UnknownParameterBlock* block);
    int UnknownFunction4eb300(unsigned long* out, const char* section, const char* key,
                              const char* def, int unused, UnknownParameterBlock* block);
    int UnknownFunction4eb480(char* out, int count, const char* section, const char* key,
                              const char* def, int unused, UnknownParameterBlock* block);
    int UnknownFunction4ebdb0();                     // reads the "Fog" section
    int UnknownFunction4eca20();                     // opens the "ResourceFiles" archives
    void UnknownFunction4edf20(ProjectedShadow* shadow, int flags); // shadows for the casters
    void UnknownFunction4ecc10();                    // reads the "Stadium" section
    void UnknownFunction4ef9c0(const char* section, char* file, int is3D,
                               UnknownSound3DParameters* params, unsigned long* flags,
                               float* oneShotDistance, float* randomTriggerPercent,
                               int* oneShot, int* force2D);
    // 0x004de580 is a shared empty `ret 0xc` body (also BaseQuarryEvent and
    // NationalRace slot 34) that the readers call directly with the value
    // they read: a string or, in 0x004ebdb0, a float. The two overloads
    // here are provisional.
    void UnknownFunction4de580(const char* section, const char* key, const char* value);
    void UnknownFunction4de580(const char* section, const char* key, float value);
    void UnknownFunction4de580(const char* section, const char* key, int value);
    void UnknownFunction4de580(const char* section, const char* key, const Vector3* value);
    // A second function folded into the same empty body: VC6 does not
    // cross-jump its calls with the ones above (0x004ebfc0).
    void UnknownFolded4de580(const char* section, const char* key, const char* value);
    // 0x004efb20: loads the scene: its file, the start and podium positions,
    // the environment, lights, fog, resources, stadium and the rest.
    int UnknownFunction4efb20(void* owner, LightManager* lights, int a3, int a4, int a5,
                              const char* cubeDirectory, void (*progress)(int), int interval);
    // 0x004ecd60: reads "StaticModels": a model per "Model<n>", with its
    // physics object, collision points and particle emitters or its
    // collision object, and its sound emitter. `progress` is called every
    // `interval` models.
    int UnknownFunction4ecd60(LightManager* lights, int a2, int a3, int a4, void (*progress)(int),
                              int interval);           // near miss (samples/track)
    int UnknownFunction4edfe0(LightManager* lights, int a3, int a4, void (*progress)(int),
                              int interval);           // not reconstructed
    int UnknownFunction4ef4c0();                     // 0x004ef4c0: reads the "Sounds" section
    int UnknownFunction4ebfc0(const char* directory, const char* cubeDirectory); // "Environment"
    int UnknownFunction4eb570(int index);            // reads "Light<index + 1>"
    // 0x00464e80 is likewise a shared empty `ret 4` body (FollowCamera slot 52
    // among others), called directly with the formatted "Cannot find data" text.
    void UnknownFunction464e80(const char* message);
    int UnknownFunction4f0d20(int* count);           // loads the scene file
    int UnknownFunction4f0ec0(char* path);           // model path -> its SLT file
    // 0x004f1130: counts the textures of the scene's models and animations
    // by width (0x004f0390) and reads the terrain width, whether a cube file
    // is set and the ecosystem file path, without building anything.
    int UnknownFunction4f1130(unsigned long* counts, float* width, int* hasCube, char* ecosystem);

    UnknownTrackGameObject574* field_0x2c;
    char field_0x30[0x40];
    Vector3 field_0x70;
    Vector3 field_0x7c;
    char field_0x88;
    unsigned char field_0x89[3];
    Vector3 field_0x8c;                              // "PodiumPosition"
    Vector3 field_0x98;                              // "PodiumDirection"
    UnknownSceneEnvironment* field_0xa4;
    int field_0xa8;
    UnknownSceneLight* field_0xac;
    UnknownSceneFog* field_0xb0;
    UnknownSceneTable* field_0xb4;
    UnknownSceneBuffer* field_0xb8;
    int field_0xbc;
    int field_0xc0;
    SoundGroup* field_0xc4;
    ContainerList<Sound*> field_0xc8;
    UnknownParameterBlock field_0xdc;
    unsigned char field_0x6a0_bit0 : 1;              // "Stadium" section present
    unsigned char field_0x6a1[3];
    float field_0x6a4;                               // Stadium "Top"
    float field_0x6a8;                               // Stadium "Bottom"
    float field_0x6ac;                               // Stadium "Scale"
    Vector3 field_0x6b0;                             // Stadium "Position"
    char field_0x6bc[0x100];
    int field_0x7bc;                                 // toggled by slot 22
    int field_0x7c0;
    FILE* field_0x7c4;
    char field_0x7c8[0x104];
};

// The detail-level table 0x004eff30 selects by Game+0x2d0 (declared alike
// in BikeRace.h).
extern unsigned char* g_UnknownGlobal689f18;
extern unsigned char g_UnknownGlobal5744c8[];
extern unsigned char g_UnknownGlobal574428[];

int UnknownFunction4f0310(UnknownResourceManager* manager, const char* name, const char* path);
// 0x004f0390: counts the textures of the model file `path` into counts[0..3]
// (0x004f00e0), loading them through `resources`; not reconstructed.
int UnknownFunction4f0390(const char* path, UnknownSceneResourceManager* resources, unsigned long* counts);
int UnknownFunction4e9980(int* out, const char* name, const UnknownSceneKeyword* table);
// 0x004f00e0: counts the texture by width (256, 128, 64 or 32) in counts[0..3];
// "PROCEDURAL" textures count as 64.
int UnknownFunction4f00e0(UnknownSceneTextureInfo* texture, int* counts);

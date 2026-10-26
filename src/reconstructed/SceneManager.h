#pragma once

#include <stdio.h>

#include "ContainerList.h"
#include "GameObject.h"
#include "MatrixUtil.h"
#include "Parameterblocks.h"
#include "SoultreeLodSettings.h"
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
    void SetAxesPtr(const Vector3* look, const Vector3* up, int orthogonalize, int keepZ); // 0x004fbd70
    void SetPosition(const Vector3* position); // 0x004fc660
};

// The object a key-framed scene entry holds: CollisionCharacter (0x294
// bytes, constructor 0x004318d0, src/krusty2/motion), a
// D3DIMSoultreeCharacter with its own vtable at +0, a vbptr at +4 and the
// GameObject virtual base at +0x268. Tier 2: 0x004eb040 calls 0x004a8b40
// (D3DIMSoultreeCharacter::SetMotion in src/krusty2/motion) on it, whose
// +0x1a4 buffer 0x004eaec0 compares as the name. Only what the scene uses is
// declared.
struct UnknownSceneObject : public virtual GameObject {
public:
    explicit UnknownSceneObject(int flags);            // 0x004318d0
    virtual void UnknownVirtualSlot0();

    // 0x004319c0: loads the character and returns the GameObject to add
    // (CollisionCharacter::Load, src/krusty2/motion).
    GameObject* Load(void* owner, const char* name, const char* colPath,
                     LightManager* lights, int a4, int a5, int a6);
    void* FindMotion(const char* name, int unused); // 0x004a6b30 (FindMotion)
    void SetMotion(void* motion);          // 0x004a8b40 (SetMotion)
    int AdvanceMotion(float time, int a, int b); // 0x004a6bb0: advances the motion

    unsigned char field_0x008[0x0c - 0x08];
    int motionStopped;                        // +0xC  Character +0x0c (D3DIMSoultreeCharacter.h motionStopped): set when a non-looping motion ends, cleared by SetMotion 0x004a8b40; AdvanceMotion returns 0 while set
    int motionTime;  // +0x10  Character +0x10: motion time (slot 7 adds time to it); RestartEntry zeroes it before AdvanceMotion
    unsigned char field_0x014[0x1a0 - 0x14];
    UnknownSceneLodObject* modelNode;         // +0x1a0
    char fileName[0x6c];  // +0x1A4  FindEntryByFileName (0x004eaec0) compares it, up to its '.', with the path's file name
    void* collisionObject;                // +0x210 CollisionCharacter's CollisionObject (QuarryStuntEvent.cpp)
    unsigned char field_0x214[0x268 - 0x214];
};

// What UnknownSceneEntry::field_0x08 holds for procedural entries (without
// bit 3): CarProcedural (0x22c bytes), a GameObject whose slot 10 0x004eb040
// calls. Only what the scene uses is declared.
class UnknownSceneAnimatedObject : public GameObject {
public:
    explicit UnknownSceneAnimatedObject(int flags);    // 0x0042f390 (CarProcedural)
    // Slot 27 (0x0042f600, CarProcedural.h): loads the model and its path;
    // returns the object to add.
    virtual GameObject* UnknownVirtualSlot27(void* owner, const char* name, const char* colPath,
                                             LightManager* lights, int a4, int a5, const char* vuePath,
                                             const Vector3* position, float fps, int frontWheelsTurn,
                                             float lagDistance, float a11, int tires, float a13,
                                             float a14, float a15);

    unsigned char field_0x2c[0x34 - 0x2c];
    UnknownSceneLodObject* carModel;  // +0x34  CarProcedural +0x34 carModel; SetDetailLevel/BikeRace use it as the model node
    void* sensorCollider;  // +0x38  CarProcedural +0x38 sensorCollider (QuarryStuntEvent sets collision ignore lists)
    void* bodyCollider;  // +0x3C  CarProcedural +0x3c bodyCollider
    unsigned char field_0x40[0x5c - 0x40];
    int skipSmoothing;                        // +0x5C  CarProcedural +0x5c; RestartEntry sets it unless the restart time was clamped
    unsigned char field_0x60[0x194 - 0x60];
    float pathTime;  // +0x194  CarProcedural +0x194 pathTime; RestartEntry (0x004eb040) sets it to the restart time
    // +0x20c (the terrain) stays unnamed here: naming it together with
    // +0x38/+0x3c changes VC6's code for 0x004eb570.
    unsigned char field_0x198[0x22c - 0x198];
};

// 0x44-byte element of UnknownSceneTable::field_0x04 (an "Animation<n>"
// section). The flag byte at +0 is read and written as single bits
// (0x004ea880 tests bit 3, 0x004eafd0 sets bit 2). BikeRace.cpp reads
// +0x04, +0x08 and +0x2c under their provisional names.
struct UnknownSceneEntry {
    unsigned char inRandomSet : 1;            // +0x00 bit 0: cleared for every entry, set for each entry a RandomSet lists
    unsigned char useBlendedKeys : 1;         // +0x00 bit 1: the "UseBlendedKeys" parameter
    unsigned char automaticSequence : 1;      // +0x00 bit 2: the "AutomaticSequence" parameter; EnableAutomaticSequence sets it
    unsigned char keyFramed : 1;              // +0x00 key-framed character (else a procedural car)
    unsigned char excludeFromCameraIteration : 1; // +0x00 bit 4: the "ExcludeFromCameraIteration" parameter; bikerace.cpp's camera (0x0041f1d0) skips it
    unsigned char animatesWhileHeld : 1;      // +0x00 bit 5 (provisional): Scene slot 10 still advances the entry while holdAnimations is set; cleared by 0x004edfe0, bikerace.cpp sets it for the 30SecMan
    unsigned char field_0x01[3];
    UnknownSceneObject* character;            // +0x04 key-framed (bit 3)
    UnknownSceneAnimatedObject* proceduralObject; // +0x08 procedural
    Vector3 position;                     // "Position" (or "Offset")
    int motionFrame;                          // +0x18  the frame number AdvanceMotion 0x004a6bb0 returned (RestartEntry, Scene slot 10 0x004eae7d)
    int motionCount;                      // "NumberOfMotions" (1 for procedural entries)
    void** motions;
    char currentMotion;                   // index into motions
    signed char sequenceLength;           // motionSequence count ("NumberInSequence")
    signed char sequenceIndex;            // index into motionSequence
    unsigned char field_0x27;
    char* motionSequence;                 // motion numbers (1-based, "MotionSequence")
    char fileName[0x18];                      // +0x2c the MCF or SLT file name, at most 0x14 characters
};

// 0x10-byte element of UnknownSceneTable::randomSets (a "RandomSet<n>"
// section).
struct UnknownSceneEntry2 {
    signed char count;                    // "NumberInSequence"
    unsigned char field_0x01[3];
    UnknownSceneEntry* entry;             // 0x004eff30 reads it as an entry
    signed char* animationIndices;        // the a of each "a.m" pair
    signed char* motionIndices;           // the m of each pair
};

// Owned by Scene+0xb4; freed with its arrays by the destructor.
struct UnknownSceneTable {
    int entryCount;                           // +0x00 entry count ("NumberOfAnimations")
    UnknownSceneEntry* entries;               // +0x04
    int randomSetCount;
    UnknownSceneEntry2* randomSets;
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
    ShadowCaster* model;                      // +0x04 the model (a D3DIMSoultreeObject)
    UnknownScenePhysicsObject* physicsObject; // +0x08
    UnknownSceneCollisionObject* collisionObject; // +0x0c
    char fileName[0x18];                      // +0x10 the model's file name, at most 0x14 characters
};

// Owned by Scene+0xb8: the shadow casters.
struct UnknownSceneBuffer {
    int casterCount;                          // +0x00 caster count
    UnknownSceneCaster* casters;              // +0x04
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
    char textureName[0x40];
};

// 0x8c-byte element of Scene::field_0xac, filled by 0x004eb570 from the
// "Light<n>" sections. Names follow the section keys.
struct UnknownSceneLight {
    LightEmitter* emitter;                    // +0x00 the emitter 0x004efb20 creates
    int type;                             // "Type" keyword (0x00572c28 table: Undefined..AmbientNoise)
    unsigned long color;                  // "ColorRGB"
    Vector3 position;                     // "Position"
    Vector3* effectivePosition;  // +0x18  ReadLight points it at position, at direction (PlaceLightAtInfinity) or 0
    Vector3 direction;                    // position scaled to 80000 units (at infinity)
    Vector3 look;                         // "LookVector" or towards "TargetPosition"
    Vector3* emitterDirection;  // +0x34  &look or 0; Load passes it as LightEmitter::Create's direction
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

// Scene's local manager has the same 0x18-byte layout and lifecycle.
typedef UnknownResourceManager UnknownSceneResourceManager;

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

    virtual void Enable();                           // 0x004eaa60
    virtual void OnEnable();                         // 0x004eaab0
    virtual int Tick(float frameTime);                 // 0x004eab00, not reconstructed
    virtual int UnknownVirtualSlot14();              // 0x004f0040
    virtual int OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);

    // 0x004ea7e0: stores the owner and settings, adds the sound group child.
    Scene* UnknownFunction4ea7e0(UnknownTrackGameObject574* manager, int value, int unused,
                                 char flag);
    // 0x004eaec0: the entry object named like `path`'s file name.
    UnknownSceneObject* FindEntryByFileName(const char* path, int* index);
    void EnableAutomaticSequence(int index);  // 0x004eafd0: sets entry `index`'s automaticSequence
    void SetDetailLevel(int level);           // 0x004eff30: detail level (QuarryStuntEvent.cpp)
    void RestartAllEntries(float time);       // 0x004eb000: RestartEntry(i, time, 1, 0) for every entry
    int RestartEntry(int index, float time, int force, int motion);
    // 0x004eb160/0x004eb300/0x004eb480: read "x,y,z", "r,g,b" and a Y/T/1
    // flag list from `block` (default: the scene file, parameters). Parameter names provisional.
    int ReadVector(Vector3* out, const char* section, const char* key,
                              const char* def, int useDefault, UnknownParameterBlock* block);
    int ReadColor(unsigned long* out, const char* section, const char* key,
                              const char* def, int unused, UnknownParameterBlock* block);
    int ReadFlags(char* out, int count, const char* section, const char* key,
                              const char* def, int unused, UnknownParameterBlock* block);
    int ReadFog();                        // 0x004ebdb0: the "Fog" section
    int OpenResourceFiles();              // 0x004eca20: opens the "ResourceFiles" archives
    void AttachCasterShadows(ProjectedShadow* shadow, int flags); // shadows for the casters
    void ReadStadium();                   // 0x004ecc10: the "Stadium" section
    // 0x004ef9c0: one sound emitter's settings from `section`.
    void ReadSoundSettings(const char* section, char* file, int is3D,
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
    int Load(void* owner, LightManager* lights, int a3, int a4, int a5,
                              const char* cubeDirectory, void (*progress)(int), int interval);
    // 0x004ecd60: reads "StaticModels": a model per "Model<n>", with its
    // physics object, collision points and particle emitters or its
    // collision object, and its sound emitter. `progress` is called every
    // `interval` models.
    int ReadStaticModels(LightManager* lights, int a2, int a3, int a4, void (*progress)(int),
                              int interval);           // near miss (samples/track)
    // 0x004edfe0: reads "Animations": a key-framed character or a procedural
    // car per "Animation<n>", with its motions and sound emitter, then the
    // "RandomSet<n>" sections.
    int ReadAnimations(LightManager* lights, int a3, int a4, void (*progress)(int),
                              int interval);           // near miss (samples/track)
    int ReadSounds();                                // 0x004ef4c0: reads the "Sounds" section
    int ReadEnvironment(const char* directory, const char* cubeDirectory); // 0x004ebfc0: "Environment"
    int ReadLight(int index);             // 0x004eb570: reads "Light<index + 1>"
    // 0x00464e80 is likewise a shared empty `ret 4` body (FollowCamera slot 52
    // among others), called directly with the formatted "Cannot find data" text.
    void UnknownFunction464e80(const char* message);
    int CountObjects(int* count);         // 0x004f0d20: loads the scene file, counts its objects
    int ResolveSltPath(char* path);           // model path -> its SLT file
    // 0x004f1130: counts the textures of the scene's models and animations
    // by width (0x004f0390) and reads the terrain width, whether a cube file
    // is set and the ecosystem file path, without building anything.
    int CountTextures(unsigned long* counts, float* width, int* hasCube, char* ecosystem);

    UnknownTrackGameObject574* sceneManager; // the owner 0x004ea7e0 stores
    char sceneName[0x40];                 // "SceneInfo" "SceneName"
    Vector3 defaultStartPosition;             // +0x70 "SceneInfo" "DefaultStartPosition"
    Vector3 defaultStartDirection;            // +0x7c "SceneInfo" "DefaultStartDirection"
    char field_0x88;
    unsigned char field_0x89[3];
    Vector3 podiumPosition;                          // "PodiumPosition"
    Vector3 podiumDirection;                         // "PodiumDirection"
    // QuarryStuntEvent.cpp reads +0xa4..+0xbc under their provisional names.
    UnknownSceneEnvironment* environment;     // +0xa4 "Environment" section
    int lightCount;                           // +0xa8 light count ("Lights" "NumberOfLights")
    UnknownSceneLight* sceneLights;           // +0xac "Light<n>" sections
    UnknownSceneFog* fog;                     // +0xb0 "Fog" section
    UnknownSceneTable* animationTable;        // +0xb4 "Animations" section (BikeRace.cpp reads it)
    UnknownSceneBuffer* staticModels;         // +0xb8 "StaticModels" section
    int crowdPresent;                         // +0xbc "Sounds" "CrowdPresent"
    int auralScape;                       // the AuralScape the sound emitters use
    SoundGroup* soundGroup;
    ContainerList<Sound*> sounds;
    UnknownParameterBlock parameters;     // the scene file
    unsigned char hasStadium : 1;                    // "Stadium" section present
    unsigned char field_0x6a1[3];
    float stadiumTop;                                // Stadium "Top"
    float stadiumBottom;                             // Stadium "Bottom"
    float stadiumScale;                              // Stadium "Scale"
    Vector3 stadiumPosition;                         // Stadium "Position"
    char stadiumFile[0x100];              // Stadium "FileName"
    int visible;                          // slot 14 draws only while set; control 5 toggles it
    int holdAnimations;                       // +0x7C0  provisional: while set slot 10 (0x004eab65) advances only key-framed entries with animatesWhileHeld and skips the random sets; BikeRace sets it on restart and clears it when the race starts
    FILE* logFile;                        // "scnmgr.log"
    char scenePath[0x104];
};


// 0x004f0310: registers texture `name` with `manager` (see the definition).
int AddResource(UnknownResourceManager* manager, const char* name, const char* path);
// 0x004f0390: counts the textures of the model file `path` into counts[0..3]
// (0x004f00e0), loading them through `resources`; not reconstructed.
int CountModelTextures(const char* path, UnknownSceneResourceManager* resources, unsigned long* counts);
int FindKeyword(int* out, const char* name, const UnknownSceneKeyword* table); // 0x004e9980
// 0x004f00e0: counts the texture by width (256, 128, 64 or 32) in counts[0..3];
// "PROCEDURAL" textures count as 64.
int CountTexture(UnknownSceneTextureInfo* texture, int* counts);

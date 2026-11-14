// Bring-up (best-guess-unreviewed) BaseQuarryEvent::LoadEventObjects (0x004de590, 7952
// bytes; QuarryStuntEvent.cpp, docs/QUARRYSTUNTEVENT.md), unresolved from
// QuarryStuntEvent.cpp. The body is the near miss in
// samples/game/QuarryStuntEventLoaderNearMiss.cpp (2207 instructions against retail's
// 2208; call sequence, allocation sizes, error paths and memory-budget tree aligned), with
// these bring-up changes:
//   * every object the loader creates or calls through a local UnknownQuarry* view is
//     created/called through the canonical class, by the forwards in
//     Group3_QuarryLoadLinks.h (implemented in Group3_QuarryLink*.cpp, separate units
//     because the canonical headers cannot share one);
//   * VisibilityQuadTree::Setup gets the six arguments retail pushes (the near miss
//     passed the scene as a seventh argument to a `ret 0x18` callee);
//   * the objects are allocated inline through the canonical constructors, as retail does
//     (new, EH state, null test, constructor), declared over the views' storage;
//   * the fog objects use Fog.h's Fog/FogOff/FogOn directly; FogOn's folded Create
//     (0x00486a10) is defined at the end of this file and called, as retail calls it;
//   * SkyCube.h, TerrainShadow.h and krusty2's ProjectedShadow.h, CollisionObject.h,
//     ParticleManager.h, VisibilityQuadTree.h and Terrain.h can share this unit and
//     QuarryEvent.h declares EcoSystem::Create, so those objects and the ecosystem are
//     created and called through their own classes;
//   * the memory estimate reads EcoSystem.cpp's preloaded-texture count (0x0059af10).
// Linked score 0.985, 2208 instructions against retail's 2208 (was 0.908 at 2101: the
// out-of-line G3Q_New* allocation helpers dropped retail's inline new/EH/null-test/
// constructor sequences, about 100 instructions). Left, all scheduling: the small slots below
// 0x1c, register choices in the decision tree and around the fog/sky block, the collision
// loop's head (retail enters a rotated loop with a jmp and indexes the entry table with
// [ecx + edx + 4]) and the racer loop's register choice (esi/edi swapped).
// Move into src/reconstructed/QuarryStuntEvent.cpp once it matches.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "../reconstructed/QuarryEvent.h"

#include "../reconstructed/ArcadeObject.h"
#include "../reconstructed/AuralScape.h"
#include "../reconstructed/DebugAlloc.h"
#include "../reconstructed/PCVideoCard.h"
#include "../reconstructed/EventManager.h"
#include "../reconstructed/LightEmitter.h"
#include "../reconstructed/MemTag.h"
#include "../reconstructed/PCGame.h"
#include "../reconstructed/PCRenderTarget.h"
#include "../reconstructed/SceneManager.h"
#include "../reconstructed/SkyCube.h"
#include "shadow/ProjectedShadow.h"
#include "collision/CollisionObject.h"
#include "effects/ParticleManager.h"
#include "visibility/VisibilityQuadTree.h"
#include "broadphase/Terrain.h"
#include "../reconstructed/TerrainShadow.h"
#include "../reconstructed/TextureMap.h"
#include "../reconstructed/TextureMapManager.h"
#include "../reconstructed/TrackGame.h"
#include "../reconstructed/ResourceEntry.h"
#include "Group3_QuarryLoadLinks.h"

// 0x0059af10: the count of ecosystem textures PreloadEcosystem preloaded, which the memory
// estimate weighs at 0x2aaaa bytes (defined in src/reconstructed/EcoSystem.cpp; cleared at
// 0x00457250/0x0045c040, incremented per preloaded texture).
extern int g_UnknownGlobal59af10;

// 0x0045c040 (cdecl): preloads the ecosystem `path` from `stream`.
int PreloadEcosystem(TextureMapManager* manager, char* path, FileStream* stream,
                     int format);

// A physics static model (Scene caster +0x08).
struct QuarryLoadPhysicsObjectView {
    unsigned char field_0x000[0x128];
    CollisionObject* collisionObject;   // +0x128 SoultreePhysicsBaseObjectCore.h
    unsigned char field_0x12c[0x1f4 - 0x12c];
    OverlayTerrainView* terrain;                   // +0x1f4
    void* terrainScale;                        // +0x1f8 copied from the terrain's +0x40
};

// The terrain as the loader reads it.
struct QuarryLoadTerrainView {
    unsigned char field_0x00[0x40];
    void* worldScale;                          // +0x40 SoultreeProbe::worldScale
};

// CarProcedural's terrain pointer (Scene's SceneCarProceduralView
// leaves +0x20c unnamed).
struct QuarryLoadCarProceduralView {
    unsigned char field_0x000[0x20c];
    OverlayTerrainView* groundProvider;            // +0x20c CarProcedural.h
};

// A racer as the loader reads it.
struct QuarryLoadRiderView {
    unsigned char field_0x000[0x1a0];
    D3DIMSoultreeObject* modelNode;               // +0x1a0
};
struct QuarryLoadWreckerView {
    unsigned char field_0x00[0x38];
    CollisionObject* collisionModel;    // +0x38 Wrecker.h
};
struct QuarryLoadLodBikeRaceView {
    unsigned char field_0x000[0x18c];
    unsigned char forceHighLod;                // +0x18c BikeRace.h's "ForceHighLOD"
};
struct QuarryLoadKrustyBikeView {
    unsigned char field_0x000[0x3bc];
    D3DIMSoultreeObject* modelNode;             // +0x3bc bike model
    unsigned char field_0x3c0[0x5c4 - 0x3c0];
    QuarryLoadRiderView* riderCharacter;      // +0x5c4
    unsigned char loadInfo[0x5e0 - 0x5c8];    // +0x5c8 KrustyBike loadInfo (Bike, BikeA5C8) (bytes kept)
    char riderName[0x10];                     // +0x5e0 KrustyBike riderName (Bike)
    CollisionObject* frontWheel;           // +0x5f0
    CollisionObject* rearWheel;            // +0x5f4
    unsigned char field_0x5f8[0x604 - 0x5f8];
    QuarryLoadWreckerView* wrecker;  // +0x604  Bike +0x604 wrecker (its +0x38 collisionModel gets the car's ignore list)
    unsigned char field_0x608[0x735 - 0x608];
    unsigned char isRemote;  // +0x735  KrustyBike +0x735 isRemote
    unsigned char field_0x736[0x740 - 0x736];
    QuarryLoadLodBikeRaceView* race;  // +0x740  KrustyBike +0x740 race
    unsigned char field_0x744[0x1414 - 0x744];
    CollisionObject* bikeAndRiderCollision;  // +0x1414  KrustyBike +0x1414 bikeAndRiderCollision; the loader adds it as an ignored collision owner
    CollisionObject* bikeCollision;  // +0x1418  KrustyBike +0x1418 bikeCollision; the loader adds it as an ignored collision owner
};

// BikeRace (0x400 bytes, constructor 0x00417c30) as the loader uses it.
struct QuarryLoadStartObjectView {
    unsigned char field_0x000[0x210];
    CollisionObject* collisionObject; // +0x210 krusty2 CollisionCharacter::collisionObject
};
class QuarryLoadBikeRaceView : public GameObject {
public:
    explicit QuarryLoadBikeRaceView(int flags);                     // 0x00417c30
    // 0x00417ed0: loads the race; returns this, or 0.
    QuarryLoadBikeRaceView* Create(void* target, void* particles, LightManager* lights,
                                             TextureContext* textures, OverlayTerrainView* terrain,
                                             QuarryEventKrustyBikeCameraView* camera, Scene* scene, BaseQuarryEvent* event,
                                             void* shadow);
    QuarryLoadKrustyBikeView* NextBike(int* iterator);  // 0x004204e0: the next racer, or 0
    void SetDetailLevel(int level);                     // 0x00423790: detail level

    unsigned char field_0x2c[0x38 - 0x2c];
    QuarryLoadKrustyBikeView* localRacer;                     // +0x38 the player's racer
    unsigned char field_0x3c[0x64 - 0x3c];
    QuarryLoadStartObjectView* startGate;              // +0x64 (BikeRaceCore.h)
    int startGateEntry;                                 // +0x68
    QuarryLoadStartObjectView* thirtySecondMan;        // +0x6c
    int thirtySecondManEntry;                           // +0x70
    unsigned char field_0x74[0x400 - 0x74];
};

// The KrustyBikeCamera at BaseQuarryEvent+0x3c (0x3bc bytes).
class QuarryLoadKrustyBikeCameraView : public GameObject {
public:
    explicit QuarryLoadKrustyBikeCameraView(int flags);               // 0x00497cb0
    // 0x00497d90: sets the camera up; returns it.
    QuarryLoadKrustyBikeCameraView* Create(void* target, float a, float b, float c, float d, float e,
                                                   int f, int count, int* presets);
    void SetSubject(OverlayTerrainView* terrain);       // 0x00463600

    Matrix4 worldMatrix;                      // +0x02c Camera worldMatrix
    Matrix4 projectionMatrix;                 // +0x06c Camera projectionMatrix
    Matrix4 viewMatrix;                       // +0x0ac Camera viewMatrix
    Matrix4 screenMatrix[2];                  // +0x0ec Camera screenMatrix
    float fieldOfView;                        // +0x16c Camera fieldOfView
    Vector3 eyePosition;                      // +0x170 Camera eyePosition
    Vector3 viewDirection;                    // +0x17c Camera viewDirection
    Vector3 upDirection;                      // +0x188 Camera upDirection
    float roll;                               // +0x194 Camera roll
    float focalLength;                        // +0x198 Camera focalLength
    float displayAspect;                      // +0x19c Camera displayAspect
    int viewport[6];                          // +0x1a0 Camera viewport
    float heightWidthRatio;                   // +0x1b8 Camera heightWidthRatio
    float nearPlane;                          // +0x1bc Camera nearPlane
    float farPlane;                           // +0x1c0 Camera farPlane
    int targetWidth;                          // +0x1c4 Camera targetWidth
    int targetHeight;                         // +0x1c8 Camera targetHeight
    int viewportClearFrames;                  // +0x1cc Camera viewportClearFrames
    int projectionChangeFrames;               // +0x1d0 Camera projectionChangeFrames
    int fovKeysEnabled;                       // +0x1d4 Camera fovKeysEnabled
    int widthKeysEnabled;                     // +0x1d8 Camera widthKeysEnabled
    float maxFieldOfView;                     // +0x1dc Camera maxFieldOfView
    float minFieldOfView;                     // +0x1e0 Camera minFieldOfView
    void* file;                               // +0x1e4 Camera file
    unsigned char field_0x1e8[0x200 - 0x1e8];
    float lastMoveDistance;                   // +0x200 Camera lastMoveDistance
    float lastTurnAngle;                      // +0x204 Camera lastTurnAngle
    Vector3 previousEyePosition;              // +0x208 Camera previousEyePosition
    Vector3 previousViewDirection;            // +0x214 Camera previousViewDirection
    float followDistance;                     // +0x220 FollowCamera followDistance
    float initialDistance;                    // +0x224 FollowCamera initialDistance
    float lookAheadDistance;                  // +0x228 FollowCamera lookAheadDistance
    float followPitch;                        // +0x22c FollowCamera followPitch
    float initialPitch;                       // +0x230 FollowCamera initialPitch
    float followYaw;                          // +0x234 FollowCamera followYaw
    float initialYaw;                         // +0x238 FollowCamera initialYaw
    float worldYaw;                           // +0x23c FollowCamera worldYaw
    void* subject;                            // +0x240 FollowCamera subject
    int cameraState;                          // +0x244 FollowCamera cameraState
    int savedCameraState;                     // +0x248 FollowCamera savedCameraState
    float savedFieldOfView;                   // +0x24c FollowCamera savedFieldOfView
    int resumeState;                          // +0x250 FollowCamera resumeState
    float resumeFieldOfView;                  // +0x254 FollowCamera resumeFieldOfView
    float targetFieldOfView;                  // +0x258 FollowCamera targetFieldOfView
    Vector3 resumeEyePosition;                // +0x25c FollowCamera resumeEyePosition
    int overrideActive;                       // +0x268 FollowCamera overrideActive
    int isMovingTarget;                       // +0x26c FollowCamera isMovingTarget
    float recordsPerSecond;                   // +0x270 FollowCamera recordsPerSecond
    unsigned char isSettled;  // +0x274 FollowCamera isSettled
    unsigned char isPlaced;                   // +0x275 FollowCamera isPlaced
    unsigned char isCrashView;                // +0x276 FollowCamera isCrashView
    bool settleChecked;                       // +0x277 FollowCamera settleChecked
    unsigned char wasWide;                    // +0x278 FollowCamera wasWide
    unsigned char isBlending;                 // +0x279 FollowCamera isBlending
    unsigned char field_0x27a[0x27c - 0x27a];
    void* targetEaseX;                        // +0x27c FollowCamera targetEaseX
    void* targetEaseY;                        // +0x280 FollowCamera targetEaseY
    void* targetEaseZ;                        // +0x284 FollowCamera targetEaseZ
    void* positionEaseX;                      // +0x288 FollowCamera positionEaseX
    void* positionEaseY;                      // +0x28c FollowCamera positionEaseY
    void* positionEaseZ;                      // +0x290 FollowCamera positionEaseZ
    void* fieldOfViewEase;                    // +0x294 FollowCamera fieldOfViewEase
    void* distanceEase;                       // +0x298 FollowCamera distanceEase
    Vector3 followHeading;                    // +0x29c FollowCamera followHeading
    Vector3 cachedTarget;                     // +0x2a8 FollowCamera cachedTarget
    Vector3 targetPoint;                      // +0x2b4 FollowCamera targetPoint
    float targetDistance;                     // +0x2c0 FollowCamera targetDistance
    float savedDistance;                      // +0x2c4 FollowCamera savedDistance
    float savedPitch;                         // +0x2c8 FollowCamera savedPitch
    float savedYaw;                           // +0x2cc FollowCamera savedYaw
    float defaultFollowHeight;                // +0x2d0 FollowCamera defaultFollowHeight
    float followHeight;                       // +0x2d4 FollowCamera followHeight
    float lift;                               // +0x2d8 FollowCamera lift
    int pointCapacity;                        // +0x2dc FollowCamera pointCapacity
    int pointCount;                           // +0x2e0 FollowCamera pointCount
    void* points;                             // +0x2e4 FollowCamera points
    float yawOffset;                          // +0x2e8 FollowCamera yawOffset
    float subjectCellSize;                    // +0x2ec FollowCamera subjectCellSize
    float state4FieldOfView;                  // +0x2f0 FollowCamera state4FieldOfView
    float distanceFovMinimum;                 // +0x2f4 FollowCamera distanceFovMinimum
    float distanceForMinimumFov;              // +0x2f8 FollowCamera distanceForMinimumFov
    float distanceFovMaximum;                 // +0x2fc FollowCamera distanceFovMaximum
    float distanceForMaximumFov;              // +0x300 FollowCamera distanceForMaximumFov
    float blendClock;                         // +0x304 FollowCamera blendClock
    float turnYaw;                            // +0x308 FollowCamera turnYaw
    int stateIndex;                           // +0x30c FollowCamera stateIndex
    int stateCount;                           // +0x310 FollowCamera stateCount
    int states[8];                            // +0x314 FollowCamera states
    unsigned int frameCount;                  // +0x334 FollowCamera frameCount
    void* frames;                             // +0x338 FollowCamera frames
    unsigned char field_0x33c[0x340 - 0x33c];
    float frameTime;                          // +0x340 FollowCamera frameTime
    Matrix4 orientation;                      // +0x344 FollowCamera orientation
    int targetCharacter;  // +0x384 VehicleCamera targetCharacter
    int targetCar;  // +0x388 VehicleCamera targetCar
    int targetCaster;  // +0x38C VehicleCamera targetCaster
    unsigned char vehicleMode;  // +0x390 VehicleCamera vehicleMode
    unsigned char targetPointCached;  // +0x391 VehicleCamera targetPointCached
    unsigned char targetHeadingCached;  // +0x392 VehicleCamera targetHeadingCached
    unsigned char field_0x393;
    QuarryLoadKrustyBikeView* vehicle;  // +0x394 VehicleCamera vehicle
    Vector3 cachedTargetHeading;              // +0x398 VehicleCamera cachedTargetHeading
    Vector3 cachedTargetPoint;                // +0x3a4 VehicleCamera cachedTargetPoint
    QuarryLoadKrustyBikeView* bike;  // +0x3B0 BikeCamera bike
    QuarryLoadKrustyBikeView* krustyBike;  // +0x3B4 KrustyBikeCamera krustyBike
    unsigned char raceView[0x3bc - 0x3b8];  // +0x3b8 KrustyBikeCamera raceView (bytes kept)
};

// Fog (0x5c bytes, QuarryEvent.h's Fog) and the objects the sky adds.
class QuarryLoadFogView : public GameObject {
public:
    explicit QuarryLoadFogView(int flags);                      // 0x00462620
    Fog* Create(void* target, unsigned int color, float visibility, float haziness,
                float a, float b, float c);

    unsigned char field_0x2c[0x5c - 0x2c];
};
class QuarryLoadFogOffView : public GameObject {
public:
    explicit QuarryLoadFogOffView(int flags);                // 0x00462e10
};
class QuarryLoadFogOnView : public GameObject {
public:
    explicit QuarryLoadFogOnView(int flags);                  // 0x00462e90
    GameObject* Create(void* target, Fog* fog);  // 0x00486a10

    unsigned char fog[0x30 - 0x2c];  // +0x2c FogOn fog (bytes kept)
};

// The last race object (0x48 bytes).
class QuarryLoadSoultreeQuadTreeRendererView : public GameObject {
public:
    explicit QuarryLoadSoultreeQuadTreeRendererView(int flags);               // 0x00504490
    void InsertNodesInQuadtree();                              // 0x005046e0

    unsigned char field_0x2c[0x48 - 0x2c];
};

// Retail allocates each object inline (operator new, EH state, null test, then the (int)
// constructor). These declare the canonical constructors (??0X@@QAE@H@Z) over the views,
// whose storage gives retail's allocation sizes; they add no members. EcoSystem's
// constructor is declared in QuarryEvent.h.
class KrustyBikeCamera : public QuarryLoadKrustyBikeCameraView {
public:
    explicit KrustyBikeCamera(int flags);                      // 0x00497cb0
};
class BikeRace : public QuarryLoadBikeRaceView {
public:
    explicit BikeRace(int flags);                              // 0x00417c30
};
class SoultreeQuadTreeRenderer : public QuarryLoadSoultreeQuadTreeRendererView {
public:
    explicit SoultreeQuadTreeRenderer(int flags);              // 0x00504490
};

// The overlays' race hooks.
struct QuarryLoadOverlayHooks {
    void SetStatsSource(QuarryLoadBikeRaceView* race);       // 0x00519880 (StatsOverlay)
    // 0x0051d730 is ChatOverlay::CollectRacers; VisualCue's CollectRacers
    // below has the same view signature, so this one keeps its address name.
    void CollectChatRacers(QuarryLoadBikeRaceView* race);       // 0x0051d730 (ChatOverlay)
    void TakeRacers(QuarryLoadBikeRaceView* race, void* chat); // 0x0051baf0 (RadarOverlay::TakeRacers)
    void CollectRacers(QuarryLoadBikeRaceView* race);       // 0x0048af20 (VisualCue)
};

// The bytes the render target's colour buffers take.
static inline int FrameBufferBytes(RenderTarget* target) {
    return PixelFormatBytesPerPixel(target->surfacePixelFormat) * (target->frameModulus + 1) * target->targetHeight *
           target->targetWidth;
}

// 0x004de590
int BaseQuarryEvent::LoadEventObjects(LoadProgressFn progress) {
    int terrainBytes;
    int textureBytes;
    int forced;
    int needed;
    int sceneSteps;
    float terrainWidth;
    int hasCube;
    unsigned long modelCounts[4];
    int presets[8];
    TextureContext terrainTextures;
    TextureContext modelTextures;
    TextureContext skyTextures;
    MEMORYSTATUS memory;
    char text[0x184];
    char ecosystem[0x104];
    char path[0x104];
    char key[0x100];
    char terrainPath[0x104];

    GlobalMemoryStatus(&memory);
    ecosystem[0] = 0;
    g_TrackGame->sceneObject->CountSceneTextures(modelCounts, (char*)&terrainWidth, (char*)&hasCube,
                                                (int)ecosystem);
    sceneSteps = 1;
    if (g_TrackGame->sceneObject->CountSceneObjects(&sceneSteps)) {
        int step = (int)(sceneSteps / (sceneSteps * 0.1f));
        if (progress)
            progress(&step);
    }
    FileStream* stream = new(__FILE__, 252) FileStream(g_ResourceManager);
    FileStream* preloaded = stream;
    if (ecosystem[0]) {
        if (strstr(ecosystem, ".est"))
            strcpy(strstr(ecosystem, ".est"), ".esb");
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, ecosystem, "rb", 0))
            sprintf(text, "No Ecosystem found in env file.  Ecosystem not preloaed.\n");
        else
            PreloadEcosystem(g_TrackGame->textureManager, ecosystem, stream, 0x115c);
    }

    // The texture detail levels: 0 keeps every texture, 1 and 2 halve them.
    if (g_TrackGame->bladeRenderer) {
        g_TrackGame->texturesCached = 0;
        if (memory.dwTotalPhys > 0x5a00000) {
            terrainTextures.halvings = 0;
            skyTextures.halvings = 0;
            modelTextures.halvings = 0;
        } else if (memory.dwTotalPhys > 0x3c00000) {
            terrainTextures.halvings = 1;
            skyTextures.halvings = 0;
            modelTextures.halvings = 0;
        } else if (memory.dwTotalPhys <= 0x2000000) {
            terrainTextures.halvings = 2;
            skyTextures.halvings = 2;
            modelTextures.halvings = 2;
        } else {
            terrainTextures.halvings = 1;
            skyTextures.halvings = 1;
            modelTextures.halvings = 1;
        }
    } else {
        int videoMemory = ((PCRenderTarget*)renderContext)->targetDisplay->totalVideoMemory;
        sprintf(key, "DriverInfo\\%s\\VideoMemoryMB", ((PCRenderTarget*)renderContext)->targetDisplay->driverGuidText);
        int megabytes = g_TrackGame->GetRegistryInt(key, -1);
        forced = g_TrackGame->mode.videoSettings[0];
        if (megabytes != -1)
            videoMemory = megabytes << 20;
        PCRenderTarget* target = (PCRenderTarget*)renderContext;
        int available = videoMemory - FrameBufferBytes(target);
        if (g_TrackGame->osVersion.platformId != 2 && !(target->targetDisplay->driverCaps & 0x400)) {
            available = videoMemory - 0x400000;
            if (!(target->deviceCaps & 0x4000))
                available = videoMemory - 0x200000;
        }
        terrainBytes = EstimateTerrainBytes((int)terrainWidth);
        int cubeBytes = hasCube ? EstimateCubeBytes() : 0;
        textureBytes = EstimateModelTextureBytes((int*)modelCounts);
        int raceBytes = EstimateRaceBytes();
        needed = g_UnknownGlobal59af10 * 0x2aaaa + raceBytes + textureBytes + cubeBytes + terrainBytes + 0xa0000;
        terrainTextures.halvings = 0;
        skyTextures.halvings = 0;
        modelTextures.halvings = 0;
        if (g_TrackGame->display->isAGP) {
            g_TrackGame->texturesCached = 0;
            if (available - needed <= 0) {
                if (forced) {
                    if (needed - terrainBytes + terrainBytes / 4 < available) {
                        terrainTextures.halvings = 1;
                    } else if (needed - cubeBytes - terrainBytes + (cubeBytes + terrainBytes) / 4 < available) {
                        terrainTextures.halvings = 1;
                        skyTextures.halvings = 1;
                    } else if (needed - textureBytes - cubeBytes - terrainBytes +
                                   (textureBytes + cubeBytes + terrainBytes) / 4 < available) {
                        terrainTextures.halvings = 1;
                        skyTextures.halvings = 1;
                        modelTextures.halvings = 1;
                    } else {
                        terrainTextures.halvings = 2;
                        skyTextures.halvings = 2;
                        modelTextures.halvings = 2;
                    }
                } else if (available - needed + memory.dwTotalPhys <= 0x2800000 && memory.dwTotalPhys <= 0x5000000) {
                    if (memory.dwTotalPhys > 0x3c00000) {
                        terrainTextures.halvings = 1;
                    } else if (needed - terrainBytes + terrainBytes / 4 < available) {
                        terrainTextures.halvings = 1;
                    } else if (needed - cubeBytes - terrainBytes + (cubeBytes + terrainBytes) / 4 < available) {
                        terrainTextures.halvings = 1;
                        skyTextures.halvings = 1;
                    } else if (needed - textureBytes - cubeBytes - terrainBytes +
                                   (textureBytes + cubeBytes + terrainBytes) / 4 < available) {
                        terrainTextures.halvings = 1;
                        skyTextures.halvings = 1;
                        modelTextures.halvings = 1;
                    } else if (needed - textureBytes - cubeBytes - terrainBytes + (textureBytes + cubeBytes) / 4 +
                                   terrainBytes / 16 < available) {
                        terrainTextures.halvings = 2;
                        skyTextures.halvings = 1;
                        modelTextures.halvings = 1;
                    } else if (needed - textureBytes - cubeBytes - terrainBytes + textureBytes / 4 +
                                   (cubeBytes + terrainBytes) / 16 < available) {
                        terrainTextures.halvings = 2;
                        skyTextures.halvings = 2;
                        modelTextures.halvings = 1;
                    } else {
                        terrainTextures.halvings = 2;
                        skyTextures.halvings = 2;
                        modelTextures.halvings = 2;
                    }
                }
            }
        } else {
            int percent = g_TrackGame->GetRegistryInt("MarginPercentage", 10);
            int minimum = g_TrackGame->GetRegistryInt("MinMarginKBytes", 0x200) << 10;
            int margin = percent * available / 100;
            if (margin < minimum)
                margin = minimum;
            available -= margin;
            if (available < 0)
                available = 0;
            if (available > needed) {
                terrainTextures.halvings = 0;
                skyTextures.halvings = 0;
                modelTextures.halvings = 0;
                g_TrackGame->texturesCached = 0;
            } else if (!forced && memory.dwTotalPhys > 0x5a00000) {
                terrainTextures.halvings = 0;
                skyTextures.halvings = 0;
                modelTextures.halvings = 0;
                g_TrackGame->texturesCached = 1;
            } else if (needed - terrainBytes + terrainBytes / 4 < available) {
                g_TrackGame->texturesCached = 0;
                terrainTextures.halvings = 1;
            } else if (!forced && memory.dwTotalPhys > 0x3c00000) {
                g_TrackGame->texturesCached = 1;
                terrainTextures.halvings = 1;
            } else {
                int all = textureBytes + cubeBytes + terrainBytes;
                if (needed - textureBytes - cubeBytes - terrainBytes + all / 4 < available) {
                    g_TrackGame->texturesCached = 0;
                    terrainTextures.halvings = 1;
                    skyTextures.halvings = 1;
                    modelTextures.halvings = 1;
                } else if (needed - textureBytes - cubeBytes - terrainBytes + (textureBytes + cubeBytes) / 4 +
                               terrainBytes / 16 < available) {
                    g_TrackGame->texturesCached = 0;
                    terrainTextures.halvings = 2;
                    skyTextures.halvings = 1;
                    modelTextures.halvings = 1;
                } else if (needed - textureBytes - cubeBytes - terrainBytes + textureBytes / 4 +
                               (cubeBytes + terrainBytes) / 16 < available) {
                    g_TrackGame->texturesCached = 0;
                    terrainTextures.halvings = 2;
                    skyTextures.halvings = 2;
                    modelTextures.halvings = 1;
                } else {
                    if (needed - textureBytes - cubeBytes - terrainBytes + all / 16 < available)
                        g_TrackGame->texturesCached = 0;
                    else
                        g_TrackGame->texturesCached = !forced;
                    terrainTextures.halvings = 2;
                    skyTextures.halvings = 2;
                    modelTextures.halvings = 2;
                }
            }
        }
        if (g_TrackGame->texturesCached) {
            opaqueTextureGroup = (GameObject*)g_TrackGame->textureManager->CreateGroup(
                ((PCRenderTarget*)renderContext)->surfacePixelFormat, 0, 2, 1);
            alphaTextureGroup = (GameObject*)g_TrackGame->textureManager->CreateGroup(0x115c, 0, 2, 1);
        }
    }
    modelTextures.manager = skyTextures.manager = terrainTextures.manager = g_TrackGame->textureManager;
    modelTextures.opaqueGroup = skyTextures.opaqueGroup = terrainTextures.opaqueGroup = (ManagedTextureGroup*)opaqueTextureGroup;
    modelTextures.alphaGroup = skyTextures.alphaGroup = terrainTextures.alphaGroup = (ManagedTextureGroup*)alphaTextureGroup;
    modelTextures.opaqueFormat = skyTextures.opaqueFormat = terrainTextures.opaqueFormat = ((PCRenderTarget*)renderContext)->surfacePixelFormat;
    modelTextures.alphaFormat = skyTextures.alphaFormat = terrainTextures.alphaFormat = 0x115c;
    if (progress)
        progress(0);

    g_MemTagStack->Push("Audio");
    auralScape = (new(__FILE__, 576) AuralScape(g_TrackGame->mode.soundOn))
                     ->Create(renderContext, 16);
    auralScape->reverbEnabled = g_TrackGame->mode.eaxEnabled;
    auralScapeListener = auralScape->NextListenerNumber();
    auralScape->AddListener(auralScapeListener, 1.0f, 1.0f);
    g_MemTagStack->Pop(0);

    lightManager = new(__FILE__, 582) LightManager(1);
    AppendChild(lightManager->SetRenderContext(renderContext), -1);
    presets[0] = 0;
    presets[1] = 1;
    presets[2] = 2;
    presets[3] = 3;
    presets[4] = 4;
    presets[5] = 5;
    presets[6] = 6;
    presets[7] = 7;
    raceCamera = (QuarryEventKrustyBikeCameraView*)(new(__FILE__, 585) KrustyBikeCamera(1))
                     ->Create(renderContext, 0.55f, 0.3f, 27.0f, 7.0f, 1.048f, 2, 8, presets);
    if (!AppendChild((GameObject*)raceCamera, -1)) {
        sprintf(text, "KrustyBikeCamera not created.\n");
        return 0;
    }
    if (progress)
        progress(0);
    CreateOverlays();

    g_MemTagStack->Push("Particles");
    particleManager = new(__FILE__, 613) ParticleManager(g_TrackGame->mode.particlesOn);
    g_MemTagStack->Push("Scene");
    eventScene = (new(__FILE__, 624) Scene(isEnabled))
                     ->AttachSceneManager(g_TrackGame->sceneObject, (int)auralScape, (int)&modelTextures, 8);
    if (!eventScene) {
        sprintf(text, "Scene not created.\n");
        return 0;
    }
    if (!eventScene->Load(renderContext, lightManager, (int)particleManager, (int)g_TrackGame->textureManager,
                                           (int)&modelTextures, "Teraform\\Skies", (void (*)(int))progress,
                                           (int)ceil(sceneSteps * 0.1f)))
        return 0;
    if (progress)
        progress(0);

    g_MemTagStack->Push("QuadTree");
    VisibilityQuadTree* visibility = (VisibilityQuadTree*)(new(__FILE__, 659) VisibilityQuadTree(1))
        ->Setup((int)renderContext, 0.0f, 0.0f,
                                eventScene->environment->terrainWidth * eventScene->environment->terrainWidthScale * 256.0f,
                                eventScene->environment->terrainBreadth * eventScene->environment->terrainBreadthScale * 256.0f,
                                96.0f);
    if (!visibility)
        sprintf(text, "VisibilityQuadTree not created.\n");
    g_MemTagStack->Pop(0);
    SceneLight* shadowLight = 0;
    SceneLight* flareLight = 0;
    for (int i = 0; i < eventScene->lightCount; i++) {
        if (eventScene->sceneLights[i].emitsLight) {
            if (eventScene->sceneLights[i].emitter) {
                AppendChild(eventScene->sceneLights[i].emitter, -1);
                if (eventScene->sceneLights[i].type == 6)
                    ambientLight = eventScene->sceneLights[i].emitter;
                else if (eventScene->sceneLights[i].type == 2 || eventScene->sceneLights[i].type == 4)
                    sunLight = eventScene->sceneLights[i].emitter;
            }
            if (!shadowLight && eventScene->sceneLights[i].castsShadows)
                shadowLight = &eventScene->sceneLights[i];
            if (!flareLight && eventScene->sceneLights[i].hasLensFlare)
                flareLight = &eventScene->sceneLights[i];
        }
    }
    if (shadowLight) {
        projectedShadow = new(__FILE__, 698) ProjectedShadow(g_TrackGame->mode.shadowsOn);
        projectedShadow = ((ProjectedShadow*)projectedShadow)->Init((ShadowRenderTarget*)renderContext, g_TrackGame->textureManager);
    }
    if (!preloaded)
        stream = new(__FILE__, 713) FileStream(g_ResourceManager);

    g_MemTagStack->Push("Sky");
    char* cubeFile = eventScene->environment->cubeFile;
    if (cubeFile && *cubeFile) {
        eventFog = (new(__FILE__, 720) Fog(1))
                         ->Create(renderContext, eventScene->fog->color, eventScene->fog->visibility,
                                  eventScene->fog->haziness, 768.0f, 256.0f, 128.0f);
        if (!AppendChild((GameObject*)eventFog, -1))
            sprintf(text, "Fog not created.\n");
        if (eventFog)
            eventFog->SetDetailLevel(g_TrackGame->mode.videoSettings[4]);
        AppendChild((new(__FILE__, 729) FogOff(1))->SetRenderContext(renderContext), -1);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, eventScene->environment->cubeFile, "rb", 0)) {
            sprintf(text, "No Sky found in resources.  Sky not created.\n");
            if (stream)
                delete stream;
            return 0;
        }
        skyCube = (new(__FILE__, 744) SkyCube(g_TrackGame->mode.skyCubeOn))
                         ->Create(renderContext, stream, (CubeTextureContextView*)&skyTextures, 1024.0f);
        if (!AppendChild((GameObject*)skyCube, -1))
            sprintf(text, "SkyCube not created.\n");
        AppendChild((new(__FILE__, 750) FogOn(1))->Create(renderContext, eventFog), -1);
        eventFog->SetParameters(eventScene->fog->color, eventFog->fogVisibility, eventFog->fogHaziness);
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("Terrain");
    if (!g_TrackGame->sceneObject->OpenArchiveFile(terrainStream, eventScene->environment->terrainFile, "rb",
                                                                   (int)terrainPath)) {
        sprintf(text, "No Terrain found in env file.  Terrain not created.\n");
        return 0;
    }
    eventTerrain = (OverlayTerrainView*)(new(__FILE__, 799) Terrain(1))
                     ->Load((int)renderContext, (TerrainStream*)terrainStream, 1,
                                             (TerrainSurfaceDesc*)&terrainTextures,
                                             g_TrackGame->mode.videoSettings[3], 0,
                                             eventScene->environment->detailTexture, 0);
    if (!AppendChild((GameObject*)eventTerrain, -1)) {
        sprintf(text, "Terrain not created.\n");
        return 0;
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("Shadow");
    if (projectedShadow) {
        terrainShadow = (new(__FILE__, 822) TerrainShadow(g_TrackGame->mode.shadowsOn))
                         ->Attach((int)renderContext, (Terrain*)eventTerrain,
                                  (ProjectedShadow*)projectedShadow);
        AppendChild((GameObject*)terrainShadow, -1);
    }
    AppendChild(visibility, -1);

    g_MemTagStack->Push("EcoSystem");
    if (!stream)
        stream = new(__FILE__, 848) FileStream(g_ResourceManager);
    if (strstr(eventScene->environment->ecosystemFile, ".est"))
        strcpy(strstr(eventScene->environment->ecosystemFile, ".est"), ".esb");
    if (stream && !g_TrackGame->sceneObject->OpenArchiveFile(stream, eventScene->environment->ecosystemFile,
                                                                           "rb", 0)) {
        sprintf(text, "No Ecosystem found in env file.  Ecosystem not created.\n");
    } else {
        ecoSystem = (new(__FILE__, 870) EcoSystem(1))
                         ->Create(renderContext, g_TrackGame->textureManager,
                                                 (EcoTerrainView*)eventTerrain, lightManager,
                                                 (char*)eventScene->environment, stream, 0x115c,
                                                 g_TrackGame->mode.raceSettings.treeCollision,
                                                 g_TrackGame->mode.videoSettings[1]);
        if (!ecoSystem)
            sprintf(text, "EcoSystem not created.\n");
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("3DObjects");
    if (!PrepareEvent3DObjects())
        return 0;
    g_MemTagStack->Push("BikeRace");
    raceView = (EventBikeRaceView*)new(__FILE__, 898) BikeRace(0);
    g_MemTagStack->Push("Particles");
    if (particleManager)
        particleManager = ((ParticleManager*)particleManager)
                         ->Create((int)renderContext, g_TrackGame->textureManager,
                                  eventScene->environment->particleTexture, (int)lightManager);
    g_MemTagStack->Push("3DObjects");
    if (!Create3DObjects((int)&modelTextures))
        return 0;
    g_TrackGame->mode.networkGame = eventScene->crowdPresent;
    g_MemTagStack->Push("BikeRace");
    raceView = (EventBikeRaceView*)((QuarryLoadBikeRaceView*)raceView)
                     ->Create(renderContext, particleManager, lightManager, &modelTextures, eventTerrain, raceCamera,
                                             eventScene, this, projectedShadow);
    if (!raceView) {
        sprintf(text, "BikeRace create failed.\n");
        return 0;
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("Collision");
    if (eventScene->staticModels) {
        for (int i = 0; i < eventScene->staticModels->casterCount; i++) {
            SceneStaticModel* caster = &eventScene->staticModels->casters[i];
            if (caster->physics) {
                OverlayTerrainView* terrain = eventTerrain;
                QuarryLoadPhysicsObjectView* physics = (QuarryLoadPhysicsObjectView*)caster->physicsObject;
                physics->terrain = terrain;
                if (terrain)
                    physics->terrainScale = ((QuarryLoadTerrainView*)terrain)->worldScale;
                ((QuarryLoadPhysicsObjectView*)caster->physicsObject)->collisionObject->SnapPlacement();
                ((QuarryLoadPhysicsObjectView*)caster->physicsObject)->collisionObject->SetUseBroadphase(1);
                ((QuarryLoadPhysicsObjectView*)caster->physicsObject)->collisionObject->SetOwner(caster->physicsObject, 0x68);
            } else if (caster->collisionObject && ((D3DIMSoultreeObject*)caster->model)->CountNodes() == 1) {
                ((CollisionObject*)caster->collisionObject)->SnapPlacement();
                ((CollisionObject*)caster->collisionObject)->SetUseBroadphase(1);
            }
        }
    }
    if (((QuarryLoadBikeRaceView*)raceView)->thirtySecondMan && ((QuarryLoadBikeRaceView*)raceView)->thirtySecondMan->collisionObject) {
        ((QuarryLoadBikeRaceView*)raceView)->thirtySecondMan->collisionObject->Release();
        ((QuarryLoadBikeRaceView*)raceView)->thirtySecondMan->collisionObject = 0;
    }
    if (((QuarryLoadBikeRaceView*)raceView)->startGate && ((QuarryLoadBikeRaceView*)raceView)->startGate->collisionObject) {
        ((QuarryLoadBikeRaceView*)raceView)->startGate->collisionObject->Release();
        ((QuarryLoadBikeRaceView*)raceView)->startGate->collisionObject = 0;
    }
    if (eventScene->animationTable) {
        for (int i = 0; i < eventScene->animationTable->entryCount; i++) {
            if (eventScene->animationTable->entries[i].keyFramed) {
                SceneCollisionCharacterView* character = eventScene->animationTable->entries[i].character;
                if (character->collisionObject) {
                    ((CollisionObject*)character->collisionObject)->SnapPlacement();
                    ((CollisionObject*)character->collisionObject)->SetUseBroadphase(1);
                    ((CollisionObject*)character->collisionObject)->SetIgnoreListMode(0);
                    if (i != ((QuarryLoadBikeRaceView*)raceView)->thirtySecondManEntry && i != ((QuarryLoadBikeRaceView*)raceView)->startGateEntry) {
                        int iterator = 0;
                        for (QuarryLoadKrustyBikeView* racer = ((QuarryLoadBikeRaceView*)raceView)->NextBike(&iterator);
                             racer; racer = ((QuarryLoadBikeRaceView*)raceView)->NextBike(&iterator)) {
                            if (!racer->isRemote) {
                                ((CollisionObject*)character->collisionObject)->AddIgnoredOwner(racer->bikeCollision);
                                ((CollisionObject*)character->collisionObject)->AddIgnoredOwner(racer->bikeAndRiderCollision);
                                ((CollisionObject*)character->collisionObject)->AddIgnoredOwner(racer->frontWheel);
                                ((CollisionObject*)character->collisionObject)->AddIgnoredOwner(racer->rearWheel);
                                ((CollisionObject*)character->collisionObject)
                                    ->AddIgnoredOwner(racer->wrecker->collisionModel);
                            }
                        }
                    } else if (character->collisionObject) {
                        ((CollisionObject*)character->collisionObject)->Release();
                        character->collisionObject = 0;
                    }
                }
            } else {
                SceneCarProceduralView* car = eventScene->animationTable->entries[i].proceduralObject;
                ((QuarryLoadCarProceduralView*)car)->groundProvider = eventTerrain;
                ((CollisionObject*)car->sensorCollider)->SetIgnoreListMode(1);
                for (int j = 0; j < eventScene->animationTable->entryCount; j++) {
                    SceneAnimation* entry = &eventScene->animationTable->entries[j];
                    if (entry->keyFramed)
                        ((CollisionObject*)car->sensorCollider)
                            ->AddIgnoredOwner((CollisionObject*)entry->character->collisionObject);
                    else
                        ((CollisionObject*)car->sensorCollider)
                            ->AddIgnoredOwner((CollisionObject*)entry->proceduralObject->bodyCollider);
                }
                ((CollisionObject*)car->bodyCollider)->SnapPlacement();
                ((CollisionObject*)car->bodyCollider)->SetUseBroadphase(1);
                ((CollisionObject*)car->bodyCollider)->SetOwner(car, 0x69);
                ((CollisionObject*)car->bodyCollider)->SetIgnoreListMode(0);
                int iterator = 0;
                for (QuarryLoadKrustyBikeView* racer = ((QuarryLoadBikeRaceView*)raceView)->NextBike(&iterator); racer;
                     racer = ((QuarryLoadBikeRaceView*)raceView)->NextBike(&iterator)) {
                    if (!racer->isRemote) {
                        ((CollisionObject*)car->bodyCollider)->AddIgnoredOwner(racer->bikeCollision);
                        ((CollisionObject*)car->bodyCollider)->AddIgnoredOwner(racer->bikeAndRiderCollision);
                        ((CollisionObject*)car->bodyCollider)->AddIgnoredOwner(racer->frontWheel);
                        ((CollisionObject*)car->bodyCollider)->AddIgnoredOwner(racer->rearWheel);
                        ((CollisionObject*)car->bodyCollider)->AddIgnoredOwner(racer->wrecker->collisionModel);
                    }
                }
            }
        }
    }
    if (progress)
        progress(0);
    AppendChild(eventScene, -1);

    g_MemTagStack->Push("Shadow");
    eventScene->AttachCasterShadows((ProjectedShadow*)projectedShadow, g_TrackGame->mode.shadowsOn);
    g_MemTagStack->Pop(0);
    if (ecoSystem)
        AppendChild((GameObject*)ecoSystem, -1);
    AppendChild(raceView, -1);
    OnRaceViewAttached((int)raceView);
    AppendChild((GameObject*)particleManager, -1);
    UnknownVirtualSlot28();
    AttachOverlays();

    g_MemTagStack->Push("3DObjects");
    int kind = g_TrackGame->mode.GetSelectedSeries();
    if (kind >= 2 && kind <= 4)
        sprintf(path, "%s\\%s", "Res", "podium.slt");
    else
        sprintf(path, "%s\\%s", "Res", "truckpodium.slt");
    g_TrackGame->eventManager->podiumObject =
        (ArcadeObject*)(new(__FILE__, 1093) ArcadeObject(0))
            ->Create(renderContext, (int)lightManager, (int)&modelTextures, path,
                                    g_TrackGame->eventManager->podiumPosition, 0, 0, 2.0f, 0.5f, 0.5f, 0);
    if (!raceView->AppendChild((GameObject*)g_TrackGame->eventManager->podiumObject, -1))
        return 0;
    if (auralScape) {
        auralScape->groundSubject = (FollowCameraTerrainView*)eventTerrain;
        AppendChild(auralScape, -1);
    }

    g_MemTagStack->Push("Shadow");
    if (projectedShadow) {
        AppendChild((GameObject*)projectedShadow, -1);
        ((ProjectedShadow*)projectedShadow)->AddCaster((ShadowCaster*)((QuarryLoadBikeRaceView*)raceView)->localRacer->modelNode);
        ((ProjectedShadow*)projectedShadow)
            ->AddCaster((ShadowCaster*)((QuarryLoadBikeRaceView*)raceView)->localRacer->riderCharacter->modelNode);
        ((ProjectedShadow*)projectedShadow)->SetLight((const ShadowVec3*)&shadowLight->position,
                                                (const ShadowVec3*)&shadowLight->look, 3);
        ((ProjectedShadow*)projectedShadow)->SetShadowStrength(0x80);
        int ambient = (int)(ambientLight->ambientRGBA[1] * 255.0f);
        if (!ambientLight)
            ambientLight = lightManager->FindLightOfType(6);
        if (ambientLight)
            ambient = (int)(ambientLight->ambientRGBA[1] * 255.0f);
        ((ProjectedShadow*)projectedShadow)->SetShadowGrey(ambient);
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("Collision");
    if (!CreateEventCollision())
        return 0;
    g_MemTagStack->Push("3DObjects");
    CreateRacer3DObjects((int)((QuarryLoadBikeRaceView*)raceView)->localRacer);
    g_MemTagStack->Push("BikeRace");
    ((QuarryLoadKrustyBikeCameraView*)raceCamera)->SetSubject(eventTerrain);
    QuarryLoadKrustyBikeCameraView* camera = (QuarryLoadKrustyBikeCameraView*)raceCamera;
    QuarryLoadKrustyBikeView* bike = ((QuarryLoadBikeRaceView*)raceView)->localRacer;
    if (camera->bike && !bike->race->forceHighLod) {
        camera->bike->modelNode->SetLowestLod(1);
        camera->bike->riderCharacter->modelNode->SetLowestLod(1);
        if (!camera->bike->modelNode->currentLod)
            camera->bike->modelNode->SetLod(1);
        if (!camera->bike->riderCharacter->modelNode->currentLod)
            camera->bike->riderCharacter->modelNode->SetLod(1);
    }
    camera->vehicle = bike;
    camera->targetPointCached = 0;
    camera->targetHeadingCached = 0;
    camera->vehicleMode = 1;
    camera->targetCaster = 0;
    camera->targetCharacter = 0;
    camera->targetCar = 0;
    camera->bike = bike;
    camera->isSettled = 0;
    camera->krustyBike = bike;
    if (!bike->race->forceHighLod) {
        bike->modelNode->SetLowestLod(0);
        camera->bike->riderCharacter->modelNode->SetLowestLod(0);
    }
    camera->isSettled = 0;
    if (statsOverlay)
        ((QuarryLoadOverlayHooks*)statsOverlay)->SetStatsSource((QuarryLoadBikeRaceView*)raceView);
    if (chatOverlay)
        ((QuarryLoadOverlayHooks*)chatOverlay)->CollectChatRacers((QuarryLoadBikeRaceView*)raceView);
    if (radarOverlay)
        ((QuarryLoadOverlayHooks*)radarOverlay)->TakeRacers((QuarryLoadBikeRaceView*)raceView, chatOverlay);
    if (visualCue)
        ((QuarryLoadOverlayHooks*)visualCue)->CollectRacers((QuarryLoadBikeRaceView*)raceView);
    g_MemTagStack->Pop(0);

    if (!g_TrackGame->mode.instrumentOverlayOn)
        DisableByClassName("InstrumentOverlay");
    if (!g_TrackGame->mode.skyCubeOn)
        DisableByClassName("SkyCube");
    if (!g_TrackGame->mode.particlesOn) {
        DisableByClassName("ParticleManager");
        DisableByClassName("DirtParticleEmitter");
    }
    if (!g_TrackGame->mode.shadowsOn) {
        DisableByClassName("ProjectedShadow");
        DisableByClassName("TerrainShadow");
        DisableByClassName("D3DIMSoultreeShadow");
    }
    ((RenderTarget*)renderContext)->clearTarget = !(skyCube && g_TrackGame->mode.skyCubeOn);
    loaded = 1;
    if (g_TrackGame->texturesCached) {
        g_MemTagStack->Push("TextureCache");
        g_TrackGame->textureManager->ShareCacheLimit();
    }
    g_MemTagStack->Pop(0);
    eventScene->SetDetailLevel(g_TrackGame->mode.videoSettings[2]);
    G3Q_BikeRaceSetDetailLevel(raceView, g_TrackGame->mode.videoSettings[2]);
    if (progress)
        progress(0);
    QuarryLoadSoultreeQuadTreeRendererView* last = new(__FILE__, 1251) SoultreeQuadTreeRenderer(1);
    last->SetRenderContext(renderContext);
    AppendChild(last, -1);
    last->InsertNodesInQuadtree();
    return 1;
}

// 0x00486a10: FogOn::Create (Fog.h). Retail folds it with ToolTip::Create
// (src/reconstructed/GUIManager.cpp, the same body); the build has no FogOn definition, so
// the loader's call is given one here.
GameObject* FogOn::Create(void* target, Fog* fog)
{
    GameObject::SetRenderContext(target);
    this->fog = fog;
    return this;
}

// ---------------------------------------------------------------------------------------
// The local views' calls, forwarded to the canonical classes (Group3_QuarryLoadLinks.h).
// ---------------------------------------------------------------------------------------


QuarryLoadBikeRaceView* QuarryLoadBikeRaceView::Create(void* target, void* particles, LightManager* lights,
                                             TextureContext* textures, OverlayTerrainView* terrain,
                                             QuarryEventKrustyBikeCameraView* camera, Scene* scene, BaseQuarryEvent* event,
                                             void* shadow)
{
    return (QuarryLoadBikeRaceView*)G3Q_BikeRaceCreate(this, target, particles, lights, textures, terrain, camera,
                                                  scene, event, shadow);
}
QuarryLoadKrustyBikeView* QuarryLoadBikeRaceView::NextBike(int* iterator)
{
    return (QuarryLoadKrustyBikeView*)G3Q_BikeRaceNextBike(this, iterator);
}

QuarryLoadKrustyBikeCameraView* QuarryLoadKrustyBikeCameraView::Create(void* target, float a, float b, float c, float d, float e,
                                                         int f, int count, int* presets)
{
    return (QuarryLoadKrustyBikeCameraView*)G3Q_BikeCameraSetupFlyby(this, target, a, b, c, d, e, f, count, presets);
}
void QuarryLoadKrustyBikeCameraView::SetSubject(OverlayTerrainView* terrain) { G3Q_BikeCameraSetSubject(this, terrain); }

void QuarryLoadSoultreeQuadTreeRendererView::InsertNodesInQuadtree() { G3Q_QuadTreeRendererInsertNodes(this); }

void QuarryLoadOverlayHooks::SetStatsSource(QuarryLoadBikeRaceView* race) { G3Q_StatsOverlaySetSource(this, race); }
void QuarryLoadOverlayHooks::CollectChatRacers(QuarryLoadBikeRaceView* race) { G3Q_ChatOverlayCollectRacers(this, race); }
void QuarryLoadOverlayHooks::TakeRacers(QuarryLoadBikeRaceView* race, void* chat) { G3Q_RadarOverlayTakeRacers(this, race, chat); }
void QuarryLoadOverlayHooks::CollectRacers(QuarryLoadBikeRaceView* race) { G3Q_VisualCueCollectRacers(this, race); }

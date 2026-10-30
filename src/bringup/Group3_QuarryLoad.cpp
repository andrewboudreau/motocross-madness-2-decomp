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
//   * the fog objects use Fog.h's Fog/FogOff/FogOn directly; FogOn's folded Create
//     (0x00486a10) is written out;
//   * g_UnknownGlobal59af10 is a static of EcoSystem.cpp: see the best guess below.
// Move into src/reconstructed/QuarryStuntEvent.cpp once it matches.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "../reconstructed/QuarryEvent.h"

#include "../reconstructed/ArcadeObject.h"
#include "../reconstructed/AuralScape.h"
#include "../reconstructed/DebugAlloc.h"
#include "../reconstructed/Display.h"
#include "../reconstructed/EventManager.h"
#include "../reconstructed/LightEmitter.h"
#include "../reconstructed/MemTag.h"
#include "../reconstructed/PCGame.h"
#include "../reconstructed/PCRenderTarget.h"
#include "../reconstructed/SceneManager.h"
#include "../reconstructed/TextureMap.h"
#include "../reconstructed/TextureMapManager.h"
#include "../reconstructed/TrackGame.h"
#include "../reconstructed/UnknownResourceManager.h"
#include "Group3_QuarryLoadLinks.h"

// 0x0059af10: the count of ecosystem textures PreloadEcosystem preloaded, which the memory
// estimate weighs at 0x2aaaa bytes. It is a file static of src/reconstructed/EcoSystem.cpp
// (cleared at 0x00457250/0x0045c040, incremented per preloaded texture) and cannot be read
// from here.
// best guess: 0, so the estimate leaves the preloaded ecosystem textures out; this can only
// choose a finer texture detail level than retail on a card short of video memory.
static int g_UnknownGlobal59af10 = 0;

// 0x0045c040 (cdecl): preloads the ecosystem `path` from `stream`.
int PreloadEcosystem(TextureMapManager* manager, char* path, UnknownTextureStream* stream,
                     int format);

// A CollisionObject (QuadTree object at +0, GameObject at +0xc).
class UnknownQuarryQuadTreeNode {
public:
    virtual void UnknownVirtualSlot0();
    int field_0x04;
    int field_0x08;
};
class UnknownQuarryCollision : public UnknownQuarryQuadTreeNode, public GameObject {
public:
    void SetUseBroadphase(int value);                     // 0x00432120
    void SnapPlacement();                              // 0x00435fe0
    void SetIgnoreListMode(int value);                     // 0x00439400
    void AddIgnoredOwner(UnknownQuarryCollision* other); // 0x00439410: also collides with `other`
    void SetOwner(void* model, int kind);         // 0x004394d0
};

// A physics static model (Scene caster +0x08).
struct UnknownQuarryPhysicsObject {
    unsigned char field_0x000[0x128];
    UnknownQuarryCollision* collisionObject;   // +0x128 SoultreePhysicsBaseObjectCore.h
    unsigned char field_0x12c[0x1f4 - 0x12c];
    UnknownTerrain* terrain;                   // +0x1f4
    void* terrainScale;                        // +0x1f8 copied from the terrain's +0x40
};

// The terrain as the loader reads it.
struct UnknownQuarryTerrain {
    unsigned char field_0x00[0x40];
    void* worldScale;                          // +0x40 SoultreeProbe::worldScale
};

// CarProcedural's terrain pointer (Scene's UnknownSceneAnimatedObject
// leaves +0x20c unnamed).
struct UnknownQuarryCar {
    unsigned char field_0x000[0x20c];
    UnknownTerrain* groundProvider;            // +0x20c CarProcedural.h
};

// A racer as the loader reads it.
struct UnknownQuarryRacerRider {
    unsigned char field_0x000[0x1a0];
    D3DIMSoultreeObject* modelNode;               // +0x1a0
};
struct UnknownQuarryRacerState {
    unsigned char field_0x00[0x38];
    UnknownQuarryCollision* collisionModel;    // +0x38 Wrecker.h
};
struct UnknownQuarryRaceView {
    unsigned char field_0x000[0x18c];
    unsigned char forceHighLod;                // +0x18c BikeRace.h's "ForceHighLOD"
};
struct UnknownQuarryRacer {
    unsigned char field_0x000[0x3bc];
    D3DIMSoultreeObject* modelNode;             // +0x3bc bike model
    unsigned char field_0x3c0[0x5c4 - 0x3c0];
    UnknownQuarryRacerRider* riderCharacter;      // +0x5c4
    unsigned char field_0x5c8[0x5f0 - 0x5c8];
    UnknownQuarryCollision* frontWheel;           // +0x5f0
    UnknownQuarryCollision* rearWheel;            // +0x5f4
    unsigned char field_0x5f8[0x604 - 0x5f8];
    UnknownQuarryRacerState* wrecker;  // +0x604  Bike +0x604 wrecker (its +0x38 collisionModel gets the car's ignore list)
    unsigned char field_0x608[0x735 - 0x608];
    unsigned char isRemote;  // +0x735  KrustyBike +0x735 isRemote
    unsigned char field_0x736[0x740 - 0x736];
    UnknownQuarryRaceView* race;  // +0x740  KrustyBike +0x740 race
    unsigned char field_0x744[0x1414 - 0x744];
    UnknownQuarryCollision* bikeAndRiderCollision;  // +0x1414  KrustyBike +0x1414 bikeAndRiderCollision; the loader adds it as an ignored collision owner
    UnknownQuarryCollision* bikeCollision;  // +0x1418  KrustyBike +0x1418 bikeCollision; the loader adds it as an ignored collision owner
};

// BikeRace (0x400 bytes, constructor 0x00417c30) as the loader uses it.
struct UnknownQuarryRaceCharacter {
    unsigned char field_0x000[0x210];
    UnknownQuarryCollision* collisionObject; // +0x210 krusty2 CollisionCharacter::collisionObject
};
class UnknownQuarryRace : public GameObject {
public:
    explicit UnknownQuarryRace(int flags);                     // 0x00417c30
    // 0x00417ed0: loads the race; returns this, or 0.
    UnknownQuarryRace* Create(void* target, void* particles, LightManager* lights,
                                             UnknownTextureFormatChoice* textures, UnknownTerrain* terrain,
                                             UnknownQuarryCamera* camera, Scene* scene, BaseQuarryEvent* event,
                                             void* shadow);
    UnknownQuarryRacer* NextBike(int* iterator);  // 0x004204e0: the next racer, or 0
    void SetDetailLevel(int level);                     // 0x00423790: detail level

    unsigned char field_0x2c[0x38 - 0x2c];
    UnknownQuarryRacer* localRacer;                     // +0x38 the player's racer
    unsigned char field_0x3c[0x64 - 0x3c];
    UnknownQuarryRaceCharacter* startGate;              // +0x64 (BikeRaceCore.h)
    int startGateEntry;                                 // +0x68
    UnknownQuarryRaceCharacter* thirtySecondMan;        // +0x6c
    int thirtySecondManEntry;                           // +0x70
    unsigned char field_0x74[0x400 - 0x74];
};

// The KrustyBikeCamera at BaseQuarryEvent+0x3c (0x3bc bytes).
class UnknownQuarryBikeCamera : public GameObject {
public:
    explicit UnknownQuarryBikeCamera(int flags);               // 0x00497cb0
    // 0x00497d90: sets the camera up; returns it.
    UnknownQuarryBikeCamera* Create(void* target, float a, float b, float c, float d, float e,
                                                   int f, int count, int* presets);
    void SetSubject(UnknownTerrain* terrain);       // 0x00463600

    unsigned char field_0x02c[0x274 - 0x2c];
    unsigned char isSettled;  // +0x274 FollowCamera isSettled
    unsigned char field_0x275[0x384 - 0x275];
    int targetCharacter;  // +0x384 VehicleCamera targetCharacter
    int targetCar;  // +0x388 VehicleCamera targetCar
    int targetCaster;  // +0x38C VehicleCamera targetCaster
    unsigned char vehicleMode;  // +0x390 VehicleCamera vehicleMode
    unsigned char targetPointCached;  // +0x391 VehicleCamera targetPointCached
    unsigned char targetHeadingCached;  // +0x392 VehicleCamera targetHeadingCached
    unsigned char field_0x393;
    UnknownQuarryRacer* vehicle;  // +0x394 VehicleCamera vehicle
    unsigned char field_0x398[0x3b0 - 0x398];
    UnknownQuarryRacer* bike;  // +0x3B0 BikeCamera bike
    UnknownQuarryRacer* krustyBike;  // +0x3B4 KrustyBikeCamera krustyBike
    unsigned char field_0x3b8[0x3bc - 0x3b8];
};

// ParticleManager (0x2058 bytes); slot 27 loads it.
class UnknownQuarryParticles : public GameObject {
public:
    explicit UnknownQuarryParticles(int flags);                // 0x004ba390
    virtual UnknownQuarryParticles* UnknownVirtualSlot27(void* target, TextureMapManager* manager,
                                                         const char* texture, LightManager* lights);

    unsigned char field_0x2c[0x2058 - 0x2c];
};

// The visibility quadtree (0x8a0 bytes); 0x0052d460 returns its GameObject
// base at +0x874.
class UnknownQuarryQuadTreeHead {
public:
    virtual void UnknownVirtualSlot0();
    unsigned char field_0x004[0x874 - 0x04];
};
class UnknownQuarryVisibility : public UnknownQuarryQuadTreeHead, public GameObject {
public:
    explicit UnknownQuarryVisibility(int flags);               // 0x0052d340
    // 0x0052d460: VisibilityQuadTree::Setup (src/krusty2/visibility).
    // Retail pushes six arguments (ret 0x18); the near miss passed the scene as a seventh.
    GameObject* Setup(void* target, float x0, float z0, float width, float breadth, float minCell);
};

// ProjectedShadow (0x134 bytes) and TerrainShadow (0x6780 bytes).
class UnknownQuarryShadow : public GameObject {
public:
    explicit UnknownQuarryShadow(int flags);                   // 0x004da570
    UnknownQuarryShadow* Init(void* target, TextureMapManager* manager);
    void AddCaster(D3DIMSoultreeObject* model);    // 0x004dab00: casts a shadow
    void SetShadowStrength(int value);                     // 0x004dace0
    void SetShadowGrey(int value);                     // 0x004dae30
    void SetLight(const Vector3* position, const Vector3* direction, int mode); // 0x004dae50 (ProjectedShadow::SetLight)

    unsigned char field_0x2c[0x134 - 0x2c];
};
class UnknownQuarryTerrainShadow : public GameObject {
public:
    explicit UnknownQuarryTerrainShadow(int flags);            // 0x00508ae0
    // 0x00508b80: TerrainShadow::Attach (samples/physics/shadow).
    GameObject* Attach(void* target, UnknownTerrain* terrain, UnknownQuarryShadow* shadow);

    unsigned char field_0x2c[0x6780 - 0x2c];
};

// Fog (0x5c bytes, QuarryEvent.h's Fog) and the objects the sky adds.
class UnknownQuarryFog : public GameObject {
public:
    explicit UnknownQuarryFog(int flags);                      // 0x00462620
    Fog* Create(void* target, unsigned int color, float visibility, float haziness,
                float a, float b, float c);

    unsigned char field_0x2c[0x5c - 0x2c];
};
class UnknownQuarryFogObject : public GameObject {
public:
    explicit UnknownQuarryFogObject(int flags);                // 0x00462e10
};
class UnknownQuarryFogLink : public GameObject {
public:
    explicit UnknownQuarryFogLink(int flags);                  // 0x00462e90
    GameObject* Create(void* target, Fog* fog);  // 0x00486a10

    unsigned char field_0x2c[0x30 - 0x2c];
};
class UnknownQuarrySkyCube : public GameObject {
public:
    explicit UnknownQuarrySkyCube(int flags);                  // 0x004fb1e0
    GameObject* Create(void* target, UnknownTextureStream* stream,
                                      UnknownTextureFormatChoice* textures, float size);

    unsigned char field_0x2c[0x48 - 0x2c];
};

// Terrain (0xcc4 bytes) and EcoSystem (0x5c4 bytes).
class UnknownQuarryTerrainObject : public GameObject {
public:
    explicit UnknownQuarryTerrainObject(int flags);            // 0x00505830
    // 0x005059d0: Terrain::Load (src/krusty2/broadphase/Terrain.h).
    UnknownTerrain* Load(void* target, UnknownTextureStream* stream, int a,
                         UnknownTextureFormatChoice* textures, int level, int b,
                         const char* detailTexture, int c);

    unsigned char field_0x2c[0xcc4 - 0x2c];
};
class UnknownQuarryEcoSystem : public GameObject {
public:
    explicit UnknownQuarryEcoSystem(int flags);                // 0x00457250
    EcoSystem* Create(void* target, TextureMapManager* manager, UnknownTerrain* terrain,
                                     LightManager* lights, UnknownSceneEnvironment* environment,
                                     UnknownTextureStream* stream, int format, int a, int level);

    unsigned char field_0x2c[0x5c4 - 0x2c];
};

// The last race object (0x48 bytes).
class UnknownQuarryRaceObject : public GameObject {
public:
    explicit UnknownQuarryRaceObject(int flags);               // 0x00504490
    void InsertNodesInQuadtree();                              // 0x005046e0

    unsigned char field_0x2c[0x48 - 0x2c];
};

// The overlays' race hooks.
struct UnknownQuarryOverlayHooks {
    void SetStatsSource(UnknownQuarryRace* race);       // 0x00519880 (StatsOverlay)
    // 0x0051d730 is ChatOverlay::CollectRacers; VisualCue's CollectRacers
    // below has the same view signature, so this one keeps its address name.
    void UnknownFunction51d730(UnknownQuarryRace* race);       // 0x0051d730 (ChatOverlay)
    void TakeRacers(UnknownQuarryRace* race, void* chat); // 0x0051baf0 (RadarOverlay::TakeRacers)
    void CollectRacers(UnknownQuarryRace* race);       // 0x0048af20 (VisualCue)
};

// The bytes the render target's colour buffers take.
static inline int FrameBufferBytes(RenderTarget* target) {
    return PixelFormatBytesPerPixel(target->surfacePixelFormat) * (target->frameModulus + 1) * target->targetHeight *
           target->targetWidth;
}

// 0x004de590
int BaseQuarryEvent::LoadEventObjects(UnknownProgressCallback progress) {
    int terrainBytes;
    int textureBytes;
    int forced;
    int needed;
    int sceneSteps;
    float terrainWidth;
    int hasCube;
    unsigned long modelCounts[4];
    int presets[8];
    UnknownTextureFormatChoice terrainTextures;
    UnknownTextureFormatChoice modelTextures;
    UnknownTextureFormatChoice skyTextures;
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
    UnknownTextureStream* stream = new(__FILE__, 252) UnknownTextureStream(g_ResourceManager);
    UnknownTextureStream* preloaded = stream;
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
        if (((UnknownDisplay*)g_TrackGame->display)->isAGP) {
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
    raceCamera = (UnknownQuarryCamera*)(((UnknownQuarryBikeCamera*)G3Q_NewBikeCamera(__FILE__, 585, 1)))
                     ->Create(renderContext, 0.55f, 0.3f, 27.0f, 7.0f, 1.048f, 2, 8, presets);
    if (!AppendChild((GameObject*)raceCamera, -1)) {
        sprintf(text, "KrustyBikeCamera not created.\n");
        return 0;
    }
    if (progress)
        progress(0);
    CreateOverlays();

    g_MemTagStack->Push("Particles");
    particleManager = ((UnknownQuarryParticles*)G3Q_NewParticleManager(__FILE__, 613, g_TrackGame->mode.particlesOn));
    g_MemTagStack->Push("Scene");
    eventScene = (new(__FILE__, 624) Scene(isEnabled))
                     ->UnknownFunction4ea7e0(g_TrackGame->sceneObject, (int)auralScape, (int)&modelTextures, 8);
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
    UnknownQuarryVisibility* visibility = (UnknownQuarryVisibility*)(((UnknownQuarryVisibility*)G3Q_NewVisibilityQuadTree(__FILE__, 659, 1)))
        ->Setup(renderContext, 0.0f, 0.0f,
                                eventScene->environment->terrainWidth * eventScene->environment->terrainWidthScale * 256.0f,
                                eventScene->environment->terrainBreadth * eventScene->environment->terrainBreadthScale * 256.0f,
                                96.0f);
    if (!visibility)
        sprintf(text, "VisibilityQuadTree not created.\n");
    g_MemTagStack->Pop(0);
    UnknownSceneLight* shadowLight = 0;
    UnknownSceneLight* flareLight = 0;
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
        projectedShadow = ((UnknownQuarryShadow*)G3Q_NewProjectedShadow(__FILE__, 698, g_TrackGame->mode.shadowsOn));
        projectedShadow = ((UnknownQuarryShadow*)projectedShadow)->Init(renderContext, g_TrackGame->textureManager);
    }
    if (!preloaded)
        stream = new(__FILE__, 713) UnknownTextureStream(g_ResourceManager);

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
        skyCube = (((UnknownQuarrySkyCube*)G3Q_NewSkyCube(__FILE__, 744, g_TrackGame->mode.skyCubeOn)))
                         ->Create(renderContext, stream, &skyTextures, 1024.0f);
        if (!AppendChild((GameObject*)skyCube, -1))
            sprintf(text, "SkyCube not created.\n");
        // 0x00486a10 (FogOn's Create, a folded body): GameObject::SetRenderContext called
        // directly (0x004692f0), then the fog at +0x2c; returns the object.
        FogOn* fogLink = new(__FILE__, 750) FogOn(1);
        if (fogLink) {
            fogLink->GameObject::SetRenderContext(renderContext);
            fogLink->fog = eventFog;
        }
        AppendChild(fogLink, -1);
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
    eventTerrain = (((UnknownQuarryTerrainObject*)G3Q_NewTerrain(__FILE__, 799, 1)))
                     ->Load(renderContext, terrainStream, 1, &terrainTextures,
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
        terrainShadow = (((UnknownQuarryTerrainShadow*)G3Q_NewTerrainShadow(__FILE__, 822, g_TrackGame->mode.shadowsOn)))
                         ->Attach(renderContext, eventTerrain, (UnknownQuarryShadow*)projectedShadow);
        AppendChild((GameObject*)terrainShadow, -1);
    }
    AppendChild(visibility, -1);

    g_MemTagStack->Push("EcoSystem");
    if (!stream)
        stream = new(__FILE__, 848) UnknownTextureStream(g_ResourceManager);
    if (strstr(eventScene->environment->ecosystemFile, ".est"))
        strcpy(strstr(eventScene->environment->ecosystemFile, ".est"), ".esb");
    if (stream && !g_TrackGame->sceneObject->OpenArchiveFile(stream, eventScene->environment->ecosystemFile,
                                                                           "rb", 0)) {
        sprintf(text, "No Ecosystem found in env file.  Ecosystem not created.\n");
    } else {
        ecoSystem = (((UnknownQuarryEcoSystem*)G3Q_NewEcoSystem(__FILE__, 870, 1)))
                         ->Create(renderContext, g_TrackGame->textureManager, eventTerrain, lightManager,
                                                 eventScene->environment, stream, 0x115c,
                                                 g_TrackGame->mode.raceSettings.treeCollision,
                                                 g_TrackGame->mode.videoSettings[1]);
        if (!ecoSystem)
            sprintf(text, "EcoSystem not created.\n");
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("3DObjects");
    if (!UnknownVirtualSlot35())
        return 0;
    g_MemTagStack->Push("BikeRace");
    raceView = (UnknownKrustyBikeView*)((UnknownQuarryRace*)G3Q_NewBikeRace(__FILE__, 898, 0));
    g_MemTagStack->Push("Particles");
    if (particleManager)
        particleManager = ((UnknownQuarryParticles*)particleManager)
                         ->UnknownVirtualSlot27(renderContext, g_TrackGame->textureManager,
                                                eventScene->environment->particleTexture, lightManager);
    g_MemTagStack->Push("3DObjects");
    if (!UnknownVirtualSlot27((int)&modelTextures))
        return 0;
    g_TrackGame->mode.networkGame = eventScene->crowdPresent;
    g_MemTagStack->Push("BikeRace");
    raceView = (UnknownKrustyBikeView*)((UnknownQuarryRace*)raceView)
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
            UnknownSceneCaster* caster = &eventScene->staticModels->casters[i];
            if (caster->physics) {
                UnknownTerrain* terrain = eventTerrain;
                UnknownQuarryPhysicsObject* physics = (UnknownQuarryPhysicsObject*)caster->physicsObject;
                physics->terrain = terrain;
                if (terrain)
                    physics->terrainScale = ((UnknownQuarryTerrain*)terrain)->worldScale;
                ((UnknownQuarryPhysicsObject*)caster->physicsObject)->collisionObject->SnapPlacement();
                ((UnknownQuarryPhysicsObject*)caster->physicsObject)->collisionObject->SetUseBroadphase(1);
                ((UnknownQuarryPhysicsObject*)caster->physicsObject)->collisionObject->SetOwner(caster->physicsObject, 0x68);
            } else if (caster->collisionObject && ((D3DIMSoultreeObject*)caster->model)->CountNodes() == 1) {
                ((UnknownQuarryCollision*)caster->collisionObject)->SnapPlacement();
                ((UnknownQuarryCollision*)caster->collisionObject)->SetUseBroadphase(1);
            }
        }
    }
    if (((UnknownQuarryRace*)raceView)->thirtySecondMan && ((UnknownQuarryRace*)raceView)->thirtySecondMan->collisionObject) {
        ((UnknownQuarryRace*)raceView)->thirtySecondMan->collisionObject->Release();
        ((UnknownQuarryRace*)raceView)->thirtySecondMan->collisionObject = 0;
    }
    if (((UnknownQuarryRace*)raceView)->startGate && ((UnknownQuarryRace*)raceView)->startGate->collisionObject) {
        ((UnknownQuarryRace*)raceView)->startGate->collisionObject->Release();
        ((UnknownQuarryRace*)raceView)->startGate->collisionObject = 0;
    }
    if (eventScene->animationTable) {
        for (int i = 0; i < eventScene->animationTable->entryCount; i++) {
            if (eventScene->animationTable->entries[i].keyFramed) {
                UnknownSceneObject* character = eventScene->animationTable->entries[i].character;
                if (character->collisionObject) {
                    ((UnknownQuarryCollision*)character->collisionObject)->SnapPlacement();
                    ((UnknownQuarryCollision*)character->collisionObject)->SetUseBroadphase(1);
                    ((UnknownQuarryCollision*)character->collisionObject)->SetIgnoreListMode(0);
                    if (i != ((UnknownQuarryRace*)raceView)->thirtySecondManEntry && i != ((UnknownQuarryRace*)raceView)->startGateEntry) {
                        int iterator = 0;
                        for (UnknownQuarryRacer* racer = ((UnknownQuarryRace*)raceView)->NextBike(&iterator);
                             racer; racer = ((UnknownQuarryRace*)raceView)->NextBike(&iterator)) {
                            if (!racer->isRemote) {
                                ((UnknownQuarryCollision*)character->collisionObject)->AddIgnoredOwner(racer->bikeCollision);
                                ((UnknownQuarryCollision*)character->collisionObject)->AddIgnoredOwner(racer->bikeAndRiderCollision);
                                ((UnknownQuarryCollision*)character->collisionObject)->AddIgnoredOwner(racer->frontWheel);
                                ((UnknownQuarryCollision*)character->collisionObject)->AddIgnoredOwner(racer->rearWheel);
                                ((UnknownQuarryCollision*)character->collisionObject)
                                    ->AddIgnoredOwner(racer->wrecker->collisionModel);
                            }
                        }
                    } else if (character->collisionObject) {
                        ((UnknownQuarryCollision*)character->collisionObject)->Release();
                        character->collisionObject = 0;
                    }
                }
            } else {
                UnknownSceneAnimatedObject* car = eventScene->animationTable->entries[i].proceduralObject;
                ((UnknownQuarryCar*)car)->groundProvider = eventTerrain;
                ((UnknownQuarryCollision*)car->sensorCollider)->SetIgnoreListMode(1);
                for (int j = 0; j < eventScene->animationTable->entryCount; j++) {
                    UnknownSceneEntry* entry = &eventScene->animationTable->entries[j];
                    if (entry->keyFramed)
                        ((UnknownQuarryCollision*)car->sensorCollider)
                            ->AddIgnoredOwner((UnknownQuarryCollision*)entry->character->collisionObject);
                    else
                        ((UnknownQuarryCollision*)car->sensorCollider)
                            ->AddIgnoredOwner((UnknownQuarryCollision*)entry->proceduralObject->bodyCollider);
                }
                ((UnknownQuarryCollision*)car->bodyCollider)->SnapPlacement();
                ((UnknownQuarryCollision*)car->bodyCollider)->SetUseBroadphase(1);
                ((UnknownQuarryCollision*)car->bodyCollider)->SetOwner(car, 0x69);
                ((UnknownQuarryCollision*)car->bodyCollider)->SetIgnoreListMode(0);
                int iterator = 0;
                for (UnknownQuarryRacer* racer = ((UnknownQuarryRace*)raceView)->NextBike(&iterator); racer;
                     racer = ((UnknownQuarryRace*)raceView)->NextBike(&iterator)) {
                    if (!racer->isRemote) {
                        ((UnknownQuarryCollision*)car->bodyCollider)->AddIgnoredOwner(racer->bikeCollision);
                        ((UnknownQuarryCollision*)car->bodyCollider)->AddIgnoredOwner(racer->bikeAndRiderCollision);
                        ((UnknownQuarryCollision*)car->bodyCollider)->AddIgnoredOwner(racer->frontWheel);
                        ((UnknownQuarryCollision*)car->bodyCollider)->AddIgnoredOwner(racer->rearWheel);
                        ((UnknownQuarryCollision*)car->bodyCollider)->AddIgnoredOwner(racer->wrecker->collisionModel);
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
    UnknownVirtualSlot32((int)raceView);
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
        auralScape->groundSubject = (UnknownFollowCameraSubject*)eventTerrain;
        AppendChild(auralScape, -1);
    }

    g_MemTagStack->Push("Shadow");
    if (projectedShadow) {
        AppendChild((GameObject*)projectedShadow, -1);
        ((UnknownQuarryShadow*)projectedShadow)->AddCaster(((UnknownQuarryRace*)raceView)->localRacer->modelNode);
        ((UnknownQuarryShadow*)projectedShadow)
            ->AddCaster(((UnknownQuarryRace*)raceView)->localRacer->riderCharacter->modelNode);
        ((UnknownQuarryShadow*)projectedShadow)->SetLight(&shadowLight->position, &shadowLight->look, 3);
        ((UnknownQuarryShadow*)projectedShadow)->SetShadowStrength(0x80);
        int ambient = (int)(ambientLight->ambientRGBA[1] * 255.0f);
        if (!ambientLight)
            ambientLight = lightManager->FindLightOfType(6);
        if (ambientLight)
            ambient = (int)(ambientLight->ambientRGBA[1] * 255.0f);
        ((UnknownQuarryShadow*)projectedShadow)->SetShadowGrey(ambient);
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("Collision");
    if (!UnknownVirtualSlot31())
        return 0;
    g_MemTagStack->Push("3DObjects");
    UnknownVirtualSlot33((int)((UnknownQuarryRace*)raceView)->localRacer);
    g_MemTagStack->Push("BikeRace");
    ((UnknownQuarryBikeCamera*)raceCamera)->SetSubject(eventTerrain);
    UnknownQuarryBikeCamera* camera = (UnknownQuarryBikeCamera*)raceCamera;
    UnknownQuarryRacer* bike = ((UnknownQuarryRace*)raceView)->localRacer;
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
        ((UnknownQuarryOverlayHooks*)statsOverlay)->SetStatsSource((UnknownQuarryRace*)raceView);
    if (chatOverlay)
        ((UnknownQuarryOverlayHooks*)chatOverlay)->UnknownFunction51d730((UnknownQuarryRace*)raceView);
    if (radarOverlay)
        ((UnknownQuarryOverlayHooks*)radarOverlay)->TakeRacers((UnknownQuarryRace*)raceView, chatOverlay);
    if (visualCue)
        ((UnknownQuarryOverlayHooks*)visualCue)->CollectRacers((UnknownQuarryRace*)raceView);
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
    UnknownQuarryRaceObject* last = ((UnknownQuarryRaceObject*)G3Q_NewQuadTreeRenderer(__FILE__, 1251, 1));
    last->SetRenderContext(renderContext);
    AppendChild(last, -1);
    last->InsertNodesInQuadtree();
    return 1;
}

// ---------------------------------------------------------------------------------------
// The local views' calls, forwarded to the canonical classes (Group3_QuarryLoadLinks.h).
// ---------------------------------------------------------------------------------------

void UnknownQuarryCollision::SetUseBroadphase(int value) { G3Q_CollisionSetUseBroadphase(this, value); }
void UnknownQuarryCollision::SnapPlacement() { G3Q_CollisionSnapPlacement(this); }
void UnknownQuarryCollision::SetIgnoreListMode(int value) { G3Q_CollisionSetIgnoreListMode(this, value); }
void UnknownQuarryCollision::AddIgnoredOwner(UnknownQuarryCollision* other) { G3Q_CollisionAddIgnoredOwner(this, other); }
void UnknownQuarryCollision::SetOwner(void* model, int kind) { G3Q_CollisionSetOwner(this, model, kind); }

UnknownQuarryRace* UnknownQuarryRace::Create(void* target, void* particles, LightManager* lights,
                                             UnknownTextureFormatChoice* textures, UnknownTerrain* terrain,
                                             UnknownQuarryCamera* camera, Scene* scene, BaseQuarryEvent* event,
                                             void* shadow)
{
    return (UnknownQuarryRace*)G3Q_BikeRaceCreate(this, target, particles, lights, textures, terrain, camera,
                                                  scene, event, shadow);
}
UnknownQuarryRacer* UnknownQuarryRace::NextBike(int* iterator)
{
    return (UnknownQuarryRacer*)G3Q_BikeRaceNextBike(this, iterator);
}

UnknownQuarryBikeCamera* UnknownQuarryBikeCamera::Create(void* target, float a, float b, float c, float d, float e,
                                                         int f, int count, int* presets)
{
    return (UnknownQuarryBikeCamera*)G3Q_BikeCameraSetupFlyby(this, target, a, b, c, d, e, f, count, presets);
}
void UnknownQuarryBikeCamera::SetSubject(UnknownTerrain* terrain) { G3Q_BikeCameraSetSubject(this, terrain); }

GameObject* UnknownQuarryVisibility::Setup(void* target, float x0, float z0, float width, float breadth, float minCell)
{
    return (GameObject*)G3Q_VisibilitySetup(this, target, x0, z0, width, breadth, minCell);
}

UnknownQuarryShadow* UnknownQuarryShadow::Init(void* target, TextureMapManager* manager)
{
    return (UnknownQuarryShadow*)G3Q_ProjectedShadowInit(this, target, manager);
}
void UnknownQuarryShadow::AddCaster(D3DIMSoultreeObject* model) { G3Q_ProjectedShadowAddCaster(this, model); }
void UnknownQuarryShadow::SetShadowStrength(int value) { G3Q_ProjectedShadowSetStrength(this, value); }
void UnknownQuarryShadow::SetShadowGrey(int value) { G3Q_ProjectedShadowSetGrey(this, value); }
void UnknownQuarryShadow::SetLight(const Vector3* position, const Vector3* direction, int mode)
{
    G3Q_ProjectedShadowSetLight(this, position, direction, mode);
}

GameObject* UnknownQuarryTerrainShadow::Attach(void* target, UnknownTerrain* terrain, UnknownQuarryShadow* shadow)
{
    return (GameObject*)G3Q_TerrainShadowAttach(this, target, terrain, shadow);
}

GameObject* UnknownQuarrySkyCube::Create(void* target, UnknownTextureStream* stream,
                                         UnknownTextureFormatChoice* textures, float size)
{
    return (GameObject*)G3Q_SkyCubeCreate(this, target, stream, textures, size);
}

UnknownTerrain* UnknownQuarryTerrainObject::Load(void* target, UnknownTextureStream* stream, int a,
                                                 UnknownTextureFormatChoice* textures, int level, int b,
                                                 const char* detailTexture, int c)
{
    return (UnknownTerrain*)G3Q_TerrainLoad(this, target, stream, a, textures, level, b, detailTexture, c);
}

EcoSystem* UnknownQuarryEcoSystem::Create(void* target, TextureMapManager* manager, UnknownTerrain* terrain,
                                          LightManager* lights, UnknownSceneEnvironment* environment,
                                          UnknownTextureStream* stream, int format, int a, int level)
{
    return (EcoSystem*)G3Q_EcoSystemCreate(this, target, manager, terrain, lights, environment, stream, format,
                                           a, level);
}

void UnknownQuarryRaceObject::InsertNodesInQuadtree() { G3Q_QuadTreeRendererInsertNodes(this); }

void UnknownQuarryOverlayHooks::SetStatsSource(UnknownQuarryRace* race) { G3Q_StatsOverlaySetSource(this, race); }
void UnknownQuarryOverlayHooks::UnknownFunction51d730(UnknownQuarryRace* race) { G3Q_ChatOverlayCollectRacers(this, race); }
void UnknownQuarryOverlayHooks::TakeRacers(UnknownQuarryRace* race, void* chat) { G3Q_RadarOverlayTakeRacers(this, race, chat); }
void UnknownQuarryOverlayHooks::CollectRacers(UnknownQuarryRace* race) { G3Q_VisualCueCollectRacers(this, race); }

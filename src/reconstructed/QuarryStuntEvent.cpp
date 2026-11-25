#include "BikeRaceCore.h"
#include "../krusty2/broadphase/Terrain.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "QuarryEvent.h"

#include "AuralScape.h"
#include "CarProcedural.h"
#include "ControlInterface.h"
#include "DebugAlloc.h"
#include "DebugOverlay.h"
#include "LightEmitter.h"
#include "MemTag.h"
#include "RaceView.h"
#include "RenderTarget.h"
#include "SceneManager.h"
#include "SelectGamePicProcs.h"
#include "TextureMap.h"
#include "TrackGame.h"
#include "EventManager.h"
#include "TrackOverlay.h"
#include "Krusty3DObjects.h"
#include "ResourceEntry.h"
#include "ArcadeObject.h"
#include "FollowCamera.h"
#include "KrustyBikeCamera.h"
#include "PCGame.h"
#include "PCRenderTarget.h"
#include "PCVideoCard.h"
#include "SkyCube.h"
#include "TerrainShadow.h"
#include "TextureMapManager.h"
#include "../krusty2/collision/CollisionObject.h"
#include "../krusty2/motion/CollisionCharacter.h"
#include "../krusty2/effects/ParticleManager.h"
#include "../krusty2/shadow/ProjectedShadow.h"
#include "../krusty2/soultree/SoultreeQuadTreeRenderer.h"
#include "../krusty2/visibility/VisibilityQuadTree.h"

// Reconstruction of D:\aardvark\VC\krusty2\QuarryStuntEvent.cpp (literal
// __FILE__ at 0x00572154): BaseQuarryEvent, 0x004de2a0..0x004e1fa7, between
// Quantize.cpp (ends 0x004de29f) and racesnd.cpp (RaceSound's constructor
// 0x004e1fb0). Names are provisional. The loader 0x004de590 and slot 12
// (0x004e14a0) are not exact.

#define SoundSystem() ((PCSoundInterface*)g_TrackGame->soundInterface)

// The debug page and its selected row (0x00572150, -1; 0x00689bbc), shared by
// slots 12 and 23.
static int s_DebugPage = -1;

static int s_DebugRow;

// 0x004de2a0
BaseQuarryEvent::BaseQuarryEvent(int flags) : GameObject(flags) {
    eventScene = 0;
    raceCamera = 0;
    raceView = 0;
    skyCube = 0;
    eventTerrain = 0;
    ecoSystem = 0;
    field_0x4c = 0;
    loaded = 0;
    terrainShadow = 0;
    projectedShadow = 0;
    clockMinutes = 0;
    clockSeconds = 0;
    particleManager = 0;
    lightManager = 0;
    visualCue = 0;
    opaqueTextureGroup = 0;
    alphaTextureGroup = 0;
    auralScape = 0;
    auralScapeListener = 0;
    radarOverlay = 0;
    statsOverlay = 0;
    chatOverlay = 0;
    instrumentOverlay = 0;
    textQueue = 0;
    eventFog = 0;
    terrainStream = new(__FILE__, 92) FileStream(g_ResourceManager);
    if (!terrainStream)
        return;
}

// 0x004de3b0
BaseQuarryEvent* BaseQuarryEvent::Create(RenderTarget* target, LoadProgressFn progress) {
    GameObject::SetRenderContext(target);
    g_MemTagStack->ReportTotals("entering BaseQuarryEvent::Create");
    if (!LoadEventObjects(progress)) {
        Release();
        return 0;
    }
    g_MemTagStack->ReportTotals("exiting BaseQuarryEvent::Create");
    return this;
}

// 0x004de400
int BaseQuarryEvent::Render3D() {
    return GameObject::Render3D();
}

// 0x004de410
int BaseQuarryEvent::Tick(float frameTime) {
    GameObject::Tick(frameTime);
    if (auralScape)
        auralScape->SetListener(auralScapeListener, &raceCamera->eyePosition, &raceCamera->viewDirection,
                                          &raceCamera->upDirection, &raceCamera->followedBike->velocity);
    if (g_TrackGame->uiInteractionBlocked)
        return 1;
    if (radarOverlay && visualCue && statsOverlay) {
        visualCue->skippedRacer = radarOverlay->followedIndex;
        int index = visualCue->NextCueIndex();
        if (index != -1) {
            radarOverlay->cueIndex = index;
            statsOverlay->SetCueIndex(index);
        }
        radarOverlay->cueShown = visualCue->CanCycleCues();
        chatOverlay->ShowCueTag(visualCue->CanCycleCues());
    }
    if (!g_TrackGame->eventManager->waitingForRacers && !raceView->networkStartPending && raceView->isRacing) {
        clockSeconds += g_TrackGame->frameTime;
        while (clockSeconds >= 60.0f) {
            clockMinutes += 1.0f;
            clockSeconds -= 60.0f;
            if (clockSeconds < 0.0f)
                clockSeconds = 0.0f;
        }
    } else if (raceView->networkStartPending) {
        ResetClock();
    }
    SoundSystem()->CommitListenerSettings();
    return 1;
}

// 0x004de580
void BaseQuarryEvent::UnknownVirtualSlot34(int a, int b, int c) {
}

// The loader (0x004de590): its globals and the views of the objects it reads.
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

// The KrustyBikeCamera members the loader writes (FollowCamera's +0x274,
// VehicleCamera's +0x384..+0x394, BikeCamera's +0x3b0 and KrustyBikeCamera's
// +0x3b4, which those headers keep protected).
struct QuarryLoadKrustyBikeCameraView {
    unsigned char field_0x000[0x274];
    unsigned char isSettled;                      // +0x274 FollowCamera isSettled
    unsigned char field_0x275[0x384 - 0x275];
    void* targetCharacter;                        // +0x384 VehicleCamera targetCharacter
    void* targetCar;                              // +0x388 VehicleCamera targetCar
    void* targetCaster;                           // +0x38c VehicleCamera targetCaster
    unsigned char vehicleMode;                    // +0x390 VehicleCamera vehicleMode
    unsigned char targetPointCached;              // +0x391 VehicleCamera targetPointCached
    unsigned char targetHeadingCached;            // +0x392 VehicleCamera targetHeadingCached
    QuarryLoadKrustyBikeView* vehicle;            // +0x394 VehicleCamera vehicle
    unsigned char field_0x398[0x3b0 - 0x398];
    QuarryLoadKrustyBikeView* bike;               // +0x3b0 BikeCamera bike
    QuarryLoadKrustyBikeView* krustyBike;         // +0x3b4 KrustyBikeCamera krustyBike

    // Inline: the same sequence as BikeRace.h's StartFollowing (three times
    // in 0x0041eb20). Follows `racer`, showing the previous racer's model
    // again and hiding the new one's.
    void StartFollowing(QuarryLoadKrustyBikeView* racer) {
        if (bike && !racer->race->forceHighLod) {
            bike->modelNode->SetLowestLod(1);
            bike->riderCharacter->modelNode->SetLowestLod(1);
            if (!bike->modelNode->currentLod)
                bike->modelNode->SetLod(1);
            if (!bike->riderCharacter->modelNode->currentLod)
                bike->riderCharacter->modelNode->SetLod(1);
        }
        vehicle = racer;
        targetPointCached = 0;
        targetHeadingCached = 0;
        vehicleMode = 1;
        targetCaster = 0;
        targetCharacter = 0;
        targetCar = 0;
        bike = racer;
        isSettled = 0;
        krustyBike = racer;
        if (!racer->race->forceHighLod) {
            racer->modelNode->SetLowestLod(0);
            bike->riderCharacter->modelNode->SetLowestLod(0);
        }
        isSettled = 0;
    }
};

// The bytes the render target's colour buffers take.
static inline int FrameBufferBytes(RenderTarget* target) {
    return PixelFormatBytesPerPixel(target->surfacePixelFormat) * (target->frameModulus + 1) * target->targetHeight *
           target->targetWidth;
}

// 0x004de590. Not exact (2206 instructions against retail's 2208, linked
// score 0.986; all scheduling: the small slots below 0x1c, register choices
// in the memory-budget tree and around the fog/sky block, the collision
// loop's head (retail enters a rotated loop with a jmp and indexes the entry
// table with [ecx + edx + 4]) and the racer loop's esi/edi swap;
// docs/QUARRYSTUNTEVENT.md).
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
                     ->SetupFlyby(renderContext, 0.55f, 0.3f, 27.0f, 7.0f, 1.048f, 2, 8, presets);
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
    raceView = (EventBikeRaceView*)((BikeRace*)raceView)
                     ->Create(renderContext, particleManager, lightManager, &modelTextures,
                              (Terrain*)eventTerrain, (KrustyBikeCamera*)raceCamera, eventScene,
                              (RaceGateEvent*)this, (ProjectedShadow*)projectedShadow);
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
    if (((BikeRace*)raceView)->thirtySecondMan && ((BikeRace*)raceView)->thirtySecondMan->collisionObject) {
        ((BikeRace*)raceView)->thirtySecondMan->collisionObject->Release();
        ((BikeRace*)raceView)->thirtySecondMan->collisionObject = 0;
    }
    if (((BikeRace*)raceView)->startGate && ((BikeRace*)raceView)->startGate->collisionObject) {
        ((BikeRace*)raceView)->startGate->collisionObject->Release();
        ((BikeRace*)raceView)->startGate->collisionObject = 0;
    }
    if (eventScene->animationTable) {
        for (int i = 0; i < eventScene->animationTable->entryCount; i++) {
            if (eventScene->animationTable->entries[i].keyFramed) {
                CollisionCharacter* character = eventScene->animationTable->entries[i].character;
                if (character->collisionObject) {
                    character->collisionObject->SnapPlacement();
                    character->collisionObject->SetUseBroadphase(1);
                    character->collisionObject->SetIgnoreListMode(0);
                    if (i != ((BikeRace*)raceView)->thirtySecondManEntry && i != ((BikeRace*)raceView)->startGateEntry) {
                        int iterator = 0;
                        for (QuarryLoadKrustyBikeView* racer = (QuarryLoadKrustyBikeView*)((BikeRace*)raceView)->NextBike(&iterator);
                             racer; racer = (QuarryLoadKrustyBikeView*)((BikeRace*)raceView)->NextBike(&iterator)) {
                            if (!racer->isRemote) {
                                character->collisionObject->AddIgnoredOwner(racer->bikeCollision);
                                character->collisionObject->AddIgnoredOwner(racer->bikeAndRiderCollision);
                                character->collisionObject->AddIgnoredOwner(racer->frontWheel);
                                character->collisionObject->AddIgnoredOwner(racer->rearWheel);
                                character->collisionObject
                                    ->AddIgnoredOwner(racer->wrecker->collisionModel);
                            }
                        }
                    } else if (character->collisionObject) {
                        character->collisionObject->Release();
                        character->collisionObject = 0;
                    }
                }
            } else {
                CarProcedural* car = eventScene->animationTable->entries[i].proceduralObject;
                car->groundProvider = eventTerrain;
                car->sensorCollider->SetIgnoreListMode(1);
                for (int j = 0; j < eventScene->animationTable->entryCount; j++) {
                    SceneAnimation* entry = &eventScene->animationTable->entries[j];
                    if (entry->keyFramed)
                        car->sensorCollider
                            ->AddIgnoredOwner(entry->character->collisionObject);
                    else
                        car->sensorCollider
                            ->AddIgnoredOwner(entry->proceduralObject->bodyCollider);
                }
                car->bodyCollider->SnapPlacement();
                car->bodyCollider->SetUseBroadphase(1);
                car->bodyCollider->SetOwner(car, 0x69);
                car->bodyCollider->SetIgnoreListMode(0);
                int iterator = 0;
                for (QuarryLoadKrustyBikeView* racer = (QuarryLoadKrustyBikeView*)((BikeRace*)raceView)->NextBike(&iterator); racer;
                     racer = (QuarryLoadKrustyBikeView*)((BikeRace*)raceView)->NextBike(&iterator)) {
                    if (!racer->isRemote) {
                        car->bodyCollider->AddIgnoredOwner(racer->bikeCollision);
                        car->bodyCollider->AddIgnoredOwner(racer->bikeAndRiderCollision);
                        car->bodyCollider->AddIgnoredOwner(racer->frontWheel);
                        car->bodyCollider->AddIgnoredOwner(racer->rearWheel);
                        car->bodyCollider->AddIgnoredOwner(racer->wrecker->collisionModel);
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
        ((ProjectedShadow*)projectedShadow)->AddCaster((ShadowCaster*)((QuarryLoadKrustyBikeView*)((BikeRace*)raceView)->localRacer)->modelNode);
        ((ProjectedShadow*)projectedShadow)
            ->AddCaster((ShadowCaster*)((QuarryLoadKrustyBikeView*)((BikeRace*)raceView)->localRacer)->riderCharacter->modelNode);
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
    CreateRacer3DObjects((int)((BikeRace*)raceView)->localRacer);
    g_MemTagStack->Push("BikeRace");
    ((FollowCamera*)(KrustyBikeCamera*)raceCamera)->SetSubject((FollowCameraTerrainView*)eventTerrain);
    ((QuarryLoadKrustyBikeCameraView*)raceCamera)->StartFollowing((QuarryLoadKrustyBikeView*)((BikeRace*)raceView)->localRacer);
    if (statsOverlay)
        statsOverlay->SetStatsSource((int)raceView);
    if (chatOverlay)
        chatOverlay->CollectRacers((ChatBikeRaceView*)raceView);
    if (radarOverlay)
        radarOverlay->TakeRacers(raceView, (int)chatOverlay);
    if (visualCue)
        visualCue->CollectRacers((VisualCueBikeRaceView*)raceView);
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
    ((BikeRace*)raceView)->SetDetailLevel(g_TrackGame->mode.videoSettings[2]);
    if (progress)
        progress(0);
    SoultreeQuadTreeRenderer* last = new(__FILE__, 1251) SoultreeQuadTreeRenderer(1);
    last->SetRenderContext(renderContext);
    AppendChild(last, -1);
    last->InsertNodesInQuadtree();
    return 1;
}

// 0x004e04a0
int BaseQuarryEvent::EstimateTerrainBytes(int width) {
    int size;
    if (width == 3)
        size = 35;
    else
        size = width == 5 ? 55 : 106;
    return size * 0x80000 / 3;
}

// 0x004e04e0
int BaseQuarryEvent::EstimateCubeBytes() {
    return 0x200000;
}

// 0x004e04f0
int BaseQuarryEvent::EstimateModelTextureBytes(int* counts) {
    return counts[0] * 0x80000 / 3 + counts[1] * 0x20000 / 3 + counts[2] * 0x8000 / 3 + counts[3] * 0x2000 / 3;
}

// 0x004e0560: the race memory estimate (counts of eligible bikes and
// opponents, capped by the circuit's +0x460). The sum is a local and the
// bajaEnduroEvent constant an if/else assignment: VC6 still emits the branchless
// neg/sbb/and/add last, and with the ternary inside the return it keeps
// g_TrackGame in a register across the first loop and spills a count (22/331).
int BaseQuarryEvent::EstimateRaceBytes() {
    int players = 0;
    int opponents = 0;
    if (g_TrackGame->InProCircuitCareer()) {
        int i;
        for (i = 0; i < g_TrackGame->ui->bikeCount; i++) {
            if (g_TrackGame->ui->manufacturers[g_TrackGame->ui->bikes[i].manufacturer].availability <= g_TrackGame->proCircuit->maxAvailability)
                players++;
        }
        for (i = 0; i < g_TrackGame->ui->riderCount; i++) {
            if (g_TrackGame->ui->riders[i].availability <= g_TrackGame->proCircuit->maxAvailability)
                opponents++;
        }
        if (players >= g_TrackGame->proCircuit->racerCount)
            players = g_TrackGame->proCircuit->racerCount;
        if (opponents >= g_TrackGame->proCircuit->racerCount)
            opponents = g_TrackGame->proCircuit->racerCount;
    } else {
        if (g_TrackGame->playerCount == 1) {
            if (g_TrackGame->mode.raceSettings.raceMode == 4)
                opponents = 1;
            else
                opponents = g_TrackGame->mode.raceSettings.aiRacerCount + 1;
        } else {
            opponents = g_TrackGame->mode.raceSettings.multiplayerAiRacerCount + g_TrackGame->playerCount;
        }
        players = opponents;
    }
    int total = opponents * 0x80000 / 3 + opponents * 0x8000 / 3 + players * 0x80000 / 3 + 0x80000 / 3;
    int extra;
    if (g_TrackGame->bajaEnduroEvent)
        extra = 2 * (0x80000 / 3);
    else
        extra = 3 * (0x80000 / 3);
    return total + extra;
}

// 0x004e06a0
int BaseQuarryEvent::Create3DObjects(int value) {
    char message[388];
    visualCue = (new(__FILE__, 1369) VisualCue(g_TrackGame->mode.visualCuesOn))
                     ->LoadVisualCue(renderContext, (int)lightManager, value, (RunwayTerrainView*)eventTerrain,
                                     Vector3(0.0f, 0.0f, 0.0f), (VisualCueBikeRaceView*)raceView, 45,
                                     (ArcadeCameraView*)raceCamera, 3.0f, 0.1f, 0.85f);
    if (!AppendChild(visualCue, -1)) {
        sprintf(message, "Visual Cue not loaded.\n");
        return 0;
    }
    return 1;
}

// 0x004e07c0
void BaseQuarryEvent::CreateOverlays() {
    char message[388];
    char name[0x40];
    OverlayRect screen;
    OverlayRect cue;
    OverlayRect text;
    int width;
    int height;

    instrumentOverlay = new(__FILE__, 1412) InstrumentOverlay(g_TrackGame->mode.instrumentOverlayOn);
    instrumentOverlay = instrumentOverlay->Setup((RenderTarget*)renderContext, g_TrackGame->textureManager,
                                                   (InstrumentKrustyBikeCameraView*)raceCamera);
    if (!instrumentOverlay)
        sprintf(message, "Instrument Overlay not created.\n");

    // One load each of the height and width feeds two copies: the cue's size
    // uses height/width, its position and the overlays the screen rectangle.
    height = ((RenderTarget*)renderContext)->targetHeight;
    screen.bottom = ((RenderTarget*)renderContext)->targetHeight;
    width = ((RenderTarget*)renderContext)->targetWidth;
    screen.right = ((RenderTarget*)renderContext)->targetWidth;
    screen.top = 0;
    screen.left = 0;
    if (visualCue) {
        cue.top = (int)((visualCue->screenY + 0.06f) * screen.bottom);
        cue.left = (int)((visualCue->screenX - 0.04f) * screen.right);
    } else {
        cue.top = 0;
        cue.left = 0;
    }
    cue.bottom = cue.top - (int)(height * -0.020833334f);
    cue.right = cue.left - (int)(width * -0.046875f);

    chatOverlay = new(__FILE__, 1433) ChatOverlay(1);
    chatOverlay = chatOverlay->Setup((RenderTarget*)renderContext, g_TrackGame->textureManager, raceCamera, screen, cue);
    if (!chatOverlay)
        sprintf(message, "Chat Overlay not created.\n");
    if (chatOverlay && g_TrackGame->mode.chatInputLine == 1)
        chatOverlay->ShowInputLine();

    radarOverlay = new(__FILE__, 1444) RadarOverlay(1);
    radarOverlay = radarOverlay->LoadMap((RenderTarget*)renderContext, g_TrackGame->textureManager, (int)raceCamera, screen);
    if (!radarOverlay)
        sprintf(message, "Radar Overlay not created.\n");

    statsOverlay = new(__FILE__, 1451) StatsOverlay(1);
    statsOverlay = statsOverlay->LoadPanel((RenderTarget*)renderContext, g_TrackGame->textureManager, raceCamera, screen);
    if (!statsOverlay)
        sprintf(message, "Stats Overlay not created.\n");

    // Retail reads the height and loads 10 and 256 before the allocation and
    // computes the top after it.
    text.bottom = ((RenderTarget*)renderContext)->targetHeight;
    text.left = 10;
    text.right = 256;
    textQueue = new(__FILE__, 1463) TextQueueOverlay(0);
    text.top = text.bottom - 20;
    textQueue = textQueue->Setup(renderContext, text);
    if (!textQueue)
        sprintf(message, "TextQueueOverlay not created.\n");

    if (textQueue) {
        if (g_TrackGame->mode.raceSettings.eventType == 1 || g_TrackGame->mode.raceSettings.eventType == 5)
            g_TrackGame->sceneObject->ReadCurrentSceneName(
                name, g_TrackGame->sceneObject->sceneFile, g_TrackGame->mode.raceSettings.trackNumber);
        else
            g_TrackGame->sceneObject->ReadCurrentSceneName(
                name, g_TrackGame->sceneObject->sceneFile, 0);
        TextQueueMessage line(name, 3.25f);
        textQueue->ReplaceHeadMessage(&line);
    }
}

// 0x004e0be0
void BaseQuarryEvent::AttachOverlays() {
    AppendChild(instrumentOverlay, -1);
    AppendChild(chatOverlay, -1);
    AppendChild(radarOverlay, -1);
    AppendChild(statsOverlay, -1);
    AppendChild(textQueue, -1);
}

// 0x004e0c30
void BaseQuarryEvent::ShowOnOffMessage(int id, int on) {
    char name[0x80];
    char state[0x80];
    char buffer[0x100];

    g_TrackGame->LoadResourceString(id, name, 0x80);
    if (on)
        g_TrackGame->LoadResourceString(0x1407, state, 0x80);
    else
        g_TrackGame->LoadResourceString(0x1408, state, 0x80);
    sprintf(buffer, "%s %s", name, state);
    TextQueueMessage message(buffer, 1.5f);
    textQueue->ReplaceHeadMessage(&message);
}

// 0x004e0ce0
int BaseQuarryEvent::OnControlPress(ControlEvent* event, InputButtonState* entry) {
    if (GameObject::OnControlPress(event, entry))
        return 1;
    if (TestInputEvent(0x31, 0, event, 0xc)) {
        if (g_TrackGame->mode.showRiderNames && g_TrackGame->mode.showRiderPositions) {
            g_TrackGame->mode.showRiderPositions = 0;
            ShowOnOffMessage(0x1412, 1);
        } else if (g_TrackGame->mode.showRiderNames) {
            g_TrackGame->mode.showRiderNames = 0;
            g_TrackGame->mode.showRiderPositions = 1;
            ShowOnOffMessage(0x1413, 1);
        } else if (g_TrackGame->mode.showRiderPositions) {
            g_TrackGame->mode.showRiderPositions = 0;
            ShowOnOffMessage(0x1414, 0);
        } else {
            g_TrackGame->mode.showRiderNames = 1;
            g_TrackGame->mode.showRiderPositions = 1;
            ShowOnOffMessage(0x1414, 1);
        }
        chatOverlay->RedrawNameTags(-1);
        return 1;
    }
    if (TestInputEvent(0x31, 0, event, 0x80000000) && statsOverlay) {
        chatOverlay->CycleNameTagShade();
        return 1;
    }
    if (TestInputEvent(0x3e, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.visualCuesOn) {
            DisableByClassName("RunwayLights");
            DisableByClassName("VisualCue");
        } else {
            EnableByClassName("RunwayLights");
            EnableByClassName("VisualCue");
        }
        g_TrackGame->mode.visualCuesOn = 1 - g_TrackGame->mode.visualCuesOn;
        ShowOnOffMessage(0x13b6, g_TrackGame->mode.visualCuesOn);
        return 1;
    }
    if (TestInputEvent(0x3f, 0, event, 0x80000000)) {
        g_TrackGame->mode.showRaceStatistics = 1 - g_TrackGame->mode.showRaceStatistics;
        ShowOnOffMessage(0x1415, g_TrackGame->mode.showRaceStatistics);
        return 1;
    }
    if (TestInputEvent(0x40, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.showNameTags)
            DisableByClassName("RadarOverlay");
        else
            EnableByClassName("RadarOverlay");
        g_TrackGame->mode.showNameTags = 1 - g_TrackGame->mode.showNameTags;
        ShowOnOffMessage(0x1441, g_TrackGame->mode.showNameTags);
        return 1;
    }
    if (TestInputEvent(0x41, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.instrumentOverlayOn)
            DisableByClassName("InstrumentOverlay");
        else
            EnableByClassName("InstrumentOverlay");
        g_TrackGame->mode.instrumentOverlayOn = 1 - g_TrackGame->mode.instrumentOverlayOn;
        ShowOnOffMessage(0x1416, g_TrackGame->mode.instrumentOverlayOn);
        return 1;
    }
    if (TestInputEvent(0x42, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.skyCubeOn)
            DisableByClassName("SkyCube");
        else
            EnableByClassName("SkyCube");
        g_TrackGame->mode.skyCubeOn = 1 - g_TrackGame->mode.skyCubeOn;
        ShowOnOffMessage(0x1411, g_TrackGame->mode.skyCubeOn);
        ((RenderTarget*)renderContext)->clearTarget = !(skyCube && g_TrackGame->mode.skyCubeOn);
        return 1;
    }
    if (TestInputEvent(0x43, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.particlesOn) {
            DisableByClassName("ParticleManager");
            DisableByClassName("DirtParticleEmitter");
            DisableByClassName("DustParticleEmitter");
            DisableByClassName("DirtChunkParticleEmitter");
            DisableByClassName("SteamParticleEmitter");
        } else {
            EnableByClassName("ParticleManager");
            EnableByClassName("DirtParticleEmitter");
            EnableByClassName("DustParticleEmitter");
            EnableByClassName("DirtChunkParticleEmitter");
            EnableByClassName("SteamParticleEmitter");
        }
        g_TrackGame->mode.particlesOn = 1 - g_TrackGame->mode.particlesOn;
        ShowOnOffMessage(0x140f, g_TrackGame->mode.particlesOn);
        return 1;
    }
    if (TestInputEvent(0x44, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.shadowsOn) {
            DisableByClassName("ProjectedShadow");
            DisableByClassName("TerrainShadow");
            DisableByClassName("D3DIMSoultreeShadow");
        } else {
            EnableByClassName("ProjectedShadow");
            EnableByClassName("TerrainShadow");
            EnableByClassName("D3DIMSoultreeShadow");
        }
        g_TrackGame->mode.shadowsOn = 1 - g_TrackGame->mode.shadowsOn;
        ShowOnOffMessage(0x1410, g_TrackGame->mode.shadowsOn);
        return 1;
    }
    if (TestInputEvent(0x57, 0, event, 0x80000000)) {
        int level = g_TrackGame->mode.detailLevel - 1;
        if (level < 0)
            level = 0;
        g_TrackGame->mode.detailLevel = level;
        g_TrackGame->mode.videoSettings[3] = level;
        g_TrackGame->mode.videoSettings[1] = level;
        g_TrackGame->mode.videoSettings[2] = level;
        g_TrackGame->mode.videoSettings[4] = level;
        reinterpret_cast<Terrain*>(eventTerrain)->SelectQuality(level);
        if (ecoSystem)
            ecoSystem->SetDetailLevel(level);
        if (eventFog)
            eventFog->SetDetailLevel(level);
        eventScene->SetDetailLevel(level);
        reinterpret_cast<BikeRace*>(raceView)->SetDetailLevel(level);
        if (textQueue) {
            char text[0x80];
            char buffer[0x100];
            g_TrackGame->LoadResourceString(0x14c4, text, 0x80);
            sprintf(buffer, "%s = %d", text, level + 1);
            TextQueueMessage message(buffer, 1.5f);
            textQueue->ReplaceHeadMessage(&message);
        }
        return 1;
    }
    if (TestInputEvent(0x58, 0, event, 0x80000000)) {
        int level = g_TrackGame->mode.detailLevel + 1;
        if (level >= 9)
            level = 9;
        g_TrackGame->mode.detailLevel = level;
        g_TrackGame->mode.videoSettings[3] = level;
        g_TrackGame->mode.videoSettings[1] = level;
        g_TrackGame->mode.videoSettings[2] = level;
        g_TrackGame->mode.videoSettings[4] = level;
        reinterpret_cast<Terrain*>(eventTerrain)->SelectQuality(level);
        if (ecoSystem)
            ecoSystem->SetDetailLevel(level);
        if (eventFog)
            eventFog->SetDetailLevel(level);
        eventScene->SetDetailLevel(level);
        reinterpret_cast<BikeRace*>(raceView)->SetDetailLevel(level);
        if (textQueue) {
            char text[0x80];
            char buffer[0x100];
            g_TrackGame->LoadResourceString(0x14c4, text, 0x80);
            sprintf(buffer, "%s = %d", text, level + 1);
            TextQueueMessage message(buffer, 1.5f);
            textQueue->ReplaceHeadMessage(&message);
        }
        return 1;
    }
    if (g_TrackGame->testKey && g_TrackGame->debugOverlay) {
        if (s_DebugPage < 0)
            s_DebugPage = g_TrackGame->debugOverlay->NewPage();
        if (TestInputEvent(0x1c, 0, event, 0x80)) {
            g_TrackGame->debugOverlay->SetRowHighlight(s_DebugPage, s_DebugRow + 1, 0);
            s_DebugRow++;
            if (s_DebugRow > 10)
                s_DebugRow = 0;
        }
    }
    return 0;
}

// 0x004e14a0. Not exact (166 of 1756 bytes, 1760 compiled: the prologue
// keeps the page in edx where retail reloads TrackGame+0x38 into ecx, and
// the rest shifts by that one reload; docs/QUARRYSTUNTEVENT.md forms tried).
int BaseQuarryEvent::PrepareGeometry() {
    GameObject::PrepareGeometry();
    if (g_TrackGame->testKey && g_TrackGame->debugOverlay) {
        if (s_DebugPage < 0)
            s_DebugPage = g_TrackGame->debugOverlay->NewPage();
        if (g_TrackGame->debugOverlay->shownPage == s_DebugPage) {
            char atmosphere[0x40];
            int down = g_TrackGame->controlInterface->IsDeviceButtonDown(0xd0, 0, 0x80, 0);
            int up = g_TrackGame->controlInterface->IsDeviceButtonDown(0xc8, 0, 0x80, 0);
            if (up || down) {
                unsigned int sun = sunLight->PackedColor();
                unsigned int ambient = ambientLight->PackedColor();
                Fog* fog = eventFog;
                float visibility;
                float haziness;
                unsigned int color;
                int value;
                if (fog) {
                    visibility = fog->fogVisibility;
                    haziness = fog->fogHaziness;
                    color = fog->fogColor;
                } else {
                    visibility = 0.0f;
                    haziness = 0.0f;
                    color = 0;
                }
                switch (s_DebugRow) {
                case 0:
                    if (up) {
                        visibility += 0.05f;
                        if (visibility > 1.0f)
                            visibility = 1.0f;
                    } else if (down) {
                        visibility -= 0.05f;
                        if (visibility < 0.0f)
                            visibility = 0.0f;
                    }
                    break;
                case 1:
                    if (up) {
                        haziness -= 0.05f;
                        if (haziness < 0.0f)
                            haziness = 0.0f;
                    } else if (down) {
                        haziness += 0.05f;
                        if (haziness > 1.0f)
                            haziness = 1.0f;
                    }
                    break;
                case 2:
                    value = (color >> 16) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    color = (color & 0xff00ffff) | (value << 16);
                    break;
                case 3:
                    value = (color >> 8) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    color = (color & 0xffff00ff) | (value << 8);
                    break;
                case 4:
                    value = color & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    color = (color & 0xffffff00) | value;
                    break;
                case 5:
                    value = (sun >> 16) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    sun = (sun & 0xff00ffff) | (value << 16);
                    break;
                case 6:
                    value = (sun >> 8) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    sun = (sun & 0xffff00ff) | (value << 8);
                    break;
                case 7:
                    value = sun & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    sun = (sun & 0xffffff00) | value;
                    break;
                case 8:
                    value = (ambient >> 16) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    ambient = (ambient & 0xff00ffff) | (value << 16);
                    break;
                case 9:
                    value = (ambient >> 8) & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    ambient = (ambient & 0xffff00ff) | (value << 8);
                    break;
                case 10:
                    value = ambient & 0xff;
                    if (up) {
                        if (++value > 0xff)
                            value = 0xff;
                    } else if (down) {
                        if (--value < 0)
                            value = 0;
                    }
                    ambient = (ambient & 0xffffff00) | value;
                    break;
                }
                if (fog)
                    fog->SetParameters(color, visibility, haziness);
                sunLight->SetColor(sun);
                ambientLight->SetColor(ambient);
                if (ecoSystem)
                    ecoSystem->LightingChanged();
            }
            if (eventFog) {
                if (eventFog->fogKind == 0x100)
                    strcpy(atmosphere, "(Table Fog)");
                else if (eventFog->fogKind == 0x80)
                    strcpy(atmosphere, "(Vertex Fog)");
                else if (eventFog->fogKind == 0x10000)
                    strcpy(atmosphere, "(Range Fog)");
            } else {
                strcpy(atmosphere, "(No Fog)");
            }
            g_TrackGame->debugOverlay->Title(s_DebugPage, "Atmosphere %s", atmosphere);
            if (eventFog) {
                g_TrackGame->debugOverlay->Print(s_DebugPage, "Visibility %f", eventFog->fogVisibility);
                g_TrackGame->debugOverlay->Print(s_DebugPage, "Haziness %f", eventFog->fogHaziness);
                g_TrackGame->debugOverlay->Print(s_DebugPage, "Fog R      %d", (eventFog->fogColor >> 16) & 0xff);
                g_TrackGame->debugOverlay->Print(s_DebugPage, "Fog G      %d", (eventFog->fogColor >> 8) & 0xff);
                g_TrackGame->debugOverlay->Print(s_DebugPage, "Fog B      %d", eventFog->fogColor & 0xff);
            }
            g_TrackGame->debugOverlay->Print(s_DebugPage, "SunLight R %d", (sunLight->PackedColor() >> 16) & 0xff);
            g_TrackGame->debugOverlay->Print(s_DebugPage, "SunLight G %d", (sunLight->PackedColor() >> 8) & 0xff);
            g_TrackGame->debugOverlay->Print(s_DebugPage, "SunLight B %d", sunLight->PackedColor() & 0xff);
            g_TrackGame->debugOverlay->Print(s_DebugPage, "Ambient  R %d", (ambientLight->PackedColor() >> 16) & 0xff);
            g_TrackGame->debugOverlay->Print(s_DebugPage, "Ambient  G %d", (ambientLight->PackedColor() >> 8) & 0xff);
            g_TrackGame->debugOverlay->Print(s_DebugPage, "Ambient  B %d", ambientLight->PackedColor() & 0xff);
            g_TrackGame->debugOverlay->SetRowHighlight(s_DebugPage, s_DebugRow + 1, 1);
        }
    }
    return 0;
}

// The four vector constants that open many retail files (see
// src/krusty2/math/Math3D.h): 0x00689b90, 0x00689ba0, 0x00689bb0 and
// 0x00689b80. Their initialisers 0x004e1b80..0x004e1cbb sit between slot 12
// (0x004e14a0) and slot 24, so the definitions come late in the file; defining
// them first changes the code VC6 emits for the constructor.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// The data of network message 5 (a player left): the player's id at +0x08.
struct DPMsgDestroyPlayerOrGroup {
    int type;                                 // +0x00 5: a DirectPlay system message (no Send passes 5; NetMessage::Set 0x004aacc0 reads the whole dword when from == 0)
    int playerType;
    int playerId;                             // +0x08 matched against racers' networkPlayerId
};

// 0x004e1cc0
int BaseQuarryEvent::OnNetworkMessage(int type, void* data, int from, int to, int flags) {
    if (GameObject::OnNetworkMessage(type, data, from, to, flags))
        return 1;
    if (g_TrackGame->uiInteractionBlocked)
        return 1;
    if (type == 5) {
        DPMsgDestroyPlayerOrGroup* leave = (DPMsgDestroyPlayerOrGroup*)data;
        char name[0x80];
        char text[0x80];
        char buffer[0x80];
        strcpy(name, "");
        for (int i = 0; i < g_TrackGame->mode.racerCount; i++) {
            if (g_TrackGame->mode.racers[i].networkPlayerId == leave->playerId) {
                int n = strlen(g_TrackGame->mode.racers[i].playerName);
                int length = n > 0x7f ? 0x7f : n;
                strncpy(name, g_TrackGame->mode.racers[i].playerName, length);
                name[length] = 0;
                break;
            }
        }
        g_TrackGame->LoadResourceString(0xbd6, text, 0x80);
        sprintf(buffer, "%s %s", name, text);
        TextQueueMessage message(buffer, 3.25f);
        textQueue->AppendMessage(&message);
        reinterpret_cast<BikeRace*>(raceView)->HandlePlayerLeft(leave->playerId);
        if (g_TrackGame->mode.raceSettings.raceMode == 2)
            g_TrackGame->eventManager->RemovePlayer(leave->playerId);
    } else if (from && type == 0x89) {
        char text[0x80];
        char buffer[0x80];
        g_TrackGame->LoadResourceString(0xbd5, text, 0x80);
        sprintf(buffer, "%s %s", (char*)data + 4, text);
        TextQueueMessage message(buffer, 3.25f);
        textQueue->AppendMessage(&message);
        reinterpret_cast<BikeRace*>(raceView)->HandlePlayerLeft(from);
        if (g_TrackGame->mode.raceSettings.raceMode == 2)
            g_TrackGame->eventManager->RemovePlayer(from);
    }
    return 0;
}

// 0x004e1f00
void BaseQuarryEvent::ResetClock() {
    clockMinutes = 0.0f;
    clockSeconds = 0.0f;
}

// 0x004e1f10
BaseQuarryEvent::~BaseQuarryEvent() {
    delete terrainStream;
    Release();
    if (g_TrackGame->texturesCached) {
        opaqueTextureGroup->Release();
        alphaTextureGroup->Release();
    }
    ((RenderTarget*)renderContext)->currentCamera = 0;
}

// The remaining overrides are bodies the linker folded with identical ones
// elsewhere; the retail vtable 0x0055766c points at the surviving copies.

// 0x00499af0 (KrustyUI's slot 18 is the same body)
int BaseQuarryEvent::OnActivate() {
    GameObject::OnActivate();
    return 1;
}

// 0x00464e90
void BaseQuarryEvent::UnknownVirtualSlot28() {
}

// 0x00467ae0
int BaseQuarryEvent::CreateEventCollision() {
    return 1;
}

// 0x00464e80
void BaseQuarryEvent::OnRaceViewAttached(int value) {
}

// 0x00464e80
void BaseQuarryEvent::CreateRacer3DObjects(int value) {
}

// 0x00467ae0
int BaseQuarryEvent::PrepareEvent3DObjects() {
    return 1;
}

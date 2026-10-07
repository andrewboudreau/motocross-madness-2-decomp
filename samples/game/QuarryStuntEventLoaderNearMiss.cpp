// Near-miss candidate for BaseQuarryEvent::UnknownFunction4de590 (0x004de590,
// 7952 bytes; QuarryStuntEvent.cpp, docs/QUARRYSTUNTEVENT.md): the event
// loader. It picks texture detail levels from the video and system memory,
// then creates the audio, lights, camera, particles, scene, visibility
// quadtree, shadows, fog and sky, terrain, ecosystem, race, collision links,
// podium and the remaining race objects, calling `progress` between stages.
//
// Status: near miss, about 95% of instructions align (difflib over the
// register- and frame-normalised listings; 445 of 8005 bytes at the same
// positions). The call sequence, allocation sizes and lines, EH states,
// error paths and frame size (0x650) match. Differences: the stack slots of
// most locals (VC6 orders the frame by use, not by declaration), register
// choices and expression shapes in the memory-budget decision tree
// (0x004de7e0..0x004ded5e), the zero register retail keeps in esi around
// the collision links, and the animation loop's rotation.
//
// The views below are local to this sample: they declare only what the
// loader touches of classes reconstructed (or not) elsewhere, under
// UnknownQuarry* names. Addresses are bound in the .bindings.json.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "../../src/reconstructed/QuarryEvent.h"

#include "../../src/reconstructed/ArcadeObject.h"
#include "../../src/reconstructed/AuralScape.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/EventManager.h"
#include "../../src/reconstructed/LightEmitter.h"
#include "../../src/reconstructed/MemTag.h"
#include "../../src/reconstructed/PCGame.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/SceneManager.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/TextureMapManager.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/UnknownResourceManager.h"

// 0x0059af10: an object count the memory estimate weighs at 0x2aaaa bytes.
extern int g_UnknownGlobal59af10;

// 0x0045c040 (cdecl): preloads the ecosystem `path` from `stream`.
void UnknownFunction45c040(TextureMapManager* manager, const char* path, UnknownTextureStream* stream,
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
    void UnknownFunction432120(int value);                     // 0x00432120
    void UnknownFunction435fe0();                              // 0x00435fe0
    void UnknownFunction439400(int value);                     // 0x00439400
    void UnknownFunction439410(UnknownQuarryCollision* other); // 0x00439410: also collides with `other`
    void UnknownFunction4394d0(void* model, int kind);         // 0x004394d0
};

// A physics static model (Scene caster +0x08).
struct UnknownQuarryPhysicsObject {
    unsigned char field_0x000[0x128];
    UnknownQuarryCollision* field_0x128;
    unsigned char field_0x12c[0x1f4 - 0x12c];
    UnknownTerrain* field_0x1f4;
    void* field_0x1f8;
};

// The terrain as the loader reads it.
struct UnknownQuarryTerrain {
    unsigned char field_0x00[0x40];
    void* field_0x40;
};

// CarProcedural's terrain pointer (Scene's UnknownSceneAnimatedObject
// leaves +0x20c unnamed).
struct UnknownQuarryCar {
    unsigned char field_0x000[0x20c];
    UnknownTerrain* field_0x20c;
};

// A racer as the loader reads it.
struct UnknownQuarryRacerRider {
    unsigned char field_0x000[0x1a0];
    D3DIMSoultreeObject* field_0x1a0;
};
struct UnknownQuarryRacerState {
    unsigned char field_0x00[0x38];
    UnknownQuarryCollision* field_0x38;
};
struct UnknownQuarryRaceView {
    unsigned char field_0x000[0x18c];
    unsigned char field_0x18c;
};
struct UnknownQuarryRacer {
    unsigned char field_0x000[0x3bc];
    D3DIMSoultreeObject* field_0x3bc;           // bike model
    unsigned char field_0x3c0[0x5c4 - 0x3c0];
    UnknownQuarryRacerRider* field_0x5c4;
    unsigned char field_0x5c8[0x5f0 - 0x5c8];
    UnknownQuarryCollision* field_0x5f0;
    UnknownQuarryCollision* field_0x5f4;
    unsigned char field_0x5f8[0x604 - 0x5f8];
    UnknownQuarryRacerState* field_0x604;
    unsigned char field_0x608[0x735 - 0x608];
    unsigned char field_0x735;
    unsigned char field_0x736[0x740 - 0x736];
    UnknownQuarryRaceView* field_0x740;
    unsigned char field_0x744[0x1414 - 0x744];
    UnknownQuarryCollision* field_0x1414;
    UnknownQuarryCollision* field_0x1418;
};

// BikeRace (0x400 bytes, constructor 0x00417c30) as the loader uses it.
struct UnknownQuarryRaceCharacter {
    unsigned char field_0x000[0x210];
    UnknownQuarryCollision* field_0x210;
};
class UnknownQuarryRace : public GameObject {
public:
    explicit UnknownQuarryRace(int flags);                     // 0x00417c30
    // 0x00417ed0: loads the race; returns this, or 0.
    UnknownQuarryRace* UnknownFunction417ed0(void* target, void* particles, LightManager* lights,
                                             UnknownTextureFormatChoice* textures, UnknownTerrain* terrain,
                                             UnknownQuarryCamera* camera, Scene* scene, BaseQuarryEvent* event,
                                             void* shadow);
    UnknownQuarryRacer* UnknownFunction4204e0(int* iterator);  // 0x004204e0: the next racer, or 0
    void UnknownFunction423790(int level);                     // 0x00423790: detail level

    unsigned char field_0x2c[0x38 - 0x2c];
    UnknownQuarryRacer* field_0x38;                            // the player's racer
    unsigned char field_0x3c[0x64 - 0x3c];
    UnknownQuarryRaceCharacter* field_0x64;
    int field_0x68;
    UnknownQuarryRaceCharacter* field_0x6c;
    int field_0x70;
    unsigned char field_0x74[0x400 - 0x74];
};

// The KrustyBikeCamera at BaseQuarryEvent+0x3c (0x3bc bytes).
class UnknownQuarryBikeCamera : public GameObject {
public:
    explicit UnknownQuarryBikeCamera(int flags);               // 0x00497cb0
    // 0x00497d90: sets the camera up; returns it.
    UnknownQuarryBikeCamera* UnknownFunction497d90(void* target, float a, float b, float c, float d, float e,
                                                   int f, int count, int* presets);
    void UnknownFunction463600(UnknownTerrain* terrain);       // 0x00463600

    unsigned char field_0x02c[0x274 - 0x2c];
    unsigned char field_0x274;
    unsigned char field_0x275[0x384 - 0x275];
    int field_0x384;
    int field_0x388;
    int field_0x38c;
    unsigned char field_0x390;
    unsigned char field_0x391;
    unsigned char field_0x392;
    unsigned char field_0x393;
    UnknownQuarryRacer* field_0x394;
    unsigned char field_0x398[0x3b0 - 0x398];
    UnknownQuarryRacer* field_0x3b0;
    UnknownQuarryRacer* field_0x3b4;
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
    GameObject* UnknownFunction52d460(void* target, int a, int b, float width, float breadth, Scene* scene,
                                      float height);
};

// ProjectedShadow (0x134 bytes) and TerrainShadow (0x6780 bytes).
class UnknownQuarryShadow : public GameObject {
public:
    explicit UnknownQuarryShadow(int flags);                   // 0x004da570
    UnknownQuarryShadow* UnknownFunction4da7b0(void* target, TextureMapManager* manager);
    void UnknownFunction4dab00(D3DIMSoultreeObject* model);    // 0x004dab00: casts a shadow
    void UnknownFunction4dace0(int value);                     // 0x004dace0
    void UnknownFunction4dae30(int value);                     // 0x004dae30
    void UnknownFunction4dae50(const Vector3* position, const Vector3* direction, int a); // 0x004dae50

    unsigned char field_0x2c[0x134 - 0x2c];
};
class UnknownQuarryTerrainShadow : public GameObject {
public:
    explicit UnknownQuarryTerrainShadow(int flags);            // 0x00508ae0
    GameObject* UnknownFunction508b80(void* target, UnknownTerrain* terrain, UnknownQuarryShadow* shadow);

    unsigned char field_0x2c[0x6780 - 0x2c];
};

// Fog (0x5c bytes, QuarryEvent.h's Fog) and the objects the sky adds.
class UnknownQuarryFog : public GameObject {
public:
    explicit UnknownQuarryFog(int flags);                      // 0x00462620
    Fog* UnknownFunction462680(void* target, unsigned int color, float visibility, float haziness,
                               float a, float b, float c);

    unsigned char field_0x2c[0x5c - 0x2c];
};
class UnknownQuarryFogObject : public GameObject {
public:
    explicit UnknownQuarryFogObject(int flags);                // 0x00462e10

    unsigned char field_0x2c[0x2c - 0x2c + 1];
};
class UnknownQuarryFogLink : public GameObject {
public:
    explicit UnknownQuarryFogLink(int flags);                  // 0x00462e90
    GameObject* UnknownFunction486a10(void* target, Fog* fog);  // 0x00486a10

    unsigned char field_0x2c[0x30 - 0x2c];
};
class UnknownQuarrySkyCube : public GameObject {
public:
    explicit UnknownQuarrySkyCube(int flags);                  // 0x004fb1e0
    GameObject* UnknownFunction4fb230(void* target, UnknownTextureStream* stream,
                                      UnknownTextureFormatChoice* textures, float size);

    unsigned char field_0x2c[0x48 - 0x2c];
};

// Terrain (0xcc4 bytes) and EcoSystem (0x5c4 bytes).
class UnknownQuarryTerrainObject : public GameObject {
public:
    explicit UnknownQuarryTerrainObject(int flags);            // 0x00505830
    UnknownTerrain* UnknownFunction5059d0(void* target, UnknownTextureStream* stream, int a,
                                          UnknownTextureFormatChoice* textures, int level, int b,
                                          const char* detailTexture, int c);

    unsigned char field_0x2c[0xcc4 - 0x2c];
};
class UnknownQuarryEcoSystem : public GameObject {
public:
    explicit UnknownQuarryEcoSystem(int flags);                // 0x00457250
    EcoSystem* UnknownFunction4594d0(void* target, TextureMapManager* manager, UnknownTerrain* terrain,
                                     LightManager* lights, UnknownSceneEnvironment* environment,
                                     UnknownTextureStream* stream, int format, int a, int level);

    unsigned char field_0x2c[0x5c4 - 0x2c];
};

// The last race object (0x48 bytes).
class UnknownQuarryRaceObject : public GameObject {
public:
    explicit UnknownQuarryRaceObject(int flags);               // 0x00504490
    void UnknownFunction5046e0();                              // 0x005046e0

    unsigned char field_0x2c[0x48 - 0x2c];
};

// The overlays' race hooks.
struct UnknownQuarryOverlayHooks {
    void UnknownFunction519880(UnknownQuarryRace* race);       // 0x00519880 (StatsOverlay)
    void UnknownFunction51d730(UnknownQuarryRace* race);       // 0x0051d730 (ChatOverlay)
    void UnknownFunction51baf0(UnknownQuarryRace* race, void* chat); // 0x0051baf0 (RadarOverlay)
    void UnknownFunction48af20(UnknownQuarryRace* race);       // 0x0048af20 (VisualCue)
};

// The bytes the render target's colour buffers take.
static inline int FrameBufferBytes(RenderTarget* target) {
    return UnknownFunction511970(target->field_0x28) * (target->field_0x14 + 1) * target->field_0x10 *
           target->field_0x0c;
}

// 0x004de590
int BaseQuarryEvent::UnknownFunction4de590(UnknownProgressCallback progress) {
    int terrainBytes;
    int textureBytes;
    int forced;
    int needed;
    int step;
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
    g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9ac0(modelCounts, (char*)&terrainWidth, (char*)&hasCube,
                                                              (int)ecosystem);
    sceneSteps = 1;
    if (g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9a10(&sceneSteps)) {
        step = (int)(sceneSteps / (sceneSteps * 0.1f));
        if (progress)
            progress(&step);
    }
    UnknownTextureStream* stream = new(__FILE__, 252) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    UnknownTextureStream* preloaded = stream;
    if (ecosystem[0]) {
        if (strstr(ecosystem, ".est"))
            strcpy(strstr(ecosystem, ".est"), ".esb");
        if (!g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, ecosystem, "rb", 0))
            sprintf(text, "No Ecosystem found in env file.  Ecosystem not preloaed.\n");
        else
            UnknownFunction45c040(g_UnknownGlobal56e26c->field_0x3c, ecosystem, stream, 0x115c);
    }

    // The texture detail levels: 0 keeps every texture, 1 and 2 halve them.
    if (g_UnknownGlobal56e26c->field_0x2d0) {
        g_UnknownGlobal56e26c->field_0x2d5_bit2 = 0;
        if (memory.dwTotalPhys > 0x5a00000) {
            terrainTextures.field_0x14 = 0;
            skyTextures.field_0x14 = 0;
            modelTextures.field_0x14 = 0;
        } else if (memory.dwTotalPhys > 0x3c00000) {
            terrainTextures.field_0x14 = 1;
            skyTextures.field_0x14 = 0;
            modelTextures.field_0x14 = 0;
        } else if (memory.dwTotalPhys <= 0x2000000) {
            terrainTextures.field_0x14 = 2;
            skyTextures.field_0x14 = 2;
            modelTextures.field_0x14 = 2;
        } else {
            terrainTextures.field_0x14 = 1;
            skyTextures.field_0x14 = 1;
            modelTextures.field_0x14 = 1;
        }
    } else {
        int videoMemory = ((PCRenderTarget*)field_0x18)->field_0x04->field_0x54;
        sprintf(key, "DriverInfo\\%s\\VideoMemoryMB", ((PCRenderTarget*)field_0x18)->field_0x04->field_0x4bc);
        int megabytes = g_UnknownGlobal56e26c->UnknownVirtualSlot20(key, -1);
        forced = g_UnknownGlobal56e26c->mode.field_0x195c[0];
        if (megabytes != -1)
            videoMemory = megabytes << 20;
        PCRenderTarget* target = (PCRenderTarget*)field_0x18;
        int available = videoMemory - FrameBufferBytes(target);
        if (g_UnknownGlobal56e26c->field_0x424.platformId != 2 && !(target->field_0x04->field_0x1b8 & 0x400)) {
            available = videoMemory - 0x400000;
            if (!(target->field_0x164 & 0x4000))
                available = videoMemory - 0x200000;
        }
        terrainBytes = UnknownFunction4e04a0((int)terrainWidth);
        int cubeBytes = hasCube ? UnknownFunction4e04e0() : 0;
        textureBytes = UnknownFunction4e04f0((int*)modelCounts);
        int raceBytes = UnknownFunction4e0560();
        needed = g_UnknownGlobal59af10 * 0x2aaaa + raceBytes + textureBytes + cubeBytes + terrainBytes + 0xa0000;
        terrainTextures.field_0x14 = 0;
        skyTextures.field_0x14 = 0;
        modelTextures.field_0x14 = 0;
        if (((UnknownDisplay*)g_UnknownGlobal56e26c->field_0x0c)->field_0x9f0) {
            g_UnknownGlobal56e26c->field_0x2d5_bit2 = 0;
            if (available - needed <= 0) {
                if (forced) {
                    if (needed - terrainBytes + terrainBytes / 4 < available) {
                        terrainTextures.field_0x14 = 1;
                    } else if (needed - cubeBytes - terrainBytes + (cubeBytes + terrainBytes) / 4 < available) {
                        terrainTextures.field_0x14 = 1;
                        skyTextures.field_0x14 = 1;
                    } else if (needed - textureBytes - cubeBytes - terrainBytes +
                                   (textureBytes + cubeBytes + terrainBytes) / 4 < available) {
                        terrainTextures.field_0x14 = 1;
                        skyTextures.field_0x14 = 1;
                        modelTextures.field_0x14 = 1;
                    } else {
                        terrainTextures.field_0x14 = 2;
                        skyTextures.field_0x14 = 2;
                        modelTextures.field_0x14 = 2;
                    }
                } else if (available - needed + memory.dwTotalPhys <= 0x2800000 && memory.dwTotalPhys <= 0x5000000) {
                    if (memory.dwTotalPhys > 0x3c00000) {
                        terrainTextures.field_0x14 = 1;
                    } else if (needed - terrainBytes + terrainBytes / 4 < available) {
                        terrainTextures.field_0x14 = 1;
                    } else if (needed - cubeBytes - terrainBytes + (cubeBytes + terrainBytes) / 4 < available) {
                        terrainTextures.field_0x14 = 1;
                        skyTextures.field_0x14 = 1;
                    } else if (needed - textureBytes - cubeBytes - terrainBytes +
                                   (textureBytes + cubeBytes + terrainBytes) / 4 < available) {
                        terrainTextures.field_0x14 = 1;
                        skyTextures.field_0x14 = 1;
                        modelTextures.field_0x14 = 1;
                    } else if (needed - textureBytes - cubeBytes - terrainBytes + (textureBytes + cubeBytes) / 4 +
                                   terrainBytes / 16 < available) {
                        terrainTextures.field_0x14 = 2;
                        skyTextures.field_0x14 = 1;
                        modelTextures.field_0x14 = 1;
                    } else {
                        terrainTextures.field_0x14 = 2;
                        skyTextures.field_0x14 = 2;
                        if (needed - textureBytes - cubeBytes - terrainBytes + (textureBytes + cubeBytes) / 4 +
                                (cubeBytes + terrainBytes) / 16 < available)
                            modelTextures.field_0x14 = 1;
                        else
                            modelTextures.field_0x14 = 2;
                    }
                }
            }
        } else {
            int margin = available * g_UnknownGlobal56e26c->UnknownVirtualSlot20("MarginPercentage", 10) / 100;
            int minimum = g_UnknownGlobal56e26c->UnknownVirtualSlot20("MinMarginKBytes", 0x200) << 10;
            if (margin < minimum)
                margin = minimum;
            available -= margin;
            if (available < 0)
                available = 0;
            if (available > needed) {
                terrainTextures.field_0x14 = 0;
                skyTextures.field_0x14 = 0;
                modelTextures.field_0x14 = 0;
                g_UnknownGlobal56e26c->field_0x2d5_bit2 = 0;
            } else if (!forced && memory.dwTotalPhys > 0x5a00000) {
                terrainTextures.field_0x14 = 0;
                skyTextures.field_0x14 = 0;
                modelTextures.field_0x14 = 0;
                g_UnknownGlobal56e26c->field_0x2d5_bit2 = 1;
            } else if (needed - terrainBytes + terrainBytes / 4 < available) {
                g_UnknownGlobal56e26c->field_0x2d5_bit2 = 0;
                terrainTextures.field_0x14 = 1;
            } else if (!forced && memory.dwTotalPhys > 0x3c00000) {
                g_UnknownGlobal56e26c->field_0x2d5_bit2 = 1;
                terrainTextures.field_0x14 = 1;
            } else {
                int all = textureBytes + cubeBytes + terrainBytes;
                if (needed - textureBytes - cubeBytes - terrainBytes + all / 4 < available) {
                    g_UnknownGlobal56e26c->field_0x2d5_bit2 = 0;
                    terrainTextures.field_0x14 = 1;
                    skyTextures.field_0x14 = 1;
                    modelTextures.field_0x14 = 1;
                } else if (needed - textureBytes - cubeBytes - terrainBytes + (textureBytes + cubeBytes) / 4 +
                               terrainBytes / 16 < available) {
                    g_UnknownGlobal56e26c->field_0x2d5_bit2 = 0;
                    terrainTextures.field_0x14 = 2;
                    skyTextures.field_0x14 = 1;
                    modelTextures.field_0x14 = 1;
                } else if (needed - textureBytes - cubeBytes - terrainBytes + (textureBytes + cubeBytes) / 4 +
                               (cubeBytes + terrainBytes) / 16 < available) {
                    g_UnknownGlobal56e26c->field_0x2d5_bit2 = 0;
                    terrainTextures.field_0x14 = 2;
                    skyTextures.field_0x14 = 2;
                    modelTextures.field_0x14 = 1;
                } else {
                    if (needed - textureBytes - cubeBytes - terrainBytes + all / 16 < available)
                        g_UnknownGlobal56e26c->field_0x2d5_bit2 = 0;
                    else
                        g_UnknownGlobal56e26c->field_0x2d5_bit2 = !forced;
                    terrainTextures.field_0x14 = 2;
                    skyTextures.field_0x14 = 2;
                    modelTextures.field_0x14 = 2;
                }
            }
        }
        if (g_UnknownGlobal56e26c->field_0x2d5_bit2) {
            field_0x94 = (GameObject*)g_UnknownGlobal56e26c->field_0x3c->UnknownFunction511180(
                ((PCRenderTarget*)field_0x18)->field_0x28, 0, 2, 1);
            field_0x98 = (GameObject*)g_UnknownGlobal56e26c->field_0x3c->UnknownFunction511180(0x115c, 0, 2, 1);
        }
    }
    modelTextures.field_0x00 = g_UnknownGlobal56e26c->field_0x3c;
    skyTextures.field_0x00 = g_UnknownGlobal56e26c->field_0x3c;
    terrainTextures.field_0x00 = g_UnknownGlobal56e26c->field_0x3c;
    modelTextures.field_0x04 = (ManagedTextureGroup*)field_0x94;
    skyTextures.field_0x04 = (ManagedTextureGroup*)field_0x94;
    terrainTextures.field_0x04 = (ManagedTextureGroup*)field_0x94;
    modelTextures.field_0x08 = (ManagedTextureGroup*)field_0x98;
    skyTextures.field_0x08 = (ManagedTextureGroup*)field_0x98;
    terrainTextures.field_0x08 = (ManagedTextureGroup*)field_0x98;
    modelTextures.field_0x0c = ((PCRenderTarget*)field_0x18)->field_0x28;
    skyTextures.field_0x0c = ((PCRenderTarget*)field_0x18)->field_0x28;
    terrainTextures.field_0x0c = ((PCRenderTarget*)field_0x18)->field_0x28;
    modelTextures.field_0x10 = 0x115c;
    skyTextures.field_0x10 = 0x115c;
    terrainTextures.field_0x10 = 0x115c;
    if (progress)
        progress(0);

    g_MemTagStack->Push("Audio");
    field_0x9c = (new(__FILE__, 576) AuralScape(g_UnknownGlobal56e26c->mode.field_0xa28))
                     ->UnknownFunction402f00(field_0x18, 16);
    field_0x9c->field_0xa0_bit0 = g_UnknownGlobal56e26c->mode.field_0xa34;
    field_0xa0 = field_0x9c->UnknownFunction403000();
    field_0x9c->UnknownFunction403010(field_0xa0, 1.0f, 1.0f);
    g_MemTagStack->Pop(0);

    field_0x7c = new(__FILE__, 582) LightManager(1);
    UnknownFunction469190(field_0x7c->UnknownVirtualSlot8(field_0x18), -1);
    presets[0] = 0;
    presets[1] = 1;
    presets[2] = 2;
    presets[3] = 3;
    presets[4] = 4;
    presets[5] = 5;
    presets[6] = 6;
    presets[7] = 7;
    field_0x3c = (UnknownQuarryCamera*)(new(__FILE__, 585) UnknownQuarryBikeCamera(1))
                     ->UnknownFunction497d90(field_0x18, 0.55f, 0.3f, 27.0f, 7.0f, 1.0485f, 2, 8, presets);
    if (!UnknownFunction469190((GameObject*)field_0x3c, -1)) {
        sprintf(text, "KrustyBikeCamera not created.\n");
        return 0;
    }
    if (progress)
        progress(0);
    UnknownVirtualSlot29();

    g_MemTagStack->Push("Particles");
    field_0x78 = new(__FILE__, 613) UnknownQuarryParticles(g_UnknownGlobal56e26c->mode.field_0xa54);
    g_MemTagStack->Push("Scene");
    field_0x2c = (new(__FILE__, 624) Scene(field_0x25_bit0))
                     ->UnknownFunction4ea7e0(g_UnknownGlobal56e26c->sceneObject, (int)field_0x9c, (int)&modelTextures, 8);
    if (!field_0x2c) {
        sprintf(text, "Scene not created.\n");
        return 0;
    }
    if (!field_0x2c->UnknownFunction4efb20(field_0x18, field_0x7c, (int)field_0x78, (int)g_UnknownGlobal56e26c->field_0x3c,
                                           (int)&modelTextures, "Teraform\\Skies", (void (*)(int))progress,
                                           (int)ceil(sceneSteps * 0.1f)))
        return 0;
    if (progress)
        progress(0);

    g_MemTagStack->Push("QuadTree");
    UnknownQuarryVisibility* visibility = (UnknownQuarryVisibility*)(new(__FILE__, 659) UnknownQuarryVisibility(1))
        ->UnknownFunction52d460(field_0x18, 0, 0,
                                field_0x2c->field_0xa4->terrainWidth * field_0x2c->field_0xa4->terrainWidthScale * 256.0f,
                                field_0x2c->field_0xa4->terrainBreadth * field_0x2c->field_0xa4->terrainBreadthScale * 256.0f,
                                field_0x2c, 96.0f);
    if (!visibility)
        sprintf(text, "VisibilityQuadTree not created.\n");
    g_MemTagStack->Pop(0);
    UnknownSceneLight* shadowLight = 0;
    UnknownSceneLight* flareLight = 0;
    for (int i = 0; i < field_0x2c->field_0xa8; i++) {
        if (field_0x2c->field_0xac[i].emitsLight) {
            if (field_0x2c->field_0xac[i].field_0x00) {
                UnknownFunction469190(field_0x2c->field_0xac[i].field_0x00, -1);
                if (field_0x2c->field_0xac[i].type == 6)
                    field_0x84 = field_0x2c->field_0xac[i].field_0x00;
                else if (field_0x2c->field_0xac[i].type == 2 || field_0x2c->field_0xac[i].type == 4)
                    field_0x80 = field_0x2c->field_0xac[i].field_0x00;
            }
            if (!shadowLight && field_0x2c->field_0xac[i].castsShadows)
                shadowLight = &field_0x2c->field_0xac[i];
            if (!flareLight && field_0x2c->field_0xac[i].hasLensFlare)
                flareLight = &field_0x2c->field_0xac[i];
        }
    }
    if (shadowLight) {
        field_0x58 = new(__FILE__, 698) UnknownQuarryShadow(g_UnknownGlobal56e26c->mode.field_0xa50);
        field_0x58 = ((UnknownQuarryShadow*)field_0x58)->UnknownFunction4da7b0(field_0x18, g_UnknownGlobal56e26c->field_0x3c);
    }
    if (!preloaded)
        stream = new(__FILE__, 713) UnknownTextureStream((int)g_UnknownResourceManager572b44);

    g_MemTagStack->Push("Sky");
    char* cubeFile = field_0x2c->field_0xa4->cubeFile;
    if (cubeFile && *cubeFile) {
        field_0x88 = (new(__FILE__, 720) UnknownQuarryFog(1))
                         ->UnknownFunction462680(field_0x18, field_0x2c->field_0xb0->color, field_0x2c->field_0xb0->visibility,
                                                 field_0x2c->field_0xb0->haziness, 768.0f, 256.0f, 128.0f);
        if (!UnknownFunction469190((GameObject*)field_0x88, -1))
            sprintf(text, "Fog not created.\n");
        if (field_0x88)
            field_0x88->UnknownFunction462db0(g_UnknownGlobal56e26c->mode.field_0x195c[4]);
        UnknownFunction469190((new(__FILE__, 729) UnknownQuarryFogObject(1))->UnknownVirtualSlot8(field_0x18), -1);
        if (!g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, field_0x2c->field_0xa4->cubeFile, "rb", 0)) {
            sprintf(text, "No Sky found in resources.  Sky not created.\n");
            if (stream)
                delete stream;
            return 0;
        }
        field_0x30 = (new(__FILE__, 744) UnknownQuarrySkyCube(g_UnknownGlobal56e26c->mode.field_0xa5c))
                         ->UnknownFunction4fb230(field_0x18, stream, &skyTextures, 1024.0f);
        if (!UnknownFunction469190((GameObject*)field_0x30, -1))
            sprintf(text, "SkyCube not created.\n");
        UnknownFunction469190((new(__FILE__, 750) UnknownQuarryFogLink(1))->UnknownFunction486a10(field_0x18, field_0x88), -1);
        field_0x88->UnknownFunction4627a0(field_0x2c->field_0xb0->color, field_0x88->field_0x38, field_0x88->field_0x40);
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("Terrain");
    if (!g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(field_0x44, field_0x2c->field_0xa4->terrainFile, "rb",
                                                                   (int)terrainPath)) {
        sprintf(text, "No Terrain found in env file.  Terrain not created.\n");
        return 0;
    }
    field_0x40 = (new(__FILE__, 799) UnknownQuarryTerrainObject(1))
                     ->UnknownFunction5059d0(field_0x18, field_0x44, 1, &terrainTextures,
                                             g_UnknownGlobal56e26c->mode.field_0x195c[3], 0,
                                             field_0x2c->field_0xa4->detailTexture, 0);
    if (!UnknownFunction469190((GameObject*)field_0x40, -1)) {
        sprintf(text, "Terrain not created.\n");
        return 0;
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("Shadow");
    if (field_0x58) {
        field_0x54 = (new(__FILE__, 822) UnknownQuarryTerrainShadow(g_UnknownGlobal56e26c->mode.field_0xa50))
                         ->UnknownFunction508b80(field_0x18, field_0x40, (UnknownQuarryShadow*)field_0x58);
        UnknownFunction469190((GameObject*)field_0x54, -1);
    }
    UnknownFunction469190(visibility, -1);

    g_MemTagStack->Push("EcoSystem");
    if (!stream)
        stream = new(__FILE__, 848) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (strstr(field_0x2c->field_0xa4->ecosystemFile, ".est"))
        strcpy(strstr(field_0x2c->field_0xa4->ecosystemFile, ".est"), ".esb");
    if (stream && !g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, field_0x2c->field_0xa4->ecosystemFile,
                                                                           "rb", 0)) {
        sprintf(text, "No Ecosystem found in env file.  Ecosystem not created.\n");
    } else {
        field_0x48 = (new(__FILE__, 870) UnknownQuarryEcoSystem(1))
                         ->UnknownFunction4594d0(field_0x18, g_UnknownGlobal56e26c->field_0x3c, field_0x40, field_0x7c,
                                                 field_0x2c->field_0xa4, stream, 0x115c,
                                                 g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x10,
                                                 g_UnknownGlobal56e26c->mode.field_0x195c[1]);
        if (!field_0x48)
            sprintf(text, "EcoSystem not created.\n");
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("3DObjects");
    if (!UnknownVirtualSlot35())
        return 0;
    g_MemTagStack->Push("BikeRace");
    field_0x34 = (UnknownKrustyBikeView*)new(__FILE__, 898) UnknownQuarryRace(0);
    g_MemTagStack->Push("Particles");
    if (field_0x78)
        field_0x78 = ((UnknownQuarryParticles*)field_0x78)
                         ->UnknownVirtualSlot27(field_0x18, g_UnknownGlobal56e26c->field_0x3c,
                                                field_0x2c->field_0xa4->particleTexture, field_0x7c);
    g_MemTagStack->Push("3DObjects");
    if (!UnknownVirtualSlot27((int)&modelTextures))
        return 0;
    g_UnknownGlobal56e26c->mode.field_0x23a4 = field_0x2c->field_0xbc;
    g_MemTagStack->Push("BikeRace");
    field_0x34 = (UnknownKrustyBikeView*)((UnknownQuarryRace*)field_0x34)
                     ->UnknownFunction417ed0(field_0x18, field_0x78, field_0x7c, &modelTextures, field_0x40, field_0x3c,
                                             field_0x2c, this, field_0x58);
    if (!field_0x34) {
        sprintf(text, "BikeRace create failed.\n");
        return 0;
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("Collision");
    if (field_0x2c->field_0xb8) {
        for (int i = 0; i < field_0x2c->field_0xb8->field_0x00; i++) {
            if (field_0x2c->field_0xb8->field_0x04[i].physics) {
                ((UnknownQuarryPhysicsObject*)field_0x2c->field_0xb8->field_0x04[i].field_0x08)->field_0x1f4 = field_0x40;
                if (field_0x40)
                    ((UnknownQuarryPhysicsObject*)field_0x2c->field_0xb8->field_0x04[i].field_0x08)->field_0x1f8 =
                        ((UnknownQuarryTerrain*)field_0x40)->field_0x40;
                ((UnknownQuarryPhysicsObject*)field_0x2c->field_0xb8->field_0x04[i].field_0x08)->field_0x128->UnknownFunction435fe0();
                ((UnknownQuarryPhysicsObject*)field_0x2c->field_0xb8->field_0x04[i].field_0x08)->field_0x128->UnknownFunction432120(1);
                ((UnknownQuarryPhysicsObject*)field_0x2c->field_0xb8->field_0x04[i].field_0x08)->field_0x128->UnknownFunction4394d0(field_0x2c->field_0xb8->field_0x04[i].field_0x08, 0x68);
            } else if (field_0x2c->field_0xb8->field_0x04[i].field_0x0c && ((D3DIMSoultreeObject*)field_0x2c->field_0xb8->field_0x04[i].field_0x04)->UnknownFunction4fda30() == 1) {
                ((UnknownQuarryCollision*)field_0x2c->field_0xb8->field_0x04[i].field_0x0c)->UnknownFunction435fe0();
                ((UnknownQuarryCollision*)field_0x2c->field_0xb8->field_0x04[i].field_0x0c)->UnknownFunction432120(1);
            }
        }
    }
    UnknownQuarryRace* race = (UnknownQuarryRace*)field_0x34;
    if (race->field_0x6c && race->field_0x6c->field_0x210) {
        race->field_0x6c->field_0x210->Release();
        ((UnknownQuarryRace*)field_0x34)->field_0x6c->field_0x210 = 0;
    }
    if (((UnknownQuarryRace*)field_0x34)->field_0x64 && ((UnknownQuarryRace*)field_0x34)->field_0x64->field_0x210) {
        ((UnknownQuarryRace*)field_0x34)->field_0x64->field_0x210->Release();
        ((UnknownQuarryRace*)field_0x34)->field_0x64->field_0x210 = 0;
    }
    if (field_0x2c->field_0xb4) {
        for (int i = 0; i < field_0x2c->field_0xb4->field_0x00; i++) {
            if (field_0x2c->field_0xb4->field_0x04[i].field_0x00_bit3) {
                UnknownSceneObject* character = field_0x2c->field_0xb4->field_0x04[i].field_0x04;
                if (character->field_0x210) {
                    ((UnknownQuarryCollision*)character->field_0x210)->UnknownFunction435fe0();
                    ((UnknownQuarryCollision*)character->field_0x210)->UnknownFunction432120(1);
                    ((UnknownQuarryCollision*)character->field_0x210)->UnknownFunction439400(0);
                    if (i != ((UnknownQuarryRace*)field_0x34)->field_0x70 && i != ((UnknownQuarryRace*)field_0x34)->field_0x68) {
                        int iterator = 0;
                        for (UnknownQuarryRacer* racer = ((UnknownQuarryRace*)field_0x34)->UnknownFunction4204e0(&iterator);
                             racer; racer = ((UnknownQuarryRace*)field_0x34)->UnknownFunction4204e0(&iterator)) {
                            if (!racer->field_0x735) {
                                ((UnknownQuarryCollision*)character->field_0x210)->UnknownFunction439410(racer->field_0x1418);
                                ((UnknownQuarryCollision*)character->field_0x210)->UnknownFunction439410(racer->field_0x1414);
                                ((UnknownQuarryCollision*)character->field_0x210)->UnknownFunction439410(racer->field_0x5f0);
                                ((UnknownQuarryCollision*)character->field_0x210)->UnknownFunction439410(racer->field_0x5f4);
                                ((UnknownQuarryCollision*)character->field_0x210)
                                    ->UnknownFunction439410(racer->field_0x604->field_0x38);
                            }
                        }
                    } else if (character->field_0x210) {
                        ((UnknownQuarryCollision*)character->field_0x210)->Release();
                        character->field_0x210 = 0;
                    }
                }
            } else {
                UnknownSceneAnimatedObject* car = field_0x2c->field_0xb4->field_0x04[i].field_0x08;
                ((UnknownQuarryCar*)car)->field_0x20c = field_0x40;
                ((UnknownQuarryCollision*)car->field_0x38)->UnknownFunction439400(1);
                for (int j = 0; j < field_0x2c->field_0xb4->field_0x00; j++) {
                    UnknownSceneEntry* entry = &field_0x2c->field_0xb4->field_0x04[j];
                    ((UnknownQuarryCollision*)car->field_0x38)
                        ->UnknownFunction439410(entry->field_0x00_bit3 ? (UnknownQuarryCollision*)entry->field_0x04->field_0x210
                                                                       : (UnknownQuarryCollision*)entry->field_0x08->field_0x3c);
                }
                ((UnknownQuarryCollision*)car->field_0x3c)->UnknownFunction435fe0();
                ((UnknownQuarryCollision*)car->field_0x3c)->UnknownFunction432120(1);
                ((UnknownQuarryCollision*)car->field_0x3c)->UnknownFunction4394d0(car, 0x69);
                ((UnknownQuarryCollision*)car->field_0x3c)->UnknownFunction439400(0);
                int iterator = 0;
                for (UnknownQuarryRacer* racer = ((UnknownQuarryRace*)field_0x34)->UnknownFunction4204e0(&iterator); racer;
                     racer = ((UnknownQuarryRace*)field_0x34)->UnknownFunction4204e0(&iterator)) {
                    if (!racer->field_0x735) {
                        ((UnknownQuarryCollision*)car->field_0x3c)->UnknownFunction439410(racer->field_0x1418);
                        ((UnknownQuarryCollision*)car->field_0x3c)->UnknownFunction439410(racer->field_0x1414);
                        ((UnknownQuarryCollision*)car->field_0x3c)->UnknownFunction439410(racer->field_0x5f0);
                        ((UnknownQuarryCollision*)car->field_0x3c)->UnknownFunction439410(racer->field_0x5f4);
                        ((UnknownQuarryCollision*)car->field_0x3c)->UnknownFunction439410(racer->field_0x604->field_0x38);
                    }
                }
            }
        }
    }
    if (progress)
        progress(0);
    UnknownFunction469190(field_0x2c, -1);

    g_MemTagStack->Push("Shadow");
    field_0x2c->UnknownFunction4edf20((ProjectedShadow*)field_0x58, g_UnknownGlobal56e26c->mode.field_0xa50);
    g_MemTagStack->Pop(0);
    if (field_0x48)
        UnknownFunction469190((GameObject*)field_0x48, -1);
    UnknownFunction469190(field_0x34, -1);
    UnknownVirtualSlot32((int)field_0x34);
    UnknownFunction469190((GameObject*)field_0x78, -1);
    UnknownVirtualSlot28();
    UnknownVirtualSlot30();

    g_MemTagStack->Push("3DObjects");
    int kind = g_UnknownGlobal56e26c->mode.UnknownFunction524100();
    if (kind >= 2 && kind <= 4)
        sprintf(path, "%s\\%s", "Res", "podium.slt");
    else
        sprintf(path, "%s\\%s", "Res", "truckpodium.slt");
    g_UnknownGlobal56e26c->eventManager->field_0x3d0 =
        (int)(new(__FILE__, 1093) ArcadeObject(0))
            ->UnknownFunction401310(field_0x18, (int)field_0x7c, (int)&modelTextures, path,
                                    g_UnknownGlobal56e26c->eventManager->field_0x3c4, 0, 0, 2.0f, 0.5f, 0.5f, 0);
    if (!field_0x34->UnknownFunction469190((GameObject*)g_UnknownGlobal56e26c->eventManager->field_0x3d0, -1))
        return 0;
    if (field_0x9c) {
        field_0x9c->field_0x98 = (UnknownFollowCameraSubject*)field_0x40;
        UnknownFunction469190(field_0x9c, -1);
    }

    g_MemTagStack->Push("Shadow");
    if (field_0x58) {
        UnknownFunction469190((GameObject*)field_0x58, -1);
        ((UnknownQuarryShadow*)field_0x58)->UnknownFunction4dab00(((UnknownQuarryRace*)field_0x34)->field_0x38->field_0x3bc);
        ((UnknownQuarryShadow*)field_0x58)
            ->UnknownFunction4dab00(((UnknownQuarryRace*)field_0x34)->field_0x38->field_0x5c4->field_0x1a0);
        ((UnknownQuarryShadow*)field_0x58)->UnknownFunction4dae50(&shadowLight->position, &shadowLight->look, 3);
        ((UnknownQuarryShadow*)field_0x58)->UnknownFunction4dace0(0x80);
        int ambient = (int)(field_0x84->field_0x54[1] * 255.0f);
        if (!field_0x84)
            field_0x84 = field_0x7c->UnknownFunction4a0190(6);
        if (field_0x84)
            ambient = (int)(field_0x84->field_0x54[1] * 255.0f);
        ((UnknownQuarryShadow*)field_0x58)->UnknownFunction4dae30(ambient);
    }
    if (progress)
        progress(0);

    g_MemTagStack->Push("Collision");
    if (!UnknownVirtualSlot31())
        return 0;
    g_MemTagStack->Push("3DObjects");
    UnknownVirtualSlot33((int)((UnknownQuarryRace*)field_0x34)->field_0x38);
    g_MemTagStack->Push("BikeRace");
    ((UnknownQuarryBikeCamera*)field_0x3c)->UnknownFunction463600(field_0x40);
    UnknownQuarryBikeCamera* camera = (UnknownQuarryBikeCamera*)field_0x3c;
    UnknownQuarryRacer* bike = ((UnknownQuarryRace*)field_0x34)->field_0x38;
    if (camera->field_0x3b0 && !bike->field_0x740->field_0x18c) {
        camera->field_0x3b0->field_0x3bc->UnknownFunction444d40(1);
        camera->field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction444d40(1);
        if (!camera->field_0x3b0->field_0x3bc->field_0x27c)
            camera->field_0x3b0->field_0x3bc->UnknownFunction444d00(1);
        if (!camera->field_0x3b0->field_0x5c4->field_0x1a0->field_0x27c)
            camera->field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction444d00(1);
    }
    camera->field_0x394 = bike;
    camera->field_0x391 = 0;
    camera->field_0x392 = 0;
    camera->field_0x390 = 1;
    camera->field_0x38c = 0;
    camera->field_0x384 = 0;
    camera->field_0x388 = 0;
    camera->field_0x3b0 = bike;
    camera->field_0x274 = 0;
    camera->field_0x3b4 = bike;
    if (!bike->field_0x740->field_0x18c) {
        bike->field_0x3bc->UnknownFunction444d40(0);
        camera->field_0x3b0->field_0x5c4->field_0x1a0->UnknownFunction444d40(0);
    }
    camera->field_0x274 = 0;
    if (field_0x5c)
        ((UnknownQuarryOverlayHooks*)field_0x5c)->UnknownFunction519880((UnknownQuarryRace*)field_0x34);
    if (field_0x64)
        ((UnknownQuarryOverlayHooks*)field_0x64)->UnknownFunction51d730((UnknownQuarryRace*)field_0x34);
    if (field_0x60)
        ((UnknownQuarryOverlayHooks*)field_0x60)->UnknownFunction51baf0((UnknownQuarryRace*)field_0x34, field_0x64);
    if (field_0x38)
        ((UnknownQuarryOverlayHooks*)field_0x38)->UnknownFunction48af20((UnknownQuarryRace*)field_0x34);
    g_MemTagStack->Pop(0);

    if (!g_UnknownGlobal56e26c->mode.field_0x6ac)
        UnknownFunction468dd0("InstrumentOverlay");
    if (!g_UnknownGlobal56e26c->mode.field_0xa5c)
        UnknownFunction468dd0("SkyCube");
    if (!g_UnknownGlobal56e26c->mode.field_0xa54) {
        UnknownFunction468dd0("ParticleManager");
        UnknownFunction468dd0("DirtParticleEmitter");
    }
    if (!g_UnknownGlobal56e26c->mode.field_0xa50) {
        UnknownFunction468dd0("ProjectedShadow");
        UnknownFunction468dd0("TerrainShadow");
        UnknownFunction468dd0("D3DIMSoultreeShadow");
    }
    ((RenderTarget*)field_0x18)->field_0x34 = !(field_0x30 && g_UnknownGlobal56e26c->mode.field_0xa5c);
    field_0x50 = (void*)1;
    if (g_UnknownGlobal56e26c->field_0x2d5_bit2) {
        g_MemTagStack->Push("TextureCache");
        g_UnknownGlobal56e26c->field_0x3c->UnknownFunction5113d0();
    }
    g_MemTagStack->Pop(0);
    field_0x2c->UnknownFunction4eff30(g_UnknownGlobal56e26c->mode.field_0x195c[2]);
    field_0x34->UnknownFunction423790(g_UnknownGlobal56e26c->mode.field_0x195c[2]);
    if (progress)
        progress(0);
    UnknownQuarryRaceObject* last = new(__FILE__, 1251) UnknownQuarryRaceObject(1);
    last->UnknownVirtualSlot8(field_0x18);
    UnknownFunction469190(last, -1);
    last->UnknownFunction5046e0();
    return 1;
}

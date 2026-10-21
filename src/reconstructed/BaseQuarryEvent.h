#pragma once

#include "GameObject.h"

class AuralScape;
class ChatOverlay;
class EcoSystem;
class InstrumentOverlay;
class LightEmitter;
class LightManager;
class RadarOverlay;
class RenderTarget;
class Scene;
class StatsOverlay;
class TextQueueOverlay;
class UnknownTerrain;
class UnknownTextureStream;
class VisualCue;
class Fog;
struct UnknownKrustyBikeView;
struct UnknownQuarryCamera;

// Progress callback the events' initialisers take (EventManager passes the
// cdecl 0x0045cb20).
typedef void (*UnknownProgressCallback)(int* step);

// RTTI: BaseQuarryEvent : GameObject (vtable 0x0055766c; 0xa4 bytes, the
// size EventManager 0x0045cb70 allocates). QuarryStuntEvent.cpp
// (D:\aardvark\VC\krusty2\QuarryStuntEvent.cpp, literal 0x00572154) holds
// its code, 0x004de2a0..0x004e1fa7; reconstructed in QuarryStuntEvent.cpp.
// TrackGame keeps these race-mode objects at +0x558..+0x568 (declared there
// as TrackGameViewOwner). Member names are provisional; 0x004de3b0 is the
// retail "BaseQuarryEvent::Create" (its MemTag strings).
class BaseQuarryEvent : public GameObject {
public:
    explicit BaseQuarryEvent(int flags);      // 0x004de2a0
    virtual ~BaseQuarryEvent();               // 0x004e1f10 (deleting wrapper 0x004de390)
    virtual int Tick(float frameTime);                 // 0x004de410
    virtual int PrepareGeometry();            // 0x004e14a0: the "Atmosphere" debug page
    // Slot 14, 0x004de400: `jmp GameObject slot 14` (folded with the same
    // body of other classes).
    virtual int UnknownVirtualSlot14();
    virtual int OnActivate();                 // 0x00499af0 (folded with KrustyUI's)
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);       // 0x004e0ce0
    virtual int OnNetworkMessage(int type, void* data, int from, int to, int flags);       // 0x004e1cc0
    // Slots 27-35 are introduced here (NationalRace overrides 27, 29 and 30).
    virtual int UnknownVirtualSlot27(int value); // 0x004e06a0: creates the visual cue
    virtual void UnknownVirtualSlot28();      // 0x00464e90 (shared empty body)
    // Slot 29 (0x004e07c0): creates the instrument, chat, radar, stats and
    // text-queue overlays; LoadEventObjects calls it (0x004df036).
    virtual void CreateOverlays();
    // Slot 30 (0x004e0be0): appends those overlays as children.
    virtual void AttachOverlays();
    virtual int UnknownVirtualSlot31();       // 0x00467ae0 (shared `return 1` body)
    virtual void UnknownVirtualSlot32(int value); // 0x00464e80 (shared empty body)
    virtual void UnknownVirtualSlot33(int value); // 0x00464e80
    virtual void UnknownVirtualSlot34(int a, int b, int c); // 0x004de580 (shared empty body)
    virtual int UnknownVirtualSlot35();       // 0x00467ae0

    // 0x004de3b0: GameObject's slot 8, then loads the event; on failure it
    // releases itself (slot 2) and returns 0, else returns this.
    BaseQuarryEvent* Create(RenderTarget* target, UnknownProgressCallback progress);
    // 0x004de590 (about 7.9 KB, Create's only callee that can fail): picks
    // texture detail from the video memory, then creates the audio, camera,
    // scene, quadtree, fog and sky, terrain, shadows, ecosystem, race and
    // collision links (MemTagStack tags "Audio" .. "Collision"), calling
    // `progress` between stages; a near miss in
    // samples/game/QuarryStuntEventLoaderNearMiss.cpp.
    int LoadEventObjects(UnknownProgressCallback progress);
    // 0x004e04a0, 0x004e04e0, 0x004e04f0, 0x004e0560: memory estimates
    // (bytes) 0x004de590 sums and compares against the free video memory:
    // the terrain by its width, the cube, the model textures by the
    // per-width counts from Scene::CountTextures, and the race's bikes.
    int EstimateTerrainBytes(int width);
    int EstimateCubeBytes();
    int EstimateModelTextureBytes(int* counts);
    int EstimateRaceBytes();
    // 0x004e0c30: shows "<string id> <on/off>" (strings 0x1407/0x1408) in
    // the text queue.
    void ShowOnOffMessage(int id, int on);
    void ResetClock();             // 0x004e1f00: clears the clock

    Scene* eventScene;
    void* skyCube;                         // SkyCube
    UnknownKrustyBikeView* raceView;
    VisualCue* visualCue;
    UnknownQuarryCamera* raceCamera;
    UnknownTerrain* eventTerrain;               // Terrain (src/krusty2/broadphase/Terrain.h)
    UnknownTextureStream* terrainStream;        // +0x44 opened on the env's terrain file and handed to the terrain loader; the destructor deletes it
    EcoSystem* ecoSystem;
    void* field_0x4c;
    int loaded;                                 // +0x50 0 in the constructor, 1 when LoadEventObjects completes (no reader found here)
    void* terrainShadow;                         // TerrainShadow
    void* projectedShadow;                         // ProjectedShadow
    StatsOverlay* statsOverlay;
    RadarOverlay* radarOverlay;
    ChatOverlay* chatOverlay;
    InstrumentOverlay* instrumentOverlay;
    TextQueueOverlay* textQueue;
    float clockMinutes;                         // race clock, minutes
    float clockSeconds;                         // race clock, seconds
    void* particleManager;                         // ParticleManager
    LightManager* lightManager;
    LightEmitter* sunLight;                 // sun
    LightEmitter* ambientLight;                 // ambient
    Fog* eventFog;
    unsigned char field_0x8c[0x94 - 0x8c];
    GameObject* opaqueTextureGroup;             // +0x94 textureManager CreateGroup(surface format) when texturesCached; the texture contexts' opaqueGroup
    GameObject* alphaTextureGroup;              // +0x98 CreateGroup(0x115c); the texture contexts' alphaGroup
    AuralScape* auralScape;
    int auralScapeListener;                           // AuralScape listener
};

#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"
#include "Fog.h"

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
struct UnknownKrustyBikeView;

// Progress callback the events' initialisers take (EventManager passes the
// cdecl 0x0045cb20).
typedef void (*UnknownProgressCallback)(int* step);

// The race camera at BaseQuarryEvent+0x3c (a KrustyBikeCamera: 0x004de590
// constructs it with 0x00497cb0, which writes vtable 0x00554754) as this file
// reads it: the listener's position and axes, and the followed bike
// (velocity at +0x64). These are Camera's protected +0x170..+0x188 and
// BikeCamera's +0x3b0.
struct UnknownQuarryCameraSubject {
    unsigned char field_0x00[0x64];
    Vector3 field_0x64;
};

struct UnknownQuarryCamera {
    unsigned char field_0x000[0x170];
    Vector3 field_0x170;                      // position
    Vector3 field_0x17c;                      // forward
    Vector3 field_0x188;                      // up
    unsigned char field_0x194[0x3b0 - 0x194];
    UnknownQuarryCameraSubject* field_0x3b0;  // followed bike
};

// RTTI: VisualCue : GameObject (vtable 0x00554320; Krusty3DObjects.cpp).
// 0xfc bytes (the allocation at 0x004e06cd). Only the members
// QuarryStuntEvent.cpp uses are declared; names are provisional.
class VisualCue : public GameObject {
public:
    explicit VisualCue(int flags);            // 0x0048ad60
    // 0x0048adf0 (ret 0x34): loads the cue; returns this, or 0.
    VisualCue* UnknownFunction48adf0(void* target, LightManager* lights, int value, UnknownTerrain* terrain,
                                     Vector3 offset, UnknownKrustyBikeView* view, int count,
                                     UnknownQuarryCamera* camera, float a9, float a10, float a11);
    // 0x0048bc30: the next cue index when it changed, else -1.
    int UnknownFunction48bc30();
    // 0x0048bc80: 1 while a cue is active and more than one exists.
    int UnknownFunction48bc80();

    unsigned char field_0x2c[0x48 - 0x2c];
    float field_0x48;                         // screen x, fraction of the width
    float field_0x4c;                         // screen y, fraction of the height
    unsigned char field_0x50[0xc8 - 0x50];
    int field_0xc8;
    unsigned char field_0xcc[0xfc - 0xcc];
};

// RTTI: EcoSystem (vtable 0x00552508); constructed by 0x004de590 with
// 0x00457250. Only the methods this file calls.
class EcoSystem {
public:
    void UnknownFunction4594c0(int level);    // 0x004594c0: detail level
    void UnknownFunction45a9a0();             // 0x0045a9a0: lighting changed
};

// Fog (vtable 0x005526dc, 0x5c bytes; constructed by 0x004de590 with
// 0x00462620) is declared in Fog.h.

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
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004de410
    virtual int UnknownVirtualSlot12();       // 0x004e14a0: the "Atmosphere" debug page
    // Slot 14, 0x004de400: `jmp GameObject slot 14` (folded with the same
    // body of other classes).
    virtual int UnknownVirtualSlot14();
    virtual int UnknownVirtualSlot18();       // 0x00499af0 (folded with KrustyUI's)
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004e0ce0
    virtual int UnknownVirtualSlot24(int type, void* data, int from, int to, int flags);   // 0x004e1cc0
    // Slots 27-35 are introduced here (NationalRace overrides 27, 29 and 30).
    virtual int UnknownVirtualSlot27(int value); // 0x004e06a0: creates the visual cue
    virtual void UnknownVirtualSlot28();      // 0x00464e90 (shared empty body)
    virtual void UnknownVirtualSlot29();      // 0x004e07c0: creates the overlays
    virtual void UnknownVirtualSlot30();      // 0x004e0be0: adds the overlays as children
    virtual int UnknownVirtualSlot31();       // 0x00467ae0 (shared `return 1` body)
    virtual void UnknownVirtualSlot32(int value); // 0x00464e80 (shared empty body)
    virtual void UnknownVirtualSlot33(int value); // 0x00464e80
    virtual void UnknownVirtualSlot34(int a, int b, int c); // 0x004de580 (shared empty body)
    virtual int UnknownVirtualSlot35();       // 0x00467ae0

    // 0x004de3b0: GameObject's slot 8, then loads the event; on failure it
    // releases itself (slot 2) and returns 0, else returns this.
    BaseQuarryEvent* Create(RenderTarget* target, UnknownProgressCallback progress);
    // 0x004de590 (about 7.9 KB): loads the scene, terrain, lights, sounds
    // and race objects; not reconstructed.
    int UnknownFunction4de590(UnknownProgressCallback progress);
    // 0x004e04a0, 0x004e04e0, 0x004e04f0, 0x004e0560: memory estimates
    // (bytes) 0x004de590 sums.
    int UnknownFunction4e04a0(int value);
    int UnknownFunction4e04e0();
    int UnknownFunction4e04f0(int* counts);
    int UnknownFunction4e0560();
    // 0x004e0c30: shows "<string id> <on/off>" (strings 0x1407/0x1408) in
    // the text queue.
    void ShowOnOffMessage(int id, int on);
    void UnknownFunction4e1f00();             // 0x004e1f00: clears the clock

    Scene* eventScene;
    void* skyCube;                         // SkyCube
    UnknownKrustyBikeView* raceView;
    VisualCue* visualCue;
    UnknownQuarryCamera* raceCamera;
    UnknownTerrain* eventTerrain;               // Terrain (src/krusty2/broadphase/Terrain.h)
    UnknownTextureStream* field_0x44;
    EcoSystem* ecoSystem;
    void* field_0x4c;
    void* field_0x50;
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
    GameObject* field_0x94;
    GameObject* field_0x98;
    AuralScape* auralScape;
    int auralScapeListener;                           // AuralScape listener
};

// RTTI: NationalRace : BaseQuarryEvent (vtable 0x00555354; 0xb0 bytes).
// Its code is NationalRace.cpp (D:\aardvark\VC\krusty2\NationalRace.cpp,
// literal 0x0056e338), 0x004aa7f0..0x004aaaf2, reconstructed in
// src/reconstructed/NationalRace.cpp. The destructor is compiler-generated.
class RunwayLights;
class DropTextOverlay;

class NationalRace : public BaseQuarryEvent {
public:
    explicit NationalRace(int flags);         // 0x004aa7f0
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004aaa70: shows the finish text once
    virtual int UnknownVirtualSlot27(int value); // 0x004aa890: the visual cue, then the runway lights
    virtual void UnknownVirtualSlot29();      // 0x004aa970: the overlays, then the drop text
    virtual void UnknownVirtualSlot30();      // 0x004aaa50: adds the drop text as a child
    // 0x004aa850: the base initialiser, then 0x0048ad50 on +0xa4.
    NationalRace* Create(RenderTarget* target, UnknownProgressCallback progress);

    RunwayLights* runwayLights;
    DropTextOverlay* finishText;              // the finish text (string 0x913)
    int finishTextShown;                           // finish text shown
};

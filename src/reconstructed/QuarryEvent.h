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

// The race camera at BaseQuarryEvent+0x3c (a KrustyBikeCamera: 0x004de590
// constructs it with 0x00497cb0, which writes vtable 0x00554754) as this file
// reads it: the listener's position and axes, and the followed bike
// (velocity at +0x64). These are Camera's protected +0x170..+0x188 and
// BikeCamera's +0x3b0.
struct UnknownQuarryCameraSubject {
    unsigned char field_0x00[0x64];
    Vector3 velocity;                         // +0x64 krusty2 SoultreePhysicsBaseObject::velocity
};

struct UnknownQuarryCamera {
    unsigned char field_0x000[0x170];
    Vector3 eyePosition;                      // +0x170 position
    Vector3 viewDirection;                    // +0x17c forward
    Vector3 upDirection;                      // +0x188 up
    unsigned char field_0x194[0x3b0 - 0x194];
    UnknownQuarryCameraSubject* followedBike; // +0x3b0 followed bike
};

// RTTI: VisualCue : GameObject (vtable 0x00554320; Krusty3DObjects.cpp).
// 0xfc bytes (the allocation at 0x004e06cd). Only the members
// QuarryStuntEvent.cpp uses are declared; names are provisional.
class VisualCue : public GameObject {
public:
    explicit VisualCue(int flags);            // 0x0048ad60
    // 0x0048adf0 (ret 0x34): loads the cue; returns this, or 0.
    VisualCue* LoadVisualCue(void* target, LightManager* lights, int value, UnknownTerrain* terrain,
                                     Vector3 offset, UnknownKrustyBikeView* view, int count,
                                     UnknownQuarryCamera* camera, float a9, float a10, float a11);
    // 0x0048bc30: the next cue index when it changed, else -1.
    int NextCueIndex();
    // 0x0048bc80: 1 while a cue is active and more than one exists.
    int CanCycleCues();

    unsigned char field_0x2c[0x48 - 0x2c];
    float screenX;                            // +0x48 ArcadeObject::Create's screenXFraction: fraction of the width
    float screenY;                            // +0x4c ArcadeObject::Create's screenYFraction: fraction of the height
    unsigned char field_0x50[0xc8 - 0x50];
    int skippedRacer;                         // +0xc8 Tick copies RadarOverlay::followedIndex; NextCueIndex steps past it (Krusty3DObjects.h)
    unsigned char field_0xcc[0xfc - 0xcc];
};

// RTTI: EcoSystem (vtable 0x00552508); constructed by 0x004de590 with
// 0x00457250. Only the methods this file calls.
class EcoSystem {
public:
    void SetDetailLevel(int level);    // 0x004594c0: detail level
    void LightingChanged();             // 0x0045a9a0: lighting changed
};

// Fog (vtable 0x005526dc, 0x5c bytes; constructed by 0x004de590 with
// 0x00462620) is declared in Fog.h.

#include "BaseQuarryEvent.h"

// RTTI: NationalRace : BaseQuarryEvent (vtable 0x00555354; 0xb0 bytes).
// Its code is NationalRace.cpp (D:\aardvark\VC\krusty2\NationalRace.cpp,
// literal 0x0056e338), 0x004aa7f0..0x004aaaf2, reconstructed in
// src/reconstructed/NationalRace.cpp. The destructor is compiler-generated.
class RunwayLights;
class DropTextOverlay;

class NationalRace : public BaseQuarryEvent {
public:
    explicit NationalRace(int flags);         // 0x004aa7f0
    virtual int Tick(float frameTime);                 // 0x004aaa70: shows the finish text once
    virtual int UnknownVirtualSlot27(int value); // 0x004aa890: the visual cue, then the runway lights
    virtual void CreateOverlays();            // 0x004aa970: the overlays, then the drop text
    virtual void AttachOverlays();            // 0x004aaa50: the overlays, then the drop text as a child
    // 0x004aa850: the base initialiser, then 0x0048ad50 on +0xa4.
    NationalRace* Create(RenderTarget* target, UnknownProgressCallback progress);

    RunwayLights* runwayLights;
    DropTextOverlay* finishText;              // the finish text (string 0x913)
    int finishTextShown;                           // finish text shown
};

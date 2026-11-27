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
struct TextureContext;
class RadarOverlay;
class RenderTarget;
class Scene;
class StatsOverlay;
class TextureMapManager;
class TextQueueOverlay;
class Terrain;
class FileStream;
struct EventBikeRaceView;

// RTTI: EcoSystem (vtable 0x00552508, 0x5c4 bytes); constructed by 0x004de590
// with 0x00457250.
#include "EcoSystem.h"

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
    virtual int Create3DObjects(const TextureContext* textures); // 0x004aa890: the visual cue, then the runway lights
    virtual void CreateOverlays();            // 0x004aa970: the overlays, then the drop text
    virtual void AttachOverlays();            // 0x004aaa50: the overlays, then the drop text as a child
    // 0x004aa850: the base initialiser, then 0x0048ad50 on +0xa4.
    NationalRace* Create(RenderTarget* target, LoadProgressFn progress);

    RunwayLights* runwayLights;
    DropTextOverlay* finishText;              // the finish text (string 0x913)
    int finishTextShown;                           // finish text shown
};

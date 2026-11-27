#include <stdio.h>

#include "QuarryEvent.h"

#include "DebugAlloc.h"
#include "RaceView.h"
#include "RenderTarget.h"
#include "TrackGame.h"
#include "EventManager.h"
#include "../krusty2/vehicle/KrustyBikeCore.h"
#include "TrackOverlay.h"
#include "Krusty3DObjects.h"   // RunwayLights

// Reconstruction of D:\aardvark\VC\krusty2\NationalRace.cpp (literal
// __FILE__ at 0x0056e338, used by slots 27 and 29): NationalRace,
// 0x004aa7f0..0x004aaaf2, between MSZoneInterface.cpp (last xref
// 0x004aa64d) and Net.cpp (first xref 0x004aae60). RTTI confirms
// NationalRace : BaseQuarryEvent (vtable 0x00555354) and the overridden
// slots 0, 10, 27, 29 and 30. Member and function names are provisional.

// 0x004aa7f0
NationalRace::NationalRace(int flags) : BaseQuarryEvent(flags) {
    runwayLights = 0;
    finishTextShown = 0;
    finishText = 0;
}

// 0x004aa850
NationalRace* NationalRace::Create(RenderTarget* target, LoadProgressFn progress) {
    if (!BaseQuarryEvent::Create(target, progress))
        return 0;
    if (runwayLights)
        runwayLights->SetRacer((RunwayKrustyBikeView*)raceView->localRacer);
    return this;
}

// 0x004aa890
int NationalRace::Create3DObjects(const TextureContext* textures) {
    char message[0x104];

    if (!BaseQuarryEvent::Create3DObjects(textures))
        return 0;
    runwayLights = (new (__FILE__, 53) RunwayLights(g_TrackGame->mode.visualCuesOn))
                     ->SetupLights(renderContext, lightManager, textures, eventTerrain);
    if (!AppendChild(runwayLights, -1)) {
        sprintf(message, "Runway Lights not loaded.\n");
        return 0;
    }
    return 1;
}

// 0x004aa970
void NationalRace::CreateOverlays() {
    char name[128];
    char message[128];

    BaseQuarryEvent::CreateOverlays();
    g_TrackGame->LoadResourceString(0x913, name, sizeof(name));
    finishText = new (__FILE__, 71) DropTextOverlay(1);
    finishText = finishText->Setup(renderContext, g_TrackGame->textureManager, 3.0f, 0, sizeof(name), name);
    if (!finishText)
        sprintf(message, "Drop Text Overlay not created.\n");
}

// 0x004aaa50
void NationalRace::AttachOverlays() {
    BaseQuarryEvent::AttachOverlays();
    AppendChild(finishText, -1);
}

// 0x004aaa70
int NationalRace::Tick(float frameTime) {
    BaseQuarryEvent::Tick(frameTime);
    if (g_TrackGame->mode.raceSettings.raceMode && g_TrackGame->mode.raceSettings.raceMode != 4 &&
        !g_TrackGame->uiInteractionBlocked && !finishTextShown) {
        if (raceView->localRacer->lapCount + 1 == g_TrackGame->mode.raceSettings.lapCount &&
            raceView->isRacing) {
            if (finishText)
                finishText->ReshowText();
            finishTextShown = 1;
        }
    }
    return 1;
}

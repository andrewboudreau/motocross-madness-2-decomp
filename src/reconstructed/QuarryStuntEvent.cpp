#include "../krusty2/broadphase/Terrain.h"
#include <stdio.h>
#include <string.h>

#include "QuarryEvent.h"

#include "AuralScape.h"
#include "ControlInterface.h"
#include "DebugAlloc.h"
#include "DebugOverlay.h"
#include "LightEmitter.h"
#include "MemTag.h"
#include "RaceView.h"
#include "RenderTarget.h"
#include "SceneManager.h"
#include "TextureMap.h"
#include "TrackGame.h"
#include "EventManager.h"
#include "TrackOverlay.h"
#include "UnknownResourceManager.h"

// Reconstruction of D:\aardvark\VC\krusty2\QuarryStuntEvent.cpp (literal
// __FILE__ at 0x00572154): BaseQuarryEvent, 0x004de2a0..0x004e1fa7, between
// Quantize.cpp (ends 0x004de29f) and racesnd.cpp (RaceSound's constructor
// 0x004e1fb0). Names are provisional. Not here: the loader 0x004de590
// (about 7.9 KB, not reconstructed) and the near miss slot 12 (0x004e14a0),
// kept in samples/game/QuarryStuntEventNearMisses.cpp.

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
    terrainStream = new(__FILE__, 92) UnknownTextureStream(g_ResourceManager);
    if (!terrainStream)
        return;
}

// 0x004de3b0
BaseQuarryEvent* BaseQuarryEvent::Create(RenderTarget* target, UnknownProgressCallback progress) {
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
int BaseQuarryEvent::UnknownVirtualSlot14() {
    return GameObject::UnknownVirtualSlot14();
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
            statsOverlay->UnknownFunction464e80(index);
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

// KrustyUI's manufacturers (+0x48) and riders (+0x58), 0xc8-byte records,
// and its bikes (+0x50, 0x94 bytes, the manufacturer index first).
struct UnknownQuarryBikeModel {
    unsigned char field_0x00[0xc4];
    int availability;                         // +0xc4 KrustyUI.cpp's UnknownKrustyUIModelEntry: "Availability"
};

struct UnknownQuarryBikeChoice {
    int manufacturer;                         // +0x00 KrustyUI.cpp's UnknownKrustyUIBikeEntry: index into +0x48
    unsigned char field_0x04[0x94 - 0x04];
};

// 0x004e0560: the race memory estimate (counts of eligible bikes and
// opponents, capped by the circuit's +0x460). The sum is a local and the
// field_0x560 constant an if/else assignment: VC6 still emits the branchless
// neg/sbb/and/add last, and with the ternary inside the return it keeps
// g_TrackGame in a register across the first loop and spills a count (22/331).
int BaseQuarryEvent::EstimateRaceBytes() {
    int players = 0;
    int opponents = 0;
    if (g_TrackGame->InProCircuitCareer()) {
        int i;
        for (i = 0; i < g_TrackGame->ui->bikeCount; i++) {
            if (((UnknownQuarryBikeModel*)g_TrackGame->ui->manufacturers)[((UnknownQuarryBikeChoice*)g_TrackGame->ui->bikes)[i].manufacturer].availability <= g_TrackGame->proCircuit->maxModelKind)
                players++;
        }
        for (i = 0; i < g_TrackGame->ui->riderCount; i++) {
            if (((UnknownQuarryBikeModel*)g_TrackGame->ui->riders)[i].availability <= g_TrackGame->proCircuit->maxModelKind)
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
int BaseQuarryEvent::UnknownVirtualSlot27(int value) {
    char message[388];
    visualCue = (new(__FILE__, 1369) VisualCue(g_TrackGame->mode.visualCuesOn))
                     ->LoadVisualCue(renderContext, lightManager, value, eventTerrain, Vector3(0.0f, 0.0f, 0.0f),
                                             raceView, 45, raceCamera, 3.0f, 0.1f, 0.85f);
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
    UnknownOverlayRect screen;
    UnknownOverlayRect cue;
    UnknownOverlayRect text;
    int width;
    int height;

    instrumentOverlay = new(__FILE__, 1412) InstrumentOverlay(g_TrackGame->mode.instrumentOverlayOn);
    instrumentOverlay = instrumentOverlay->Setup((RenderTarget*)renderContext, g_TrackGame->textureManager,
                                                   (UnknownInstrumentSource*)raceCamera);
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
        UnknownMessage line(name, 3.25f);
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
    UnknownMessage message(buffer, 1.5f);
    textQueue->ReplaceHeadMessage(&message);
}

// 0x004e0ce0
int BaseQuarryEvent::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
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
        raceView->SetDetailLevel(level);
        if (textQueue) {
            char text[0x80];
            char buffer[0x100];
            g_TrackGame->LoadResourceString(0x14c4, text, 0x80);
            sprintf(buffer, "%s = %d", text, level + 1);
            UnknownMessage message(buffer, 1.5f);
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
        raceView->SetDetailLevel(level);
        if (textQueue) {
            char text[0x80];
            char buffer[0x100];
            g_TrackGame->LoadResourceString(0x14c4, text, 0x80);
            sprintf(buffer, "%s = %d", text, level + 1);
            UnknownMessage message(buffer, 1.5f);
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
struct UnknownQuarryLeaveMessage {
    int field_0x00;
    int field_0x04;
    int playerId;                             // +0x08 matched against racers' networkPlayerId
};

// 0x004e1cc0
int BaseQuarryEvent::OnNetworkMessage(int type, void* data, int from, int to, int flags) {
    if (GameObject::OnNetworkMessage(type, data, from, to, flags))
        return 1;
    if (g_TrackGame->uiInteractionBlocked)
        return 1;
    if (type == 5) {
        UnknownQuarryLeaveMessage* leave = (UnknownQuarryLeaveMessage*)data;
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
        UnknownMessage message(buffer, 3.25f);
        textQueue->AppendMessage(&message);
        raceView->HandlePlayerLeft(leave->playerId);
        if (g_TrackGame->mode.raceSettings.raceMode == 2)
            g_TrackGame->eventManager->RemovePlayer(leave->playerId);
    } else if (from && type == 0x89) {
        char text[0x80];
        char buffer[0x80];
        g_TrackGame->LoadResourceString(0xbd5, text, 0x80);
        sprintf(buffer, "%s %s", (char*)data + 4, text);
        UnknownMessage message(buffer, 3.25f);
        textQueue->AppendMessage(&message);
        raceView->HandlePlayerLeft(from);
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
int BaseQuarryEvent::UnknownVirtualSlot31() {
    return 1;
}

// 0x00464e80
void BaseQuarryEvent::UnknownVirtualSlot32(int value) {
}

// 0x00464e80
void BaseQuarryEvent::UnknownVirtualSlot33(int value) {
}

// 0x00467ae0
int BaseQuarryEvent::UnknownVirtualSlot35() {
    return 1;
}

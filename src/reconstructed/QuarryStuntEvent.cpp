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
#include "TrackOverlay.h"
#include "UnknownResourceManager.h"

// Reconstruction of D:\aardvark\VC\krusty2\QuarryStuntEvent.cpp (literal
// __FILE__ at 0x00572154): BaseQuarryEvent, 0x004de2a0..0x004e1fa7, between
// Quantize.cpp (ends 0x004de29f) and racesnd.cpp (RaceSound's constructor
// 0x004e1fb0). Names are provisional. Not here: the loader 0x004de590
// (about 7.9 KB, not reconstructed) and the near misses 0x004e0560 and slot
// 12 (0x004e14a0), kept in samples/game/QuarryStuntEventNearMisses.cpp.

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
    field_0x50 = 0;
    terrainShadow = 0;
    projectedShadow = 0;
    clockMinutes = 0;
    clockSeconds = 0;
    particleManager = 0;
    lightManager = 0;
    visualCue = 0;
    field_0x94 = 0;
    field_0x98 = 0;
    auralScape = 0;
    auralScapeListener = 0;
    radarOverlay = 0;
    statsOverlay = 0;
    chatOverlay = 0;
    instrumentOverlay = 0;
    textQueue = 0;
    eventFog = 0;
    field_0x44 = new(__FILE__, 92) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (!field_0x44)
        return;
}

// 0x004de3b0
BaseQuarryEvent* BaseQuarryEvent::Create(RenderTarget* target, UnknownProgressCallback progress) {
    GameObject::UnknownVirtualSlot8(target);
    g_MemTagStack->UnknownFunction4a2bc0("entering BaseQuarryEvent::Create");
    if (!UnknownFunction4de590(progress)) {
        Release();
        return 0;
    }
    g_MemTagStack->UnknownFunction4a2bc0("exiting BaseQuarryEvent::Create");
    return this;
}

// 0x004de400
int BaseQuarryEvent::UnknownVirtualSlot14() {
    return GameObject::UnknownVirtualSlot14();
}

// 0x004de410
int BaseQuarryEvent::UnknownVirtualSlot10(float frameTime) {
    GameObject::UnknownVirtualSlot10(frameTime);
    if (auralScape)
        auralScape->UnknownFunction4030a0(auralScapeListener, &raceCamera->field_0x170, &raceCamera->field_0x17c,
                                          &raceCamera->field_0x188, &raceCamera->field_0x3b0->field_0x64);
    if (g_TrackGame->uiInteractionBlocked)
        return 1;
    if (radarOverlay && visualCue && statsOverlay) {
        visualCue->field_0xc8 = radarOverlay->field_0x170;
        int index = visualCue->UnknownFunction48bc30();
        if (index != -1) {
            radarOverlay->field_0x16c = index;
            statsOverlay->UnknownFunction464e80(index);
        }
        radarOverlay->field_0x174 = visualCue->UnknownFunction48bc80();
        chatOverlay->UnknownFunction51d980(visualCue->UnknownFunction48bc80());
    }
    if (!g_TrackGame->eventManager->field_0x34 && !raceView->field_0x18e && raceView->field_0x18a) {
        clockSeconds += g_TrackGame->frameTime;
        while (clockSeconds >= 60.0f) {
            clockMinutes += 1.0f;
            clockSeconds -= 60.0f;
            if (clockSeconds < 0.0f)
                clockSeconds = 0.0f;
        }
    } else if (raceView->field_0x18e) {
        UnknownFunction4e1f00();
    }
    SoundSystem()->UnknownFunction4beb80();
    return 1;
}

// 0x004de580
void BaseQuarryEvent::UnknownVirtualSlot34(int a, int b, int c) {
}

// 0x004e04a0
int BaseQuarryEvent::UnknownFunction4e04a0(int value) {
    int size;
    if (value == 3)
        size = 35;
    else
        size = value == 5 ? 55 : 106;
    return size * 0x80000 / 3;
}

// 0x004e04e0
int BaseQuarryEvent::UnknownFunction4e04e0() {
    return 0x200000;
}

// 0x004e04f0
int BaseQuarryEvent::UnknownFunction4e04f0(int* counts) {
    return counts[0] * 0x80000 / 3 + counts[1] * 0x20000 / 3 + counts[2] * 0x8000 / 3 + counts[3] * 0x2000 / 3;
}

// 0x004e06a0
int BaseQuarryEvent::UnknownVirtualSlot27(int value) {
    char message[388];
    visualCue = (new(__FILE__, 1369) VisualCue(g_TrackGame->mode.field_0x6a8))
                     ->UnknownFunction48adf0(field_0x18, lightManager, value, eventTerrain, Vector3(0.0f, 0.0f, 0.0f),
                                             raceView, 45, raceCamera, 3.0f, 0.1f, 0.85f);
    if (!AppendChild(visualCue, -1)) {
        sprintf(message, "Visual Cue not loaded.\n");
        return 0;
    }
    return 1;
}

// 0x004e07c0
void BaseQuarryEvent::UnknownVirtualSlot29() {
    char message[388];
    char name[0x40];
    UnknownOverlayRect screen;
    UnknownOverlayRect cue;
    UnknownOverlayRect text;
    int width;
    int height;

    instrumentOverlay = new(__FILE__, 1412) InstrumentOverlay(g_TrackGame->mode.field_0x6ac);
    instrumentOverlay = instrumentOverlay->UnknownFunction518770((RenderTarget*)field_0x18, g_TrackGame->field_0x3c,
                                                   (UnknownInstrumentSource*)raceCamera);
    if (!instrumentOverlay)
        sprintf(message, "Instrument Overlay not created.\n");

    // One load each of the height and width feeds two copies: the cue's size
    // uses height/width, its position and the overlays the screen rectangle.
    height = ((RenderTarget*)field_0x18)->field_0x10;
    screen.bottom = ((RenderTarget*)field_0x18)->field_0x10;
    width = ((RenderTarget*)field_0x18)->field_0x0c;
    screen.right = ((RenderTarget*)field_0x18)->field_0x0c;
    screen.top = 0;
    screen.left = 0;
    if (visualCue) {
        cue.top = (int)((visualCue->field_0x4c + 0.06f) * screen.bottom);
        cue.left = (int)((visualCue->field_0x48 - 0.04f) * screen.right);
    } else {
        cue.top = 0;
        cue.left = 0;
    }
    cue.bottom = cue.top - (int)(height * -0.020833334f);
    cue.right = cue.left - (int)(width * -0.046875f);

    chatOverlay = new(__FILE__, 1433) ChatOverlay(1);
    chatOverlay = chatOverlay->UnknownFunction51cf80((RenderTarget*)field_0x18, g_TrackGame->field_0x3c, raceCamera, screen, cue);
    if (!chatOverlay)
        sprintf(message, "Chat Overlay not created.\n");
    if (chatOverlay && g_TrackGame->mode.field_0x6b4 == 1)
        chatOverlay->UnknownFunction51dd10();

    radarOverlay = new(__FILE__, 1444) RadarOverlay(1);
    radarOverlay = radarOverlay->UnknownFunction51b840((RenderTarget*)field_0x18, g_TrackGame->field_0x3c, (int)raceCamera, screen);
    if (!radarOverlay)
        sprintf(message, "Radar Overlay not created.\n");

    statsOverlay = new(__FILE__, 1451) StatsOverlay(1);
    statsOverlay = statsOverlay->UnknownFunction5194b0((RenderTarget*)field_0x18, g_TrackGame->field_0x3c, raceCamera, screen);
    if (!statsOverlay)
        sprintf(message, "Stats Overlay not created.\n");

    // Retail reads the height and loads 10 and 256 before the allocation and
    // computes the top after it.
    text.bottom = ((RenderTarget*)field_0x18)->field_0x10;
    text.left = 10;
    text.right = 256;
    textQueue = new(__FILE__, 1463) TextQueueOverlay(0);
    text.top = text.bottom - 20;
    textQueue = textQueue->UnknownFunction51b320(field_0x18, text);
    if (!textQueue)
        sprintf(message, "TextQueueOverlay not created.\n");

    if (textQueue) {
        if (g_TrackGame->mode.field_0x27f8.field_0x04 == 1 || g_TrackGame->mode.field_0x27f8.field_0x04 == 5)
            g_TrackGame->sceneObject->UnknownFunction4ea390(
                name, g_TrackGame->sceneObject->field_0x24c, g_TrackGame->mode.field_0x27f8.field_0x34);
        else
            g_TrackGame->sceneObject->UnknownFunction4ea390(
                name, g_TrackGame->sceneObject->field_0x24c, 0);
        UnknownMessage line(name, 3.25f);
        textQueue->UnknownFunction51b540(&line);
    }
}

// 0x004e0be0
void BaseQuarryEvent::UnknownVirtualSlot30() {
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
    textQueue->UnknownFunction51b540(&message);
}

// 0x004e0ce0
int BaseQuarryEvent::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (GameObject::UnknownVirtualSlot23(event, entry))
        return 1;
    if (UnknownFunction43caa0(0x31, 0, event, 0xc)) {
        if (g_TrackGame->mode.field_0x6bc && g_TrackGame->mode.field_0x6c0) {
            g_TrackGame->mode.field_0x6c0 = 0;
            ShowOnOffMessage(0x1412, 1);
        } else if (g_TrackGame->mode.field_0x6bc) {
            g_TrackGame->mode.field_0x6bc = 0;
            g_TrackGame->mode.field_0x6c0 = 1;
            ShowOnOffMessage(0x1413, 1);
        } else if (g_TrackGame->mode.field_0x6c0) {
            g_TrackGame->mode.field_0x6c0 = 0;
            ShowOnOffMessage(0x1414, 0);
        } else {
            g_TrackGame->mode.field_0x6bc = 1;
            g_TrackGame->mode.field_0x6c0 = 1;
            ShowOnOffMessage(0x1414, 1);
        }
        chatOverlay->UnknownFunction51e910(-1);
        return 1;
    }
    if (UnknownFunction43caa0(0x31, 0, event, 0x80000000) && statsOverlay) {
        chatOverlay->UnknownFunction51e7c0();
        return 1;
    }
    if (UnknownFunction43caa0(0x3e, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.field_0x6a8) {
            UnknownFunction468dd0("RunwayLights");
            UnknownFunction468dd0("VisualCue");
        } else {
            UnknownFunction468f10("RunwayLights");
            UnknownFunction468f10("VisualCue");
        }
        g_TrackGame->mode.field_0x6a8 = 1 - g_TrackGame->mode.field_0x6a8;
        ShowOnOffMessage(0x13b6, g_TrackGame->mode.field_0x6a8);
        return 1;
    }
    if (UnknownFunction43caa0(0x3f, 0, event, 0x80000000)) {
        g_TrackGame->mode.field_0x6b0 = 1 - g_TrackGame->mode.field_0x6b0;
        ShowOnOffMessage(0x1415, g_TrackGame->mode.field_0x6b0);
        return 1;
    }
    if (UnknownFunction43caa0(0x40, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.field_0x6b8)
            UnknownFunction468dd0("RadarOverlay");
        else
            UnknownFunction468f10("RadarOverlay");
        g_TrackGame->mode.field_0x6b8 = 1 - g_TrackGame->mode.field_0x6b8;
        ShowOnOffMessage(0x1441, g_TrackGame->mode.field_0x6b8);
        return 1;
    }
    if (UnknownFunction43caa0(0x41, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.field_0x6ac)
            UnknownFunction468dd0("InstrumentOverlay");
        else
            UnknownFunction468f10("InstrumentOverlay");
        g_TrackGame->mode.field_0x6ac = 1 - g_TrackGame->mode.field_0x6ac;
        ShowOnOffMessage(0x1416, g_TrackGame->mode.field_0x6ac);
        return 1;
    }
    if (UnknownFunction43caa0(0x42, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.field_0xa5c)
            UnknownFunction468dd0("SkyCube");
        else
            UnknownFunction468f10("SkyCube");
        g_TrackGame->mode.field_0xa5c = 1 - g_TrackGame->mode.field_0xa5c;
        ShowOnOffMessage(0x1411, g_TrackGame->mode.field_0xa5c);
        ((RenderTarget*)field_0x18)->field_0x34 = !(skyCube && g_TrackGame->mode.field_0xa5c);
        return 1;
    }
    if (UnknownFunction43caa0(0x43, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.field_0xa54) {
            UnknownFunction468dd0("ParticleManager");
            UnknownFunction468dd0("DirtParticleEmitter");
            UnknownFunction468dd0("DustParticleEmitter");
            UnknownFunction468dd0("DirtChunkParticleEmitter");
            UnknownFunction468dd0("SteamParticleEmitter");
        } else {
            UnknownFunction468f10("ParticleManager");
            UnknownFunction468f10("DirtParticleEmitter");
            UnknownFunction468f10("DustParticleEmitter");
            UnknownFunction468f10("DirtChunkParticleEmitter");
            UnknownFunction468f10("SteamParticleEmitter");
        }
        g_TrackGame->mode.field_0xa54 = 1 - g_TrackGame->mode.field_0xa54;
        ShowOnOffMessage(0x140f, g_TrackGame->mode.field_0xa54);
        return 1;
    }
    if (UnknownFunction43caa0(0x44, 0, event, 0x80000000)) {
        if (g_TrackGame->mode.field_0xa50) {
            UnknownFunction468dd0("ProjectedShadow");
            UnknownFunction468dd0("TerrainShadow");
            UnknownFunction468dd0("D3DIMSoultreeShadow");
        } else {
            UnknownFunction468f10("ProjectedShadow");
            UnknownFunction468f10("TerrainShadow");
            UnknownFunction468f10("D3DIMSoultreeShadow");
        }
        g_TrackGame->mode.field_0xa50 = 1 - g_TrackGame->mode.field_0xa50;
        ShowOnOffMessage(0x1410, g_TrackGame->mode.field_0xa50);
        return 1;
    }
    if (UnknownFunction43caa0(0x57, 0, event, 0x80000000)) {
        int level = g_TrackGame->mode.field_0xa64 - 1;
        if (level < 0)
            level = 0;
        g_TrackGame->mode.field_0xa64 = level;
        g_TrackGame->mode.field_0x195c[3] = level;
        g_TrackGame->mode.field_0x195c[1] = level;
        g_TrackGame->mode.field_0x195c[2] = level;
        g_TrackGame->mode.field_0x195c[4] = level;
        eventTerrain->UnknownFunction507960(level);
        if (ecoSystem)
            ecoSystem->UnknownFunction4594c0(level);
        if (eventFog)
            eventFog->UnknownFunction462db0(level);
        eventScene->UnknownFunction4eff30(level);
        raceView->UnknownFunction423790(level);
        if (textQueue) {
            char text[0x80];
            char buffer[0x100];
            g_TrackGame->LoadResourceString(0x14c4, text, 0x80);
            sprintf(buffer, "%s = %d", text, level + 1);
            UnknownMessage message(buffer, 1.5f);
            textQueue->UnknownFunction51b540(&message);
        }
        return 1;
    }
    if (UnknownFunction43caa0(0x58, 0, event, 0x80000000)) {
        int level = g_TrackGame->mode.field_0xa64 + 1;
        if (level >= 9)
            level = 9;
        g_TrackGame->mode.field_0xa64 = level;
        g_TrackGame->mode.field_0x195c[3] = level;
        g_TrackGame->mode.field_0x195c[1] = level;
        g_TrackGame->mode.field_0x195c[2] = level;
        g_TrackGame->mode.field_0x195c[4] = level;
        eventTerrain->UnknownFunction507960(level);
        if (ecoSystem)
            ecoSystem->UnknownFunction4594c0(level);
        if (eventFog)
            eventFog->UnknownFunction462db0(level);
        eventScene->UnknownFunction4eff30(level);
        raceView->UnknownFunction423790(level);
        if (textQueue) {
            char text[0x80];
            char buffer[0x100];
            g_TrackGame->LoadResourceString(0x14c4, text, 0x80);
            sprintf(buffer, "%s = %d", text, level + 1);
            UnknownMessage message(buffer, 1.5f);
            textQueue->UnknownFunction51b540(&message);
        }
        return 1;
    }
    if (g_TrackGame->field_0x2d4_bit2 && g_TrackGame->debugOverlay) {
        if (s_DebugPage < 0)
            s_DebugPage = g_TrackGame->debugOverlay->NewPage();
        if (UnknownFunction43caa0(0x1c, 0, event, 0x80)) {
            g_TrackGame->debugOverlay->UnknownFunction448000(s_DebugPage, s_DebugRow + 1, 0);
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
    int field_0x08;
};

// 0x004e1cc0
int BaseQuarryEvent::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    if (GameObject::UnknownVirtualSlot24(type, data, from, to, flags))
        return 1;
    if (g_TrackGame->uiInteractionBlocked)
        return 1;
    if (type == 5) {
        UnknownQuarryLeaveMessage* leave = (UnknownQuarryLeaveMessage*)data;
        char name[0x80];
        char text[0x80];
        char buffer[0x80];
        strcpy(name, "");
        for (int i = 0; i < g_TrackGame->mode.field_0x1be0; i++) {
            if (g_TrackGame->mode.field_0x1be4[i].field_0xd4 == leave->field_0x08) {
                int n = strlen(g_TrackGame->mode.field_0x1be4[i].field_0xdc);
                int length = n > 0x7f ? 0x7f : n;
                strncpy(name, g_TrackGame->mode.field_0x1be4[i].field_0xdc, length);
                name[length] = 0;
                break;
            }
        }
        g_TrackGame->LoadResourceString(0xbd6, text, 0x80);
        sprintf(buffer, "%s %s", name, text);
        UnknownMessage message(buffer, 3.25f);
        textQueue->UnknownFunction51b5e0(&message);
        raceView->UnknownFunction420590(leave->field_0x08);
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2)
            g_TrackGame->eventManager->UnknownFunction45fbd0(leave->field_0x08);
    } else if (from && type == 0x89) {
        char text[0x80];
        char buffer[0x80];
        g_TrackGame->LoadResourceString(0xbd5, text, 0x80);
        sprintf(buffer, "%s %s", (char*)data + 4, text);
        UnknownMessage message(buffer, 3.25f);
        textQueue->UnknownFunction51b5e0(&message);
        raceView->UnknownFunction420590(from);
        if (g_TrackGame->mode.field_0x27f8.field_0x00 == 2)
            g_TrackGame->eventManager->UnknownFunction45fbd0(from);
    }
    return 0;
}

// 0x004e1f00
void BaseQuarryEvent::UnknownFunction4e1f00() {
    clockMinutes = 0.0f;
    clockSeconds = 0.0f;
}

// 0x004e1f10
BaseQuarryEvent::~BaseQuarryEvent() {
    delete field_0x44;
    Release();
    if (g_TrackGame->field_0x2d5_bit2) {
        field_0x94->Release();
        field_0x98->Release();
    }
    ((RenderTarget*)field_0x18)->field_0x08 = 0;
}

// The remaining overrides are bodies the linker folded with identical ones
// elsewhere; the retail vtable 0x0055766c points at the surviving copies.

// 0x00499af0 (KrustyUI's slot 18 is the same body)
int BaseQuarryEvent::UnknownVirtualSlot18() {
    GameObject::UnknownVirtualSlot18();
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

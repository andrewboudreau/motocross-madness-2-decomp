// Near-miss KrustyUI candidates, kept out of src/reconstructed until they
// match. See docs/KRUSTYUI.md.
//
// KrustyUI::Create (0x004988a0, 1104 bytes): TrackGame slot
// 4's initialiser. The control flow, calls, both debug `new`s (GUI, line
// 121; Intro1Dlg, line 201), the strncpy clamps and the frame (text is a
// 260-byte buffer read 128 at a time) match: 872 of 1104 bytes. From the
// scale store on, retail picks eax/ecx where VC6 here picks edx/eax (scale,
// layer +0xc0, GUI +0x34c) and loads the global before pushing the joystick
// filter. Inline-helper, block-local, local-gui and local-filter forms do
// not change it.
// Session 14: `if (scale)`, `scale != 0` and a `!= 0` load test are inert
// (872); `else scale = 0` instead of the leading store is worse (363).
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/OptionProcs.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/TextureMap.h"
#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/UIDialog.h"

void GuiProgressCallback();

// cdecl 0x0047b570 (gameui.cpp): resizes a DebugMalloc'd block.
void* ResizeDebugBlock(void* block, unsigned int size);

// Global at 0x0068a498 (defined in KrustyUI.cpp).
extern char g_ResourceNames[36][16];

// 0x004988a0: GameObject's slot 8, then loads the short strings and the GUI
// settings, creates and sets up the GUI, restores the last profile and the
// joystick filter, and (offline, when asked) opens the intro dialog.
GameObject* KrustyUI::Create(RenderTarget* target, int showIntro) {
    char font[128];
    char text[260];
    float scale;
    unsigned long size;
    int length;
    int count;
    GameObject::SetRenderContext(target);
    size = 16;
    for (int i = 0; i < 36; i++) {
        g_TrackGame->LoadResourceString(5000 + i, text, 128);
        length = strlen(text);
        count = length > 15 ? 15 : length;
        strncpy(g_ResourceNames[i], text, count);
        g_ResourceNames[i][count] = 0;
    }
    scale = 0;
    if (g_TrackGame->LoadResourceString(0xfed, text, 128))
        scale = (float)atof(text);
    if (g_TrackGame->LoadResourceString(0xff2, text, 128)) {
        length = strlen(text);
        count = length > 127 ? 127 : length;
        strncpy(font, text, count);
    } else {
        length = strlen("Arial");
        count = length > 127 ? 127 : length;
        strncpy(font, "Arial", count);
    }
    font[count] = 0;
    gui = new(__FILE__, 121) UnknownKrustyUIGui(1);
    if (!AppendSibling(gui->SetUp(
            renderContext, g_TrackGame->display->palette, g_TrackGame->textureManager, 0,
            0, 0, "Arial", 15, "ui\\cursor.tga", GuiProgressCallback, -1))) {
        Release();
        return 0;
    }
    if (font[0] && _strnicmp(font, "NONE", 4)) {
        gui->SetToolTipFont(font);
        if (g_TrackGame->LoadResourceString(0x1469, text, 128))
            gui->toolTipBold = atoi(text);
    }
    if (scale != 0.0f)
        gui->fontSizeOffset = scale;
    gui->CreateBackground();
    gui->SetDialogDirectory("ui");
    gui->SetWaitCursorImage("ui\\wait.tga");
    gui->GetUser(0)->ForgetDevices();
    gui->GetUser(0)->AcceptDevice(gui->keyboardDevice);
    gui->ShowCursors(0);
    g_ResourceManager->AddArchive("ui\\uires.res", 0);
    g_TrackGame->mode.LoadBonusTrackNames();
    g_TrackGame->GetRegistryString("MRUProfile", "",
                                                 g_TrackGame->mode.profileName, &size);
    g_TrackGame->mode.LoadProfile();
    gui->uiAnimations = g_TrackGame->mode.uiAnimations;
    if (!g_TrackGame->mode.toolTips)
        gui->GetUser(0)->userToolTip->enabled = 0;
    if (g_TrackGame->controlInterface->activeJoystick)
        ((PCJoystickDevice*)g_TrackGame->controlInterface->activeJoystick)
            ->UnknownMethod4c3ae0(g_TrackGame->GetRegistryFlag("JoystickFilter", 0));
    bikes = 0;
    riders = 0;
    manufacturers = 0;
    LoadBikesAndRiders();
    LoadGarageTables();
    gui->soundVolume =
        (g_TrackGame->mode.uiSounds ? (g_TrackGame->mode.uiVolume - 100) * 25 : -10000) -
        200;
    if ((!g_TrackGame->network || !g_TrackGame->network->lobbyConnected) && showIntro)
        gui->ShowDialog(new(__FILE__, 201) Intro1Dlg, 0, 2, 0, 0, 0, 0, 1);
    return this;
}

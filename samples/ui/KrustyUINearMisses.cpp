// Near-miss KrustyUI candidates, kept out of src/reconstructed until they
// match. See docs/KRUSTYUI.md.
//
// KrustyUI::UnknownFunction4988a0 (0x004988a0, 1104 bytes): TrackGame slot
// 4's initialiser. The control flow, calls, both debug `new`s (GUI, line
// 121; Intro1Dlg, line 201), the strncpy clamps and the frame (text is a
// 260-byte buffer read 128 at a time) match: 872 of 1104 bytes. From the
// scale store on, retail picks eax/ecx where VC6 here picks edx/eax (scale,
// layer +0xc0, GUI +0x34c) and loads the global before pushing the joystick
// filter. Inline-helper, block-local, local-gui and local-filter forms do
// not change it.
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCJoystickDevice.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/UIDialog.h"

void UnknownFunction49bda0();

// Global at 0x0068a498 (defined in KrustyUI.cpp).
extern char g_UnknownStrings68a498[36][16];

// Zero-filled global at 0x00577738, the "MRUProfile" default.
extern char g_UnknownString577738[];

// 0x004988a0: GameObject's slot 8, then loads the short strings and the GUI
// settings, creates and sets up the GUI, restores the last profile and the
// joystick filter, and (offline, when asked) opens the intro dialog.
GameObject* KrustyUI::UnknownFunction4988a0(RenderTarget* target, int showIntro) {
    char font[128];
    char text[260];
    float scale;
    unsigned long size;
    int length;
    int count;
    GameObject::UnknownVirtualSlot8(target);
    size = 16;
    for (int i = 0; i < 36; i++) {
        g_UnknownGlobal56e26c->UnknownFunction521970(5000 + i, text, 128);
        length = strlen(text);
        count = length > 15 ? 15 : length;
        strncpy(g_UnknownStrings68a498[i], text, count);
        g_UnknownStrings68a498[i][count] = 0;
    }
    scale = 0;
    if (g_UnknownGlobal56e26c->UnknownFunction521970(0xfed, text, 128))
        scale = (float)atof(text);
    if (g_UnknownGlobal56e26c->UnknownFunction521970(0xff2, text, 128)) {
        length = strlen(text);
        count = length > 127 ? 127 : length;
        strncpy(font, text, count);
    } else {
        length = strlen("Arial");
        count = length > 127 ? 127 : length;
        strncpy(font, "Arial", count);
    }
    font[count] = 0;
    field_0x2c = new(__FILE__, 121) UnknownKrustyUIGui(1);
    if (!UnknownFunction469130(field_0x2c->UnknownFunction4853b0(
            field_0x18, g_UnknownGlobal56e26c->field_0x0c->field_0x64, g_UnknownGlobal56e26c->field_0x3c, 0,
            0, 0, "Arial", 15, "ui\\cursor.tga", UnknownFunction49bda0, -1))) {
        Release();
        return 0;
    }
    if (font[0] && _strnicmp(font, "NONE", 4)) {
        field_0x2c->UnknownFunction4866c0(font);
        if (g_UnknownGlobal56e26c->UnknownFunction521970(0x1469, text, 128))
            field_0x2c->field_0x3d4 = atoi(text);
    }
    if (scale != 0.0f)
        field_0x2c->field_0x3d0 = scale;
    field_0x2c->UnknownFunction485ef0();
    field_0x2c->UnknownFunction485d70("ui");
    field_0x2c->UnknownFunction486560("ui\\wait.tga");
    field_0x2c->UnknownFunction486540(0)->UnknownFunction487d60();
    field_0x2c->UnknownFunction486540(0)->UnknownFunction487c30(field_0x2c->field_0x30c);
    field_0x2c->UnknownFunction486630(0);
    g_UnknownResourceManager572b44->UnknownFunction4e9030("ui\\uires.res", 0);
    g_UnknownGlobal56e26c->mode.UnknownFunction523e50();
    g_UnknownGlobal56e26c->UnknownVirtualSlot23("MRUProfile", g_UnknownString577738,
                                                 g_UnknownGlobal56e26c->mode.field_0x00, &size);
    g_UnknownGlobal56e26c->mode.UnknownFunction5231f0();
    field_0x2c->field_0x0ec = g_UnknownGlobal56e26c->mode.field_0x6c8;
    if (!g_UnknownGlobal56e26c->mode.field_0x6d4)
        field_0x2c->UnknownFunction486540(0)->field_0xc0->field_0x5c = 0;
    if (g_UnknownGlobal56e26c->field_0x14->activeJoystick)
        ((PCJoystickDevice*)g_UnknownGlobal56e26c->field_0x14->activeJoystick)
            ->UnknownMethod4c3ae0(g_UnknownGlobal56e26c->UnknownVirtualSlot22("JoystickFilter", 0));
    field_0x50 = 0;
    field_0x58 = 0;
    field_0x48 = 0;
    UnknownFunction49a8b0();
    UnknownFunction49a540();
    field_0x2c->field_0x34c =
        (g_UnknownGlobal56e26c->mode.field_0xa24 ? (g_UnknownGlobal56e26c->mode.field_0xa3c - 100) * 25 : -10000) -
        200;
    if ((!g_UnknownGlobal56e26c->field_0x08 || !g_UnknownGlobal56e26c->field_0x08->field_0x14) && showIntro)
        field_0x2c->UnknownFunction485a70(new(__FILE__, 201) Intro1Dlg, 0, 2, 0, 0, 0, 0, 1);
    return this;
}

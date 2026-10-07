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
//
// KrustyUI::UnknownFunction49b0d0 (0x0049b0d0, 916 bytes): picks the AI
// racers' bikes and riders. 880 of 916 bytes: retail lays the class 1/2
// test out as "<= 0.2: class 2 or probe; > 0.2: class 1 or probe" with the
// second arm placed after the probe loop; every if/else, goto and ternary
// form tried here keeps both arms before it.
//
// KrustyUI::UnknownFunction49a8b0 (0x0049a8b0, 1897 bytes): loads
// bikes.pb and riders.pb. 1891 of 1897 bytes: retail keeps the
// manufacturer index at [esp+0x1c] and the bike count at [esp+0x20]; VC6
// here swaps the two slots. Declaration order, scope, names and loop forms
// do not move them.
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

void UnknownFunction49bda0();

// cdecl 0x0047b570 (gameui.cpp): resizes a DebugMalloc'd block.
void* UnknownFunction47b570(void* block, unsigned int size);

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

// The same views and helper as KrustyUI.cpp.
struct UnknownKrustyUIModelEntry {
    char field_0x00[0x40];
    char field_0x40[0x40];
    char field_0x80[0x40];
    int field_0xc0;
    int field_0xc4;
};

struct UnknownKrustyUIBikeEntry {
    int field_0x00;
    char field_0x04[0x44];
    char field_0x48[0x40];
    int field_0x88;
    int field_0x8c;
    int field_0x90;
};

static inline float RandomUnit() {
    return (float)(rand() * (1.0f / 32768));
}

// 0x0049b0d0
void KrustyUI::UnknownFunction49b0d0(int* bikes, int count, int* riders) {
    int* usedBikes = (int*)DebugCalloc(field_0x54, sizeof(int), __FILE__, 1164);
    int* usedRiders = (int*)DebugCalloc(field_0x5c, sizeof(int), __FILE__, 1165);
    int playerClass = UnknownBikeClassOf(
        ((UnknownTrackGameModeOptionsFd8*)g_UnknownGlobal56e26c->mode.field_0xfd8)->field_0x00);
    srand(UnknownFunction4bfa80());
    int i;
    for (i = 0; i < field_0x5c; i++) {
        int type = ((UnknownKrustyUIModelEntry*)field_0x58)[i].field_0xc4;
        if (type < 2 || type > 3)
            usedRiders[i] = -1;
    }
    for (i = 0; i < field_0x54; i++) {
        int type = ((UnknownKrustyUIModelEntry*)field_0x48)[((UnknownKrustyUIBikeEntry*)field_0x50)[i].field_0x00].field_0xc4;
        if (type < 2 || type > 3) {
            usedBikes[i] = -1;
            continue;
        }
        int bikeClass = UnknownBikeClassOf(((UnknownKrustyUIBikeEntry*)field_0x50)[i].field_0x8c);
        if (playerClass == bikeClass || (playerClass == 1 && bikeClass == 2)
            || (playerClass == 2 && bikeClass == 1) || (playerClass == 3 && bikeClass == 4)
            || (playerClass == 4 && bikeClass == 3))
            continue;
        usedBikes[i] = -1;
    }
    for (int k = 0; k < count; k++) {
        int index;
        float chance;
        int bikeClass;
        do {
            index = (int)(RandomUnit() * (field_0x54 - 1));
        } while (usedBikes[index] == -1);
        int start = index;
        if (!usedBikes[index]) {
            chance = RandomUnit();
            bikeClass = UnknownBikeClassOf(((UnknownKrustyUIBikeEntry*)field_0x50)[index].field_0x8c);
            switch (bikeClass) {
            case 1:
            case 2:
                // Four in five picks keep class 1, the rest class 2.
                if (chance > 0.2f) {
                    if (bikeClass != 1)
                        break;
                    goto found;
                }
                if (bikeClass == 2)
                    goto found;
                break;
            case 3:
            case 4:
                if (chance > 0.5f)
                    goto found;
                goto found;
            default:
                goto found;
            }
        }
        do {
            if (++index == field_0x54)
                index = 0;
            if (index == start)
                break;
        } while (usedBikes[index]);
    found:
        usedBikes[index] = 1;
        bikes[k] = index;
        do {
            index = (int)(RandomUnit() * (field_0x5c - 1));
        } while (usedRiders[index] == -1);
        start = index;
        while (usedRiders[index]) {
            if (++index == field_0x5c)
                index = 0;
            if (index == start)
                break;
        }
        usedRiders[index] = 1;
        riders[k] = index;
    }
    DebugFree(usedBikes, __FILE__, 1299);
    DebugFree(usedRiders, __FILE__, 1300);
}

// 0x0049a8b0: reads the bike models and bikes (bikes.pb) and the riders
// (riders.pb) into the +0x48, +0x50 and +0x58 lists. Only manufacturers 3
// and 7, their second bike, and riders 42 and 44 are loaded.
void KrustyUI::UnknownFunction49a8b0() {
    int count;
    char key[256];
    UnknownParameterBlock block;
    UnknownTextureStream* stream = new(__FILE__, 913) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (field_0x48)
        DebugFree(field_0x48, __FILE__, 918);
    if (field_0x50)
        DebugFree(field_0x50, __FILE__, 919);
    if (field_0x58)
        DebugFree(field_0x58, __FILE__, 920);
    field_0x50 = 0;
    field_0x58 = 0;
    field_0x5c = 0;
    field_0x54 = 0;
    field_0x4c = 0;
    if (stream->UnknownFunction460f50("bikes.pb", "r", 0)) {
        block.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
        block.UnknownFunction4b78f0("Information");
        block.UnknownFunction4b7f10("NumberOfManufacturers", 0, &count);
        for (int i = 1; i <= count; i++) {
            if (i != 7 && i != 3)
                continue;
            int bikeCount;
            int availability;
            char name[260];
            char uiModel[260];
            char model[260];
            sprintf(key, "Manufacturer_%02d", i);
            block.UnknownFunction4b78f0(key);
            block.UnknownFunction4b7f10("NumberOfBikes", 0, &bikeCount);
            block.UnknownFunction4b7ec0("Name", "(no name)", name, -1);
            block.UnknownFunction4b7ec0("UIModelFile", "", uiModel, -1);
            block.UnknownFunction4b7ec0("ModelFile", "", model, -1);
            block.UnknownFunction4b7f10("Availability", 1, &availability);
            field_0x48 = UnknownFunction47b570(field_0x48, (field_0x4c + 1) * sizeof(UnknownKrustyUIModelEntry));
            int length = strlen(name);
            int n = length > 63 ? 63 : length;
            strncpy(((UnknownKrustyUIModelEntry*)field_0x48)[field_0x4c].field_0x00, name, n);
            ((UnknownKrustyUIModelEntry*)field_0x48)[field_0x4c].field_0x00[n] = 0;
            length = strlen(model);
            n = length > 63 ? 63 : length;
            strncpy(((UnknownKrustyUIModelEntry*)field_0x48)[field_0x4c].field_0x40, model, n);
            ((UnknownKrustyUIModelEntry*)field_0x48)[field_0x4c].field_0x40[n] = 0;
            length = strlen(uiModel);
            n = length > 63 ? 63 : length;
            strncpy(((UnknownKrustyUIModelEntry*)field_0x48)[field_0x4c].field_0x80, uiModel, n);
            ((UnknownKrustyUIModelEntry*)field_0x48)[field_0x4c].field_0x80[n] = 0;
            ((UnknownKrustyUIModelEntry*)field_0x48)[field_0x4c].field_0xc4 = availability;
            for (int j = 1; j <= bikeCount; j++) {
                if (j != 2)
                    continue;
                char bikeName[260];
                char texture[260];
                sprintf(key, "BikeName_%02d", j);
                block.UnknownFunction4b7ec0(key, "(no name)", bikeName, -1);
                sprintf(key, "BikeTexture_%02d", j);
                block.UnknownFunction4b7ec0(key, "", texture, -1);
                field_0x50 = UnknownFunction47b570(field_0x50, (field_0x54 + 1) * sizeof(UnknownKrustyUIBikeEntry));
                ((UnknownKrustyUIBikeEntry*)field_0x50)[field_0x54].field_0x00 = field_0x4c;
                ((UnknownKrustyUIBikeEntry*)field_0x50)[field_0x54].field_0x88 = 0;
                length = strlen(bikeName);
                n = length > 63 ? 63 : length;
                strncpy(((UnknownKrustyUIBikeEntry*)field_0x50)[field_0x54].field_0x04, bikeName, n);
                ((UnknownKrustyUIBikeEntry*)field_0x50)[field_0x54].field_0x04[n] = 0;
                length = strlen(texture);
                n = length > 63 ? 63 : length;
                strncpy(((UnknownKrustyUIBikeEntry*)field_0x50)[field_0x54].field_0x48, texture, n);
                ((UnknownKrustyUIBikeEntry*)field_0x50)[field_0x54].field_0x48[n] = 0;
                sprintf(key, "BikeEngineSize_%02d", j);
                block.UnknownFunction4b7f10(key, 250, &((UnknownKrustyUIBikeEntry*)field_0x50)[field_0x54].field_0x8c);
                sprintf(key, "BikeFourStroke_%02d", j);
                block.UnknownFunction4b7f10(key, 0, &((UnknownKrustyUIBikeEntry*)field_0x50)[field_0x54].field_0x90);
                field_0x54++;
            }
            field_0x4c++;
        }
    }
    if (stream->UnknownFunction460f50("riders.pb", "r", 0)) {
        block.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
        block.UnknownFunction4b78f0("Information");
        block.UnknownFunction4b7f10("NumberOfRiders", 0, &count);
        for (int i = 1; i <= count; i++) {
            if (i != 42 && i != 44)
                continue;
            int availability;
            char riderName[260];
            char file[260];
            sprintf(key, "Rider_%02d", i);
            block.UnknownFunction4b78f0(key);
            block.UnknownFunction4b7ec0("Name", "(no name)", riderName, -1);
            block.UnknownFunction4b7ec0("File", "", file, -1);
            block.UnknownFunction4b7f10("Availability", 1, &availability);
            field_0x58 = UnknownFunction47b570(field_0x58, (field_0x5c + 1) * sizeof(UnknownKrustyUIModelEntry));
            ((UnknownKrustyUIModelEntry*)field_0x58)[field_0x5c].field_0xc0 = 0;
            int length = strlen(riderName);
            int n = length > 63 ? 63 : length;
            strncpy(((UnknownKrustyUIModelEntry*)field_0x58)[field_0x5c].field_0x00, riderName, n);
            ((UnknownKrustyUIModelEntry*)field_0x58)[field_0x5c].field_0x00[n] = 0;
            length = strlen(file);
            n = length > 127 ? 127 : length;
            strncpy(((UnknownKrustyUIModelEntry*)field_0x58)[field_0x5c].field_0x40, file, n);
            ((UnknownKrustyUIModelEntry*)field_0x58)[field_0x5c].field_0x40[n] = 0;
            ((UnknownKrustyUIModelEntry*)field_0x58)[field_0x5c].field_0xc4 = availability;
            field_0x5c++;
        }
    }
    delete stream;
}

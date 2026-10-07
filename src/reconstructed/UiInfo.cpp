// UiInfo.cpp -- D:\aardvark\VC\krusty2\uiinfo.cpp (0x00521f30..0x00524106):
// TrackGameMode's methods. The literal __FILE__ (0x00575780) is used at
// 0x00522134 (constructor, line 71), 0x00522d2d..0x00522d9f (lines 383-385)
// and 0x00523e81 (line 873); TypeRegistry.cpp ends at 0x00521f2b and VCR.cpp
// starts at 0x00524110. Near misses are in samples/game/UiInfoNearMisses.cpp.
// The constructor also makes VC6 emit SessionInfoType's inline constructor
// (0x00523b90), its implicit destructor (0x00523c80, the same bytes as
// InfoType's) and UnknownTrackGameSessionList's implicit destructor
// (0x00522420), which the constructor's and destructor's unwind actions call.
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TrackGame.h"

#include "DebugAlloc.h"
#include "Parameterblocks.h"
#include "SoundInterface.h"
#include "TextureMap.h"
#include "UnknownResourceManager.h"

// USER32 and SHELL32 imports.
extern "C" __declspec(dllimport) long __stdcall SendMessageA(void* window, unsigned int message,
                                                            unsigned int wParam, long lParam);
extern "C" __declspec(dllimport) int __stdcall LoadStringA(void* instance, unsigned int id,
                                                          char* buffer, int size);
extern "C" __declspec(dllimport) int __stdcall ShowCursor(int show);
extern "C" __declspec(dllimport) int __stdcall MessageBoxA(void* window, const char* text,
                                                          const char* caption, unsigned int type);
extern "C" __declspec(dllimport) void* __stdcall ShellExecuteA(void* window, const char* operation,
                                                              const char* file,
                                                              const char* parameters,
                                                              const char* directory, int show);

// The help files' directory: 0x00523d30 passes the shared empty literal
// both as 0x00523a60's directory and to a strcmp against "".
#define UNKNOWN_HELP_DIRECTORY ""

// 0x00521f30
UnknownTrackGameRacerSlot::UnknownTrackGameRacerSlot() {
    int length;
    int count;

    field_0xd4 = 0;
    length = strlen("");
    count = length > 0xf ? 0xf : length;
    strncpy(field_0xdc, "", count);
    field_0xdc[count] = 0;
    field_0xc8 = 0;
    field_0xcc = 0;
    field_0x80[0] = 0;
    field_0x40[0] = 0;
    field_0x00[0] = 0;
    field_0xd0 = FLT_MAX;
    field_0xf4 = 1;
}

// 0x00521fb0
void UnknownTrackGameRacerSlot::UnknownFunction521fb0(void* a, void* b, void* c) {
    int length = strlen((char*)a);
    int count = length > 0x3f ? 0x3f : length;
    strncpy(field_0x00, (char*)a, count);
    field_0x00[count] = 0;
    length = strlen((char*)b);
    count = length > 0x3f ? 0x3f : length;
    strncpy(field_0x40, (char*)b, count);
    field_0x40[count] = 0;
    length = strlen((char*)c);
    count = length > 0x3f ? 0x3f : length;
    strncpy(field_0x80, (char*)c, count);
    field_0x80[count] = 0;
}

// 0x00522050
void UnknownTrackGameRacerSlot::UnknownFunction522050() {
    field_0xc8 = 0;
}

// 0x00522060
#define UNKNOWN_SET_DIRECTORY(index, name)                                        \
    length = strlen(name);                                                        \
    count = length > 0xff ? 0xff : length;                                        \
    strncpy(field_0xa0[index], name, count);                                      \
    field_0xa0[index][count] = 0;

TrackGameMode::TrackGameMode() {
    int i;
    int length;
    int count;

    UnknownFunction522440();
    field_0x23a4 = 0;
    field_0x6a8 = 1;
    memset(&field_0x23a8, 0, sizeof(field_0x23a8));
    field_0x23cc = 2;
    strcpy(field_0x23d0, "");
    strcpy(field_0x24d4, "");
    strcpy(field_0x25d8, "");
    field_0x25dc = 0;
    field_0x25dc = new(__FILE__, 71) UnknownDriveList;
    if (!field_0x25dc->UnknownFunction449e60()) {
        delete field_0x25dc;
        field_0x25dc = 0;
    }
    field_0x25e0 = 0;
    field_0x25e4 = 0;
    field_0x25e8 = 0;
    field_0x9c = 1;
    field_0x25ec[0] = 0;
    field_0x26f0 = 0;
    field_0x26f4[0] = 0;
    field_0x1974.field_0xc0 = -1;
    field_0x1bcc = (int)((rand() * (1.0f / 32768.0f)) * 999.0f);
    if (field_0x1bcc <= 99)
        field_0x1bcc += 100;
    for (i = 0; i < 6; i++) {
        field_0x1384[i] = 0;
        field_0x139c[i][0] = 0;
    }
    field_0x1bd0 = -1;
    field_0x1bd4 = -1;
    field_0x1bd8 = 0;
    field_0x1be0 = 0;
    field_0x10ec = 0;
    UNKNOWN_SET_DIRECTORY(0, "Teraform\\Quarries")
    UNKNOWN_SET_DIRECTORY(1, "Teraform\\Baja")
    UNKNOWN_SET_DIRECTORY(2, "Teraform\\National")
    UNKNOWN_SET_DIRECTORY(3, "Teraform\\SX")
    UNKNOWN_SET_DIRECTORY(4, "Teraform\\Tag")
    UNKNOWN_SET_DIRECTORY(5, "Teraform\\Enduro")
}

#undef UNKNOWN_SET_DIRECTORY

// 0x005225f0
TrackGameMode::~TrackGameMode() {
    if (field_0x25e0)
        delete field_0x25e0;
    if (field_0x25e4)
        delete field_0x25e4;
    if (field_0x25e8)
        delete field_0x25e8;
}

// 0x00522680 (TrackGame slot 4): clears the network race state.
int TrackGameMode::ResetNetworkRace() {
    int i;

    field_0x6a4 = 0;
    field_0xfd4 = 0;
    field_0x1be0 = 0;
    if (field_0x27f8.field_0x00 != 2) {
        for (i = 0; i < 8; i++)
            field_0x1be4[i] = UnknownTrackGameRacerSlot();
    }
    for (i = 0; i < 8; i++)
        field_0x27f8.field_0x14c[i] = UnknownTrackGameModeEntry();
    field_0x27f8.field_0x35 = 0;
    return 1;
}

// 0x00522720: the defaults of the TrackGameMode+0x6ac options; the last
// 0x14 bytes are cleared with an inline memset.
void TrackGameMode::UnknownFunction522720(UnknownTrackGameModeOptions6ac* options) {
    options->field_0x00 = 1;
    options->field_0x04 = 1;
    options->field_0x1c = 1;
    options->field_0x08 = 0;
    options->field_0x0c = 1;
    options->field_0x10 = 1;
    options->field_0x14 = 1;
    options->field_0x18 = 1;
    options->field_0x30 = 2;
    options->field_0x34 = 0;
    options->field_0x38 = 0;
    options->field_0x3c = 0;
    options->field_0x40 = 0;
    options->field_0x44 = 0;
    options->field_0x20 = 1;
    options->field_0x24 = 1;
    options->field_0x28 = 1;
    options->field_0x2c = 1;
    memset(&options->field_0x360, 0, sizeof(options->field_0x360));
}

// 0x00522780
void TrackGameMode::UnknownFunction522780(UnknownTrackGameModeOptionsA20* options) {
    options->field_0x00 = 0;
    options->field_0x08 = 1;
    options->field_0x04 = 1;
    options->field_0x0c = 1;
    options->field_0x20 = 100;
    options->field_0x24 = 100;
    options->field_0x10 = 0;
    options->field_0x1c = 100;
    options->field_0x14 = 0;
    options->field_0x28 = 0;
    options->field_0x18 = 1;
}

// 0x005227d0
void TrackGameMode::UnknownFunction5227d0(UnknownTrackGameModeOptionsA4c* options) {
    options->field_0x00 = 0;
    options->field_0x04 = 1;
    options->field_0x08 = 1;
    options->field_0x0c = 0;
    options->field_0x10 = 1;
    options->field_0x18 = 6;
}

// 0x00522800
void TrackGameMode::UnknownFunction522800(UnknownTrackGameModeOptionsA68* options) {
    options->field_0x00[0] = 50;
    options->field_0x1c = 50;
    options->field_0x20 = 1;
    options->field_0x00[1] = 50;
    options->field_0x00[2] = 50;
    options->field_0x00[3] = 50;
    options->field_0x00[4] = 50;
    options->field_0x00[5] = 50;
    options->field_0x00[6] = 50;
    options->field_0x24 = 0;
    options->field_0x28 = 1;
    options->field_0x2c = 1;
}

// 0x00522840: reloads the ten names from string resources 0x1417..0x1420
// (the argument is unused; the names are TrackGameMode+0x145c).
#define UNKNOWN_LOAD_NAME(index, id)                                              \
    g_TrackGame->LoadResourceString(id, text, 0x80);                 \
    length = strlen(text);                                                        \
    count = length > 0x7f ? 0x7f : length;                                        \
    strncpy(g_TrackGame->mode.field_0x145c[index], text, count);             \
    g_TrackGame->mode.field_0x145c[index][count] = 0;

void TrackGameMode::UnknownFunction522840(char (*names)[0x80]) {
    char text[0x100];
    int length;
    int count;

    UNKNOWN_LOAD_NAME(1, 0x1417)
    UNKNOWN_LOAD_NAME(2, 0x1418)
    UNKNOWN_LOAD_NAME(3, 0x1419)
    UNKNOWN_LOAD_NAME(4, 0x141a)
    UNKNOWN_LOAD_NAME(5, 0x141b)
    UNKNOWN_LOAD_NAME(6, 0x141c)
    UNKNOWN_LOAD_NAME(7, 0x141d)
    UNKNOWN_LOAD_NAME(8, 0x141e)
    UNKNOWN_LOAD_NAME(9, 0x141f)
    UNKNOWN_LOAD_NAME(0, 0x1420)
}

#undef UNKNOWN_LOAD_NAME

// 0x00522bf0: the garage tables from KrustyUI (defaults when there is none).
void TrackGameMode::UnknownFunction522bf0(UnknownTrackGameModeOptionsFd8* options) {
    int i;
    int j;
    int k;

    for (i = 0; i < 11; i++)
        options->field_0x24[i] = g_TrackGame->ui ? g_TrackGame->ui->field_0x68[1][0][i] : 10;
    for (k = 0; k < 5; k++) {
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 11; i++)
                field_0x10f0[k][j][i] =
                    g_TrackGame->ui ? g_TrackGame->ui->field_0x43c[k] : 0;
        }
    }
}

// 0x00522c70
void TrackGameMode::UnknownFunction522c70(UnknownTrackGameModeOptionsFd8* options) {
    options->field_0x0c[0] = 0.5f;
    options->field_0x0c[1] = 0.5f;
    options->field_0x0c[2] = 0.5f;
    options->field_0x0c[3] = 0.5f;
    options->field_0x0c[4] = 0.5f;
    options->field_0x0c[5] = 0.5f;
}

// 0x00522c90
void TrackGameMode::UnknownFunction522c90(UnknownTrackGameModeOptionsFd8* options) {
    options->field_0x00 = 250;
    options->field_0x04 = 0;
    UnknownFunction522bf0(options);
}

// 0x00522cb0
void TrackGameMode::UnknownFunction522cb0(UnknownTrackGameModeOptionsFd8* options) {
    UnknownFunction522c70(options);
    UnknownFunction522c90(options);
}

// 0x00522cd0
void TrackGameMode::UnknownFunction522cd0() {
    int i;

    for (i = 0; i < 8; i++) {
        UnknownTrackGameModeEntry entry;
        entry.field_0x00 = 0;
        entry.field_0x04 = 0;
        field_0x27f8.field_0x14c[i] = entry;
    }
}


// 0x00522d00
int TrackGameMode::CreateDirectoryLists() {

    ChooseDisplayMode(0, &field_0xa4c);
    field_0x25e0 = new(__FILE__, 383) DirectoryList;
    field_0x25e4 = new(__FILE__, 384) CombinedDirectoryList;
    field_0x25e8 = new(__FILE__, 385) CombinedDirectoryList;
    if (field_0x25dc && field_0x25e4 && field_0x25e8)
        return 1;
    return 0;
}

// 0x00522e20
void TrackGameMode::ChooseDisplayMode(const void* guid, int* mode) {
    TrackGame* game = g_TrackGame;
    int i;

    if (!game->display)
        return;
    if (game->field_0x548_bit0 || !guid ||
        memcmp(guid, &game->display->field_0x4ac, sizeof(UnknownGuid)) ||
        !game->display->field_0x10[*mode].field_0x14) {
        *mode = 0;
        if (g_TrackGame->field_0x2d0) {
            for (i = 0; i < g_TrackGame->display->field_0x08; i++) {
                UnknownDisplayMode* entry = &g_TrackGame->display->field_0x10[i];
                if (entry->width == 640 && entry->height == 480 && entry->bitDepth == 16 &&
                    entry->field_0x18) {
                    *mode = i;
                    break;
                }
            }
            if (!*mode) {
                for (i = 0; i < g_TrackGame->display->field_0x08; i++) {
                    if (!*mode ||
                        (g_TrackGame->display->field_0x10[i].field_0x18 &&
                         g_TrackGame->display->field_0x10[i].bitDepth == 16 &&
                         g_TrackGame->display->field_0x10[i].height >
                             g_TrackGame->display->field_0x10[*mode].height))
                        *mode = i;
                }
            }
        } else {
            for (i = 0; i < g_TrackGame->display->field_0x08; i++) {
                UnknownDisplayMode* entry = &g_TrackGame->display->field_0x10[i];
                if (entry->width == 640 && entry->height == 480 && entry->bitDepth == 16 &&
                    entry->field_0x14) {
                    *mode = i;
                    break;
                }
            }
            if (!*mode) {
                for (i = 0; i < g_TrackGame->display->field_0x08; i++) {
                    if (!*mode ||
                        (g_TrackGame->display->field_0x10[i].field_0x14 &&
                         g_TrackGame->display->field_0x10[i].bitDepth == 16 &&
                         g_TrackGame->display->field_0x10[i].height >
                             g_TrackGame->display->field_0x10[*mode].height))
                        *mode = i;
                }
            }
        }
    }
    field_0x23a8 = g_TrackGame->display->field_0x4ac;
}

// 0x00523000: loads the profile's control file.
void TrackGameMode::UnknownFunction523000() {
    unsigned long size;
    char controller[0x80];
    char last[0x80];
    char path[0x104];

    if (!g_TrackGame->field_0x33fc)
        return;
    sprintf(path, "%s\\%s\\%s", "ui\\profile", field_0x00, "control.ctl");
    g_TrackGame->field_0x33fc->UnknownFunction448990(path);
    size = 0x80;
    g_TrackGame->GetRegistryString("UseControllerId", "", controller, &size);
    g_TrackGame->GetRegistryString("LastControllerId", "", last, &size);
    if (!g_TrackGame->GetRegistryFlag("PresetSelected", 0) || strcmp(last, controller))
        g_TrackGame->field_0x33fc->UnknownFunction449220();
    g_TrackGame->field_0x33fc->UnknownFunction448e90(path, -1);
}

// 0x00523130: saves the profile's control file.
void TrackGameMode::SaveControllerChoice() {
    unsigned long size;
    char controller[0x80];
    char path[0x104];

    if (!g_TrackGame->field_0x33fc)
        return;
    if (g_TrackGame->GetRegistryFlag("UseLastController", 0))
        g_TrackGame->SetRegistryFlag("PresetSelected", 1);
    size = 0x80;
    g_TrackGame->GetRegistryString("UseControllerId", "", controller, &size);
    g_TrackGame->SetRegistryString("LastControllerId", controller);
    sprintf(path, "%s\\%s\\%s", "ui\\profile", field_0x00, "control.ctl");
    g_TrackGame->field_0x33fc->UnknownFunction4489e0(path);
}

// 0x005231f0 (TrackGame slot 4): loads the profile; 1 when it has a name.
// Without a profile file the name is cleared and a default TrackGameMode is
// built and destroyed on the stack.
int TrackGameMode::UnknownFunction5231f0() {
    char path[0x104];
    FILE* file;

    sprintf(path, "%s\\%s\\%s.prf", "ui\\profile", field_0x00, field_0x00);
    file = fopen(path, "rb");
    if (!file) {
        strcpy(field_0x00, "");
        TrackGameMode();
        return 0;
    }
    fread(field_0x00, 0x10, 1, file);
    fread(&field_0x9c, 4, 1, file);
    fread(&field_0x6ac, 0x374, 1, file);
    fread(&field_0xa20, 0x2c, 1, file);
    fread(&field_0xa4c, 0x1c, 1, file);
    fread(field_0xa68, 0x30, 1, file);
    fread(field_0x145c, 0x500, 1, file);
    fread(field_0x195c, 0x18, 1, file);
    fread(field_0x1034, 0x5c, 1, file);
    fread(field_0x1090, 0x5c, 1, file);
    fread(&field_0x1a3c, 0xc8, 1, file);
    fread(field_0x1b04, 0xc8, 1, file);
    fread(&field_0x23a8, 0x10, 1, file);
    fread(&field_0x23b8, 4, 1, file);
    fread(&field_0x23bc, 4, 1, file);
    fread(&field_0x23c0, 4, 1, file);
    fread(&field_0x23c4, 4, 1, file);
    fread(&field_0x23c8, 4, 1, file);
    fread(field_0x10, 0x80, 1, file);
    fread(&field_0x29e4, 0x1ec, 1, file);
    fread(&field_0x2bd0, 0x1ec, 1, file);
    fread(field_0x1384, 0x18, 1, file);
    fread(&field_0x1bdc, 4, 1, file);
    fread(&field_0x90, 4, 1, file);
    fread(&field_0x98, 4, 1, file);
    fread(field_0x10f0, 0x294, 1, file);
    fread(&field_0x1bcc, 4, 1, file);
    fclose(file);
    field_0x27f8.field_0x35 = 0;
    ChooseDisplayMode(&field_0x23a8, &field_0xa4c);
    g_TrackGame->ui->field_0x48c = 1;
    UnknownFunction523000();
    if (g_TrackGame->soundInterface) {
        ((PCSoundInterface*)g_TrackGame->soundInterface)->field_0x45c_bit3 = field_0xa34;
        ((PCSoundInterface*)g_TrackGame->soundInterface)->UnknownFunction4be910(22050, 1, field_0xa48 ? 16 : 8);
    }
    field_0x27f8 = field_0x29e4;
    field_0x94 = field_0x98;
    memcpy(field_0xfd8, field_0x1034, sizeof(field_0xfd8));
    memcpy(&field_0x1974, &field_0x1a3c, sizeof(field_0x1974));
    return strcmp(field_0x00, "") != 0;
}

// 0x00523580: saves the profile.
void TrackGameMode::UnknownFunction523580() {
    char path[0x104];
    FILE* file;

    if (!field_0x00[0])
        return;
    sprintf(path, "%s\\%s\\%s.prf", "ui\\profile", field_0x00, field_0x00);
    file = fopen(path, "wb");
    if (!file)
        return;
    fwrite(field_0x00, 0x10, 1, file);
    fwrite(&field_0x9c, 4, 1, file);
    fwrite(&field_0x6ac, 0x374, 1, file);
    fwrite(&field_0xa20, 0x2c, 1, file);
    fwrite(&field_0xa4c, 0x1c, 1, file);
    fwrite(field_0xa68, 0x30, 1, file);
    fwrite(field_0x145c, 0x500, 1, file);
    fwrite(field_0x195c, 0x18, 1, file);
    fwrite(field_0x1034, 0x5c, 1, file);
    fwrite(field_0x1090, 0x5c, 1, file);
    fwrite(&field_0x1a3c, 0xc8, 1, file);
    fwrite(field_0x1b04, 0xc8, 1, file);
    fwrite(&field_0x23a8, 0x10, 1, file);
    fwrite(&field_0x23b8, 4, 1, file);
    fwrite(&field_0x23bc, 4, 1, file);
    fwrite(&field_0x23c0, 4, 1, file);
    fwrite(&field_0x23c4, 4, 1, file);
    fwrite(&field_0x23c8, 4, 1, file);
    fwrite(field_0x10, 0x80, 1, file);
    fwrite(&field_0x29e4, 0x1ec, 1, file);
    fwrite(&field_0x2bd0, 0x1ec, 1, file);
    fwrite(field_0x1384, 0x18, 1, file);
    fwrite(&field_0x1bdc, 4, 1, file);
    fwrite(&field_0x90, 4, 1, file);
    fwrite(&field_0x98, 4, 1, file);
    fwrite(field_0x10f0, 0x294, 1, file);
    fwrite(&field_0x1bcc, 4, 1, file);
    fclose(file);
    g_TrackGame->SetRegistryString("MRUProfile", field_0x00);
    if (g_TrackGame->ui)
        g_TrackGame->ui->field_0x48c = 1;
    SaveControllerChoice();
}

// 0x00523800
int TrackGameMode::WaitForCd() {
    char text[0x80];

    SendMessageA(g_TrackGame->field_0x31c, 0x112, 0xf020, 0);
    while (!FindCdDirectory()) {
        if (LoadStringA(g_TrackGame->field_0x420, 0x13b5, text, 0x80)) {
            ShowCursor(1);
            if (MessageBoxA(g_TrackGame->field_0x31c, text, g_TrackGame->field_0x3a0, 0x15) == 2) {
                SendMessageA(g_TrackGame->field_0x31c, 0x10, 0, 0);
                return 0;
            }
        }
    }
    SendMessageA(g_TrackGame->field_0x31c, 0x112, 0xf120, 0);
    return 1;
}

// 0x00523a60
int TrackGameMode::FindFileDirectory(int value, char* name, const char* kind, char* path) {
    char drive[4];
    char file[0x104];
    char directory[0x100];
    char found[0x104];
    char extension[0x100];
    char base[0x100];
    int result;

    strcpy(path, "");
    if (!*(char*)value)
        sprintf(file, "%s.%s", name, kind);
    else
        sprintf(file, "%s\\%s.%s", (char*)value, name, kind);
    result = UnknownFunction5238f0(file, found);
    if (!result)
        return 0;
    _splitpath(found, drive, directory, base, extension);
    sprintf(path, "%s%s", drive, directory);
    path[strlen(path) - 1] = 0;
    return result;
}

// 0x00523b70
void TrackGameMode::UnknownFunction523b70(char* name) {
    CopySeriesDirectory((short)field_0x27f8.field_0x04, (short)field_0x27f8.field_0x00, name);
}

// 0x00523bb0
int TrackGameMode::CopySeriesDirectory(short a, short b, char* name) {
    strcpy(name, field_0xa0[field_0x27f8.field_0x08]);
    return 1;
}

// 0x00523bf0: looks for the "MCM2" CD.
int TrackGameMode::FindCdDirectory() {
    int index;

    if (field_0x23cc != 2) {
        if (!field_0x25dc)
            return 0;
        field_0x25dc->UnknownFunction449e70();
        if (!field_0x25dc->UnknownFunction44a010("MCM2", &index, 5))
            return 0;
        if (!field_0x25dc->UnknownFunction44a0b0(index, field_0x25d8))
            return 0;
        sprintf(field_0x24d4, "%s%s", field_0x25d8, "game");
    }
    return 1;
}

// 0x00523c90 (TrackGame slot 1)
int TrackGameMode::FindDataDirectory() {
    unsigned long size;
    char text[0x104];

    size = 0x104;
    if (!g_TrackGame->GetRegistryString("InstallType", "", text, &size))
        return 0;
    if (!_stricmp(text, "Full"))
        field_0x23cc = 2;
    else
        field_0x23cc = 1;
    size = 0x104;
    return g_TrackGame->GetRegistryString("HardDriveRootPath", "", field_0x23d0, &size) != 0;
}

// 0x00523d30
int TrackGameMode::OpenHelp(const char* topic, const char* parameters) {
    char name[0x104];
    char directory[0x104];
    char path[0x104];

    if (FindFileDirectory((int)UNKNOWN_HELP_DIRECTORY, (char*)topic, "hlp", directory)) {
        if (!strcmp(UNKNOWN_HELP_DIRECTORY, ""))
            sprintf(name, "%s.hlp", topic);
        else
            sprintf(name, "%s\\%s.hlp", UNKNOWN_HELP_DIRECTORY, topic);
        if (UnknownFunction5238f0(name, path)) {
            SendMessageA(g_TrackGame->field_0x31c, 0x112, 0xf020, 0);
            ShellExecuteA(g_TrackGame->field_0x31c, 0, path, parameters, directory, 1);
            return 1;
        }
    }
    return 0;
}

// 0x00523e50 (KrustyUI 0x004988a0): the bonus track names from PCSched.pb.
void TrackGameMode::UnknownFunction523e50() {
    UnknownParameterBlock block;
    char key[0x100];
    UnknownTextureStream* stream = new(__FILE__, 873) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    int i;

    for (i = 0; i < 6; i++) {
        field_0x1384[i] = 0;
        field_0x139c[i][0] = 0;
    }
    if (stream->UnknownFunction460f50("PCSched.pb", "r", 0)) {
        int track;

        block.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
        block.UnknownFunction4b78f0("Enduro");
        block.UnknownFunction4b7f10("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.UnknownFunction4b7ec0(key, "", field_0x139c[5], -1);
        block.UnknownFunction4b78f0("Baja");
        block.UnknownFunction4b7f10("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.UnknownFunction4b7ec0(key, "", field_0x139c[1], -1);
        block.UnknownFunction4b78f0("Nationals");
        block.UnknownFunction4b7f10("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.UnknownFunction4b7ec0(key, "", field_0x139c[2], -1);
        block.UnknownFunction4b78f0("Supercross");
        block.UnknownFunction4b7f10("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.UnknownFunction4b7ec0(key, "", field_0x139c[3], -1);
    }
    if (stream)
        delete stream;
}

// 0x005240e0
void TrackGameMode::UnknownFunction5240e0(int series) {
    field_0x27f8.field_0x08 = series;
    field_0x6a0 = (int)field_0xa0[series];
}

// 0x00524100
int TrackGameMode::UnknownFunction524100() {
    return field_0x27f8.field_0x08;
}

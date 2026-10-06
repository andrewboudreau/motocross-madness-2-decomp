// UiInfo.cpp -- D:\aardvark\VC\krusty2\uiinfo.cpp (0x00521f30..0x00524106):
// TrackGameMode's methods. The literal __FILE__ (0x00575780) is used at
// 0x00522134 (constructor, line 71), 0x00522d2d..0x00522d9f (lines 383-385)
// and 0x00523e81 (line 873); TypeRegistry.cpp ends at 0x00521f2b and VCR.cpp
// starts at 0x00524110. Near misses and the functions blocked by TrackGame.h's
// layout are in samples/game/UiInfoNearMisses.cpp.
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TrackGame.h"

#include "DebugAlloc.h"
#include "Parameterblocks.h"
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

// TrackGameMode's own layout runs to +0x2dc0 (0x005231f0 builds a
// 0x2dc0-byte TrackGameMode on its stack), but TrackGame.h still declares
// everything past +0xa4c as TrackGame members (TrackGame+0xfc4..+0x3337).
// Until that header is restructured, this TU reaches those members
// through the enclosing TrackGame.
static inline TrackGame* UnknownModeOwner(TrackGameMode* mode) {
    return (TrackGame*)((char*)mode - offsetof(TrackGame, mode));
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
// (the argument is unused; the names are TrackGame+0x19d4).
#define UNKNOWN_LOAD_NAME(index, id)                                              \
    g_UnknownGlobal56e26c->UnknownFunction521970(id, text, 0x80);                 \
    length = strlen(text);                                                        \
    count = length > 0x7f ? 0x7f : length;                                        \
    strncpy(g_UnknownGlobal56e26c->field_0x19d4[index], text, count);             \
    g_UnknownGlobal56e26c->field_0x19d4[index][count] = 0;

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
        options->field_0x24[i] = g_UnknownGlobal56e26c->ui ? g_UnknownGlobal56e26c->ui->field_0x68[1][0][i] : 10;
    for (k = 0; k < 5; k++) {
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 11; i++)
                UnknownModeOwner(this)->field_0x1668[k][j][i] =
                    g_UnknownGlobal56e26c->ui ? g_UnknownGlobal56e26c->ui->field_0x43c[k] : 0;
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
        UnknownModeOwner(this)->field_0x2ebc[i] = entry;
    }
}


// 0x00522d00
int TrackGameMode::UnknownFunction522d00() {
    TrackGame* owner = UnknownModeOwner(this);

    UnknownFunction522e20(0, &owner->field_0xfc4);
    owner->field_0x2b58 = new(__FILE__, 383) DirectoryList;
    owner->field_0x2b5c = new(__FILE__, 384) CombinedDirectoryList;
    owner->field_0x2b60 = new(__FILE__, 385) CombinedDirectoryList;
    if (owner->field_0x2b54 && owner->field_0x2b5c && owner->field_0x2b60)
        return 1;
    return 0;
}

// 0x00522e20
void TrackGameMode::UnknownFunction522e20(const void* guid, int* mode) {
    TrackGame* game = g_UnknownGlobal56e26c;
    int i;

    if (!game->field_0x0c)
        return;
    if (game->field_0x548_bit0 || !guid ||
        memcmp(guid, &game->field_0x0c->field_0x4ac, sizeof(UnknownGuid)) ||
        !game->field_0x0c->field_0x10[*mode].field_0x14) {
        *mode = 0;
        if (g_UnknownGlobal56e26c->field_0x2d0) {
            for (i = 0; i < g_UnknownGlobal56e26c->field_0x0c->field_0x08; i++) {
                UnknownDisplayMode* entry = &g_UnknownGlobal56e26c->field_0x0c->field_0x10[i];
                if (entry->width == 640 && entry->height == 480 && entry->bitDepth == 16 &&
                    entry->field_0x18) {
                    *mode = i;
                    break;
                }
            }
            if (!*mode) {
                for (i = 0; i < g_UnknownGlobal56e26c->field_0x0c->field_0x08; i++) {
                    if (!*mode ||
                        (g_UnknownGlobal56e26c->field_0x0c->field_0x10[i].field_0x18 &&
                         g_UnknownGlobal56e26c->field_0x0c->field_0x10[i].bitDepth == 16 &&
                         g_UnknownGlobal56e26c->field_0x0c->field_0x10[i].height >
                             g_UnknownGlobal56e26c->field_0x0c->field_0x10[*mode].height))
                        *mode = i;
                }
            }
        } else {
            for (i = 0; i < g_UnknownGlobal56e26c->field_0x0c->field_0x08; i++) {
                UnknownDisplayMode* entry = &g_UnknownGlobal56e26c->field_0x0c->field_0x10[i];
                if (entry->width == 640 && entry->height == 480 && entry->bitDepth == 16 &&
                    entry->field_0x14) {
                    *mode = i;
                    break;
                }
            }
            if (!*mode) {
                for (i = 0; i < g_UnknownGlobal56e26c->field_0x0c->field_0x08; i++) {
                    if (!*mode ||
                        (g_UnknownGlobal56e26c->field_0x0c->field_0x10[i].field_0x14 &&
                         g_UnknownGlobal56e26c->field_0x0c->field_0x10[i].bitDepth == 16 &&
                         g_UnknownGlobal56e26c->field_0x0c->field_0x10[i].height >
                             g_UnknownGlobal56e26c->field_0x0c->field_0x10[*mode].height))
                        *mode = i;
                }
            }
        }
    }
    UnknownModeOwner(this)->field_0x2920 = g_UnknownGlobal56e26c->field_0x0c->field_0x4ac;
}

// 0x00523000: loads the profile's control file.
void TrackGameMode::UnknownFunction523000() {
    unsigned long size;
    char controller[0x80];
    char last[0x80];
    char path[0x104];

    if (!g_UnknownGlobal56e26c->field_0x33fc)
        return;
    sprintf(path, "%s\\%s\\%s", "ui\\profile", field_0x00, "control.ctl");
    g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448990(path);
    size = 0x80;
    g_UnknownGlobal56e26c->UnknownVirtualSlot23("UseControllerId", "", controller, &size);
    g_UnknownGlobal56e26c->UnknownVirtualSlot23("LastControllerId", "", last, &size);
    if (!g_UnknownGlobal56e26c->UnknownVirtualSlot22("PresetSelected", 0) || strcmp(last, controller))
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449220();
    g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448e90(path, -1);
}

// 0x00523130: saves the profile's control file.
void TrackGameMode::UnknownFunction523130() {
    unsigned long size;
    char controller[0x80];
    char path[0x104];

    if (!g_UnknownGlobal56e26c->field_0x33fc)
        return;
    if (g_UnknownGlobal56e26c->UnknownVirtualSlot22("UseLastController", 0))
        g_UnknownGlobal56e26c->UnknownVirtualSlot27("PresetSelected", 1);
    size = 0x80;
    g_UnknownGlobal56e26c->UnknownVirtualSlot23("UseControllerId", "", controller, &size);
    g_UnknownGlobal56e26c->UnknownVirtualSlot28("LastControllerId", controller);
    sprintf(path, "%s\\%s\\%s", "ui\\profile", field_0x00, "control.ctl");
    g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction4489e0(path);
}

// 0x00523580: saves the profile.
void TrackGameMode::UnknownFunction523580() {
    TrackGame* owner = UnknownModeOwner(this);
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
    fwrite(&owner->field_0xfc4, 0x1c, 1, file);
    fwrite(owner->field_0xfe0, 0x30, 1, file);
    fwrite(owner->field_0x19d4, 0x500, 1, file);
    fwrite(owner->field_0x1ed4, 0x18, 1, file);
    fwrite(owner->field_0x15ac, 0x5c, 1, file);
    fwrite(owner->field_0x1608, 0x5c, 1, file);
    fwrite(owner->field_0x1fb4, 0xc8, 1, file);
    fwrite(owner->field_0x207c, 0xc8, 1, file);
    fwrite(&owner->field_0x2920, 0x10, 1, file);
    fwrite(&owner->field_0x2930, 4, 1, file);
    fwrite(&owner->field_0x2934, 4, 1, file);
    fwrite(&owner->field_0x2938, 4, 1, file);
    fwrite(&owner->field_0x293c, 4, 1, file);
    fwrite(&owner->field_0x2940, 4, 1, file);
    fwrite(field_0x10, 0x80, 1, file);
    fwrite(owner->field_0x2f5c, 0x1ec, 1, file);
    fwrite(owner->field_0x3148, 0x1ec, 1, file);
    fwrite(owner->field_0x18fc, 0x18, 1, file);
    fwrite(&owner->field_0x2154, 4, 1, file);
    fwrite(&field_0x90, 4, 1, file);
    fwrite(&field_0x98, 4, 1, file);
    fwrite(owner->field_0x1668, 0x294, 1, file);
    fwrite(&owner->field_0x2144, 4, 1, file);
    fclose(file);
    g_UnknownGlobal56e26c->UnknownVirtualSlot28("MRUProfile", field_0x00);
    if (g_UnknownGlobal56e26c->ui)
        g_UnknownGlobal56e26c->ui->field_0x48c = 1;
    UnknownFunction523130();
}

// 0x00523800
int TrackGameMode::UnknownFunction523800() {
    char text[0x80];

    SendMessageA(g_UnknownGlobal56e26c->field_0x31c, 0x112, 0xf020, 0);
    while (!UnknownFunction523bf0()) {
        if (LoadStringA(g_UnknownGlobal56e26c->field_0x420, 0x13b5, text, 0x80)) {
            ShowCursor(1);
            if (MessageBoxA(g_UnknownGlobal56e26c->field_0x31c, text, g_UnknownGlobal56e26c->field_0x3a0, 0x15) == 2) {
                SendMessageA(g_UnknownGlobal56e26c->field_0x31c, 0x10, 0, 0);
                return 0;
            }
        }
    }
    SendMessageA(g_UnknownGlobal56e26c->field_0x31c, 0x112, 0xf120, 0);
    return 1;
}

// 0x00523a60
int TrackGameMode::UnknownFunction523a60(int value, char* name, const char* kind, char* path) {
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
    TrackGame* owner = UnknownModeOwner(this);
    UnknownFunction523bb0((short)owner->field_0x2d74, (short)owner->field_0x2d70, name);
}

// 0x00523bb0
int TrackGameMode::UnknownFunction523bb0(short a, short b, char* name) {
    strcpy(name, field_0xa0[UnknownModeOwner(this)->field_0x2d78]);
    return 1;
}

// 0x00523bf0: looks for the "MCM2" CD.
int TrackGameMode::UnknownFunction523bf0() {
    TrackGame* owner = UnknownModeOwner(this);
    int index;

    if (owner->field_0x2944 != 2) {
        if (!owner->field_0x2b54)
            return 0;
        owner->field_0x2b54->UnknownFunction449e70();
        if (!owner->field_0x2b54->UnknownFunction44a010("MCM2", &index, 5))
            return 0;
        if (!owner->field_0x2b54->UnknownFunction44a0b0(index, owner->field_0x2b50))
            return 0;
        sprintf(owner->field_0x2a4c, "%s%s", owner->field_0x2b50, "game");
    }
    return 1;
}

// 0x00523c90 (TrackGame slot 1)
int TrackGameMode::UnknownFunction523c90() {
    TrackGame* owner = UnknownModeOwner(this);
    unsigned long size;
    char text[0x104];

    size = 0x104;
    if (!g_UnknownGlobal56e26c->UnknownVirtualSlot23("InstallType", "", text, &size))
        return 0;
    if (!_stricmp(text, "Full"))
        owner->field_0x2944 = 2;
    else
        owner->field_0x2944 = 1;
    size = 0x104;
    return g_UnknownGlobal56e26c->UnknownVirtualSlot23("HardDriveRootPath", "", owner->field_0x2948, &size) != 0;
}

// 0x00523d30
int TrackGameMode::UnknownFunction523d30(const char* topic, const char* parameters) {
    char name[0x104];
    char directory[0x104];
    char path[0x104];

    if (UnknownFunction523a60((int)UNKNOWN_HELP_DIRECTORY, (char*)topic, "hlp", directory)) {
        if (!strcmp(UNKNOWN_HELP_DIRECTORY, ""))
            sprintf(name, "%s.hlp", topic);
        else
            sprintf(name, "%s\\%s.hlp", UNKNOWN_HELP_DIRECTORY, topic);
        if (UnknownFunction5238f0(name, path)) {
            SendMessageA(g_UnknownGlobal56e26c->field_0x31c, 0x112, 0xf020, 0);
            ShellExecuteA(g_UnknownGlobal56e26c->field_0x31c, 0, path, parameters, directory, 1);
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
        UnknownModeOwner(this)->field_0x18fc[i] = 0;
        UnknownModeOwner(this)->field_0x1914[i][0] = 0;
    }
    if (stream->UnknownFunction460f50("PCSched.pb", "r", 0)) {
        int track;

        block.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
        block.UnknownFunction4b78f0("Enduro");
        block.UnknownFunction4b7f10("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.UnknownFunction4b7ec0(key, "", UnknownModeOwner(this)->field_0x1914[5], -1);
        block.UnknownFunction4b78f0("Baja");
        block.UnknownFunction4b7f10("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.UnknownFunction4b7ec0(key, "", UnknownModeOwner(this)->field_0x1914[1], -1);
        block.UnknownFunction4b78f0("Nationals");
        block.UnknownFunction4b7f10("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.UnknownFunction4b7ec0(key, "", UnknownModeOwner(this)->field_0x1914[2], -1);
        block.UnknownFunction4b78f0("Supercross");
        block.UnknownFunction4b7f10("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.UnknownFunction4b7ec0(key, "", UnknownModeOwner(this)->field_0x1914[3], -1);
    }
    if (stream)
        delete stream;
}

// 0x005240e0
void TrackGameMode::UnknownFunction5240e0(int series) {
    UnknownModeOwner(this)->field_0x2d78 = series;
    field_0x6a0 = (int)field_0xa0[series];
}

// 0x00524100
int TrackGameMode::UnknownFunction524100() {
    return UnknownModeOwner(this)->field_0x2d78;
}

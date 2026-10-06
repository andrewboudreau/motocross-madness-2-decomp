// Near-miss uiinfo.cpp candidates (0x00521f30..0x00524106),
// kept out of src/reconstructed/UiInfo.cpp until they match.
//
// TrackGameMode::UnknownFunction522440 (0x00522440, 427 bytes): restores the
// default settings. Calls, constants and stores all match; 355 of 427 bytes.
// Retail keeps 1 in ecx and 2 in edx from the start (and reuses edx for the
// final +0x98/+0x94 stores), computes &field_0x27f8 before the first
// stores and places the second 0x1ec-byte copy after the +0x196c store; VC6
// here picks eax for 1 and schedules the copies and the memset differently.
// Store-order permutations, struct-assignment copies and memset forms tried.
//
// TrackGameMode::UnknownFunction522720 (0x00522720, 92 bytes): 54 of 101.
// Retail holds 1 in ecx and 0 in edx, then advances eax by 0x360 and clears
// a fresh ecx for the last five fields (an inline helper on a sub-object);
// VC6 here swaps ecx/edx and folds the +0x360 offsets. Store-order, inline
// member/static helper and local-constant forms tried.
//
// TrackGameMode::UnknownFunction5238f0 (0x005238f0, 353 bytes): 189 of 379.
// Retail shares one `return 0` epilogue and keeps fopen's result in eax until
// the found block (mov edx, eax there); the nested/goto/condition forms tried
// share the epilogue (194 of 353) but still copy the result into edx right
// after each fopen.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x00522440: restores the default settings.
void TrackGameMode::UnknownFunction522440() {
    UnknownFunction522720((UnknownTrackGameModeOptions6ac*)&field_0x6ac);
    UnknownFunction522780((UnknownTrackGameModeOptionsA20*)&field_0xa20);
    UnknownFunction5227d0((UnknownTrackGameModeOptionsA4c*)&field_0xa4c);
    g_UnknownGlobal56e26c->mode.UnknownFunction522e20(0, &field_0xa4c);
    UnknownFunction522800((UnknownTrackGameModeOptionsA68*)field_0xa68);
    UnknownFunction522840(field_0x145c);
    UnknownFunction522cb0((UnknownTrackGameModeOptionsFd8*)field_0xfd8);
    UnknownFunction522cb0((UnknownTrackGameModeOptionsFd8*)field_0x1034);
    UnknownFunction522cb0((UnknownTrackGameModeOptionsFd8*)field_0x1090);
    UnknownFunction522cd0();
    field_0x27f8.field_0x30 = 1;
    field_0x27f8.field_0x10 = 1;
    field_0x27f8.field_0x14 = 1;
    field_0x27f8.field_0x0c = 2;
    field_0x27f8.field_0x00 = 0;
    field_0x27f8.field_0x20 = 5;
    field_0x27f8.field_0x140 = 15.0f;
    field_0x27f8.field_0x2c = 0;
    field_0x27f8.field_0x2d = 30;
    field_0x27f8.field_0x24 = 0;
    field_0x27f8.field_0x28 = 0;
    field_0x27f8.field_0x18 = 0;
    field_0x27f8.field_0x1c = 4;
    field_0x23b8 = 0;
    memcpy(&field_0x2bd0, &field_0x27f8, sizeof(field_0x2bd0));
    memcpy(&field_0x29e4, &field_0x27f8, sizeof(field_0x29e4));
    *(unsigned char*)&field_0x1970 &= ~1;
    field_0x23bc = 10.3f;
    field_0x23c0 = 0.47123885f;
    field_0x23c4 = 1.0646508f;
    field_0x23c8 = 85.0f;
    field_0x195c[4] = field_0xa64;
    field_0x195c[3] = field_0xa64;
    field_0x195c[2] = field_0xa64;
    field_0x195c[1] = field_0xa64;
    field_0x1bdc = 0;
    field_0x90 = 0;
    field_0x2dbc = 0;
    field_0x1bcc = 101;
    memset(field_0x1384, 0, sizeof(field_0x1384));
    field_0x98 = 2;
    field_0x94 = 2;
}

// 0x00522720
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
    options->field_0x360.UnknownReset();
}

// 0x005238f0 (TrackGame slot 18): finds `name` under the installed data
// directory, else on the CD.
int TrackGameMode::UnknownFunction5238f0(const char* name, char* path) {
    char buffer[0x104];
    int skip;
    FILE* file;

    strcpy(path, "");
    skip = strlen(field_0x23d0);
    if (!_strnicmp(name, field_0x23d0, skip)) {
        skip++;
    } else {
        skip = strlen(field_0x24d4);
        if (skip && !_strnicmp(name, field_0x24d4, skip))
            skip++;
        else
            skip = 0;
    }
    sprintf(buffer, "%s\\%s", field_0x23d0, name + skip);
    file = fopen(buffer, "r");
    if (!file) {
        if (field_0x23cc == 2)
            return 0;
        if (!UnknownFunction523bf0() && !UnknownFunction523800())
            return 0;
        sprintf(buffer, "%s\\%s", field_0x24d4, name + skip);
        file = fopen(buffer, "r");
        if (!file)
            return 0;
    }
    strcpy(path, buffer);
    fclose(file);
    return 1;
}

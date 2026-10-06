// Near-miss and blocked uiinfo.cpp candidates (0x00521f30..0x00524106),
// kept out of src/reconstructed/UiInfo.cpp until they match.
//
// TrackGameMode::UnknownFunction522440 (0x00522440, 427 bytes): restores the
// default settings. Calls, constants and stores all match; 355 of 427 bytes.
// Retail keeps 1 in ecx and 2 in edx from the start (and reuses edx for the
// final +0x98/+0x94 stores), computes &TrackGame+0x2d70 before the first
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
//
// Blocked by TrackGame.h's layout (see UiInfo.cpp): TrackGameMode is really
// 0x2dc0 bytes and owns TrackGame+0xfc4..+0x3337, including five
// SessionInfoType records at its +0xa98 (eh vector constructor/destructor
// iterators 0x00536234/0x00536140 with 0x00523b90/0x00523c80) and eight
// UnknownTrackGameRacerSlot records with a constructor (0x00521f30) at its
// +0x1be4. Declaring those types with constructors in TrackGame would change
// TrackGame's own constructor, so these cannot match yet:
//   0x00521f30 UnknownTrackGameRacerSlot constructor (sketched in a comment below)
//   0x00522060 TrackGameMode constructor, unwind funclet 0x00522420
//   0x005225f0 TrackGameMode destructor
//   0x00522680 TrackGameMode slot-4 reset (temporary racer records)
//   0x005231f0 profile load (constructs a temporary TrackGameMode)
//   0x00523b90 SessionInfoType's inline constructor, emitted here
// The sketches below record their behaviour.
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/TrackGame.h"

static inline TrackGame* UnknownModeOwner(TrackGameMode* mode) {
    return (TrackGame*)((char*)mode - offsetof(TrackGame, mode));
}

// 0x00522440: restores the default settings.
void TrackGameMode::UnknownFunction522440() {
    TrackGame* owner = UnknownModeOwner(this);
    UnknownFunction522720((UnknownTrackGameModeOptions6ac*)&field_0x6ac);
    UnknownFunction522780((UnknownTrackGameModeOptionsA20*)&field_0xa20);
    UnknownFunction5227d0((UnknownTrackGameModeOptionsA4c*)&owner->field_0xfc4);
    g_UnknownGlobal56e26c->mode.UnknownFunction522e20(0, &owner->field_0xfc4);
    UnknownFunction522800((UnknownTrackGameModeOptionsA68*)owner->field_0xfe0);
    UnknownFunction522840(owner->field_0x19d4);
    UnknownFunction522cb0((UnknownTrackGameModeOptionsFd8*)owner->field_0x1550);
    UnknownFunction522cb0((UnknownTrackGameModeOptionsFd8*)owner->field_0x15ac);
    UnknownFunction522cb0((UnknownTrackGameModeOptionsFd8*)owner->field_0x1608);
    UnknownFunction522cd0();
    owner->field_0x2da0 = 1;
    owner->field_0x2d80 = 1;
    owner->field_0x2d84 = 1;
    owner->field_0x2d7c = 2;
    owner->field_0x2d70 = 0;
    owner->field_0x2d90 = 5;
    owner->field_0x2eb0 = 15.0f;
    owner->field_0x2d9c = 0;
    owner->field_0x2d9d = 30;
    owner->field_0x2d94 = 0;
    owner->field_0x2d98 = 0;
    owner->field_0x2d88 = 0;
    owner->field_0x2d8c = 4;
    owner->field_0x2930 = 0;
    memcpy(owner->field_0x3148, &owner->field_0x2d70, sizeof(owner->field_0x3148));
    memcpy(owner->field_0x2f5c, &owner->field_0x2d70, sizeof(owner->field_0x2f5c));
    *(unsigned char*)&owner->field_0x1ee8 &= ~1;
    owner->field_0x2934 = 10.3f;
    owner->field_0x2938 = 0.47123885f;
    owner->field_0x293c = 1.0646508f;
    owner->field_0x2940 = 85.0f;
    owner->field_0x1ed4[4] = owner->field_0xfdc;
    owner->field_0x1ed4[3] = owner->field_0xfdc;
    owner->field_0x1ed4[2] = owner->field_0xfdc;
    owner->field_0x1ed4[1] = owner->field_0xfdc;
    owner->field_0x2154 = 0;
    field_0x90 = 0;
    owner->field_0x3334 = 0;
    owner->field_0x2144 = 101;
    memset(owner->field_0x18fc, 0, sizeof(owner->field_0x18fc));
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
    TrackGame* owner = UnknownModeOwner(this);
    char buffer[0x104];
    int skip;
    FILE* file;

    strcpy(path, "");
    skip = strlen(owner->field_0x2948);
    if (!_strnicmp(name, owner->field_0x2948, skip)) {
        skip++;
    } else {
        skip = strlen(owner->field_0x2a4c);
        if (skip && !_strnicmp(name, owner->field_0x2a4c, skip))
            skip++;
        else
            skip = 0;
    }
    sprintf(buffer, "%s\\%s", owner->field_0x2948, name + skip);
    file = fopen(buffer, "r");
    if (!file) {
        if (owner->field_0x2944 == 2)
            return 0;
        if (!UnknownFunction523bf0() && !UnknownFunction523800())
            return 0;
        sprintf(buffer, "%s\\%s", owner->field_0x2a4c, name + skip);
        file = fopen(buffer, "r");
        if (!file)
            return 0;
    }
    strcpy(path, buffer);
    fclose(file);
    return 1;
}

// 0x00521f30 (UnknownTrackGameRacerSlot's constructor; not declared, see above):
//     field_0xd4 = 0;
//     strncpy(field_0xdc, "", 0) with the 15-character cap; field_0xdc[0] = 0;
//     field_0xc8 = 0; field_0xcc = 0;
//     field_0x80[0] = 0; field_0x40[0] = 0; field_0x00[0] = 0;
//     field_0xd0 = FLT_MAX (0x7f7fffff); byte +0xf4 = 1.

// 0x00522060 (sketch; the session array and racer records are missing).
TrackGameMode::TrackGameMode() {
    TrackGame* owner = UnknownModeOwner(this);
    int i;

    // eh vector constructor iterator: owner->field_0x1010[5] (SessionInfoType)
    // 8 x 0x00521f30 on owner->field_0x215c
    for (i = 0; i < 8; i++) {
        owner->field_0x2ebc[i].field_0x00 = 0;
        owner->field_0x2ebc[i].field_0x04 = 0;
    }
    // ... and the same eight-entry arrays at TrackGame+0x30a8 and +0x3294.
    UnknownFunction522440();
    owner->field_0x291c = 0;
    field_0x6a8 = 1;
    memset(&owner->field_0x2920, 0, sizeof(owner->field_0x2920));
    owner->field_0x2944 = 2;
    strcpy(owner->field_0x2948, "");
    strcpy(owner->field_0x2a4c, "");
    strcpy(owner->field_0x2b50, "");
    owner->field_0x2b54 = 0;
    owner->field_0x2b54 = new(__FILE__, 71) UnknownDriveList;   // the folded two-field reset 0x004676a0
    if (!owner->field_0x2b54->UnknownFunction449e60()) {
        delete owner->field_0x2b54;
        owner->field_0x2b54 = 0;
    }
    owner->field_0x2b58 = 0;
    owner->field_0x2b5c = 0;
    owner->field_0x2b60 = 0;
    field_0x9c = 1;
    owner->field_0x2b64[0] = 0;
    owner->field_0x2c68 = 0;
    owner->field_0x2c6c[0] = 0;
    ((UnknownTrackGameRacerSlot*)owner->field_0x1eec)->field_0xc0 = -1;
    owner->field_0x2144 = (int)(rand() * (1.0f / 32768.0f) * 999.0f);
    if (owner->field_0x2144 <= 99)
        owner->field_0x2144 += 100;
    for (i = 0; i < 6; i++) {
        owner->field_0x18fc[i] = 0;
        owner->field_0x1914[i][0] = 0;
    }
    owner->field_0x2148 = -1;
    owner->field_0x214c = -1;
    owner->field_0x2150 = 0;
    owner->field_0x2158 = 0;
    owner->field_0x1664 = 0;
    // field_0xa0[0..5] = "Teraform\\Quarries", "Teraform\\Baja",
    // "Teraform\\National", "Teraform\\SX", "Teraform\\Tag",
    // "Teraform\\Enduro" (strlen-capped strncpy, at most 255 chars).
}

// 0x005225f0 (sketch; the session array's destructor iterator is missing).
TrackGameMode::~TrackGameMode() {
    TrackGame* owner = UnknownModeOwner(this);

    if (owner->field_0x2b58)
        delete owner->field_0x2b58;
    if (owner->field_0x2b5c)
        delete owner->field_0x2b5c;
    if (owner->field_0x2b60)
        delete owner->field_0x2b60;
}

// 0x005231f0 (sketch): loads the profile; when it is missing, clears the
// name and builds (and destroys) a temporary TrackGameMode, which needs the
// real 0x2dc0-byte class. Reads the same blocks 0x00523580 writes, then:
//     owner->field_0x2da5 = 0;
//     UnknownFunction522e20(&owner->field_0x2920, &owner->field_0xfc4);
//     g_UnknownGlobal56e26c->ui->field_0x48c = 1;          (no null test)
//     UnknownFunction523000();
//     sound (Game+0x04): bit 3 of +0x45c = field_0xa34 & 1, then
//         0x004be910(22050, 1, field_0xa48 ? 16 : 8);
//     +0x2d70 block = +0x2f5c block; field_0x94 = field_0x98;
//     +0x1550 = +0x15ac (0x5c bytes); +0x1eec = +0x1fb4 (0xc8 bytes);
//     return strcmp(field_0x00, "") != 0;

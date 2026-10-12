// Near-miss uiinfo.cpp candidates (0x00521f30..0x00524106),
// kept out of src/reconstructed/UiInfo.cpp until they match.
//
// TrackGameMode::ResetDefaults (0x00522440, 427 bytes): restores the
// default settings. Calls, constants and stores all match; 383 of 427 bytes.
// The leading +0x27f8 store order 0x30, 0x0c, 0x10, 0x00, 0x14 gives
// retail's ecx = 1 / edx = 2 constants and &field_0x27f8 setup, and an
// explicit zeroing loop for bonusTrackUnlocked (not memset) gives retail's rep
// stosd placement. Left: retail schedules the +0x2828 store after `mov
// edx, 2` and places the second 0x1ec-byte rep movsd after the +0x196c
// store. Exhaustive sweeps (tools/sweep_variants.py) found nothing better:
// all 5040 orders of the first seven stores x memcpy/struct-assignment
// copies; 120 orders of the five +0x27f8 stores x 15 positions of the
// second memcpy x memset/loop zeroing; memcpy vs struct assignment for
// both copies at every position (moving the second copy later costs
// 100+ bytes). Earlier passes also tried a reference or pointer alias of
// field_0x27f8 for the stores and the memcpy sources (32 combinations).
// An inline settings-reset helper over the 13 stores (720 variants) turns
// them into [eax + N] stores (174, 394 bytes).
//
// TrackGameMode::FindDataFile (0x005238f0, 353 bytes): 190 of 357.
// The found block written once after the CD retry (`if (file) { ...; return
// 1; } return 0;`) gives retail's block layout and its single `return 0`
// tail. Left: retail keeps fopen's result in eax until the found block (mov
// edx, eax there); VC6 here gives `file` edx for its whole life and copies
// it right after each fopen. Nested/goto/condition forms, a combined
// `!file && ... && (FindCdDirectory() || WaitForCd())` retry and an inline
// found helper taking the file by value were tried, and so were `!file`/
// `file == 0` x three second-fopen forms x five found-block forms (30, best
// 190; an `int found` flag scores 191 but grows the function to 387 bytes).
// The found block written out in both fopen branches is not merged by VC6
// (193 of 422).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x00522440: restores the default settings.
void TrackGameMode::ResetDefaults() {
    ResetInterfaceOptions((UnknownTrackGameModeOptions6ac*)&instrumentOverlayOn);
    ResetSoundOptions((UnknownTrackGameModeOptionsA20*)&field_0xa20);
    ResetGraphicsOptions((UnknownTrackGameModeOptionsA4c*)&displayModeIndex);
    g_TrackGame->mode.ChooseDisplayMode(0, &displayModeIndex);
    UnknownFunction522800((UnknownTrackGameModeOptionsA68*)field_0xa68);
    ReloadResourceStrings(resourceStrings);
    UnknownFunction522cb0((UnknownTrackGameModeOptionsFd8*)garageOptions);
    UnknownFunction522cb0((UnknownTrackGameModeOptionsFd8*)optionsBackup);
    UnknownFunction522cb0((UnknownTrackGameModeOptionsFd8*)field_0x1090);
    UnknownFunction522cd0();
    raceSettings.field_0x30 = 1;
    raceSettings.field_0x0c = 2;
    raceSettings.treeCollision = 1;
    raceSettings.raceMode = 0;
    raceSettings.riderCollision = 1;
    raceSettings.lapCount = 5;
    raceSettings.timeLimitMinutes = 15.0f;
    raceSettings.field_0x2c = 0;
    raceSettings.field_0x2d = 30;
    raceSettings.aiRacerCount = 0;
    raceSettings.field_0x28 = 0;
    raceSettings.shortTimeLimit = 0;
    raceSettings.maxBikeClass = 4;
    field_0x23b8 = 0;
    memcpy(&field_0x2bd0, &raceSettings, sizeof(field_0x2bd0));
    memcpy(&savedSettings, &raceSettings, sizeof(savedSettings));
    *(unsigned char*)&bikeUnlockFlags &= ~1;
    field_0x23bc = 10.3f;
    field_0x23c0 = 0.47123885f;
    field_0x23c4 = 1.0646508f;
    field_0x23c8 = 85.0f;
    videoSettings[4] = detailLevel;
    videoSettings[3] = detailLevel;
    videoSettings[2] = detailLevel;
    videoSettings[1] = detailLevel;
    field_0x1bdc = 0;
    field_0x90 = 0;
    saveReplay = 0;
    plateNumber = 101;
    for (int i = 0; i < 6; i++)
        bonusTrackUnlocked[i] = 0;
    savedDifficulty = 2;
    difficulty = 2;
}

// 0x005238f0 (TrackGame slot 18): finds `name` under the installed data
// directory, else on the CD.
int TrackGameMode::FindDataFile(const char* name, char* path) {
    char buffer[0x104];
    int skip;
    FILE* file;

    strcpy(path, "");
    skip = strlen(installDirectory);
    if (!_strnicmp(name, installDirectory, skip)) {
        skip++;
    } else {
        skip = strlen(cdDataDirectory);
        if (skip && !_strnicmp(name, cdDataDirectory, skip))
            skip++;
        else
            skip = 0;
    }
    sprintf(buffer, "%s\\%s", installDirectory, name + skip);
    file = fopen(buffer, "r");
    if (!file) {
        if (installType == 2)
            return 0;
        if (!FindCdDirectory() && !WaitForCd())
            return 0;
        sprintf(buffer, "%s\\%s", cdDataDirectory, name + skip);
        file = fopen(buffer, "r");
    }
    if (file) {
        strcpy(path, buffer);
        fclose(file);
        return 1;
    }
    return 0;
}

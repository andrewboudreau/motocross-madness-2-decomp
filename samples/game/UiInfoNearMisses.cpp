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
// Session 21 (60 forms, best 383): chained stores for the three 1s
// (`gateSeed = treeCollision = riderCollision = 1`, or two of them),
// `races = savedDifficulty = difficulty = 2`, the four detail levels chained
// in either direction, through a `level` local or a countdown loop, and
// `savedDifficulty = difficulty = 2` in either order.
// Session 24, unchanged (383): `int one = 1` and/or `int two = 2` locals
// (declared at the top or before the stores) and a `detail` local; the two
// difficulty stores moved first (209), `savedDifficulty = raceSettings.races`
// (204) and `raceSettings.races = difficulty = 2` (332) are worse. A
// preceding function of any size leaves the code unchanged, so the placement
// of the second `rep movsd` is decided inside this function.
//
// TrackGameMode::FindDataFile (0x005238f0, 355 bytes): candidate 357 bytes.
// The old 353-byte extent cut off the final ret 8 operand; see docs/UIINFO.md.
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
// Session 21 (48 forms, best 190): an inline UseFile(path, buffer, file)
// found helper returned from every exit and an inline OpenIn(buffer, dir,
// name) sprintf+fopen helper, in the early-return, nested and combined-retry
// layouts, with `file` declared at the top or at the first fopen.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/TrackGame.h"

// 0x00522440: restores the default settings.
void TrackGameMode::ResetDefaults() {
    ResetInterfaceOptions((UnknownTrackGameModeOptions6ac*)&instrumentOverlayOn);
    ResetSoundOptions((UnknownTrackGameModeOptionsA20*)&cdAudioOn);
    ResetGraphicsOptions((UnknownTrackGameModeOptionsA4c*)&displayModeIndex);
    g_TrackGame->mode.ChooseDisplayMode(0, &displayModeIndex);
    ResetControlOptions((UnknownTrackGameModeOptionsA68*)responseSliders);
    ReloadResourceStrings(resourceStrings);
    ResetGarageOptions((UnknownTrackGameModeOptionsFd8*)garageOptions);
    ResetGarageOptions((UnknownTrackGameModeOptionsFd8*)optionsBackup);
    ResetGarageOptions((UnknownTrackGameModeOptionsFd8*)multiplayerGarageOptions);
    ResetSettingsEntries();
    raceSettings.gateSeed = 1;
    raceSettings.races = 2;
    raceSettings.treeCollision = 1;
    raceSettings.raceMode = 0;
    raceSettings.riderCollision = 1;
    raceSettings.lapCount = 5;
    raceSettings.timeLimitMinutes = 15.0f;
    raceSettings.randomGates = 0;
    raceSettings.gateCount = 30;
    raceSettings.aiRacerCount = 0;
    raceSettings.multiplayerAiRacerCount = 0;
    raceSettings.shortTimeLimit = 0;
    raceSettings.maxBikeClass = 4;
    savedCameraState = 0;
    memcpy(&multiplayerSettings, &raceSettings, sizeof(multiplayerSettings));
    memcpy(&savedSettings, &raceSettings, sizeof(savedSettings));
    *(unsigned char*)&bikeUnlockFlags &= ~1;
    savedCameraDistance = 10.3f;
    savedCameraPitch = 0.47123885f;
    savedCameraYaw = 1.0646508f;
    savedCameraFieldOfView = 85.0f;
    videoSettings[4] = detailLevel;
    videoSettings[3] = detailLevel;
    videoSettings[2] = detailLevel;
    videoSettings[1] = detailLevel;
    field_0x1bdc = 0;
    nameTagShade = 0;
    saveReplay = 0;
    plateNumber = 101;
    for (int i = 0; i < 6; i++)
        bonusTrackUnlocked[i] = 0;
    savedDifficulty = 2;
    difficulty = 2;
}

// Session 25 for 0x005238f0 (192 forms, none above 190/357): the fopen
// assigned in the condition, `first`/`second` result locals, inline
// OpenText/CloseFile helpers, a `found` copy, `!= NULL`, an early `return 0`
// after the second fopen, six spellings of the found block, and 0..8 dead
// initialised locals.
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

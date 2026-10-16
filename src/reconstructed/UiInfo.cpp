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

    networkPlayerId = 0;
    length = strlen("");
    count = length > 0xf ? 0xf : length;
    strncpy(playerName, "", count);
    playerName[count] = 0;
    hasLeft = 0;
    isReady = 0;
    riderName[0] = 0;
    bikeName[0] = 0;
    bikeModelName[0] = 0;
    lowestMessageDelay = FLT_MAX;
    hasMprSkin = 1;
}

// 0x00521fb0
void UnknownTrackGameRacerSlot::SetBikeAndRiderNames(void* modelText, void* bikeText, void* riderText) {
    int length = strlen((char*)modelText);
    int count = length > 0x3f ? 0x3f : length;
    strncpy(bikeModelName, (char*)modelText, count);
    bikeModelName[count] = 0;
    length = strlen((char*)bikeText);
    count = length > 0x3f ? 0x3f : length;
    strncpy(bikeName, (char*)bikeText, count);
    bikeName[count] = 0;
    length = strlen((char*)riderText);
    count = length > 0x3f ? 0x3f : length;
    strncpy(riderName, (char*)riderText, count);
    riderName[count] = 0;
}

// 0x00522050
void UnknownTrackGameRacerSlot::ClearLeft() {
    hasLeft = 0;
}

// 0x00522060
#define UNKNOWN_SET_DIRECTORY(index, name)                                        \
    length = strlen(name);                                                        \
    count = length > 0xff ? 0xff : length;                                        \
    strncpy(seriesDirectories[index], name, count);                                      \
    seriesDirectories[index][count] = 0;

TrackGameMode::TrackGameMode() {
    int i;
    int length;
    int count;

    ResetDefaults();
    networkGame = 0;
    visualCuesOn = 1;
    memset(&displayAdapterGuid, 0, sizeof(displayAdapterGuid));
    installType = 2;
    strcpy(installDirectory, "");
    strcpy(cdDataDirectory, "");
    strcpy(cdDriveRoot, "");
    driveList = 0;
    driveList = new(__FILE__, 71) UnknownDriveList;
    if (!driveList->Rescan()) {
        delete driveList;
        driveList = 0;
    }
    highScoreDirectoryList = 0;
    trackDirectoryList = 0;
    field_0x25e8 = 0;
    showNewbieDialog = 1;
    ghostFile[0] = 0;
    saveGhost = 0;
    replayFile[0] = 0;
    racerChoice.modelIndex = -1;
    plateNumber = (int)((rand() * (1.0f / 32768.0f)) * 999.0f);
    if (plateNumber <= 99)
        plateNumber += 100;
    for (i = 0; i < 6; i++) {
        bonusTrackUnlocked[i] = 0;
        bonusTrackNames[i][0] = 0;
    }
    lobbyRank = -1;
    lobbyPresetFlags = -1;
    maxOpponents = 0;
    racerCount = 0;
    championshipInProgress = 0;
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
    if (highScoreDirectoryList)
        delete highScoreDirectoryList;
    if (trackDirectoryList)
        delete trackDirectoryList;
    if (field_0x25e8)
        delete field_0x25e8;
}

// 0x00522680 (TrackGame slot 4): clears the network race state.
int TrackGameMode::ResetNetworkRace() {
    int i;

    field_0x6a4 = 0;
    selectedSession = 0;
    racerCount = 0;
    if (raceSettings.raceMode != 2) {
        for (i = 0; i < 8; i++)
            racers[i] = UnknownTrackGameRacerSlot();
    }
    for (i = 0; i < 8; i++)
        raceSettings.gridOrder[i] = UnknownTrackGameModeEntry();
    raceSettings.networkPlayerCount = 0;
    return 1;
}

// 0x00522720: the defaults of the TrackGameMode+0x6ac options (overlay,
// name tags, intro movie; the name is inference); the last
// 0x14 bytes are cleared with an inline memset.
void TrackGameMode::ResetInterfaceOptions(UnknownTrackGameModeOptions6ac* options) {
    options->instrumentOverlayOn = 1;
    options->showRaceStatistics = 1;
    options->uiAnimations = 1;
    options->chatInputLine = 0;
    options->showNameTags = 1;
    options->showRiderNames = 1;
    options->showRiderPositions = 1;
    options->showPodiums = 1;
    options->field_0x30 = 2;
    options->field_0x34 = 0;
    options->field_0x38 = 0;
    options->field_0x3c = 0;
    options->field_0x40 = 0;
    options->field_0x44 = 0;
    options->introMovieOn = 1;
    options->playerIndicator = 1;
    options->toolTips = 1;
    options->wreckResetOnTrack = 1;
    memset(&options->garageCurveChoice, 0, sizeof(options->garageCurveChoice));
}

// 0x00522780: the defaults of +0xa20..+0xa4b, mostly sound settings (sound
// on at +0xa28, volume 100 at +0xa40, 8-bit at +0xa48); the name is inference.
void TrackGameMode::ResetSoundOptions(UnknownTrackGameModeOptionsA20* options) {
    options->field_0x00 = 0;
    options->soundOn = 1;
    options->uiSounds = 1;
    options->field_0x0c = 1;
    options->soundVolume = 100;
    options->field_0x24 = 100;
    options->soundHardware = 0;
    options->uiVolume = 100;
    options->eaxEnabled = 0;
    options->use16BitSound = 0;
    options->positionSounds = 1;
}

// 0x005227d0: the defaults of +0xa4c..+0xa67: display mode 0, shadows,
// particles and sky cube on, detail level 6.
void TrackGameMode::ResetGraphicsOptions(UnknownTrackGameModeOptionsA4c* options) {
    options->displayMode = 0;
    options->shadows = 1;
    options->particles = 1;
    options->field_0x0c = 0;
    options->skyCube = 1;
    options->detailLevel = 6;
}

// 0x00522800
void TrackGameMode::ResetControlOptions(UnknownTrackGameModeOptionsA68* options) {
    options->responseSliders[0] = 50;
    options->feedback = 50;
    options->joystickOn = 1;
    options->responseSliders[1] = 50;
    options->responseSliders[2] = 50;
    options->responseSliders[3] = 50;
    options->responseSliders[4] = 50;
    options->responseSliders[5] = 50;
    options->responseSliders[6] = 50;
    options->gasGyro = 0;
    options->brakeGyro = 1;
    options->analogAxes = 1;
}

// 0x00522840: reloads the ten names from string resources 0x1417..0x1420
// (the argument is unused; the names are TrackGameMode+0x145c).
#define UNKNOWN_LOAD_NAME(index, id)                                              \
    g_TrackGame->LoadResourceString(id, text, 0x80);                 \
    length = strlen(text);                                                        \
    count = length > 0x7f ? 0x7f : length;                                        \
    strncpy(g_TrackGame->mode.resourceStrings[index], text, count);             \
    g_TrackGame->mode.resourceStrings[index][count] = 0;

void TrackGameMode::ReloadResourceStrings(char (*names)[0x80]) {
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
void TrackGameMode::CopyGarageTables(UnknownTrackGameModeOptionsFd8* options) {
    int i;
    int j;
    int k;

    for (i = 0; i < 11; i++)
        options->eqBands[i] = g_TrackGame->ui ? g_TrackGame->ui->standardCurves[1][0][i] : 10;
    for (k = 0; k < 5; k++) {
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 11; i++)
                customGarageCurves[k][j][i] =
                    g_TrackGame->ui ? g_TrackGame->ui->minRange[k] : 0;
        }
    }
}

// 0x00522c70
void TrackGameMode::ResetSuspension(UnknownTrackGameModeOptionsFd8* options) {
    options->suspension[0] = 0.5f;
    options->suspension[1] = 0.5f;
    options->suspension[2] = 0.5f;
    options->suspension[3] = 0.5f;
    options->suspension[4] = 0.5f;
    options->suspension[5] = 0.5f;
}

// 0x00522c90
void TrackGameMode::ResetEngineOptions(UnknownTrackGameModeOptionsFd8* options) {
    options->engineSize = 250;
    options->fourStroke = 0;
    CopyGarageTables(options);
}

// 0x00522cb0
void TrackGameMode::ResetGarageOptions(UnknownTrackGameModeOptionsFd8* options) {
    ResetSuspension(options);
    ResetEngineOptions(options);
}

// 0x00522cd0
void TrackGameMode::ResetSettingsEntries() {
    int i;

    for (i = 0; i < 8; i++) {
        UnknownTrackGameModeEntry entry;
        entry.playerId = 0;
        entry.points = 0;
        raceSettings.gridOrder[i] = entry;
    }
}


// 0x00522d00
int TrackGameMode::CreateDirectoryLists() {

    ChooseDisplayMode(0, &displayModeIndex);
    highScoreDirectoryList = new(__FILE__, 383) DirectoryList;
    trackDirectoryList = new(__FILE__, 384) CombinedDirectoryList;
    field_0x25e8 = new(__FILE__, 385) CombinedDirectoryList;
    if (driveList && trackDirectoryList && field_0x25e8)
        return 1;
    return 0;
}

// 0x00522e20
void TrackGameMode::ChooseDisplayMode(const void* guid, int* mode) {
    TrackGame* game = g_TrackGame;
    int i;

    if (!game->display)
        return;
    if (game->displayProfilesStale || !guid ||
        memcmp(guid, &game->display->driverGuid, sizeof(UnknownGuid)) ||
        !game->display->displayModes[*mode].field_0x14) {
        *mode = 0;
        if (g_TrackGame->bladeRenderer) {
            for (i = 0; i < g_TrackGame->display->displayModeCount; i++) {
                UnknownDisplayMode* entry = &g_TrackGame->display->displayModes[i];
                if (entry->width == 640 && entry->height == 480 && entry->bitDepth == 16 &&
                    entry->field_0x18) {
                    *mode = i;
                    break;
                }
            }
            if (!*mode) {
                for (i = 0; i < g_TrackGame->display->displayModeCount; i++) {
                    if (!*mode ||
                        (g_TrackGame->display->displayModes[i].field_0x18 &&
                         g_TrackGame->display->displayModes[i].bitDepth == 16 &&
                         g_TrackGame->display->displayModes[i].height >
                             g_TrackGame->display->displayModes[*mode].height))
                        *mode = i;
                }
            }
        } else {
            for (i = 0; i < g_TrackGame->display->displayModeCount; i++) {
                UnknownDisplayMode* entry = &g_TrackGame->display->displayModes[i];
                if (entry->width == 640 && entry->height == 480 && entry->bitDepth == 16 &&
                    entry->field_0x14) {
                    *mode = i;
                    break;
                }
            }
            if (!*mode) {
                for (i = 0; i < g_TrackGame->display->displayModeCount; i++) {
                    if (!*mode ||
                        (g_TrackGame->display->displayModes[i].field_0x14 &&
                         g_TrackGame->display->displayModes[i].bitDepth == 16 &&
                         g_TrackGame->display->displayModes[i].height >
                             g_TrackGame->display->displayModes[*mode].height))
                        *mode = i;
                }
            }
        }
    }
    displayAdapterGuid = g_TrackGame->display->driverGuid;
}

// 0x00523000: loads the profile's control file.
void TrackGameMode::LoadControllerChoice() {
    unsigned long size;
    char controller[0x80];
    char last[0x80];
    char path[0x104];

    if (!g_TrackGame->controlSetup)
        return;
    sprintf(path, "%s\\%s\\%s", "ui\\profile", profileName, "control.ctl");
    g_TrackGame->controlSetup->LoadControlFile(path);
    size = 0x80;
    g_TrackGame->GetRegistryString("UseControllerId", "", controller, &size);
    g_TrackGame->GetRegistryString("LastControllerId", "", last, &size);
    if (!g_TrackGame->GetRegistryFlag("PresetSelected", 0) || strcmp(last, controller))
        g_TrackGame->controlSetup->ChooseDefaultDevice();
    g_TrackGame->controlSetup->ApplyDevice(path, -1);
}

// 0x00523130: saves the profile's control file.
void TrackGameMode::SaveControllerChoice() {
    unsigned long size;
    char controller[0x80];
    char path[0x104];

    if (!g_TrackGame->controlSetup)
        return;
    if (g_TrackGame->GetRegistryFlag("UseLastController", 0))
        g_TrackGame->SetRegistryFlag("PresetSelected", 1);
    size = 0x80;
    g_TrackGame->GetRegistryString("UseControllerId", "", controller, &size);
    g_TrackGame->SetRegistryString("LastControllerId", controller);
    sprintf(path, "%s\\%s\\%s", "ui\\profile", profileName, "control.ctl");
    g_TrackGame->controlSetup->SaveControlFile(path);
}

// 0x005231f0 (TrackGame slot 4): loads the profile; 1 when it has a name.
// Without a profile file the name is cleared and a default TrackGameMode is
// built and destroyed on the stack.
int TrackGameMode::LoadProfile() {
    char path[0x104];
    FILE* file;

    sprintf(path, "%s\\%s\\%s.prf", "ui\\profile", profileName, profileName);
    file = fopen(path, "rb");
    if (!file) {
        strcpy(profileName, "");
        TrackGameMode();
        return 0;
    }
    fread(profileName, 0x10, 1, file);
    fread(&showNewbieDialog, 4, 1, file);
    fread(&instrumentOverlayOn, 0x374, 1, file);
    fread(&field_0xa20, 0x2c, 1, file);
    fread(&displayModeIndex, 0x1c, 1, file);
    fread(field_0xa68, 0x30, 1, file);
    fread(resourceStrings, 0x500, 1, file);
    fread(videoSettings, 0x18, 1, file);
    fread(optionsBackup, 0x5c, 1, file);
    fread(multiplayerGarageOptions, 0x5c, 1, file);
    fread(&racerChoiceBackup, 0xc8, 1, file);
    fread(multiplayerRacerChoice, 0xc8, 1, file);
    fread(&displayAdapterGuid, 0x10, 1, file);
    fread(&savedCameraState, 4, 1, file);
    fread(&savedCameraDistance, 4, 1, file);
    fread(&savedCameraPitch, 4, 1, file);
    fread(&savedCameraYaw, 4, 1, file);
    fread(&savedCameraFieldOfView, 4, 1, file);
    fread(playerName, 0x80, 1, file);
    fread(&savedSettings, 0x1ec, 1, file);
    fread(&multiplayerSettings, 0x1ec, 1, file);
    fread(bonusTrackUnlocked, 0x18, 1, file);
    fread(&field_0x1bdc, 4, 1, file);
    fread(&nameTagShade, 4, 1, file);
    fread(&savedDifficulty, 4, 1, file);
    fread(customGarageCurves, 0x294, 1, file);
    fread(&plateNumber, 4, 1, file);
    fclose(file);
    raceSettings.networkPlayerCount = 0;
    ChooseDisplayMode(&displayAdapterGuid, &displayModeIndex);
    g_TrackGame->ui->profileLoaded = 1;
    LoadControllerChoice();
    if (g_TrackGame->soundInterface) {
        ((PCSoundInterface*)g_TrackGame->soundInterface)->allowEAXExtension = eaxEnabled;
        ((PCSoundInterface*)g_TrackGame->soundInterface)->SetPrimaryFormat(22050, 1, use16BitSound ? 16 : 8);
    }
    raceSettings = savedSettings;
    difficulty = savedDifficulty;
    memcpy(garageOptions, optionsBackup, sizeof(garageOptions));
    memcpy(&racerChoice, &racerChoiceBackup, sizeof(racerChoice));
    return strcmp(profileName, "") != 0;
}

// 0x00523580: saves the profile.
void TrackGameMode::SaveProfile() {
    char path[0x104];
    FILE* file;

    if (!profileName[0])
        return;
    sprintf(path, "%s\\%s\\%s.prf", "ui\\profile", profileName, profileName);
    file = fopen(path, "wb");
    if (!file)
        return;
    fwrite(profileName, 0x10, 1, file);
    fwrite(&showNewbieDialog, 4, 1, file);
    fwrite(&instrumentOverlayOn, 0x374, 1, file);
    fwrite(&field_0xa20, 0x2c, 1, file);
    fwrite(&displayModeIndex, 0x1c, 1, file);
    fwrite(field_0xa68, 0x30, 1, file);
    fwrite(resourceStrings, 0x500, 1, file);
    fwrite(videoSettings, 0x18, 1, file);
    fwrite(optionsBackup, 0x5c, 1, file);
    fwrite(multiplayerGarageOptions, 0x5c, 1, file);
    fwrite(&racerChoiceBackup, 0xc8, 1, file);
    fwrite(multiplayerRacerChoice, 0xc8, 1, file);
    fwrite(&displayAdapterGuid, 0x10, 1, file);
    fwrite(&savedCameraState, 4, 1, file);
    fwrite(&savedCameraDistance, 4, 1, file);
    fwrite(&savedCameraPitch, 4, 1, file);
    fwrite(&savedCameraYaw, 4, 1, file);
    fwrite(&savedCameraFieldOfView, 4, 1, file);
    fwrite(playerName, 0x80, 1, file);
    fwrite(&savedSettings, 0x1ec, 1, file);
    fwrite(&multiplayerSettings, 0x1ec, 1, file);
    fwrite(bonusTrackUnlocked, 0x18, 1, file);
    fwrite(&field_0x1bdc, 4, 1, file);
    fwrite(&nameTagShade, 4, 1, file);
    fwrite(&savedDifficulty, 4, 1, file);
    fwrite(customGarageCurves, 0x294, 1, file);
    fwrite(&plateNumber, 4, 1, file);
    fclose(file);
    g_TrackGame->SetRegistryString("MRUProfile", profileName);
    if (g_TrackGame->ui)
        g_TrackGame->ui->profileLoaded = 1;
    SaveControllerChoice();
}

// 0x00523800
int TrackGameMode::WaitForCd() {
    char text[0x80];

    SendMessageA(g_TrackGame->windowHandle, 0x112, 0xf020, 0);
    while (!FindCdDirectory()) {
        if (LoadStringA(g_TrackGame->resourceInstance, 0x13b5, text, 0x80)) {
            ShowCursor(1);
            if (MessageBoxA(g_TrackGame->windowHandle, text, g_TrackGame->applicationName, 0x15) == 2) {
                SendMessageA(g_TrackGame->windowHandle, 0x10, 0, 0);
                return 0;
            }
        }
    }
    SendMessageA(g_TrackGame->windowHandle, 0x112, 0xf120, 0);
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
    result = FindDataFile(file, found);
    if (!result)
        return 0;
    _splitpath(found, drive, directory, base, extension);
    sprintf(path, "%s%s", drive, directory);
    path[strlen(path) - 1] = 0;
    return result;
}

// 0x00523b70
void TrackGameMode::CopyCurrentSeriesDirectory(char* name) {
    CopySeriesDirectory((short)raceSettings.eventType, (short)raceSettings.raceMode, name);
}

// 0x00523bb0
int TrackGameMode::CopySeriesDirectory(short a, short b, char* name) {
    strcpy(name, seriesDirectories[raceSettings.series]);
    return 1;
}

// 0x00523bf0: looks for the "MCM2" CD.
int TrackGameMode::FindCdDirectory() {
    int index;

    if (installType != 2) {
        if (!driveList)
            return 0;
        driveList->ListDrives();
        if (!driveList->FindVolume("MCM2", &index, 5))
            return 0;
        if (!driveList->GetDriveRoot(index, cdDriveRoot))
            return 0;
        sprintf(cdDataDirectory, "%s%s", cdDriveRoot, "game");
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
        installType = 2;
    else
        installType = 1;
    size = 0x104;
    return g_TrackGame->GetRegistryString("HardDriveRootPath", "", installDirectory, &size) != 0;
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
        if (FindDataFile(name, path)) {
            SendMessageA(g_TrackGame->windowHandle, 0x112, 0xf020, 0);
            ShellExecuteA(g_TrackGame->windowHandle, 0, path, parameters, directory, 1);
            return 1;
        }
    }
    return 0;
}

// 0x00523e50 (KrustyUI 0x004988a0): the bonus track names from PCSched.pb.
void TrackGameMode::LoadBonusTrackNames() {
    UnknownParameterBlock block;
    char key[0x100];
    UnknownTextureStream* stream = new(__FILE__, 873) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    int i;

    for (i = 0; i < 6; i++) {
        bonusTrackUnlocked[i] = 0;
        bonusTrackNames[i][0] = 0;
    }
    if (stream->Open("PCSched.pb", "r", 0)) {
        int track;

        block.Attach((UnknownParameterStream*)stream, 0, 1);
        block.SelectSection("Enduro");
        block.GetInt("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.GetString(key, "", bonusTrackNames[5], -1);
        block.SelectSection("Baja");
        block.GetInt("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.GetString(key, "", bonusTrackNames[1], -1);
        block.SelectSection("Nationals");
        block.GetInt("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.GetString(key, "", bonusTrackNames[2], -1);
        block.SelectSection("Supercross");
        block.GetInt("BonusTrack", 0, &track);
        sprintf(key, "Track_%d", track);
        block.GetString(key, "", bonusTrackNames[3], -1);
    }
    if (stream)
        delete stream;
}

// 0x005240e0
void TrackGameMode::SelectSeries(int series) {
    raceSettings.series = series;
    currentSeriesDirectory = (int)seriesDirectories[series];
}

// 0x00524100
int TrackGameMode::GetSelectedSeries() {
    return raceSettings.series;
}

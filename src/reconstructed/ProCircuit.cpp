#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ProCircuit.h"
#include "Parameterblocks.h"
#include "SelectGamePicProcs.h"
#include "TextureMap.h"
#include "TrackGame.h"
#include "UnknownResourceManager.h"
#include "MatrixUtil.h"
#include "DebugAlloc.h"

// The four vector constants of many retail files (see Cube.cpp):
// 0x00689ac8, 0x00689ad8, 0x00689ae8 and 0x00689ab8, initialised by
// 0x004d49e0..0x004d4b1b, after this file's last function. The set before
// the constructor (0x004d3340) closes the gearbox unit (GearRatios.cpp),
// whose 0x004d31b0 reads its zero vector; this file reads none of its own.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// A random value in [0, 1).
#define UNKNOWN_RANDOM_UNIT() (rand() * (1.0f / 32768.0f))

// Copies at most sizeof(destination) - 1 characters of `source` and
// terminates them.
#define UNKNOWN_COPY_TEXT(destination, source)                                          \
    {                                                                                   \
        int length = strlen(source);                                                    \
        int count = length > (int)sizeof(destination) - 1 ? (int)sizeof(destination) - 1 : length; \
        strncpy(destination, source, count);                                            \
        destination[count] = 0;                                                         \
    }

// 0x004d3480
UnknownTrackGameObject3444::UnknownTrackGameObject3444()
{
    UnknownTextureStream* stream = new(__FILE__, 19) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    UnknownParameterBlock block;
    char key[128];
    char line[64];
    float defaults[9];

    if (stream->Open("PCTables.pb", "r", 0)) {
        block.Attach((UnknownParameterStream*)stream, 0, 1);
        block.SelectSection("ProCircuit");
        block.GetInt("StartingCapital", 5000, &startingCapital);
        block.GetInt("EnduroFee", 300, &entryFees[5]);
        block.GetInt("BajaFee", 500, &entryFees[1]);
        block.GetInt("NationalsFee", 1500, &entryFees[2]);
        block.GetInt("SupercrossFee", 2500, &entryFees[3]);
        block.GetFloat("PurseMultiplier", 17.5f, &purseMultiplier);
        block.GetFloat("RepairCapPct", 0.1f, &repairCapPct);
        block.GetFloat("MedicalCapPct", 0.05f, &medicalCapPct);
        block.GetFloat("StuntCapPct", 0.075f, &stuntCapPct);
        float total = 0.0f;
        for (int i = 0; i < 11; i++) {
            sprintf(key, "%d_PlacePct", i + 1);
            block.GetFloat(key, 0.0f, &placePct[i]);
            total += placePct[i];
        }
        if (total <= 1.0f)
            placePct[0] += 1.0f - total;
    } else {
        startingCapital = 5000;
        entryFees[5] = 300;
        entryFees[1] = 500;
        entryFees[2] = 1500;
        entryFees[3] = 2500;
        purseMultiplier = 17.5f;
        repairCapPct = 0.1f;
        medicalCapPct = 0.05f;
        stuntCapPct = 0.075f;
        defaults[0] = 33.0f;
        defaults[1] = 23.0f;
        defaults[2] = 14.0f;
        defaults[3] = 9.0f;
        defaults[4] = 7.0f;
        defaults[5] = 5.0f;
        defaults[6] = 4.0f;
        defaults[7] = 3.0f;
        defaults[8] = 2.0f;
        for (int i = 0; i < 11; i++) {
            if (i < 9)
                placePct[i] = defaults[i];
            else
                placePct[i] = 0;
        }
    }

    racerNameCount = 0;
    racerNames = 0;
    FILE* file = fopen("ui\\PCNames.txt", "r");
    if (file) {
        while (fgets(line, 64, file)) {
            char* end = strchr(line, '\n');
            if (end)
                *end = 0;
            racerNames = (char**)DebugRealloc(racerNames, (racerNameCount + 1) * 4, __FILE__, 85);
            racerNames[racerNameCount] = (char*)DebugMalloc(strlen(line) + 1, __FILE__, 86);
            strcpy(racerNames[racerNameCount], line);
            racerNameCount++;
        }
        fclose(file);
    }

    if (stream->Open("PCSched.pb", "r", 0)) {
        block.Attach((UnknownParameterStream*)stream, 0, 1);
        schedules[0].tracks = 0;
        schedules[4].tracks = 0;
        block.SelectSection("Enduro");
        ReadSchedule(&block, &schedules[5], "Teraform\\Enduro");
        block.SelectSection("Baja");
        ReadSchedule(&block, &schedules[1], "Teraform\\Baja");
        block.SelectSection("Nationals");
        ReadSchedule(&block, &schedules[2], "Teraform\\National");
        block.SelectSection("Supercross");
        ReadSchedule(&block, &schedules[3], "Teraform\\SX");
    } else {
        for (int i = 0; i < 6; i++) {
            schedules[i].tracks = 0;
            schedules[i].trackCount = 0;
            schedules[i].bonusTrack = 0;
        }
    }

    if (stream)
        delete stream;
}

// 0x004d39f0
UnknownTrackGameObject3444::~UnknownTrackGameObject3444()
{
    for (int i = 0; i < racerNameCount; i++)
        DebugFree(racerNames[i], __FILE__, 139);
    DebugFree(racerNames, __FILE__, 141);
    for (int j = 0; j < 6; j++)
        FreeSchedule(&schedules[j]);
}

// 0x004d3a60
void UnknownTrackGameObject3444::FreeSchedule(UnknownProCircuitSchedule* schedule)
{
    if (schedule->tracks) {
        for (int i = 0; i < schedule->trackCount; i++) {
            if (schedule->tracks[i].displayName)
                DebugFree(schedule->tracks[i].displayName, __FILE__, 153);
            if (schedule->tracks[i].trackKey)
                DebugFree(schedule->tracks[i].trackKey, __FILE__, 154);
            if (schedule->tracks[i].keyCopy)
                DebugFree(schedule->tracks[i].keyCopy, __FILE__, 155);
        }
        DebugFree(schedule->tracks, __FILE__, 158);
    }
}

// 0x004d3b00
void UnknownTrackGameObject3444::StartCareer(const char* name, int a, int b, int count)
{
    racerCount = count;
    difficulty = a;
    bikeClassRule = b;
    series = 5;
    raceIndex = 1;
    maxModelKind = 1;
    feesPaid = 0;
    strncpy(careerName, name, 64);
    careerName[strlen(name) > 63 ? 63 : strlen(name)] = 0;

    char text[256];
    g_TrackGame->LoadResourceString(0x14bc, messageText, 128);
    g_TrackGame->LoadResourceString(0x14bd, text, 128);
    strcat(messageText, "\n");
    strcat(messageText, text);
    g_TrackGame->LoadResourceString(0x14be, text, 128);
    strcat(messageText, "\n");
    strcat(messageText, text);
    g_TrackGame->LoadResourceString(0x14bf, text, 128);
    strcat(messageText, "\n");
    strcat(messageText, text);
    careerFlags = 0;

    UnknownProCircuitStats zero;
    zero.medicalCost = 0;
    zero.rank = 0;
    zero.repairCost = 0;
    zero.purseWinnings = 0;
    zero.points = 0;
    zero.stuntWinnings = 0;
    int* used = (int*)DebugCalloc(racerNameCount, 4, __FILE__, 203);
    srand(ReadClock());
    PickComputerRacers();

    for (int i = 0; i < racerCount; i++) {
        racers[i].money = startingCapital;
        racers[i].debts = 0;
        if (i > 0) {
            int n = (int)(UNKNOWN_RANDOM_UNIT() * (racerNameCount - 1));
            while (used[n])
                n = (int)(UNKNOWN_RANDOM_UNIT() * (racerNameCount - 1));
            UNKNOWN_COPY_TEXT(racers[i].racerName, racerNames[n])
            used[n] = 1;
        } else {
            UNKNOWN_COPY_TEXT(racers[i].racerName, g_TrackGame->mode.profileName)
        }
        racers[i].raceStats = zero;
        racers[i].totalStats = zero;
    }

    KrustyUI* ui = g_TrackGame->ui;
    int rider;
    for (rider = 0; rider < ui->riderCount; rider++) {
        if (((UnknownKrustyUIModel*)ui->riders)[rider].modelKind == 1)
            break;
    }
    UNKNOWN_COPY_TEXT(racers[0].riderModel, ((UnknownKrustyUIModel*)ui->riders)[rider].modelName)

    ui = g_TrackGame->ui;
    int bike;
    for (bike = 0; bike < ui->bikeCount; bike++) {
        if (((UnknownKrustyUIModel*)ui->manufacturers)[((UnknownKrustyUIBike*)ui->bikes)[bike].model].modelKind == 1) {
            int bikeClass = BikeClassOf(((UnknownKrustyUIBike*)ui->bikes)[bike].engineSize);
            switch (bikeClassRule) {
            case 1:
                if (bikeClass == 0)
                    goto found;
                break;
            case 2:
                if (bikeClass == 1 || bikeClass == 2)
                    goto found;
                break;
            case 3:
                goto found;
            }
        }
    }
found:
    UNKNOWN_COPY_TEXT(racers[0].bikeModel,
                      ((UnknownKrustyUIModel*)ui->manufacturers)[((UnknownKrustyUIBike*)ui->bikes)[bike].model].modelName)
    UNKNOWN_COPY_TEXT(racers[0].bikeName,
                    ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bike].texture)
    UNKNOWN_COPY_TEXT(racers[0].bikeDisplayName,
                    ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bike].displayName)
    racers[0].engineSize = ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bike].engineSize;
    racers[0].fourStroke = ((UnknownKrustyUIBike*)g_TrackGame->ui->bikes)[bike].fourStroke;
    DebugFree(used, __FILE__, 273);
}

// 0x004d4100
int UnknownTrackGameObject3444::LoadSaved(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file) {
        fread(this, 0x1225, 1, file);
        fclose(file);
        return 1;
    }
    return 0;
}

// 0x004d4150
int UnknownTrackGameObject3444::Save(const char* path)
{
    FILE* file = fopen(path, "wb");
    if (file) {
        fwrite(this, 0x1225, 1, file);
        fclose(file);
        return 1;
    }
    return 0;
}

// 0x004d41a0
int UnknownTrackGameObject3444::AdvanceAfterRace()
{
    int id;
    char text[1024];
    char title[2048];
    strcpy(title, "");
    raceIndex++;
    int placed = racers[0].totalStats.rank <= 1;
    int bonus = raceIndex == schedules[series].bonusTrack;
    int done = raceIndex > schedules[series].trackCount;
    switch (series) {
    case 1:
        if (done || (bonus && !placed)) {
            series = 2;
            raceIndex = 1;
            maxModelKind = 3;
            PickComputerRacers();
            g_TrackGame->LoadResourceString(0x145c, title, 128);
        }
        break;
    case 2:
        if (done || (bonus && !placed)) {
            series = 3;
            raceIndex = 1;
            PickComputerRacers();
            g_TrackGame->LoadResourceString(0x145c, title, 128);
        }
        break;
    case 3:
        if (done || (bonus && !placed)) {
            careerFlags |= 8;
            racers[0].money += (int)(racers[0].debts * -1.1f);
            g_TrackGame->LoadResourceString(0x145c, title, 128);
            if (racers[0].totalStats.rank <= 3)
                *(unsigned char*)&g_TrackGame->mode.bikeUnlockFlags |= 1; // retail ORs the low byte
        }
        break;
    default:
        if (done || (bonus && !placed)) {
            maxModelKind = 2;
            series = 1;
            raceIndex = 1;
            PickComputerRacers();
            g_TrackGame->LoadResourceString(0x145c, title, 128);
        }
        break;
    }

    switch (series) {
    case 5:
        id = raceIndex + 0x149f;
        break;
    case 1:
        id = raceIndex + 0x14a9;
        break;
    case 2:
        id = raceIndex + 0x14e5;
        break;
    case 3:
        id = raceIndex + 0x1517;
        break;
    }
    g_TrackGame->LoadResourceString(id, text, 1024);
    if (title[0]) {
        sprintf(messageText, "%s\n\n%s", title, text);
        return 1;
    }
    UNKNOWN_COPY_TEXT(messageText, text)
    return 1;
}

// 0x004d4420
int UnknownTrackGameObject3444::ReadSchedule(UnknownParameterBlock* block,
                                                      UnknownProCircuitSchedule* schedule,
                                                      const char* directory)
{
    block->GetInt("NumberOfTracks", 0, &schedule->trackCount);
    block->GetInt("Laps", 3, &schedule->laps);
    if (schedule->trackCount) {
        schedule->tracks =
            (UnknownProCircuitTrack*)DebugMalloc(schedule->trackCount * sizeof(UnknownProCircuitTrack), __FILE__, 433);
        for (int i = 0; i < schedule->trackCount; i++) {
            char key[64];
            char value[64];
            char path[128];
            sprintf(key, "Track_%d", i + 1);
            block->GetString(key, "", value, -1);
            schedule->tracks[i].trackKey = (char*)DebugMalloc(strlen(value) + 1, __FILE__, 438);
            sprintf(schedule->tracks[i].trackKey, "%s", value);
            g_TrackGame->mode.FindFileDirectory((int)directory, schedule->tracks[i].trackKey,
                                                             "env", path);
            g_TrackGame->sceneObject->SetSceneDirectory(path);
            g_TrackGame->sceneObject->ReadSceneName(key, schedule->tracks[i].trackKey, 0,
                                                                      "scn", 0, 0);
            schedule->tracks[i].displayName = (char*)DebugMalloc(strlen(key) + 1, __FILE__, 453);
            strcpy(schedule->tracks[i].displayName, key);
            strcpy(key, schedule->tracks[i].trackKey);
            schedule->tracks[i].keyCopy = (char*)DebugMalloc(strlen(key) + 1, __FILE__, 458);
            strcpy(schedule->tracks[i].keyCopy, key);
        }
    } else {
        schedule->tracks = 0;
    }
    block->GetInt("BonusTrack", 0, &schedule->bonusTrack);
    return 1;
}

// 0x004d4670
void UnknownTrackGameObject3444::PickComputerRacers()
{
    KrustyUI* ui = g_TrackGame->ui;
    int bikeKind = 0;
    int riderKind = 0;
    int i;
    for (i = 0; i < ui->bikeCount; i++) {
        int kind = ((UnknownKrustyUIModel*)ui->manufacturers)[((UnknownKrustyUIBike*)ui->bikes)[i].model].modelKind;
        if (kind <= maxModelKind && kind > bikeKind)
            bikeKind = kind;
    }
    for (i = 0; i < ui->riderCount; i++) {
        int kind = ((UnknownKrustyUIModel*)ui->riders)[i].modelKind;
        if (kind <= maxModelKind && kind > riderKind)
            riderKind = kind;
    }

    for (int racer = 1; racer < racerCount; racer++) {
        UnknownKrustyUIModel* rider;
        do {
            ui = g_TrackGame->ui;
            rider = &((UnknownKrustyUIModel*)ui->riders)[(int)(UNKNOWN_RANDOM_UNIT() * (ui->riderCount - 1))];
        } while (rider->uiModel || rider->modelKind != riderKind);
        UNKNOWN_COPY_TEXT(racers[racer].riderModel, rider->modelName)

        UnknownKrustyUIBike* bike;
        for (;;) {
            ui = g_TrackGame->ui;
            bike = &((UnknownKrustyUIBike*)ui->bikes)[(int)(UNKNOWN_RANDOM_UNIT() * (ui->bikeCount - 1))];
            if (bike->field_0x88)
                continue;
            if (((UnknownKrustyUIModel*)ui->manufacturers)[bike->model].modelKind != bikeKind)
                continue;
            int bikeClass = BikeClassOf(bike->engineSize);
            float roll = UNKNOWN_RANDOM_UNIT();
            switch (bikeClassRule) {
            case 1:
                if (bikeClass == 0)
                    goto found;
                break;
            case 2:
                if (bikeClass == 1) {
                    if (roll <= 0.8f)
                        goto found;
                } else if (bikeClass == 2) {
                    if (roll > 0.8f)
                        goto found;
                }
                break;
            case 3:
                if (bikeClass == 3 || bikeClass == 4)
                    goto found;
                break;
            }
        }
    found:
        UNKNOWN_COPY_TEXT(racers[racer].bikeModel,
                        ((UnknownKrustyUIModel*)g_TrackGame->ui->manufacturers)[bike->model].modelName)
        UNKNOWN_COPY_TEXT(racers[racer].bikeName, bike->texture)
        UNKNOWN_COPY_TEXT(racers[racer].bikeDisplayName, bike->displayName)
        racers[racer].engineSize = bike->engineSize;
        racers[racer].fourStroke = bike->fourStroke;
    }
}

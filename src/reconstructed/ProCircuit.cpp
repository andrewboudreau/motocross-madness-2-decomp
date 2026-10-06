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

    if (stream->UnknownFunction460f50("PCTables.pb", "r", 0)) {
        block.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
        block.UnknownFunction4b78f0("ProCircuit");
        block.UnknownFunction4b7f10("StartingCapital", 5000, &field_0x1225);
        block.UnknownFunction4b7f10("EnduroFee", 300, &field_0x1229[5]);
        block.UnknownFunction4b7f10("BajaFee", 500, &field_0x1229[1]);
        block.UnknownFunction4b7f10("NationalsFee", 1500, &field_0x1229[2]);
        block.UnknownFunction4b7f10("SupercrossFee", 2500, &field_0x1229[3]);
        block.UnknownFunction4b7f40("PurseMultiplier", 17.5f, &field_0x1241);
        block.UnknownFunction4b7f40("RepairCapPct", 0.1f, &field_0x1245);
        block.UnknownFunction4b7f40("MedicalCapPct", 0.05f, &field_0x1249);
        block.UnknownFunction4b7f40("StuntCapPct", 0.075f, &field_0x124d);
        float total = 0.0f;
        for (int i = 0; i < 11; i++) {
            sprintf(key, "%d_PlacePct", i + 1);
            block.UnknownFunction4b7f40(key, 0.0f, &field_0x1251[i]);
            total += field_0x1251[i];
        }
        if (total <= 1.0f)
            field_0x1251[0] += 1.0f - total;
    } else {
        field_0x1225 = 5000;
        field_0x1229[5] = 300;
        field_0x1229[1] = 500;
        field_0x1229[2] = 1500;
        field_0x1229[3] = 2500;
        field_0x1241 = 17.5f;
        field_0x1245 = 0.1f;
        field_0x1249 = 0.05f;
        field_0x124d = 0.075f;
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
                field_0x1251[i] = defaults[i];
            else
                field_0x1251[i] = 0;
        }
    }

    field_0x1281 = 0;
    field_0x127d = 0;
    FILE* file = fopen("ui\\PCNames.txt", "r");
    if (file) {
        while (fgets(line, 64, file)) {
            char* end = strchr(line, '\n');
            if (end)
                *end = 0;
            field_0x127d = (char**)DebugRealloc(field_0x127d, (field_0x1281 + 1) * 4, __FILE__, 85);
            field_0x127d[field_0x1281] = (char*)DebugMalloc(strlen(line) + 1, __FILE__, 86);
            strcpy(field_0x127d[field_0x1281], line);
            field_0x1281++;
        }
        fclose(file);
    }

    if (stream->UnknownFunction460f50("PCSched.pb", "r", 0)) {
        block.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
        field_0x1285[0].field_0x00 = 0;
        field_0x1285[4].field_0x00 = 0;
        block.UnknownFunction4b78f0("Enduro");
        UnknownFunction4d4420(&block, &field_0x1285[5], "Teraform\\Enduro");
        block.UnknownFunction4b78f0("Baja");
        UnknownFunction4d4420(&block, &field_0x1285[1], "Teraform\\Baja");
        block.UnknownFunction4b78f0("Nationals");
        UnknownFunction4d4420(&block, &field_0x1285[2], "Teraform\\National");
        block.UnknownFunction4b78f0("Supercross");
        UnknownFunction4d4420(&block, &field_0x1285[3], "Teraform\\SX");
    } else {
        for (int i = 0; i < 6; i++) {
            field_0x1285[i].field_0x00 = 0;
            field_0x1285[i].field_0x04 = 0;
            field_0x1285[i].field_0x08 = 0;
        }
    }

    if (stream)
        delete stream;
}

// 0x004d39f0
UnknownTrackGameObject3444::~UnknownTrackGameObject3444()
{
    for (int i = 0; i < field_0x1281; i++)
        operator delete(field_0x127d[i], __FILE__, 139);
    operator delete(field_0x127d, __FILE__, 141);
    for (int j = 0; j < 6; j++)
        UnknownFunction4d3a60(&field_0x1285[j]);
}

// 0x004d3a60
void UnknownTrackGameObject3444::UnknownFunction4d3a60(UnknownProCircuitSchedule* schedule)
{
    if (schedule->field_0x00) {
        for (int i = 0; i < schedule->field_0x04; i++) {
            if (schedule->field_0x00[i].field_0x04)
                operator delete(schedule->field_0x00[i].field_0x04, __FILE__, 153);
            if (schedule->field_0x00[i].field_0x00)
                operator delete(schedule->field_0x00[i].field_0x00, __FILE__, 154);
            if (schedule->field_0x00[i].field_0x08)
                operator delete(schedule->field_0x00[i].field_0x08, __FILE__, 155);
        }
        operator delete(schedule->field_0x00, __FILE__, 158);
    }
}

// 0x004d3b00
void UnknownTrackGameObject3444::UnknownFunction4d3b00(const char* name, int a, int b, int count)
{
    field_0x460 = count;
    field_0x50 = a;
    field_0x4c = b;
    field_0x40 = 5;
    field_0x44 = 1;
    field_0x48 = 1;
    field_0x454 = 0;
    strncpy(field_0x00, name, 64);
    field_0x00[strlen(name) > 63 ? 63 : strlen(name)] = 0;

    char text[256];
    g_UnknownGlobal56e26c->UnknownFunction521970(0x14bc, field_0x54, 128);
    g_UnknownGlobal56e26c->UnknownFunction521970(0x14bd, text, 128);
    strcat(field_0x54, "\n");
    strcat(field_0x54, text);
    g_UnknownGlobal56e26c->UnknownFunction521970(0x14be, text, 128);
    strcat(field_0x54, "\n");
    strcat(field_0x54, text);
    g_UnknownGlobal56e26c->UnknownFunction521970(0x14bf, text, 128);
    strcat(field_0x54, "\n");
    strcat(field_0x54, text);
    field_0x464 = 0;

    UnknownProCircuitStats zero;
    zero.field_0x0c = 0;
    zero.field_0x10 = 0;
    zero.field_0x08 = 0;
    zero.field_0x04 = 0;
    zero.field_0x00 = 0;
    zero.field_0x14 = 0;
    int* used = (int*)DebugCalloc(field_0x1281, 4, __FILE__, 203);
    srand(UnknownFunction4bfa80());
    UnknownFunction4d4670();

    for (int i = 0; i < field_0x460; i++) {
        field_0x465[i].field_0x30 = field_0x1225;
        field_0x465[i].field_0x34 = 0;
        if (i > 0) {
            int n = (int)(UNKNOWN_RANDOM_UNIT() * (field_0x1281 - 1));
            while (used[n])
                n = (int)(UNKNOWN_RANDOM_UNIT() * (field_0x1281 - 1));
            UNKNOWN_COPY_TEXT(field_0x465[i].field_0xf8, field_0x127d[n])
            used[n] = 1;
        } else {
            UNKNOWN_COPY_TEXT(field_0x465[i].field_0xf8, g_UnknownGlobal56e26c->mode.field_0x00)
        }
        field_0x465[i].field_0x00 = zero;
        field_0x465[i].field_0x18 = zero;
    }

    KrustyUI* ui = g_UnknownGlobal56e26c->ui;
    int rider;
    for (rider = 0; rider < ui->field_0x5c; rider++) {
        if (((UnknownKrustyUIModel*)ui->field_0x58)[rider].field_0xc4 == 1)
            break;
    }
    UNKNOWN_COPY_TEXT(field_0x465[0].field_0xb8, ((UnknownKrustyUIModel*)ui->field_0x58)[rider].field_0x40)

    ui = g_UnknownGlobal56e26c->ui;
    int bike;
    for (bike = 0; bike < ui->field_0x54; bike++) {
        if (((UnknownKrustyUIModel*)ui->field_0x48)[((UnknownKrustyUIBike*)ui->field_0x50)[bike].field_0x00].field_0xc4 == 1) {
            int bikeClass = UnknownBikeClassOf(((UnknownKrustyUIBike*)ui->field_0x50)[bike].field_0x8c);
            switch (field_0x4c) {
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
    UNKNOWN_COPY_TEXT(field_0x465[0].field_0x38,
                      ((UnknownKrustyUIModel*)ui->field_0x48)[((UnknownKrustyUIBike*)ui->field_0x50)[bike].field_0x00].field_0x40)
    UNKNOWN_COPY_TEXT(field_0x465[0].field_0x78,
                    ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bike].field_0x48)
    UNKNOWN_COPY_TEXT(field_0x465[0].field_0x118,
                    ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bike].field_0x04)
    field_0x465[0].field_0x138 = ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bike].field_0x8c;
    field_0x465[0].field_0x13c = ((UnknownKrustyUIBike*)g_UnknownGlobal56e26c->ui->field_0x50)[bike].field_0x90;
    operator delete(used, __FILE__, 273);
}

// 0x004d4100
int UnknownTrackGameObject3444::UnknownFunction4d4100(const char* path)
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
int UnknownTrackGameObject3444::UnknownFunction4d4150(const char* path)
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
int UnknownTrackGameObject3444::UnknownFunction4d41a0()
{
    int id;
    char text[1024];
    char title[2048];
    strcpy(title, "");
    field_0x44++;
    int placed = field_0x465[0].field_0x18.field_0x10 <= 1;
    int bonus = field_0x44 == field_0x1285[field_0x40].field_0x08;
    int done = field_0x44 > field_0x1285[field_0x40].field_0x04;
    switch (field_0x40) {
    case 1:
        if (done || (bonus && !placed)) {
            field_0x40 = 2;
            field_0x44 = 1;
            field_0x48 = 3;
            UnknownFunction4d4670();
            g_UnknownGlobal56e26c->UnknownFunction521970(0x145c, title, 128);
        }
        break;
    case 2:
        if (done || (bonus && !placed)) {
            field_0x40 = 3;
            field_0x44 = 1;
            UnknownFunction4d4670();
            g_UnknownGlobal56e26c->UnknownFunction521970(0x145c, title, 128);
        }
        break;
    case 3:
        if (done || (bonus && !placed)) {
            field_0x464 |= 8;
            field_0x465[0].field_0x30 += (int)(field_0x465[0].field_0x34 * -1.1f);
            g_UnknownGlobal56e26c->UnknownFunction521970(0x145c, title, 128);
            if (field_0x465[0].field_0x18.field_0x10 <= 3)
                *(unsigned char*)&g_UnknownGlobal56e26c->mode.field_0x1970 |= 1; // retail ORs the low byte
        }
        break;
    default:
        if (done || (bonus && !placed)) {
            field_0x48 = 2;
            field_0x40 = 1;
            field_0x44 = 1;
            UnknownFunction4d4670();
            g_UnknownGlobal56e26c->UnknownFunction521970(0x145c, title, 128);
        }
        break;
    }

    switch (field_0x40) {
    case 5:
        id = field_0x44 + 0x149f;
        break;
    case 1:
        id = field_0x44 + 0x14a9;
        break;
    case 2:
        id = field_0x44 + 0x14e5;
        break;
    case 3:
        id = field_0x44 + 0x1517;
        break;
    }
    g_UnknownGlobal56e26c->UnknownFunction521970(id, text, 1024);
    if (title[0]) {
        sprintf(field_0x54, "%s\n\n%s", title, text);
        return 1;
    }
    UNKNOWN_COPY_TEXT(field_0x54, text)
    return 1;
}

// 0x004d4420
int UnknownTrackGameObject3444::UnknownFunction4d4420(UnknownParameterBlock* block,
                                                      UnknownProCircuitSchedule* schedule,
                                                      const char* directory)
{
    block->UnknownFunction4b7f10("NumberOfTracks", 0, &schedule->field_0x04);
    block->UnknownFunction4b7f10("Laps", 3, &schedule->field_0x0c);
    if (schedule->field_0x04) {
        schedule->field_0x00 =
            (UnknownProCircuitTrack*)DebugMalloc(schedule->field_0x04 * sizeof(UnknownProCircuitTrack), __FILE__, 433);
        for (int i = 0; i < schedule->field_0x04; i++) {
            char key[64];
            char value[64];
            char path[128];
            sprintf(key, "Track_%d", i + 1);
            block->UnknownFunction4b7ec0(key, "", value, -1);
            schedule->field_0x00[i].field_0x00 = (char*)DebugMalloc(strlen(value) + 1, __FILE__, 438);
            sprintf(schedule->field_0x00[i].field_0x00, "%s", value);
            g_UnknownGlobal56e26c->mode.UnknownFunction523a60((int)directory, schedule->field_0x00[i].field_0x00,
                                                             "env", path);
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9b80(path);
            g_UnknownGlobal56e26c->sceneObject->UnknownFunction4ea010(key, schedule->field_0x00[i].field_0x00, 0,
                                                                      "scn", 0, 0);
            schedule->field_0x00[i].field_0x04 = (char*)DebugMalloc(strlen(key) + 1, __FILE__, 453);
            strcpy(schedule->field_0x00[i].field_0x04, key);
            strcpy(key, schedule->field_0x00[i].field_0x00);
            schedule->field_0x00[i].field_0x08 = (char*)DebugMalloc(strlen(key) + 1, __FILE__, 458);
            strcpy(schedule->field_0x00[i].field_0x08, key);
        }
    } else {
        schedule->field_0x00 = 0;
    }
    block->UnknownFunction4b7f10("BonusTrack", 0, &schedule->field_0x08);
    return 1;
}

// 0x004d4670
void UnknownTrackGameObject3444::UnknownFunction4d4670()
{
    KrustyUI* ui = g_UnknownGlobal56e26c->ui;
    int bikeKind = 0;
    int riderKind = 0;
    int i;
    for (i = 0; i < ui->field_0x54; i++) {
        int kind = ((UnknownKrustyUIModel*)ui->field_0x48)[((UnknownKrustyUIBike*)ui->field_0x50)[i].field_0x00].field_0xc4;
        if (kind <= field_0x48 && kind > bikeKind)
            bikeKind = kind;
    }
    for (i = 0; i < ui->field_0x5c; i++) {
        int kind = ((UnknownKrustyUIModel*)ui->field_0x58)[i].field_0xc4;
        if (kind <= field_0x48 && kind > riderKind)
            riderKind = kind;
    }

    for (int racer = 1; racer < field_0x460; racer++) {
        UnknownKrustyUIModel* rider;
        do {
            ui = g_UnknownGlobal56e26c->ui;
            rider = &((UnknownKrustyUIModel*)ui->field_0x58)[(int)(UNKNOWN_RANDOM_UNIT() * (ui->field_0x5c - 1))];
        } while (rider->field_0xc0 || rider->field_0xc4 != riderKind);
        UNKNOWN_COPY_TEXT(field_0x465[racer].field_0xb8, rider->field_0x40)

        UnknownKrustyUIBike* bike;
        for (;;) {
            ui = g_UnknownGlobal56e26c->ui;
            bike = &((UnknownKrustyUIBike*)ui->field_0x50)[(int)(UNKNOWN_RANDOM_UNIT() * (ui->field_0x54 - 1))];
            if (bike->field_0x88)
                continue;
            if (((UnknownKrustyUIModel*)ui->field_0x48)[bike->field_0x00].field_0xc4 != bikeKind)
                continue;
            int bikeClass = UnknownBikeClassOf(bike->field_0x8c);
            float roll = UNKNOWN_RANDOM_UNIT();
            switch (field_0x4c) {
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
        UNKNOWN_COPY_TEXT(field_0x465[racer].field_0x38,
                        ((UnknownKrustyUIModel*)g_UnknownGlobal56e26c->ui->field_0x48)[bike->field_0x00].field_0x40)
        UNKNOWN_COPY_TEXT(field_0x465[racer].field_0x78, bike->field_0x48)
        UNKNOWN_COPY_TEXT(field_0x465[racer].field_0x118, bike->field_0x04)
        field_0x465[racer].field_0x138 = bike->field_0x8c;
        field_0x465[racer].field_0x13c = bike->field_0x90;
    }
}

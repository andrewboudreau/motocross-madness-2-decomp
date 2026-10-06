#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RaceSound.h"

#include "BikeCamera.h"
#include "DebugAlloc.h"
#include "KeyboardDevice.h"
#include "Parameterblocks.h"
#include "RaceView.h"
#include "TextureMap.h"
#include "TrackGame.h"

#define SoundSystem() ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)

// 0x004e1fb0
RaceSound::RaceSound(int flags) : GameObject(flags) {
    int i;
    int k;

    field_0x12a0 = 0;
    field_0x129c = 99;
    field_0x12a4 = 0;
    field_0x12a8 = 0;
    field_0x12ac = 0;
    field_0x434 = 0;
    field_0x94 = 0;
    field_0x438 = 0;
    field_0x1290 = 0;
    field_0x1294 = 0;
    field_0x1204 = 0;
    field_0x11e8 = 0;
    field_0x2c = 0;
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x44 = 0;
    field_0x40 = 0;
    field_0x11b0 = 0;
    field_0x58 = 0;
    field_0x5c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    field_0x11b4 = 0;
    field_0x11ac = 0;
    field_0x1208 = 1;
    field_0x50 = 0;
    field_0x11ec = 0;
    field_0x11f0 = 0;
    field_0x11f4 = 0;
    for (i = 0; i < 11; i++) {
        field_0x120c[i] = 0;
        field_0x1264[i] = 0;
        field_0x1238[i] = 0;
        field_0x98[i].field_0x50 = 0;
        field_0x98[i].field_0x50 = DebugMalloc(44100, __FILE__, 77);
        memset(field_0x98[i].field_0x50, 0, 44100);
    }
    for (i = 0; i < 6; i++)
        field_0x11b8[i] = 0;
    for (i = 0; i < 6; i++)
        field_0x11d0[i] = 0;
    for (k = 0; k < 3; k++) {
        memset(field_0x840[k], 0, sizeof(field_0x840[k]));
        memset(field_0x8b8[k], 0, sizeof(field_0x8b8[k]));
        memset(field_0x930[k], 0, sizeof(field_0x930[k]));
        memset(field_0x9a8[k], 0, sizeof(field_0x9a8[k]));
        memset(field_0xa20[k], 0, sizeof(field_0xa20[k]));
        memset(field_0xa98[k], 0, sizeof(field_0xa98[k]));
        memset(field_0xb10[k], 0, sizeof(field_0xb10[k]));
        memset(field_0xb88[k], 0, sizeof(field_0xb88[k]));
    }
}

// 0x004e21b0
RaceSound::~RaceSound() {
    int i;
    int k;

    for (k = 0; k < 3; k++) {
        for (i = 0; i < 10; i++) {
            if (field_0x840[k][i])
                operator delete(field_0x840[k][i], __FILE__, 128);
            field_0x840[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0x8b8[k][i])
                operator delete(field_0x8b8[k][i], __FILE__, 132);
            field_0x8b8[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0x930[k][i])
                operator delete(field_0x930[k][i], __FILE__, 136);
            field_0x930[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0x9a8[k][i])
                operator delete(field_0x9a8[k][i], __FILE__, 140);
            field_0x9a8[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0xa20[k][i])
                operator delete(field_0xa20[k][i], __FILE__, 144);
            field_0xa20[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0xa98[k][i])
                operator delete(field_0xa98[k][i], __FILE__, 148);
            field_0xa98[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0xb10[k][i])
                operator delete(field_0xb10[k][i], __FILE__, 152);
            field_0xb10[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0xb88[k][i])
                operator delete(field_0xb88[k][i], __FILE__, 156);
            field_0xb88[k][i] = 0;
        }
    }
    for (i = 0; i < 11; i++) {
        if (field_0x98[i].field_0x50)
            operator delete(field_0x98[i].field_0x50, __FILE__, 177);
        field_0x98[i].field_0x50 = 0;
    }
    SoundSystem()->UnknownFunction4be9b0(-10000);
    Release();
}

// 0x004e23f0: creates the four sound groups and the sounds, then reads each
// present engine's section ("125", "250", "400") of audio_16.ini or
// audio_08.ini: the sample counts (divided down when several engines share
// the race) and names of eight sample sets, and three engine speeds.
RaceSound* RaceSound::UnknownFunction4e23f0(void* owner, UnknownKrustyBikeView* view,
                                            UnknownRaceSoundCamera* camera, int racers) {
    int count;
    UnknownTextureStream* stream;
    int iterator;
    char sample[0x50];
    char key[0x50];
    char section[0x50];
    char file[0x50];
    UnknownParameterBlock parameters;
    UnknownEventRacer* racer;
    int i;
    int k;

    stream = new (__FILE__, 203) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    GameObject::UnknownVirtualSlot8(owner);
    field_0x34 = view;
    field_0x38 = camera;
    field_0x3c = new (__FILE__, 213) SoundGroup(1);
    UnknownFunction469190(field_0x3c, -1);
    field_0x44 = new (__FILE__, 214) SoundGroup(1);
    UnknownFunction469190(field_0x44, -1);
    field_0x4c = new (__FILE__, 215) SoundGroup(1);
    UnknownFunction469190(field_0x4c, -1);
    field_0x48 = new (__FILE__, 216) SoundGroup(1);
    UnknownFunction469190(field_0x48, -1);
    if (SoundSystem())
        SoundSystem()->UnknownFunction4be8b0();

    field_0x45c[0] = 0;
    field_0x45c[1] = 0;
    field_0x45c[2] = 0;
    iterator = 0;
    while ((racer = field_0x34->UnknownFunction4204e0(&iterator)) != 0) {
        if (racer->field_0x738 < 250) {
            field_0x45c[0] = 1;
            if (racer == field_0x34->field_0x38)
                field_0x474 = 0;
        } else if (!racer->field_0x737) {
            field_0x45c[1] = 1;
            if (racer == field_0x34->field_0x38)
                field_0x474 = 1;
        } else {
            field_0x45c[2] = 1;
            if (racer == field_0x34->field_0x38)
                field_0x474 = 2;
        }
    }
    field_0x478 = 0;
    if (field_0x45c[0])
        field_0x478++;
    if (field_0x45c[1])
        field_0x478++;
    if (field_0x45c[2])
        field_0x478++;

    field_0x468[0] = 1;
    field_0x468[1] = 1;
    field_0x468[2] = 1;
    if (!g_UnknownGlobal56e26c->mode.field_0xa48) {
        if (racers <= 2) {
            if (field_0x478 == 2) {
                field_0x468[0] = 2;
                field_0x468[1] = 2;
                field_0x468[2] = 2;
            }
            if (field_0x478 == 3) {
                field_0x468[0] = 4;
                field_0x468[1] = 4;
                field_0x468[2] = 4;
                field_0x468[field_0x474] = 2;
            }
        }
    } else if (racers <= 2) {
        if (field_0x478 == 1) {
            field_0x468[0] = 2;
            field_0x468[1] = 2;
            field_0x468[2] = 2;
        }
        if (field_0x478 == 2) {
            field_0x468[0] = 4;
            field_0x468[1] = 4;
            field_0x468[2] = 4;
        }
        if (field_0x478 == 3) {
            field_0x468[0] = 8;
            field_0x468[1] = 8;
            field_0x468[2] = 8;
            field_0x468[field_0x474] = 4;
        }
    } else if (racers <= 6) {
        if (field_0x478 == 2) {
            field_0x468[0] = 2;
            field_0x468[1] = 2;
            field_0x468[2] = 2;
            field_0x468[field_0x474] = 1;
        }
        if (field_0x478 == 3) {
            field_0x468[0] = 4;
            field_0x468[1] = 4;
            field_0x468[2] = 4;
            field_0x468[field_0x474] = 2;
        }
    }

    for (i = 0; i < 4; i++) {
        field_0x43c[i] = new (__FILE__, 310) Sound(field_0x3c, 1);
        if (g_UnknownGlobal56e26c->mode.field_0xa48)
            UnknownFunction4e5660(field_0x43c[i], "silence_16.wav", 41, 3);
        else
            UnknownFunction4e5660(field_0x43c[i], "silence_08.wav", 41, 3);
        field_0x44c[i] = 0;
    }
    field_0x83c = new (__FILE__, 321) Sound(field_0x3c, 1);
    UnknownFunction4e5660(field_0x83c, "LandHard01.wav", 41, 3);
    if (g_UnknownGlobal56e26c->field_0x2d74 != 3) {
        field_0x11b4 = new (__FILE__, 326) Sound(field_0x44, 1);
        UnknownFunction4e5660(field_0x11b4, "launch.wav", 1, 3);
    }

    if (g_UnknownGlobal56e26c->mode.field_0xa48) {
        field_0x1298 = 2;
        strcpy(file, "audio_16.ini");
    } else {
        field_0x1298 = 1;
        strcpy(file, "audio_08.ini");
    }
    for (k = 0; k < 3; k++) {
        if (!field_0x45c[k])
            continue;
        switch (k) {
        case 0:
            strcpy(section, "125");
            break;
        case 1:
            strcpy(section, "250");
            break;
        case 2:
            strcpy(section, "400");
            break;
        }
        stream->UnknownFunction460f50(file, "r", 0);
        parameters.UnknownFunction4b77a0((UnknownParameterStream*)stream, 0, 1);
        parameters.UnknownFunction4b78f0(section);

        parameters.UnknownFunction4b7cf0("TotalIdle", &field_0x1128[k]);
        parameters.UnknownFunction4b7cf0("TotalIdle", &count);
        if (count > 1) {
            count /= field_0x468[k];
            if (count < 1)
                count = 1;
        }
        field_0x1128[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < field_0x1128[k]; i++) {
                sprintf(key, "idle_%d", i + 1);
                parameters.UnknownFunction4b7b30(key, sample, -1);
                UnknownFunction4e5500(sample, &field_0x840[k][i], &field_0xc00[k][i]);
            }
        }

        parameters.UnknownFunction4b7cf0("TotalLowTorque", &count);
        if (count > 1) {
            count /= field_0x468[k];
            if (count < 1)
                count = 1;
        }
        field_0x1134[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < field_0x1134[k]; i++) {
                sprintf(key, "LowTorque_%d", i + 1);
                parameters.UnknownFunction4b7b30(key, sample, -1);
                UnknownFunction4e5500(sample, &field_0x8b8[k][i], &field_0xc78[k][i]);
            }
        }

        parameters.UnknownFunction4b7cf0("TotalMidTorque", &count);
        if (count > 1) {
            count /= field_0x468[k];
            if (count < 1)
                count = 1;
        }
        field_0x1140[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < field_0x1140[k]; i++) {
                sprintf(key, "MidTorque_%d", i + 1);
                parameters.UnknownFunction4b7b30(key, sample, -1);
                UnknownFunction4e5500(sample, &field_0x930[k][i], &field_0xcf0[k][i]);
            }
        }

        parameters.UnknownFunction4b7cf0("TotalHighTorque", &count);
        if (count > 1) {
            count /= field_0x468[k];
            if (count < 1)
                count = 1;
        }
        field_0x114c[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < field_0x114c[k]; i++) {
                sprintf(key, "HighTorque_%d", i + 1);
                parameters.UnknownFunction4b7b30(key, sample, -1);
                UnknownFunction4e5500(sample, &field_0x9a8[k][i], &field_0xd68[k][i]);
            }
        }

        parameters.UnknownFunction4b7cf0("TotalPowerBand", &count);
        if (count > 1) {
            count /= field_0x468[k];
            if (count < 1)
                count = 1;
        }
        field_0x1158[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < field_0x1158[k]; i++) {
                sprintf(key, "PowerBand_%d", i + 1);
                parameters.UnknownFunction4b7b30(key, sample, -1);
                UnknownFunction4e5500(sample, &field_0xa20[k][i], &field_0xde0[k][i]);
            }
        }

        parameters.UnknownFunction4b7cf0("TotalDecel", &count);
        if (count > 1) {
            count /= field_0x468[k];
            if (count < 1)
                count = 1;
        }
        field_0x1164[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < field_0x1164[k]; i++) {
                sprintf(key, "Decel_%d", i + 1);
                parameters.UnknownFunction4b7b30(key, sample, -1);
                UnknownFunction4e5500(sample, &field_0xa98[k][i], &field_0xe58[k][i]);
            }
        }

        parameters.UnknownFunction4b7cf0("TotalSputter", &count);
        if (count > 1) {
            count /= field_0x468[k];
            if (count < 1)
                count = 1;
        }
        field_0x1170[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < field_0x1170[k]; i++) {
                sprintf(key, "Sputter_%d", i + 1);
                parameters.UnknownFunction4b7b30(key, sample, -1);
                UnknownFunction4e5500(sample, &field_0xb10[k][i], &field_0xed0[k][i]);
            }
        }

        parameters.UnknownFunction4b7cf0("TotalRev", &count);
        if (count > 1) {
            count /= field_0x468[k];
            if (count < 1)
                count = 1;
        }
        field_0x117c[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < field_0x117c[k]; i++) {
                sprintf(key, "Rev_%d", i + 1);
                parameters.UnknownFunction4b7b30(key, sample, -1);
                UnknownFunction4e5500(sample, &field_0xb88[k][i], &field_0xf48[k][i]);
            }
        }

        parameters.UnknownFunction4b7f10("LowTorqueMaxSpeed", 20, &field_0x47c[k]);
        parameters.UnknownFunction4b7f10("MidTorqueMaxSpeed", 55, &field_0x488[k]);
        parameters.UnknownFunction4b7f10("HighTorqueMaxSpeed", 100, &field_0x494[k]);
    }
    if (stream)
        delete stream;

    if (g_UnknownGlobal56e26c->field_0x291c) {
        field_0x11e8 = new (__FILE__, 545) Sound(field_0x4c, 1);
        UnknownFunction4e5660(field_0x11e8, "CrowdLoop.wav", 1, 3);
        field_0x11ec = new (__FILE__, 548) Sound(field_0x4c, 1);
        UnknownFunction4e5660(field_0x11ec, "applause01.wav", 1, 3);
        field_0x11f0 = new (__FILE__, 550) Sound(field_0x4c, 1);
        UnknownFunction4e5660(field_0x11f0, "applause02.wav", 1, 3);
        field_0x11f4 = new (__FILE__, 552) Sound(field_0x4c, 1);
        UnknownFunction4e5660(field_0x11f4, "applause03.wav", 1, 3);
        field_0x11f8 = new (__FILE__, 554) Sound(field_0x4c, 1);
        UnknownFunction4e5660(field_0x11f8, "boo_01.wav", 1, 3);
        field_0x11fc = new (__FILE__, 556) Sound(field_0x4c, 1);
        UnknownFunction4e5660(field_0x11fc, "oh_01.wav", 1, 3);
    }
    field_0x11ac = new (__FILE__, 566) Sound(field_0x44, 1);
    UnknownFunction4e5660(field_0x11ac, "bonus.wav", 1, 3);
    field_0x11b0 = new (__FILE__, 570) Sound(field_0x44, 1);
    UnknownFunction4e5660(field_0x11b0, "It.wav", 1, 3);
    for (i = 0; i < 6; i++) {
        field_0x11b8[i] = new (__FILE__, 577) Sound(field_0x44, 1);
        sprintf(sample, "wreck%02d.wav", i + 1);
        UnknownFunction4e5660(field_0x11b8[i], sample, 41, 3);
    }
    for (i = 0; i < 6; i++) {
        field_0x11d0[i] = new (__FILE__, 584) Sound(field_0x44, 1);
        sprintf(sample, "fall%02d.wav", i + 1);
        UnknownFunction4e5660(field_0x11d0[i], sample, 41, 3);
    }
    field_0x1204 = new (__FILE__, 589) Sound(field_0x44, 1);
    UnknownFunction4e5660(field_0x1204, "Waypoint.wav", 1, 3);
    if (SoundSystem())
        SoundSystem()->UnknownFunction4be8b0();
    return this;
}

// 0x004e3430
void RaceSound::UnknownFunction4e3430() {
    UnknownEaxListenerParameters environment;
    int iterator;
    int i;
    int count;
    int kind;
    UnknownEventRacer* racer;
    float volume;

    for (i = 0; i < 11; i++) {
        field_0x98[i].field_0x00 = 0;
        field_0x98[i].field_0x04 = 0;
        field_0x98[i].field_0x08 = -1;
        field_0x98[i].field_0x0c = 0;
        field_0x98[i].field_0x10 = 0;
        field_0x98[i].field_0x14 = 0;
        field_0x98[i].field_0x18 = 0;
        field_0x98[i].field_0x38 = 0;
        field_0x98[i].field_0x20 = 0;
        field_0x98[i].field_0x1c = 1;
        field_0x98[i].field_0x24 = 0;
        field_0x98[i].field_0x28 = 0;
        field_0x98[i].field_0x2c = 0;
        field_0x98[i].field_0x30 = 0;
        field_0x98[i].field_0x34 = 0;
        field_0x98[i].field_0x3c = 0;
        field_0x98[i].field_0x44 = 0;
        field_0x98[i].field_0x48 = 0;
        field_0x98[i].field_0x4c = 1000000;
    }

    iterator = 0;
    count = 0;
    while ((racer = field_0x34->UnknownFunction4204e0(&iterator)) != 0) {
        field_0x98[count].field_0x00 = racer;
        if (racer->field_0x738 < 250)
            field_0x98[count].field_0x0c = 0;
        else
            field_0x98[count].field_0x0c = (racer->field_0x737 != 0) + 1;
        kind = field_0x98[count].field_0x0c;
        field_0x98[count].field_0x38 = field_0x840[kind][0];
        field_0x98[count].field_0x14 = field_0xc00[kind][0];
        field_0x98[count].field_0x18 = 0;
        field_0x98[count].field_0x10 = 1;
        field_0x98[count].field_0x20 = 0;
        field_0x98[count].field_0x1c = 1;
        count++;
    }
    field_0x94 = count;

    SoundSystem()->UnknownFunction4beba0(0.3048f);
    SoundSystem()->UnknownFunction4bebd0(1.0f);
    SoundSystem()->UnknownFunction4becd0(1.0f);
    field_0x83c->UnknownFunction4bd960(10.0f, 500.0f, 0);

    volume = (g_UnknownGlobal56e26c->mode.field_0xa40 * 0.01f) * 2500.0f - 2500.0f;
    if (volume > 0.0f)
        volume = 0.0f;
    else if (volume == -2500.0f)
        volume = -10000.0f;
    SoundSystem()->UnknownFunction4be9b0((long)volume);

    for (i = 0; i < 4; i++) {
        field_0x43c[i]->UnknownFunction4bd960(50.0f, 500.0f, 0);
        if (i < field_0x94)
            UnknownFunction4e5780(field_0x43c[i], 0, 0, 1, 0);
        if (i == field_0x94)
            break;
    }
    for (i = 0; i < 6; i++)
        field_0x11b8[i]->UnknownFunction4bd960(50.0f, 500.0f, 0);
    for (i = 0; i < 6; i++)
        field_0x11d0[i]->UnknownFunction4bd960(50.0f, 500.0f, 0);
    if (g_UnknownGlobal56e26c->field_0x291c)
        UnknownFunction4e5780(field_0x11e8, 1, 1, 1, 0);

    if (g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 3 ||
        g_UnknownGlobal56e26c->mode.UnknownFunction524100() == 4) {
        environment.environment = 9;
        environment.volume = 0.361f;
        environment.decayTime = 7.0f;
        environment.damping = 0.332f;
        field_0x54 = 1;
    } else {
        environment.environment = 17;
        environment.volume = 0.0f;
        environment.decayTime = 0.0f;
        environment.damping = 0.0f;
        field_0x54 = 0;
    }
    SoundSystem()->UnknownFunction4bed40(&environment);
    SoundSystem()->UnknownFunction4beb80();
    if (!g_UnknownGlobal56e26c->mode.field_0xa28)
        g_UnknownGlobal56e26c->field_0x34->UnknownFunction468dd0("SoundGroup");
}

// 0x004e3730: moves the listener and every racer's engine sound to their
// bikes while sound is on and the race view is not paused.
int RaceSound::UnknownVirtualSlot10(float frameTime) {
    Vector3 position;
    int i;

    GameObject::UnknownVirtualSlot10(frameTime);
    if (field_0x1208) {
        UnknownFunction4e3430();
        field_0x1208 = 0;
    }
    if (SoundSystem()->field_0x2c_bit0 && g_UnknownGlobal56e26c->mode.field_0xa28 && !field_0x34->field_0x3f8) {
        field_0x12a4 += frameTime;
        field_0x12a8 += frameTime;
        field_0x12ac += frameTime;
        field_0x2c = field_0x38->field_0x3b4;
        if (field_0x2c != field_0x12a0) {
            field_0x12a0 = field_0x2c;
            field_0x1290 = field_0x2c->field_0x7a0;
            field_0x1294 = field_0x2c->field_0x7b8;
            field_0x129c = field_0x2c->field_0x784;
        }
        UnknownFunction4e39b0(frameTime);
        SoundSystem()->UnknownFunction4bec50(field_0x38->field_0x170);
        SoundSystem()->UnknownFunction4bec00(field_0x38->field_0x17c, field_0x38->field_0x188);
        SoundSystem()->UnknownFunction4bec90(field_0x38->field_0x3b0->field_0x064);
        for (i = 0; i < field_0x94; i++) {
            if (field_0x98[i].field_0x04) {
                field_0x98[i].field_0x00->field_0x3bc->UnknownFunction4fc970(&position);
                field_0x98[i].field_0x04->UnknownFunction4bd7e0(position, 0);
                field_0x98[i].field_0x04->UnknownFunction4bd8a0(field_0x98[i].field_0x00->field_0x064, 0);
            }
        }
        field_0x2c->field_0x3bc->UnknownFunction4fc970(&position);
        field_0x83c->UnknownFunction4bd7e0(position, 0);
        field_0x83c->UnknownFunction4bd8a0(field_0x2c->field_0x064, 0);
        SoundSystem()->UnknownFunction4beb80();
    }
    return 1;
}

// 0x004e39b0: sorts the channels by distance from the camera, gives the
// nearest four racers the engine voices, steps each channel's engine sample
// set from its throttle and speed, and plays the position and lap sounds.
int RaceSound::UnknownFunction4e39b0(float frameTime) {
    int i;
    int j;

    for (i = 0; i < field_0x94; i++) {
        if (!field_0x98[i].field_0x00->field_0x4a0) {
            field_0x98[i].field_0x4c =
                (int)UnknownFunction4e4460(&field_0x38->field_0x170, &field_0x98[i].field_0x00->field_0x00c);
        } else {
            field_0x98[i].field_0x4c = 1000000;
            field_0x98[i].field_0x04 = 0;
        }
    }
    qsort(field_0x98, field_0x94, sizeof(UnknownRaceSoundChannel), UnknownFunction4e5880);

    for (i = field_0x94 - 1; i > -1; i--) {
        if (field_0x98[i].field_0x04 && i >= 4) {
            field_0x44c[field_0x98[i].field_0x08] = 0;
            field_0x98[i].field_0x04->UnknownFunction4bc940(1);
            field_0x98[i].field_0x04 = 0;
            field_0x98[i].field_0x08 = -1;
        }
        if (!field_0x98[i].field_0x04 && i < 4) {
            for (j = 0; j < 4; j++) {
                if (!field_0x44c[j]) {
                    field_0x98[i].field_0x04 = field_0x43c[j];
                    field_0x98[i].field_0x08 = j;
                    field_0x44c[j] = 1;
                    UnknownFunction4e5780(field_0x98[i].field_0x04, 0, 0, 1, 0);
                    break;
                }
            }
        }
    }

    for (i = 0; i < field_0x94; i++) {
        field_0x434 = &field_0x98[i];
        if (field_0x434->field_0x00->field_0x479)
            field_0x434->field_0x48 = 0;
        else
            field_0x434->field_0x48 = 1;
        if (field_0x434->field_0x00->field_0x108) {
            field_0x434->field_0x3c += frameTime;
            if (field_0x434->field_0x3c >= 0.2f) {
                if (!field_0x434->field_0x00->field_0x478)
                    field_0x434->field_0x30 = 1;
                if (field_0x434->field_0x10 != 5 && field_0x434->field_0x10 != 6 && field_0x434->field_0x10 != 1) {
                    j = rand() % field_0x1164[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xa98[field_0x434->field_0x0c][j];
                    field_0x434->field_0x14 = field_0xe58[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x10 = 5;
                    field_0x434->field_0x20 = 1;
                    UnknownFunction4e4a30();
                }
                if (field_0x434->field_0x30 == 1 && field_0x434->field_0x00->field_0x478) {
                    field_0x434->field_0x30 = 0;
                    j = rand() % field_0x117c[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xb88[field_0x434->field_0x0c][j];
                    field_0x434->field_0x14 = field_0xf48[field_0x434->field_0x0c][j];
                    // Retail compares the old state with 6 and then restarts
                    // the sample either way.
                    if (field_0x434->field_0x10 != 6)
                        field_0x434->field_0x18 = 0;
                    else
                        field_0x434->field_0x18 = 0;
                    field_0x434->field_0x10 = 6;
                    field_0x434->field_0x20 = 1;
                    UnknownFunction4e4a30();
                }
            }
        } else {
            field_0x434->field_0x3c = 0;
            field_0x434->field_0x30 = 0;
            if (field_0x434->field_0x2c == 1 && !field_0x83c->UnknownFunction4bca80() &&
                field_0x434->field_0x00 == field_0x2c)
                UnknownFunction4e5780(field_0x83c, 0, 0, 0, 0);
            if (!field_0x434->field_0x00->field_0x478)
                field_0x434->field_0x34 = 1;
            if (field_0x434->field_0x48) {
                if (field_0x434->field_0x00->field_0x478) {
                    if (field_0x434->field_0x10 == 6) {
                        j = rand() % field_0x114c[field_0x434->field_0x0c];
                        field_0x434->field_0x38 = field_0x9a8[field_0x434->field_0x0c][j];
                        field_0x434->field_0x14 = field_0xd68[field_0x434->field_0x0c][j];
                        field_0x434->field_0x18 = 0;
                        field_0x434->field_0x10 = 2;
                        field_0x434->field_0x20 = 3;
                        UnknownFunction4e4a30();
                    }
                    if (field_0x434->field_0x10 == 1 || field_0x434->field_0x10 == 5) {
                        field_0x434->field_0x44 = (int)(field_0x434->field_0x00->field_0x0b8 * 0.68182f);
                        if (field_0x434->field_0x44 < field_0x47c[field_0x434->field_0x0c]) {
                            j = rand() % field_0x1134[field_0x434->field_0x0c];
                            field_0x434->field_0x38 = field_0x8b8[field_0x434->field_0x0c][j];
                            field_0x434->field_0x14 = field_0xc78[field_0x434->field_0x0c][j];
                            field_0x434->field_0x18 = 0;
                            field_0x434->field_0x10 = 2;
                            field_0x434->field_0x20 = 3;
                        } else if (field_0x434->field_0x44 < field_0x488[field_0x434->field_0x0c]) {
                            j = rand() % field_0x1140[field_0x434->field_0x0c];
                            field_0x434->field_0x38 = field_0x930[field_0x434->field_0x0c][j];
                            field_0x434->field_0x14 = field_0xcf0[field_0x434->field_0x0c][j];
                            field_0x434->field_0x18 = 0;
                            field_0x434->field_0x10 = 2;
                            field_0x434->field_0x20 = 3;
                        } else if (field_0x434->field_0x44 < field_0x494[field_0x434->field_0x0c]) {
                            j = rand() % field_0x114c[field_0x434->field_0x0c];
                            field_0x434->field_0x38 = field_0x9a8[field_0x434->field_0x0c][j];
                            field_0x434->field_0x14 = field_0xd68[field_0x434->field_0x0c][j];
                            field_0x434->field_0x18 = 0;
                            field_0x434->field_0x10 = 2;
                            field_0x434->field_0x20 = 3;
                        } else {
                            j = rand() % field_0x1158[field_0x434->field_0x0c];
                            field_0x434->field_0x38 = field_0xa20[field_0x434->field_0x0c][j];
                            field_0x434->field_0x14 = field_0xde0[field_0x434->field_0x0c][j];
                            field_0x434->field_0x18 = 0;
                            field_0x434->field_0x10 = 3;
                            field_0x434->field_0x20 = 4;
                        }
                        UnknownFunction4e4a30();
                    }
                } else if (field_0x434->field_0x10 == 2 || field_0x434->field_0x10 == 3 ||
                           field_0x434->field_0x10 == 4) {
                    j = rand() % field_0x1164[field_0x434->field_0x0c];
                    field_0x434->field_0x38 = field_0xa98[field_0x434->field_0x0c][j];
                    field_0x434->field_0x14 = field_0xe58[field_0x434->field_0x0c][j];
                    field_0x434->field_0x18 = 0;
                    field_0x434->field_0x10 = 5;
                    field_0x434->field_0x20 = 1;
                    UnknownFunction4e4a30();
                }
            } else if (field_0x434->field_0x00->field_0x478 == true &&
                       (field_0x434->field_0x34 == 1 || field_0x434->field_0x28 == 1)) {
                field_0x434->field_0x34 = 0;
                j = rand() % 1;
                field_0x434->field_0x38 = field_0xb88[field_0x434->field_0x0c][j];
                field_0x434->field_0x14 = field_0xf48[field_0x434->field_0x0c][j];
                if (field_0x434->field_0x10 != 6)
                    field_0x434->field_0x18 = 0;
                else
                    field_0x434->field_0x18 = 0;
                field_0x434->field_0x10 = 6;
                field_0x434->field_0x20 = 1;
                UnknownFunction4e4a30();
            }
        }
        field_0x434->field_0x28 = field_0x434->field_0x48;
        field_0x434->field_0x24 = (unsigned char)field_0x434->field_0x00->field_0x478;
        field_0x434->field_0x2c = (unsigned char)field_0x434->field_0x00->field_0x108;
        UnknownFunction4e4d10();
    }

    if (g_UnknownGlobal56e26c->mode.field_0xa38 &&
        (field_0x2c->field_0x7b8 > field_0x1294 || field_0x2c->field_0x7a0 > field_0x1290))
        UnknownFunction4e5780(field_0x1204, 0, 0, 0, 0);

    for (i = 0; i < field_0x94; i++) {
        field_0x438 = field_0x98[i].field_0x00;
        if (!field_0x438->field_0x444) {
            field_0x120c[i] = 0;
            field_0x1238[i] = 0;
            field_0x1264[i] = 0;
        } else {
            if (!field_0x120c[i]) {
                if (field_0x438 == field_0x2c && field_0x2c->field_0x460 == 12) {
                    field_0x11b4->UnknownFunction4bc940(1);
                    UnknownFunction4e5780(field_0x11b4, 0, 0, 0, 0);
                }
                if (g_UnknownGlobal56e26c->field_0x2d74 == 3) {
                    if (field_0x438 == field_0x2c && field_0x438->field_0x784 == 1)
                        UnknownFunction4e5780(field_0x11f8, 0, 0, 0, 0);
                    if (field_0x438 == field_0x2c && field_0x438->field_0x784 > 1 && field_0x12a4 > 30.0f) {
                        UnknownFunction4e5780(field_0x11fc, 0, 0, 0, 0);
                        field_0x12a4 = 0;
                    }
                }
            }
            if (field_0x438->field_0x484) {
                if (!field_0x1238[i])
                    field_0x1238[i] = UnknownFunction4e42e0(field_0x438);
                else if (!field_0x11b8[field_0x1238[i]]->UnknownFunction4bca80())
                    field_0x1238[i] = UnknownFunction4e42e0(field_0x438);
            }
            if (field_0x438->field_0x604->field_0xb0) {
                if (!field_0x1264[i])
                    field_0x1264[i] = UnknownFunction4e43a0(field_0x438);
                else if (!field_0x11d0[field_0x1264[i]]->UnknownFunction4bca80())
                    field_0x1264[i] = UnknownFunction4e43a0(field_0x438);
            }
            field_0x120c[i] = 1;
        }
        if (field_0x438 == field_0x2c && field_0x438->field_0x784 < field_0x129c) {
            if (field_0x438->field_0x784 == 1) {
                if (field_0x11f4)
                    UnknownFunction4e5780(field_0x11f4, 0, 0, 0, 0);
            } else if (field_0x12a8 > 15.0f) {
                if (rand() > 0.5f) {
                    if (field_0x11ec)
                        UnknownFunction4e5780(field_0x11ec, 0, 0, 0, 0);
                } else if (field_0x11f0)
                    UnknownFunction4e5780(field_0x11f0, 0, 0, 0, 0);
                field_0x12a8 = 0;
            }
        }
    }
    field_0x1290 = field_0x2c->field_0x7a0;
    field_0x1294 = field_0x2c->field_0x7b8;
    field_0x129c = field_0x2c->field_0x784;
    return 1;
}

// 0x004e44e0: Ctrl+V toggles the EAX environment, Ctrl+C cycles the music
// volume (race mode 3 only) and Ctrl+S turns all sound on or off.
int RaceSound::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    UnknownEaxListenerParameters environment;
    char text[0x80];
    TrackGameViewOwner* owner;

    if (GameObject::UnknownVirtualSlot23(event, entry))
        return 1;
    owner = g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0();
    if (!event->kind) {
        switch (event->control) {
        case 0x2f:
            if (!g_UnknownGlobal56e26c->field_0x14->keyboard->UnknownVirtualSlot5(0x2f, 12, 0))
                break;
            if (!field_0x54) {
                environment.environment = 9;
                environment.volume = 0.361f;
                environment.decayTime = 7.0f;
                environment.damping = 0.332f;
                field_0x54 = 1;
            } else {
                environment.environment = 17;
                environment.volume = 0.0f;
                environment.decayTime = 0.0f;
                environment.damping = 0.0f;
                field_0x54 = 0;
            }
            if (owner)
                owner->UnknownFunction4e0c30(0x14c3, field_0x54);
            SoundSystem()->UnknownFunction4bed40(&environment);
            break;
        case 0x2e:
            if (!g_UnknownGlobal56e26c->field_0x14->keyboard->UnknownVirtualSlot5(0x2e, 12, 0))
                break;
            if (g_UnknownGlobal56e26c->field_0x2d74 != 3)
                break;
            switch (field_0x50) {
            case 0:
                field_0x50 = -500;
                g_UnknownGlobal56e26c->UnknownFunction521970(0x14c0, text, sizeof(text));
                break;
            case -500:
                field_0x50 = -10000;
                g_UnknownGlobal56e26c->UnknownFunction521970(0x14c2, text, sizeof(text));
                break;
            case -10000:
                field_0x50 = 0;
                g_UnknownGlobal56e26c->UnknownFunction521970(0x14c1, text, sizeof(text));
                break;
            }
            {
                UnknownMessage message(text, 1.5f);
                if (owner)
                    owner->field_0x6c->UnknownFunction51b540(&message);
            }
            if (g_UnknownGlobal56e26c->field_0x291c)
                field_0x4c->UnknownFunction401dd0(field_0x50);
            return 1;
        case 0x1f:
            if (!g_UnknownGlobal56e26c->field_0x14->keyboard->UnknownVirtualSlot5(0x1f, 12, 0))
                break;
            g_UnknownGlobal56e26c->mode.field_0xa28 = !g_UnknownGlobal56e26c->mode.field_0xa28;
            if (g_UnknownGlobal56e26c->mode.field_0xa28) {
                g_UnknownGlobal56e26c->field_0x34->UnknownFunction468f10("SoundGroup");
                if (field_0x3c)
                    field_0x3c->UnknownVirtualSlot16(0);
                if (field_0x40)
                    field_0x40->UnknownVirtualSlot16(0);
                if (field_0x44)
                    field_0x44->UnknownVirtualSlot16(0);
                if (g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0()->field_0x2c->field_0xc4)
                    g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0()->field_0x2c->field_0xc4->UnknownVirtualSlot16(0);
                if (g_UnknownGlobal56e26c->field_0x291c)
                    UnknownFunction4e5780(field_0x11e8, 1, 1, 1, 0);
                if (field_0x2c) {
                    field_0x1294 = field_0x2c->field_0x7b8;
                    field_0x1290 = field_0x2c->field_0x7a0;
                }
                if (g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0()->field_0x9c)
                    g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0()->field_0x9c->UnknownVirtualSlot5();
            } else {
                g_UnknownGlobal56e26c->field_0x34->UnknownFunction468dd0("SoundGroup");
                if (field_0x3c)
                    field_0x3c->UnknownVirtualSlot16(1);
                if (field_0x40)
                    field_0x40->UnknownVirtualSlot16(1);
                if (field_0x44)
                    field_0x44->UnknownVirtualSlot16(1);
                if (g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0()->field_0x2c->field_0xc4)
                    g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0()->field_0x2c->field_0xc4->UnknownVirtualSlot16(1);
                if (g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0()->field_0x9c)
                    g_UnknownGlobal56e26c->eventManager->UnknownFunction45d2b0()->field_0x9c->UnknownVirtualSlot4();
            }
            if (owner)
                owner->UnknownFunction4e0c30(0x1429, g_UnknownGlobal56e26c->mode.field_0xa28);
            return 1;
        }
    }
    return 0;
}

// 0x004e42e0
int RaceSound::UnknownFunction4e42e0(UnknownEventRacer* racer) {
    Vector3 position;
    int tries = 0;
    int index = rand() % 6;

    while (field_0x11b8[index]->UnknownFunction4bca80()) {
        if (++tries >= 10)
            return 0;
        index = rand() % 6;
    }
    racer->field_0x3bc->UnknownFunction4fc970(&position);
    field_0x11b8[index]->UnknownFunction4bd7e0(position, 0);
    UnknownFunction4e5780(field_0x11b8[index], 0, 0, 0, 0);
    return index;
}

// 0x004e43a0
int RaceSound::UnknownFunction4e43a0(UnknownEventRacer* racer) {
    Vector3 position;
    int tries = 0;
    int index = rand() % 6;

    while (field_0x11d0[index]->UnknownFunction4bca80()) {
        if (++tries >= 10)
            return 0;
        index = rand() % 6;
    }
    racer->field_0x3bc->UnknownFunction4fc970(&position);
    field_0x11d0[index]->UnknownFunction4bd7e0(position, 0);
    UnknownFunction4e5780(field_0x11d0[index], 0, 0, 0, 0);
    return index;
}

// 0x004e4460
float RaceSound::UnknownFunction4e4460(Vector3* a, Vector3* b) {
    float dx = a->x - b->x;
    float dz;

    if (dx < 0.0f)
        dx = -dx;
    dz = a->z - b->z;
    if (dz < 0.0f)
        dz = -dz;
    if (dx <= 500.0f && dz <= 500.0f)
        return UnknownFunction460b50(dz * dz + dx * dx);
    return 500.0f;
}

// 0x004e4a30: fills the streamed channel's buffer from its sample, starting
// behind the play cursor (the buffer holds one second, 22050 samples).
void RaceSound::UnknownFunction4e4a30() {
    void* first;
    unsigned long write;
    void* second;
    unsigned long play;
    unsigned long secondBytes;
    unsigned long firstBytes;
    int bytes;

    if (!field_0x434->field_0x04)
        return;
    field_0x434->field_0x04->UnknownFunction4bcd40(&play, &write);
    field_0x434->field_0x04->UnknownFunction4bd020(0, 0, &first, &firstBytes, &second, &secondBytes, 2);
    if (write < (unsigned long)(5512 * field_0x1298)) {
        bytes = 11025 * field_0x1298 - write;
        memcpy((char*)field_0x434->field_0x50 + write, (char*)field_0x434->field_0x38 + field_0x434->field_0x18,
               bytes);
        memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
        field_0x434->field_0x18 += bytes;
        field_0x434->field_0x1c = 2;
    } else if (write < (unsigned long)(11025 * field_0x1298)) {
        bytes = 22050 * field_0x1298 - write;
        memcpy((char*)field_0x434->field_0x50 + write, (char*)field_0x434->field_0x38 + field_0x434->field_0x18,
               bytes);
        memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
        field_0x434->field_0x18 += bytes;
        field_0x434->field_0x1c = 1;
    } else if (write < (unsigned long)(16537 * field_0x1298)) {
        bytes = 22050 * field_0x1298 - write;
        memcpy((char*)field_0x434->field_0x50 + write, (char*)field_0x434->field_0x38 + field_0x434->field_0x18,
               bytes);
        memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
        field_0x434->field_0x18 += bytes;
        field_0x434->field_0x1c = 1;
    } else {
        bytes = 22050 * field_0x1298 - write;
        memcpy((char*)field_0x434->field_0x50 + write, (char*)field_0x434->field_0x38 + field_0x434->field_0x18,
               bytes);
        field_0x434->field_0x18 += bytes;
        memcpy(field_0x434->field_0x50, (char*)field_0x434->field_0x38 + field_0x434->field_0x18,
               11025 * field_0x1298);
        memcpy(first, field_0x434->field_0x50, 22050 * field_0x1298);
        field_0x434->field_0x18 += 11025 * field_0x1298;
        field_0x434->field_0x1c = 2;
    }
    field_0x434->field_0x04->UnknownFunction4bd080(first, 22050 * field_0x1298, second, 0);
}

// The four per-TU vector constants (see src/krusty2/math/Math3D.h); their
// dynamic initializers follow slot 23 in the code.
static const Vector3 s_UnknownVector689bd0 = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector689be0 = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector689bf0 = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 s_UnknownVector689bc0 = Vector3(0.0f, 0.0f, 1.0f);

// 0x004e5500
void RaceSound::UnknownFunction4e5500(const char* name, void** data, int* size) {
    char message[0x184];
    UnknownTextureStream* stream = new (__FILE__, 1630) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    int bytes;
    void* buffer;

    if (!g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, name, "rb", 0)) {
        sprintf(message, "%s not found in Audio.res.\n", name);
    } else {
        stream->UnknownFunction461340(stream->field_0x130, 0, 1);
        stream->UnknownFunction461640(&field_0x68, 1, sizeof(field_0x68));
        bytes = field_0x68.dataSize;
        buffer = DebugMalloc(bytes, __FILE__, 1652);
        if (!buffer) {
            *data = 0;
            *size = 0;
        } else {
            stream->UnknownFunction461640(buffer, 1, bytes);
            if (stream)
                delete stream;
            *data = buffer;
            *size = bytes;
            return;
        }
    }
    if (stream)
        delete stream;
}

// 0x004e5660
int RaceSound::UnknownFunction4e5660(Sound* sound, const char* name, int a, int b) {
    char message[0x184];
    UnknownTextureStream* stream;

    if (!sound)
        return 0;
    stream = new (__FILE__, 1678) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (!g_UnknownGlobal56e26c->sceneObject->UnknownFunction4e9cd0(stream, name, "rb", 0)) {
        sprintf(message, "%s not found in Audio.res.\n", name);
        if (stream)
            delete stream;
        return 0;
    }
    if (!sound->UnknownFunction4bc320(name, stream, a, b, 0, -1)) {
        if (stream)
            delete stream;
        return 0;
    }
    if (stream)
        delete stream;
    return 1;
}

// 0x004e5780
void RaceSound::UnknownFunction4e5780(Sound* sound, int network, int stop, int loop, int unused) {
    if (g_UnknownGlobal56e26c->mode.field_0xa28 && (g_UnknownGlobal56e26c->field_0x291c || !network) && sound) {
        if (stop)
            sound->UnknownFunction4bc940(1);
        sound->UnknownFunction4bc6b0(0, loop, 1);
    }
}

// 0x004e57d0
void RaceSound::UnknownFunction4e57d0(UnknownEventRacer* racer, float value) {
    if (field_0x2c == racer && value >= 10000.0f &&
        (!g_UnknownGlobal56e26c->field_0x2d74 || g_UnknownGlobal56e26c->field_0x2d74 == 4))
        UnknownFunction4e5780(field_0x11ac, 0, 0, 0, 0);
    if (g_UnknownGlobal56e26c->field_0x2d74 == 3 && field_0x12ac > 15.0f) {
        UnknownFunction4e5780(field_0x11ec, 0, 0, 0, 0);
        field_0x12ac = 0;
    }
}

// 0x004e5860
void RaceSound::UnknownFunction4e5860() {
    UnknownFunction4e5780(field_0x11b0, 0, 0, 0, 0);
}

// 0x004e5880
int RaceSound::UnknownFunction4e5880(const void* a, const void* b) {
    int first = ((const UnknownRaceSoundChannel*)a)->field_0x4c;
    int second = ((const UnknownRaceSoundChannel*)b)->field_0x4c;

    if (first < second)
        return -1;
    return first != second;
}

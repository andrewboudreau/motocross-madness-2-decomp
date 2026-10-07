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

#define SoundSystem() ((PCSoundInterface*)g_TrackGame->soundInterface)

// 0x004e1fb0
RaceSound::RaceSound(int flags) : GameObject(flags) {
    int i;
    int k;

    field_0x12a0 = 0;
    field_0x129c = 99;
    field_0x12a4 = 0;
    field_0x12a8 = 0;
    field_0x12ac = 0;
    streamChannel = 0;
    channelsInUse = 0;
    currentRacer = 0;
    field_0x1290 = 0;
    field_0x1294 = 0;
    field_0x1204 = 0;
    field_0x11e8 = 0;
    listenerRacer = 0;
    field_0x30 = 0;
    raceView = 0;
    listenerCamera = 0;
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
    musicVolume = 0;
    field_0x11ec = 0;
    field_0x11f0 = 0;
    field_0x11f4 = 0;
    for (i = 0; i < 11; i++) {
        field_0x120c[i] = 0;
        field_0x1264[i] = 0;
        field_0x1238[i] = 0;
        engineChannels[i].sampleBuffer = 0;
        engineChannels[i].sampleBuffer = DebugMalloc(44100, __FILE__, 77);
        memset(engineChannels[i].sampleBuffer, 0, 44100);
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
                DebugFree(field_0x840[k][i], __FILE__, 128);
            field_0x840[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0x8b8[k][i])
                DebugFree(field_0x8b8[k][i], __FILE__, 132);
            field_0x8b8[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0x930[k][i])
                DebugFree(field_0x930[k][i], __FILE__, 136);
            field_0x930[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0x9a8[k][i])
                DebugFree(field_0x9a8[k][i], __FILE__, 140);
            field_0x9a8[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0xa20[k][i])
                DebugFree(field_0xa20[k][i], __FILE__, 144);
            field_0xa20[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0xa98[k][i])
                DebugFree(field_0xa98[k][i], __FILE__, 148);
            field_0xa98[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0xb10[k][i])
                DebugFree(field_0xb10[k][i], __FILE__, 152);
            field_0xb10[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (field_0xb88[k][i])
                DebugFree(field_0xb88[k][i], __FILE__, 156);
            field_0xb88[k][i] = 0;
        }
    }
    for (i = 0; i < 11; i++) {
        if (engineChannels[i].sampleBuffer)
            DebugFree(engineChannels[i].sampleBuffer, __FILE__, 177);
        engineChannels[i].sampleBuffer = 0;
    }
    SoundSystem()->UnknownFunction4be9b0(-10000);
    Release();
}

// 0x004e23f0: creates the four sound groups and the sounds, then reads each
// present engine's section ("125", "250", "400") of audio_16.ini or
// audio_08.ini: the sample counts (divided down when several engines share
// the race) and names of eight sample sets, and three engine speeds.
RaceSound* RaceSound::Create(void* owner, UnknownKrustyBikeView* view,
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
    raceView = view;
    listenerCamera = camera;
    field_0x3c = new (__FILE__, 213) SoundGroup(1);
    AppendChild(field_0x3c, -1);
    field_0x44 = new (__FILE__, 214) SoundGroup(1);
    AppendChild(field_0x44, -1);
    crowdSounds = new (__FILE__, 215) SoundGroup(1);
    AppendChild(crowdSounds, -1);
    field_0x48 = new (__FILE__, 216) SoundGroup(1);
    AppendChild(field_0x48, -1);
    if (SoundSystem())
        SoundSystem()->UnknownFunction4be8b0();

    field_0x45c[0] = 0;
    field_0x45c[1] = 0;
    field_0x45c[2] = 0;
    iterator = 0;
    while ((racer = raceView->UnknownFunction4204e0(&iterator)) != 0) {
        if (racer->field_0x738 < 250) {
            field_0x45c[0] = 1;
            if (racer == raceView->field_0x38)
                field_0x474 = 0;
        } else if (!racer->field_0x737) {
            field_0x45c[1] = 1;
            if (racer == raceView->field_0x38)
                field_0x474 = 1;
        } else {
            field_0x45c[2] = 1;
            if (racer == raceView->field_0x38)
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
    if (!g_TrackGame->mode.field_0xa48) {
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
        engineVoices[i] = new (__FILE__, 310) Sound(field_0x3c, 1);
        if (g_TrackGame->mode.field_0xa48)
            LoadSound(engineVoices[i], "silence_16.wav", 41, 3);
        else
            LoadSound(engineVoices[i], "silence_08.wav", 41, 3);
        engineVoiceInUse[i] = 0;
    }
    ownEngine = new (__FILE__, 321) Sound(field_0x3c, 1);
    LoadSound(ownEngine, "LandHard01.wav", 41, 3);
    if (g_TrackGame->mode.field_0x27f8.field_0x04 != 3) {
        field_0x11b4 = new (__FILE__, 326) Sound(field_0x44, 1);
        LoadSound(field_0x11b4, "launch.wav", 1, 3);
    }

    if (g_TrackGame->mode.field_0xa48) {
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
                ReadSample(sample, &field_0x840[k][i], &field_0xc00[k][i]);
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
                ReadSample(sample, &field_0x8b8[k][i], &field_0xc78[k][i]);
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
                ReadSample(sample, &field_0x930[k][i], &field_0xcf0[k][i]);
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
                ReadSample(sample, &field_0x9a8[k][i], &field_0xd68[k][i]);
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
                ReadSample(sample, &field_0xa20[k][i], &field_0xde0[k][i]);
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
                ReadSample(sample, &field_0xa98[k][i], &field_0xe58[k][i]);
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
                ReadSample(sample, &field_0xb10[k][i], &field_0xed0[k][i]);
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
                ReadSample(sample, &field_0xb88[k][i], &field_0xf48[k][i]);
            }
        }

        parameters.UnknownFunction4b7f10("LowTorqueMaxSpeed", 20, &field_0x47c[k]);
        parameters.UnknownFunction4b7f10("MidTorqueMaxSpeed", 55, &field_0x488[k]);
        parameters.UnknownFunction4b7f10("HighTorqueMaxSpeed", 100, &field_0x494[k]);
    }
    if (stream)
        delete stream;

    if (g_TrackGame->mode.field_0x23a4) {
        field_0x11e8 = new (__FILE__, 545) Sound(crowdSounds, 1);
        LoadSound(field_0x11e8, "CrowdLoop.wav", 1, 3);
        field_0x11ec = new (__FILE__, 548) Sound(crowdSounds, 1);
        LoadSound(field_0x11ec, "applause01.wav", 1, 3);
        field_0x11f0 = new (__FILE__, 550) Sound(crowdSounds, 1);
        LoadSound(field_0x11f0, "applause02.wav", 1, 3);
        field_0x11f4 = new (__FILE__, 552) Sound(crowdSounds, 1);
        LoadSound(field_0x11f4, "applause03.wav", 1, 3);
        field_0x11f8 = new (__FILE__, 554) Sound(crowdSounds, 1);
        LoadSound(field_0x11f8, "boo_01.wav", 1, 3);
        field_0x11fc = new (__FILE__, 556) Sound(crowdSounds, 1);
        LoadSound(field_0x11fc, "oh_01.wav", 1, 3);
    }
    field_0x11ac = new (__FILE__, 566) Sound(field_0x44, 1);
    LoadSound(field_0x11ac, "bonus.wav", 1, 3);
    field_0x11b0 = new (__FILE__, 570) Sound(field_0x44, 1);
    LoadSound(field_0x11b0, "It.wav", 1, 3);
    for (i = 0; i < 6; i++) {
        field_0x11b8[i] = new (__FILE__, 577) Sound(field_0x44, 1);
        sprintf(sample, "wreck%02d.wav", i + 1);
        LoadSound(field_0x11b8[i], sample, 41, 3);
    }
    for (i = 0; i < 6; i++) {
        field_0x11d0[i] = new (__FILE__, 584) Sound(field_0x44, 1);
        sprintf(sample, "fall%02d.wav", i + 1);
        LoadSound(field_0x11d0[i], sample, 41, 3);
    }
    field_0x1204 = new (__FILE__, 589) Sound(field_0x44, 1);
    LoadSound(field_0x1204, "Waypoint.wav", 1, 3);
    if (SoundSystem())
        SoundSystem()->UnknownFunction4be8b0();
    return this;
}

// 0x004e3430
void RaceSound::AssignChannels() {
    UnknownEaxListenerParameters environment;
    int iterator;
    int i;
    int count;
    int kind;
    UnknownEventRacer* racer;
    float volume;

    for (i = 0; i < 11; i++) {
        engineChannels[i].channelRacer = 0;
        engineChannels[i].channelSound = 0;
        engineChannels[i].field_0x08 = -1;
        engineChannels[i].field_0x0c = 0;
        engineChannels[i].field_0x10 = 0;
        engineChannels[i].field_0x14 = 0;
        engineChannels[i].field_0x18 = 0;
        engineChannels[i].streamSample = 0;
        engineChannels[i].field_0x20 = 0;
        engineChannels[i].field_0x1c = 1;
        engineChannels[i].field_0x24 = 0;
        engineChannels[i].field_0x28 = 0;
        engineChannels[i].field_0x2c = 0;
        engineChannels[i].field_0x30 = 0;
        engineChannels[i].field_0x34 = 0;
        engineChannels[i].airborneTime = 0;
        engineChannels[i].field_0x44 = 0;
        engineChannels[i].field_0x48 = 0;
        engineChannels[i].field_0x4c = 1000000;
    }

    iterator = 0;
    count = 0;
    while ((racer = raceView->UnknownFunction4204e0(&iterator)) != 0) {
        engineChannels[count].channelRacer = racer;
        if (racer->field_0x738 < 250)
            engineChannels[count].field_0x0c = 0;
        else
            engineChannels[count].field_0x0c = (racer->field_0x737 != 0) + 1;
        kind = engineChannels[count].field_0x0c;
        engineChannels[count].streamSample = field_0x840[kind][0];
        engineChannels[count].field_0x14 = field_0xc00[kind][0];
        engineChannels[count].field_0x18 = 0;
        engineChannels[count].field_0x10 = 1;
        engineChannels[count].field_0x20 = 0;
        engineChannels[count].field_0x1c = 1;
        count++;
    }
    channelsInUse = count;

    SoundSystem()->UnknownFunction4beba0(0.3048f);
    SoundSystem()->UnknownFunction4bebd0(1.0f);
    SoundSystem()->UnknownFunction4becd0(1.0f);
    ownEngine->UnknownFunction4bd960(10.0f, 500.0f, 0);

    volume = (g_TrackGame->mode.field_0xa40 * 0.01f) * 2500.0f - 2500.0f;
    if (volume > 0.0f)
        volume = 0.0f;
    else if (volume == -2500.0f)
        volume = -10000.0f;
    SoundSystem()->UnknownFunction4be9b0((long)volume);

    for (i = 0; i < 4; i++) {
        engineVoices[i]->UnknownFunction4bd960(50.0f, 500.0f, 0);
        if (i < channelsInUse)
            PlayIfEnabled(engineVoices[i], 0, 0, 1, 0);
        if (i == channelsInUse)
            break;
    }
    for (i = 0; i < 6; i++)
        field_0x11b8[i]->UnknownFunction4bd960(50.0f, 500.0f, 0);
    for (i = 0; i < 6; i++)
        field_0x11d0[i]->UnknownFunction4bd960(50.0f, 500.0f, 0);
    if (g_TrackGame->mode.field_0x23a4)
        PlayIfEnabled(field_0x11e8, 1, 1, 1, 0);

    if (g_TrackGame->mode.UnknownFunction524100() == 3 ||
        g_TrackGame->mode.UnknownFunction524100() == 4) {
        environment.environment = 9;
        environment.volume = 0.361f;
        environment.decayTime = 7.0f;
        environment.damping = 0.332f;
        eaxEnabled = 1;
    } else {
        environment.environment = 17;
        environment.volume = 0.0f;
        environment.decayTime = 0.0f;
        environment.damping = 0.0f;
        eaxEnabled = 0;
    }
    SoundSystem()->UnknownFunction4bed40(&environment);
    SoundSystem()->UnknownFunction4beb80();
    if (!g_TrackGame->mode.field_0xa28)
        g_TrackGame->field_0x34->UnknownFunction468dd0("SoundGroup");
}

// 0x004e3730: moves the listener and every racer's engine sound to their
// bikes while sound is on and the race view is not paused.
int RaceSound::UnknownVirtualSlot10(float frameTime) {
    Vector3 position;
    int i;

    GameObject::UnknownVirtualSlot10(frameTime);
    if (field_0x1208) {
        AssignChannels();
        field_0x1208 = 0;
    }
    if (SoundSystem()->field_0x2c_bit0 && g_TrackGame->mode.field_0xa28 && !raceView->field_0x3f8) {
        field_0x12a4 += frameTime;
        field_0x12a8 += frameTime;
        field_0x12ac += frameTime;
        listenerRacer = listenerCamera->followedRacer;
        if (listenerRacer != field_0x12a0) {
            field_0x12a0 = listenerRacer;
            field_0x1290 = listenerRacer->field_0x7a0;
            field_0x1294 = listenerRacer->field_0x7b8;
            field_0x129c = listenerRacer->field_0x784;
        }
        UpdateRacerSounds(frameTime);
        SoundSystem()->UnknownFunction4bec50(listenerCamera->listenerPosition);
        SoundSystem()->UnknownFunction4bec00(listenerCamera->listenerForward, listenerCamera->listenerUp);
        SoundSystem()->UnknownFunction4bec90(listenerCamera->followedBike->field_0x064);
        for (i = 0; i < channelsInUse; i++) {
            if (engineChannels[i].channelSound) {
                engineChannels[i].channelRacer->field_0x3bc->UnknownFunction4fc970(&position);
                engineChannels[i].channelSound->UnknownFunction4bd7e0(position, 0);
                engineChannels[i].channelSound->UnknownFunction4bd8a0(engineChannels[i].channelRacer->field_0x064, 0);
            }
        }
        listenerRacer->field_0x3bc->UnknownFunction4fc970(&position);
        ownEngine->UnknownFunction4bd7e0(position, 0);
        ownEngine->UnknownFunction4bd8a0(listenerRacer->field_0x064, 0);
        SoundSystem()->UnknownFunction4beb80();
    }
    return 1;
}

// 0x004e39b0: sorts the channels by distance from the camera, gives the
// nearest four racers the engine voices, steps each channel's engine sample
// set from its throttle and speed, and plays the position and lap sounds.
int RaceSound::UpdateRacerSounds(float frameTime) {
    int i;
    int j;

    for (i = 0; i < channelsInUse; i++) {
        if (!engineChannels[i].channelRacer->field_0x4a0) {
            engineChannels[i].field_0x4c =
                (int)GroundDistance(&listenerCamera->listenerPosition, &engineChannels[i].channelRacer->field_0x00c);
        } else {
            engineChannels[i].field_0x4c = 1000000;
            engineChannels[i].channelSound = 0;
        }
    }
    qsort(engineChannels, channelsInUse, sizeof(UnknownRaceSoundChannel), CompareChannels);

    for (i = channelsInUse - 1; i > -1; i--) {
        if (engineChannels[i].channelSound && i >= 4) {
            engineVoiceInUse[engineChannels[i].field_0x08] = 0;
            engineChannels[i].channelSound->UnknownFunction4bc940(1);
            engineChannels[i].channelSound = 0;
            engineChannels[i].field_0x08 = -1;
        }
        if (!engineChannels[i].channelSound && i < 4) {
            for (j = 0; j < 4; j++) {
                if (!engineVoiceInUse[j]) {
                    engineChannels[i].channelSound = engineVoices[j];
                    engineChannels[i].field_0x08 = j;
                    engineVoiceInUse[j] = 1;
                    PlayIfEnabled(engineChannels[i].channelSound, 0, 0, 1, 0);
                    break;
                }
            }
        }
    }

    for (i = 0; i < channelsInUse; i++) {
        streamChannel = &engineChannels[i];
        if (streamChannel->channelRacer->field_0x479)
            streamChannel->field_0x48 = 0;
        else
            streamChannel->field_0x48 = 1;
        if (streamChannel->channelRacer->field_0x108) {
            streamChannel->airborneTime += frameTime;
            if (streamChannel->airborneTime >= 0.2f) {
                if (!streamChannel->channelRacer->field_0x478)
                    streamChannel->field_0x30 = 1;
                if (streamChannel->field_0x10 != 5 && streamChannel->field_0x10 != 6 && streamChannel->field_0x10 != 1) {
                    j = rand() % field_0x1164[streamChannel->field_0x0c];
                    streamChannel->streamSample = field_0xa98[streamChannel->field_0x0c][j];
                    streamChannel->field_0x14 = field_0xe58[streamChannel->field_0x0c][j];
                    streamChannel->field_0x18 = 0;
                    streamChannel->field_0x10 = 5;
                    streamChannel->field_0x20 = 1;
                    StartStream();
                }
                if (streamChannel->field_0x30 == 1 && streamChannel->channelRacer->field_0x478) {
                    streamChannel->field_0x30 = 0;
                    j = rand() % field_0x117c[streamChannel->field_0x0c];
                    streamChannel->streamSample = field_0xb88[streamChannel->field_0x0c][j];
                    streamChannel->field_0x14 = field_0xf48[streamChannel->field_0x0c][j];
                    // Retail compares the old state with 6 and then restarts
                    // the sample either way.
                    if (streamChannel->field_0x10 != 6)
                        streamChannel->field_0x18 = 0;
                    else
                        streamChannel->field_0x18 = 0;
                    streamChannel->field_0x10 = 6;
                    streamChannel->field_0x20 = 1;
                    StartStream();
                }
            }
        } else {
            streamChannel->airborneTime = 0;
            streamChannel->field_0x30 = 0;
            if (streamChannel->field_0x2c == 1 && !ownEngine->UnknownFunction4bca80() &&
                streamChannel->channelRacer == listenerRacer)
                PlayIfEnabled(ownEngine, 0, 0, 0, 0);
            if (!streamChannel->channelRacer->field_0x478)
                streamChannel->field_0x34 = 1;
            if (streamChannel->field_0x48) {
                if (streamChannel->channelRacer->field_0x478) {
                    if (streamChannel->field_0x10 == 6) {
                        j = rand() % field_0x114c[streamChannel->field_0x0c];
                        streamChannel->streamSample = field_0x9a8[streamChannel->field_0x0c][j];
                        streamChannel->field_0x14 = field_0xd68[streamChannel->field_0x0c][j];
                        streamChannel->field_0x18 = 0;
                        streamChannel->field_0x10 = 2;
                        streamChannel->field_0x20 = 3;
                        StartStream();
                    }
                    if (streamChannel->field_0x10 == 1 || streamChannel->field_0x10 == 5) {
                        streamChannel->field_0x44 = (int)(streamChannel->channelRacer->field_0x0b8 * 0.68182f);
                        if (streamChannel->field_0x44 < field_0x47c[streamChannel->field_0x0c]) {
                            j = rand() % field_0x1134[streamChannel->field_0x0c];
                            streamChannel->streamSample = field_0x8b8[streamChannel->field_0x0c][j];
                            streamChannel->field_0x14 = field_0xc78[streamChannel->field_0x0c][j];
                            streamChannel->field_0x18 = 0;
                            streamChannel->field_0x10 = 2;
                            streamChannel->field_0x20 = 3;
                        } else if (streamChannel->field_0x44 < field_0x488[streamChannel->field_0x0c]) {
                            j = rand() % field_0x1140[streamChannel->field_0x0c];
                            streamChannel->streamSample = field_0x930[streamChannel->field_0x0c][j];
                            streamChannel->field_0x14 = field_0xcf0[streamChannel->field_0x0c][j];
                            streamChannel->field_0x18 = 0;
                            streamChannel->field_0x10 = 2;
                            streamChannel->field_0x20 = 3;
                        } else if (streamChannel->field_0x44 < field_0x494[streamChannel->field_0x0c]) {
                            j = rand() % field_0x114c[streamChannel->field_0x0c];
                            streamChannel->streamSample = field_0x9a8[streamChannel->field_0x0c][j];
                            streamChannel->field_0x14 = field_0xd68[streamChannel->field_0x0c][j];
                            streamChannel->field_0x18 = 0;
                            streamChannel->field_0x10 = 2;
                            streamChannel->field_0x20 = 3;
                        } else {
                            j = rand() % field_0x1158[streamChannel->field_0x0c];
                            streamChannel->streamSample = field_0xa20[streamChannel->field_0x0c][j];
                            streamChannel->field_0x14 = field_0xde0[streamChannel->field_0x0c][j];
                            streamChannel->field_0x18 = 0;
                            streamChannel->field_0x10 = 3;
                            streamChannel->field_0x20 = 4;
                        }
                        StartStream();
                    }
                } else if (streamChannel->field_0x10 == 2 || streamChannel->field_0x10 == 3 ||
                           streamChannel->field_0x10 == 4) {
                    j = rand() % field_0x1164[streamChannel->field_0x0c];
                    streamChannel->streamSample = field_0xa98[streamChannel->field_0x0c][j];
                    streamChannel->field_0x14 = field_0xe58[streamChannel->field_0x0c][j];
                    streamChannel->field_0x18 = 0;
                    streamChannel->field_0x10 = 5;
                    streamChannel->field_0x20 = 1;
                    StartStream();
                }
            } else if (streamChannel->channelRacer->field_0x478 == true &&
                       (streamChannel->field_0x34 == 1 || streamChannel->field_0x28 == 1)) {
                streamChannel->field_0x34 = 0;
                j = rand() % 1;
                streamChannel->streamSample = field_0xb88[streamChannel->field_0x0c][j];
                streamChannel->field_0x14 = field_0xf48[streamChannel->field_0x0c][j];
                if (streamChannel->field_0x10 != 6)
                    streamChannel->field_0x18 = 0;
                else
                    streamChannel->field_0x18 = 0;
                streamChannel->field_0x10 = 6;
                streamChannel->field_0x20 = 1;
                StartStream();
            }
        }
        streamChannel->field_0x28 = streamChannel->field_0x48;
        streamChannel->field_0x24 = (unsigned char)streamChannel->channelRacer->field_0x478;
        streamChannel->field_0x2c = (unsigned char)streamChannel->channelRacer->field_0x108;
        RefillStream();
    }

    if (g_TrackGame->mode.field_0xa38 &&
        (listenerRacer->field_0x7b8 > field_0x1294 || listenerRacer->field_0x7a0 > field_0x1290))
        PlayIfEnabled(field_0x1204, 0, 0, 0, 0);

    for (i = 0; i < channelsInUse; i++) {
        currentRacer = engineChannels[i].channelRacer;
        if (!currentRacer->field_0x444) {
            field_0x120c[i] = 0;
            field_0x1238[i] = 0;
            field_0x1264[i] = 0;
        } else {
            if (!field_0x120c[i]) {
                if (currentRacer == listenerRacer && listenerRacer->field_0x460 == 12) {
                    field_0x11b4->UnknownFunction4bc940(1);
                    PlayIfEnabled(field_0x11b4, 0, 0, 0, 0);
                }
                if (g_TrackGame->mode.field_0x27f8.field_0x04 == 3) {
                    if (currentRacer == listenerRacer && currentRacer->field_0x784 == 1)
                        PlayIfEnabled(field_0x11f8, 0, 0, 0, 0);
                    if (currentRacer == listenerRacer && currentRacer->field_0x784 > 1 && field_0x12a4 > 30.0f) {
                        PlayIfEnabled(field_0x11fc, 0, 0, 0, 0);
                        field_0x12a4 = 0;
                    }
                }
            }
            if (currentRacer->field_0x484) {
                if (!field_0x1238[i])
                    field_0x1238[i] = UnknownFunction4e42e0(currentRacer);
                else if (!field_0x11b8[field_0x1238[i]]->UnknownFunction4bca80())
                    field_0x1238[i] = UnknownFunction4e42e0(currentRacer);
            }
            if (currentRacer->field_0x604->field_0xb0) {
                if (!field_0x1264[i])
                    field_0x1264[i] = UnknownFunction4e43a0(currentRacer);
                else if (!field_0x11d0[field_0x1264[i]]->UnknownFunction4bca80())
                    field_0x1264[i] = UnknownFunction4e43a0(currentRacer);
            }
            field_0x120c[i] = 1;
        }
        if (currentRacer == listenerRacer && currentRacer->field_0x784 < field_0x129c) {
            if (currentRacer->field_0x784 == 1) {
                if (field_0x11f4)
                    PlayIfEnabled(field_0x11f4, 0, 0, 0, 0);
            } else if (field_0x12a8 > 15.0f) {
                if (rand() > 0.5f) {
                    if (field_0x11ec)
                        PlayIfEnabled(field_0x11ec, 0, 0, 0, 0);
                } else if (field_0x11f0)
                    PlayIfEnabled(field_0x11f0, 0, 0, 0, 0);
                field_0x12a8 = 0;
            }
        }
    }
    field_0x1290 = listenerRacer->field_0x7a0;
    field_0x1294 = listenerRacer->field_0x7b8;
    field_0x129c = listenerRacer->field_0x784;
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
    owner = g_TrackGame->eventManager->FindRaceMode();
    if (!event->kind) {
        switch (event->control) {
        case 0x2f:
            if (!g_TrackGame->controlInterface->keyboard->UnknownVirtualSlot5(0x2f, 12, 0))
                break;
            if (!eaxEnabled) {
                environment.environment = 9;
                environment.volume = 0.361f;
                environment.decayTime = 7.0f;
                environment.damping = 0.332f;
                eaxEnabled = 1;
            } else {
                environment.environment = 17;
                environment.volume = 0.0f;
                environment.decayTime = 0.0f;
                environment.damping = 0.0f;
                eaxEnabled = 0;
            }
            if (owner)
                owner->ShowOnOffMessage(0x14c3, eaxEnabled);
            SoundSystem()->UnknownFunction4bed40(&environment);
            break;
        case 0x2e:
            if (!g_TrackGame->controlInterface->keyboard->UnknownVirtualSlot5(0x2e, 12, 0))
                break;
            if (g_TrackGame->mode.field_0x27f8.field_0x04 != 3)
                break;
            switch (musicVolume) {
            case 0:
                musicVolume = -500;
                g_TrackGame->LoadResourceString(0x14c0, text, sizeof(text));
                break;
            case -500:
                musicVolume = -10000;
                g_TrackGame->LoadResourceString(0x14c2, text, sizeof(text));
                break;
            case -10000:
                musicVolume = 0;
                g_TrackGame->LoadResourceString(0x14c1, text, sizeof(text));
                break;
            }
            {
                UnknownMessage message(text, 1.5f);
                if (owner)
                    owner->field_0x6c->UnknownFunction51b540(&message);
            }
            if (g_TrackGame->mode.field_0x23a4)
                crowdSounds->UnknownFunction401dd0(musicVolume);
            return 1;
        case 0x1f:
            if (!g_TrackGame->controlInterface->keyboard->UnknownVirtualSlot5(0x1f, 12, 0))
                break;
            g_TrackGame->mode.field_0xa28 = !g_TrackGame->mode.field_0xa28;
            if (g_TrackGame->mode.field_0xa28) {
                g_TrackGame->field_0x34->UnknownFunction468f10("SoundGroup");
                if (field_0x3c)
                    field_0x3c->UnknownVirtualSlot16(0);
                if (field_0x40)
                    field_0x40->UnknownVirtualSlot16(0);
                if (field_0x44)
                    field_0x44->UnknownVirtualSlot16(0);
                if (g_TrackGame->eventManager->FindRaceMode()->field_0x2c->field_0xc4)
                    g_TrackGame->eventManager->FindRaceMode()->field_0x2c->field_0xc4->UnknownVirtualSlot16(0);
                if (g_TrackGame->mode.field_0x23a4)
                    PlayIfEnabled(field_0x11e8, 1, 1, 1, 0);
                if (listenerRacer) {
                    field_0x1294 = listenerRacer->field_0x7b8;
                    field_0x1290 = listenerRacer->field_0x7a0;
                }
                if (g_TrackGame->eventManager->FindRaceMode()->field_0x9c)
                    g_TrackGame->eventManager->FindRaceMode()->field_0x9c->UnknownVirtualSlot5();
            } else {
                g_TrackGame->field_0x34->UnknownFunction468dd0("SoundGroup");
                if (field_0x3c)
                    field_0x3c->UnknownVirtualSlot16(1);
                if (field_0x40)
                    field_0x40->UnknownVirtualSlot16(1);
                if (field_0x44)
                    field_0x44->UnknownVirtualSlot16(1);
                if (g_TrackGame->eventManager->FindRaceMode()->field_0x2c->field_0xc4)
                    g_TrackGame->eventManager->FindRaceMode()->field_0x2c->field_0xc4->UnknownVirtualSlot16(1);
                if (g_TrackGame->eventManager->FindRaceMode()->field_0x9c)
                    g_TrackGame->eventManager->FindRaceMode()->field_0x9c->UnknownVirtualSlot4();
            }
            if (owner)
                owner->ShowOnOffMessage(0x1429, g_TrackGame->mode.field_0xa28);
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
    PlayIfEnabled(field_0x11b8[index], 0, 0, 0, 0);
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
    PlayIfEnabled(field_0x11d0[index], 0, 0, 0, 0);
    return index;
}

// 0x004e4460
float RaceSound::GroundDistance(Vector3* a, Vector3* b) {
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
void RaceSound::StartStream() {
    void* first;
    unsigned long write;
    void* second;
    unsigned long play;
    unsigned long secondBytes;
    unsigned long firstBytes;
    int bytes;

    if (!streamChannel->channelSound)
        return;
    streamChannel->channelSound->UnknownFunction4bcd40(&play, &write);
    streamChannel->channelSound->UnknownFunction4bd020(0, 0, &first, &firstBytes, &second, &secondBytes, 2);
    if (write < (unsigned long)(5512 * field_0x1298)) {
        bytes = 11025 * field_0x1298 - write;
        memcpy((char*)streamChannel->sampleBuffer + write, (char*)streamChannel->streamSample + streamChannel->field_0x18,
               bytes);
        memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
        streamChannel->field_0x18 += bytes;
        streamChannel->field_0x1c = 2;
    } else if (write < (unsigned long)(11025 * field_0x1298)) {
        bytes = 22050 * field_0x1298 - write;
        memcpy((char*)streamChannel->sampleBuffer + write, (char*)streamChannel->streamSample + streamChannel->field_0x18,
               bytes);
        memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
        streamChannel->field_0x18 += bytes;
        streamChannel->field_0x1c = 1;
    } else if (write < (unsigned long)(16537 * field_0x1298)) {
        bytes = 22050 * field_0x1298 - write;
        memcpy((char*)streamChannel->sampleBuffer + write, (char*)streamChannel->streamSample + streamChannel->field_0x18,
               bytes);
        memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
        streamChannel->field_0x18 += bytes;
        streamChannel->field_0x1c = 1;
    } else {
        bytes = 22050 * field_0x1298 - write;
        memcpy((char*)streamChannel->sampleBuffer + write, (char*)streamChannel->streamSample + streamChannel->field_0x18,
               bytes);
        streamChannel->field_0x18 += bytes;
        memcpy(streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->field_0x18,
               11025 * field_0x1298);
        memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
        streamChannel->field_0x18 += 11025 * field_0x1298;
        streamChannel->field_0x1c = 2;
    }
    streamChannel->channelSound->UnknownFunction4bd080(first, 22050 * field_0x1298, second, 0);
}

// The four per-TU vector constants (see src/krusty2/math/Math3D.h); their
// dynamic initializers follow slot 23 in the code.
static const Vector3 s_UnknownVector689bd0 = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector689be0 = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 s_UnknownVector689bf0 = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 s_UnknownVector689bc0 = Vector3(0.0f, 0.0f, 1.0f);

// 0x004e5500
void RaceSound::ReadSample(const char* name, void** data, int* size) {
    char message[0x184];
    UnknownTextureStream* stream = new (__FILE__, 1630) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    int bytes;
    void* buffer;

    if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, name, "rb", 0)) {
        sprintf(message, "%s not found in Audio.res.\n", name);
    } else {
        stream->UnknownFunction461340(stream->field_0x130, 0, 1);
        stream->UnknownFunction461640(&waveHeader, 1, sizeof(waveHeader));
        bytes = waveHeader.dataSize;
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
int RaceSound::LoadSound(Sound* sound, const char* name, int a, int b) {
    char message[0x184];
    UnknownTextureStream* stream;

    if (!sound)
        return 0;
    stream = new (__FILE__, 1678) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    if (!g_TrackGame->sceneObject->UnknownFunction4e9cd0(stream, name, "rb", 0)) {
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
void RaceSound::PlayIfEnabled(Sound* sound, int network, int stop, int loop, int unused) {
    if (g_TrackGame->mode.field_0xa28 && (g_TrackGame->mode.field_0x23a4 || !network) && sound) {
        if (stop)
            sound->UnknownFunction4bc940(1);
        sound->UnknownFunction4bc6b0(0, loop, 1);
    }
}

// 0x004e57d0
void RaceSound::UnknownFunction4e57d0(UnknownEventRacer* racer, float value) {
    if (listenerRacer == racer && value >= 10000.0f &&
        (!g_TrackGame->mode.field_0x27f8.field_0x04 || g_TrackGame->mode.field_0x27f8.field_0x04 == 4))
        PlayIfEnabled(field_0x11ac, 0, 0, 0, 0);
    if (g_TrackGame->mode.field_0x27f8.field_0x04 == 3 && field_0x12ac > 15.0f) {
        PlayIfEnabled(field_0x11ec, 0, 0, 0, 0);
        field_0x12ac = 0;
    }
}

// 0x004e5860
void RaceSound::UnknownFunction4e5860() {
    PlayIfEnabled(field_0x11b0, 0, 0, 0, 0);
}

// 0x004e5880
int RaceSound::CompareChannels(const void* a, const void* b) {
    int first = ((const UnknownRaceSoundChannel*)a)->field_0x4c;
    int second = ((const UnknownRaceSoundChannel*)b)->field_0x4c;

    if (first < second)
        return -1;
    return first != second;
}

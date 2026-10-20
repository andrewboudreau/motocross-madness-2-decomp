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

    lastListenerRacer = 0;
    lastListenerPosition = 99;
    ohClock = 0;
    applauseClock = 0;
    stuntApplauseClock = 0;
    streamChannel = 0;
    channelsInUse = 0;
    currentRacer = 0;
    lastListenerLap = 0;
    lastListenerGate = 0;
    waypointSound = 0;
    crowdLoop = 0;
    listenerRacer = 0;
    field_0x30 = 0;
    raceView = 0;
    listenerCamera = 0;
    engineGroup = 0;
    effectsGroup = 0;
    field_0x40 = 0;
    itSound = 0;
    field_0x58 = 0;
    field_0x5c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
    launchSound = 0;
    bonusSound = 0;
    assignChannelsPending = 1;
    musicVolume = 0;
    applause1 = 0;
    applause2 = 0;
    applause3 = 0;
    for (i = 0; i < 11; i++) {
        crashHandled[i] = 0;
        racerFallSound[i] = 0;
        racerWreckSound[i] = 0;
        engineChannels[i].sampleBuffer = 0;
        engineChannels[i].sampleBuffer = DebugMalloc(44100, __FILE__, 77);
        memset(engineChannels[i].sampleBuffer, 0, 44100);
    }
    for (i = 0; i < 6; i++)
        wreckSounds[i] = 0;
    for (i = 0; i < 6; i++)
        fallSounds[i] = 0;
    for (k = 0; k < 3; k++) {
        memset(idleSamples[k], 0, sizeof(idleSamples[k]));
        memset(lowTorqueSamples[k], 0, sizeof(lowTorqueSamples[k]));
        memset(midTorqueSamples[k], 0, sizeof(midTorqueSamples[k]));
        memset(highTorqueSamples[k], 0, sizeof(highTorqueSamples[k]));
        memset(powerBandSamples[k], 0, sizeof(powerBandSamples[k]));
        memset(decelSamples[k], 0, sizeof(decelSamples[k]));
        memset(sputterSamples[k], 0, sizeof(sputterSamples[k]));
        memset(revSamples[k], 0, sizeof(revSamples[k]));
    }
}

// 0x004e21b0
RaceSound::~RaceSound() {
    int i;
    int k;

    for (k = 0; k < 3; k++) {
        for (i = 0; i < 10; i++) {
            if (idleSamples[k][i])
                DebugFree(idleSamples[k][i], __FILE__, 128);
            idleSamples[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (lowTorqueSamples[k][i])
                DebugFree(lowTorqueSamples[k][i], __FILE__, 132);
            lowTorqueSamples[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (midTorqueSamples[k][i])
                DebugFree(midTorqueSamples[k][i], __FILE__, 136);
            midTorqueSamples[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (highTorqueSamples[k][i])
                DebugFree(highTorqueSamples[k][i], __FILE__, 140);
            highTorqueSamples[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (powerBandSamples[k][i])
                DebugFree(powerBandSamples[k][i], __FILE__, 144);
            powerBandSamples[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (decelSamples[k][i])
                DebugFree(decelSamples[k][i], __FILE__, 148);
            decelSamples[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (sputterSamples[k][i])
                DebugFree(sputterSamples[k][i], __FILE__, 152);
            sputterSamples[k][i] = 0;
        }
        for (i = 0; i < 10; i++) {
            if (revSamples[k][i])
                DebugFree(revSamples[k][i], __FILE__, 156);
            revSamples[k][i] = 0;
        }
    }
    for (i = 0; i < 11; i++) {
        if (engineChannels[i].sampleBuffer)
            DebugFree(engineChannels[i].sampleBuffer, __FILE__, 177);
        engineChannels[i].sampleBuffer = 0;
    }
    SoundSystem()->SetPrimaryVolume(-10000);
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

    stream = new (__FILE__, 203) UnknownTextureStream(g_ResourceManager);
    GameObject::SetRenderContext(owner);
    raceView = view;
    listenerCamera = camera;
    engineGroup = new (__FILE__, 213) SoundGroup(1);
    AppendChild(engineGroup, -1);
    effectsGroup = new (__FILE__, 214) SoundGroup(1);
    AppendChild(effectsGroup, -1);
    crowdSounds = new (__FILE__, 215) SoundGroup(1);
    AppendChild(crowdSounds, -1);
    field_0x48 = new (__FILE__, 216) SoundGroup(1);
    AppendChild(field_0x48, -1);
    if (SoundSystem())
        SoundSystem()->FormatSoundCapsReport();

    enginePresent[0] = 0;
    enginePresent[1] = 0;
    enginePresent[2] = 0;
    iterator = 0;
    while ((racer = raceView->NextBike(&iterator)) != 0) {
        if (racer->engineSize < 250) {
            enginePresent[0] = 1;
            if (racer == raceView->localRacer)
                listenerEngine = 0;
        } else if (!racer->fourStroke) {
            enginePresent[1] = 1;
            if (racer == raceView->localRacer)
                listenerEngine = 1;
        } else {
            enginePresent[2] = 1;
            if (racer == raceView->localRacer)
                listenerEngine = 2;
        }
    }
    engineCount = 0;
    if (enginePresent[0])
        engineCount++;
    if (enginePresent[1])
        engineCount++;
    if (enginePresent[2])
        engineCount++;

    sampleDivisor[0] = 1;
    sampleDivisor[1] = 1;
    sampleDivisor[2] = 1;
    if (!g_TrackGame->mode.use16BitSound) {
        if (racers <= 2) {
            if (engineCount == 2) {
                sampleDivisor[0] = 2;
                sampleDivisor[1] = 2;
                sampleDivisor[2] = 2;
            }
            if (engineCount == 3) {
                sampleDivisor[0] = 4;
                sampleDivisor[1] = 4;
                sampleDivisor[2] = 4;
                sampleDivisor[listenerEngine] = 2;
            }
        }
    } else if (racers <= 2) {
        if (engineCount == 1) {
            sampleDivisor[0] = 2;
            sampleDivisor[1] = 2;
            sampleDivisor[2] = 2;
        }
        if (engineCount == 2) {
            sampleDivisor[0] = 4;
            sampleDivisor[1] = 4;
            sampleDivisor[2] = 4;
        }
        if (engineCount == 3) {
            sampleDivisor[0] = 8;
            sampleDivisor[1] = 8;
            sampleDivisor[2] = 8;
            sampleDivisor[listenerEngine] = 4;
        }
    } else if (racers <= 6) {
        if (engineCount == 2) {
            sampleDivisor[0] = 2;
            sampleDivisor[1] = 2;
            sampleDivisor[2] = 2;
            sampleDivisor[listenerEngine] = 1;
        }
        if (engineCount == 3) {
            sampleDivisor[0] = 4;
            sampleDivisor[1] = 4;
            sampleDivisor[2] = 4;
            sampleDivisor[listenerEngine] = 2;
        }
    }

    for (i = 0; i < 4; i++) {
        engineVoices[i] = new (__FILE__, 310) Sound(engineGroup, 1);
        if (g_TrackGame->mode.use16BitSound)
            LoadSound(engineVoices[i], "silence_16.wav", 41, 3);
        else
            LoadSound(engineVoices[i], "silence_08.wav", 41, 3);
        engineVoiceInUse[i] = 0;
    }
    landHardSound = new (__FILE__, 321) Sound(engineGroup, 1);
    LoadSound(landHardSound, "LandHard01.wav", 41, 3);
    if (g_TrackGame->mode.raceSettings.eventType != 3) {
        launchSound = new (__FILE__, 326) Sound(effectsGroup, 1);
        LoadSound(launchSound, "launch.wav", 1, 3);
    }

    if (g_TrackGame->mode.use16BitSound) {
        bytesPerSample = 2;
        strcpy(file, "audio_16.ini");
    } else {
        bytesPerSample = 1;
        strcpy(file, "audio_08.ini");
    }
    for (k = 0; k < 3; k++) {
        if (!enginePresent[k])
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
        stream->Open(file, "r", 0);
        parameters.Attach((UnknownParameterStream*)stream, 0, 1);
        parameters.SelectSection(section);

        parameters.ReadInt("TotalIdle", &idleCount[k]);
        parameters.ReadInt("TotalIdle", &count);
        if (count > 1) {
            count /= sampleDivisor[k];
            if (count < 1)
                count = 1;
        }
        idleCount[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < idleCount[k]; i++) {
                sprintf(key, "idle_%d", i + 1);
                parameters.ReadString(key, sample, -1);
                ReadSample(sample, &idleSamples[k][i], &idleSampleBytes[k][i]);
            }
        }

        parameters.ReadInt("TotalLowTorque", &count);
        if (count > 1) {
            count /= sampleDivisor[k];
            if (count < 1)
                count = 1;
        }
        lowTorqueCount[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < lowTorqueCount[k]; i++) {
                sprintf(key, "LowTorque_%d", i + 1);
                parameters.ReadString(key, sample, -1);
                ReadSample(sample, &lowTorqueSamples[k][i], &lowTorqueSampleBytes[k][i]);
            }
        }

        parameters.ReadInt("TotalMidTorque", &count);
        if (count > 1) {
            count /= sampleDivisor[k];
            if (count < 1)
                count = 1;
        }
        midTorqueCount[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < midTorqueCount[k]; i++) {
                sprintf(key, "MidTorque_%d", i + 1);
                parameters.ReadString(key, sample, -1);
                ReadSample(sample, &midTorqueSamples[k][i], &midTorqueSampleBytes[k][i]);
            }
        }

        parameters.ReadInt("TotalHighTorque", &count);
        if (count > 1) {
            count /= sampleDivisor[k];
            if (count < 1)
                count = 1;
        }
        highTorqueCount[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < highTorqueCount[k]; i++) {
                sprintf(key, "HighTorque_%d", i + 1);
                parameters.ReadString(key, sample, -1);
                ReadSample(sample, &highTorqueSamples[k][i], &highTorqueSampleBytes[k][i]);
            }
        }

        parameters.ReadInt("TotalPowerBand", &count);
        if (count > 1) {
            count /= sampleDivisor[k];
            if (count < 1)
                count = 1;
        }
        powerBandCount[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < powerBandCount[k]; i++) {
                sprintf(key, "PowerBand_%d", i + 1);
                parameters.ReadString(key, sample, -1);
                ReadSample(sample, &powerBandSamples[k][i], &powerBandSampleBytes[k][i]);
            }
        }

        parameters.ReadInt("TotalDecel", &count);
        if (count > 1) {
            count /= sampleDivisor[k];
            if (count < 1)
                count = 1;
        }
        decelCount[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < decelCount[k]; i++) {
                sprintf(key, "Decel_%d", i + 1);
                parameters.ReadString(key, sample, -1);
                ReadSample(sample, &decelSamples[k][i], &decelSampleBytes[k][i]);
            }
        }

        parameters.ReadInt("TotalSputter", &count);
        if (count > 1) {
            count /= sampleDivisor[k];
            if (count < 1)
                count = 1;
        }
        sputterCount[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < sputterCount[k]; i++) {
                sprintf(key, "Sputter_%d", i + 1);
                parameters.ReadString(key, sample, -1);
                ReadSample(sample, &sputterSamples[k][i], &sputterSampleBytes[k][i]);
            }
        }

        parameters.ReadInt("TotalRev", &count);
        if (count > 1) {
            count /= sampleDivisor[k];
            if (count < 1)
                count = 1;
        }
        revCount[k] = count;
        if (count >= 1 && count <= 10) {
            for (i = 0; i < revCount[k]; i++) {
                sprintf(key, "Rev_%d", i + 1);
                parameters.ReadString(key, sample, -1);
                ReadSample(sample, &revSamples[k][i], &revSampleBytes[k][i]);
            }
        }

        parameters.GetInt("LowTorqueMaxSpeed", 20, &lowTorqueMaxSpeed[k]);
        parameters.GetInt("MidTorqueMaxSpeed", 55, &midTorqueMaxSpeed[k]);
        parameters.GetInt("HighTorqueMaxSpeed", 100, &highTorqueMaxSpeed[k]);
    }
    if (stream)
        delete stream;

    if (g_TrackGame->mode.networkGame) {
        crowdLoop = new (__FILE__, 545) Sound(crowdSounds, 1);
        LoadSound(crowdLoop, "CrowdLoop.wav", 1, 3);
        applause1 = new (__FILE__, 548) Sound(crowdSounds, 1);
        LoadSound(applause1, "applause01.wav", 1, 3);
        applause2 = new (__FILE__, 550) Sound(crowdSounds, 1);
        LoadSound(applause2, "applause02.wav", 1, 3);
        applause3 = new (__FILE__, 552) Sound(crowdSounds, 1);
        LoadSound(applause3, "applause03.wav", 1, 3);
        booSound = new (__FILE__, 554) Sound(crowdSounds, 1);
        LoadSound(booSound, "boo_01.wav", 1, 3);
        ohSound = new (__FILE__, 556) Sound(crowdSounds, 1);
        LoadSound(ohSound, "oh_01.wav", 1, 3);
    }
    bonusSound = new (__FILE__, 566) Sound(effectsGroup, 1);
    LoadSound(bonusSound, "bonus.wav", 1, 3);
    itSound = new (__FILE__, 570) Sound(effectsGroup, 1);
    LoadSound(itSound, "It.wav", 1, 3);
    for (i = 0; i < 6; i++) {
        wreckSounds[i] = new (__FILE__, 577) Sound(effectsGroup, 1);
        sprintf(sample, "wreck%02d.wav", i + 1);
        LoadSound(wreckSounds[i], sample, 41, 3);
    }
    for (i = 0; i < 6; i++) {
        fallSounds[i] = new (__FILE__, 584) Sound(effectsGroup, 1);
        sprintf(sample, "fall%02d.wav", i + 1);
        LoadSound(fallSounds[i], sample, 41, 3);
    }
    waypointSound = new (__FILE__, 589) Sound(effectsGroup, 1);
    LoadSound(waypointSound, "Waypoint.wav", 1, 3);
    if (SoundSystem())
        SoundSystem()->FormatSoundCapsReport();
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
        engineChannels[i].voiceIndex = -1;
        engineChannels[i].engine = 0;
        engineChannels[i].sampleKind = 0;
        engineChannels[i].sampleBytes = 0;
        engineChannels[i].sampleOffset = 0;
        engineChannels[i].streamSample = 0;
        engineChannels[i].nextSampleKind = 0;
        engineChannels[i].fillHalf = 1;
        engineChannels[i].wasThrottleOn = 0;
        engineChannels[i].field_0x28 = 0;
        engineChannels[i].wasAirborne = 0;
        engineChannels[i].airRevArmed = 0;
        engineChannels[i].groundRevArmed = 0;
        engineChannels[i].airborneTime = 0;
        engineChannels[i].speed = 0;
        engineChannels[i].field_0x48 = 0;
        engineChannels[i].cameraDistance = 1000000;
    }

    iterator = 0;
    count = 0;
    while ((racer = raceView->NextBike(&iterator)) != 0) {
        engineChannels[count].channelRacer = racer;
        if (racer->engineSize < 250)
            engineChannels[count].engine = 0;
        else
            engineChannels[count].engine = (racer->fourStroke != 0) + 1;
        kind = engineChannels[count].engine;
        engineChannels[count].streamSample = idleSamples[kind][0];
        engineChannels[count].sampleBytes = idleSampleBytes[kind][0];
        engineChannels[count].sampleOffset = 0;
        engineChannels[count].sampleKind = 1;
        engineChannels[count].nextSampleKind = 0;
        engineChannels[count].fillHalf = 1;
        count++;
    }
    channelsInUse = count;

    SoundSystem()->SetListenerDistanceFactor(0.3048f);
    SoundSystem()->SetListenerDopplerFactor(1.0f);
    SoundSystem()->SetListenerRolloffFactor(1.0f);
    landHardSound->Set3DDistanceRange(10.0f, 500.0f, 0);

    volume = (g_TrackGame->mode.soundVolume * 0.01f) * 2500.0f - 2500.0f;
    if (volume > 0.0f)
        volume = 0.0f;
    else if (volume == -2500.0f)
        volume = -10000.0f;
    SoundSystem()->SetPrimaryVolume((long)volume);

    for (i = 0; i < 4; i++) {
        engineVoices[i]->Set3DDistanceRange(50.0f, 500.0f, 0);
        if (i < channelsInUse)
            PlayIfEnabled(engineVoices[i], 0, 0, 1, 0);
        if (i == channelsInUse)
            break;
    }
    for (i = 0; i < 6; i++)
        wreckSounds[i]->Set3DDistanceRange(50.0f, 500.0f, 0);
    for (i = 0; i < 6; i++)
        fallSounds[i]->Set3DDistanceRange(50.0f, 500.0f, 0);
    if (g_TrackGame->mode.networkGame)
        PlayIfEnabled(crowdLoop, 1, 1, 1, 0);

    if (g_TrackGame->mode.GetSelectedSeries() == 3 ||
        g_TrackGame->mode.GetSelectedSeries() == 4) {
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
    SoundSystem()->SetEAXListenerParameters(&environment);
    SoundSystem()->CommitListenerSettings();
    if (!g_TrackGame->mode.soundOn)
        g_TrackGame->childRoot->DisableByClassName("SoundGroup");
}

// 0x004e3730: moves the listener and every racer's engine sound to their
// bikes while sound is on and the race view is not paused.
int RaceSound::Tick(float frameTime) {
    Vector3 position;
    int i;

    GameObject::Tick(frameTime);
    if (assignChannelsPending) {
        AssignChannels();
        assignChannelsPending = 0;
    }
    if (SoundSystem()->audioStarted && g_TrackGame->mode.soundOn && !raceView->replayPaused) {
        ohClock += frameTime;
        applauseClock += frameTime;
        stuntApplauseClock += frameTime;
        listenerRacer = listenerCamera->followedRacer;
        if (listenerRacer != lastListenerRacer) {
            lastListenerRacer = listenerRacer;
            lastListenerLap = listenerRacer->lapCount;
            lastListenerGate = listenerRacer->lapGate;
            lastListenerPosition = listenerRacer->finishPosition;
        }
        UpdateRacerSounds(frameTime);
        SoundSystem()->SetListenerPosition(listenerCamera->listenerPosition);
        SoundSystem()->SetListenerOrientation(listenerCamera->listenerForward, listenerCamera->listenerUp);
        SoundSystem()->SetListenerVelocity(listenerCamera->followedBike->velocity);
        for (i = 0; i < channelsInUse; i++) {
            if (engineChannels[i].channelSound) {
                engineChannels[i].channelRacer->vehicleModel->GetPosition(&position);
                engineChannels[i].channelSound->Set3DPosition(position, 0);
                engineChannels[i].channelSound->Set3DVelocity(engineChannels[i].channelRacer->velocity, 0);
            }
        }
        listenerRacer->vehicleModel->GetPosition(&position);
        landHardSound->Set3DPosition(position, 0);
        landHardSound->Set3DVelocity(listenerRacer->velocity, 0);
        SoundSystem()->CommitListenerSettings();
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
        if (!engineChannels[i].channelRacer->outOfRace) {
            engineChannels[i].cameraDistance =
                (int)GroundDistance(&listenerCamera->listenerPosition, &engineChannels[i].channelRacer->position);
        } else {
            engineChannels[i].cameraDistance = 1000000;
            engineChannels[i].channelSound = 0;
        }
    }
    qsort(engineChannels, channelsInUse, sizeof(UnknownRaceSoundChannel), CompareChannels);

    for (i = channelsInUse - 1; i > -1; i--) {
        if (engineChannels[i].channelSound && i >= 4) {
            engineVoiceInUse[engineChannels[i].voiceIndex] = 0;
            engineChannels[i].channelSound->Stop(1);
            engineChannels[i].channelSound = 0;
            engineChannels[i].voiceIndex = -1;
        }
        if (!engineChannels[i].channelSound && i < 4) {
            for (j = 0; j < 4; j++) {
                if (!engineVoiceInUse[j]) {
                    engineChannels[i].channelSound = engineVoices[j];
                    engineChannels[i].voiceIndex = j;
                    engineVoiceInUse[j] = 1;
                    PlayIfEnabled(engineChannels[i].channelSound, 0, 0, 1, 0);
                    break;
                }
            }
        }
    }

    for (i = 0; i < channelsInUse; i++) {
        streamChannel = &engineChannels[i];
        if (streamChannel->channelRacer->driveDisengaged)
            streamChannel->field_0x48 = 0;
        else
            streamChannel->field_0x48 = 1;
        if (streamChannel->channelRacer->airborne) {
            streamChannel->airborneTime += frameTime;
            if (streamChannel->airborneTime >= 0.2f) {
                if (!streamChannel->channelRacer->throttleOn)
                    streamChannel->airRevArmed = 1;
                if (streamChannel->sampleKind != 5 && streamChannel->sampleKind != 6 && streamChannel->sampleKind != 1) {
                    j = rand() % decelCount[streamChannel->engine];
                    streamChannel->streamSample = decelSamples[streamChannel->engine][j];
                    streamChannel->sampleBytes = decelSampleBytes[streamChannel->engine][j];
                    streamChannel->sampleOffset = 0;
                    streamChannel->sampleKind = 5;
                    streamChannel->nextSampleKind = 1;
                    StartStream();
                }
                if (streamChannel->airRevArmed == 1 && streamChannel->channelRacer->throttleOn) {
                    streamChannel->airRevArmed = 0;
                    j = rand() % revCount[streamChannel->engine];
                    streamChannel->streamSample = revSamples[streamChannel->engine][j];
                    streamChannel->sampleBytes = revSampleBytes[streamChannel->engine][j];
                    // Retail compares the old state with 6 and then restarts
                    // the sample either way.
                    if (streamChannel->sampleKind != 6)
                        streamChannel->sampleOffset = 0;
                    else
                        streamChannel->sampleOffset = 0;
                    streamChannel->sampleKind = 6;
                    streamChannel->nextSampleKind = 1;
                    StartStream();
                }
            }
        } else {
            streamChannel->airborneTime = 0;
            streamChannel->airRevArmed = 0;
            if (streamChannel->wasAirborne == 1 && !landHardSound->IsPlaying() &&
                streamChannel->channelRacer == listenerRacer)
                PlayIfEnabled(landHardSound, 0, 0, 0, 0);
            if (!streamChannel->channelRacer->throttleOn)
                streamChannel->groundRevArmed = 1;
            if (streamChannel->field_0x48) {
                if (streamChannel->channelRacer->throttleOn) {
                    if (streamChannel->sampleKind == 6) {
                        j = rand() % highTorqueCount[streamChannel->engine];
                        streamChannel->streamSample = highTorqueSamples[streamChannel->engine][j];
                        streamChannel->sampleBytes = highTorqueSampleBytes[streamChannel->engine][j];
                        streamChannel->sampleOffset = 0;
                        streamChannel->sampleKind = 2;
                        streamChannel->nextSampleKind = 3;
                        StartStream();
                    }
                    if (streamChannel->sampleKind == 1 || streamChannel->sampleKind == 5) {
                        streamChannel->speed = (int)(streamChannel->channelRacer->previousSpeed * 0.68182f);
                        if (streamChannel->speed < lowTorqueMaxSpeed[streamChannel->engine]) {
                            j = rand() % lowTorqueCount[streamChannel->engine];
                            streamChannel->streamSample = lowTorqueSamples[streamChannel->engine][j];
                            streamChannel->sampleBytes = lowTorqueSampleBytes[streamChannel->engine][j];
                            streamChannel->sampleOffset = 0;
                            streamChannel->sampleKind = 2;
                            streamChannel->nextSampleKind = 3;
                        } else if (streamChannel->speed < midTorqueMaxSpeed[streamChannel->engine]) {
                            j = rand() % midTorqueCount[streamChannel->engine];
                            streamChannel->streamSample = midTorqueSamples[streamChannel->engine][j];
                            streamChannel->sampleBytes = midTorqueSampleBytes[streamChannel->engine][j];
                            streamChannel->sampleOffset = 0;
                            streamChannel->sampleKind = 2;
                            streamChannel->nextSampleKind = 3;
                        } else if (streamChannel->speed < highTorqueMaxSpeed[streamChannel->engine]) {
                            j = rand() % highTorqueCount[streamChannel->engine];
                            streamChannel->streamSample = highTorqueSamples[streamChannel->engine][j];
                            streamChannel->sampleBytes = highTorqueSampleBytes[streamChannel->engine][j];
                            streamChannel->sampleOffset = 0;
                            streamChannel->sampleKind = 2;
                            streamChannel->nextSampleKind = 3;
                        } else {
                            j = rand() % powerBandCount[streamChannel->engine];
                            streamChannel->streamSample = powerBandSamples[streamChannel->engine][j];
                            streamChannel->sampleBytes = powerBandSampleBytes[streamChannel->engine][j];
                            streamChannel->sampleOffset = 0;
                            streamChannel->sampleKind = 3;
                            streamChannel->nextSampleKind = 4;
                        }
                        StartStream();
                    }
                } else if (streamChannel->sampleKind == 2 || streamChannel->sampleKind == 3 ||
                           streamChannel->sampleKind == 4) {
                    j = rand() % decelCount[streamChannel->engine];
                    streamChannel->streamSample = decelSamples[streamChannel->engine][j];
                    streamChannel->sampleBytes = decelSampleBytes[streamChannel->engine][j];
                    streamChannel->sampleOffset = 0;
                    streamChannel->sampleKind = 5;
                    streamChannel->nextSampleKind = 1;
                    StartStream();
                }
            } else if (streamChannel->channelRacer->throttleOn == true &&
                       (streamChannel->groundRevArmed == 1 || streamChannel->field_0x28 == 1)) {
                streamChannel->groundRevArmed = 0;
                j = rand() % 1;
                streamChannel->streamSample = revSamples[streamChannel->engine][j];
                streamChannel->sampleBytes = revSampleBytes[streamChannel->engine][j];
                if (streamChannel->sampleKind != 6)
                    streamChannel->sampleOffset = 0;
                else
                    streamChannel->sampleOffset = 0;
                streamChannel->sampleKind = 6;
                streamChannel->nextSampleKind = 1;
                StartStream();
            }
        }
        streamChannel->field_0x28 = streamChannel->field_0x48;
        streamChannel->wasThrottleOn = (unsigned char)streamChannel->channelRacer->throttleOn;
        streamChannel->wasAirborne = (unsigned char)streamChannel->channelRacer->airborne;
        RefillStream();
    }

    if (g_TrackGame->mode.positionSounds &&
        (listenerRacer->lapGate > lastListenerGate || listenerRacer->lapCount > lastListenerLap))
        PlayIfEnabled(waypointSound, 0, 0, 0, 0);

    for (i = 0; i < channelsInUse; i++) {
        currentRacer = engineChannels[i].channelRacer;
        if (!currentRacer->crashState) {
            crashHandled[i] = 0;
            racerWreckSound[i] = 0;
            racerFallSound[i] = 0;
        } else {
            if (!crashHandled[i]) {
                if (currentRacer == listenerRacer && listenerRacer->crashReason == 12) {
                    launchSound->Stop(1);
                    PlayIfEnabled(launchSound, 0, 0, 0, 0);
                }
                if (g_TrackGame->mode.raceSettings.eventType == 3) {
                    if (currentRacer == listenerRacer && currentRacer->finishPosition == 1)
                        PlayIfEnabled(booSound, 0, 0, 0, 0);
                    if (currentRacer == listenerRacer && currentRacer->finishPosition > 1 && ohClock > 30.0f) {
                        PlayIfEnabled(ohSound, 0, 0, 0, 0);
                        ohClock = 0;
                    }
                }
            }
            if (currentRacer->justLanded) {
                if (!racerWreckSound[i])
                    racerWreckSound[i] = PlayWreckSound(currentRacer);
                else if (!wreckSounds[racerWreckSound[i]]->IsPlaying())
                    racerWreckSound[i] = PlayWreckSound(currentRacer);
            }
            if (currentRacer->wrecker->contactThisFrame) {
                if (!racerFallSound[i])
                    racerFallSound[i] = PlayFallSound(currentRacer);
                else if (!fallSounds[racerFallSound[i]]->IsPlaying())
                    racerFallSound[i] = PlayFallSound(currentRacer);
            }
            crashHandled[i] = 1;
        }
        if (currentRacer == listenerRacer && currentRacer->finishPosition < lastListenerPosition) {
            if (currentRacer->finishPosition == 1) {
                if (applause3)
                    PlayIfEnabled(applause3, 0, 0, 0, 0);
            } else if (applauseClock > 15.0f) {
                if (rand() > 0.5f) {
                    if (applause1)
                        PlayIfEnabled(applause1, 0, 0, 0, 0);
                } else if (applause2)
                    PlayIfEnabled(applause2, 0, 0, 0, 0);
                applauseClock = 0;
            }
        }
    }
    lastListenerLap = listenerRacer->lapCount;
    lastListenerGate = listenerRacer->lapGate;
    lastListenerPosition = listenerRacer->finishPosition;
    return 1;
}

// 0x004e44e0: Ctrl+V toggles the EAX environment, Ctrl+C cycles the music
// volume (race mode 3 only) and Ctrl+S turns all sound on or off.
int RaceSound::OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry) {
    UnknownEaxListenerParameters environment;
    char text[0x80];
    TrackGameViewOwner* owner;

    if (GameObject::OnControlPress(event, entry))
        return 1;
    owner = g_TrackGame->eventManager->FindRaceMode();
    if (!event->kind) {
        switch (event->control) {
        case 0x2f:
            if (!g_TrackGame->controlInterface->keyboard->IsButtonDown(0x2f, 12, 0))
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
            SoundSystem()->SetEAXListenerParameters(&environment);
            break;
        case 0x2e:
            if (!g_TrackGame->controlInterface->keyboard->IsButtonDown(0x2e, 12, 0))
                break;
            if (g_TrackGame->mode.raceSettings.eventType != 3)
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
                    owner->textQueue->ReplaceHeadMessage(&message);
            }
            if (g_TrackGame->mode.networkGame)
                crowdSounds->SetMemberVolume(musicVolume);
            return 1;
        case 0x1f:
            if (!g_TrackGame->controlInterface->keyboard->IsButtonDown(0x1f, 12, 0))
                break;
            g_TrackGame->mode.soundOn = !g_TrackGame->mode.soundOn;
            if (g_TrackGame->mode.soundOn) {
                g_TrackGame->childRoot->EnableByClassName("SoundGroup");
                if (engineGroup)
                    engineGroup->SetPaused(0);
                if (field_0x40)
                    field_0x40->SetPaused(0);
                if (effectsGroup)
                    effectsGroup->SetPaused(0);
                if (g_TrackGame->eventManager->FindRaceMode()->eventScene->soundGroup)
                    g_TrackGame->eventManager->FindRaceMode()->eventScene->soundGroup->SetPaused(0);
                if (g_TrackGame->mode.networkGame)
                    PlayIfEnabled(crowdLoop, 1, 1, 1, 0);
                if (listenerRacer) {
                    lastListenerGate = listenerRacer->lapGate;
                    lastListenerLap = listenerRacer->lapCount;
                }
                if (g_TrackGame->eventManager->FindRaceMode()->auralScape)
                    g_TrackGame->eventManager->FindRaceMode()->auralScape->Enable();
            } else {
                g_TrackGame->childRoot->DisableByClassName("SoundGroup");
                if (engineGroup)
                    engineGroup->SetPaused(1);
                if (field_0x40)
                    field_0x40->SetPaused(1);
                if (effectsGroup)
                    effectsGroup->SetPaused(1);
                if (g_TrackGame->eventManager->FindRaceMode()->eventScene->soundGroup)
                    g_TrackGame->eventManager->FindRaceMode()->eventScene->soundGroup->SetPaused(1);
                if (g_TrackGame->eventManager->FindRaceMode()->auralScape)
                    g_TrackGame->eventManager->FindRaceMode()->auralScape->Disable();
            }
            if (owner)
                owner->ShowOnOffMessage(0x1429, g_TrackGame->mode.soundOn);
            return 1;
        }
    }
    return 0;
}

// 0x004e42e0: plays one of the six wreck sounds that is not playing (at most
// ten random picks) at the racer's position; returns its index, 0 when
// none was free.
int RaceSound::PlayWreckSound(UnknownEventRacer* racer) {
    Vector3 position;
    int tries = 0;
    int index = rand() % 6;

    while (wreckSounds[index]->IsPlaying()) {
        if (++tries >= 10)
            return 0;
        index = rand() % 6;
    }
    racer->vehicleModel->GetPosition(&position);
    wreckSounds[index]->Set3DPosition(position, 0);
    PlayIfEnabled(wreckSounds[index], 0, 0, 0, 0);
    return index;
}

// 0x004e43a0: plays one of the six fall sounds that is not playing (at most
// ten random picks) at the racer's position; returns its index, 0 when
// none was free.
int RaceSound::PlayFallSound(UnknownEventRacer* racer) {
    Vector3 position;
    int tries = 0;
    int index = rand() % 6;

    while (fallSounds[index]->IsPlaying()) {
        if (++tries >= 10)
            return 0;
        index = rand() % 6;
    }
    racer->vehicleModel->GetPosition(&position);
    fallSounds[index]->Set3DPosition(position, 0);
    PlayIfEnabled(fallSounds[index], 0, 0, 0, 0);
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
        return FastSqrt(dz * dz + dx * dx);
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
    streamChannel->channelSound->GetBufferPosition(&play, &write);
    streamChannel->channelSound->LockBuffer(0, 0, &first, &firstBytes, &second, &secondBytes, 2);
    if (write < (unsigned long)(5512 * bytesPerSample)) {
        bytes = 11025 * bytesPerSample - write;
        memcpy((char*)streamChannel->sampleBuffer + write, (char*)streamChannel->streamSample + streamChannel->sampleOffset,
               bytes);
        memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
        streamChannel->sampleOffset += bytes;
        streamChannel->fillHalf = 2;
    } else if (write < (unsigned long)(11025 * bytesPerSample)) {
        bytes = 22050 * bytesPerSample - write;
        memcpy((char*)streamChannel->sampleBuffer + write, (char*)streamChannel->streamSample + streamChannel->sampleOffset,
               bytes);
        memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
        streamChannel->sampleOffset += bytes;
        streamChannel->fillHalf = 1;
    } else if (write < (unsigned long)(16537 * bytesPerSample)) {
        bytes = 22050 * bytesPerSample - write;
        memcpy((char*)streamChannel->sampleBuffer + write, (char*)streamChannel->streamSample + streamChannel->sampleOffset,
               bytes);
        memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
        streamChannel->sampleOffset += bytes;
        streamChannel->fillHalf = 1;
    } else {
        bytes = 22050 * bytesPerSample - write;
        memcpy((char*)streamChannel->sampleBuffer + write, (char*)streamChannel->streamSample + streamChannel->sampleOffset,
               bytes);
        streamChannel->sampleOffset += bytes;
        memcpy(streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->sampleOffset,
               11025 * bytesPerSample);
        memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
        streamChannel->sampleOffset += 11025 * bytesPerSample;
        streamChannel->fillHalf = 2;
    }
    streamChannel->channelSound->UnlockBuffer(first, 22050 * bytesPerSample, second, 0);
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
    UnknownTextureStream* stream = new (__FILE__, 1630) UnknownTextureStream(g_ResourceManager);
    int bytes;
    void* buffer;

    if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, name, "rb", 0)) {
        sprintf(message, "%s not found in Audio.res.\n", name);
    } else {
        stream->Seek(stream->sliceOffset, 0, 1);
        stream->Read(&waveHeader, 1, sizeof(waveHeader));
        bytes = waveHeader.dataSize;
        buffer = DebugMalloc(bytes, __FILE__, 1652);
        if (!buffer) {
            *data = 0;
            *size = 0;
        } else {
            stream->Read(buffer, 1, bytes);
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
    stream = new (__FILE__, 1678) UnknownTextureStream(g_ResourceManager);
    if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, name, "rb", 0)) {
        sprintf(message, "%s not found in Audio.res.\n", name);
        if (stream)
            delete stream;
        return 0;
    }
    if (!sound->Load(name, stream, a, b, 0, -1)) {
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
    if (g_TrackGame->mode.soundOn && (g_TrackGame->mode.networkGame || !network) && sound) {
        if (stop)
            sound->Stop(1);
        sound->PlayWithOptions(0, loop, 1);
    }
}

// 0x004e57d0: bonus.wav for the listener's scores of 10000 and more
// (modes 0 and 4); applause01 in mode 3 at most every 15 seconds.
void RaceSound::PlayScoreSounds(UnknownEventRacer* racer, float value) {
    if (listenerRacer == racer && value >= 10000.0f &&
        (!g_TrackGame->mode.raceSettings.eventType || g_TrackGame->mode.raceSettings.eventType == 4))
        PlayIfEnabled(bonusSound, 0, 0, 0, 0);
    if (g_TrackGame->mode.raceSettings.eventType == 3 && stuntApplauseClock > 15.0f) {
        PlayIfEnabled(applause1, 0, 0, 0, 0);
        stuntApplauseClock = 0;
    }
}

// 0x004e5860
void RaceSound::PlayItSound() {
    PlayIfEnabled(itSound, 0, 0, 0, 0);
}

// 0x004e5880
int RaceSound::CompareChannels(const void* a, const void* b) {
    int first = ((const UnknownRaceSoundChannel*)a)->cameraDistance;
    int second = ((const UnknownRaceSoundChannel*)b)->cameraDistance;

    if (first < second)
        return -1;
    return first != second;
}

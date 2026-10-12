#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"
#include "PCAudio.h"

// Reconstruction of part of D:\aardvark\VC\krusty2\racesnd.cpp (literal
// __FILE__ at 0x005726b8). Evidence and the per-function table are in
// docs/RACESOUND.md.
//
// Confirmed (RTTI): RaceSound : GameObject : BaseObject, single non-virtual
// inheritance; vtable 0x00557710 (27 slots, COL 0x0055eb78). RaceSound
// overrides slot 0 (deleting destructor 0x004e2190), slot 10 (0x004e3730)
// and slot 23 (0x004e44e0). bikerace.cpp allocates 0x12b0 bytes for it
// (0x004195e4) and constructs it with 1.

struct UnknownEventRacer;
struct UnknownKrustyBikeView;

// The camera at RaceSound+0x38 as this file reads it: the listener's
// position and orientation, the followed bike (velocity at +0x64) and the
// racer it follows. These are KrustyBikeCamera's Camera and BikeCamera
// offsets; the declared class there keeps them protected.
struct UnknownRaceSoundCamera {
    unsigned char field_0x000[0x170];
    Vector3 listenerPosition;                  // position
    Vector3 listenerForward;                  // forward
    Vector3 listenerUp;                  // up
    unsigned char field_0x194[0x3b0 - 0x194];
    UnknownEventRacer* followedBike;       // followed bike
    UnknownEventRacer* followedRacer;       // racer the listener follows
};

// One 0x54-byte engine channel per racer (RaceSound+0x98, eleven of them).
// Each owns a 44100-byte sample buffer; the channel at RaceSound+0x434 is
// streamed into its Sound in two halves (0x004e4a30 / 0x004e4d10).
struct UnknownRaceSoundChannel {
    UnknownEventRacer* channelRacer;        // racer
    Sound* channelSound;
    int voiceIndex;                        // +0x08, -1 when reset
    int engine;                            // +0x0c, engine (125/250/400 section) indexing the sample sets
    int sampleKind;                        // +0x10, sample playing: 1 idle, 2 low/mid/high torque, 3 power band,
                                           // 4 sputter, 5 decel, 6 rev
    int sampleBytes;                       // +0x14, bytes in streamSample
    int sampleOffset;                      // +0x18, bytes of streamSample consumed
    int fillHalf;                          // +0x1c, half being filled (1 or 2)
    int nextSampleKind;                    // +0x20, kind to stream after this one (0: loop it)
    int field_0x24;
    int field_0x28;
    int wasAirborne;                       // +0x2c, racer +0x108 last frame (the LandHard cue)
    int field_0x30;
    int field_0x34;
    void* streamSample;                     // sample being streamed
    float airborneTime;                     // seconds airborne
    int field_0x40;
    int speed;                             // +0x44, racer +0xb8 * 0.68182 against the *MaxSpeed limits
    int field_0x48;
    int cameraDistance;                    // +0x4c, GroundDistance from the camera; 1000000 when reset
    void* sampleBuffer;                     // 44100-byte buffer
};

// EAX 1.0 listener settings (16 bytes) passed to PCSoundInterface
// 0x004bed40: an environment index, its volume, decay time and damping.
struct UnknownEaxListenerParameters {
    unsigned long environment;
    float volume;
    float decayTime;
    float damping;
};

class RaceSound : public GameObject {
public:
    explicit RaceSound(int flags);        // 0x004e1fb0
    virtual ~RaceSound();                 // 0x004e21b0 (deleting wrapper 0x004e2190)
    virtual int UnknownVirtualSlot10(float frameTime);                                    // 0x004e3730
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004e44e0

    // 0x004e23f0 (thiscall, ret 0x10; bikerace.cpp 0x00419628 passes its
    // +0x18, itself, its +0x50 camera and 6): creates the sound groups and
    // loads every sample; returns this.
    RaceSound* Create(void* owner, UnknownKrustyBikeView* view, UnknownRaceSoundCamera* camera,
                                     int racers);
    // 0x004e3430: assigns the racers to channels and resets the listener and
    // the 3D distances.
    void AssignChannels();
    int UpdateRacerSounds(float frameTime); // 0x004e39b0: per-racer sounds
    // 0x004e42e0 / 0x004e43a0: plays a random "wreck%02d.wav" (wreckSounds)
    // or "fall%02d.wav" (fallSounds) sound at `racer`; the index, or 0 after ten busy tries.
    int PlayWreckSound(UnknownEventRacer* racer);
    int PlayFallSound(UnknownEventRacer* racer);
    // 0x004e4460: the ground distance between two points, 500 when either
    // axis is further apart than that.
    float GroundDistance(Vector3* a, Vector3* b);
    void StartStream();         // 0x004e4a30: starts the stream
    void RefillStream();         // 0x004e4d10: refills a half
    // 0x004e5500: reads the raw sample `name` from Audio.res into a new
    // buffer; its size goes to *size (both 0 on failure).
    void ReadSample(const char* name, void** data, int* size);
    // 0x004e5660: loads `name` from Audio.res into `sound`.
    int LoadSound(Sound* sound, const char* name, int a, int b);
    // 0x004e5780: plays `sound` while sound is on (`network` sounds only in
    // network games); `stop` rewinds it first.
    void PlayIfEnabled(Sound* sound, int network, int stop, int loop, int unused);
    void UnknownFunction4e57d0(UnknownEventRacer* racer, float value); // 0x004e57d0
    void UnknownFunction4e5860();         // 0x004e5860: plays itSound
    // 0x004e5880 (qsort callback): orders channels by cameraDistance.
    static int CompareChannels(const void* a, const void* b);

    UnknownEventRacer* listenerRacer;        // listener's racer
    int field_0x30;
    UnknownKrustyBikeView* raceView;
    UnknownRaceSoundCamera* listenerCamera;
    SoundGroup* engineGroup;               // +0x3c, engine voices and LandHard01
    SoundGroup* field_0x40;
    SoundGroup* effectsGroup;              // +0x44, launch, bonus, wreck/fall and waypoint sounds, racer +0xb8 * 0.68182 against the *MaxSpeed limits
    SoundGroup* field_0x48;
    SoundGroup* crowdSounds;               // crowd sounds (network games); slot 23 sets its volume
    long musicVolume;                      // music volume: 0, -500 or -10000
    int eaxEnabled;                       // EAX environment on
    int field_0x58;
    int field_0x5c;
    int field_0x60;
    int field_0x64;
    UnknownWaveHeader waveHeader;         // header read by 0x004e5500
    int channelsInUse;                       // channels in use
    UnknownRaceSoundChannel engineChannels[11];
    UnknownRaceSoundChannel* streamChannel; // streamed channel
    UnknownEventRacer* currentRacer;       // racer being updated
    Sound* engineVoices[4];                // the four engine voices
    int engineVoiceInUse[4];                   // voice in use
    // Engines present in the race (audio_*.ini sections "125", "250", "400",
    // chosen by racer +0x738/+0x737),
    // the divisor applied to each engine's sample counts, the listener's
    // engine and the number of engines present (0x004e23f0).
    int enginePresent[3];                  // +0x45c
    int sampleDivisor[3];                  // +0x468
    int listenerEngine;                    // +0x474
    int engineCount;                       // +0x478
    // Engine speeds that select the next sample set, per engine.
    int lowTorqueMaxSpeed[3];              // +0x47c, "LowTorqueMaxSpeed" (20)
    int midTorqueMaxSpeed[3];              // +0x488, "MidTorqueMaxSpeed" (55)
    int highTorqueMaxSpeed[3];             // +0x494, "HighTorqueMaxSpeed" (100)
    unsigned char field_0x4a0[0x83c - 0x4a0];
    Sound* landHardSound;                   // +0x83c, "LandHard01.wav" (played on landing)
    // Eight sample sets of three engines by ten samples (data, then byte
    // counts, then the number loaded per engine).
    void* idleSamples[3][10];              // +0x840, "idle_%d"
    void* lowTorqueSamples[3][10];         // +0x8b8, "LowTorque_%d"
    void* midTorqueSamples[3][10];         // +0x930, "MidTorque_%d"
    void* highTorqueSamples[3][10];        // +0x9a8, "HighTorque_%d"
    void* powerBandSamples[3][10];         // +0xa20, "PowerBand_%d"
    void* decelSamples[3][10];             // +0xa98, "Decel_%d"
    void* sputterSamples[3][10];           // +0xb10, "Sputter_%d"
    void* revSamples[3][10];               // +0xb88, "Rev_%d"
    int idleSampleBytes[3][10];            // +0xc00
    int lowTorqueSampleBytes[3][10];       // +0xc78
    int midTorqueSampleBytes[3][10];       // +0xcf0
    int highTorqueSampleBytes[3][10];      // +0xd68
    int powerBandSampleBytes[3][10];       // +0xde0
    int decelSampleBytes[3][10];           // +0xe58
    int sputterSampleBytes[3][10];         // +0xed0
    int revSampleBytes[3][10];             // +0xf48
    unsigned char field_0xfc0[0x1128 - 0xfc0];
    int idleCount[3];                      // +0x1128, "TotalIdle" / divisor
    int lowTorqueCount[3];                 // +0x1134, "TotalLowTorque"
    int midTorqueCount[3];                 // +0x1140, "TotalMidTorque"
    int highTorqueCount[3];                // +0x114c, "TotalHighTorque"
    int powerBandCount[3];                 // +0x1158, "TotalPowerBand"
    int decelCount[3];                     // +0x1164, "TotalDecel"
    int sputterCount[3];                   // +0x1170, "TotalSputter"
    int revCount[3];                       // +0x117c, "TotalRev"
    unsigned char field_0x1188[0x11ac - 0x1188];
    Sound* bonusSound;                     // +0x11ac, "bonus.wav"
    Sound* itSound;                        // +0x11b0, "It.wav"
    Sound* launchSound;                    // +0x11b4, "launch.wav"
    Sound* wreckSounds[6];                 // +0x11b8, "wreck01.wav".."wreck06.wav"
    Sound* fallSounds[6];                  // +0x11d0, "fall01.wav".."fall06.wav"
    Sound* crowdLoop;                      // +0x11e8, "CrowdLoop.wav" (network games)
    Sound* applause1;                      // +0x11ec, "applause01.wav"
    Sound* applause2;                      // +0x11f0, "applause02.wav"
    Sound* applause3;                      // +0x11f4, "applause03.wav"
    Sound* booSound;                       // +0x11f8, "boo_01.wav"
    Sound* ohSound;                        // +0x11fc, "oh_01.wav"
    int field_0x1200;
    Sound* waypointSound;                  // +0x1204, "Waypoint.wav"
    int field_0x1208;                     // reset (0x004e3430) on the next frame
    int crashHandled[11];                     // +0x120c, set once a racer's crash (+0x444) sounds started
    int racerWreckSound[11];                  // +0x1238
    int racerFallSound[11];                   // +0x1264
    int field_0x1290;
    int field_0x1294;
    int bytesPerSample;                    // +0x1298, 2 with audio_16.ini, 1 with audio_08.ini
    int lastListenerPosition;                 // +0x129c, listener's last position (99 initially)
    UnknownEventRacer* lastListenerRacer;     // +0x12a0, listener's racer last frame
    float ohClock;                            // +0x12a4, seconds since oh_01.wav
    float applauseClock;                      // +0x12a8, seconds since applause01/02
    float stuntApplauseClock;                 // +0x12ac, seconds since 0x004e57d0's applause
};

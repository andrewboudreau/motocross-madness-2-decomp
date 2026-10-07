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
    int field_0x08;                       // -1 when reset
    int field_0x0c;                       // sample set: 0, or 1/2 by engine
    int field_0x10;
    int field_0x14;                       // bytes in field_0x38
    int field_0x18;                       // bytes of field_0x38 consumed
    int field_0x1c;                       // half being filled (1 or 2)
    int field_0x20;                       // next sample set
    int field_0x24;
    int field_0x28;
    int field_0x2c;
    int field_0x30;
    int field_0x34;
    void* streamSample;                     // sample being streamed
    float airborneTime;                     // seconds airborne
    int field_0x40;
    int field_0x44;
    int field_0x48;
    int field_0x4c;                       // 1000000 when reset
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
    // 0x004e42e0 / 0x004e43a0: plays a random idle sound from field_0x11b8
    // (field_0x11d0) at `racer`; the index, or 0 after ten busy tries.
    int UnknownFunction4e42e0(UnknownEventRacer* racer);
    int UnknownFunction4e43a0(UnknownEventRacer* racer);
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
    void UnknownFunction4e5860();         // 0x004e5860: plays field_0x11b0
    // 0x004e5880 (qsort callback): orders channels by field_0x4c.
    static int CompareChannels(const void* a, const void* b);

    UnknownEventRacer* listenerRacer;        // listener's racer
    int field_0x30;
    UnknownKrustyBikeView* raceView;
    UnknownRaceSoundCamera* listenerCamera;
    SoundGroup* field_0x3c;
    SoundGroup* field_0x40;
    SoundGroup* field_0x44;
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
    int field_0x45c[3];
    int field_0x468[3];
    int field_0x474;
    int field_0x478;
    // Engine speeds that select the next sample set, per engine.
    int field_0x47c[3];
    int field_0x488[3];
    int field_0x494[3];
    unsigned char field_0x4a0[0x83c - 0x4a0];
    Sound* ownEngine;                   // listener's own engine
    // Eight sample sets of three engines by ten samples (data, then byte
    // counts, then the number loaded per engine).
    void* field_0x840[3][10];
    void* field_0x8b8[3][10];
    void* field_0x930[3][10];
    void* field_0x9a8[3][10];
    void* field_0xa20[3][10];
    void* field_0xa98[3][10];
    void* field_0xb10[3][10];
    void* field_0xb88[3][10];
    int field_0xc00[3][10];
    int field_0xc78[3][10];
    int field_0xcf0[3][10];
    int field_0xd68[3][10];
    int field_0xde0[3][10];
    int field_0xe58[3][10];
    int field_0xed0[3][10];
    int field_0xf48[3][10];
    unsigned char field_0xfc0[0x1128 - 0xfc0];
    int field_0x1128[3];
    int field_0x1134[3];
    int field_0x1140[3];
    int field_0x114c[3];
    int field_0x1158[3];
    int field_0x1164[3];
    int field_0x1170[3];
    int field_0x117c[3];
    unsigned char field_0x1188[0x11ac - 0x1188];
    Sound* field_0x11ac;
    Sound* field_0x11b0;
    Sound* field_0x11b4;
    Sound* field_0x11b8[6];
    Sound* field_0x11d0[6];
    Sound* field_0x11e8;
    Sound* field_0x11ec;
    Sound* field_0x11f0;
    Sound* field_0x11f4;
    Sound* field_0x11f8;
    Sound* field_0x11fc;
    int field_0x1200;
    Sound* field_0x1204;
    int field_0x1208;                     // reset (0x004e3430) on the next frame
    int field_0x120c[11];
    int field_0x1238[11];
    int field_0x1264[11];
    int field_0x1290;
    int field_0x1294;
    int field_0x1298;                     // bytes per sample
    int field_0x129c;                     // listener's last position (99 initially)
    UnknownEventRacer* field_0x12a0;      // listener's racer last frame
    float field_0x12a4;                   // seconds since the matching sound
    float field_0x12a8;
    float field_0x12ac;
};

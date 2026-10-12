#pragma once

// AuralScape.h -- the reconstructed D:\aardvark\VC\krusty2\AuralScape.cpp
// (literal __FILE__ at 0x005667e8, lines 130 and 717). Code
// 0x00401a30..0x00403d4b, between Arrow.cpp and BackgroundImage.cpp; see
// AuralScape.cpp for the extent evidence.
//
// Confirmed (tier 1) by RTTI: SoundGroup : GameObject (vtable 0x00550500),
// SoundInterface (root, vtable 0x00550570, one slot), SoundEmitter :
// GameObject (0x0055057c), SoultreeSoundEmitter : SoundEmitter (0x005505f8)
// and AuralScape : GameObject (0x00550668), all with 27-slot GameObject
// vtables except SoundInterface. SoundGroup is declared in PCAudio.h and
// SoundInterface in SoundInterface.h. Allocation sizes: SoundEmitter 0x1a4
// and SoultreeSoundEmitter 0x1ac (SceneManager.cpp). Member and method names
// are provisional (tier 3); DirectSound roles follow the interface slots.

#include "GameObject.h"
#include "MatrixUtil.h"
#include "PCAudio.h"
#include "SoundInterface.h"

class AuralScape;
class UnknownVehiclePart;
struct UnknownFollowCameraSubject;

// 8-byte listener record AuralScape keeps two of (not RTTI-typed): the
// owner and a DirectSound 3D listener. Every setter is deferred (apply 1)
// and returns whether the call succeeded.
class AuralScapeListener {
public:
    AuralScapeListener();  // folded body 0x004676a0
    ~AuralScapeListener(); // 0x00402b20: releases the listener
    // 0x00402b40: queries a listener and sets its doppler and rolloff
    // factors and a distance factor of 0.3048 (feet).
    int Initialize(AuralScape* owner, float doppler, float rolloff);
    int CommitDeferredSettings();
    int SetDistanceFactor(float factor);
    int SetDopplerFactor(float factor);
    int SetRolloffFactor(float factor);
    int SetOrientation(Vector3 front, Vector3 top);
    int GetPosition(Vector3* position);
    int SetPosition(Vector3 position);
    int SetVelocity(Vector3 velocity);

    AuralScape* ownerScape;                   // +0x00
    UnknownSoundListener* soundListener;      // +0x04
};

// RTTI: SoundEmitter : GameObject (0x1a4 bytes). A named sound placed in
// the scene; AuralScape decides which emitters get a Sound.
class SoundEmitter : public GameObject {
public:
    SoundEmitter(AuralScape* scape, SoundGroup* group, int type, int flags); // 0x004020e0
    virtual ~SoundEmitter(); // 0x00402200 (deleting wrapper 0x004021e0)
    // 0x004028d0: derives the velocity from the last move.
    virtual int UnknownVirtualSlot10(float frameTime);

    // 0x00402260: records the settings, checks `name` exists in the Res
    // archive and joins the scape; releases itself and returns 0 on failure.
    SoundEmitter* UnknownFunction402260(void* target, const char* name, UnknownSound3DParameters params,
                                        unsigned long flags, float oneShotDistance,
                                        float randomTriggerPercent, int is2D, int force2D);
    void SetSoundParameters(UnknownSound3DParameters params); // 0x00402420
    void UnknownFunction402470(Vector3* position, Vector3* velocity); // 0x00402470
    // 0x00402530: the distance to the nearest listener; sets bit 1 when
    // the emitter can be heard.
    void UpdateListenerDistance(int count, AuralScapeListener** listeners);
    int IsPlaying();             // 0x00402680: whether playing
    int SetVolume(long volume);  // 0x00402690
    int SetPan(long pan);     // 0x004026b0
    int Stop();             // 0x004026d0: stops
    int LoadSound(unsigned long flags); // 0x00402700: loads the sound
    void ReleaseSound();            // 0x004027f0: releases the sound
    int Play();             // 0x00402810: starts playing

    AuralScape* ownerScape;                   // +0x2c
    SoundGroup* emitterGroup;                 // +0x30
    Sound* loadedSound;                       // +0x34
    float oneShotRange;                       // +0x38 one-shot distance
    float triggerPercent;                     // +0x3c random trigger percent (100)
    short emitterType;                        // +0x40 type
    int serialNumber;                         // +0x44 serial number (0x00577734)
    char fileName[0x104];                     // +0x48 file name
    unsigned long emitterFlags;               // +0x14c flags: 1 one-shot, 2 3D, 4, 8 loop, 0x10/0x20 buffer kind
    UnknownSound3DParameters soundParameters; // +0x150
    Vector3 previousPosition;                 // +0x190 previous position
    float listenerDistance;                   // +0x19c distance to the nearest listener
    unsigned char oneShotFired : 1;           // +0x1a0 one-shot fired
    unsigned char audible : 1;                // +0x1a0 audible
    unsigned char field_0x1a0_bit2 : 1;
    unsigned char field_0x1a0_bit3 : 1;
    unsigned char started : 1;                // +0x1a0 started
};

// RTTI: SoultreeSoundEmitter : SoundEmitter (0x1ac bytes): an emitter that
// follows a model part.
class SoultreeSoundEmitter : public SoundEmitter {
public:
    SoultreeSoundEmitter(AuralScape* scape, SoundGroup* group, int type, int flags); // 0x00402980
    // Implicit destructor 0x004029e0 (deleting wrapper 0x004029c0): a tail
    // jump to ~SoundEmitter without a vptr store.
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00402a70

    // 0x004029f0: SoundEmitter's loader, then the part (and its child
    // `partName`) to follow.
    SoultreeSoundEmitter* UnknownFunction4029f0(void* target, const char* name, const char* partName,
                                                UnknownVehiclePart* part, UnknownSound3DParameters params,
                                                unsigned long flags, float oneShotDistance,
                                                float randomTriggerPercent, int is2D, int force2D);

    UnknownVehiclePart* followedPart;         // +0x1a4
    UnknownVehiclePart* childPart;            // +0x1a8
};

// What AuralScape+0x94 points at: +0xa4 holds a table whose +0x410 has four
// reverb presets (tier 3).
struct UnknownAuralScapeReverbTable {
    unsigned char field_0x000[0x410];
    unsigned long field_0x410[4];
};

// GameObject+0x18 as AuralScape's slot 10 reads it: +0x08 points at an
// object whose +0x170 is the position the reverb zone is looked up for.
struct UnknownAuralScapeViewer {
    unsigned char field_0x000[0x170];
    Vector3 field_0x170;
};

struct UnknownAuralScapeTarget {
    unsigned char field_0x00[0x08];
    UnknownAuralScapeViewer* field_0x08;
};

struct UnknownAuralScapeReverbOwner {
    unsigned char field_0x00[0xa4];
    UnknownAuralScapeReverbTable* field_0xa4;
};

// RTTI: AuralScape : GameObject. Created by QuarryStuntEvent.cpp
// (0x004dee5c..0x004deec4).
class AuralScape : public GameObject {
public:
    explicit AuralScape(int flags); // 0x00402da0
    // Implicit destructor 0x00402e80 (deleting wrapper 0x00402e60): no vptr
    // store, then the four lists and ~GameObject.
    // 0x00403710: updates the emitters, the reverb and the listeners, and
    // fills the "Audio" debug page.
    virtual int UnknownVirtualSlot10(float frameTime);

    AuralScape* UnknownFunction402f00(void* target, int sounds); // 0x00402f00
    int NextListenerNumber();                  // 0x00403000: next number
    int AddListener(int index, float doppler, float rolloff); // 0x00403010
    void UnknownFunction4030a0(int index, Vector3* position, Vector3* front, Vector3* top,
                               Vector3* velocity); // 0x004030a0
    void PickEmittersToPlay();                 // 0x00403150: picks the emitters to play
    void UnknownUpdateDistances();                // inline: each emitter's nearest listener
    int AddEmitter(SoundEmitter* emitter); // 0x00403b20
    int RemoveEmitter(SoundEmitter* emitter); // 0x00403bc0

    int listenerNumber;                       // +0x2c
    int soundBudget;                          // +0x30 sound budget
    int soundsLoaded;                         // +0x34 sounds loaded
    int listenerCount;                        // +0x38 listener count
    AuralScapeListener* listeners[2];         // +0x3c
    ContainerList<SoundEmitter*> allEmitters; // +0x44 all
    ContainerList<SoundEmitter*> audibleEmitters; // +0x58 audible
    ContainerList<SoundEmitter*> playingEmitters; // +0x6c playing
    ContainerList<SoundEmitter*> waitingEmitters; // +0x80 waiting for a sound
    UnknownAuralScapeReverbOwner* reverbOwner; // +0x94
    UnknownFollowCameraSubject* groundSubject; // +0x98
    int debugPage;                            // +0x9c debug page
    unsigned char reverbEnabled : 1;          // +0xa0 reverb enabled
};

// 0x00402070: volume (and pan, always 0) for a sound `distance` from the
// listener: 0 inside `minDistance`, -2500 at `maxDistance` and beyond.
void UnknownFunction402070(Vector3* listener, Vector3* position, float distance, float minDistance,
                           float maxDistance, long* volume, long* pan);

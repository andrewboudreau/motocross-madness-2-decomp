#pragma once

#include <windows.h>

#include "BaseObject.h"
#include "ContainerList.h"
#include "GameObject.h"
#include "MatrixUtil.h"
#include "SoundInterface.h"

class Sound;
class UnknownTextureStream;

// DirectSound objects reached through Sound (`this` on the stack). The
// IIDs Sound queries are the bytes of IID_IDirectSound3DBuffer
// (0x00556ca0), IID_IDirectSoundNotify (0x00556cd0) and IID_IKsPropertySet
// (0x00556ce0); the method names follow those interfaces (inference).
struct UnknownSound3DBuffer {
    virtual long __stdcall QueryInterface(const UnknownGuid& iid, void** result);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
    virtual long __stdcall GetAllParameters();
    virtual long __stdcall GetConeAngles();
    virtual long __stdcall GetConeOrientation();
    virtual long __stdcall GetConeOutsideVolume();
    virtual long __stdcall GetMaxDistance();
    virtual long __stdcall GetMinDistance();
    virtual long __stdcall GetMode();
    virtual long __stdcall GetPosition();
    virtual long __stdcall GetVelocity();
    virtual long __stdcall SetAllParameters(const struct UnknownSound3DParameters* parameters,
                                            unsigned long apply);
    virtual long __stdcall SetConeAngles(unsigned long inside, unsigned long outside, unsigned long apply);
    virtual long __stdcall SetConeOrientation(float x, float y, float z, unsigned long apply);
    virtual long __stdcall SetConeOutsideVolume(long volume, unsigned long apply);
    virtual long __stdcall SetMaxDistance(float distance, unsigned long apply);
    virtual long __stdcall SetMinDistance(float distance, unsigned long apply);
    virtual long __stdcall SetMode(unsigned long mode, unsigned long apply);
    virtual long __stdcall SetPosition(float x, float y, float z, unsigned long apply);
    virtual long __stdcall SetVelocity(float x, float y, float z, unsigned long apply);
};

struct UnknownSoundNotify {
    virtual long __stdcall QueryInterface(const UnknownGuid& iid, void** result);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
    virtual long __stdcall SetNotificationPositions(unsigned long count, const struct UnknownNotifyPosition* positions);
};

struct UnknownPropertySet {
    virtual long __stdcall QueryInterface(const UnknownGuid& iid, void** result);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
    virtual long __stdcall Get();
    virtual long __stdcall Set(const UnknownGuid* set, unsigned long id, void* instance, unsigned long instanceSize,
                               void* data, unsigned long dataSize);
    virtual long __stdcall QuerySupport(const UnknownGuid* set, unsigned long id, unsigned long* support);
};

// DSBPOSITIONNOTIFY layout.
struct UnknownNotifyPosition {
    unsigned long offset;
    HANDLE event;
};

// 16-byte PCM format (PCMWAVEFORMAT layout).
struct UnknownPcmFormat {
    unsigned short formatTag;
    unsigned short channels;
    unsigned long samplesPerSec;
    unsigned long avgBytesPerSec;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
};

// DS3DBUFFER layout (0x40 bytes).
struct UnknownSound3DParameters {
    unsigned long size;
    Vector3 position;
    Vector3 velocity;
    unsigned long insideConeAngle;
    unsigned long outsideConeAngle;
    Vector3 coneOrientation;
    long coneOutsideVolume;
    float minDistance;
    float maxDistance;
    unsigned long mode;
};

// The 44-byte RIFF header of a PCM .wav file: "RIFF", size, "WAVE", "fmt ",
// the format chunk's size, the PCM format and the "data" chunk header.
#pragma pack(push, 1)
struct UnknownWaveHeader {
    char riff[4];
    unsigned long riffSize;
    char wave[4];
    char fmt[4];
    unsigned long fmtSize;
    unsigned short formatTag;
    unsigned short channels;
    unsigned long samplesPerSec;
    unsigned long avgBytesPerSec;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    char data[4];
    unsigned long dataSize;
};
#pragma pack(pop)

extern "C" const UnknownGuid IID_IDirectSound3DBuffer; // 0x00556ca0
extern "C" const UnknownGuid IID_IDirectSoundNotify;   // 0x00556cd0
extern "C" const UnknownGuid IID_IKsPropertySet;       // 0x00556ce0
// 0x00556cf0: the 3D algorithm GUID new 3D buffers request; its value
// {C241333F-1C1B-11D2-94F5-00C04FC28ACA} is dsound.h's DS3DALG_NO_VIRTUALIZATION.
extern "C" const UnknownGuid DS3DALG_NO_VIRTUALIZATION;

// RTTI: SoundGroup : GameObject (vtable 0x00550500, 27 slots). Its code is
// at 0x00401a30..0x00401f74, reconstructed in AuralScape.cpp.
class SoundGroup : public GameObject {
public:
    explicit SoundGroup(int flags);           // 0x00401a30
    virtual ~SoundGroup();                    // 0x00401af0 (deleting wrapper 0x00401ad0)
    // Slots 4-7 call the GameObject slot, then slot 16 with 1, 0, 1, 0.
    virtual void Disable();                   // 0x00401e20
    virtual void Enable();                    // 0x00401e40
    virtual void OnDisable();                 // 0x00401e60
    virtual void OnEnable();                  // 0x00401e80
    virtual int Tick(float frameTime);                 // 0x00401f30: fades the members
    virtual int SetPaused(int value);                  // 0x00401ee0: pauses the members
    virtual int OnActivate();                          // 0x00401ea0: restores the members
    int AddMember(Sound* sound);  // 0x00401b50: adds a member
    int RemoveMember(Sound* sound);  // 0x00401be0: removes a member
    void ReleaseMembers();             // 0x00401c30: releases every member
    void SetMemberVolume(long volume);  // 0x00401dd0 (racesnd.cpp music volume)

    unsigned char field_0x2c_bit0 : 1; // members may play
    long memberVolume; // +0x30: volume applied to members
    ContainerList<Sound*> members; // +0x34
};

// 0x1c-byte notification helper a streaming Sound owns at +0x3c: a thread
// that refills the buffer's halves when DirectSound signals them.
class UnknownSoundNotifier {
public:
    UnknownSoundNotifier();                       // 0x004bb720
    ~UnknownSoundNotifier();                      // 0x004bb810
    int StartRefillThread(Sound* sound);                // 0x004bb740: starts the thread
    static unsigned __stdcall UnknownThreadProc(void* context); // 0x004bb630

    Sound* ownerSound;                 // +0x00
    UnknownSoundNotify* notify;        // +0x04
    HANDLE firstHalfEvent;             // +0x08: first half played
    HANDLE secondHalfEvent;            // +0x0c: second half played
    HANDLE stopEvent;                  // +0x10
    HANDLE refillThread;               // +0x14
    unsigned char skipNotification : 1; // +0x18 bit 0: set at construction, cleared by the next notification instead of a refill
};

// A volume fade: its length in milliseconds, the volume a fade-out stops
// at, and flags (1 rewind when stopped, 2 fade out).
struct UnknownSoundFade {
    int length;
    int target;
    unsigned char flags;
};

// RTTI: Sound : BaseObject (vtable 0x00555d88), 0x1f8 bytes. Names are
// provisional; the DirectSound roles follow the interface methods called.
class Sound : public BaseObject {
public:
    Sound(SoundGroup* group, int type); // 0x004bba10
    virtual ~Sound();                   // 0x004bbb60 (deleting wrapper 0x004bbb40)

    int Play();                                         // 0x004bbcd0: plays
    int ApplyCachedSettings();                          // 0x004bbdc0: reapplies the cached settings
    int LoadWave(UnknownTextureStream* stream, unsigned long flags, unsigned long controls,
        int duplicates, int streamBytes); // 0x004bbef0: loads a .wav
    int Load(const char* name, UnknownTextureStream* stream, int flags, int controls, int duplicates,
             int streamBytes);
    unsigned long GetBufferControlFlags(unsigned long flags); // 0x004bc490: control flags
    int DuplicateFrom(Sound* source);                   // 0x004bc4c0: duplicates `source`
    int SupportsProperty(const UnknownGuid* set, unsigned long id, unsigned long support);
    int SetProperty(const UnknownGuid* set, unsigned long id, void* instance,
        unsigned long instanceSize, void* data, unsigned long dataSize);
    int PlayWithOptions(int restart, unsigned long playFlags, int preferHardware);
    int Stop(int rewind);                               // 0x004bc940: stops
    int IsPlaying();                                    // 0x004bca80: whether playing
    int SetFrequency(unsigned long frequency, int force);
    int SetVolume(long volume, int force);
    int SetPan(long pan, int force);
    int GetBufferPosition(unsigned long* play, unsigned long* write);
    int SetBufferPosition(unsigned long position);
    int RestoreBuffer();                                // 0x004bcdc0: restores a lost buffer
    int SetPaused(int paused);
    int UpdateFade(float elapsed);                      // 0x004bcea0: fades
    void ReleaseBuffers();                              // 0x004bcf50: releases the buffers
    int LockBuffer(unsigned long offset, unsigned long bytes, void** first, unsigned long* firstBytes,
        void** second, unsigned long* secondBytes, unsigned long flags);
    int UnlockBuffer(void* first, unsigned long firstBytes, void* second, unsigned long secondBytes);
    int FillBufferFromFile(UnknownSoundBuffer** buffer); // 0x004bd0c0: fills from the file
    int FillBufferFromStream(UnknownSoundBuffer** buffer, UnknownTextureStream* stream, unsigned long offset,
        unsigned long bytes);
    int CreatePendingBuffer();                          // 0x004bd4b0: creates the pending buffer
    int CreateBuffer(UnknownSoundBuffer** buffer, unsigned long bytes, unsigned long rate,
        int bits, int blockAlign, int stereo, int is3D, int isStatic,
        unsigned long flags, int hardware);
    int DuplicateBuffer(UnknownSoundBuffer** duplicate, Sound* source);
    int Query3DBuffer(UnknownSoundBuffer* buffer, UnknownSound3DBuffer** buffer3D);
    int QueryPropertySet();                             // 0x004bd710: queries the property set
    int Set3DParameters(UnknownSound3DParameters* parameters);
    int Set3DMode(unsigned long mode);
    int Set3DPosition(Vector3 position, int force);
    int Set3DVelocity(Vector3 velocity, int force);
    int Set3DDistanceRange(float minDistance, float maxDistance, int force);
    int Set3DConeAngles(unsigned long insideConeAngle, unsigned long outsideConeAngle, int force);
    int Set3DConeOrientation(Vector3 orientation, int force);
    int Set3DConeOutsideVolume(long volume, int force);

    SoundGroup* soundGroup;                      // +0x08
    UnknownSoundBuffer* dsBuffer;             // +0x0c
    UnknownSound3DBuffer* dsBuffer3D;         // +0x10: the buffer's 3D interface
    UnknownPropertySet* propertySet;         // +0x14
    UnknownSoundBuffer* pendingBuffer;       // +0x18: buffer being loaded (streaming)
    UnknownSound3DBuffer* pendingBuffer3D;   // +0x1c
    UnknownSoundBuffer** duplicateBuffers;        // +0x20
    int duplicateCount;                      // +0x24
    Sound* duplicateSource;                  // +0x28: source of a duplicate
    UnknownSoundFade fade;                   // +0x2c
    int fadeVolume;                          // +0x38
    UnknownSoundNotifier* notifier;          // +0x3c
    UnknownTextureStream* sourceStream;           // +0x40: stream of a streamed sound
    unsigned long filledHalf;                // +0x44: offset of the half last refilled
    CRITICAL_SECTION bufferLock;                  // +0x48
    char soundName[0x104];                       // +0x60
    int lastPlayTime;                        // +0x164: time last played (0x004bfa80 ms)
    short soundType;                             // +0x168
    UnknownWaveHeader waveHeader;                 // +0x16a
    unsigned long bufferBytes;               // +0x198
    unsigned long currentFrequency;                // +0x19c
    long currentVolume;                            // +0x1a0
    long currentPan;                               // +0x1a4
    UnknownSound3DParameters parameters3D;   // +0x1a8
    unsigned long soundFlags;                    // +0x1e8: 2 static, 4 streamed, 8 3D, 0x10 cone, 0x20 loop
    unsigned long controlsSet;               // +0x1ec: 0x20 frequency, 0x40 pan, 0x80 volume
    unsigned long lastPlayFlags;                // +0x1f0
    unsigned char isPacked : 1;              // +0x1f4 bit 0: packed in an archive
    unsigned char isDuplicate : 1;           // +0x1f4 bit 1
    unsigned char field_0x1f4_bit2 : 1;
    unsigned char isLooping : 1;             // +0x1f4 bit 3
    unsigned char pausedWhilePlaying : 1;    // +0x1f4 bit 4
    unsigned char isFading : 1;              // +0x1f4 bit 5
    unsigned char field_0x1f4_bit6 : 1;
    unsigned char field_0x1f4_bit7 : 1;
    unsigned char bufferReady : 1;           // +0x1f5 bit 0
    unsigned char isStopped : 1;             // +0x1f5 bit 1
    unsigned char isFilling : 1;             // +0x1f5 bit 2: FillBufferFromStream running
    unsigned char isPaused : 1;              // +0x1f5 bit 3
};

// 0x004bb890: loads (or finds) a sound by name; not a member.
Sound* LoadEntrySound(SoundGroup* group, const char* name, int flags, int controls, int duplicates,
                      int streamBytes);

// 0x54-byte sound memory manager PCSoundInterface owns at +0x46c (not
// RTTI-typed): streamed sounds are queued for its thread, which creates and
// fills their buffers, evicting idle sounds to stay within the budget.
class UnknownPCAudioObject {
public:
    UnknownPCAudioObject();  // 0x004bddf0
    ~UnknownPCAudioObject(); // 0x004bde40: stops the thread, closes the handles
    static unsigned __stdcall UnknownThreadProc(void* context); // 0x004bdc00
    int StartLoaderThread(int budget);                  // 0x004bdef0: starts the thread
    void QueueSoundLoad(Sound* sound);                  // 0x004bdfc0: queues a load
    void RecordLoadedSound(Sound* sound, int bytes);    // 0x004be0a0: records a loaded sound
    void MakeRoomForSound(int bytes);                   // 0x004be130: evicts to make room
    void UnloadSound(Sound* sound);                     // 0x004be220: unloads a sound

    HANDLE loaderThread;               // +0x00
    HANDLE stopEvent;                  // +0x04: signalled to stop the thread
    HANDLE queuedEvent;                // +0x08: signalled when a load is queued
    CRITICAL_SECTION queueLock;        // +0x0c
    unsigned long byteBudget;          // +0x24: budget in bytes
    unsigned long loadedBytes;         // +0x28
    ContainerList<Sound*> queuedSounds; // +0x2c
    ContainerList<Sound*> loadedSounds; // +0x40
};

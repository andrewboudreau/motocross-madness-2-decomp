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
// 0x00556cf0: the 3D algorithm GUID new 3D buffers request.
extern "C" const UnknownGuid g_UnknownSound3DAlgorithm;

// RTTI: SoundGroup : GameObject (vtable 0x00550500, 27 slots). Its code is
// at 0x00401a30..0x00401f74, reconstructed in AuralScape.cpp.
class SoundGroup : public GameObject {
public:
    explicit SoundGroup(int flags);           // 0x00401a30
    virtual ~SoundGroup();                    // 0x00401af0 (deleting wrapper 0x00401ad0)
    // Slots 4-7 call the GameObject slot, then slot 16 with 1, 0, 1, 0.
    virtual void UnknownVirtualSlot4();       // 0x00401e20
    virtual void UnknownVirtualSlot5();       // 0x00401e40
    virtual void UnknownVirtualSlot6();       // 0x00401e60
    virtual void UnknownVirtualSlot7();       // 0x00401e80
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00401f30: fades the members
    virtual int UnknownVirtualSlot16(int value);       // 0x00401ee0: pauses the members
    virtual int UnknownVirtualSlot18();                // 0x00401ea0: restores the members
    int UnknownFunction401b50(Sound* sound);  // 0x00401b50: adds a member
    int UnknownFunction401be0(Sound* sound);  // 0x00401be0: removes a member
    void UnknownFunction401c30();             // 0x00401c30: releases every member
    void UnknownFunction401dd0(long volume);  // 0x00401dd0 (racesnd.cpp music volume)

    unsigned char field_0x2c_bit0 : 1; // members may play
    long field_0x30; // volume applied to members
    ContainerList<Sound*> field_0x34; // members
};

// 0x1c-byte notification helper a streaming Sound owns at +0x3c: a thread
// that refills the buffer's halves when DirectSound signals them.
class UnknownSoundNotifier {
public:
    UnknownSoundNotifier();                       // 0x004bb720
    ~UnknownSoundNotifier();                      // 0x004bb810
    int StartRefillThread(Sound* sound);                // 0x004bb740: starts the thread
    static unsigned __stdcall UnknownThreadProc(void* context); // 0x004bb630

    Sound* field_0x00;
    UnknownSoundNotify* field_0x04;
    HANDLE field_0x08; // first half played
    HANDLE field_0x0c; // second half played
    HANDLE field_0x10; // stop
    HANDLE field_0x14; // thread
    unsigned char field_0x18_bit0 : 1;
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
    int UnknownFunction4bc320(const char* name, UnknownTextureStream* stream, int flags, int a, int b, int c);
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

    SoundGroup* field_0x08;
    UnknownSoundBuffer* field_0x0c;          // buffer
    UnknownSound3DBuffer* field_0x10;        // its 3D interface
    UnknownPropertySet* field_0x14;
    UnknownSoundBuffer* field_0x18;          // buffer being loaded (streaming)
    UnknownSound3DBuffer* field_0x1c;
    UnknownSoundBuffer** field_0x20;         // duplicates
    int field_0x24;                          // duplicate count
    Sound* field_0x28;                       // source of a duplicate
    UnknownSoundFade field_0x2c;
    int field_0x38;                          // fade volume
    UnknownSoundNotifier* field_0x3c;
    UnknownTextureStream* field_0x40;        // stream of a streamed sound
    unsigned long field_0x44;
    CRITICAL_SECTION field_0x48;
    char field_0x60[0x104];                  // name
    int field_0x164;                         // time last played
    short field_0x168;                       // type
    UnknownWaveHeader field_0x16a;
    unsigned long field_0x198;               // buffer bytes
    unsigned long field_0x19c;               // frequency
    long field_0x1a0;                        // volume
    long field_0x1a4;                        // pan
    UnknownSound3DParameters field_0x1a8;
    unsigned long field_0x1e8;               // flags: 2 static, 4 streamed, 8 3D, 0x10 cone, 0x20 loop
    unsigned long field_0x1ec;               // controls set: 0x20 frequency, 0x40 pan, 0x80 volume
    unsigned long field_0x1f0;               // play flags
    unsigned char field_0x1f4_bit0 : 1;      // packed in an archive
    unsigned char field_0x1f4_bit1 : 1;      // a duplicate
    unsigned char field_0x1f4_bit2 : 1;
    unsigned char field_0x1f4_bit3 : 1;      // looping
    unsigned char field_0x1f4_bit4 : 1;      // paused while playing
    unsigned char field_0x1f4_bit5 : 1;      // fading
    unsigned char field_0x1f4_bit6 : 1;
    unsigned char field_0x1f4_bit7 : 1;
    unsigned char field_0x1f5_bit0 : 1;      // buffer ready
    unsigned char field_0x1f5_bit1 : 1;      // stopped
    unsigned char field_0x1f5_bit2 : 1;
    unsigned char field_0x1f5_bit3 : 1;      // paused
};

// 0x004bb890: loads (or finds) a sound by name; not a member.
Sound* UnknownFunction4bb890(SoundGroup* group, const char* name, int flags, int a, int b, int c);

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

    HANDLE field_0x00; // thread
    HANDLE field_0x04; // event signalled to stop the thread
    HANDLE field_0x08; // event signalled when a load is queued
    CRITICAL_SECTION field_0x0c;
    unsigned long field_0x24; // budget in bytes
    unsigned long field_0x28; // bytes loaded
    ContainerList<Sound*> field_0x2c; // queued
    ContainerList<Sound*> field_0x40; // loaded
};

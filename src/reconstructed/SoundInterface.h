#pragma once

#include "ContainerList.h"
#include "Guid.h"

// RTTI: SoundInterface (root; its only slot is the destructor) and
// PCSoundInterface : SoundInterface (0x478 bytes, the size Game's
// initialiser allocates). PCSoundInterface's methods are in PCAudio.cpp
// (literal __FILE__ at 0x0056fa28).

class Sound;

class SoundInterface {
public:
    SoundInterface();          // 0x00401f80
    virtual ~SoundInterface(); // 0x00401fe0

    // Two pointer lists; PCSoundInterface's start-up Inits both to 8
    // entries growing by 8. Element types are unknown.
    ContainerList<void*> field_0x04;
    ContainerList<void*> field_0x18;
    unsigned char field_0x2c_bit0 : 1; // set once the audio has started
};

// COM-style DirectSound objects (`this` on the stack). The listener is
// queried from the primary buffer with the IID at 0x00556c90, the bytes of
// IID_IDirectSound3DListener ({279AFA84-4981-11CE-A521-0020AF0BE560}); the
// method names follow the DirectSound interfaces, which is inference from
// that IID, the buffer description and the call shapes.
struct UnknownSoundCaps;
struct UnknownSoundBufferDesc;
struct UnknownSoundBuffer;

struct UnknownDirectSound {
    virtual long __stdcall QueryInterface(const UnknownGuid& iid, void** result);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
    virtual long __stdcall CreateSoundBuffer(const UnknownSoundBufferDesc* desc, UnknownSoundBuffer** buffer,
                                             void* outer);
    virtual long __stdcall GetCaps(UnknownSoundCaps* caps);
    virtual long __stdcall DuplicateSoundBuffer(UnknownSoundBuffer* original, UnknownSoundBuffer** duplicate);
    virtual long __stdcall SetCooperativeLevel(void* window, unsigned long level);
};

struct UnknownSoundBuffer {
    virtual long __stdcall QueryInterface(const UnknownGuid& iid, void** result);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
    virtual long __stdcall GetCaps();
    virtual long __stdcall GetCurrentPosition(unsigned long* play, unsigned long* write);
    virtual long __stdcall GetFormat();
    virtual long __stdcall GetVolume(long* volume);
    virtual long __stdcall GetPan();
    virtual long __stdcall GetFrequency();
    virtual long __stdcall GetStatus(unsigned long* status);
    virtual long __stdcall Initialize();
    virtual long __stdcall Lock(unsigned long offset, unsigned long bytes, void** first, unsigned long* firstBytes,
                                void** second, unsigned long* secondBytes, unsigned long flags);
    virtual long __stdcall Play(unsigned long reserved, unsigned long priority, unsigned long flags);
    virtual long __stdcall SetCurrentPosition(unsigned long position);
    virtual long __stdcall SetFormat(const struct UnknownWaveFormat* format);
    virtual long __stdcall SetVolume(long volume);
    virtual long __stdcall SetPan(long pan);
    virtual long __stdcall SetFrequency(unsigned long frequency);
    virtual long __stdcall Stop();
    virtual long __stdcall Unlock(void* first, unsigned long firstBytes, void* second, unsigned long secondBytes);
    virtual long __stdcall Restore();
};

struct UnknownSoundListener {
    virtual long __stdcall QueryInterface(const UnknownGuid& iid, void** result);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();
    virtual long __stdcall GetAllParameters();
    virtual long __stdcall GetDistanceFactor();
    virtual long __stdcall GetDopplerFactor();
    virtual long __stdcall GetOrientation();
    virtual long __stdcall GetPosition();
    virtual long __stdcall GetRolloffFactor();
    virtual long __stdcall GetVelocity();
    virtual long __stdcall SetAllParameters();
    virtual long __stdcall SetDistanceFactor(float factor, unsigned long apply);
    virtual long __stdcall SetDopplerFactor(float factor, unsigned long apply);
    virtual long __stdcall SetOrientation(float frontX, float frontY, float frontZ, float topX, float topY,
                                          float topZ, unsigned long apply);
    virtual long __stdcall SetPosition(float x, float y, float z, unsigned long apply);
    virtual long __stdcall SetRolloffFactor(float factor, unsigned long apply);
    virtual long __stdcall SetVelocity(float x, float y, float z, unsigned long apply);
    virtual long __stdcall CommitDeferredSettings();
};

// 0x60-byte device capabilities (DSCAPS layout; GetCaps is passed size 0x60,
// and the field names follow the debug report's format string at 0x0056fac8).
struct UnknownSoundCaps {
    unsigned long size;
    unsigned long flags;
    unsigned long minSecondarySampleRate;
    unsigned long maxSecondarySampleRate;
    unsigned long primaryBuffers;
    unsigned long maxHwMixingAllBuffers;
    unsigned long maxHwMixingStaticBuffers;
    unsigned long maxHwMixingStreamingBuffers;
    unsigned long freeHwMixingAllBuffers;
    unsigned long freeHwMixingStaticBuffers;
    unsigned long freeHwMixingStreamingBuffers;
    unsigned long maxHw3DAllBuffers;
    unsigned long maxHw3DStaticBuffers;
    unsigned long maxHw3DStreamingBuffers;
    unsigned long freeHw3DAllBuffers;
    unsigned long freeHw3DStaticBuffers;
    unsigned long freeHw3DStreamingBuffers;
    unsigned long totalHwMemBytes;
    unsigned long freeHwMemBytes;
    unsigned long maxContigFreeHwMemBytes;
    unsigned long unlockTransferRateHwBuffers;
    unsigned long playCpuOverheadSwBuffers;
    unsigned long reserved1;
    unsigned long reserved2;
};

// 0x24-byte buffer description (the DirectX 7 DSBUFFERDESC size).
struct UnknownSoundBufferDesc {
    unsigned long size;
    unsigned long flags;
    unsigned long bufferBytes;
    unsigned long reserved;
    struct UnknownWaveFormat* format;
    UnknownGuid algorithm3D;
};

// Sample format (WAVEFORMATEX layout; the first 16 bytes are PCMWAVEFORMAT).
struct UnknownWaveFormat {
    unsigned short formatTag;
    unsigned short channels;
    unsigned long samplesPerSec;
    unsigned long avgBytesPerSec;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    unsigned short extraSize;
};

// dsound.dll imports (thunks 0x005330a0 and 0x005330a6).
extern "C" long __stdcall DirectSoundCreate(const UnknownGuid* device, UnknownDirectSound** result, void* outer);
extern "C" long __stdcall DirectSoundEnumerateA(int(__stdcall* callback)(UnknownGuid*, const char*, const char*,
                                                                         void*),
                                                void* context);
extern "C" const UnknownGuid IID_IDirectSound3DListener; // 0x00556c90
// 0x00556d20: {4A4E6FC1-C341-11D1-B73A-444553540000}, the EAX 1.0 listener
// property set.
extern "C" const UnknownGuid DSPROPSETID_EAX_ListenerProperties;

// 0xf0-byte record of one enumerated sound device.
struct UnknownSoundDevice {
    UnknownSoundDevice(); // 0x004be280
    // 0x004be2d0: records `guid` and `description` and the device's caps;
    // 0 when `guid` is null or the device cannot be created.
    int UnknownFunction4be2d0(UnknownGuid* guid, const char* description);

    UnknownGuid guid;
    char description[0x80];
    UnknownSoundCaps caps;
};

// Defined in PCAudio.h.
class SoundGroup;
class UnknownPCAudioObject;

class PCSoundInterface : public SoundInterface {
public:
    PCSoundInterface();           // 0x004be370
    virtual ~PCSoundInterface();  // 0x004be4b0 (deleting wrapper 0x004be490)

    // 0x004be5a0: starts the audio (TrackGame slot 4 passes 22050, 1, 8 or
    // 16, 4000000 and a mode value); 0 or a DirectSound-style error.
    long UnknownFunction4be5a0(int rate, int stereo, int bits, int value, int allowEax);
    // 0x004be7b0: device enumeration callback; records up to four devices.
    static int __stdcall UnknownEnumCallback(UnknownGuid* guid, const char* description, const char* module,
                                             void* context);
    UnknownSoundListener* UnknownFunction4be800(); // 0x004be800: queries a new listener
    int UnknownFunction4be850();                   // 0x004be850: refreshes field_0x3fc
    int UnknownFunction4be860(UnknownSoundCaps* caps); // 0x004be860
    void UnknownFunction4be8b0();                  // 0x004be8b0: formats a caps report
    int UnknownFunction4be910(int rate, int stereo, int bits); // 0x004be910: primary format
    int UnknownFunction4be9b0(long volume);        // 0x004be9b0: primary volume
    void UnknownFunction4be9e0();                  // 0x004be9e0: EAX probe
    UnknownSoundBuffer* UnknownFunction4beb10();   // 0x004beb10: creates the primary buffer
    // 0x004beb80-0x004becd0: the listener's deferred settings.
    int UnknownFunction4beb80();
    int UnknownFunction4beba0(float factor);
    int UnknownFunction4bebd0(float factor);
    int UnknownFunction4bec00(float frontX, float frontY, float frontZ, float topX, float topY, float topZ);
    int UnknownFunction4bec50(float x, float y, float z);
    int UnknownFunction4bec90(float x, float y, float z);
    int UnknownFunction4becd0(float factor);
    // 0x004bed00 / 0x004bed40: EAX listener property 1 and 0 (all
    // parameters, 16 bytes).
    int UnknownFunction4bed00(unsigned long value);
    int UnknownFunction4bed40(void* parameters);

    int field_0x30;                       // enumerated device count
    UnknownSoundDevice* field_0x34;       // chosen device
    UnknownSoundDevice field_0x38[4];
    long field_0x3f8;                     // primary volume at start-up
    UnknownSoundCaps field_0x3fc;
    unsigned char field_0x45c_bit0 : 1;   // "AllowSoundHardware"
    unsigned char field_0x45c_bit1 : 1;   // "AllowSoundEnumeration"
    unsigned char field_0x45c_bit2 : 1;   // EAX listener available
    unsigned char field_0x45c_bit3 : 1;   // "AllowEAXExtension"
    UnknownDirectSound* field_0x460;
    UnknownSoundListener* field_0x464;
    UnknownSoundBuffer* field_0x468;      // primary buffer
    UnknownPCAudioObject* field_0x46c;
    SoundGroup* field_0x470;
    Sound* field_0x474;                   // EAX probe sound
};

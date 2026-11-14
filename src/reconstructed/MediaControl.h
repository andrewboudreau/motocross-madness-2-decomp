#pragma once

// MediaControl.h -- RTTI .?AVMediaControl@@ (vtable 0x005551c0): MediaControl
// : GameObject : BaseObject, single non-virtual at mdisp 0 (tier 1). It
// overrides slots 0 (deleting destructor 0x004a2470), 4..7, 10, 16, 17 and 18.
// UIVideoStatic (gameui.cpp line 0x264d) creates it with new 0x80.
//
// Code 0x004a23d0..0x004a2abf, after the Math3D helpers and their vector set
// (XCU 179-182) and before the allocation accounting code (0x004a2ac0). No
// __FILE__ literal: the file name 'MediaControl.cpp' and the member names
// are ours (tier 3).
//
// It plays a movie through DirectShow's multimedia streams: the GUIDs it
// passes are CLSID_FilterGraph / IID_IGraphBuilder (0x00558ee0/0x00558ef0,
// only to test that DirectShow is installed), CLSID_AMMultiMediaStream /
// IID_IAMMultiMediaStream (0x00558ec0/0x00558ed0), MSPID_PrimaryVideo /
// MSPID_PrimaryAudio (0x00558eb0/0x00558ea0), IID_IDirectDrawMediaStream
// (0x00558e90), IID_IDirectDraw (0x00556080) and IID_IDirectDrawSurface4
// (0x005560f0) (tier 1 by value). The interface method indices below follow
// the amstream.h / mmstream.h layouts; the interfaces are declared here by
// slot because the VC98 headers predate them.

#include "GameObject.h"
#include "Guid.h"
#include "RenderInterfaces.h"

class UIDialog;

// The DDSURFACEDESC layout (0x6c bytes), as IDirectDrawMediaStream::GetFormat
// fills it.
struct DDSurfaceDesc {
    unsigned long size;
    unsigned long flags;
    unsigned long height;
    unsigned long width;
    long pitch;                               // +0x10 lPitch (a union with dwLinearSize)
    unsigned long backBufferCount;            // +0x14 dwBackBufferCount
    unsigned long mipMapCount;                // +0x18 dwMipMapCount (a union with the z-buffer depth and refresh rate)
    unsigned long alphaBitDepth;              // +0x1c dwAlphaBitDepth
    unsigned long reserved;                   // +0x20 dwReserved
    void* surface;                            // +0x24 lpSurface
    unsigned char field_0x28[0x6c - 0x28];    // the four DDCOLORKEYs, ddpfPixelFormat (+0x48), ddsCaps (+0x68)
};

// IStreamSample / IDirectDrawStreamSample.
struct DirectDrawStreamSample {
    virtual long __stdcall QueryInterface(const Guid* iid, void** object);
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();
    virtual long __stdcall GetMediaStream();
    virtual long __stdcall GetSampleTimes();
    virtual long __stdcall SetSampleTimes();
    virtual long __stdcall Update(unsigned long flags, void* event, void* apc, unsigned long context);
    virtual long __stdcall CompletionStatus(unsigned long flags, unsigned long milliseconds);
};

// IMediaStream / IDirectDrawMediaStream.
struct DirectDrawMediaStream {
    virtual long __stdcall QueryInterface(const Guid* iid, void** object);
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();
    virtual long __stdcall GetMultiMediaStream();
    virtual long __stdcall GetInformation();
    virtual long __stdcall SetSameFormat();
    virtual long __stdcall AllocateSample();
    virtual long __stdcall CreateSharedSample();
    virtual long __stdcall SendEndOfStream();
    virtual long __stdcall GetFormat(DDSurfaceDesc* current, void* palette,
                                     DDSurfaceDesc* desired, unsigned long* flags);
    virtual long __stdcall SetFormat();
    virtual long __stdcall GetDirectDraw();
    virtual long __stdcall SetDirectDraw();
    virtual long __stdcall CreateSample(DirectDrawSurface* surface, const void* rect,
                                        unsigned long flags, DirectDrawStreamSample** sample);
};

// IMultiMediaStream / IAMMultiMediaStream.
struct AMMultiMediaStream {
    virtual long __stdcall QueryInterface(const Guid* iid, void** object);
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();
    virtual long __stdcall GetInformation();
    virtual long __stdcall GetMediaStream(const Guid* purpose, DirectDrawMediaStream** stream);
    virtual long __stdcall EnumMediaStreams();
    virtual long __stdcall GetState(int* state);
    virtual long __stdcall SetState(int state);
    virtual long __stdcall GetTime(__int64* time);
    virtual long __stdcall GetDuration(__int64* duration);
    virtual long __stdcall Seek(__int64 time);
    virtual long __stdcall GetEndOfStreamEventHandle();
    virtual long __stdcall Initialize(int type, unsigned long flags, void* graph);
    virtual long __stdcall GetFilterGraph();
    virtual long __stdcall GetFilter();
    virtual long __stdcall AddMediaStream(void* object, const Guid* purpose, unsigned long flags,
                                          DirectDrawMediaStream** stream);
    virtual long __stdcall OpenFile(const unsigned short* file, unsigned long flags);
};

class MediaControl : public GameObject {
public:
    MediaControl(int flags);                      // 0x004a2410
    virtual ~MediaControl();                  // 0x004a2490 (deleting wrapper 0x004a2470)
    // Slots 4..7: retail folds these with SoundGroup's identical bodies
    // (0x00401e20, 0x00401e40, 0x00401e60, 0x00401e80): the GameObject
    // version, then SetPaused (slot 16) with 1 for disable, 0 for enable.
    virtual void Disable();
    virtual void Enable();
    virtual void OnDisable();
    virtual void OnEnable();
    virtual int Tick(float frameTime);                 // 0x004a27f0: next frame, end of movie
    virtual int SetPaused(int value);            // 0x004a2970: pauses (value != 0) or resumes
    virtual int OnDeactivate();               // 0x004a28c0
    virtual int OnActivate();                 // 0x004a28e0

    // 0x004a23d0: 1 when DirectShow's filter graph can be created.
    static int IsDirectShowInstalled();
    // 0x004a2560: opens `file` for `target`; `done` is called with `owner`
    // at its end. Releases itself and returns 0 when it fails.
    MediaControl* Open(void* target, const char* file, void (*done)(UIDialog* dialog),
                                        UIDialog* owner);
    int Restart();              // 0x004a2900: restarts the movie
    void Stop();             // 0x004a2940: stops the movie
    int Seek(__int64 time);  // 0x004a29f0: seeks
    int SeekToStart();              // 0x004a2a10: seeks to the start
    int IsRunning();              // 0x004a2a20: 1 while running
    int GetPosition(__int64* time); // 0x004a2a50
    int GetDuration(__int64* duration); // 0x004a2a70
    // 0x004a2a90: blits the frame into `destination`.
    long BlitFrame(DirectDrawSurface* destination, void* destinationRect, void* sourceRect, int flags);

    DirectDrawSurface* frameSurface;    // +0x2c: the frame's surface (the display's DirectDraw)
    int frameWidth;                           // +0x30: the video format's width
    int frameHeight;                          // +0x34: and height
    DirectDrawSurface* sampleSurface;   // +0x38: the sample's surface (IDirectDrawSurface4)
    int frameRect[4];                         // +0x3c: the frame rectangle
    DirectDrawSurface* field_0x4c;      // released by the destructor only
    AMMultiMediaStream* multiMediaStream; // +0x50: IAMMultiMediaStream
    DirectDraw* directDraw;   // +0x54: IDirectDraw of the display's DirectDraw
    DirectDraw* displayDirectDraw; // +0x58: the display's DirectDraw (not released)
    DirectDrawMediaStream* videoStream; // +0x5c: the primary video stream
    DirectDrawMediaStream* videoDrawStream; // +0x60: its IDirectDrawMediaStream
    DirectDrawStreamSample* videoSample; // +0x64: the stream sample
    union {
        __int64 pausedTime;                   // +0x68: time saved by slot 16
        struct {
            unsigned long pausedTimeLow;
            long pausedTimeHigh;
        };
    };
    void (*doneCallback)(UIDialog* dialog);   // +0x70: called with doneOwner at the movie's end
    UIDialog* doneOwner;                      // +0x74
    unsigned char resumeAfterPause : 1;       // +0x78 bit 0: was running when slot 16 paused it
};

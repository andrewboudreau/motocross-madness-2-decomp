#pragma once

// MediaControl.h -- RTTI .?AVMediaControl@@ (vtable 0x005551c0): MediaControl
// : GameObject : BaseObject, single non-virtual at mdisp 0 (tier 1). It
// overrides slots 0 (deleting destructor 0x004a2470), 10, 16, 17 and 18.
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
struct UnknownMediaSurfaceDesc {
    unsigned long size;
    unsigned long flags;
    unsigned long height;
    unsigned long width;
    unsigned char field_0x10[0x6c - 0x10];
};

// IStreamSample / IDirectDrawStreamSample.
struct UnknownStreamSampleInterface {
    virtual long __stdcall QueryInterface(const UnknownGuid* iid, void** object);
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall Update(unsigned long flags, void* event, void* apc, unsigned long context);
    virtual long __stdcall CompletionStatus(unsigned long flags, unsigned long milliseconds);
};

// IMediaStream / IDirectDrawMediaStream.
struct UnknownMediaStreamInterface {
    virtual long __stdcall QueryInterface(const UnknownGuid* iid, void** object);
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall GetFormat(UnknownMediaSurfaceDesc* current, void* palette,
                                     UnknownMediaSurfaceDesc* desired, unsigned long* flags);
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11();
    virtual long __stdcall UnknownMethod12();
    virtual long __stdcall CreateSample(UnknownSurfaceInterface* surface, const void* rect,
                                        unsigned long flags, UnknownStreamSampleInterface** sample);
};

// IMultiMediaStream / IAMMultiMediaStream.
struct UnknownMultiMediaStreamInterface {
    virtual long __stdcall QueryInterface(const UnknownGuid* iid, void** object);
    virtual long __stdcall AddRef();
    virtual long __stdcall Release();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall GetMediaStream(const UnknownGuid* purpose, UnknownMediaStreamInterface** stream);
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall GetState(int* state);
    virtual long __stdcall SetState(int state);
    virtual long __stdcall GetTime(__int64* time);
    virtual long __stdcall GetDuration(__int64* duration);
    virtual long __stdcall Seek(__int64 time);
    virtual long __stdcall UnknownMethod11();
    virtual long __stdcall Initialize(int type, unsigned long flags, void* graph);
    virtual long __stdcall UnknownMethod13();
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall AddMediaStream(void* object, const UnknownGuid* purpose, unsigned long flags,
                                          UnknownMediaStreamInterface** stream);
    virtual long __stdcall OpenFile(const unsigned short* file, unsigned long flags);
};

class MediaControl : public GameObject {
public:
    MediaControl(int a);                      // 0x004a2410
    virtual ~MediaControl();                  // 0x004a2490 (deleting wrapper 0x004a2470)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004a27f0: next frame, end of movie
    virtual int UnknownVirtualSlot16(int value); // 0x004a2970: pauses (value != 0) or resumes
    virtual int UnknownVirtualSlot17();       // 0x004a28c0
    virtual int UnknownVirtualSlot18();       // 0x004a28e0

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
    long BlitFrame(UnknownSurfaceInterface* destination, void* destinationRect, void* sourceRect, int flags);

    UnknownSurfaceInterface* frameSurface;    // +0x2c: the frame's surface (the display's DirectDraw)
    int frameWidth;                           // +0x30: the video format's width
    int frameHeight;                          // +0x34: and height
    UnknownSurfaceInterface* sampleSurface;   // +0x38: the sample's surface (IDirectDrawSurface4)
    int frameRect[4];                         // +0x3c: the frame rectangle
    UnknownSurfaceInterface* field_0x4c;      // released by the destructor only
    UnknownMultiMediaStreamInterface* multiMediaStream; // +0x50: IAMMultiMediaStream
    UnknownDirectDrawInterface* directDraw;   // +0x54: IDirectDraw of the display's DirectDraw
    UnknownDirectDrawInterface* displayDirectDraw; // +0x58: the display's DirectDraw (not released)
    UnknownMediaStreamInterface* videoStream; // +0x5c: the primary video stream
    UnknownMediaStreamInterface* videoDrawStream; // +0x60: its IDirectDrawMediaStream
    UnknownStreamSampleInterface* videoSample; // +0x64: the stream sample
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

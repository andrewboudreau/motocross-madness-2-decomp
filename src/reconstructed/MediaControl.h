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
    virtual long __stdcall UnknownMethod0(const UnknownGuid* iid, void** object); // QueryInterface
    virtual long __stdcall UnknownMethod1();                              // AddRef
    virtual long __stdcall UnknownMethod2();                              // Release
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6(unsigned long flags, void* event, void* apc,
                                          unsigned long context);         // Update
    virtual long __stdcall UnknownMethod7(unsigned long flags, unsigned long milliseconds); // CompletionStatus
};

// IMediaStream / IDirectDrawMediaStream.
struct UnknownMediaStreamInterface {
    virtual long __stdcall UnknownMethod0(const UnknownGuid* iid, void** object); // QueryInterface
    virtual long __stdcall UnknownMethod1();                              // AddRef
    virtual long __stdcall UnknownMethod2();                              // Release
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9(UnknownMediaSurfaceDesc* current, void* palette,
                                          UnknownMediaSurfaceDesc* desired, unsigned long* flags); // GetFormat
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11();
    virtual long __stdcall UnknownMethod12();
    virtual long __stdcall UnknownMethod13(UnknownSurfaceInterface* surface, const void* rect,
                                           unsigned long flags,
                                           UnknownStreamSampleInterface** sample); // CreateSample
};

// IMultiMediaStream / IAMMultiMediaStream.
struct UnknownMultiMediaStreamInterface {
    virtual long __stdcall UnknownMethod0(const UnknownGuid* iid, void** object); // QueryInterface
    virtual long __stdcall UnknownMethod1();                              // AddRef
    virtual long __stdcall UnknownMethod2();                              // Release
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4(const UnknownGuid* purpose,
                                          UnknownMediaStreamInterface** stream); // GetMediaStream
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6(int* state);                   // GetState
    virtual long __stdcall UnknownMethod7(int state);                    // SetState
    virtual long __stdcall UnknownMethod8(__int64* time);                // GetTime
    virtual long __stdcall UnknownMethod9(__int64* duration);            // GetDuration
    virtual long __stdcall UnknownMethod10(__int64 time);                // Seek
    virtual long __stdcall UnknownMethod11();
    virtual long __stdcall UnknownMethod12(int type, unsigned long flags, void* graph); // Initialize
    virtual long __stdcall UnknownMethod13();
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall UnknownMethod15(void* object, const UnknownGuid* purpose, unsigned long flags,
                                           UnknownMediaStreamInterface** stream); // AddMediaStream
    virtual long __stdcall UnknownMethod16(const unsigned short* file, unsigned long flags); // OpenFile
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
    static int UnknownFunction4a23d0();
    // 0x004a2560: opens `file` for `target`; `done` is called with `owner`
    // at its end. Releases itself and returns 0 when it fails.
    MediaControl* UnknownFunction4a2560(void* target, const char* file, void (*done)(UIDialog* dialog),
                                        UIDialog* owner);
    int UnknownFunction4a2900();              // 0x004a2900: restarts the movie
    void UnknownFunction4a2940();             // 0x004a2940: stops the movie
    int UnknownFunction4a29f0(__int64 time);  // 0x004a29f0: seeks
    int UnknownFunction4a2a10();              // 0x004a2a10: seeks to the start
    int UnknownFunction4a2a20();              // 0x004a2a20: 1 while running
    int UnknownFunction4a2a50(__int64* time); // 0x004a2a50
    int UnknownFunction4a2a70(__int64* duration); // 0x004a2a70
    // 0x004a2a90: blits the frame into `destination`.
    long UnknownFunction4a2a90(UnknownSurfaceInterface* destination, void* destinationRect,
                               void* sourceRect, int flags);

    UnknownSurfaceInterface* field_0x2c;      // the frame's surface
    int field_0x30;                           // width
    int field_0x34;                           // height
    UnknownSurfaceInterface* field_0x38;      // the sample's surface
    int field_0x3c[4];                        // the frame rectangle
    UnknownSurfaceInterface* field_0x4c;      // released by the destructor only
    UnknownMultiMediaStreamInterface* field_0x50;
    UnknownDirectDrawInterface* field_0x54;
    UnknownDirectDrawInterface* field_0x58;   // the display's (not referenced)
    UnknownMediaStreamInterface* field_0x5c;  // the video stream
    UnknownMediaStreamInterface* field_0x60;  // its DirectDraw stream
    UnknownStreamSampleInterface* field_0x64;
    union {
        __int64 field_0x68;                   // time saved by slot 16
        struct {
            unsigned long field_0x68_low;
            long field_0x68_high;
        };
    };
    void (*field_0x70)(UIDialog* dialog);
    UIDialog* field_0x74;
    unsigned char field_0x78_bit0 : 1;        // was running when slot 16 paused it
};

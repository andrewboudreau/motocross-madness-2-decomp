// MediaControl.cpp -- see MediaControl.h for the evidence.

#include <windows.h>
#include <string.h>

#include "MediaControl.h"

#include "DebugAlloc.h"
#include "PCVideoCard.h"
#include "PCRenderTarget.h"

extern "C" const Guid CLSID_FilterGraph;           // 0x00558ee0
extern "C" const Guid IID_IGraphBuilder;           // 0x00558ef0
extern "C" const Guid CLSID_AMMultiMediaStream;    // 0x00558ec0
extern "C" const Guid IID_IAMMultiMediaStream;     // 0x00558ed0
extern "C" const Guid MSPID_PrimaryVideo;          // 0x00558eb0
extern "C" const Guid MSPID_PrimaryAudio;          // 0x00558ea0
extern "C" const Guid IID_IDirectDrawMediaStream;  // 0x00558e90
extern "C" const Guid IID_IDirectDraw;             // 0x00556080
extern "C" const Guid IID_IDirectDrawSurface4;     // 0x005560f0

#define TARGET() ((PCRenderTarget*)renderContext)

// 0x004a23d0
int MediaControl::IsDirectShowInstalled()
{
    DirectDrawSurface* graph;
    if (CoCreateInstance(*(const GUID*)&CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
                         *(const GUID*)&IID_IGraphBuilder, (void**)&graph) >= 0) {
        graph->Release();
        return 1;
    }
    return 0;
}

// 0x004a2410
MediaControl::MediaControl(int flags)
    : GameObject(flags)
{
    pausedTimeLow = 0;
    resumeAfterPause = 0;
    field_0x4c = 0;
    multiMediaStream = 0;
    videoStream = 0;
    videoDrawStream = 0;
    videoSample = 0;
    frameSurface = 0;
    sampleSurface = 0;
    frameWidth = 0;
    frameHeight = 0;
    directDraw = 0;
    displayDirectDraw = 0;
    doneCallback = 0;
    doneOwner = 0;
    pausedTimeHigh = 0;
}

// 0x004a2490
MediaControl::~MediaControl()
{
    Stop();
    if (frameSurface) {
        frameSurface->Release();
        frameSurface = 0;
    }
    if (sampleSurface) {
        sampleSurface->Release();
        sampleSurface = 0;
    }
    if (videoSample) {
        videoSample->Release();
        videoSample = 0;
    }
    if (videoDrawStream) {
        videoDrawStream->Release();
        videoDrawStream = 0;
    }
    if (videoStream) {
        videoStream->Release();
        videoStream = 0;
    }
    if (multiMediaStream) {
        multiMediaStream->Release();
        multiMediaStream = 0;
    }
    if (field_0x4c) {
        field_0x4c->Release();
        field_0x4c = 0;
    }
    if (directDraw) {
        directDraw->Release();
        directDraw = 0;
    }
}

// 0x004a2560. Not exact (53 of 646 bytes, 656 compiled: retail keeps a zero in
// ebx and &videoDrawStream in ebp after the DirectDraw stream QueryInterface and
// pushes ebx for the later zero arguments; VC6 pushes immediates and uses one
// register fewer, so the frame offsets differ by one dword;
// docs/MEDIACONTROL.md forms tried)
MediaControl* MediaControl::Open(void* target, const char* file,
                                                  void (*done)(UIDialog* dialog), UIDialog* owner)
{
    DirectDrawSurface* surface;
    DDSurfaceDesc2 frameDesc;
    DDSurfaceDesc format;
    unsigned short path[MAX_PATH];
    long result;

    GameObject::SetRenderContext(target);
    if (!IsDirectShowInstalled()) {
        goto failed;
    }
    displayDirectDraw = ((PCRenderTarget*)target)->targetDisplay->directDraw;
    doneCallback = done;
    doneOwner = owner;
    if (CoCreateInstance(*(const GUID*)&CLSID_AMMultiMediaStream, 0, CLSCTX_INPROC_SERVER,
                         *(const GUID*)&IID_IAMMultiMediaStream, (void**)&multiMediaStream)) {
        goto failed;
    }
    if (multiMediaStream->Initialize(0, 0, 0) < 0) {
        goto failed;
    }
    if (multiMediaStream->AddMediaStream(displayDirectDraw, &MSPID_PrimaryVideo, 0, 0) < 0) {
        goto failed;
    }
    result = multiMediaStream->AddMediaStream(0, &MSPID_PrimaryAudio, 1, 0);
    if (result != 0x80040256 && result < 0) {
        goto failed;
    }
    multiMediaStream->AddRef();
    MultiByteToWideChar(CP_ACP, 0, file, -1, path, MAX_PATH);
    if (multiMediaStream->OpenFile(path, 0) < 0) {
        goto failed;
    }
    if (multiMediaStream->GetMediaStream(&MSPID_PrimaryVideo, &videoStream) < 0) {
        goto failed;
    }
    if (videoStream->QueryInterface(&IID_IDirectDrawMediaStream, (void**)&videoDrawStream) < 0) {
        goto failed;
    }
    memset(&format, 0, sizeof(format));
    format.size = sizeof(format);
    if (videoDrawStream->GetFormat(&format, 0, 0, 0) < 0) {
        goto failed;
    }
    frameWidth = format.width;
    frameHeight = format.height;
    surface = 0;
    if (TARGET()->targetDisplay->directDraw->QueryInterface(&IID_IDirectDraw, (void**)&directDraw)) {
        goto failed;
    }
    if (directDraw->CreateSurface((DDSurfaceDesc2*)&format, &surface, 0)) {
        goto failed;
    }
    if (videoDrawStream->CreateSample(surface, 0, 1, &videoSample) < 0) {
        goto failed;
    }
    if (surface->QueryInterface(&IID_IDirectDrawSurface4, (void**)&sampleSurface)) {
        goto failed;
    }
    memset(&frameDesc, 0, sizeof(frameDesc));
    frameDesc.size = sizeof(frameDesc);
    if (TARGET()->renderSurface->GetSurfaceDesc(&frameDesc)) {
        goto failed;
    }
    frameDesc.height = frameHeight;
    frameDesc.width = frameWidth;
    frameDesc.flags = 0x1007;
    frameDesc.caps[0] = 0x2800;
    if (displayDirectDraw->CreateSurface(&frameDesc, &frameSurface, 0)) {
        goto failed;
    }
    frameRect[2] = frameWidth;
    frameRect[1] = 0;
    frameRect[0] = 0;
    frameRect[3] = frameHeight;
    return this;

failed:
    Release();
    return 0;
}

// 0x00401e20 (folded with SoundGroup::Disable)
void MediaControl::Disable()
{
    GameObject::Disable();
    SetPaused(1);
}

// 0x00401e40 (folded with SoundGroup::Enable)
void MediaControl::Enable()
{
    GameObject::Enable();
    SetPaused(0);
}

// 0x00401e60 (folded with SoundGroup::OnDisable)
void MediaControl::OnDisable()
{
    GameObject::OnDisable();
    SetPaused(1);
}

// 0x00401e80 (folded with SoundGroup::OnEnable)
void MediaControl::OnEnable()
{
    GameObject::OnEnable();
    SetPaused(0);
}

// 0x004a27f0
int MediaControl::Tick(float frameTime)
{
    __int64 time;
    __int64 duration;

    GameObject::Tick(frameTime);
    if (!videoSample) {
        return 0;
    }
    if (IsRunning()) {
        GetPosition(&time);
        GetDuration(&duration);
        if (time >= duration) {
            Stop();
            if (doneCallback) {
                doneCallback(doneOwner);
            }
        } else if (videoSample->CompletionStatus(2, 25) == 0) {
            BlitFrame(frameSurface, frameRect, 0, 0x1000000);
            if (videoSample->Update(1, 0, 0, 0) < 0) {
                return 0;
            }
        }
    }
    return 1;
}

// 0x004a28c0
int MediaControl::OnDeactivate()
{
    SetPaused(1);
    return GameObject::OnDeactivate();
}

// 0x004a28e0
int MediaControl::OnActivate()
{
    SetPaused(0);
    return GameObject::OnActivate();
}

// 0x004a2900. The break-out-of-do form places the shared `return 0` before
// `return 1`, as retail does (jl, then jge over it); the &&, nested-if,
// early-return, goto and result-flag forms all put `return 1` first.
int MediaControl::Restart()
{
    do {
        if (multiMediaStream->SetState(1) < 0) {
            break;
        }
        if (videoSample->Update(0, 0, 0, 0) < 0) {
            break;
        }
        return 1;
    } while (0);
    return 0;
}

// 0x004a2940
void MediaControl::Stop()
{
    if (IsRunning() && multiMediaStream->SetState(0) >= 0) {
        SeekToStart();
    }
}

// 0x004a2970
int MediaControl::SetPaused(int value)
{
    if (value) {
        if (!isPaused) {
            resumeAfterPause = IsRunning();
            GetPosition(&pausedTime);
            multiMediaStream->SetState(0);
        }
    } else if (resumeAfterPause) {
        Seek(pausedTime);
        multiMediaStream->SetState(1);
    }
    return GameObject::SetPaused(value);
}

// 0x004a29f0
int MediaControl::Seek(__int64 time)
{
    return multiMediaStream->Seek(time) >= 0;
}

// 0x004a2a10
int MediaControl::SeekToStart()
{
    return Seek(0);
}

// 0x004a2a20
int MediaControl::IsRunning()
{
    int state;
    if (multiMediaStream->GetState(&state) >= 0 && state == 1) {
        return 1;
    }
    return 0;
}

// 0x004a2a50
int MediaControl::GetPosition(__int64* time)
{
    return multiMediaStream->GetTime(time) >= 0;
}

// 0x004a2a70
int MediaControl::GetDuration(__int64* duration)
{
    return multiMediaStream->GetDuration(duration) >= 0;
}

// 0x004a2a90
long MediaControl::BlitFrame(DirectDrawSurface* destination, void* destinationRect,
                                         void* sourceRect, int flags)
{
    if (!sampleSurface) {
        return 0x80004005;
    }
    return destination->Blt(destinationRect, sampleSurface, sourceRect, flags, 0);
}

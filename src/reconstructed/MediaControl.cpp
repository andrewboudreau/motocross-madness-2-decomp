// MediaControl.cpp -- see MediaControl.h for the evidence.

#include <windows.h>

#include "MediaControl.h"

#include "DebugAlloc.h"

extern "C" const UnknownGuid CLSID_FilterGraph;           // 0x00558ee0
extern "C" const UnknownGuid IID_IGraphBuilder;           // 0x00558ef0

// 0x004a2560 (open) is a near miss: samples/gameui/MediaControlNearMisses.cpp.

// 0x004a23d0
int MediaControl::IsDirectShowInstalled()
{
    UnknownSurfaceInterface* graph;
    if (CoCreateInstance(*(const GUID*)&CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
                         *(const GUID*)&IID_IGraphBuilder, (void**)&graph) >= 0) {
        graph->Release();
        return 1;
    }
    return 0;
}

// 0x004a2410
MediaControl::MediaControl(int a)
    : GameObject(a)
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
long MediaControl::BlitFrame(UnknownSurfaceInterface* destination, void* destinationRect,
                                         void* sourceRect, int flags)
{
    if (!sampleSurface) {
        return 0x80004005;
    }
    return destination->Blt(destinationRect, sampleSurface, sourceRect, flags, 0);
}

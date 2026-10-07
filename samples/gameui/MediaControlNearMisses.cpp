// Near-miss MediaControl.cpp candidates (src/reconstructed/MediaControl.cpp,
// MediaControl.h), kept out of src/reconstructed until they match.
//
// 0x004a2560 (open, 646 bytes): every call, argument, GUID and branch target
// matches. Retail keeps a zero in ebx and &field_0x60 in ebp after the
// QueryInterface for the DirectDraw stream and pushes ebx for the later zero
// arguments; VC6 here pushes immediates and uses one register fewer, so the
// frame offsets differ by one dword. Local order does not change it.
//
// 0x004a2900 (restart, 50 bytes): retail tests SetState with `jl` to the
// shared `return 0` and Update with `jge` to a `return 1` placed after it;
// the &&, nested-if, early-return, ternary and result-flag forms all emit the
// two `jl` tests with `return 1` first.

#include <windows.h>
#include <string.h>

#include "../../src/reconstructed/MediaControl.h"

#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/PCRenderTarget.h"

extern "C" const UnknownGuid CLSID_AMMultiMediaStream;    // 0x00558ec0
extern "C" const UnknownGuid IID_IAMMultiMediaStream;     // 0x00558ed0
extern "C" const UnknownGuid MSPID_PrimaryVideo;          // 0x00558eb0
extern "C" const UnknownGuid MSPID_PrimaryAudio;          // 0x00558ea0
extern "C" const UnknownGuid IID_IDirectDrawMediaStream;  // 0x00558e90
extern "C" const UnknownGuid IID_IDirectDraw;             // 0x00556080
extern "C" const UnknownGuid IID_IDirectDrawSurface4;     // 0x005560f0

#define TARGET() ((PCRenderTarget*)field_0x18)

// 0x004a2560
MediaControl* MediaControl::UnknownFunction4a2560(void* target, const char* file,
                                                  void (*done)(UIDialog* dialog), UIDialog* owner)
{
    UnknownSurfaceInterface* surface;
    UnknownSurfaceDesc frameDesc;
    UnknownMediaSurfaceDesc format;
    unsigned short path[MAX_PATH];
    long result;

    GameObject::UnknownVirtualSlot8(target);
    if (!UnknownFunction4a23d0()) {
        goto failed;
    }
    field_0x58 = ((PCRenderTarget*)target)->field_0x04->directDraw;
    field_0x70 = done;
    field_0x74 = owner;
    if (CoCreateInstance(*(const GUID*)&CLSID_AMMultiMediaStream, 0, CLSCTX_INPROC_SERVER,
                         *(const GUID*)&IID_IAMMultiMediaStream, (void**)&field_0x50)) {
        goto failed;
    }
    if (field_0x50->UnknownMethod12(0, 0, 0) < 0) {
        goto failed;
    }
    if (field_0x50->UnknownMethod15(field_0x58, &MSPID_PrimaryVideo, 0, 0) < 0) {
        goto failed;
    }
    result = field_0x50->UnknownMethod15(0, &MSPID_PrimaryAudio, 1, 0);
    if (result != 0x80040256 && result < 0) {
        goto failed;
    }
    field_0x50->UnknownMethod1();
    MultiByteToWideChar(CP_ACP, 0, file, -1, path, MAX_PATH);
    if (field_0x50->UnknownMethod16(path, 0) < 0) {
        goto failed;
    }
    if (field_0x50->UnknownMethod4(&MSPID_PrimaryVideo, &field_0x5c) < 0) {
        goto failed;
    }
    if (field_0x5c->UnknownMethod0(&IID_IDirectDrawMediaStream, (void**)&field_0x60) < 0) {
        goto failed;
    }
    memset(&format, 0, sizeof(format));
    format.size = sizeof(format);
    if (field_0x60->UnknownMethod9(&format, 0, 0, 0) < 0) {
        goto failed;
    }
    field_0x30 = format.width;
    field_0x34 = format.height;
    surface = 0;
    if (TARGET()->field_0x04->directDraw->QueryInterface(&IID_IDirectDraw, (void**)&field_0x54)) {
        goto failed;
    }
    if (field_0x54->CreateSurface((UnknownSurfaceDesc*)&format, &surface, 0)) {
        goto failed;
    }
    if (field_0x60->UnknownMethod13(surface, 0, 1, &field_0x64) < 0) {
        goto failed;
    }
    if (surface->QueryInterface(&IID_IDirectDrawSurface4, (void**)&field_0x38)) {
        goto failed;
    }
    memset(&frameDesc, 0, sizeof(frameDesc));
    frameDesc.size = sizeof(frameDesc);
    if (TARGET()->renderSurface->GetSurfaceDesc(&frameDesc)) {
        goto failed;
    }
    frameDesc.height = field_0x34;
    frameDesc.width = field_0x30;
    frameDesc.flags = 0x1007;
    frameDesc.caps[0] = 0x2800;
    if (field_0x58->CreateSurface(&frameDesc, &field_0x2c, 0)) {
        goto failed;
    }
    field_0x3c[2] = field_0x30;
    field_0x3c[1] = 0;
    field_0x3c[0] = 0;
    field_0x3c[3] = field_0x34;
    return this;

failed:
    Release();
    return 0;
}

// 0x004a2900
int MediaControl::Restart()
{
    if (field_0x50->UnknownMethod7(1) >= 0 && field_0x64->UnknownMethod6(0, 0, 0, 0) >= 0) {
        return 1;
    }
    return 0;
}

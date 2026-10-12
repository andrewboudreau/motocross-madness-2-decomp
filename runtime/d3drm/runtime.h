/* Shared declarations of the d3drm.dll runtime module. */
#ifndef MCM2_RUNTIME_H
#define MCM2_RUNTIME_H

/* Return address of a __stdcall function, read just below its first argument. */
#define CALLER_OF(firstArgument) (((DWORD*)&(firstArgument))[-1])

void LogLine(const char* format, ...);
void InstallStartupTrace(void);
void TraceDirectDrawObject(void* object);
void TraceInnerDirectDrawObject(void* object);
void TraceDirectInputObject(void* object);
void InstallSampler(void);

/* present.c: emulated 16-bit full screen. */
int EmulatesFullscreen(void);
DWORD EmulatedCooperativeLevel(HWND window, DWORD flags);
void SetEmulatedDisplayMode(DWORD width, DWORD height, DWORD depth);
void GetEmulatedDisplayMode(DWORD* desc);
void MakeSixteenBitMode(DWORD* desc);
void InjectDisplayFormat(DWORD* desc);
void NameSurfaceType(DWORD* desc);
void* RealSurface(void* surface);
void* CurrentEmulatedPrimary(void);
void MapCursorToGame(POINT* point);
LONG PresentedFrames(void);
HRESULT CreateEmulatedPrimary(void* directDraw, const DWORD* desc, void** out,
                              HRESULT(__stdcall* create)(void*, void*, void**, void*));

#endif

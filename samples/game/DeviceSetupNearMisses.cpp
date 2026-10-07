// DeviceSetupNearMisses.cpp -- near miss for the DirectX probe 0x00448560
// (1011 bytes; this candidate is 1006), the first function of the DeviceSetup.cpp unit
// (src/reconstructed/DeviceSetup.cpp, docs/DEVICESETUP.md).
//
// Status: same instruction sequence, branch layout and frame layout except
// for one register swap (18% masked). Retail keeps the zero constant in edi,
// the LoadLibrary import pointer in ebp and GetProcAddress in edi once the
// zero is dead; VC6 here swaps the two, which spills DirectInputCreate to
// the stack and grows the frame by four bytes. Tried: dropping the dead NULL
// initialisers, separate NT-path library variables, the NT path reusing the
// 9x path's DIHinst/DirectInputCreate (or DDHinst), and the SDK's DX7 tail
// (`pDD7 = NULL`, `DirectDrawCreateEx = NULL`, nested ifs, `hr`): all give
// the same 1006 bytes, or 1014 with the SDK tail.
//
// It is the DirectX SDK's GetDXVersion sample (GetDXVer.cpp) changed to
// report whether "Blade.dll" loads (on NT 5 and in place of DDRAW.DLL on
// Windows 9x), to free the input library before testing its entry point and
// to report 0x501 for builds after 0x549.

#include <windows.h>
#include <ddraw.h>

#include "../../src/reconstructed/DeviceSetup.h"

// The VC6 DDRAW.H is the DirectX 5 header; the two later interface IDs are
// declared here (dxguid; retail bytes at the commented addresses).
extern "C" const GUID IID_IDirectDrawSurface4; // 0x005560f0
extern "C" const GUID IID_IDirectDraw7;        // 0x005560b0

typedef HRESULT(WINAPI* DIRECTDRAWCREATE)(GUID*, LPDIRECTDRAW*, IUnknown*);
typedef HRESULT(WINAPI* DIRECTDRAWCREATEEX)(GUID*, VOID**, REFIID, IUnknown*);
typedef HRESULT(WINAPI* DIRECTINPUTCREATE)(HINSTANCE, DWORD, VOID**, IUnknown*);

// 0x00448560
void GetDXVersion(DWORD* pdwDXVersion, DWORD* pdwDXPlatform, DWORD* pdwBlade)
{
    HRESULT hr;
    HINSTANCE DDHinst;
    HINSTANCE DIHinst;
    LPDIRECTDRAW pDDraw = 0;
    LPDIRECTDRAW2 pDDraw2 = 0;
    DIRECTDRAWCREATE DirectDrawCreate;
    DIRECTINPUTCREATE DirectInputCreate;
    OSVERSIONINFO osVer;
    LPDIRECTDRAWSURFACE pSurf = NULL;
    LPDIRECTDRAWSURFACE3 pSurf3 = NULL;
    IUnknown* pSurf4 = NULL;

    // First get the windows platform
    osVer.dwOSVersionInfoSize = sizeof(osVer);
    if (!GetVersionEx(&osVer)) {
        *pdwDXVersion = 0;
        *pdwDXPlatform = 0;
        return;
    }

    if (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        *pdwDXPlatform = VER_PLATFORM_WIN32_NT;
        // NT 4.0 is DX2, 4.0 SP3 is DX3; no DX on earlier versions.
        if (osVer.dwMajorVersion < 4) {
            *pdwDXPlatform = 0;
            return;
        }

        if (osVer.dwMajorVersion == 4) {
            *pdwDXVersion = 0x200;

            // SP3 and later have DirectInput.
            HINSTANCE hDI = LoadLibrary("DINPUT.DLL");
            if (hDI == 0)
                return;

            DIRECTINPUTCREATE pfnCreate =
                (DIRECTINPUTCREATE)GetProcAddress(hDI, "DirectInputCreateA");
            FreeLibrary(hDI);

            if (pfnCreate == 0)
                return;

            *pdwDXVersion = 0x300;
            return;
        }

        // Windows 2000 and later
        *pdwDXVersion = 0x700;
        DDHinst = LoadLibrary("Blade.dll");
        if (DDHinst == 0) {
            *pdwBlade = 0;
            return;
        }
        *pdwBlade = 1;
        FreeLibrary(DDHinst);
        return;
    }

    // Windows 9x
    *pdwDXPlatform = VER_PLATFORM_WIN32_WINDOWS;

    DDHinst = LoadLibrary("Blade.dll");
    *pdwBlade = 1;
    if (DDHinst == 0) {
        *pdwBlade = 0;
        DDHinst = LoadLibrary("DDRAW.DLL");
        if (DDHinst == 0) {
            *pdwDXVersion = 0;
            *pdwDXPlatform = 0;
            return;
        }
    }

    DirectDrawCreate = (DIRECTDRAWCREATE)GetProcAddress(DDHinst, "DirectDrawCreate");
    if (DirectDrawCreate == 0) {
        *pdwDXVersion = 0;
        *pdwDXPlatform = 0;
        FreeLibrary(DDHinst);
        return;
    }

    hr = DirectDrawCreate(NULL, &pDDraw, NULL);
    if (FAILED(hr)) {
        *pdwDXVersion = 0;
        *pdwDXPlatform = 0;
        FreeLibrary(DDHinst);
        return;
    }

    // DirectDraw exists: at least DX1.
    *pdwDXVersion = 0x100;

    hr = pDDraw->QueryInterface(IID_IDirectDraw2, (VOID**)&pDDraw2);
    if (FAILED(hr)) {
        pDDraw->Release();
        FreeLibrary(DDHinst);
        return;
    }

    // IDirectDraw2: at least DX2.
    pDDraw2->Release();
    *pdwDXVersion = 0x200;

    // DirectInput was added for DX3.
    DIHinst = LoadLibrary("DINPUT.DLL");
    if (DIHinst == 0) {
        pDDraw->Release();
        FreeLibrary(DDHinst);
        return;
    }

    DirectInputCreate = (DIRECTINPUTCREATE)GetProcAddress(DIHinst, "DirectInputCreateA");
    FreeLibrary(DIHinst);
    if (DirectInputCreate == 0) {
        FreeLibrary(DDHinst);
        pDDraw->Release();
        return;
    }

    *pdwDXVersion = 0x300;

    // IDirectDrawSurface3 means DX5; it needs a primary surface to QI.
    DDSURFACEDESC ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hr = pDDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);
    if (FAILED(hr)) {
        pDDraw->Release();
        FreeLibrary(DDHinst);
        *pdwDXVersion = 0;
        return;
    }

    hr = pDDraw->CreateSurface(&ddsd, &pSurf, NULL);
    if (FAILED(hr)) {
        pDDraw->Release();
        FreeLibrary(DDHinst);
        *pdwDXVersion = 0;
        return;
    }

    if (FAILED(pSurf->QueryInterface(IID_IDirectDrawSurface3, (VOID**)&pSurf3))) {
        pSurf->Release();
        pDDraw->Release();
        FreeLibrary(DDHinst);
        return;
    }

    *pdwDXVersion = 0x500;
    if (LOWORD(osVer.dwBuildNumber) > 0x549)
        *pdwDXVersion = 0x501;

    // IDirectDrawSurface4 arrived with DX6.
    if (FAILED(pSurf->QueryInterface(IID_IDirectDrawSurface4, (VOID**)&pSurf4))) {
        pSurf->Release();
        pDDraw->Release();
        FreeLibrary(DDHinst);
        return;
    }

    *pdwDXVersion = 0x600;
    pSurf->Release();
    pDDraw->Release();

    // DX7 if a DirectDraw7 object can be created.
    IUnknown* pDD7;
    DIRECTDRAWCREATEEX DirectDrawCreateEx =
        (DIRECTDRAWCREATEEX)GetProcAddress(DDHinst, "DirectDrawCreateEx");
    if (DirectDrawCreateEx != NULL &&
        SUCCEEDED(DirectDrawCreateEx(NULL, (VOID**)&pDD7, IID_IDirectDraw7, NULL))) {
        *pdwDXVersion = 0x700;
        pDD7->Release();
    }

    FreeLibrary(DDHinst);
}

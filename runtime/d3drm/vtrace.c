/*
 * DirectDraw 7 / Direct3D 7 call tracing by vtable patching.
 *
 * TraceDirectDrawObject is called with every IDirectDraw7 the game gets from
 * blade.dll. The first time a vtable is seen, the slots below are replaced
 * with logging wrappers; the originals are kept per vtable, since Blade's
 * software objects and the system DirectDraw objects have different ones.
 * Slot numbers follow the DirectX 7 SDK declaration order (see
 * src/reconstructed/RenderInterfaces.h).
 */
#include <windows.h>
#include <string.h>

#include "runtime.h"

#define MAX_VTABLES 8
#define MAX_SLOTS 32

typedef struct PatchedVtable {
    void** vtable;
    void* original[MAX_SLOTS];
} PatchedVtable;

static PatchedVtable s_directDraw[MAX_VTABLES];
static PatchedVtable s_direct3D[MAX_VTABLES];
static PatchedVtable s_surface[MAX_VTABLES];
static PatchedVtable s_innerDirectDraw[MAX_VTABLES];

static void* Original(PatchedVtable* table, void* self, int slot)
{
    void** vtable = *(void***)self;
    int i;
    for (i = 0; i < MAX_VTABLES; i++)
        if (table[i].vtable == vtable)
            return table[i].original[slot];
    return NULL;
}

/* Windows applies apphelp shims to mcm2.exe that rewrite some of ddraw's
   vtable slots in memory; those wrappers are logged and kept. */
/* True when `address` lies inside the module image that holds `vtable`. */
static int SameModule(void** vtable, void* address)
{
    MEMORY_BASIC_INFORMATION a, b;
    return VirtualQuery(vtable, &a, sizeof(a)) && VirtualQuery(address, &b, sizeof(b)) &&
           a.AllocationBase == b.AllocationBase;
}

static void PatchSlots(PatchedVtable* table, void* self, const int* slots, void* const* wrappers,
                       int count, const char* kind)
{
    void** vtable = *(void***)self;
    DWORD oldProtect;
    int i;

    for (i = 0; i < MAX_VTABLES && table[i].vtable; i++)
        if (table[i].vtable == vtable)
            return;
    if (i == MAX_VTABLES)
        return;
    table[i].vtable = vtable;
    VirtualProtect(vtable, MAX_SLOTS * sizeof(void*), PAGE_READWRITE, &oldProtect);
    for (count--; count >= 0; count--) {
        void* current = vtable[slots[count]];
        if (!SameModule(vtable, current))
            LogLine("vtrace: %s slot %d is already redirected to %p", kind, slots[count],
                    current);
        table[i].original[slots[count]] = current;
        vtable[slots[count]] = wrappers[count];
    }
    VirtualProtect(vtable, MAX_SLOTS * sizeof(void*), oldProtect, &oldProtect);
    LogLine("vtrace: patched %s vtable %p", kind, vtable);
}

static const char* IidName(const GUID* iid)
{
    static const GUID kDirect3D7 = {0xf5049e77, 0x4861, 0x11d2,
                                    {0xa4, 0x07, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0xa8}};
    static const GUID kDirectDraw7 = {0x15e65ec0, 0x3b9c, 0x11d2,
                                      {0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b}};
    if (!memcmp(iid, &kDirect3D7, sizeof(GUID)))
        return "IID_IDirect3D7";
    if (!memcmp(iid, &kDirectDraw7, sizeof(GUID)))
        return "IID_IDirectDraw7";
    return "other";
}

static void FormatGuid(char* out, const GUID* guid)
{
    if (!guid) {
        lstrcpyA(out, "NULL");
        return;
    }
    wsprintfA(out, "{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", guid->Data1,
              guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2],
              guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

/* ---- IDirectDrawSurface7 (primary) ---- */

typedef HRESULT(__stdcall* FlipProc)(void*, void*, DWORD);
typedef HRESULT(__stdcall* LockProc)(void*, RECT*, DWORD*, DWORD, HANDLE);
typedef HRESULT(__stdcall* UnlockProc)(void*, RECT*);

#define SURFACE_SLOT(object, slot) ((*(void***)(object))[slot])

static void* s_primary;
static DWORD s_traceStart;
enum { FLIP, BLT, BLTFAST, GETDC, ISLOST, RESTORE, KINDS };
static const char* const kKindNames[KINDS] = {"Flip", "Blt", "BltFast", "GetDC", "IsLost",
                                              "Restore"};
static LONG s_calls[KINDS];
static LONG s_toPrimary[KINDS];
static LONG s_failed[KINDS];
static int s_nextDump;
static DWORD s_nextExtraDump;
static DWORD s_lastTriggerCheck;
static DWORD s_nextSummary;

/* Seconds after the primary surface exists at which the front buffer is saved
   to frames\ next to the executable. */
static const DWORD kDumpSeconds[] = {3, 6, 9, 12, 15, 20, 25, 30, 40, 50, 60};

/* Writes a locked 16-bit surface (DDSURFACEDESC2 in desc) as a 24-bit BMP. */
static void WriteFrame(const DWORD* desc, DWORD seconds)
{
    char path[MAX_PATH];
    char* slash;
    BITMAPFILEHEADER file;
    BITMAPINFOHEADER info;
    DWORD width = desc[3], height = desc[2], pitch = desc[4], depth = desc[21];
    DWORD rMask = desc[22], gMask = desc[23], bMask = desc[24];
    const BYTE* bits = (const BYTE*)desc[9];
    DWORD rowBytes = (width * 3 + 3) & ~3u;
    static BYTE row[3 * 4096];
    HANDLE out;
    DWORD written;
    long y;

    if (depth != 16 || width > 4096 || !bits || !rMask || !gMask || !bMask) {
        LogLine("frame %lus: %lux%lu %lu bpp not dumped", seconds, width, height, depth);
        return;
    }
    GetModuleFileNameA(NULL, path, sizeof(path));
    slash = strrchr(path, '\\');
    lstrcpyA(slash ? slash + 1 : path, "frames");
    CreateDirectoryA(path, NULL);
    wsprintfA(path + lstrlenA(path), "\\frame_%03lus.bmp", seconds);
    out = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (out == INVALID_HANDLE_VALUE)
        return;
    memset(&file, 0, sizeof(file));
    memset(&info, 0, sizeof(info));
    file.bfType = 0x4d42;
    file.bfOffBits = sizeof(file) + sizeof(info);
    file.bfSize = file.bfOffBits + rowBytes * height;
    info.biSize = sizeof(info);
    info.biWidth = width;
    info.biHeight = height;
    info.biPlanes = 1;
    info.biBitCount = 24;
    WriteFile(out, &file, sizeof(file), &written, NULL);
    WriteFile(out, &info, sizeof(info), &written, NULL);
    for (y = height - 1; y >= 0; y--) {
        const WORD* src = (const WORD*)(bits + y * pitch);
        DWORD x;
        memset(row, 0, rowBytes);
        for (x = 0; x < width; x++) {
            DWORD p = src[x];
            /* 565 or 555: scale each field to 8 bits. */
            row[x * 3] = (BYTE)((p & bMask) * 255 / bMask);
            row[x * 3 + 1] = (BYTE)((p & gMask) * 255 / gMask);
            row[x * 3 + 2] = (BYTE)((p & rMask) * 255 / rMask);
        }
        WriteFile(out, row, rowBytes, &written, NULL);
    }
    CloseHandle(out);
    LogLine("frame %lus: wrote %s", seconds, path);
}

static void DumpFrontBuffer(DWORD seconds)
{
    DWORD desc[31];
    HRESULT result;
    memset(desc, 0, sizeof(desc));
    desc[0] = sizeof(desc);
    result = ((LockProc)SURFACE_SLOT(s_primary, 25))(s_primary, NULL, desc, 0x11, NULL);
    if (result) {
        LogLine("frame %lus: Lock -> 0x%08lx", seconds, result);
        return;
    }
    WriteFrame(desc, seconds);
    ((UnlockProc)SURFACE_SLOT(s_primary, 32))(s_primary, NULL);
}

/* Counts a surface call. Every 5 seconds logs the counts so far; on the dump
   schedule also saves the front buffer. */
static void CountCall(int kind, void* self, HRESULT result)
{
    char line[400];
    DWORD seconds;
    int i, length = 0;
    InterlockedIncrement(&s_calls[kind]);
    if (self == s_primary)
        InterlockedIncrement(&s_toPrimary[kind]);
    if (result)
        InterlockedIncrement(&s_failed[kind]);
    if (!s_primary)
        return;
    seconds = (GetTickCount() - s_traceStart) / 1000;
    if (GetTickCount() - s_lastTriggerCheck >= 500) {
        /* A file named "dumpframe" next to the executable saves the current
           front buffer as frames\frame_NNNs.bmp and is then removed. */
        char trigger[MAX_PATH];
        char* slash;
        s_lastTriggerCheck = GetTickCount();
        GetModuleFileNameA(NULL, trigger, sizeof(trigger));
        slash = strrchr(trigger, '\\');
        lstrcpyA(slash ? slash + 1 : trigger, "dumpframe");
        if (GetFileAttributesA(trigger) != 0xFFFFFFFFUL) {
            DeleteFileA(trigger);
            DumpFrontBuffer(seconds);
        }
    }
    if (seconds < s_nextSummary)
        return;
    s_nextSummary = seconds + 5;
    for (i = 0; i < KINDS; i++)
        length += wsprintfA(line + length, "%s %ld (primary %ld, failed %ld)%s", kKindNames[i],
                            s_calls[i], s_toPrimary[i], s_failed[i], i + 1 < KINDS ? ", " : "");
    LogLine("surface calls by %lus: %s; %ld frames presented", seconds, line, PresentedFrames());
    if (s_nextDump < sizeof(kDumpSeconds) / sizeof(kDumpSeconds[0]) &&
        seconds >= kDumpSeconds[s_nextDump]) {
        while (s_nextDump < sizeof(kDumpSeconds) / sizeof(kDumpSeconds[0]) &&
               seconds >= kDumpSeconds[s_nextDump])
            s_nextDump++;
        DumpFrontBuffer(seconds);
    } else if (seconds > 60 && seconds <= 900 && seconds >= s_nextExtraDump) {
        /* Past the fixed table, keep a frame every 10 seconds (15 minutes). */
        s_nextExtraDump = seconds + 10;
        DumpFrontBuffer(seconds);
    }
}

typedef HRESULT(__stdcall* BltProc)(void*, RECT*, void*, RECT*, DWORD, void*);
typedef HRESULT(__stdcall* BltFastProc)(void*, DWORD, DWORD, void*, RECT*, DWORD);
typedef HRESULT(__stdcall* GetDCProc)(void*, HDC*);

typedef HRESULT(__stdcall* GetSurfaceDescProc)(void*, DWORD*);

/* Logs the first failures of each kind, with both surfaces' formats. */
static void LogFailure(const char* call, void* self, void* src, HRESULT result, DWORD caller)
{
    static LONG s_failures;
    DWORD a[31], b[31];
    if (InterlockedIncrement(&s_failures) > 24)
        return;
    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    a[0] = b[0] = sizeof(a);
    ((GetSurfaceDescProc)SURFACE_SLOT(self, 22))(self, a);
    if (src)
        ((GetSurfaceDescProc)SURFACE_SLOT(src, 22))(src, b);
    LogLine("%s from 0x%08lx failed 0x%08lx: dst %p %lux%lu %lubpp caps 0x%lx%s, src %p %lux%lu "
            "%lubpp caps 0x%lx", call, caller, result, self, a[3], a[2], a[21], a[26],
            self == s_primary ? " (primary)" : "", src, b[3], b[2], b[21], b[26]);
}

static HRESULT __stdcall SurfaceFlip(void* self, void* target, DWORD flags)
{
    HRESULT result = ((FlipProc)Original(s_surface, self, 11))(self, target, flags);
    if (result)
        LogFailure("Flip", self, NULL, result, CALLER_OF(self));
    CountCall(FLIP, self, result);
    return result;
}

static HRESULT __stdcall SurfaceBlt(void* self, RECT* dst, void* src, RECT* srcRect, DWORD flags,
                                    void* fx)
{
    HRESULT result =
        ((BltProc)Original(s_surface, self, 5))(self, dst, RealSurface(src), srcRect, flags, fx);
    if (result)
        LogFailure("Blt", self, src, result, CALLER_OF(self));
    CountCall(BLT, self, result);
    return result;
}

static HRESULT __stdcall SurfaceBltFast(void* self, DWORD x, DWORD y, void* src, RECT* srcRect,
                                        DWORD flags)
{
    HRESULT result =
        ((BltFastProc)Original(s_surface, self, 7))(self, x, y, RealSurface(src), srcRect, flags);
    if (result)
        LogFailure("BltFast", self, src, result, CALLER_OF(self));
    CountCall(BLTFAST, self, result);
    return result;
}

static HRESULT __stdcall SurfaceGetDC(void* self, HDC* dc)
{
    HRESULT result = ((GetDCProc)Original(s_surface, self, 17))(self, dc);
    CountCall(GETDC, self, result);
    return result;
}

typedef HRESULT(__stdcall* NoArgumentProc)(void*);

static HRESULT __stdcall SurfaceIsLost(void* self)
{
    HRESULT result = ((NoArgumentProc)Original(s_surface, self, 24))(self);
    CountCall(ISLOST, self, result);
    return result;
}

static HRESULT __stdcall SurfaceRestore(void* self)
{
    HRESULT result = ((NoArgumentProc)Original(s_surface, self, 27))(self);
    LogLine("IDirectDrawSurface7(%p%s)::Restore from 0x%08lx -> 0x%08lx", self,
            self == s_primary ? ", primary" : "", CALLER_OF(self), result);
    CountCall(RESTORE, self, result);
    return result;
}

static void TracePrimarySurface(void* surface)
{
    static const int slots[] = {5, 7, 11, 17, 24, 27};
    static void* const wrappers[] = {SurfaceBlt,   SurfaceBltFast, SurfaceFlip,
                                     SurfaceGetDC, SurfaceIsLost,  SurfaceRestore};
    s_primary = surface;
    if (surface == CurrentEmulatedPrimary())
        surface = RealSurface(surface);
    s_traceStart = GetTickCount();
    s_nextDump = 0;
    s_nextExtraDump = 0;
    s_nextSummary = 0;
    PatchSlots(s_surface, surface, slots, wrappers, 6, "IDirectDrawSurface7");
}

/* ---- IDirect3D7 ---- */

typedef HRESULT(__stdcall* CreateDeviceProc)(void*, const GUID*, void*, void**);
typedef HRESULT(__stdcall* CreateVertexBufferProc)(void*, void*, void**, DWORD);
typedef HRESULT(__stdcall* EnumDevicesProc)(void*, void*, void*);

static HRESULT __stdcall D3DEnumDevices(void* self, void* callback, void* context)
{
    HRESULT result = ((EnumDevicesProc)Original(s_direct3D, self, 3))(self, callback, context);
    LogLine("IDirect3D7(%p)::EnumDevices -> 0x%08lx", self, result);
    return result;
}

static HRESULT __stdcall D3DCreateDevice(void* self, const GUID* clsid, void* surface, void** out)
{
    char guid[64];
    HRESULT result =
        ((CreateDeviceProc)Original(s_direct3D, self, 4))(self, clsid, surface, out);
    FormatGuid(guid, clsid);
    LogLine("IDirect3D7(%p)::CreateDevice(%s, surface %p) -> 0x%08lx", self, guid, surface,
            result);
    return result;
}

static HRESULT __stdcall D3DCreateVertexBuffer(void* self, void* desc, void** out, DWORD flags)
{
    HRESULT result =
        ((CreateVertexBufferProc)Original(s_direct3D, self, 5))(self, desc, out, flags);
    DWORD* d = (DWORD*)desc;
    LogLine("IDirect3D7(%p)::CreateVertexBuffer(caps 0x%lx, fvf 0x%lx, %lu vertices) -> 0x%08lx",
            self, d[2], d[3], d[4], result);
    return result;
}

static void TraceDirect3DObject(void* object)
{
    static const int slots[] = {3, 4, 5};
    static void* const wrappers[] = {D3DEnumDevices, D3DCreateDevice, D3DCreateVertexBuffer};
    PatchSlots(s_direct3D, object, slots, wrappers, 3, "IDirect3D7");
}

/* ---- IDirectDraw7 ---- */

typedef HRESULT(__stdcall* QueryInterfaceProc)(void*, const GUID*, void**);
typedef HRESULT(__stdcall* CreateSurfaceProc)(void*, void*, void**, void*);
typedef HRESULT(__stdcall* GetCapsProc)(void*, void*, void*);
typedef HRESULT(__stdcall* SetCooperativeLevelProc)(void*, HWND, DWORD);
typedef HRESULT(__stdcall* SetDisplayModeProc)(void*, DWORD, DWORD, DWORD, DWORD, DWORD);
typedef HRESULT(__stdcall* GetAvailableVidMemProc)(void*, void*, DWORD*, DWORD*);
typedef HRESULT(__stdcall* EnumModesCallback)(void*, void*);
typedef HRESULT(__stdcall* EnumDisplayModesProc)(void*, DWORD, void*, void*, EnumModesCallback);

typedef HRESULT(__stdcall* GetDisplayModeProc)(void*, DWORD*);
typedef HRESULT(__stdcall* GetGDISurfaceProc)(void*, void**);

typedef struct ModeCount {
    EnumModesCallback callback;
    void* context;
    int byDepth[33];
    int addTwins;
    int addEights;
} ModeCount;

/* DDSURFACEDESC2 pixel format dwRGBBitCount is DWORD 21. With emulated full
   screen, every 32-bit mode is followed by its 16-bit twin. */
static HRESULT __stdcall CountMode(void* desc, void* context)
{
    ModeCount* count = (ModeCount*)context;
    DWORD depth = ((DWORD*)desc)[21];
    DWORD twin[31];
    count->byDepth[depth <= 32 ? depth : 0]++;
    if (count->callback(desc, count->context) == 0)
        return 0; /* DDENUMRET_CANCEL */
    if (depth != 32)
        return 1;
    if (count->addEights) {
        memcpy(twin, desc, sizeof(twin));
        MakeEightBitMode(twin);
        count->byDepth[8]++;
        if (count->callback(twin, count->context) == 0)
            return 0;
    }
    if (!count->addTwins)
        return 1;
    memcpy(twin, desc, sizeof(twin));
    MakeSixteenBitMode(twin);
    count->byDepth[16]++;
    return count->callback(twin, count->context);
}

static HRESULT __stdcall DDQueryInterface(void* self, const GUID* iid, void** out)
{
    HRESULT result = ((QueryInterfaceProc)Original(s_directDraw, self, 0))(self, iid, out);
    const char* name = IidName(iid);
    LogLine("IDirectDraw7(%p)::QueryInterface(%s) -> 0x%08lx", self, name, result);
    if (result == 0 && out && *out && !strcmp(name, "IID_IDirect3D7"))
        TraceDirect3DObject(*out);
    return result;
}

static LONG s_createFailures;

static HRESULT __stdcall DDCreateSurface(void* self, void* desc, void** out, void* outer)
{
    CreateSurfaceProc create = (CreateSurfaceProc)Original(s_directDraw, self, 6);
    DWORD copy[31];
    DWORD* d = (DWORD*)desc;
    HRESULT result;

    if (EmulatesFullscreen()) {
        if (d[26] & 0x200) { /* DDSCAPS_PRIMARYSURFACE */
            result = CreateEmulatedPrimary(self, d, out, create);
            if (result == 0)
                TracePrimarySurface(*out);
            return result;
        }
        memcpy(copy, desc, sizeof(copy));
        InjectDisplayFormat(copy);
        NameSurfaceType(copy);
        d = copy;
    }
    result = create(self, d, out, outer);
    /* DDSURFACEDESC2: flags +4, height +8, width +12, back buffers +20,
       pixel format bit count +84, ddsCaps.dwCaps +104. */
    if (result || (d[26] & 0x200)) /* failures, and every primary surface */
        LogLine("IDirectDraw7(%p)::CreateSurface(flags 0x%lx, %lux%lu, %lu bpp, back %lu, "
                "caps 0x%lx) -> 0x%08lx", self, d[1], d[3], d[2], (d[1] & 0x1000) ? d[21] : 0,
                d[5], d[26], result);
    if (result && InterlockedIncrement(&s_createFailures) <= 8) {
        char hex[31 * 9 + 1];
        int k;
        for (k = 0; k < 31; k++)
            wsprintfA(hex + k * 9, "%08lx ", d[k]);
        LogLine("  desc %s", hex);
    }
    if (result == 0 && (d[26] & 0x200) && out && *out)
        TracePrimarySurface(*out);
    return result;
}

static HRESULT __stdcall DDGetDisplayMode(void* self, DWORD* desc)
{
    if (EmulatesFullscreen()) {
        GetEmulatedDisplayMode(desc);
        return 0;
    }
    return ((GetDisplayModeProc)Original(s_directDraw, self, 12))(self, desc);
}

static HRESULT __stdcall DDGetGDISurface(void* self, void** out)
{
    void* primary = CurrentEmulatedPrimary();
    if (EmulatesFullscreen() && primary) {
        ((ULONG(__stdcall*)(void*))SURFACE_SLOT(primary, 1))(primary);
        *out = primary;
        return 0;
    }
    return ((GetGDISurfaceProc)Original(s_directDraw, self, 14))(self, out);
}

static HRESULT __stdcall DDFlipToGDISurface(void* self)
{
    if (EmulatesFullscreen())
        return 0;
    return ((HRESULT(__stdcall*)(void*))Original(s_directDraw, self, 10))(self);
}

static HRESULT __stdcall CountSixteenBitModes(void* desc, void* context)
{
    DWORD depth = ((DWORD*)desc)[21];
    if (depth == 16 || depth == 8)
        ++((int*)context)[depth == 8];
    return 1;
}

static HRESULT __stdcall DDEnumDisplayModes(void* self, DWORD flags, void* desc, void* context,
                                            EnumModesCallback callback)
{
    ModeCount count;
    HRESULT result;
    memset(&count, 0, sizeof(count));
    count.callback = callback;
    count.context = context;
    if (EmulatesFullscreen()) {
        /* Twins only when the system offers no 16-bit modes itself. */
        /* Likewise 8-bit twins: the game's mode choice treats a 640x480x16
           entry at index 0 as "none" (retail lists 8-bit modes first). */
        int native[2] = {0, 0};
        ((EnumDisplayModesProc)Original(s_directDraw, self, 8))(self, flags, desc, native,
                                                                CountSixteenBitModes);
        count.addTwins = native[0] == 0;
        count.addEights = native[1] == 0;
    }
    result = ((EnumDisplayModesProc)Original(s_directDraw, self, 8))(self, flags, desc, &count,
                                                                     CountMode);
    LogLine("IDirectDraw7(%p)::EnumDisplayModes -> 0x%08lx: %d 8-bit, %d 16-bit, %d 24-bit, "
            "%d 32-bit modes", self, result, count.byDepth[8], count.byDepth[16],
            count.byDepth[24], count.byDepth[32]);
    return result;
}

/*
 * The game keeps video memory sizes in signed ints: CreateModeSurfaces
 * (0x4ca520) fails when the flip chain does not fit in "total - 2 * mode
 * size", and a card with 2 GB or more reads as negative there. Report at
 * most 256 MB, plenty for a game that targeted 16 and 32 MB cards.
 */
#define MAX_VIDEO_MEMORY 0x10000000UL

static void ClampVideoMemory(DWORD* bytes)
{
    if (bytes && *bytes > MAX_VIDEO_MEMORY)
        *bytes = MAX_VIDEO_MEMORY;
}

/* DDCAPS_DX7: dwSize is DWORD 0, dwVidMemTotal and dwVidMemFree DWORDs 15
   and 16. */
static void ClampCapsVideoMemory(DWORD* caps)
{
    if (caps && caps[0] >= 17 * sizeof(DWORD)) {
        ClampVideoMemory(&caps[15]);
        ClampVideoMemory(&caps[16]);
    }
}

static HRESULT __stdcall DDGetCaps(void* self, void* driver, void* emulation)
{
    HRESULT result =
        ((GetCapsProc)Original(s_directDraw, self, 11))(self, driver, emulation);
    if (result == 0) {
        ClampCapsVideoMemory((DWORD*)driver);
        ClampCapsVideoMemory((DWORD*)emulation);
    }
    LogLine("IDirectDraw7(%p)::GetCaps -> 0x%08lx, driver dwCaps 0x%08lx dwCaps2 0x%08lx, "
            "emulation dwCaps 0x%08lx", self, result, driver ? ((DWORD*)driver)[1] : 0,
            driver ? ((DWORD*)driver)[2] : 0, emulation ? ((DWORD*)emulation)[1] : 0);
    return result;
}

static HRESULT __stdcall DDSetCooperativeLevel(void* self, HWND window, DWORD flags)
{
    DWORD used = EmulatesFullscreen() ? EmulatedCooperativeLevel(window, flags) : flags;
    HRESULT result =
        ((SetCooperativeLevelProc)Original(s_directDraw, self, 20))(self, window, used);
    LogLine("IDirectDraw7(%p)::SetCooperativeLevel(%p, 0x%lx, used 0x%lx) -> 0x%08lx", self,
            window, flags, used, result);
    return result;
}

static HRESULT __stdcall DDSetDisplayMode(void* self, DWORD width, DWORD height, DWORD bpp,
                                          DWORD refresh, DWORD flags)
{
    HRESULT result;
    if (EmulatesFullscreen()) {
        SetEmulatedDisplayMode(width, height, bpp);
        LogLine("IDirectDraw7(%p)::SetDisplayMode(%lux%lux%lu) emulated", self, width, height,
                bpp);
        return 0;
    }
    result = ((SetDisplayModeProc)Original(s_directDraw, self, 21))(self, width, height, bpp,
                                                                     refresh, flags);
    LogLine("IDirectDraw7(%p)::SetDisplayMode(%lux%lux%lu) -> 0x%08lx", self, width, height, bpp,
            result);
    return result;
}

static HRESULT __stdcall DDGetAvailableVidMem(void* self, void* caps, DWORD* total, DWORD* free)
{
    HRESULT result = ((GetAvailableVidMemProc)Original(s_directDraw, self, 23))(self, caps, total,
                                                                                 free);
    LogLine("IDirectDraw7(%p)::GetAvailableVidMem(caps 0x%lx) -> 0x%08lx, total %lu free %lu%s",
            self, caps ? *(DWORD*)caps : 0, result, total ? *total : 0, free ? *free : 0,
            result == 0 && ((total && *total > MAX_VIDEO_MEMORY) ||
                            (free && *free > MAX_VIDEO_MEMORY))
                ? " (reported as at most 256 MB)"
                : "");
    if (result == 0) {
        ClampVideoMemory(total);
        ClampVideoMemory(free);
    }
    return result;
}

void TraceDirectDrawObject(void* object)
{
    static const int slots[] = {0, 6, 8, 10, 11, 12, 14, 20, 21, 23};
    static void* const wrappers[] = {DDQueryInterface,   DDCreateSurface,       DDEnumDisplayModes,
                                     DDFlipToGDISurface, DDGetCaps,             DDGetDisplayMode,
                                     DDGetGDISurface,    DDSetCooperativeLevel, DDSetDisplayMode,
                                     DDGetAvailableVidMem};
    if (object)
        PatchSlots(s_directDraw, object, slots, wrappers, 10, "IDirectDraw7");
}

/* ---- blade's own system IDirectDraw7 objects ---- */

static HRESULT __stdcall InnerCreateSurface(void* self, void* desc, void** out, void* outer)
{
    HRESULT result = ((CreateSurfaceProc)Original(s_innerDirectDraw, self, 6))(self, desc, out,
                                                                                 outer);
    DWORD* d = (DWORD*)desc;
    if (result)
        LogLine("blade's IDirectDraw7(%p)::CreateSurface(flags 0x%lx, %lux%lu, %lu bpp masks "
                "%lx/%lx/%lx, caps 0x%lx) -> 0x%08lx", self, d[1], d[3], d[2],
                (d[1] & 0x1000) ? d[21] : 0, d[22], d[23], d[24], d[26], result);
    return result;
}

static HRESULT __stdcall InnerGetDisplayMode(void* self, DWORD* desc)
{
    static LONG s_logged;
    if (EmulatesFullscreen()) {
        if (InterlockedIncrement(&s_logged) <= 3)
            LogLine("blade's IDirectDraw7(%p)::GetDisplayMode -> emulated", self);
        GetEmulatedDisplayMode(desc);
        return 0;
    }
    return ((GetDisplayModeProc)Original(s_innerDirectDraw, self, 12))(self, desc);
}

static HRESULT __stdcall InnerSetCooperativeLevel(void* self, HWND window, DWORD flags)
{
    DWORD used = EmulatesFullscreen() ? EmulatedCooperativeLevel(window, flags) : flags;
    HRESULT result =
        ((SetCooperativeLevelProc)Original(s_innerDirectDraw, self, 20))(self, window, used);
    LogLine("blade's IDirectDraw7(%p)::SetCooperativeLevel(%p, 0x%lx, used 0x%lx) -> 0x%08lx",
            self, window, flags, used, result);
    return result;
}

static HRESULT __stdcall InnerSetDisplayMode(void* self, DWORD width, DWORD height, DWORD bpp,
                                             DWORD refresh, DWORD flags)
{
    LogLine("blade's IDirectDraw7(%p)::SetDisplayMode(%lux%lux%lu)%s", self, width, height, bpp,
            EmulatesFullscreen() ? " emulated" : "");
    if (EmulatesFullscreen())
        return 0;
    return ((SetDisplayModeProc)Original(s_innerDirectDraw, self, 21))(self, width, height, bpp,
                                                                        refresh, flags);
}

void TraceInnerDirectDrawObject(void* object)
{
    static const int slots[] = {6, 12, 20, 21};
    static void* const wrappers[] = {InnerCreateSurface, InnerGetDisplayMode,
                                     InnerSetCooperativeLevel, InnerSetDisplayMode};
    if (object)
        PatchSlots(s_innerDirectDraw, object, slots, wrappers, 4, "blade's system IDirectDraw7");
}

/* ---- DirectInput 7 ---- */

static PatchedVtable s_directInput[MAX_VTABLES];
static PatchedVtable s_inputDevice[MAX_VTABLES];
static LONG s_inputFailures;

static void TraceInputDevice(void* device);

static HRESULT __stdcall DICreateDeviceEx(void* self, const GUID* guid, const GUID* iid,
                                          void** out, void* outer)
{
    HRESULT result = ((HRESULT(__stdcall*)(void*, const GUID*, const GUID*, void**, void*))
                          Original(s_directInput, self, 9))(self, guid, iid, out, outer);
    LogLine("IDirectInput7(%p)::CreateDeviceEx({%08lx-...}) -> 0x%08lx, device %p", self,
            guid->Data1, result, out ? *out : 0);
    if (result == 0 && out && *out)
        TraceInputDevice(*out);
    return result;
}

/* Devices whose first Acquire has been seen. PCKeyboardDevice slot 2
   (0x004c4460) releases the keyboard for the whole session when its first
   Acquire fails, which it does with DIERR_OTHERAPPHASPRIO (E_ACCESSDENIED)
   whenever the game window does not have the foreground at start-up (for
   example when the game is launched from a background process). That first
   failure is reported as DI_NOEFFECT; the readers re-acquire on every
   DIERR_NOTACQUIRED, so the keyboard works once the window has focus. */
#define DIERR_OTHERAPPHASPRIO E_ACCESSDENIED
#define DI_NOEFFECT S_FALSE

static void* s_acquiredOnce[32];
static LONG s_acquiredOnceCount;

static int FirstAcquire(void* self)
{
    LONG i, count = s_acquiredOnceCount;
    for (i = 0; i < count; i++)
        if (s_acquiredOnce[i] == self)
            return 0;
    if (count < (LONG)(sizeof(s_acquiredOnce) / sizeof(s_acquiredOnce[0]))) {
        s_acquiredOnce[count] = self;
        s_acquiredOnceCount = count + 1;
    }
    return 1;
}

static HRESULT __stdcall DIDAcquire(void* self)
{
    HRESULT result = ((HRESULT(__stdcall*)(void*))Original(s_inputDevice, self, 7))(self);
    if (FirstAcquire(self) && result == DIERR_OTHERAPPHASPRIO) {
        LogLine("IDirectInputDevice7(%p)::Acquire -> 0x%08lx at creation (window not in the "
                "foreground), reported as DI_NOEFFECT", self, result);
        return DI_NOEFFECT;
    }
    if (result && result != 1 && InterlockedIncrement(&s_inputFailures) <= 20)
        LogLine("IDirectInputDevice7(%p)::Acquire -> 0x%08lx", self, result);
    return result;
}

static HRESULT __stdcall DIDGetDeviceState(void* self, DWORD size, void* data)
{
    HRESULT result =
        ((HRESULT(__stdcall*)(void*, DWORD, void*))Original(s_inputDevice, self, 9))(self, size,
                                                                                      data);
    if (result && InterlockedIncrement(&s_inputFailures) <= 20)
        LogLine("IDirectInputDevice7(%p)::GetDeviceState(%lu) -> 0x%08lx", self, size, result);
    {
        static LONG s_moves, s_calls;
        LONG* m = (LONG*)data;
        InterlockedIncrement(&s_calls);
        if (result == 0 && size >= 16 && size <= 20 && (m[0] || m[1]) &&
            InterlockedIncrement(&s_moves) <= 10)
            LogLine("IDirectInputDevice7(%p)::GetDeviceState mouse moved %ld,%ld (call %ld)", self,
                    m[0], m[1], s_calls);
        if (s_calls == 2000)
            LogLine("IDirectInputDevice7: 2000 GetDeviceState calls, size %lu", size);
    }
    return result;
}

static HRESULT __stdcall DIDGetDeviceData(void* self, DWORD size, void* data, DWORD* count,
                                          DWORD flags)
{
    HRESULT result = ((HRESULT(__stdcall*)(void*, DWORD, void*, DWORD*, DWORD))Original(
        s_inputDevice, self, 10))(self, size, data, count, flags);
    if (result && result != 1 && InterlockedIncrement(&s_inputFailures) <= 20)
        LogLine("IDirectInputDevice7(%p)::GetDeviceData -> 0x%08lx", self, result);
    {
        static LONG s_items, s_calls;
        if (InterlockedIncrement(&s_calls) == 2000)
            LogLine("IDirectInputDevice7: 2000 GetDeviceData calls");
        if (result == 0 && count && *count && data && InterlockedIncrement(&s_items) <= 10)
            LogLine("IDirectInputDevice7(%p)::GetDeviceData -> %lu items", self, *count);
    }
    return result;
}

static HRESULT __stdcall DIDSetCooperativeLevel(void* self, HWND window, DWORD flags)
{
    HRESULT result = ((HRESULT(__stdcall*)(void*, HWND, DWORD))Original(s_inputDevice, self, 13))(
        self, window, flags);
    LogLine("IDirectInputDevice7(%p)::SetCooperativeLevel(%p, 0x%lx) -> 0x%08lx", self, window,
            flags, result);
    return result;
}

static void TraceInputDevice(void* device)
{
    static const int slots[] = {7, 9, 10, 13};
    static void* const wrappers[] = {DIDAcquire, DIDGetDeviceState, DIDGetDeviceData,
                                     DIDSetCooperativeLevel};
    PatchSlots(s_inputDevice, device, slots, wrappers, 4, "IDirectInputDevice7");
}

void TraceDirectInputObject(void* object)
{
    static const int slots[] = {9};
    static void* const wrappers[] = {DICreateDeviceEx};
    if (object)
        PatchSlots(s_directInput, object, slots, wrappers, 1, "IDirectInput7");
}

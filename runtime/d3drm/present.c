/*
 * Emulated 16-bit full screen.
 *
 * MCM2 only runs in 16-bit display modes, which current Windows emulates
 * through the DWM8And16BitMitigation shim. Under that shim the flip chain is
 * lost right after creation and never restored (the shim keeps calling
 * Restore on the implicit back buffer, which fails with
 * DDERR_IMPLICITLYCREATED), so nothing is ever presented.
 *
 * Instead, the game's DirectDraw stays in normal (windowed) cooperative level
 * on the 32-bit desktop:
 *   - SetDisplayMode and GetDisplayMode only record and report the mode;
 *   - EnumDisplayModes reports a 16-bit twin of every 32-bit mode;
 *   - surfaces created without a pixel format get RGB 565 instead of the
 *     desktop format;
 *   - the primary surface is an EmulatedPrimary: a small IDirectDrawSurface7
 *     over two RGB 565 surfaces: a system-memory front buffer and a back
 *     buffer in video memory for hardware Direct3D (the HAL device renders
 *     into it), system memory otherwise. Flip copies
 *     the back buffer into the game's window with StretchDIBits, scaled to
 *     fit with black bars, and into the front surface.
 * The game window is made a borderless popup covering the primary monitor
 * (CoverScreen), so this looks like full screen. Flip is paced to MCM2_FPS.
 * MCM2_NATIVE_FULLSCREEN=1 turns the emulation off.
 */
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

#define DDSD_CAPS 0x1
#define DDSD_HEIGHT 0x2
#define DDSD_WIDTH 0x4
#define DDSD_PITCH 0x8
#define DDSD_BACKBUFFERCOUNT 0x20
#define DDSD_PIXELFORMAT 0x1000
#define DDSCAPS_BACKBUFFER 0x4
#define DDSCAPS_COMPLEX 0x8
#define DDSCAPS_FLIP 0x10
#define DDSCAPS_FRONTBUFFER 0x20
#define DDSCAPS_OFFSCREENPLAIN 0x40
#define DDSCAPS_OVERLAY 0x80
#define DDSCAPS_PRIMARYSURFACE 0x200
#define DDSCAPS_SYSTEMMEMORY 0x800
#define DDSCAPS_TEXTURE 0x1000
#define DDSCAPS_3DDEVICE 0x2000
#define DDSCAPS_VIDEOMEMORY 0x4000
#define DDSCAPS_VISIBLE 0x8000
#define DDSCAPS_ZBUFFER 0x20000
#define DDSCAPS_LOCALVIDMEM 0x10000000
#define DDSCL_NORMAL 0x8
#define DDSCL_FULLSCREEN 0x1
#define DDSCL_ALLOWREBOOT 0x2
#define DDSCL_EXCLUSIVE 0x10
#define DDPF_RGB 0x40
#define DDLOCK_WAIT 0x1
#define DDLOCK_READONLY 0x10
#define DDBLTFAST_WAIT 0x10
#define E_NOINTERFACE_ ((HRESULT)0x80004002L)
#define DDERR_NOTFOUND_ ((HRESULT)0x887600ffL)

/* DDSURFACEDESC2 as DWORDs: see vtrace.c. */
enum {
    SD_SIZE = 0, SD_FLAGS = 1, SD_HEIGHT = 2, SD_WIDTH = 3, SD_PITCH = 4, SD_BACKBUFFERS = 5,
    SD_SURFACE = 9, SD_PF_SIZE = 18, SD_PF_FLAGS = 19, SD_PF_BITS = 21, SD_PF_RMASK = 22,
    SD_PF_GMASK = 23, SD_PF_BMASK = 24, SD_PF_AMASK = 25, SD_CAPS = 26, SD_DWORDS = 31
};

#define SLOT(object, slot) ((*(void***)(object))[slot])

typedef HRESULT(__stdcall* Call0)(void*);
typedef HRESULT(__stdcall* Call1)(void*, DWORD);
typedef HRESULT(__stdcall* Call2)(void*, DWORD, DWORD);
typedef HRESULT(__stdcall* Call3)(void*, DWORD, DWORD, DWORD);
typedef HRESULT(__stdcall* Call4)(void*, DWORD, DWORD, DWORD, DWORD);
typedef HRESULT(__stdcall* Call5)(void*, DWORD, DWORD, DWORD, DWORD, DWORD);
typedef ULONG(__stdcall* RefProc)(void*);

typedef struct EmulatedPrimary {
    void** vtable;
    LONG references;
    void* front;
    void* back; /* NULL without back buffers */
    DWORD caps;
} EmulatedPrimary;

static int s_enabled = -1;
static HWND s_window;
static DWORD s_width = 640, s_height = 480, s_depth = 16;
static EmulatedPrimary* s_primary;
static LONG s_presents;
/* Where the last frame was drawn in the window's client area. */
static RECT s_drawn;

int EmulatesFullscreen(void)
{
    char value[8];
    if (s_enabled < 0) {
        s_enabled = !(GetEnvironmentVariableA("MCM2_NATIVE_FULLSCREEN", value, sizeof(value)) &&
                      value[0] == '1');
        LogLine("present: %s", s_enabled ? "emulating 16-bit full screen in the game window"
                                         : "native full screen (MCM2_NATIVE_FULLSCREEN=1)");
    }
    return s_enabled;
}

/*
 * Exclusive DirectDraw stretched the game window over the screen; the game
 * itself leaves it at the small size it chose for windowed mode. Make it a
 * borderless window covering the primary monitor instead.
 */
static void CoverScreen(HWND window)
{
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    SetWindowLongA(window, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(window, HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    LogLine("present: game window %p now covers the %dx%d screen", window, width, height);
}

DWORD EmulatedCooperativeLevel(HWND window, DWORD flags)
{
    if (!(flags & DDSCL_EXCLUSIVE))
        return flags;
    s_window = window;
    CoverScreen(window);
    return (flags & ~(DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT)) | DDSCL_NORMAL;
}

void SetEmulatedDisplayMode(DWORD width, DWORD height, DWORD depth)
{
    s_width = width;
    s_height = height;
    s_depth = depth;
}

static void SetRgb565(DWORD* desc)
{
    memset(desc + SD_PF_SIZE, 0, 8 * sizeof(DWORD));
    desc[SD_PF_SIZE] = 32;
    desc[SD_PF_FLAGS] = DDPF_RGB;
    desc[SD_PF_BITS] = 16;
    desc[SD_PF_RMASK] = 0xf800;
    desc[SD_PF_GMASK] = 0x07e0;
    desc[SD_PF_BMASK] = 0x001f;
}

void GetEmulatedDisplayMode(DWORD* desc)
{
    DWORD size = desc[SD_SIZE];
    memset(desc, 0, size < SD_DWORDS * 4 ? size : SD_DWORDS * 4);
    desc[SD_SIZE] = size;
    desc[SD_FLAGS] = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT;
    desc[SD_WIDTH] = s_width;
    desc[SD_HEIGHT] = s_height;
    desc[SD_PITCH] = s_width * 2;
    SetRgb565(desc);
}

/* Turns a 32-bit display mode description into its 16-bit twin. */
void MakeSixteenBitMode(DWORD* desc)
{
    desc[SD_PITCH] = desc[SD_WIDTH] * 2;
    SetRgb565(desc);
}

/* Turns a 32-bit display mode description into a palettized 8-bit twin. */
void MakeEightBitMode(DWORD* desc)
{
    memset(desc + SD_PF_SIZE, 0, 8 * sizeof(DWORD));
    desc[SD_PF_SIZE] = 32;
    desc[SD_PF_FLAGS] = DDPF_RGB | 0x20; /* DDPF_PALETTEINDEXED8 */
    desc[SD_PF_BITS] = 8;
    desc[SD_PITCH] = desc[SD_WIDTH];
}

/* Surfaces without an explicit format would get the 32-bit desktop format;
   the game expects the 16-bit display format. */
void InjectDisplayFormat(DWORD* desc)
{
    if (desc[SD_FLAGS] & DDSD_PIXELFORMAT)
        return;
    if (desc[SD_CAPS] & (DDSCAPS_ZBUFFER | DDSCAPS_PRIMARYSURFACE))
        return;
    desc[SD_FLAGS] |= DDSD_PIXELFORMAT;
    SetRgb565(desc);
}

/* On a 32-bit desktop, Windows' ddraw rejects the game's 16-bit system-memory
   surfaces that name no surface type (its render target and 2D images) with
   DDERR_INVALIDPIXELFORMAT, and accepts them as offscreen plain surfaces. */
void NameSurfaceType(DWORD* desc)
{
    if ((desc[SD_FLAGS] & DDSD_PIXELFORMAT) &&
        !(desc[SD_CAPS] & (DDSCAPS_OFFSCREENPLAIN | DDSCAPS_TEXTURE | DDSCAPS_ZBUFFER |
                           DDSCAPS_PRIMARYSURFACE | DDSCAPS_OVERLAY)))
        desc[SD_CAPS] |= DDSCAPS_OFFSCREENPLAIN;
}

void* RealSurface(void* surface)
{
    if (surface && s_primary && surface == (void*)s_primary)
        return s_primary->front;
    return surface;
}

void* CurrentEmulatedPrimary(void)
{
    return s_primary;
}

/* ---- presentation ---- */

static void Present(EmulatedPrimary* self)
{
    struct {
        BITMAPINFOHEADER header;
        DWORD masks[3];
    } info;
    DWORD desc[SD_DWORDS];
    RECT client;
    HDC dc;
    int width, height, drawWidth, drawHeight, x, y;

    memset(desc, 0, sizeof(desc));
    desc[SD_SIZE] = sizeof(desc);
    if (((Call4)SLOT(self->back, 25))(self->back, 0, (DWORD)desc, DDLOCK_WAIT | DDLOCK_READONLY,
                                      0))
        return;
    width = desc[SD_WIDTH];
    height = desc[SD_HEIGHT];
    memset(&info, 0, sizeof(info));
    info.header.biSize = sizeof(info.header);
    info.header.biWidth = desc[SD_PITCH] / 2;
    info.header.biHeight = -height;
    info.header.biPlanes = 1;
    info.header.biBitCount = 16;
    info.header.biCompression = BI_BITFIELDS;
    info.masks[0] = desc[SD_PF_RMASK];
    info.masks[1] = desc[SD_PF_GMASK];
    info.masks[2] = desc[SD_PF_BMASK];

    GetClientRect(s_window, &client);
    if (client.right * height <= client.bottom * width) {
        drawWidth = client.right;
        drawHeight = client.right * height / width;
    } else {
        drawHeight = client.bottom;
        drawWidth = client.bottom * width / height;
    }
    x = (client.right - drawWidth) / 2;
    y = (client.bottom - drawHeight) / 2;
    dc = GetDC(s_window);
    if (dc) {
        SetStretchBltMode(dc, COLORONCOLOR);
        StretchDIBits(dc, x, y, drawWidth, drawHeight, 0, 0, width, height,
                      (void*)desc[SD_SURFACE], (BITMAPINFO*)&info, DIB_RGB_COLORS, SRCCOPY);
        if (x > 0) {
            PatBlt(dc, 0, 0, x, client.bottom, BLACKNESS);
            PatBlt(dc, x + drawWidth, 0, client.right - x - drawWidth, client.bottom, BLACKNESS);
        }
        if (y > 0) {
            PatBlt(dc, 0, 0, client.right, y, BLACKNESS);
            PatBlt(dc, 0, y + drawHeight, client.right, client.bottom - y - drawHeight, BLACKNESS);
        }
        ReleaseDC(s_window, dc);
    }
    ((Call1)SLOT(self->back, 32))(self->back, 0);
    /* Keep front-buffer reads (and frame dumps) consistent with the screen. */
    ((Call5)SLOT(self->front, 7))(self->front, 0, 0, (DWORD)self->back, 0, DDBLTFAST_WAIT);
    SetRect(&s_drawn, x, y, x + drawWidth, y + drawHeight);
    if (InterlockedIncrement(&s_presents) == 1)
        LogLine("present: first frame %dx%d shown at %d,%d %dx%d in a %ldx%ld window", width,
                height, x, y, drawWidth, drawHeight, client.right, client.bottom);
}

/* ---- EmulatedPrimary: IDirectDrawSurface7 ---- */

#define FRONT(self) (((EmulatedPrimary*)(self))->front)
#define FORWARD0(name, slot) \
    static HRESULT __stdcall name(void* self) { return ((Call0)SLOT(FRONT(self), slot))(FRONT(self)); }
#define FORWARD1(name, slot) \
    static HRESULT __stdcall name(void* self, DWORD a) \
    { return ((Call1)SLOT(FRONT(self), slot))(FRONT(self), a); }
#define FORWARD2(name, slot) \
    static HRESULT __stdcall name(void* self, DWORD a, DWORD b) \
    { return ((Call2)SLOT(FRONT(self), slot))(FRONT(self), a, b); }
#define FORWARD3(name, slot) \
    static HRESULT __stdcall name(void* self, DWORD a, DWORD b, DWORD c) \
    { return ((Call3)SLOT(FRONT(self), slot))(FRONT(self), a, b, c); }
#define FORWARD4(name, slot) \
    static HRESULT __stdcall name(void* self, DWORD a, DWORD b, DWORD c, DWORD d) \
    { return ((Call4)SLOT(FRONT(self), slot))(FRONT(self), a, b, c, d); }
#define FORWARD5(name, slot) \
    static HRESULT __stdcall name(void* self, DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) \
    { return ((Call5)SLOT(FRONT(self), slot))(FRONT(self), a, b, c, d, e); }

static HRESULT __stdcall PrimaryQueryInterface(void* self, const GUID* iid, void** out)
{
    static const GUID kSurface7 = {0x06675a80, 0x3b9b, 0x11d2,
                                   {0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b}};
    if (!memcmp(iid, &kSurface7, sizeof(GUID))) {
        InterlockedIncrement(&((EmulatedPrimary*)self)->references);
        *out = self;
        return 0;
    }
    /* No gamma control or older interfaces; the game handles their absence. */
    *out = NULL;
    return E_NOINTERFACE_;
}

static ULONG __stdcall PrimaryAddRef(void* self)
{
    return InterlockedIncrement(&((EmulatedPrimary*)self)->references);
}

static ULONG __stdcall PrimaryRelease(void* self)
{
    EmulatedPrimary* primary = (EmulatedPrimary*)self;
    LONG left = InterlockedDecrement(&primary->references);
    if (left == 0) {
        if (primary->back)
            ((RefProc)SLOT(primary->back, 2))(primary->back);
        ((RefProc)SLOT(primary->front, 2))(primary->front);
        if (s_primary == primary)
            s_primary = NULL;
        HeapFree(GetProcessHeap(), 0, primary);
        LogLine("present: released the emulated primary");
    }
    return left;
}

static HRESULT __stdcall PrimaryBlt(void* self, DWORD rect, DWORD source, DWORD sourceRect,
                                    DWORD flags, DWORD fx)
{
    HRESULT result = ((Call5)SLOT(FRONT(self), 5))(FRONT(self), rect,
                                                   (DWORD)RealSurface((void*)source), sourceRect,
                                                   flags, fx);
    /* A blit to the primary is a present in the game's windowed path. */
    if (result == 0 && ((EmulatedPrimary*)self)->back == NULL) {
        EmulatedPrimary view = *(EmulatedPrimary*)self;
        view.back = view.front;
        Present(&view);
    }
    return result;
}

static HRESULT __stdcall PrimaryBltFast(void* self, DWORD x, DWORD y, DWORD source, DWORD rect,
                                        DWORD flags)
{
    return ((Call5)SLOT(FRONT(self), 7))(FRONT(self), x, y, (DWORD)RealSurface((void*)source),
                                         rect, flags);
}

/*
 * Exclusive-mode Flip waited for vertical blank; the emulated one returns at
 * once, so the game would present over a thousand frames a second. Pace it
 * to MCM2_FPS frames a second (default 60, 0 for no limit). Sleep is precise
 * to about a millisecond because the timer resolution stays at 1 ms (see
 * GameTimeBeginPeriod in trace.c); the last stretch is spun.
 */
static void WaitForFrameSlot(void)
{
    static int s_fps = -1;
    static LARGE_INTEGER s_frequency, s_next;
    LARGE_INTEGER now;
    LONGLONG period, remaining;
    char value[16];

    if (s_fps < 0) {
        s_fps = 60;
        if (GetEnvironmentVariableA("MCM2_FPS", value, sizeof(value)))
            s_fps = atoi(value);
        if (s_fps < 0)
            s_fps = 0;
        QueryPerformanceFrequency(&s_frequency);
        LogLine("present: %s", s_fps ? "frame rate limited (MCM2_FPS)" : "frame rate unlimited");
        if (s_fps)
            LogLine("present: %d frames a second", s_fps);
    }
    if (!s_fps)
        return;
    period = s_frequency.QuadPart / s_fps;
    QueryPerformanceCounter(&now);
    if (s_next.QuadPart == 0 || now.QuadPart - s_next.QuadPart > period) {
        /* First frame, or too far behind to catch up: start over from now. */
        s_next.QuadPart = now.QuadPart + period;
        return;
    }
    for (;;) {
        remaining = s_next.QuadPart - now.QuadPart;
        if (remaining <= 0)
            break;
        if (remaining * 1000 > s_frequency.QuadPart * 2)
            Sleep(1);
        else
            Sleep(0);
        QueryPerformanceCounter(&now);
    }
    s_next.QuadPart += period;
}

static HRESULT __stdcall PrimaryFlip(void* self, DWORD target, DWORD flags)
{
    EmulatedPrimary* primary = (EmulatedPrimary*)self;
    (void)target;
    (void)flags;
    if (primary->back) {
        WaitForFrameSlot();
        Present(primary);
    }
    return 0;
}

static HRESULT __stdcall PrimaryGetAttachedSurface(void* self, DWORD* caps, void** out)
{
    EmulatedPrimary* primary = (EmulatedPrimary*)self;
    if (primary->back && (caps[0] & DDSCAPS_BACKBUFFER)) {
        ((RefProc)SLOT(primary->back, 1))(primary->back);
        *out = primary->back;
        return 0;
    }
    *out = NULL;
    return DDERR_NOTFOUND_;
}

static HRESULT __stdcall PrimaryGetCaps(void* self, DWORD* caps)
{
    caps[0] = ((EmulatedPrimary*)self)->caps;
    caps[1] = caps[2] = caps[3] = 0;
    return 0;
}

static HRESULT __stdcall PrimaryGetSurfaceDesc(void* self, DWORD* desc)
{
    HRESULT result = ((Call1)SLOT(FRONT(self), 22))(FRONT(self), (DWORD)desc);
    if (result == 0) {
        desc[SD_CAPS] = ((EmulatedPrimary*)self)->caps;
        if (((EmulatedPrimary*)self)->back) {
            desc[SD_FLAGS] |= DDSD_BACKBUFFERCOUNT;
            desc[SD_BACKBUFFERS] = 1;
        }
    }
    return result;
}

static HRESULT __stdcall PrimaryOk0(void* self)
{
    (void)self;
    return 0;
}

static HRESULT __stdcall PrimaryOk1(void* self, DWORD a)
{
    (void)self;
    (void)a;
    return 0;
}

FORWARD1(PrimaryAddAttachedSurface, 3)
FORWARD1(PrimaryAddOverlayDirtyRect, 4)
FORWARD3(PrimaryBltBatch, 6)
FORWARD2(PrimaryDeleteAttachedSurface, 8)
FORWARD2(PrimaryEnumAttachedSurfaces, 9)
FORWARD3(PrimaryEnumOverlayZOrders, 10)
FORWARD1(PrimaryGetBltStatus, 13)
FORWARD1(PrimaryGetClipper, 15)
FORWARD2(PrimaryGetColorKey, 16)
FORWARD1(PrimaryGetDC, 17)
FORWARD2(PrimaryGetOverlayPosition, 19)
FORWARD1(PrimaryGetPalette, 20)
FORWARD1(PrimaryGetPixelFormat, 21)
FORWARD2(PrimaryInitialize, 23)
FORWARD4(PrimaryLock, 25)
FORWARD1(PrimaryReleaseDC, 26)
FORWARD1(PrimarySetClipper, 28)
FORWARD2(PrimarySetColorKey, 29)
FORWARD2(PrimarySetOverlayPosition, 30)
FORWARD1(PrimarySetPalette, 31)
FORWARD1(PrimaryUnlock, 32)
FORWARD5(PrimaryUpdateOverlay, 33)
FORWARD1(PrimaryUpdateOverlayDisplay, 34)
FORWARD2(PrimaryUpdateOverlayZOrder, 35)
FORWARD1(PrimaryGetDDInterface, 36)
FORWARD1(PrimaryPageLock, 37)
FORWARD1(PrimaryPageUnlock, 38)
FORWARD2(PrimarySetSurfaceDesc, 39)
FORWARD4(PrimarySetPrivateData, 40)
FORWARD3(PrimaryGetPrivateData, 41)
FORWARD1(PrimaryFreePrivateData, 42)
FORWARD1(PrimaryGetUniquenessValue, 43)
FORWARD0(PrimaryChangeUniquenessValue, 44)
FORWARD1(PrimarySetPriority, 45)
FORWARD1(PrimaryGetPriority, 46)
FORWARD1(PrimarySetLOD, 47)
FORWARD1(PrimaryGetLOD, 48)

static void* s_primaryVtable[49] = {
    PrimaryQueryInterface,       PrimaryAddRef,
    PrimaryRelease,              PrimaryAddAttachedSurface,
    PrimaryAddOverlayDirtyRect,  PrimaryBlt,
    PrimaryBltBatch,             PrimaryBltFast,
    PrimaryDeleteAttachedSurface, PrimaryEnumAttachedSurfaces,
    PrimaryEnumOverlayZOrders,   PrimaryFlip,
    PrimaryGetAttachedSurface,   PrimaryGetBltStatus,
    PrimaryGetCaps,              PrimaryGetClipper,
    PrimaryGetColorKey,          PrimaryGetDC,
    PrimaryOk1, /* GetFlipStatus */ PrimaryGetOverlayPosition,
    PrimaryGetPalette,           PrimaryGetPixelFormat,
    PrimaryGetSurfaceDesc,       PrimaryInitialize,
    PrimaryOk0, /* IsLost */     PrimaryLock,
    PrimaryReleaseDC,            PrimaryOk0, /* Restore */
    PrimarySetClipper,           PrimarySetColorKey,
    PrimarySetOverlayPosition,   PrimarySetPalette,
    PrimaryUnlock,               PrimaryUpdateOverlay,
    PrimaryUpdateOverlayDisplay, PrimaryUpdateOverlayZOrder,
    PrimaryGetDDInterface,       PrimaryPageLock,
    PrimaryPageUnlock,           PrimarySetSurfaceDesc,
    PrimarySetPrivateData,       PrimaryGetPrivateData,
    PrimaryFreePrivateData,      PrimaryGetUniquenessValue,
    PrimaryChangeUniquenessValue, PrimarySetPriority,
    PrimaryGetPriority,          PrimarySetLOD,
    PrimaryGetLOD,
};

typedef HRESULT(__stdcall* CreateSurfaceCall)(void*, void*, void**, void*);

static HRESULT CreatePlain(void* directDraw, CreateSurfaceCall create, DWORD memory,
                           DWORD extraCaps, void** out)
{
    DWORD desc[SD_DWORDS];
    memset(desc, 0, sizeof(desc));
    desc[SD_SIZE] = sizeof(desc);
    desc[SD_FLAGS] = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc[SD_WIDTH] = s_width;
    desc[SD_HEIGHT] = s_height;
    desc[SD_CAPS] = DDSCAPS_OFFSCREENPLAIN | memory | extraCaps;
    SetRgb565(desc);
    return create(directDraw, desc, out, NULL);
}

HRESULT CreateEmulatedPrimary(void* directDraw, const DWORD* desc, void** out,
                              CreateSurfaceCall create)
{
    EmulatedPrimary* primary;
    DWORD backBuffers = (desc[SD_FLAGS] & DDSD_BACKBUFFERCOUNT) ? desc[SD_BACKBUFFERS] : 0;
    HRESULT result;

    primary = (EmulatedPrimary*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*primary));
    if (!primary)
        return E_OUTOFMEMORY;
    primary->vtable = s_primaryVtable;
    primary->references = 1;
    primary->caps = (desc[SD_CAPS] & ~DDSCAPS_SYSTEMMEMORY) | DDSCAPS_FRONTBUFFER |
                    DDSCAPS_VISIBLE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
    result = CreatePlain(directDraw, create, DDSCAPS_SYSTEMMEMORY, 0, &primary->front);
    /* A hardware Direct3D device renders into the back buffer, so that one
       must be in video memory when the game asks for a 3D flip chain there. */
    if (result == 0 && backBuffers && (desc[SD_CAPS] & DDSCAPS_3DDEVICE) &&
        (desc[SD_CAPS] & DDSCAPS_VIDEOMEMORY)) {
        result = CreatePlain(directDraw, create, DDSCAPS_VIDEOMEMORY, DDSCAPS_3DDEVICE,
                             &primary->back);
        LogLine("present: video-memory 3D back buffer -> 0x%08lx", result);
    } else if (result == 0 && backBuffers) {
        result = CreatePlain(directDraw, create, DDSCAPS_SYSTEMMEMORY,
                             desc[SD_CAPS] & DDSCAPS_3DDEVICE, &primary->back);
    }
    if (result) {
        if (primary->front)
            ((RefProc)SLOT(primary->front, 2))(primary->front);
        HeapFree(GetProcessHeap(), 0, primary);
        LogLine("present: could not create the %lux%lu emulated primary (0x%08lx)", s_width,
                s_height, result);
        return result;
    }
    s_primary = primary;
    *out = primary;
    LogLine("present: emulated primary %lux%lu RGB565 with %lu back buffer(s), front %p back %p",
            s_width, s_height, backBuffers, primary->front, primary->back);
    return 0;
}

/*
 * The game reads the mouse with GetCursorPos and subtracts its window's
 * client origin, expecting a client area the size of the display mode. Map
 * the real cursor from the scaled picture back into display-mode pixels.
 */
void MapCursorToGame(POINT* point)
{
    POINT client;
    int width = s_drawn.right - s_drawn.left, height = s_drawn.bottom - s_drawn.top;
    if (!s_window || width <= 0 || height <= 0)
        return;
    client = *point;
    ScreenToClient(s_window, &client);
    client.x = (client.x - s_drawn.left) * (LONG)s_width / width;
    client.y = (client.y - s_drawn.top) * (LONG)s_height / height;
    client.x = client.x < 0 ? 0 : client.x >= (LONG)s_width ? (LONG)s_width - 1 : client.x;
    client.y = client.y < 0 ? 0 : client.y >= (LONG)s_height ? (LONG)s_height - 1 : client.y;
    ClientToScreen(s_window, &client);
    *point = client;
}

LONG PresentedFrames(void)
{
    return s_presents;
}

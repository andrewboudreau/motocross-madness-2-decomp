#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DebugAlloc.h"
#include "Game.h"

#include "ControlInterface.h"
#include "DebugOverlay.h"
#include "GameObject.h"
#include "MemTag.h"
#include "PCControl.h"
#include "PeakHold.h"
#include "SoundInterface.h"
#include "TextureMapManager.h"
#include "TrackGame.h"

// 0x0065b4ac: a file cleared by the constructor and closed by the
// destructor (0x00534c3d is the CRT's fclose).
static FILE* s_UnknownFile65b4ac;

// 0x0056b334 and 0x0056b350: stored XOR 0x5b and decoded by the constructor
// ("SOFTWARE\\Rainbow Studios\\" and "TestKey").
static char s_UnknownEncoded56b334[] =
    "\x08\x14\x1d\x0f\x0c\x1a\x09\x1e\x07\x09\x3a\x32\x35\x39\x34\x2c\x7b\x08\x2f\x2e\x3f\x32\x34\x28\x07";
static char s_UnknownEncoded56b350[] = "\x0f\x3e\x28\x2f\x10\x3e\x22";

// KERNEL32 MEMORYSTATUS and its import (slot 8's memory page).
struct UnknownMemoryStatus {
    unsigned long length;
    unsigned long memoryLoad;
    unsigned long totalPhys;
    unsigned long availPhys;
    unsigned long totalPageFile;
    unsigned long availPageFile;
    unsigned long totalVirtual;
    unsigned long availVirtual;
};
extern "C" __declspec(dllimport) void __stdcall GlobalMemoryStatus(UnknownMemoryStatus* status);

// 0x0065b490..0x0065b528: peak-held frame timings for slot 8's profile page,
// constructed in this order (0x00467860..0x00467980).
static UnknownPeakHold s_NetPeak(5000);              // 0x0065b518
static UnknownPeakHold s_TickPeak(5000);             // 0x0065b4c8
static UnknownPeakHold s_PrepFramePeak(5000);        // 0x0065b528
static UnknownPeakHold s_UpdateScreenPeak(5000);     // 0x0065b4e8
static UnknownPeakHold s_PrepareGeometryPeak(5000);  // 0x0065b490
static UnknownPeakHold s_WaitForFlipPeak(5000);      // 0x0065b4b8
static UnknownPeakHold s_RenderPre3DPeak(5000);      // 0x0065b4f8
static UnknownPeakHold s_Render3DPeak(5000);         // 0x0065b4d8
static UnknownPeakHold s_RenderPost3DPeak(5000);     // 0x0065b4a0
static UnknownPeakHold s_ElapsedPeak(5000);          // 0x0065b508

// 0x0065b534..0x0065b540: phase times measured by slot 10; 0x0065b544 is
// the time of the previous frame.
int g_UnknownTickTime;                               // 0x0065b534
int g_UnknownPrepFrameTime;                          // 0x0065b538
int g_UnknownUpdateScreenTime;                       // 0x0065b53c
int g_UnknownNetTime;                                // 0x0065b540
static unsigned int s_LastFrameTime;                 // 0x0065b544

static inline void DecodeString(char* text) {
    for (; *text; text++)
        *text ^= 0x5b;
}

// 0x00467990
Game::Game() {
    UnknownFunction460ad0();
    field_0x2d5_bit3 = 1;
    field_0x2d4_bit2 = 0;
    field_0x2d4_bit1 = 1;
    field_0x04 = 0;
    field_0x08 = 0;
    field_0x0c = 0;
    field_0x10 = 0;
    field_0x14 = 0;
    field_0x34 = 0;
    field_0x2f4 = 0;
    field_0x3c = 0;
    field_0x18 = 1;
    field_0x2d0 = 0;
    field_0x38 = 0;
    s_UnknownFile65b4ac = 0;
    strcpy(field_0x40, "");
    field_0x2dc = 0;
    field_0x2d8 = 0;
    field_0x2e0 = 1.0f;
    field_0x2d4_bit3 = 1;
    field_0x2d4_bit4 = 1;
    field_0x2d4_bit5 = 1;
    field_0x2d4_bit6 = 0;
    field_0x2d4_bit7 = 0;
    field_0x1c4 = 0;
    field_0x1c8 = 0;
    field_0x2d5_bit0 = 0;
    field_0x2d5_bit1 = 0;
    field_0x2d5_bit2 = 1;
    field_0x2e4 = 1.0f;
    field_0x2e8 = 0;
    field_0x2ec = 0.05f;
    field_0x2f0 = 0;
    DecodeString(s_UnknownEncoded56b334);
    DecodeString(s_UnknownEncoded56b350);
}

// 0x00467ae0 (identical code for slots 5 and 6)
int Game::UnknownVirtualSlot1() {
    return 1;
}

int Game::UnknownVirtualSlot5() {
    return 1;
}

int Game::UnknownVirtualSlot6() {
    return 1;
}

// 0x00467af0: creates the PC control interface and sets it up (slot 1).
int Game::UnknownVirtualSlot2() {
    field_0x14 = new(__FILE__, 187) PCControlInterface;
    return field_0x14->UnknownVirtualSlot1() != 0;
}

// 0x00467e80: slot 31's result is kept at +0x10 and handed to the interface.
int Game::UnknownVirtualSlot33() {
    field_0x10 = UnknownVirtualSlot31();
    if (!field_0x10)
        return 0;
    if (field_0x2f4)
        field_0x2f4->UnknownVirtualSlot25(field_0x10);
    return 1;
}

// 0x00467eb0: renders a frame, timing each phase, and with bit 2 of +0x2d4
// fills the debug overlay's profile and memory pages.
int Game::UnknownVirtualSlot8() {
    if (field_0x0c->field_0x70_bit2)
        field_0x0c->UnknownVirtualSlot4(0);
    unsigned int last = UnknownFunction4bfa80();
    field_0x2f4->UnknownVirtualSlot12();
    unsigned int now = UnknownFunction4bfa80();
    int prepareGeometry = now - last;
    last = now;
    if (field_0x0c->field_0x70_bit2)
        field_0x0c->UnknownVirtualSlot4(1);
    now = UnknownFunction4bfa80();
    int waitForFlip = now - last;
    last = now;
    field_0x2f4->UnknownVirtualSlot13();
    now = UnknownFunction4bfa80();
    int renderPre3D = now - last;
    int render3D = 0;
    if (field_0x2d5_bit3) {
        if (!field_0x10->UnknownVirtualSlot1())
            goto failed;
        field_0x10->UnknownVirtualSlot12(0, 0);
        UnknownVirtualSlot7();
        last = UnknownFunction4bfa80();
        field_0x2f4->UnknownVirtualSlot14();
        render3D = UnknownFunction4bfa80() - last;
        if (!field_0x10->UnknownVirtualSlot2()) {
        failed:
            return 0;
        }
    }
    last = UnknownFunction4bfa80();
    field_0x2f4->UnknownVirtualSlot15();
    now = UnknownFunction4bfa80();
    int renderPost3D = now - last;
    int elapsed = now - s_LastFrameTime;
    if (field_0x2d4_bit2) {
        if (field_0x38) {
            static int profilePage = -1;
            if (profilePage < 0)
                profilePage = field_0x38->NewPage();
            s_NetPeak.UnknownFunction4cb6b0(g_UnknownNetTime);
            s_TickPeak.UnknownFunction4cb6b0(g_UnknownTickTime);
            s_PrepFramePeak.UnknownFunction4cb6b0(g_UnknownPrepFrameTime);
            s_UpdateScreenPeak.UnknownFunction4cb6b0(g_UnknownUpdateScreenTime);
            s_PrepareGeometryPeak.UnknownFunction4cb6b0(prepareGeometry);
            s_WaitForFlipPeak.UnknownFunction4cb6b0(waitForFlip);
            s_RenderPre3DPeak.UnknownFunction4cb6b0(renderPre3D);
            s_Render3DPeak.UnknownFunction4cb6b0(render3D);
            s_RenderPost3DPeak.UnknownFunction4cb6b0(renderPost3D);
            s_ElapsedPeak.UnknownFunction4cb6b0(elapsed);
            if (field_0x38->field_0x25_bit0 && field_0x38->field_0x26c4 == profilePage) {
                UnknownDisplayMode* mode = &field_0x0c->field_0x10[field_0x0c->field_0x0c];
                field_0x38->UnknownFunction447fa0(profilePage, "%c %d x %d %d bit(x%d)", 'R',
                                                  mode->width, mode->height, mode->bitDepth,
                                                  field_0x0c->field_0x78);
                field_0x38->UnknownFunction447f40(profilePage, "%s", field_0x0c->field_0x5c0.description);
                field_0x38->UnknownFunction447f40(profilePage, "ElapsedTime:   % 3d (%d)",
                                                  elapsed, s_ElapsedPeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "PrepFrameTime: % 3d (%d)",
                                                  g_UnknownPrepFrameTime,
                                                  s_PrepFramePeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "UpdateScrnTime:% 3d (%d)",
                                                  g_UnknownUpdateScreenTime,
                                                  s_UpdateScreenPeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "Net            % 3d (%d)",
                                                  g_UnknownNetTime, s_NetPeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "Tick           % 3d (%d)",
                                                  g_UnknownTickTime, s_TickPeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "PrepareGeometry% 3d (%d)",
                                                  prepareGeometry,
                                                  s_PrepareGeometryPeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "WaitForFlip    % 3d (%d)",
                                                  waitForFlip, s_WaitForFlipPeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "RenderPre3D    % 3d (%d)",
                                                  renderPre3D, s_RenderPre3DPeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "Render3D       % 3d (%d)",
                                                  render3D, s_Render3DPeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "   [this overlay %d]",
                                                  field_0x38->field_0x36dc);
                field_0x38->UnknownFunction447f40(profilePage, "RenderPost3D   % 3d (%d)",
                                                  renderPost3D, s_RenderPost3DPeak.UnknownFunction4cb690());
                field_0x38->UnknownFunction447f40(profilePage, "TotalTransforms% 5d", field_0x10->field_0x38);
                field_0x38->UnknownFunction447f40(profilePage, "TotalPoints    % 5d", field_0x10->field_0x3c);
                field_0x38->UnknownFunction447f40(profilePage, "TotalLines     % 5d", field_0x10->field_0x40);
                field_0x38->UnknownFunction447f40(profilePage, "TotalTriangles % 5d", field_0x10->field_0x44);
            }
            static int memoryPage = -1;
            if (memoryPage < 0)
                memoryPage = field_0x38->NewPage();
            if (field_0x38->field_0x25_bit0 && field_0x38->field_0x26c4 == memoryPage) {
                UnknownMemoryStatus status;
                GlobalMemoryStatus(&status);
                field_0x38->UnknownFunction447fa0(memoryPage, "Memory");
                field_0x38->UnknownFunction447f40(memoryPage, "  Ours   in DirectX       Area");
                int count = g_MemTagStack->count;
                int ours = 0;
                int directx = 0;
                for (int i = 0; i < count; i++) {
                    field_0x38->UnknownFunction447f40(memoryPage, "%8d %8d %12s",
                                                      g_MemTagStack->ours[i],
                                                      g_MemTagStack->directx[i],
                                                      g_MemTagStack->names[i]);
                    ours += g_MemTagStack->ours[i];
                    directx += g_MemTagStack->directx[i];
                }
                field_0x38->UnknownFunction447f40(memoryPage, "%8d %8d %12s", ours, directx, "Totals");
                field_0x38->UnknownFunction447f40(memoryPage, "Grand Total    %10d", ours + directx);
                field_0x38->UnknownFunction447f40(memoryPage, "TotalPhys       %10d", status.totalPhys);
                field_0x38->UnknownFunction447f40(memoryPage, "AvailPhys       %10d", status.availPhys);
                field_0x38->UnknownFunction447f40(memoryPage, "TotalVirtual    %10d", status.totalVirtual);
                field_0x38->UnknownFunction447f40(memoryPage, "AvailVirtual    %10d", status.availVirtual);
                field_0x38->UnknownFunction447f40(memoryPage, "Memory Load     %8d %%", status.memoryLoad);
                field_0x38->UnknownFunction447f40(memoryPage, "%s", field_0x0c->field_0x5c0.description);
                field_0x38->UnknownFunction447f40(memoryPage, "Total VidMem    %10d", field_0x0c->field_0x54);
                field_0x38->UnknownFunction447f40(memoryPage, "IsAGP           %s",
                                                  field_0x0c->field_0x9f0 ? "TRUE" : "FALSE");
                field_0x38->UnknownFunction447f40(memoryPage, "VideoMemoryMB   %d",
                                                  UnknownVirtualSlot20("VideoMemoryMB", -1));
                field_0x38->UnknownFunction447f40(memoryPage, "PartialTexBlt   %s",
                                                  g_UnknownGlobal56e26c->field_0x0c->field_0x5bc ? "Yes" : "No");
                field_0x38->UnknownFunction447f40(memoryPage, "TexturesCached  %s",
                                                  g_UnknownGlobal56e26c->field_0x2d5_bit2 ? "Yes" : "No");
            }
        }
        s_LastFrameTime = now;
    }
    return 1;
}

// 0x004685c0
int Game::UnknownVirtualSlot9() {
    field_0x0c->UnknownVirtualSlot3();
    return 1;
}

// 0x00468880
void Game::UnknownFunction468880() {
    field_0x2d8 = field_0x2dc = UnknownFunction4bfa80();
}

// 0x004688a0
int Game::UnknownVirtualSlot11(int value) {
    if (field_0x08)
        return 0;
    field_0x2d5_bit0 = value;
    field_0x2f4->UnknownVirtualSlot16(value);
    return 1;
}

// 0x004688e0
int Game::UnknownVirtualSlot12(int value) {
    return field_0x2f4->UnknownVirtualSlot19(value) != 0;
}

// 0x00468900: releases go to the interface unless bit 0 of +0x2d5 is set.
int Game::UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (field_0x2d5_bit0)
        return 1;
    return field_0x2f4->UnknownVirtualSlot22(event, entry) != 0;
}

// 0x00468930: presses go to the interface unless bit 0 of +0x2d5 is set.
// Control 0x20 also drives the +0x38 object; with bit 0 of +0x2d4, Ctrl+F
// (key 0x21 with modifier 0xc) toggles +0x1c4 and E (0x12) clears it,
// setting +0x1c8.
int Game::UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (field_0x2d5_bit0)
        return 1;
    if (field_0x2d4_bit2 && field_0x38 && UnknownFunction43caa0(0x20, 0, event, 0x80)) {
        if (field_0x38->field_0x25_bit0) {
            if (field_0x38->UnknownFunction4484f0())
                field_0x38->UnknownVirtualSlot4();
        } else {
            field_0x38->UnknownVirtualSlot5();
        }
    }
    if (field_0x2d4_bit0) {
        if (field_0x14->UnknownVirtualSlot3(0x21, 0, 0xc, 0)) {
            field_0x1c4 = 1 - field_0x1c4;
        } else if (field_0x14->UnknownVirtualSlot3(0x12, 0, 0x3f, 0) && field_0x1c4) {
            field_0x1c8 = 1;
            field_0x1c4 = 0;
        }
    }
    return field_0x2f4->UnknownVirtualSlot23(event, entry) != 0;
}

// 0x00468a10
Game::~Game() {
    if (s_UnknownFile65b4ac)
        fclose(s_UnknownFile65b4ac);
}

// 0x00468a30: tears down the interface, the network object, the owners and
// the ControlInterface.
int Game::UnknownVirtualSlot15() {
    field_0x2d5_bit1 = 1;
    if (g_UnknownGlobal56c470 && field_0x38 && g_UnknownGlobal56c470->field_0x2c)
        g_UnknownGlobal56c470->UnknownFunction4691f0();
    g_UnknownStatic65b478.UnknownFunction4677c0();
    if (field_0x2f4) {
        field_0x2f4->Release();
        field_0x2f4 = 0;
    }
    if (field_0x08) {
        delete field_0x08;
        field_0x08 = 0;
    }
    if (field_0x04) {
        delete field_0x04;
        field_0x04 = 0;
    }
    if (field_0x14) {
        delete field_0x14;
        field_0x14 = 0;
    }
    if (field_0x10) {
        delete field_0x10;
        field_0x10 = 0;
    }
    field_0x0c = 0;
    UnknownFunction52d0d0();
    return 1;
}

// 0x00468ae0
int Game::UnknownVirtualSlot16(int value) {
    field_0x08 = new(__FILE__, 979) NetworkInterface;
    if (field_0x08 && field_0x08->Initialize(value) < 0) {
        delete field_0x08;
        field_0x08 = 0;
        return 0;
    }
    return field_0x08 != 0;
}

// 0x00468ba0: hands a network message to the +0x2f4 object.
int Game::UnknownVirtualSlot17(int type, void* data, int from, int to, int flags) {
    return field_0x2f4->UnknownVirtualSlot24(type, data, from, to, flags) != 0;
}

// 0x00468bd0: `path` = the +0x1cc directory, "\\" and `name`.
int Game::UnknownVirtualSlot18(const char* name, char* path) {
    strcpy(path, field_0x1cc);
    strcat(path, "\\");
    strcat(path, name);
    return 1;
}

// 0x00468c60
int Game::UnknownVirtualSlot30(int, char* text) {
    strcpy(text, "No Strings Available");
    return 0;
}

// 0x00468c90 (identical code for slot 4)
int Game::UnknownVirtualSlot3() {
    return 1;
}

int Game::UnknownVirtualSlot4() {
    return 1;
}

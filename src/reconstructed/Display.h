#pragma once

#include "DisplayMode.h"
#include "RenderInterfaces.h"

// The display object: Game+0x0c, also kept at RenderTarget+0x04 (PCGame
// slot 31 hands it to PCRenderTarget 0x004c4f80, which stores it through
// 0x004e8ca0). Game slot 9 calls its slot 3; RenderTarget 0x004e8cc0 reads
// +0x6c; Camera 0x0042e550 reads the current mode's size; Game slot 8
// prints its mode and memory members. Its class is not established.
struct UnknownDisplay {
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    // PCGame slot 32: width, height, depth, 2, 0, windowed, 1.
    virtual int UnknownVirtualSlot2(int width, int height, int bitDepth, int a, int b,
                                    int windowed, int c);
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4(int value);  // called with 0 and 1 around a frame
    int UnknownFunction4c9c90();                  // 0x004c9c90: full-screen setup
    int UnknownFunction4c9d20(int x, int y, int width, int height); // 0x004c9d20: windowed

    int field_0x04;
    int field_0x08;                               // display mode count
    int field_0x0c;                               // current display mode
    UnknownDisplayMode* field_0x10;
    unsigned char field_0x14[0x54 - 0x14];
    int field_0x54;                               // "Total VidMem"
    unsigned char field_0x58[0x6c - 0x58];
    int field_0x6c;      // freezes RenderTarget's frame index (0x004e8cc0)
    unsigned char field_0x70;                     // bit 2 gates slot 4
    unsigned char field_0x71[0x78 - 0x71];
    int field_0x78;
    unsigned char field_0x7c[0x19c - 0x7c];
    UnknownSurfaceInterface* field_0x19c;     // PCGame slot 5: IsLost/Restore
    unsigned char field_0x1a0[0x5bc - 0x1a0];
    int field_0x5bc;                              // "PartialTexBlt"
    unsigned char field_0x5c0[0x7c0 - 0x5c0];
    char field_0x7c0[0x9f0 - 0x7c0];              // description string
    int field_0x9f0;                              // "IsAGP"
    unsigned char field_0x9f4[0xb74 - 0x9f4];
    unsigned char field_0xb74;                    // bit 1: keep oversized modes (0x004c0760)
};

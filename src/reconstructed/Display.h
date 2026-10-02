#pragma once

#include "DisplayMode.h"
#include "RenderInterfaces.h"

class RenderTarget;

// 0x430-byte device identifier, saved whole as "DriverInfo\\<name>\\
// DeviceIdentifier" (the DDDEVICEIDENTIFIER2 size; inference).
struct UnknownDeviceIdentifier {
    char driver[0x200];
    char description[0x200];                      // Game slot 8 prints it
    unsigned long driverVersion[2];
    unsigned long vendorId;
    unsigned long deviceId;
    unsigned long subSysId;
    unsigned long revision;
    unsigned char deviceGuid[16];
    unsigned long whqlLevel;
    unsigned long field_0x42c;
};

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
    void UnknownFunction4ca790(int width, int height, int windowed); // 0x004ca790
    void UnknownFunction4ca900(int mode, int windowed);              // 0x004ca900
    int UnknownFunction4ca5a0(int* value, RenderTarget* target);     // 0x004ca5a0
    void UnknownFunction4c9b50();                                    // 0x004c9b50
    void UnknownFunction4cab00(RenderTarget* target);                // 0x004cab00: blit timing
    // 0x0052d250: sorts the modes and returns the index of the matching one.
    int UnknownFunction52d250(int width, int height, int bitDepth, int a, int b);

    int field_0x04;
    int field_0x08;                               // display mode count
    int field_0x0c;                               // current display mode
    UnknownDisplayMode* field_0x10;
    int field_0x14[16];                           // per texture kind (TextureMapManager 0x00511580)
    int field_0x54;                               // "Total VidMem"
    int field_0x58;
    int field_0x5c;
    int field_0x60;                               // "TextureCacheLimit" (0x7fffffff if unset)
    int field_0x64;                               // passed to the GUI setup (KrustyUI 0x004988a0)
    unsigned char field_0x68[0x6c - 0x68];
    int field_0x6c;      // freezes RenderTarget's frame index (0x004e8cc0)
    unsigned char field_0x70_bit0 : 1;            // "Use8BitTextures"
    unsigned char field_0x70_bit1 : 1;
    unsigned char field_0x70_bit2 : 1;            // gates slot 4 (Game slot 8)
    unsigned char field_0x70_bits : 5;
    unsigned char field_0x71[0x78 - 0x71];
    int field_0x78;
    unsigned char field_0x7c[0x190 - 0x7c];
    UnknownDirectDrawInterface* field_0x190;
    unsigned char field_0x194[0x19c - 0x194];
    UnknownSurfaceInterface* field_0x19c;     // PCGame slot 5: IsLost/Restore
    UnknownSurfaceInterface* field_0x1a0;     // PCGame slot 31's surface without +0x1a8
    int field_0x1a4;
    UnknownSurfaceInterface* field_0x1a8;
    unsigned char field_0x1ac[0x1b8 - 0x1ac];
    unsigned int field_0x1b8;                     // capability bits (PCGame 0x004c0d10: 0x1, 0x400)
    unsigned char field_0x1bc[0x4bc - 0x1bc];
    char field_0x4bc[0x100];                      // driver name (PCGame 0x004c1610)
    int field_0x5bc;                              // "PartialTexBlt"
    UnknownDeviceIdentifier field_0x5c0;
    int field_0x9f0;                              // "IsAGP"
    unsigned char field_0x9f4[0xb74 - 0x9f4];
    unsigned char field_0xb74_bit0 : 1;
    unsigned char field_0xb74_bit1 : 1;           // disabled in this mode; keeps all modes
    unsigned char field_0xb74_bits : 6;
};

// 0x0068a754 / 0x0068a764: the enumerated displays (PCGame 0x004c16b0
// profiles each one).
extern UnknownDisplay* g_UnknownDisplays68a754[4];
extern int g_UnknownDisplayCount68a764;

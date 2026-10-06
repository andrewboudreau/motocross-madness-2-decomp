#pragma once

#include "DisplayMode.h"
#include "Guid.h"
#include "RenderInterfaces.h"
#include "VideoCard.h"

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

// The display object: RTTI PCVideoCard : VideoCard (COL 0x0055e4a8, vtable
// 0x00556df4, 0xb78 bytes, constructor 0x004c9760; TU PCVideoCard.cpp,
// src/reconstructed/PCVideoCard.cpp). The stand-in name is kept for the
// existing bindings. Game+0x0c, also kept at RenderTarget+0x04 (PCGame
// slot 31 hands it to PCRenderTarget 0x004c4f80, which stores it through
// 0x004e8ca0). Game slot 9 calls its slot 3; RenderTarget 0x004e8cc0 reads
// +0x6c; Camera 0x0042e550 reads the current mode's size; Game slot 8
// prints its mode and memory members.
struct UnknownDisplay : public VideoCard {
    UnknownDisplay();                             // 0x004c9760
    // Implicit destructor 0x004c9820 (jumps to VideoCard's; deleting wrapper 0x004c9800).
    virtual void UnknownVirtualSlot1();           // 0x004c9ba0: releases everything
    virtual int UnknownVirtualSlot2(int width, int height, int bitDepth, int a, int b,
                                    int windowed, int c); // 0x004c9ec0
    virtual int UnknownVirtualSlot3();            // 0x004ca270: presents a frame
    virtual void UnknownVirtualSlot4(int value);  // 0x004ca4a0
    // 0x004c9830: creates the DirectDraw object for `guid` (0: the primary
    // display) and reads its identity, caps and modes; deletes itself and
    // returns 0 on failure.
    UnknownDisplay* UnknownFunction4c9830(UnknownGuid* guid, char* description, char* name,
                                          void* window);
    int UnknownFunction4c9c90();                  // 0x004c9c90: full-screen setup
    int UnknownFunction4c9d20(int x, int y, int width, int height); // 0x004c9d20: windowed
    int UnknownFunction4ca790(int width, int height, int windowed); // 0x004ca790
    int UnknownFunction4ca900(int mode, int windowed);              // 0x004ca900
    int UnknownFunction4ca5a0(int* value, RenderTarget* target);     // 0x004ca5a0
    void UnknownFunction4c9b50();                                    // 0x004c9b50: re-reads the caps
    void UnknownFunction4c9c10();                                    // 0x004c9c10: releases the surfaces
    int UnknownFunction4c9f30(int backBuffers);                      // 0x004c9f30: flipping chain
    int UnknownFunction4ca130(int backBuffers);                      // 0x004ca130
    int UnknownFunction4ca520(int windowed);                         // 0x004ca520
    void UnknownFunction4cab00(RenderTarget* target);                // 0x004cab00: blit timing
    void UnknownFunction4cb5b0(int enable);       // 0x004cb5b0 (GUIManager.cpp 0x004868b0)

    UnknownDirectDrawInterface* field_0x190;      // IDirectDraw7 (DirectDrawCreateEx)
    UnknownDirect3DInterface* field_0x194;        // IDirect3D7 (QueryInterface)
    void* field_0x198;
    UnknownSurfaceInterface* field_0x19c;     // primary; PCGame slot 5: IsLost/Restore
    UnknownSurfaceInterface* field_0x1a0;     // back buffer; PCGame slot 31's surface without +0x1a8
    UnknownGammaControlInterface* field_0x1a4;
    UnknownSurfaceInterface* field_0x1a8;     // windowed render surface
    UnknownClipperInterface* field_0x1ac;
    void* field_0x1b0;                            // window handle
    // Hardware caps (0x17c bytes, the DDCAPS layout; GetCaps).
    unsigned int field_0x1b4;                     // size
    unsigned int field_0x1b8;                     // capability bits (PCGame 0x004c0d10: 0x1, 0x400)
    unsigned int field_0x1bc;                     // caps2
    unsigned char field_0x1c0[0x1f0 - 0x1c0];
    unsigned int field_0x1f0;                     // total video memory
    unsigned char field_0x1f4[0x330 - 0x1f4];
    unsigned int field_0x330;                     // emulation caps (DDCAPS) from here
    unsigned char field_0x334[0x4ac - 0x334];
    UnknownGuid field_0x4ac;                      // DirectDraw driver GUID (0: primary)
    char field_0x4bc[0x100];                      // driver name: the GUID text (PCGame 0x004c1610)
    int field_0x5bc;                              // "PartialTexBlt"
    UnknownDeviceIdentifier field_0x5c0;
    int field_0x9f0;                              // "IsAGP"
    unsigned char field_0x9f4[0xa70 - 0x9f4];
    unsigned int field_0xa70;                     // cooperative level flags (8: normal)
    char field_0xa74[0x80];                       // enumeration description
    char field_0xaf4[0x80];                       // enumeration name
    unsigned char field_0xb74_bit0 : 1;           // "DriverInfo\<name>\WaitForFlip"
    unsigned char field_0xb74_bit1 : 1;           // disabled in this mode; keeps all modes
    unsigned char field_0xb74_bit2 : 1;           // "IsPowerVR"
    unsigned char field_0xb74_bit3 : 1;           // gamma ramp (cleared by caps2 0x100000)
    unsigned char field_0xb74_bits : 4;
};

// 0x0068a754 / 0x0068a764: the enumerated displays (PCGame 0x004c16b0
// profiles each one).
extern UnknownDisplay* g_UnknownDisplays68a754[4];
extern int g_UnknownDisplayCount68a764;

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
    UnknownDisplay* InitializeDisplay(UnknownGuid* guid, char* description, char* name,
        void* window);
    int SetFullscreenCooperativeLevel();                // 0x004c9c90: full-screen setup
    int SetWindowedCooperativeLevel(int x, int y, int width, int height); // 0x004c9d20: windowed
    int CreateWindowedSurfaces(int width, int height, int windowed); // 0x004ca790
    int SetFullscreenDisplayMode(int mode, int windowed); // 0x004ca900
    int ProbeNonLocalTextureMemory(int* value, RenderTarget* target); // 0x004ca5a0
    void RefreshDriverCaps();                           // 0x004c9b50: re-reads the caps
    void ReleasePrimarySurface();                       // 0x004c9c10: releases the surfaces
    int CreateFlipChain(int backBuffers);               // 0x004c9f30: flipping chain
    int CreateSystemRenderSurface(int backBuffers);     // 0x004ca130
    int CreateModeSurfaces(int windowed);               // 0x004ca520
    void ProbePartialTextureUploads(RenderTarget* target); // 0x004cab00: blit timing
    int SetGDISurfaceVisible(int enable);               // 0x004cb5b0 (GUIManager.cpp 0x004868b0)

    UnknownDirectDrawInterface* directDraw;    // +0x190: IDirectDraw7 (DirectDrawCreateEx)
    UnknownDirect3DInterface* direct3D;        // +0x194: IDirect3D7 (QueryInterface)
    void* field_0x198;
    UnknownSurfaceInterface* primarySurface;   // +0x19c: primary; PCGame slot 5: IsLost/Restore
    UnknownSurfaceInterface* backBuffer;       // +0x1a0: back buffer; PCGame slot 31's surface without +0x1a8
    UnknownGammaControlInterface* gammaControl; // +0x1a4
    UnknownSurfaceInterface* renderSurface;    // +0x1a8: windowed render surface
    UnknownClipperInterface* clipper;          // +0x1ac
    void* windowHandle;                        // +0x1b0: window handle
    // Hardware caps (0x17c bytes, the DDCAPS layout; GetCaps).
    unsigned int driverCapsSize;               // +0x1b4: size
    unsigned int driverCaps;                   // +0x1b8: capability bits (PCGame 0x004c0d10: 0x1, 0x400)
    unsigned int driverCaps2;                  // +0x1bc: caps2
    unsigned char field_0x1c0[0x1f0 - 0x1c0];
    unsigned int driverVideoMemory;            // +0x1f0: total video memory
    unsigned char field_0x1f4[0x330 - 0x1f4];
    unsigned int emulationCapsSize;            // +0x330: emulation caps (DDCAPS) from here
    unsigned char field_0x334[0x4ac - 0x334];
    UnknownGuid driverGuid;                    // +0x4ac: DirectDraw driver GUID (0: primary)
    char driverGuidText[0x100];                // +0x4bc: driver name: the GUID text (PCGame 0x004c1610)
    int partialTextureUploadResult;            // +0x5bc: "PartialTexBlt"
    UnknownDeviceIdentifier deviceIdentifier;  // +0x5c0
    int isAGP;                                 // +0x9f0: "IsAGP"
    unsigned char field_0x9f4[0xa70 - 0x9f4];
    unsigned int cooperativeLevel;             // +0xa70: cooperative level flags (8: normal)
    char driverDescription[0x80];              // +0xa74: enumeration description
    char driverName[0x80];                     // +0xaf4: enumeration name
    unsigned char waitForFlip : 1;             // +0xb74 bit 0: "DriverInfo\<name>\WaitForFlip"
    unsigned char keepAllDisplayModes : 1;     // +0xb74 bit 1: disabled in this mode; keeps all modes
    unsigned char isPowerVR : 1;               // +0xb74 bit 2: "IsPowerVR"
    unsigned char useGammaRamp : 1;            // +0xb74 bit 3: gamma ramp (cleared by caps2 0x100000)
    unsigned char field_0xb74_bits : 4;
};

// 0x0068a754 / 0x0068a764: the enumerated displays (PCGame 0x004c16b0
// profiles each one).
extern UnknownDisplay* g_UnknownDisplays68a754[4];
extern int g_UnknownDisplayCount68a764;

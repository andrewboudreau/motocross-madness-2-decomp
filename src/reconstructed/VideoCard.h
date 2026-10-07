#pragma once

#include "DisplayMode.h"

// cdecl 0x004bfa80: the current time stamp (also declared in Game.h).
unsigned int UnknownFunction4bfa80();

// RTTI: VideoCard (COL 0x0055fa28, vtable 0x00558d74), the base of
// PCVideoCard (UnknownDisplay in Display.h). TU VideoCard.cpp (literal
// __FILE__ "D:\\aardvark\\VC\\krusty2\\VideoCard.cpp", 0x0052d22a). Its
// table holds the deleting destructor (0x0052d200), slot 1 (0x0052d220) and
// three _purecall slots. The constructor 0x0052d180 sets +0x04..+0x18f;
// PCVideoCard's fields start at +0x190. Names are provisional.
class VideoCard {
public:
    VideoCard();                                  // 0x0052d180
    virtual ~VideoCard();                         // 0x0052d240 (deleting wrapper 0x0052d200)
    virtual void UnknownVirtualSlot1();           // 0x0052d220: frees the mode table
    // PCGame slot 32: width, height, depth, 2, 0, windowed, 1.
    virtual int UnknownVirtualSlot2(int width, int height, int bitDepth, int a, int b,
                                    int windowed, int c) = 0;
    virtual int UnknownVirtualSlot3() = 0;        // presents a frame
    virtual void UnknownVirtualSlot4(int value) = 0; // called with 0 and 1 around a frame

    // 0x0052d250: sorts the modes and returns the index of the matching one
    // (a zero `a`/`b` matches any mode field_0x10/refresh rate), or -1.
    int FindDisplayMode(int width, int height, int bitDepth, int a, int b);

    // Inline (PCVideoCard.cpp): time since the last present and the
    // shortest such time.
    void RecordFrameTime() {
        unsigned int now = UnknownFunction4bfa80();
        lastFrameTime = now - lastPresentTime;
        lastPresentTime = now;
        if (lastFrameTime < shortestFrameTime)
            shortestFrameTime = lastFrameTime;
    }

    int displayModeCapacity;                   // +0x04
    int displayModeCount;                      // +0x08: display mode count
    int currentDisplayMode;                    // +0x0c: current display mode (-1: none)
    UnknownDisplayMode* displayModes;          // +0x10: DebugRealloc'd mode table
    int field_0x14[16];                           // per texture kind (TextureMapManager 0x00511580)
    int totalVideoMemory;                      // +0x54: "Total VidMem"
    int field_0x58;
    int field_0x5c;
    int textureCacheLimit;                     // +0x60: "TextureCacheLimit" (0x7fffffff if unset)
    int field_0x64;                               // passed to the GUI setup (KrustyUI 0x004988a0)
    int field_0x68;
    int freezeFrameIndex;                      // +0x6c: freezes RenderTarget's frame index (0x004e8cc0)
    unsigned char use8BitTextures : 1;         // +0x70 bit 0: "Use8BitTextures"
    unsigned char field_0x70_bit1 : 1;
    unsigned char lastFlipFailed : 1;          // +0x70 bit 2: last flip failed; gates slot 4 (Game slot 8)
    unsigned char useFlipChain : 1;            // +0x70 bit 3: full-screen flipping chain (0x004ca900)
    unsigned char field_0x70_bits : 4;
    unsigned char field_0x71[0x74 - 0x71];
    int videoMemoryBudget;                     // +0x74: video memory: caps total plus the desktop
    int frameBufferCount;                      // +0x78: back buffers + 1
    unsigned int lastPresentTime;              // +0x7c: time stamp of the last present (0x004bfa80)
    unsigned int lastFrameTime;                // +0x80: last frame time
    unsigned int shortestFrameTime;            // +0x84: shortest frame time
    int field_0x88;
    int field_0x8c;
    int field_0x90[64];
};

// 0x0052d120 (cdecl qsort comparator): orders modes by depth, height,
// width, field_0x10, then refresh rate.
int CompareDisplayModes(const void* first, const void* second);

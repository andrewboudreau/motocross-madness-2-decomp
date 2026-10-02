#pragma once

#include "BaseObject.h"
#include "MemTag.h"
#include "RenderInterfaces.h"
#include "TextureMapManager.h"

// A render state and its value (RenderTarget slot 8).
struct UnknownRenderStatePair {
    int state;
    int value;
};

// RTTI: TextureMap : BaseObject (vtable 0x00558354; its code sits among
// Texmap.cpp's literals). Slots 4-20 are pure (`_purecall`). Only what
// PCTextureMap uses is declared; names are provisional.
class TextureMap : public BaseObject {
public:
    TextureMap(TextureMapManager* manager, int value); // 0x0050a4e0
    virtual ~TextureMap();                            // 0x0050ab40 (deleting wrapper 0x0050a570)
    virtual void UnknownVirtualSlot4() = 0;
    virtual void UnknownVirtualSlot5() = 0;
    virtual void UnknownVirtualSlot6() = 0;
    virtual int UnknownVirtualSlot7() = 0;
    virtual int UnknownVirtualSlot8(int a, int b, int c) = 0;
    virtual void UnknownVirtualSlot9() = 0;
    virtual void UnknownVirtualSlot10() = 0;
    virtual int UnknownVirtualSlot11() = 0;
    virtual void UnknownVirtualSlot12() = 0;
    virtual void* UnknownVirtualSlot13(void* rect, long* pitch, int flags) = 0;
    virtual int UnknownVirtualSlot14(void* rect) = 0;
    virtual void UnknownVirtualSlot15() = 0;
    virtual void* UnknownVirtualSlot16(int level) = 0;
    virtual int UnknownVirtualSlot17(int level) = 0;
    virtual void UnknownVirtualSlot18() = 0;
    virtual void UnknownVirtualSlot19() = 0;
    virtual void UnknownVirtualSlot20() = 0;

    int field_0x08;
    int field_0x0c;
    TextureMapManager* field_0x10;            // manager (constructor argument)
    int field_0x14;                           // width
    int field_0x18;                           // height
    int field_0x1c;
    int field_0x20;                           // pixel format
    int field_0x24;                           // 1: no mip levels
    unsigned char field_0x28[0x44 - 0x28];
    int field_0x44;                           // render-state pair count
    UnknownRenderStatePair field_0x48[4];     // applied by slot 19 (length not established)
    unsigned char field_0x68[0x70 - 0x68];
};

// Object at PCTextureMap+0x7c; its destructor 0x0052d050 sits among
// vfwdeco.cpp's literals.
class UnknownVideoDecoder {
public:
    ~UnknownVideoDecoder();                   // 0x0052d050
};

// cdecl 0x00511970 (Tgafile.cpp): bytes per pixel of a pixel format.
int UnknownFunction511970(int format);

// Global at 0x00689964; when set, PCTextureMap tells the manager
// (0x00511580) about lost or released surfaces.
extern int g_UnknownGlobal689964;

// RTTI: PCTextureMap : TextureMap (vtable 0x00555fec; its code sits among
// PCTexMap.cpp's literals). Names are provisional.
class PCTextureMap : public TextureMap {
public:
    PCTextureMap(TextureMapManager* manager, int value); // 0x004c5f00
    virtual ~PCTextureMap();                  // 0x004c5f50 (deleting wrapper 0x004c5f30)
    virtual void UnknownVirtualSlot4();       // 0x004c69c0
    virtual void UnknownVirtualSlot5();       // 0x004c6080
    virtual void UnknownVirtualSlot6();       // 0x004c71c0
    virtual int UnknownVirtualSlot7();        // 0x004c7470: whether +0x74 exists
    virtual int UnknownVirtualSlot8(int a, int b, int c); // 0x004c7480
    virtual void UnknownVirtualSlot9();       // 0x004c7640
    virtual void UnknownVirtualSlot10();      // 0x004c7610: releases the texture surface
    virtual int UnknownVirtualSlot11();       // 0x004c7970: sets it as texture stage 0
    virtual void UnknownVirtualSlot12();      // 0x004c6040: restores a lost surface
    virtual void* UnknownVirtualSlot13(void* rect, long* pitch, int flags); // 0x004c79e0: locks +0x70
    virtual int UnknownVirtualSlot14(void* rect); // 0x004c7a50: unlocks +0x70
    virtual void UnknownVirtualSlot15();      // 0x004c77d0
    virtual void* UnknownVirtualSlot16(int level); // 0x004c7a70: locks a mip level
    virtual int UnknownVirtualSlot17(int level); // 0x004c7ad0: unlocks it
    virtual void UnknownVirtualSlot18();      // 0x004c81d0
    virtual void UnknownVirtualSlot19();      // 0x004c79a0: binds and applies the render states
    virtual void UnknownVirtualSlot20();      // 0x004c8430

    int UnknownFunction4c7420();              // 0x004c7420: recreates a lost texture surface
    // 0x004c7b00: blits +0x70 into `destination` unless `skip`.
    int UnknownFunction4c7b00(void* destinationRect, UnknownSurfaceInterface* destination,
                              void* sourceRect, int flags, int skip);
    UnknownSurfaceInterface* UnknownFunction4c83a0(int level); // 0x004c83a0: mip level surface

    UnknownSurfaceInterface* field_0x70;      // system-memory surface (counted in DirectX memory)
    UnknownSurfaceInterface* field_0x74;      // texture surface
    int field_0x78;
    UnknownVideoDecoder* field_0x7c;
};

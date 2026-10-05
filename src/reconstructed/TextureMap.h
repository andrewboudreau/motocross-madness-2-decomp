#pragma once

#include "BaseObject.h"
#include "TextureMapManager.h"
#include "Tgafile.h"

// Palette object at TextureMap+0x2c: +0x710 maps 555 colours to palette
// indices.
struct UnknownTexturePalette {
    unsigned char field_0x000[0x10];
    unsigned char field_0x010[256][3];             // RGB entries
    unsigned char field_0x310[0x710 - 0x310];
    unsigned char field_0x710[0x8000];
};

// Formats slots 4 and 5 pick between: +0x0c without alpha, +0x10 with
// alpha. +0x14 is how many times slot 5 halves textures wider than 32.
struct UnknownTextureFormatChoice {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
};

// The stream textures load from: 0x00461600 returns the read position,
// 0x00461640 reads `count` items of `size` bytes and returns how many it
// read. Both follow +0x1c to the innermost stream first.
class UnknownTextureStream {
public:
    UnknownTextureStream(int a);              // 0x00460d10
    ~UnknownTextureStream();                  // 0x00460d60
    int UnknownFunction460f50(const char* path, const char* mode, int a); // 0x00460f50: opens `path`
    int UnknownFunction461340(int offset, int a, int origin); // 0x00461340: seeks
    int UnknownFunction461600();
    int UnknownFunction461640(void* buffer, int size, int count);

    unsigned char field_0x000[0x1c];
    UnknownTextureStream* field_0x1c;         // inner stream
    unsigned char field_0x020[0x130 - 0x20];
    int field_0x130;                          // start offset in the inner stream
};

// A render state and its value (RenderTarget slot 8).
struct UnknownRenderStatePair {
    int state;
    int value;
};

// RTTI: TextureMap : BaseObject (vtable 0x00558354; its code sits among
// Texmap.cpp's literals). Slots 4-20 are pure (`_purecall`). Names are
// provisional.
class TextureMap : public BaseObject {
public:
    // 0x0050a4e0: registers with `manager` (0x005112f0) when `registered`.
    TextureMap(TextureMapManager* manager, int registered);
    virtual ~TextureMap();                            // 0x0050ab40 (deleting wrapper 0x0050a570)
    virtual int UnknownVirtualSlot4(void* bits, int width, int height, int stride, int minimumSize,
                                    int sourceFormat, int format, UnknownTexturePalette* palette,
                                    int flags, void* surfacePalette, int checkMemory, int unused,
                                    int addressU, int addressV, UnknownTextureFormatChoice* choice,
                                    int alphaThreshold, unsigned int key) = 0;
    virtual int UnknownVirtualSlot5(UnknownTextureStream* stream, int width, int height, int minimumSize,
                                    int fileFormat, int dataSize, int format, UnknownTexturePalette* palette,
                                    int flags, void* surfacePalette, int addressU, int addressV,
                                    UnknownTextureFormatChoice* choice, int alphaThreshold, unsigned int key) = 0;
    virtual TextureMap* UnknownVirtualSlot6() = 0;
    virtual int UnknownVirtualSlot7() = 0;
    virtual int UnknownVirtualSlot8(int a, int b, int c) = 0;
    virtual int UnknownVirtualSlot9(struct UnknownRect* rect, int mode) = 0;
    virtual void UnknownVirtualSlot10() = 0;
    virtual int UnknownVirtualSlot11() = 0;
    virtual void UnknownVirtualSlot12() = 0;
    virtual void* UnknownVirtualSlot13(void* rect, long* pitch, int flags) = 0;
    virtual int UnknownVirtualSlot14(void* rect) = 0;
    virtual int UnknownVirtualSlot15(int filter) = 0;
    virtual void* UnknownVirtualSlot16(int level) = 0;
    virtual int UnknownVirtualSlot17(int level) = 0;
    virtual int UnknownVirtualSlot18(unsigned int color) = 0;
    virtual void UnknownVirtualSlot19() = 0;
    virtual int UnknownVirtualSlot20() = 0;

    // 0x0050abd0: sets the address-mode render states (0x13, 0x14) of an
    // alpha texture; 0 for other formats.
    int UnknownFunction50abd0(int addressU, int addressV);

    TextureMap* field_0x08;                   // next in the manager's list
    TextureMap* field_0x0c;                   // previous in the manager's list
    TextureMapManager* field_0x10;            // manager (constructor argument)
    int field_0x14;                           // width
    int field_0x18;                           // height
    int field_0x1c;
    int field_0x20;                           // pixel format
    int field_0x24;                           // mip level count
    BaseObject* field_0x28;                   // released by the destructor
    UnknownTexturePalette* field_0x2c;
    int field_0x30;                           // has a colour key
    int field_0x34;                           // colour key (also +0x38)
    int field_0x38;
    int field_0x3c;                           // from Pixtrans 0x004d24d0 (slot 4)
    int field_0x40;
    int field_0x44;                           // render-state pair count
    UnknownRenderStatePair field_0x48[4];     // applied by slot 19 (length not established)
    int field_0x68;                           // bit 0: a ManagedTexture
    int field_0x6c;                           // format choice +0x14 (slot 5)
};


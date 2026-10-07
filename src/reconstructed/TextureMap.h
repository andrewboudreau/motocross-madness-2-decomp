#pragma once

#include <stdio.h>

#include "BaseObject.h"
#include "TextureMapManager.h"
#include "Tgafile.h"

// Palette object at TextureMap+0x2c: +0x710 maps 555 colours to palette
// indices.
struct UnknownTexturePalette {
    // ColorMapper's table getters (Quantize.h): Pixtrans.cpp calls them on
    // its palette argument.
    unsigned char* UnknownFunction4de270();        // 0x004de270: &field_0x010
    unsigned char* UnknownFunction4de280();        // 0x004de280: &field_0x710 (555 to index)
    unsigned char* UnknownFunction4de290();        // 0x004de290: &field_0x8710 (565 to index)

    unsigned char field_0x000[0x10];
    unsigned char field_0x010[256][3];             // RGB entries
    unsigned short field_0x310[256];               // 16-bit entries (Pixtrans 0x004d11c0)
    unsigned short field_0x510[256];               // 16-bit entries (Pixtrans 0x004d1150)
    unsigned char field_0x710[0x8000];
};

// Formats slots 4 and 5 pick between: +0x0c without alpha, +0x10 with
// alpha. +0x14 is how many times slot 5 halves textures wider than 32.
// 0x0050a590 also takes the manager (+0x00) and the ManagedTextureGroups
// for textures without (+0x04) and with (+0x08) alpha from it.
struct UnknownTextureFormatChoice {
    TextureMapManager* field_0x00;
    ManagedTextureGroup* field_0x04;
    ManagedTextureGroup* field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
};

// The stream textures load from: 0x00461600 returns the read position,
// 0x00461640 reads `count` items of `size` bytes and returns how many it
// read. Both follow +0x1c to the innermost stream first.
class UnknownResourceManager;

class UnknownTextureStream {
public:
    UnknownTextureStream(int a);              // 0x00460d10
    ~UnknownTextureStream();                  // 0x00460d60
    int UnknownFunction460f50(const char* path, const char* mode, int a); // 0x00460f50: opens `path`
    int UnknownFunction461340(int offset, int a, int origin); // 0x00461340: seeks
    int UnknownFunction461600();
    int UnknownFunction461640(void* buffer, int size, int count);
    int UnknownFunction461980();              // 0x00461980: reads a byte
    int UnknownFunction461aa0(char* buffer, int size); // 0x00461aa0: reads a line (CarProcedural.cpp)
    void UnknownFunction461cb0(int* a, int* b); // 0x00461cb0 (SceneManager 0x004ea0fd)
    // 0x00460e70: 1 when `path` opens and passes 0x00460db0's check; the
    // file is closed again (ResourceManager.cpp passes it to 0x00460f50).
    int UnknownFunction460e70(const char* path);
    // 0x00460d70: the archive holding `name` (its offset and length go to
    // +0x130 and +0x04), or 0 (FileStream.cpp).
    UnknownTextureStream* UnknownFunction460d70(const char* name);
    int UnknownFunction460db0();              // 0x00460db0: checks the "FAOE" header
    int UnknownFunction461310(int unused);    // 0x00461310: binary mode
    int UnknownFunction4618e0(const void* buffer, int size, int count); // 0x004618e0: writes
    int UnknownFunction461a60(int c);         // 0x00461a60: writes a byte
    int UnknownFunction461b90(const char* path); // 0x00461b90: writes the header, keyed by the name
    void UnknownFunction461d20();             // 0x00461d20: restarts the key
    // 0x0043e9e0 (out-of-line copy): the text-mode byte.
    char UnknownFunction43e9e0()
    {
        if (field_0x1c)
            return field_0x1c->UnknownFunction43e9e0();
        return field_0x01;
    }
    // 0x0043e9b0 (out-of-line copy): sets the text-mode byte.
    void UnknownFunction43e9b0(char mode)
    {
        if (field_0x1c)
            field_0x1c->UnknownFunction43e9b0(mode);
        else
            field_0x01 = mode;
    }
    // 0x004e9960 (out-of-line copy after ResourceManager.cpp's code):
    // field_0x08 of the innermost stream.
    int UnknownFunction4e9960()
    {
        if (field_0x1c)
            return field_0x1c->UnknownFunction4e9960();
        return field_0x08;
    }
    // 0x00430ff0 (out-of-line copy): whether the stream is at its end: past
    // its length within the inner stream, else the buffer is drained and the
    // file is at end-of-file.
    int UnknownFunction430ff0()
    {
        if (field_0x1c) {
            if (field_0x04 > 0 && UnknownFunction461600() >= field_0x130 + field_0x04)
                return 1;
            return field_0x1c->UnknownFunction430ff0();
        }
        if (field_0x124 != field_0x128)
            return 0;
        return feof(field_0x14);
    }

    char field_0x00;                          // 'A', second byte of the "FAOE" header
    char field_0x01;                          // text mode; the running key of an encoded file
    char field_0x02;                          // 'F', first header byte
    char field_0x03;                          // key derived from the file name (0x00461b90)
    int field_0x04;                           // length within the inner stream (0: to its end)
    int field_0x08;                           // 1: the file is encoded
    int field_0x0c;
    int field_0x10;                           // header size before the data (4 when encoded)
    FILE* field_0x14;                         // the open file
    char field_0x18;                          // 'E', fourth header byte
    unsigned char field_0x019[0x1c - 0x19];
    UnknownTextureStream* field_0x1c;         // inner stream
    char field_0x20;                          // 'O', third header byte
    char field_0x21[0x124 - 0x21];            // read buffer
    char* field_0x124;                        // buffer position
    char* field_0x128;                        // buffer end
    UnknownResourceManager* field_0x12c;      // the archives (constructor argument)
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

class Palette8;

// 0x0050a590 (cdecl): the texture `name` (format `format`), shared through
// the resource manager (0x00572b44; AddRef on reuse when `addRef`). With an
// archive entry and `fromArchive` it reads the file format, data size,
// width, height and a palette name from the entry's stream and loads it
// through slot 5; otherwise it loads the TGA file (0x005125c0), halves it
// while wider than 32 up to choice+0x14 times and loads it through slot 4.
// A ManagedTexture when `choice` has a group for the format, else a
// PCTextureMap; 0 on failure.
TextureMap* UnknownFunction50a590(TextureMapManager* manager, const char* name, int format,
                                  Palette8* palette, int flags, int addressU, int addressV,
                                  UnknownTextureFormatChoice* choice, int alphaThreshold,
                                  unsigned int key, int addRef, int fromArchive);

#pragma once

// Cube.h -- types of D:\aardvark\VC\krusty2\cube.cpp (__FILE__ literal at
// 0x0056888c, xrefs 0x0043d4ff and 0x0043d55c in 0x0043d460, lines 0x2c3 and
// 0x2c8).
//
// Confirmed (tier 1): RTTI .?AVCube@@ : BaseObject (COL 0x0055b190), vtable
// 0x00551288 with four slots: the deleting destructor 0x0043d210 and
// BaseObject's AddRef/Release/GetRefCount. The constructor 0x0043d1f0 and
// the destructor 0x0043d6c0 write that vptr.
//
// Extent (strong inference): 0x0043d080..0x0043d88f. 0x0043d080 (declared
// in ControlInterface.h, matched in ControlInterface.cpp) is called only by
// 0x0043d460 here and by cubedraw.cpp 0x0043e179, never by ControlInterface
// code, so it most likely opens this file. The four vector initializers
// 0x0043d750..0x0043d88b build 0x00579870 (zero), 0x00579880 (x),
// 0x00579890 (y) and 0x00579860 (z); 0x0043d110 reads 0x00579870, so they
// are this file's copies of the header constants (their .CRT$XCU entries
// 0x0056611c..0x00566128 are contiguous) and sit after the functions, as in
// Griddraw.cpp. cubedraw.cpp starts with its empty initializers at 0x0043d890.
//
// A Cube is six faces (0xf8 bytes each) of up to sixteen textures read from
// a stream. Names below are provisional (tier 3).

#include "BaseObject.h"
#include "MatrixUtil.h"

class TextureMap;
class TextureMapManager;
class ManagedTextureGroup;
class UnknownTextureStream;
struct UnknownTexturePalette;

// cdecl 0x0043d080 (ControlInterface.h declares the same function): maps
// the 2-bit field `index` of byte `byteIndex` of `bits` to 0x200, 0x40,
// 0x80 or 0x100. Cube uses it as a face texture's size.
int UnknownFunction43d080(int bits, int index, int byteIndex);

// The texture context 0x0043d460 receives: the manager the textures belong
// to and, when set, the group that takes ManagedTextures. The same pointer
// is handed on as PCTextureMap slot 5's format-choice argument.
struct UnknownCubeTextureContext {
    TextureMapManager* field_0x00;
    ManagedTextureGroup* field_0x04;
};

// One face, 0xf8 bytes (stride from 0x0043d110, 0x0043d230, 0x0043d460).
struct UnknownCubeFace {
    int field_0x00;                     // 2-bit size codes (0x0043d080)
    unsigned short field_0x04;          // bit j: texture j is present
    unsigned short field_0x06;
    TextureMap* field_0x08[16];         // textures (released by the destructor)
    int field_0x48[16];                 // read when the flags have bit 0
    unsigned short field_0x88[16];      // 0xffff; read when the flags have bit 2
    int field_0xa8[16];                 // -1
    unsigned short field_0xe8;          // bit j: cell j is on screen (cubedraw.cpp 0x0043e330)
    unsigned short field_0xea;          // field_0xe8.. are not reset by 0x0043d110
    int field_0xec;
    int field_0xf0;
    int field_0xf4;
};

// RTTI: Cube : BaseObject, 0x614 bytes (DrawableCube's loader allocates
// 0x614 at cubedraw.cpp line 0x6d).
class Cube : public BaseObject {
public:
    Cube();                                     // 0x0043d1f0
    virtual ~Cube();                            // 0x0043d6c0 (deleting wrapper 0x0043d210)

    void UnknownFunction43d110();               // 0x0043d110: resets every member
    // 0x0043d230: reads the header from `stream`; releases itself and
    // returns 0 on a short read (or when `a` is nonzero).
    Cube* UnknownFunction43d230(UnknownTextureStream* stream, int a, ManagedTextureGroup* group,
                                int baseOffset);
    // 0x0043d400: copies whichever of the three vectors are given.
    int UnknownFunction43d400(const Vector3* a, const Vector3* b, const Vector3* c);
    // 0x0043d460: creates texture `index` of face `face` and reads it.
    int UnknownFunction43d460(int face, int index, UnknownTexturePalette* palette,
                              UnknownCubeTextureContext* context);
    // 0x0043d630: seeks to the texture data and reads every missing texture.
    int UnknownFunction43d630(UnknownTexturePalette* palette, UnknownCubeTextureContext* context);

    UnknownTextureStream* field_0x08;
    int field_0x0c;                             // file format (Tgafile.h)
    short field_0x10;                           // bit 1: textures are 0x800-aligned
    unsigned short field_0x12;
    Vector3 field_0x14;
    Vector3 field_0x20;
    Vector3 field_0x2c;
    UnknownCubeFace field_0x38[6];
    int field_0x608;                            // texture data offset in the stream
    int field_0x60c;                            // alignment base
    ManagedTextureGroup* field_0x610;
};

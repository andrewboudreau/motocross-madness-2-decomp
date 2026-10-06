#pragma once

// CubeDraw.h -- types of D:\aardvark\VC\krusty2\cubedraw.cpp (__FILE__
// literal at 0x005688cc, xrefs 0x0043da8f, 0x0043db82 and 0x0043e2ca).
//
// Confirmed (tier 1): RTTI .?AVDrawableCube@@ : GameObject : BaseObject
// (COL 0x0055b1e0), vtable 0x0055129c written by the constructor 0x0043d910
// and the destructor 0x0043e290. It overrides slot 0 (deleting destructor
// 0x0043d960), slot 12 (0x0043e850), slot 13 (the shared `return 1` body
// 0x00467ae0) and slot 14 (0x0043e4c0).
//
// Extent (strong inference): 0x0043d890..0x0043e9ff. The four empty
// initializers 0x0043d890..0x0043d900 open it; their .CRT$XCU entries
// (0x0056613c..0x00566148) follow those of the vector initializers
// 0x0043e870..0x0043e9ab (0x0056612c..0x00566138) with no other file's
// between them, and cube.cpp's entries (0x0056611c..0x00566128) come just
// before. So 0x0043e870..0x0043e9ab are most likely this file's header
// constants emitted after its functions (Griddraw.cpp shows the same late
// placement, 0x004807c0), and the out-of-line Parameterblocks.h copies
// 0x0043e9b0 and 0x0043e9e0 are first called from 0x0043d980 here. cursor.cpp
// would then start with the GameCursor constructor 0x0043ea00.
//
// A DrawableCube is a sky box: six faces of a 4x4 grid of textured cells
// around the camera, drawn through the owner's RenderTarget. Names are
// provisional (tier 3).

#include "GameObject.h"
#include "MatrixUtil.h"

class Camera;
class ColorMapper;
class Cube;
class ManagedTextureGroup;
class RenderTarget;
class UnknownTextureStream;
struct UnknownCubeTextureContext;

// 0x20-byte lit vertex (FVF 0x1e2: position, reserved, diffuse, specular,
// one texture coordinate pair), the D3DLVERTEX layout.
struct UnknownCubeVertex {
    Vector3 position;
    int reserved;
    unsigned int diffuse;
    unsigned int specular;
    float tu;
    float tv;
};

// Per-cell ManagedTexture coordinate state (0x0057ea10, 96 entries): the
// scale and u/v offsets 0x00510910 maps the cell's vertices from.
struct UnknownCubeTextureMapping {
    float scale;
    float offsetU;
    float offsetV;
};

// The camera fields 0x0043e330 reads (Camera.h keeps them protected): the
// view matrix's third column (+0xb4, +0xc4, +0xd4) and the position.
struct UnknownCubeCameraView {
    unsigned char field_0x000[0xac];
    Matrix4 field_0xac;                       // view matrix
    unsigned char field_0x0ec[0x170 - 0xec];
    Vector3 field_0x170;                      // position
};

// The object at 0x00575a98 (TrackOverlay.h and Griddraw.h view other
// members of it). Tier 3 semantics.
class UnknownCubeClipper {
public:
    // 0x0052f0a0: transforms `count` points of `source` by `matrix`.
    void UnknownFunction52f0a0(Vector3* destination, Vector3* source, Matrix4* matrix, int count);
    // 0x0052f140: whether a face with `normal` can face the camera looking along `forward`.
    int UnknownFunction52f140(Camera* camera, Vector3* forward, Vector3* normal);
    // 0x0052f4d0: whether the quad of `points` a, b, c, d is on screen.
    int UnknownFunction52f4d0(Camera* camera, Vector3* points, int a, int b, int c, int d);
};
extern UnknownCubeClipper* g_UnknownCubeClipper575a98;

class DrawableCube : public GameObject {
public:
    explicit DrawableCube(int flags);         // 0x0043d910
    virtual ~DrawableCube();                  // 0x0043e290 (deleting wrapper 0x0043d960)
    virtual int UnknownVirtualSlot12();       // 0x0043e850: visibility, then texture use
    virtual int UnknownVirtualSlot13();       // 0x00467ae0 (shared `return 1` body)
    virtual int UnknownVirtualSlot14();       // 0x0043e4c0: draws the visible cells

    // 0x0043d980: reads the palettes and cubes listed in `stream`.
    DrawableCube* UnknownFunction43d980(void* value, UnknownTextureStream* stream,
                                        UnknownCubeTextureContext* context);
    int UnknownFunction43dc60();              // 0x0043dc60: builds the face geometry
    // 0x0043e0b0: loads the cube's textures and sets the cells' texture coordinates.
    int UnknownFunction43e0b0(Cube* cube, UnknownCubeTextureContext* context);
    int UnknownFunction43e210();              // 0x0043e210: records texture use
    void UnknownFunction43e330();             // 0x0043e330: marks the visible cells

    RenderTarget* Target() const { return (RenderTarget*)field_0x18; }

    Cube* field_0x2c;
    ColorMapper* field_0x30;                  // palette while loading
    ManagedTextureGroup* field_0x34;          // the context's group
    int field_0x38;
    int field_0x3c;
    int field_0x40;
};

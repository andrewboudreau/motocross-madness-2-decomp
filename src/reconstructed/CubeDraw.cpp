// CubeDraw.cpp -- reconstruction of D:\aardvark\VC\krusty2\cubedraw.cpp.
// See CubeDraw.h for the TU extent and the class. The geometry builder
// 0x0043dc60, 0x0043e0b0, 0x0043e330 and slot 14 0x0043e4c0 (with its static
// matrix and atexit thunk 0x0043e840) are near misses in
// samples/render/CubeDrawNearMisses.cpp.

#include <string.h>

#include "CubeDraw.h"

#include "Cube.h"
#include "DebugAlloc.h"
#include "ManagedTexture.h"
#include "Parameterblocks.h"
#include "Quantize.h"
#include "RenderTarget.h"
#include "TextureMap.h"

// The four vector constants that open many retail files. Strong inference:
// this file's copies are 0x00579920, 0x00579a60, 0x0057a200 and 0x005798a8,
// built by 0x0043e870..0x0043e9ab (see CubeDraw.h).
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x00579930: the current face's grid points relative to the camera.
static Vector3 s_viewPoints[25];
// 0x00579a70: the face normals.
static Vector3 s_faceNormals[6];
// 0x00579af8: each face's 5x5 grid of points.
static Vector3 s_facePoints[6][25];
// 0x0057a210: two triangles per cell, 16 cells per face.
static UnknownCubeVertex s_cellVertices[6][16][6];
// 0x0057ea10
static UnknownCubeTextureMapping s_cellMappings[6][16];
// 0x0057ee94 / 0x0057ee98: released by the destructor (line 0x192).
static void* s_UnknownGlobal57ee94;
static int s_UnknownGlobal57ee98;

// 0x0043d910
DrawableCube::DrawableCube(int flags) : GameObject(flags)
{
    field_0x2c = 0;
    field_0x30 = 0;
    field_0x40 = 0;
    memset(s_cellMappings, 0, sizeof(s_cellMappings));
    for (int i = 0; i < 96; i++)
        (&s_cellMappings[0][0])[i].scale = 1.0f;
}

// 0x0043d980
DrawableCube* DrawableCube::UnknownFunction43d980(void* value, UnknownTextureStream* stream,
                                                  UnknownCubeTextureContext* context)
{
    int offset;
    int paletteCount;
    int start;
    int cubeCount;
    char mode;
    int i;

    GameObject::UnknownVirtualSlot8(value);
    field_0x34 = context->field_0x04;
    if (!stream)
        goto fail;
    if (stream->field_0x1c)
        stream->UnknownFunction461340(stream->field_0x130, 0, 0);
    start = stream->UnknownFunction461600();
    if (stream->UnknownFunction461640(&paletteCount, 4, 1) != 1)
        goto fail;
    if (stream->UnknownFunction461640(&cubeCount, 4, 1) != 1)
        goto fail;
    for (i = 0; i < paletteCount; i++) {
        if (stream->UnknownFunction461640(&offset, 4, 1) != 1)
            goto fail;
        int position = stream->UnknownFunction461600();
        mode = ((UnknownParameterStream*)stream)->UnknownFunction43e9e0();
        if (stream->UnknownFunction461340(offset + start, 0, 1))
            goto fail;
        if (field_0x30)
            field_0x30->Release();
        field_0x30 = new (__FILE__, 0x5f) ColorMapper(stream);
        if (stream->UnknownFunction461340(position, 0, 1))
            goto fail;
        ((UnknownParameterStream*)stream)->UnknownFunction43e9b0(mode);
    }
    for (i = 0; i < cubeCount; i++) {
        if (stream->UnknownFunction461640(&offset, 4, 1) != 1)
            goto fail;
        int position = stream->UnknownFunction461600();
        mode = ((UnknownParameterStream*)stream)->UnknownFunction43e9e0();
        if (stream->UnknownFunction461340(offset + start, 0, 1))
            goto fail;
        if (field_0x2c)
            field_0x2c->Release();
        field_0x2c = (new (__FILE__, 0x6d) Cube)->UnknownFunction43d230(stream, 0, field_0x34, start);
        if (!field_0x2c)
            goto fail;
        if (stream->UnknownFunction461340(position, 0, 1))
            goto fail;
        ((UnknownParameterStream*)stream)->UnknownFunction43e9b0(mode);
    }
    UnknownFunction43dc60();
    UnknownFunction43e0b0(field_0x2c, context);
    field_0x30->Release();
    field_0x30 = 0;
    return this;
fail:
    Release();
    return 0;
}

// 0x0043e210
int DrawableCube::UnknownFunction43e210()
{
    for (int face = 0; face < 6; face++) {
        for (int i = 0; i < 16; i++) {
            if (field_0x34 && (field_0x2c->field_0x38[face].field_0xe8 & (1 << i))) {
                TextureMap* texture = field_0x2c->field_0x38[face].field_0x08[i];
                if (texture && (texture->field_0x68 & 1))
                    ((ManagedTexture*)texture)->UnknownFunction510820(7.0f);
            }
        }
    }
    return 1;
}

// 0x0043e290
DrawableCube::~DrawableCube()
{
    if (s_UnknownGlobal57ee94)
        operator delete(s_UnknownGlobal57ee94, __FILE__, 0x192);
    s_UnknownGlobal57ee94 = 0;
    s_UnknownGlobal57ee98 = 0;
    if (field_0x30)
        field_0x30->Release();
    if (field_0x2c)
        field_0x2c->Release();
}

// 0x0043e850
int DrawableCube::UnknownVirtualSlot12()
{
    UnknownFunction43e330();
    UnknownFunction43e210();
    return 1;
}

// 0x00467ae0 (shared `return 1` body)
int DrawableCube::UnknownVirtualSlot13()
{
    return 1;
}

// Cube.cpp -- reconstruction of D:\aardvark\VC\krusty2\cube.cpp. See Cube.h
// for the TU extent and the class.

#include "Cube.h"

#include "DebugAlloc.h"
#include "ManagedTexture.h"
#include "TextureMap.h"
#include "TextureMapManager.h"
#include "Tgafile.h"

// The four vector constants that open many retail files: 0x00579870,
// 0x00579880, 0x00579890 and 0x00579860, built by 0x0043d750..0x0043d88b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x0043d0d0: skips `stream` forward to the next 0x800 boundary past `base`.
static void UnknownFunction43d0d0(UnknownTextureStream* stream, int base)
{
    int padding = 0x800 - (stream->UnknownFunction461600() - base) % 0x800;
    if (padding != 0x800)
        stream->UnknownFunction461340(padding, 1, 1);
}

// 0x0043d110
void Cube::Reset()
{
    textureDataOffset = 0;
    field_0x610 = 0;
    field_0x08 = 0;
    fileFormat = 0;
    field_0x10 = 0;
    field_0x12 = 0xffff;
    field_0x14 = kVec3Zero;
    field_0x20 = kVec3Zero;
    field_0x2c = kVec3Zero;
    for (int face = 0; face < 6; face++) {
        field_0x38[face].sizeCodes = 0;
        field_0x38[face].presentTextures = 0;
        field_0x38[face].field_0x06 = 0;
        for (int i = 0; i < 16; i++) {
            field_0x38[face].textures[i] = 0;
            field_0x38[face].field_0x48[i] = 0;
            field_0x38[face].field_0x88[i] = 0xffff;
            field_0x38[face].field_0xa8[i] = -1;
        }
        field_0x38[face].field_0xec = 0;
        field_0x38[face].field_0xf0 = 0;
        field_0x38[face].field_0xf4 = 0;
    }
}

// 0x0043d1f0
Cube::Cube()
{
    Reset();
}

// 0x0043d230: reads the cube header from `stream`: the file format, the flags, the
// per-face texture masks and size codes, the three vectors, then (flags bit 2) the
// per-face 0x88 tables, (bit 0) the per-face 0x48 tables and (bit 1) the 0x800
// alignment skip; records the texture data offset. Returns 0 and releases the cube
// when a read fails or `a` is set. The three argument stores come in the order
// group, stream, baseOffset: VC6 hands the dead argument slots out in the order of
// the arguments' first use, and that order homes `flags` in the group slot, the
// face pointer in the stream slot and the face counter in the baseOffset slot
// (docs/VC6_FRAME_LAYOUT.md).
Cube* Cube::UnknownFunction43d230(UnknownTextureStream* stream, int a, ManagedTextureGroup* group,
                                  int baseOffset)
{
    short flags;
    int face;
    int i;

    field_0x610 = group;
    field_0x08 = stream;
    alignmentBase = baseOffset;
    if (stream->UnknownFunction461640(&fileFormat, 4, 1) != 1)
        goto fail;
    if (stream->UnknownFunction461640(&flags, 2, 1) != 1)
        goto fail;
    if (stream->UnknownFunction461640(&field_0x12, 2, 1) != 1)
        goto fail;
    for (face = 0; face < 6; face++) {
        if (stream->UnknownFunction461640(&field_0x38[face].presentTextures, 2, 1) != 1)
            goto fail;
    }
    for (face = 0; face < 6; face++) {
        if (stream->UnknownFunction461640(&field_0x38[face].sizeCodes, 4, 1) != 1)
            goto fail;
    }
    if (stream->UnknownFunction461640(&field_0x14, 12, 1) != 1)
        goto fail;
    if (stream->UnknownFunction461640(&field_0x20, 12, 1) != 1)
        goto fail;
    if (stream->UnknownFunction461640(&field_0x2c, 12, 1) != 1)
        goto fail;
    if (flags & 4) {
        for (face = 0; face < 6; face++) {
            if (stream->UnknownFunction461640(field_0x38[face].field_0x88, 0x20, 1) != 1)
                goto fail;
        }
    }
    if (flags & 1) {
        for (face = 0; face < 6; face++) {
            for (i = 0; i < 16; i++) {
                if (stream->UnknownFunction461640(&field_0x38[face].field_0x48[i], 4, 1) != 1)
                    goto fail;
            }
        }
    }
    if (flags & 2)
        UnknownFunction43d0d0(stream, alignmentBase);
    textureDataOffset = stream->UnknownFunction461600();
    if (a) {
    fail:
        Release();
        return 0;
    }
    return this;
}

// 0x0043d400
int Cube::UnknownFunction43d400(const Vector3* a, const Vector3* b, const Vector3* c)
{
    if (a)
        field_0x14 = *a;
    if (b)
        field_0x20 = *b;
    if (c)
        field_0x2c = *c;
    return 1;
}

// 0x0043d460
int Cube::LoadTexture(int face, int index, UnknownTexturePalette* palette,
                                UnknownCubeTextureContext* context)
{
    int size = UnknownFunction43d080(field_0x38[face].sizeCodes, index % 4, index / 4);
    int dataSize;
    if (IsCompressedFormat(fileFormat))
        field_0x08->UnknownFunction461640(&dataSize, 4, 1);
    else
        dataSize = BytesPerPixel(fileFormat) * size * size;

    if (context->field_0x04) {
        ManagedTexture* texture = new (__FILE__, 0x2c3) ManagedTexture(context->field_0x00);
        context->field_0x04->UnknownFunction50c6c0(texture);
        field_0x38[face].textures[index] = texture;
    } else {
        field_0x38[face].textures[index] = new (__FILE__, 0x2c8) PCTextureMap(context->field_0x00, 1);
    }
    if (field_0x38[face].textures[index]) {
        field_0x38[face].textures[index]->UnknownVirtualSlot5(
            field_0x08, size, size, 1, fileFormat, dataSize, 0, palette, 2, 0, 2, 1,
            (UnknownTextureFormatChoice*)context, 0x80, 0xff00ff);
        if (!field_0x610)
            field_0x38[face].textures[index]->UnknownVirtualSlot8(1, 0, 0);
    }
    if (field_0x10 & 2)
        UnknownFunction43d0d0(field_0x08, alignmentBase);
    return 1;
}

// 0x0043d630
int Cube::LoadMissingTextures(UnknownTexturePalette* palette, UnknownCubeTextureContext* context)
{
    field_0x08->UnknownFunction461340(textureDataOffset, 0, 1);
    for (int face = 0; face < 6; face++) {
        for (int i = 0; i < 16; i++) {
            if ((field_0x38[face].presentTextures & (1 << i)) && !field_0x38[face].textures[i])
                LoadTexture(face, i, palette, context);
        }
    }
    return 1;
}

// 0x0043d6c0
Cube::~Cube()
{
    for (int face = 5; face >= 0; face--) {
        for (int i = 15; i >= 0; i--) {
            if (field_0x38[face].textures[i]) {
                field_0x38[face].textures[i]->Release();
                field_0x38[face].textures[i] = 0;
            }
        }
    }
}

// Cube.cpp -- reconstruction of D:\aardvark\VC\krusty2\cube.cpp. See Cube.h
// for the TU extent and the class. The header reader 0x0043d230 is a near
// miss in samples/render/CubeNearMisses.cpp.

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
void Cube::UnknownFunction43d110()
{
    field_0x608 = 0;
    field_0x610 = 0;
    field_0x08 = 0;
    field_0x0c = 0;
    field_0x10 = 0;
    field_0x12 = 0xffff;
    field_0x14 = kVec3Zero;
    field_0x20 = kVec3Zero;
    field_0x2c = kVec3Zero;
    for (int face = 0; face < 6; face++) {
        field_0x38[face].field_0x00 = 0;
        field_0x38[face].field_0x04 = 0;
        field_0x38[face].field_0x06 = 0;
        for (int i = 0; i < 16; i++) {
            field_0x38[face].field_0x08[i] = 0;
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
    UnknownFunction43d110();
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
int Cube::UnknownFunction43d460(int face, int index, UnknownTexturePalette* palette,
                                UnknownCubeTextureContext* context)
{
    int size = UnknownFunction43d080(field_0x38[face].field_0x00, index % 4, index / 4);
    int dataSize;
    if (UnknownFunction511800(field_0x0c))
        field_0x08->UnknownFunction461640(&dataSize, 4, 1);
    else
        dataSize = UnknownFunction511740(field_0x0c) * size * size;

    if (context->field_0x04) {
        ManagedTexture* texture = new (__FILE__, 0x2c3) ManagedTexture(context->field_0x00);
        context->field_0x04->UnknownFunction50c6c0(texture);
        field_0x38[face].field_0x08[index] = texture;
    } else {
        field_0x38[face].field_0x08[index] = new (__FILE__, 0x2c8) PCTextureMap(context->field_0x00, 1);
    }
    if (field_0x38[face].field_0x08[index]) {
        field_0x38[face].field_0x08[index]->UnknownVirtualSlot5(
            field_0x08, size, size, 1, field_0x0c, dataSize, 0, palette, 2, 0, 2, 1,
            (UnknownTextureFormatChoice*)context, 0x80, 0xff00ff);
        if (!field_0x610)
            field_0x38[face].field_0x08[index]->UnknownVirtualSlot8(1, 0, 0);
    }
    if (field_0x10 & 2)
        UnknownFunction43d0d0(field_0x08, field_0x60c);
    return 1;
}

// 0x0043d630
int Cube::UnknownFunction43d630(UnknownTexturePalette* palette, UnknownCubeTextureContext* context)
{
    field_0x08->UnknownFunction461340(field_0x608, 0, 1);
    for (int face = 0; face < 6; face++) {
        for (int i = 0; i < 16; i++) {
            if ((field_0x38[face].field_0x04 & (1 << i)) && !field_0x38[face].field_0x08[i])
                UnknownFunction43d460(face, i, palette, context);
        }
    }
    return 1;
}

// 0x0043d6c0
Cube::~Cube()
{
    for (int face = 5; face >= 0; face--) {
        for (int i = 15; i >= 0; i--) {
            if (field_0x38[face].field_0x08[i]) {
                field_0x38[face].field_0x08[i]->Release();
                field_0x38[face].field_0x08[i] = 0;
            }
        }
    }
}

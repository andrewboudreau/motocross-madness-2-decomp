// Near-miss cube.cpp candidate, kept out of src/reconstructed until it
// matches. See src/reconstructed/Cube.h.
//
// 0x0043d230 (452 bytes, 445/452 positions): the reads, the shared failure
// block and its placement match. VC6 here spills the face counter and the
// face pointer of the bit-0 loop into the dead `stream` and `baseOffset`
// argument slots the other way round from retail (retail: counter in
// `baseOffset`'s slot, pointer in `stream`'s). Declaration order, separate
// loop variables, an int `flags` and a pointer local do not change it; nor
// do a counter of its own for the bit-0 loop (top-level or block-scoped), a
// counter per loop, an explicit `int* p`/`UnknownCubeFace* f` walk, the
// `field_0x48 + i` spelling or a `return this` before the `fail` tail
// (docs/NEAR_MISS_INDEX.md, class a).

#include "../../src/reconstructed/Cube.h"

#include "../../src/reconstructed/TextureMap.h"

// Cube.cpp's file-static 0x0043d0d0, declared here with the same mangled name.
void UnknownFunction43d0d0(UnknownTextureStream* stream, int base);

// 0x0043d230
Cube* Cube::UnknownFunction43d230(UnknownTextureStream* stream, int a, ManagedTextureGroup* group,
                                  int baseOffset)
{
    short flags;
    int face;
    int i;

    field_0x610 = group;
    alignmentBase = baseOffset;
    field_0x08 = stream;
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

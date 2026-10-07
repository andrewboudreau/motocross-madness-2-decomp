// Near-miss Grid1.cpp candidates, kept out of src/reconstructed until they
// match. They use src/reconstructed/Grid1.h; the bindings are in
// Grid1NearMisses.bindings.json. Scores are matching bytes under
// vc6_o2_mt with relocations resolved.
//
// DrawableGridNodeSharedTextures slot 7 (0x0047d370, 172 bytes; 101 of 172):
// control flow, the bit-field extraction (shl 16 / sar 17 into a named
// index), the reloads of the extra block after the store and both render
// state calls match. Retail keeps `this` in esi, the extra block in edi and
// the record in ebp; VC6 here keeps `this` in edi, the extra block in ebp
// and the record in esi, and addresses the reloaded record as
// [blocks + offset + 0x18] instead of forming the pointer first. A plain
// cast of `extra`, inline accessors, a record reload through a named
// pointer, nested ifs, an early return and declaring the record before the
// extra block all leave the allocation unchanged.
//
// Slot 3 (0x0047caa0, 1728 bytes; ~104 positions of 1740, aligned
// instruction ratio 0.58): control flow, the jump table, the case
// fall-through chain, the per-block loop (nested ifs, else paths out of
// line), all calls and the x87 `scale` kept on the FPU stack to the final
// divide match. Differences: retail keeps a zero constant in ebp that is
// coalesced with `block` (re-zeroed by `xor ebp,ebp` at the loop exit and
// the in-loop retry), `stage` in ebx and the CSE'd extra pointer in edx;
// VC6 here spills `block` (one extra frame slot, 0x14) and caches the extra
// pointer in ebp. Declaring `block` without an initializer and the loop as
// `for (; block < 16; block++)` produces retail's zero/ebp coalescing
// (ratio 0.71) but reads an uninitialised variable, so it is not used.
// The stage ladder's fourth compare (retail keeps `fcomp g*4` with no
// branch after a speculative `stage = 3`) is folded away here.
//
// 0x0047d470 (783 bytes; 309 of 786): identical control flow and calls;
// retail keeps the row (y & 15) in eax, the head length in edx and the run
// line pointer in ebx (frame 0x834), VC6 here uses edx/ebx and spills the
// line pointer (frame 0x838). Writing the tile-span test as a subtraction
// (`last - first == 0`) is required for retail's `sub ecx,edx`; the
// equality form becomes xor/test. Inline row terms, unsigned x/y, a
// `tile += row * tileStride` reuse, and the order of the line/rest
// statements were tried without closing the gap.
//
// 0x0047d780 (977 bytes; 216 of 1008): the per-block variant (4 x 4 pixel
// cells, width / 4 tiles, four rows per run, a dead `done` accumulator that
// retail keeps). Retail computes the head source pointer before the
// `x & mask` test and reuses row * tileStride for the line pointer; texture
// in edi and tileSize in esi (swapped here); its inner row copies keep the
// destination in ebp/[esp+0x14].

#include "../../src/reconstructed/Grid1.h"

#include <string.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/Pixtrans.h"
#include "../../src/reconstructed/Tgafile.h"
#include "../../src/reconstructed/D3DConstants.h"

// 0x0047d370: selects and binds one block's texture.
void DrawableGridNodeSharedTextures::UnknownVirtualSlot7(int block)
{
    Grid1NodeExtra* x = Extra();
    Grid1BlockTexture* record = &x->blocks[block];
    int index = record->index;
    if (index == -1) {
        x->texture = 0;
    } else if (record->own) {
        x->texture = record->texture;
        if (Extra()->blocks[block].ageEntry.size)
            Terrain()->textureAgeManager->UnknownFunction401250(&Extra()->blocks[block].ageEntry);
    } else {
        x->texture = Terrain()->textures[index];
    }
    if (Extra()->texture) {
        Extra()->texture->UnknownVirtualSlot19();
    } else {
        Terrain()->renderer->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        Terrain()->renderer->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}

// 0x0056c034, defined in src/reconstructed/Grid1.cpp.
extern float g_grid1DetailDistance;

// 0x0066529c: textures the nodes built for themselves (slot 3).
int g_grid1OwnTextureCount;

// 0x0047caa0: picks the node's texture for the given distance. The detail
// (node size in pixels on screen) selects a stage: 0 borrows a part of the
// parent's node texture, 1 a part of the parent's block texture, 2 the
// node's own texture and 3 one texture per 4 x 4 block. A stage that is
// unavailable falls back to the next coarser one; textures are built only
// for the stage the detail asks for.
void DrawableGridNodeSharedTextures::UnknownVirtualSlot3(float distance)
{
    int block = 0;
    int keep = 0;
    ManagedTexture* previous = Extra()->texture;
    float detail;
    if (distance != 0.0f)
        detail = (float)((16 << shift) * Terrain()->field_0x70) / distance;
    else
        detail = 10000.0f;

    int stage;
    if (g_grid1DetailDistance * 0.0625f >= detail)
        stage = 0;
    else if (g_grid1DetailDistance * 0.25f >= detail)
        stage = 1;
    else if (detail <= g_grid1DetailDistance)
        stage = 2;
    else if (g_grid1DetailDistance * 4.0f >= detail)
        stage = 3;
    else
        stage = 3;

    int wanted = stage;
    if (((Grid1RenderHost*)Terrain()->renderer)->camera->field_0x200 > 0.5f && level == 0)
        wanted = stage > 2 ? 2 : stage;

    float scale;
    int index;
retry:
    switch (stage) {
    case 0:
        if (parent) {
            Grid1NodeExtra* p = ((DrawableGridNodeSharedTextures*)parent)->Extra();
            if (p->index != -1 && !p->own)
                goto parentTexture;
        }
    case 1:
        if (parent) {
            Grid1BlockTexture* blocks = ((DrawableGridNodeSharedTextures*)parent)->Extra()->blocks;
            if (blocks) {
                Grid1BlockTexture* record = &blocks[(((field_0x2a >> 2) & 0x30) | (field_0x2a & 0xc)) >> 2];
                index = record->index;
                if (index != -1 && !record->own)
                    goto parentBlockTexture;
            }
        }
    case 2:
        if (Extra()->index != -1) {
            if (!Extra()->own)
                goto sharedTexture;
            if (Extra()->textureFlags & 0x10000)
                goto ownTexture;
            if (stage == wanted)
                goto createTexture;
            stage = wanted;
            goto retry;
        }
    case 3:
        if (Extra()->blocks) {
            keep = data->b3;
            UnknownFunction483040();
            data->field_0x158 = 0.0f;
            data->field_0x15c = 0.0f;
            data->field_0x160 = 1.0f;
            Extra()->texture = Terrain()->textures[0];
            for (block = 0; block < 16; block++) {
                Grid1BlockTexture* record = &Extra()->blocks[block];
                if (record->index != -1) {
                    if (record->own) {
                        if (!(Extra()->textureFlags & (1 << block))) {
                            if (stage == wanted) {
                                if (!UnknownFunction481a20(block)) {
                                    Extra()->blocks[block].texture = 0;
                                } else {
                                    Extra()->blocks[block].texture = Terrain()->AcquireOwnedObject();
                                    UnknownFunction47d780(block, Extra()->blocks[block].texture);
                                    if (Extra()->blocks[block].texture->field_0x68 & 1)
                                        Extra()->blocks[block].texture->UnknownFunction510670();
                                    Extra()->textureFlags |= 1 << block;
                                    g_grid1OwnTextureCount++;
                                    Terrain()->textureAgeManager->UnknownFunction401050(
                                        &Extra()->blocks[block].ageEntry, (int (*)(void*))EvictBlockTexture,
                                        this, block, UnknownFunction511970(Terrain()->nodeTextureFormat) * 0x40000 / 3);
                                }
                            } else {
                                stage = wanted;
                                goto retry;
                            }
                        }
                    } else {
                        record->texture = Terrain()->textures[record->index];
                    }
                } else {
                    record->texture = 0;
                }
            }
            Extra()->texture = Terrain()->textures[0];
            scale = 4.0f;
            data->field_0x158 = 0.0f;
            data->field_0x15c = 0.0f;
            data->field_0x160 = 0.25f;
            goto done;
        }
    default:
        if (stage == 0) {
            Extra()->texture = 0;
            scale = 16.0f;
            data->field_0x158 = 0.0f;
            data->field_0x15c = 0.0f;
            data->field_0x160 = 0.0f;
            Extra()->texture = 0;
            goto done;
        }
        stage--;
        goto retry;
    }

parentTexture:
    Extra()->texture = Terrain()->textures[((DrawableGridNodeSharedTextures*)parent)->Extra()->index];
    UnknownFunction483100();
    {
        int z = field_0x2a >> 4;
        int x = field_0x2a % 16;
        data->field_0x158 = x * 0.0625f;
        data->field_0x15c = z * 0.0625f;
    }
    scale = 0.0625f;
    data->field_0x160 = 1.0f / 256.0f;
    goto done;

parentBlockTexture:
    Extra()->texture = Terrain()->textures[index];
    UnknownFunction483100();
    {
        int z = field_0x2a >> 4;
        int x = field_0x2a % 16;
        data->field_0x158 = x % 4 * 0.25f;
        data->field_0x15c = z % 4 * 0.25f;
    }
    scale = 0.25f;
    data->field_0x160 = 1.0f / 64.0f;
    goto done;

sharedTexture:
    Extra()->texture = Terrain()->textures[Extra()->index];
    UnknownFunction483100();
    scale = 1.0f;
    data->field_0x158 = 0.0f;
    data->field_0x15c = 0.0f;
    data->field_0x160 = 1.0f / 16.0f;
    if (Extra()->texture->field_0x14 == 0x40) {
        scale = 0.25f;
        data->field_0x160 = 1.0f;
    }
    goto done;

createTexture:
    Extra()->field_0x10 = Terrain()->AcquireOwnedObject();
    UnknownFunction47d470((ManagedTexture*)Extra()->field_0x10);
    if (((ManagedTexture*)Extra()->field_0x10)->field_0x68 & 1)
        ((ManagedTexture*)Extra()->field_0x10)->UnknownFunction510670();
    Extra()->textureFlags |= 0x10000;
    g_grid1OwnTextureCount++;
    Terrain()->textureAgeManager->UnknownFunction401050(&Extra()->ageEntry, EvictNodeTexture, this, 0,
                                                  UnknownFunction511970(Terrain()->nodeTextureFormat) * 0x40000 / 3);
ownTexture:
    Extra()->texture = (ManagedTexture*)Extra()->field_0x10;
    UnknownFunction483100();
    scale = 1.0f;
    data->field_0x158 = 0.0f;
    data->field_0x15c = 0.0f;
    data->field_0x160 = 1.0f / 16.0f;
    if (Extra()->texture->field_0x14 == 0x40) {
        scale = 0.25f;
        data->field_0x160 = 1.0f;
    } else if (Extra()->texture->field_0x14 == 0x80) {
        scale = 0.5f;
        data->field_0x160 = 1.0f / 16.0f;
    }

done:
    if (Extra()->texture != previous && !keep) {
        data->b1 = 1;
        Extra()->field_0x28 = 1.0f;
        Extra()->field_0x2c = 0.0f;
        Extra()->field_0x30 = 0.0f;
    }
    Extra()->detail = detail / scale;
}

// 0x006652a0: the 256 x 256 staging buffer of 0x0047d470 (Grid1.cpp's .bss,
// just before Gridbase.cpp's tables).
static unsigned char* g_grid1TextureBuffer;

// 0x0047d470: fills the node-wide texture from the node's runs. Each run is
// a (tile, length - 1) byte pair; a run copies `length` pixels of one row of
// a 16 x 16 tile texture, tiled across the 256 x 256 node. A smaller texture
// gets the result downsampled.
int DrawableGridNodeSharedTextures::UnknownFunction47d470(ManagedTexture* texture)
{
    int tiles[256];
    unsigned char* tileBits[256];
    int x = 0;
    int y = 0;
    int count = 0;
    unsigned char* tile;

    if (!g_grid1TextureBuffer)
        g_grid1TextureBuffer = (unsigned char*)DebugMalloc(0x20000, __FILE__, 0x23d);

    int index = Extra()->index;
    if (index == -1 || !Extra()->own)
        return 0;

    unsigned char* bits = (unsigned char*)texture->UnknownVirtualSlot13(0, 0, 0);
    unsigned char* out = texture->field_0x14 == 0x100 ? bits : g_grid1TextureBuffer;
    int bpp = UnknownFunction511970(texture->field_0x20);
    int tileStride = bpp << 4;
    unsigned char* runs = (unsigned char*)Extra()->field_0x04 + index * 2;
    do {
        int id = *runs++;
        int length = *runs++ + 1;
        int i;
        for (i = 0; i < count; i++) {
            if (tiles[i] == id) {
                tile = tileBits[i];
                break;
            }
        }
        if (i == count) {
            tiles[count] = id;
            tile = (unsigned char*)Terrain()->textures[id]->UnknownVirtualSlot16(16);
            tileBits[count] = tile;
            count++;
        }
        int row = y & 15;
        if (((x + length - 1) & ~15) - (x & ~15) == 0) {
            memcpy(out, tile + (x & 15) * bpp + row * tileStride, length * bpp);
            out += length * bpp;
        } else {
            int first = 0;
            if (x & 15) {
                first = 16 - (x & 15);
                memcpy(out, tile + (x & 15) * bpp + row * tileStride, first * bpp);
                out += first * bpp;
            }
            unsigned char* line = tile + row * tileStride;
            int rest = length - first;
            while (rest > 16) {
                memcpy(out, line, tileStride);
                out += tileStride;
                rest -= 16;
            }
            if (rest) {
                memcpy(out, line, rest * bpp);
                out += rest * bpp;
            }
        }
        x += length;
        if (x == 0x100) {
            x = 0;
            y++;
        }
    } while (y < 0x100);

    for (int i = 0; i < count; i++)
        Terrain()->textures[tiles[i]]->UnknownVirtualSlot17(16);
    if (texture->field_0x14 != 0x100)
        UnknownFunction4d1b90(bits, g_grid1TextureBuffer, texture->field_0x14, texture->field_0x14,
                              texture->field_0x14, texture->field_0x14 * 2, 1, texture->field_0x20,
                              texture->field_0x2c, 2);
    texture->UnknownVirtualSlot14(0);
    texture->UnknownVirtualSlot15(1);
    return 1;
}

// 0x0047d780: the same for one 4 x 4 block: its texture is `width` pixels
// wide, each run cell is 4 x 4 pixels and the tiles are width / 4 wide.
int DrawableGridNodeSharedTextures::UnknownFunction47d780(int block, ManagedTexture* texture)
{
    int tiles[256];
    unsigned char* tileBits[256];
    int x = 0;
    int y = 0;
    int count = 0;
    unsigned char* tile;

    Grid1BlockTexture* blocks = Extra()->blocks;
    if (!blocks)
        return 0;
    int index = blocks[block].index;
    if (index == -1 || !blocks[block].own)
        return 0;

    unsigned char* out = (unsigned char*)texture->UnknownVirtualSlot13(0, 0, 0);
    int width = texture->field_0x14;
    int tileSize = width / 4;
    int bpp = UnknownFunction511970(texture->field_0x20);
    int pitch = bpp * width;
    int tileStride = bpp * tileSize;
    unsigned char* runs = (unsigned char*)Extra()->field_0x04 + index * 2;
    int done = 0;
    while (y < width) {
        int id = *runs++;
        int length = (*runs++ + 1) * 4;
        done += length;
        int i;
        for (i = 0; i < count; i++) {
            if (tiles[i] == id) {
                tile = tileBits[i];
                break;
            }
        }
        if (i == count) {
            tiles[count] = id;
            tile = (unsigned char*)Terrain()->textures[id]->UnknownVirtualSlot16(tileSize);
            tileBits[count] = tile;
            count++;
        }
        int row = y & (tileSize - 1);
        if (((x + length - 1) & ~(tileSize - 1)) - (x & ~(tileSize - 1)) == 0) {
            unsigned char* src = tile + (x & (tileSize - 1)) * bpp + row * tileStride;
            unsigned char* dst = out;
            for (int k = 0; k < 4; k++) {
                memcpy(dst, src, length * bpp);
                src += tileStride;
                dst += pitch;
            }
            out += length * bpp;
        } else {
            int first = 0;
            if (x & (tileSize - 1)) {
                first = tileSize - (x & (tileSize - 1));
                unsigned char* src = tile + (x & (tileSize - 1)) * bpp + row * tileStride;
                unsigned char* dst = out;
                for (int k = 0; k < 4; k++) {
                    memcpy(dst, src, first * bpp);
                    src += tileStride;
                    dst += pitch;
                }
                out += first * bpp;
            }
            unsigned char* line = tile + row * tileStride;
            int rest = length - first;
            while (rest > tileSize) {
                unsigned char* src = line;
                unsigned char* dst = out;
                for (int k = 0; k < 4; k++) {
                    memcpy(dst, src, tileStride);
                    src += tileStride;
                    dst += pitch;
                }
                out += tileStride;
                rest -= tileSize;
            }
            if (rest) {
                unsigned char* src = line;
                unsigned char* dst = out;
                for (int k = 0; k < 4; k++) {
                    memcpy(dst, src, rest * bpp);
                    src += tileStride;
                    dst += pitch;
                }
                out += rest * bpp;
            }
        }
        x += length;
        if (x == width) {
            out += pitch * 3;
            x = 0;
            done = 0;
            y += 4;
        }
    }

    for (int j = 0; j < count; j++)
        Terrain()->textures[tiles[j]]->UnknownVirtualSlot17(tileSize);
    texture->UnknownVirtualSlot14(0);
    texture->UnknownVirtualSlot15(1);
    return 1;
}

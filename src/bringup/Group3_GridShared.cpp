// Link bring-up definitions (best-guess admissions, see docs/GAME_BUILD.md):
// DrawableGridNodeSharedTextures slot 3 (0x0047caa0) and the two texture
// builders it calls, BuildNodeTexture (0x0047d470) and BuildBlockTexture
// (0x0047d780), which no other build object defines.
//
// Bodies are taken unchanged from the near-miss file
// samples/render/Grid1NearMisses.cpp (see its header for the per-function
// byte-match notes: control flow, jump table and every call agree with retail;
// remaining differences are register allocation and one folded compare whose two
// arms both select stage 3, so behavior is the same).  The sample is not linked
// as a whole; only these functions and their two globals are admitted here.
// best guess: behavior not differentially tested.
#include "broadphase/Terrain.h"
#include "Grid1.h"

#include <string.h>

#include "DebugAlloc.h"
#include "Pixtrans.h"
#include "Tgafile.h"
#include "D3DConstants.h"

// 0x00505600 is defined on the canonical Terrain (TerrainSupport.cpp, returns
// BaseObject*); Grid1.h's Grid1Terrain view declares it returning ManagedTexture*
// and has no definition.  Call the canonical one, keeping the raw pointer as retail.
static ManagedTexture* Grid1AcquireOwnedTexture(GridTerrain* terrain)
{
    return (ManagedTexture*)(void*)reinterpret_cast< ::Terrain*>(terrain)->AcquireOwnedObject();
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
        detail = (float)((16 << shift) * ((Grid1Terrain*)terrain)->cameraFocalLength) / distance;
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
    if (((Grid1RenderHost*)((Grid1Terrain*)terrain)->renderer)->camera->lastMoveDistance > 0.5f && level == 0)
        wanted = stage > 2 ? 2 : stage;

    float scale;
    int index;
retry:
    switch (stage) {
    case 0:
        if (parent) {
            Grid1NodeExtra* parentExtra = ((DrawableGridNodeSharedTextures*)parent)->Extra();
            if (parentExtra->index != -1 && !parentExtra->own)
                goto parentTexture;
        }
    case 1:
        if (parent) {
            Grid1BlockTexture* blocks = ((DrawableGridNodeSharedTextures*)parent)->Extra()->blocks;
            if (blocks) {
                Grid1BlockTexture* record = &blocks[(((childIndex >> 2) & 0x30) | (childIndex & 0xc)) >> 2];
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
            ForceBlockCentres();
            data->texOriginU = 0.0f;
            data->texOriginV = 0.0f;
            data->texStep = 1.0f;
            Extra()->texture = ((Grid1Terrain*)terrain)->textures[0];
            for (block = 0; block < 16; block++) {
                Grid1BlockTexture* record = &Extra()->blocks[block];
                if (record->index != -1) {
                    if (record->own) {
                        if (!(Extra()->textureFlags & (1 << block))) {
                            if (stage == wanted) {
                                if (!TestBlockFrustum(block)) {
                                    Extra()->blocks[block].texture = 0;
                                } else {
                                    ((Grid1NodeExtra*)extra)->blocks[block].texture = Grid1AcquireOwnedTexture(terrain);
                                    BuildBlockTexture(block, Extra()->blocks[block].texture);
                                    if (Extra()->blocks[block].texture->mapFlags & 1)
                                        Extra()->blocks[block].texture->Reregister();
                                    Extra()->textureFlags |= 1 << block;
                                    g_grid1OwnTextureCount++;
                                    ((Grid1Terrain*)terrain)->textureAgeManager->Register(
                                        &((Grid1NodeExtra*)extra)->blocks[block].ageEntry, EvictBlockTexture,
                                        this, block, PixelFormatBytesPerPixel(((Grid1Terrain*)terrain)->nodeTextureFormat) * 0x40000 / 3);
                                }
                            } else {
                                stage = wanted;
                                goto retry;
                            }
                        }
                    } else {
                        record->texture = ((Grid1Terrain*)terrain)->textures[record->index];
                    }
                } else {
                    record->texture = 0;
                }
            }
            Extra()->texture = ((Grid1Terrain*)terrain)->textures[0];
            scale = 4.0f;
            data->texOriginU = 0.0f;
            data->texOriginV = 0.0f;
            data->texStep = 0.25f;
            goto done;
        }
    default:
        if (stage == 0) {
            Extra()->texture = 0;
            scale = 16.0f;
            data->texOriginU = 0.0f;
            data->texOriginV = 0.0f;
            data->texStep = 0.0f;
            Extra()->texture = 0;
            goto done;
        }
        stage--;
        goto retry;
    }

parentTexture:
    Extra()->texture = ((Grid1Terrain*)terrain)->textures[((DrawableGridNodeSharedTextures*)parent)->Extra()->index];
    ReleaseBlockCentres();
    {
        int z = childIndex >> 4;
        int x = childIndex % 16;
        data->texOriginU = x * 0.0625f;
        data->texOriginV = z * 0.0625f;
    }
    scale = 0.0625f;
    data->texStep = 1.0f / 256.0f;
    goto done;

parentBlockTexture:
    Extra()->texture = ((Grid1Terrain*)terrain)->textures[index];
    ReleaseBlockCentres();
    {
        int z = childIndex >> 4;
        int x = childIndex % 16;
        data->texOriginU = x % 4 * 0.25f;
        data->texOriginV = z % 4 * 0.25f;
    }
    scale = 0.25f;
    data->texStep = 1.0f / 64.0f;
    goto done;

sharedTexture:
    Extra()->texture = ((Grid1Terrain*)terrain)->textures[Extra()->index];
    ReleaseBlockCentres();
    scale = 1.0f;
    data->texOriginU = 0.0f;
    data->texOriginV = 0.0f;
    data->texStep = 1.0f / 16.0f;
    if (Extra()->texture->textureWidth == 0x40) {
        scale = 0.25f;
        data->texStep = 1.0f;
    }
    goto done;

createTexture:
    ((Grid1NodeExtra*)extra)->builtTexture = Grid1AcquireOwnedTexture(terrain);
    BuildNodeTexture((ManagedTexture*)Extra()->builtTexture);
    if (((ManagedTexture*)Extra()->builtTexture)->mapFlags & 1)
        ((ManagedTexture*)Extra()->builtTexture)->Reregister();
    Extra()->textureFlags |= 0x10000;
    g_grid1OwnTextureCount++;
    ((Grid1Terrain*)terrain)->textureAgeManager->Register(&((Grid1NodeExtra*)extra)->ageEntry, EvictNodeTexture, this, 0,
                                                  PixelFormatBytesPerPixel(((Grid1Terrain*)terrain)->nodeTextureFormat) * 0x40000 / 3);
ownTexture:
    Extra()->texture = (ManagedTexture*)Extra()->builtTexture;
    ReleaseBlockCentres();
    scale = 1.0f;
    data->texOriginU = 0.0f;
    data->texOriginV = 0.0f;
    data->texStep = 1.0f / 16.0f;
    if (Extra()->texture->textureWidth == 0x40) {
        scale = 0.25f;
        data->texStep = 1.0f;
    } else if (Extra()->texture->textureWidth == 0x80) {
        scale = 0.5f;
        data->texStep = 1.0f / 16.0f;
    }

done:
    if (Extra()->texture != previous && !keep) {
        data->b1 = 1;
        Extra()->mappingScale = 1.0f;
        Extra()->mappingOffsetU = 0.0f;
        Extra()->mappingOffsetV = 0.0f;
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
int DrawableGridNodeSharedTextures::BuildNodeTexture(ManagedTexture* texture)
{
    int tiles[256];
    unsigned char* tileBits[256];
    int x = 0;
    int y = 0;
    int count = 0;
    unsigned char* tile;

    if (!g_grid1TextureBuffer)
        g_grid1TextureBuffer = (unsigned char*)DebugMalloc(0x20000, "D:\\aardvark\\VC\\krusty2\\Grid1.cpp", 0x23d);

    int index = Extra()->index;
    if (index == -1 || !Extra()->own)
        return 0;

    unsigned char* bits = (unsigned char*)texture->LockSurface(0, 0, 0);
    unsigned char* out = texture->textureWidth == 0x100 ? bits : g_grid1TextureBuffer;
    int bpp = PixelFormatBytesPerPixel(texture->mapFormat);
    int tileStride = bpp << 4;
    unsigned char* runs = (unsigned char*)Extra()->data + index * 2;
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
            tile = (unsigned char*)Terrain()->textures[id]->LockLevel(16);
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
            int rest = length - first;
            unsigned char* line = tile + row * tileStride;
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
        Terrain()->textures[tiles[i]]->UnlockLevel(16);
    if (texture->textureWidth != 0x100)
        Downsample(bits, g_grid1TextureBuffer, texture->textureWidth, texture->textureWidth,
                              texture->textureWidth, texture->textureWidth * 2, 1, texture->mapFormat,
                              texture->colorMapper, 2);
    texture->UnlockSurface(0);
    texture->BuildMipLevels(1);
    return 1;
}

// 0x0047d780: the same for one 4 x 4 block: its texture is `width` pixels
// wide, each run cell is 4 x 4 pixels and the tiles are width / 4 wide.
int DrawableGridNodeSharedTextures::BuildBlockTexture(int block, ManagedTexture* texture)
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

    unsigned char* out = (unsigned char*)texture->LockSurface(0, 0, 0);
    int width = texture->textureWidth;
    int tileSize = width / 4;
    int bpp = PixelFormatBytesPerPixel(texture->mapFormat);
    int pitch = bpp * width;
    int done = 0;
    int tileStride = bpp * tileSize;
    unsigned char* runs = (unsigned char*)Extra()->data + index * 2;
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
            tile = (unsigned char*)Terrain()->textures[id]->LockLevel(tileSize);
            tileBits[count] = tile;
            count++;
        }
        int row = y & (tileSize - 1);
        if (((x + length - 1) & ~(tileSize - 1)) - (x & ~(tileSize - 1)) == 0) {
            unsigned char* dst = out;
            unsigned char* src = tile + (x & (tileSize - 1)) * bpp + row * tileStride;
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
        Terrain()->textures[tiles[j]]->UnlockLevel(tileSize);
    texture->UnlockSurface(0);
    texture->BuildMipLevels(1);
    return 1;
}

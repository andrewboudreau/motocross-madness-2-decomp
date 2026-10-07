// Grid1.cpp -- reconstruction of part of D:\aardvark\VC\krusty2\Grid1.cpp.
// See Grid1.h.

#include "Grid1.h"

#include "DebugAlloc.h"
#include "D3DConstants.h"

// Row offsets into 16 x 16 and 17 x 17 grids: the first copy of the shared
// header's tables (0x0056bfac and 0x0056bff0; Griddraw.cpp declares the same
// two). Grid1.cpp does not read them.
static int g_gridRow16[17] = {
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x100,
};
static int g_gridRow17[18] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x110, 0x121,
};

// 0x0056c034: Grid1.cpp's own initialised data (slot 3's distance unit).
float g_grid1DetailDistance = 256.0f;

// 0x0047c880
DrawableGridNodeSharedTextures::DrawableGridNodeSharedTextures(UnknownTextureStream* stream, int a1,
                                                               GridNode* parent, GridTerrain* terrain,
                                                               int level, int shift, int x, int z,
                                                               int index, float* boundsMin,
                                                               float* boundsMax)
    : DrawableGridNode((int)stream, a1, parent, terrain, level, shift, x, z, index, (int)boundsMin,
                       (int)boundsMax)
{
    UnknownFunction47e600(stream, a1, (DrawableGridNode*)parent, level, shift, x, z, index, boundsMin,
                          boundsMax);
}

// 0x0047c960
GridNode* DrawableGridNodeSharedTextures::UnknownVirtualSlot2(UnknownTextureStream* stream, int a1,
                                                              DrawableGridNode* parent, int level,
                                                              int shift, int x, int z, int index,
                                                              void* a8, void* a9)
{
    return new (__FILE__, 61) DrawableGridNodeSharedTextures(stream, a1, this, parent->terrain, level,
                                                             shift, x, z, index, (float*)a8, (float*)a9);
}

// 0x0047ca00: eviction callback for the node-wide texture.
int EvictNodeTexture(void* owner)
{
    DrawableGridNodeSharedTextures* node = (DrawableGridNodeSharedTextures*)owner;
    node->Terrain()->RetireOwnedObject(node->Extra()->field_0x10);
    node->Extra()->field_0x10 = 0;
    node->Extra()->textureFlags &= ~0x10000;
    node->Extra()->ageEntry.size = 0;
    return 1;
}

// 0x0047ca40: eviction callback for one block's texture.
int EvictBlockTexture(void* owner, int block)
{
    DrawableGridNodeSharedTextures* node = (DrawableGridNodeSharedTextures*)owner;
    node->Terrain()->RetireOwnedObject(node->Extra()->blocks[block].texture);
    node->Extra()->blocks[block].texture = 0;
    node->Extra()->textureFlags &= ~(1 << block);
    node->Extra()->blocks[block].ageEntry.size = 0;
    return 1;
}

// 0x0047d160: records the texture detail the node (block -1) or one block
// needs and keeps the largest size per texture in the use table.
void DrawableGridNodeSharedTextures::UnknownVirtualSlot4(int block)
{
    ManagedTexture* texture;
    if (block == -1)
        texture = Extra()->texture;
    else
        texture = Extra()->blocks[block].texture;
    if (!texture)
        return;

    float detail;
    if (block != -1) {
        float distance = UnknownFunction481b30(block);
        if (distance != 0.0f)
            Extra()->blocks[block].detail =
                (float)((16 << shift) * Terrain()->field_0x70) / (distance * 4.0f);
        else
            Extra()->blocks[block].detail = 10000.0f;
        detail = Extra()->blocks[block].detail;
    } else {
        detail = Extra()->detail;
    }
    float width = (float)texture->field_0x14 * 2;
    if (detail > width)
        detail = width;

    int size = 1;
    int level = 0;
    for (int n = (int)detail >> 1; n; n >>= 1) {
        size <<= 1;
        level++;
    }
    float fraction = (detail - size) / size;
    size <<= 1;
    if (fraction < 0.0f)
        fraction = 0.0f;
    if (!(texture->field_0x68 & 1))
        return;
    texture->UnknownFunction510820(level + 1 + fraction);

    int count = g_gridTextureCount;
    int i;
    for (i = 0; i < count; i++) {
        if (g_gridTextures[i] == texture) {
            if (size > g_gridTextureSizes[i])
                g_gridTextureSizes[i] = size < 0x100 ? size : 0x100;
            break;
        }
    }
    if (i == count) {
        g_gridTextures[count] = texture;
        g_gridTextureSizes[count] = size < 0x100 ? size : 0x100;
        g_gridTextureCount = count + 1;
    }
}

// 0x0047d310: binds the node-wide texture, or turns texturing off.
void DrawableGridNodeSharedTextures::UnknownVirtualSlot6()
{
    if (Extra()->texture) {
        Extra()->texture->UnknownVirtualSlot19();
        if (Extra()->ageEntry.size)
            Terrain()->textureAgeManager->MarkUsed(&Extra()->ageEntry);
    } else {
        Terrain()->renderer->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        Terrain()->renderer->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}

// 0x0047d420: the texture size of one block (width << level), 0 without one.
int DrawableGridNodeSharedTextures::UnknownVirtualSlot8(int block)
{
    Grid1BlockTexture* record = &Extra()->blocks[block];
    int index = record->index;
    if (index != -1) {
        ManagedTexture* texture;
        if (record->own)
            texture = record->texture;
        else
            texture = Terrain()->textures[index];
        if (texture)
            return texture->field_0x14 << texture->field_0x6c;
    }
    return 0;
}

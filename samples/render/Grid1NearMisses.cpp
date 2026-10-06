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
// Not reconstructed: slot 3 (0x0047caa0, 1728 bytes, a switch over the
// detail level with retries that picks or creates the node or block
// textures), 0x0047d470 (783 bytes, fills a texture from the node's
// colour data; Grid1.cpp line 0x23d allocates its 0x20000-byte buffer
// 0x006652a0) and 0x0047d780 (977 bytes, the per-block version called from
// slot 3).

#include "../../src/reconstructed/Grid1.h"

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
            Terrain()->field_0xc84->UnknownFunction401250(&Extra()->blocks[block].ageEntry);
    } else {
        x->texture = Terrain()->textures[index];
    }
    if (Extra()->texture) {
        Extra()->texture->UnknownVirtualSlot19();
    } else {
        Terrain()->field_0x18->UnknownVirtualSlot7(0, 1, 1);
        Terrain()->field_0x18->UnknownVirtualSlot7(0, 4, 1);
    }
}


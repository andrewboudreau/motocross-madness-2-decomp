// Near-miss FontTexture.cpp candidates (src/reconstructed/FontTexture.cpp),
// kept out of src/reconstructed until they match.
//
// FontTexture::FindCell (0x00467340, 63 bytes): the cell
// lookup. Retail keeps the loop test at the top (`cmp i, count; jae`, then
// `jmp` back after `i++`); VC6 here rotates the loop (`test count; jbe`
// up front, test at the bottom). for, while, for(;;) with breaks, continue,
// a cached count and a local array pointer all rotate or are worse.
// The same unrotated shape appears in FontTextureManager.cpp's 0x004677f0.
//
// FontTexture::UnknownFunction4673d0 (0x004673d0, 645 bytes): the loader.
// 607 of 645 bytes; every instruction matches except placement of the
// esi/edi saves: retail pushes all four callee-saved registers before the
// lookup and shares the epilogue with the early return; VC6 here delays the
// esi/edi pushes past the early return (shrink-wrapping), shifting stack
// offsets. Restructured early-exit forms (nested if, local font, single
// enclosing if) do not change it.
//
// CharacterCell's inline virtual destructor (0x00467680) and its deleting
// wrapper (0x00467660) are emitted only in a TU that constructs a cell, so
// with the loader here they are compiled (and match exactly) in this file.

// The canonical translation unit comes first: VC6's EH state numbering
// depends on which callees it can see are non-throwing.
#include "../../src/reconstructed/FontTexture.cpp"

#include "../../src/reconstructed/TrackOverlay.h"
#include "../../src/reconstructed/UnknownResourceManager.h"

// 0x00467340
CharacterCell* FontTexture::FindCell(int code) {
    unsigned int i;

    for (i = 0; i < cells->Count(); i++) {
        if (code == (*cells)[i]->code)
            return (*cells)[i];
    }
    return 0;
}

// 0x004673d0: the cell file holds the count, then per cell its code and
// four rectangle values.
FontTexture* FontTexture::UnknownFunction4673d0(const char* name, const char* path, const char* textureName,
                                                int format, TextureMapManager* manager, int a5) {
    UnknownFontTextureNode* node;
    UnknownTextureStream* stream;
    unsigned int count;
    FontTexture* font;
    unsigned int i;
    TextureMap* texture;

    node = s_UnknownManager65b478.UnknownFunction4677f0(name);
    if (node && node->font)
        return node->font;
    count = 0;
    stream = new(__FILE__, 138) UnknownTextureStream((int)g_UnknownResourceManager572b44);
    stream->UnknownFunction460f50(path, "rb", 0);
    stream->UnknownFunction461640(&count, 4, 1);
    font = new(__FILE__, 152) FontTexture(count, name);
    int code = 0;
    float rect[4] = {0, 0, 0, 0};
    for (i = 0; i < count; i++) {
        stream->UnknownFunction461640(&code, 4, 1);
        stream->UnknownFunction461640(rect, 4, 4);
        font->SetCell(i, new(__FILE__, 166) CharacterCell(code,
            Rectangle2D(rect[g_UnknownGlobal57972c], rect[g_UnknownGlobal568468], rect[g_UnknownGlobal56846c],
                        rect[g_UnknownGlobal568470])));
    }
    texture = UnknownFunction50a590(manager, textureName, format, 0, 0, 5, 6, a5, 0x80, 0xff00ff, 1, 0);
    if (texture) {
        texture->UnknownVirtualSlot8(1, 0, 0);
        font->SetTexture(texture);
        s_UnknownManager65b478.UnknownFunction4676c0(font);
    } else {
        delete font;
        font = 0;
    }
    delete stream;
    return font;
}

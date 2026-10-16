#include <string.h>

#include "FontTexture.h"

#include "DebugAlloc.h"
#include "TextureMap.h"
#include "TrackOverlay.h"
#include "UnknownResourceManager.h"

UnknownFontTextureManager FontTexture::s_UnknownManager65b478;

// 0x00467180
FontTexture::FontTexture(unsigned int count, const char* name)
    : fontName(0), cells(new(__FILE__, 22) EArray<CharacterCell*>(count)), texture(0) {
    int length;
    unsigned int i;

    length = strlen(name);
    fontName = new(__FILE__, 25) char[length + 1];
    strcpy(fontName, name);
    for (i = 0; i < count; i++)
        (*cells)[i] = 0;
}

// 0x00467280
void FontTexture::Destroy(FontTexture* font) {
    if (font)
        delete font;
}

// 0x00467290
FontTexture::~FontTexture() {
    unsigned int count;
    unsigned int i;

    s_UnknownManager65b478.DeleteFont(this);
    count = cells->Count();
    for (i = 0; i < count; i++) {
        if ((*cells)[i]) {
            CharacterCell* cell = (*cells)[i];
            (*cells)[i] = 0;
            delete cell;
        }
    }
    delete fontName;
    fontName = 0;
    delete cells;
    cells = 0;
    if (texture) {
        texture->Release();
        texture = 0;
    }
}

// 0x00467340: the cell with character code `code`, or 0. The test stays
// at the top of the loop only as `while (1)` with a `break`; a `for` or
// `while (i < count)` loop is rotated to test at the bottom.
CharacterCell* FontTexture::FindCell(int code) {
    unsigned int i = 0;

    while (1) {
        if (i >= cells->Count())
            break;
        if (code == (*cells)[i]->code)
            return (*cells)[i];
        i++;
    }
    return 0;
}

// 0x00467380
void FontTexture::SetCell(unsigned int index, CharacterCell* cell) {
    (*cells)[index] = cell;
}

// 0x004673a0
void FontTexture::SetTexture(TextureMap* newTexture) {
    texture = newTexture;
}

// 0x004673b0
const char* FontTexture::GetFontName() {
    return fontName;
}

// 0x004673c0
TextureMap* FontTexture::GetTexture() {
    return texture;
}

// 0x004673d0: the loader. The cell file holds the count, then per cell its
// code and four rectangle values. The lookup hit and the load path share one
// return: an early `return node->font;` lets VC6 delay the esi/edi saves past
// it, and a block around the cell loop's locals moves their stack slots.
FontTexture* FontTexture::Load(const char* name, const char* path, const char* textureName,
                                                int format, TextureMapManager* manager, int a5) {
    UnknownFontTextureNode* node;
    UnknownTextureStream* stream;
    unsigned int count;
    FontTexture* font;
    unsigned int i;
    TextureMap* texture;

    node = s_UnknownManager65b478.FindFont(name);
    if (node && node->font) {
        font = node->font;
    } else {
        count = 0;
        stream = new(__FILE__, 138) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        stream->Open(path, "rb", 0);
        stream->Read(&count, 4, 1);
        font = new(__FILE__, 152) FontTexture(count, name);
        int code = 0;
        float rect[4] = {0, 0, 0, 0};
        for (i = 0; i < count; i++) {
            stream->Read(&code, 4, 1);
            stream->Read(rect, 4, 4);
            font->SetCell(i, new(__FILE__, 166) CharacterCell(code,
                Rectangle2D(rect[g_UnknownGlobal57972c], rect[g_UnknownGlobal568468], rect[g_UnknownGlobal56846c],
                            rect[g_UnknownGlobal568470])));
        }
        texture = TerrainLoadTexture(manager, textureName, format, 0, 0, 5, 6, a5, 0x80, 0xff00ff, 1, 0);
        if (texture) {
            texture->CreateTextureSurface(1, 0, 0);
            font->SetTexture(texture);
            s_UnknownManager65b478.AddFont(font);
        } else {
            delete font;
            font = 0;
        }
        delete stream;
    }
    return font;
}

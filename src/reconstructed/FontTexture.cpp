#include <string.h>

#include "FontTexture.h"

#include "DebugAlloc.h"
#include "TextureMap.h"

UnknownFontTextureManager FontTexture::s_UnknownManager65b478;

// 0x00467180
FontTexture::FontTexture(unsigned int count, const char* name)
    : fontName(0), cells(new(__FILE__, 22) EArray<CharacterCell*>(count)), field_0x0c(0) {
    int length;
    unsigned int i;

    length = strlen(name);
    fontName = new(__FILE__, 25) char[length + 1];
    strcpy(fontName, name);
    for (i = 0; i < count; i++)
        (*cells)[i] = 0;
}

// 0x00467280
void FontTexture::UnknownFunction467280(FontTexture* font) {
    if (font)
        delete font;
}

// 0x00467290
FontTexture::~FontTexture() {
    unsigned int count;
    unsigned int i;

    s_UnknownManager65b478.UnknownFunction467760(this);
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
    if (field_0x0c) {
        field_0x0c->Release();
        field_0x0c = 0;
    }
}

// 0x00467380
void FontTexture::SetCell(unsigned int index, CharacterCell* cell) {
    (*cells)[index] = cell;
}

// 0x004673a0
void FontTexture::SetTexture(TextureMap* texture) {
    field_0x0c = texture;
}

// 0x004673b0
const char* FontTexture::UnknownFunction4673b0() {
    return fontName;
}

// 0x004673c0
TextureMap* FontTexture::GetTexture() {
    return field_0x0c;
}

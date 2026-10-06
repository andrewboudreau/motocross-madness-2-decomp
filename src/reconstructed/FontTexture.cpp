#include <string.h>

#include "FontTexture.h"

#include "DebugAlloc.h"
#include "TextureMap.h"

UnknownFontTextureManager FontTexture::s_UnknownManager65b478;

// 0x00467180
FontTexture::FontTexture(unsigned int count, const char* name)
    : field_0x04(0), field_0x08(new(__FILE__, 22) EArray<CharacterCell*>(count)), field_0x0c(0) {
    int length;
    unsigned int i;

    length = strlen(name);
    field_0x04 = new(__FILE__, 25) char[length + 1];
    strcpy(field_0x04, name);
    for (i = 0; i < count; i++)
        (*field_0x08)[i] = 0;
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
    count = field_0x08->Count();
    for (i = 0; i < count; i++) {
        if ((*field_0x08)[i]) {
            CharacterCell* cell = (*field_0x08)[i];
            (*field_0x08)[i] = 0;
            delete cell;
        }
    }
    delete field_0x04;
    field_0x04 = 0;
    delete field_0x08;
    field_0x08 = 0;
    if (field_0x0c) {
        field_0x0c->Release();
        field_0x0c = 0;
    }
}

// 0x00467380
void FontTexture::UnknownFunction467380(unsigned int index, CharacterCell* cell) {
    (*field_0x08)[index] = cell;
}

// 0x004673a0
void FontTexture::UnknownFunction4673a0(TextureMap* texture) {
    field_0x0c = texture;
}

// 0x004673b0
const char* FontTexture::UnknownFunction4673b0() {
    return field_0x04;
}

// 0x004673c0
TextureMap* FontTexture::UnknownFunction4673c0() {
    return field_0x0c;
}

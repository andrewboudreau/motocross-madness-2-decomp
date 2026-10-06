#include <string.h>

#include "FontTextureManager.h"

#include "DebugAlloc.h"
#include "FontTexture.h"

// 0x00467690
UnknownFontTextureManager::UnknownFontTextureManager() {
}

// 0x004676a0
UnknownStatic65b478::UnknownStatic65b478() {
    field_0x00 = 0;
    field_0x04 = 0;
}

// 0x004676b0
UnknownStatic65b478::~UnknownStatic65b478() {
    UnknownFunction4677c0();
}

// 0x004676c0
void UnknownStatic65b478::UnknownFunction4676c0(FontTexture* font) {
    UnknownFontTextureNode* last = 0;
    UnknownFontTextureNode* node;

    for (node = field_0x00; node; node = node->next)
        last = node;
    if (!last)
        field_0x00 = new(__FILE__, 25) UnknownFontTextureNode(font);
    else
        last->next = new(__FILE__, 29) UnknownFontTextureNode(font);
    field_0x04++;
}

// 0x004677c0
void UnknownStatic65b478::UnknownFunction4677c0() {
    UnknownFontTextureNode* node;

    while ((node = field_0x00) != 0) {
        field_0x00 = node->next;
        FontTexture::UnknownFunction467280(node->font);
        delete node;
        field_0x04--;
    }
}

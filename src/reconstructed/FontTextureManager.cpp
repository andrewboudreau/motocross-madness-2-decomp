#include <string.h>

#include "FontTextureManager.h"

#include "DebugAlloc.h"
#include "FontTexture.h"

// 0x00467690
UnknownFontTextureManager::UnknownFontTextureManager() {
}

// 0x004676a0
UnknownStatic65b478::UnknownStatic65b478() {
    firstNode = 0;
    nodeCount = 0;
}

// 0x004676b0
UnknownStatic65b478::~UnknownStatic65b478() {
    UnknownFunction4677c0();
}

// 0x004676c0
void UnknownStatic65b478::UnknownFunction4676c0(FontTexture* font) {
    UnknownFontTextureNode* last = 0;
    UnknownFontTextureNode* node;

    for (node = firstNode; node; node = node->next)
        last = node;
    if (!last)
        firstNode = new(__FILE__, 25) UnknownFontTextureNode(font);
    else
        last->next = new(__FILE__, 29) UnknownFontTextureNode(font);
    nodeCount++;
}

// 0x004677c0
void UnknownStatic65b478::UnknownFunction4677c0() {
    UnknownFontTextureNode* node;

    while ((node = firstNode) != 0) {
        firstNode = node->next;
        FontTexture::UnknownFunction467280(node->font);
        delete node;
        nodeCount--;
    }
}

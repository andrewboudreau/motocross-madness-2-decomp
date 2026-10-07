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

// 0x00467760: unlinks and frees the node holding `font`, then releases
// the font. Each branch of the unlink ends in its own `delete node`; VC6
// merges the two into one tail, which keeps the delete's stack cleanup
// apart from the release call's (retail's two `add esp, 4`).
void UnknownStatic65b478::UnknownFunction467760(FontTexture* font) {
    UnknownFontTextureNode* previous = 0;
    UnknownFontTextureNode* node;

    for (node = firstNode; node; node = node->next) {
        if (font == node->font) {
            if (!previous) {
                firstNode = node->next;
                delete node;
            } else {
                previous->next = node->next;
                delete node;
            }
            FontTexture::UnknownFunction467280(font);
            nodeCount--;
            return;
        }
        previous = node;
    }
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

// 0x004677f0: the node whose font is named `name`, or 0. Written as
// `while (1)` with breaks, the `node != 0` test stays at the top of the
// loop as in retail; a `while (node && ...)` condition is rotated.
UnknownFontTextureNode* UnknownStatic65b478::UnknownFunction4677f0(const char* name) {
    UnknownFontTextureNode* node = firstNode;

    while (1) {
        if (!node)
            break;
        if (strcmp(name, node->font->UnknownFunction4673b0()) == 0)
            break;
        node = node->next;
    }
    return node;
}

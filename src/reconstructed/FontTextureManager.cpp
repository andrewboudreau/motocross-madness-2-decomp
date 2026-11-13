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
    DeleteFonts();
}

// 0x004676c0
void UnknownStatic65b478::AddFont(FontTexture* font) {
    FontTextureNode* last = 0;
    FontTextureNode* node;

    for (node = firstNode; node; node = node->next)
        last = node;
    if (!last)
        firstNode = new(__FILE__, 25) FontTextureNode(font);
    else
        last->next = new(__FILE__, 29) FontTextureNode(font);
    nodeCount++;
}

// 0x00467760: unlinks and frees the node holding `font`, then releases
// the font. Each branch of the unlink ends in its own `delete node`; VC6
// merges the two into one tail, which keeps the delete's stack cleanup
// apart from the release call's (retail's two `add esp, 4`).
void UnknownStatic65b478::DeleteFont(FontTexture* font) {
    FontTextureNode* previous = 0;
    FontTextureNode* node;

    for (node = firstNode; node; node = node->next) {
        if (font == node->font) {
            if (!previous) {
                firstNode = node->next;
                delete node;
            } else {
                previous->next = node->next;
                delete node;
            }
            FontTexture::Destroy(font);
            nodeCount--;
            return;
        }
        previous = node;
    }
}

// 0x004677c0
void UnknownStatic65b478::DeleteFonts() {
    FontTextureNode* node;

    while ((node = firstNode) != 0) {
        firstNode = node->next;
        FontTexture::Destroy(node->font);
        delete node;
        nodeCount--;
    }
}

// 0x004677f0: the node whose font is named `name`, or 0. Written as
// `while (1)` with breaks, the `node != 0` test stays at the top of the
// loop as in retail; a `while (node && ...)` condition is rotated.
FontTextureNode* UnknownStatic65b478::FindFont(const char* name) {
    FontTextureNode* node = firstNode;

    while (1) {
        if (!node)
            break;
        if (strcmp(name, node->font->GetFontName()) == 0)
            break;
        node = node->next;
    }
    return node;
}

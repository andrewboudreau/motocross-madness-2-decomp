// Near-miss FontTextureManager.cpp candidates
// (src/reconstructed/FontTextureManager.cpp), kept out of src/reconstructed
// until they match.
//
// UnknownStatic65b478::UnknownFunction467760 (0x00467760, 78 bytes): the
// removal. Retail compares `cmp font, [node]` and pops the node delete's
// argument before pushing the font for 0x00467280 (two separate
// `add esp, 4`); VC6 here compares `cmp [node], font` and merges the two
// cleanups into one `add esp, 8`, two bytes shorter. Swapping the compare
// operands fixes the compare only; break-then-unlink forms are worse.
//
// UnknownStatic65b478::UnknownFunction4677f0 (0x004677f0, 88 bytes): the
// lookup by name. Inline strcmp and the walk match; retail keeps the
// `node != 0` test at the loop top with a `jmp` back (and loads `name`
// before it), VC6 here rotates the loop. while, for(;;) with breaks and
// early-return forms all rotate (see FontTextureNearMisses.cpp).

// The canonical translation unit comes first: VC6's EH state numbering
// depends on which callees it can see are non-throwing.
#include "../../src/reconstructed/FontTextureManager.cpp"

// 0x00467760
void UnknownStatic65b478::UnknownFunction467760(FontTexture* font) {
    UnknownFontTextureNode* previous = 0;
    UnknownFontTextureNode* node;

    for (node = firstNode; node; node = node->next) {
        if (font == node->font) {
            if (!previous)
                firstNode = node->next;
            else
                previous->next = node->next;
            delete node;
            FontTexture::UnknownFunction467280(font);
            nodeCount--;
            return;
        }
        previous = node;
    }
}

// 0x004677f0
UnknownFontTextureNode* UnknownStatic65b478::UnknownFunction4677f0(const char* name) {
    UnknownFontTextureNode* node;

    node = firstNode;
    while (node && strcmp(name, node->font->UnknownFunction4673b0()) != 0)
        node = node->next;
    return node;
}

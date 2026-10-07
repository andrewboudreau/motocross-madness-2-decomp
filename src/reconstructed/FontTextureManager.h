#pragma once

class FontTexture;

// FontTextureManager.cpp (literal __FILE__ at 0x0056b2fc; xrefs 0x004676da
// and 0x0046771a): a singly linked list of the loaded FontTextures.
// Names are provisional.

// List node (8 bytes, allocated at lines 25 and 29).
struct UnknownFontTextureNode {
    UnknownFontTextureNode(FontTexture* font) : font(font), next(0) {}

    FontTexture* font;
    UnknownFontTextureNode* next;
};

// The list. Game.cpp calls 0x004677c0 on the static object at 0x0065b478
// on shutdown. Its constructor body is shared (identical code folding) with
// other two-field constructors, e.g. TrackGame.h's 0x004676a0.
class UnknownStatic65b478 {
public:
    UnknownStatic65b478();                    // 0x004676a0
    ~UnknownStatic65b478();                   // 0x004676b0: deletes every font

    void UnknownFunction4676c0(FontTexture* font); // 0x004676c0: appends
    // 0x00467760: unlinks `font` and deletes it (through 0x00467280);
    // nothing when it is not listed.
    void UnknownFunction467760(FontTexture* font);
    void UnknownFunction4677c0();             // 0x004677c0: deletes every font
    // 0x004677f0: the node of the font named `name`, or 0.
    UnknownFontTextureNode* UnknownFunction4677f0(const char* name);

    UnknownFontTextureNode* firstNode;        // first node
    int nodeCount;                            // node count
};

// The type of the static object at 0x0065b478: it adds nothing to the list
// (the object is 8 bytes; its destruction guard byte follows at
// 0x0065b480), but has its own out-of-line constructor 0x00467690, which
// only calls the list's. Its destructor (0x00467150, in FontTexture.cpp's
// code) is the implicit one.
class UnknownFontTextureManager : public UnknownStatic65b478 {
public:
    UnknownFontTextureManager();              // 0x00467690
};

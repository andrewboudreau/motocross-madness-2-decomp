#pragma once

#include "EArray.h"
#include "FontTextureManager.h"
#include "Rectangle2D.h"

class TextureMap;
class TextureMapManager;

// RTTI: CharacterCell (no bases; vtable 0x00552994, one slot: the deleting
// destructor 0x00467660). 0x24 bytes, made by FontTexture's loader: a
// character code and its rectangle in the font texture. Its constructor is
// inlined; the destructor 0x00467680 is emitted in FontTexture.cpp.
class CharacterCell {
public:
    CharacterCell(int code, const Rectangle2D& rect) : code(code), field_0x08(rect) {}
    virtual ~CharacterCell() {}

    // 0x0050beb0 (out-of-line copy after TextService.cpp's code): the cell
    // rectangle, in texture coordinates.
    Rectangle2D UnknownFunction50beb0() { return field_0x08; }

    int code;                                 // character code
    Rectangle2D field_0x08;
};

// FontTexture.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\FontTexture.cpp"
// at 0x0056b2b4). RTTI: FontTexture (no bases; vtable 0x0055298c, one slot:
// the deleting destructor 0x00467160). A named font: its character cells
// and the texture they index. Fonts register with the list at 0x0065b478.
// Member names are provisional.
class FontTexture {
public:
    FontTexture(unsigned int count, const char* name); // 0x00467180 (ret 8)
    virtual ~FontTexture();                   // 0x00467290 (deleting wrapper 0x00467160)

    // 0x00467280 (cdecl): deletes `font` if there is one.
    static void UnknownFunction467280(FontTexture* font);
    CharacterCell* FindCell(int code);              // 0x00467340: the cell for `code`, or 0
    void SetCell(unsigned int index, CharacterCell* cell);               // 0x00467380
    void SetTexture(TextureMap* texture);            // 0x004673a0
    const char* UnknownFunction4673b0();      // 0x004673b0: the name
    TextureMap* GetTexture();                 // 0x004673c0: the texture
    // 0x004673d0 (cdecl): the font `name`, already loaded or read from the
    // cell file `path` with its texture `textureName` (through 0x0050a590).
    static FontTexture* UnknownFunction4673d0(const char* name, const char* path, const char* textureName,
                                              int format, TextureMapManager* manager, int a5);

    // 0x0065b478. A static data member: its atexit destructor 0x00467130 is
    // guarded by a flag byte, as VC6 emits for class statics. The owning
    // class is not established. Game.h views the object as
    // g_UnknownStatic65b478.
    static UnknownFontTextureManager s_UnknownManager65b478;

    char* fontName;                           // name
    EArray<CharacterCell*>* cells;            // cells
    TextureMap* field_0x0c;
};

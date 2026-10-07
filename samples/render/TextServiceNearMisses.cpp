// Near-miss TextService.cpp candidates (src/reconstructed/TextService.cpp),
// kept out of src/reconstructed until they match.
//
// UnknownOverlayText::SelectFont (0x0050ade0, 125 bytes): the
// font lookup by name. 117 of 125: written as `while (1)` with the count
// test and the match as breaks (the form that makes FontTexture.cpp's
// 0x00467340 and FontTextureManager.cpp's 0x004677f0 exact), the loop
// test stays at the top as in retail. Only the element load in the loop
// differs: retail computes `i - base` in eax with the list in ecx, VC6
// here keeps the list in eax and the index in edx. Declaration order,
// `++i`, `== 0`, a named element or name local do not change it; `for`
// and `while (i < count)` loops rotate.
//
// UnknownFunction50b400 (0x0050b400, 758 bytes; 4444 texels),
// UnknownFunction50b700 (0x0050b700, 758 bytes; 1555 texels) and the 32-bit
// body of UnknownFunction50b080 (0x0050b080, 890 bytes; its format dispatch
// matches): same instructions, different register/stack assignment. Retail
// keeps fontStride in ebp (stride at [esp+0x34]), w in esi and v in the slot
// later reused for the row count; VC6 here gives ebp to stride and esi to v,
// and computes the row offsets after the `h == 0` test instead of before.
// Pointer-walking rows, fully indexed rows, top-of-function declarations,
// signed coordinates, swapped declarations and inline pitch expressions are
// all further off.
//
// UnknownFunction50ba00 (0x0050ba00, 802 bytes): 770 of 802; only the
// scheduling of two stores (v3.sx and v5.sx) differs. Writing through a
// vertex pointer, other vertex orders, field orders and an inline vertex
// setter are further off.
//
// CharacterCell::UnknownFunction50beb0 (0x0050beb0, the out-of-line copy of
// FontTexture.h's inline) is emitted only in a TU that calls it, so it is
// compiled (and matches exactly) in this file.

// The canonical translation unit comes first.
#include "../../src/reconstructed/TextService.cpp"

// 0x0050ade0
void UnknownOverlayText::SelectFont(const char* font) {
    unsigned int count;
    unsigned int i;

    count = fonts->Count();
    i = 0;
    while (1) {
        if (i >= count)
            return;
        if (!strcmp(font, (*fonts)[i]->UnknownFunction4673b0()))
            break;
        i++;
    }
    currentFont = (*fonts)[i];
}

// 0x0050b080
int UnknownOverlayText::UnknownFunction50b080(TextureMap* texture, int x, int y, const char* text, int color) {
    if (texture->field_0x20 == 1555)
        return UnknownFunction50b700(texture, x, y, text, UnknownFunction43b660(color, texture->field_0x20));
    if (texture->field_0x20 == 4444)
        return UnknownFunction50b400(texture, x, y, text, UnknownFunction43b660(color, texture->field_0x20));

    TextureMap* fontTexture = currentFont->GetTexture();
    unsigned int width = texture->field_0x14;
    long pitch = 0;
    long fontPitch = 0;
    unsigned int fontWidth = fontTexture->field_0x14;
    unsigned int fontHeight = fontTexture->field_0x18;
    unsigned int* bits = (unsigned int*)texture->UnknownVirtualSlot13(0, &pitch, 0);
    unsigned int* fontBits = (unsigned int*)fontTexture->UnknownVirtualSlot13(0, &fontPitch, 0);
    Rectangle2D cell(0, 0, 0, 0);
    unsigned int length = strlen(text);
    unsigned int stride = (unsigned long)pitch >> 2;
    unsigned int fontStride = (unsigned long)fontPitch >> 2;
    int end = 0;
    unsigned int px = x;
    unsigned int py = y;
    unsigned int i;

    for (i = 0; i < length; i++) {
        CharacterCell* character = currentFont->FindCell(text[i]);
        if (character) {
            cell = character->UnknownFunction50beb0();
            unsigned int u = (unsigned int)(fontWidth * cell.field_0x04.x);
            unsigned int v = (unsigned int)(fontHeight * cell.field_0x04.y);
            unsigned int w = (unsigned int)((cell.field_0x10.x - cell.field_0x04.x) * fontWidth);
            unsigned int h = (unsigned int)((cell.field_0x10.y - cell.field_0x04.y) * fontHeight);
            if (px + w >= width) {
                px = x;
                py += h;
            }
            unsigned int source = fontStride * v;
            unsigned int dest = py * stride;
            for (unsigned int row = 0; row < h; row++) {
                end = px;
                for (unsigned int column = u; column < u + w; column++) {
                    bits[dest + end] = UnknownFunction43b560(fontBits[source + column], bits[dest + end], color);
                    end++;
                }
                source += fontStride;
                dest += stride;
            }
            px += w;
        }
    }
    texture->UnknownVirtualSlot14(0);
    fontTexture->UnknownVirtualSlot14(0);
    texture->UnknownVirtualSlot9(0, 1);
    return end;
}

// 0x0050b400
int UnknownOverlayText::UnknownFunction50b400(TextureMap* texture, int x, int y, const char* text, int color) {
    TextureMap* fontTexture = currentFont->GetTexture();
    unsigned int width = texture->field_0x14;
    long pitch = 0;
    long fontPitch = 0;
    unsigned int fontWidth = fontTexture->field_0x14;
    unsigned int fontHeight = fontTexture->field_0x18;
    unsigned short* bits = (unsigned short*)texture->UnknownVirtualSlot13(0, &pitch, 0);
    unsigned short* fontBits = (unsigned short*)fontTexture->UnknownVirtualSlot13(0, &fontPitch, 0);
    Rectangle2D cell(0, 0, 0, 0);
    unsigned int length = strlen(text);
    unsigned int stride = (unsigned long)pitch >> 1;
    unsigned int fontStride = (unsigned long)fontPitch >> 1;
    int end = 0;
    unsigned int px = x;
    unsigned int py = y;
    unsigned int i;

    for (i = 0; i < length; i++) {
        CharacterCell* character = currentFont->FindCell(text[i]);
        if (character) {
            cell = character->UnknownFunction50beb0();
            unsigned int u = (unsigned int)(fontWidth * cell.field_0x04.x);
            unsigned int v = (unsigned int)(fontHeight * cell.field_0x04.y);
            unsigned int w = (unsigned int)((cell.field_0x10.x - cell.field_0x04.x) * fontWidth);
            unsigned int h = (unsigned int)((cell.field_0x10.y - cell.field_0x04.y) * fontHeight);
            if (px + w >= width) {
                px = x;
                py += h;
            }
            unsigned int source = fontStride * v;
            unsigned int dest = py * stride;
            for (unsigned int row = 0; row < h; row++) {
                end = px;
                for (unsigned int column = u; column < u + w; column++) {
                    bits[dest + end] = UnknownFunction43b320(fontBits[source + column], bits[dest + end], color);
                    end++;
                }
                source += fontStride;
                dest += stride;
            }
            px += w;
        }
    }
    texture->UnknownVirtualSlot14(0);
    fontTexture->UnknownVirtualSlot14(0);
    texture->UnknownVirtualSlot9(0, 1);
    return end;
}

// 0x0050b700
int UnknownOverlayText::UnknownFunction50b700(TextureMap* texture, int x, int y, const char* text, int color) {
    TextureMap* fontTexture = currentFont->GetTexture();
    unsigned int width = texture->field_0x14;
    long pitch = 0;
    long fontPitch = 0;
    unsigned int fontWidth = fontTexture->field_0x14;
    unsigned int fontHeight = fontTexture->field_0x18;
    unsigned short* bits = (unsigned short*)texture->UnknownVirtualSlot13(0, &pitch, 0);
    unsigned short* fontBits = (unsigned short*)fontTexture->UnknownVirtualSlot13(0, &fontPitch, 0);
    Rectangle2D cell(0, 0, 0, 0);
    unsigned int length = strlen(text);
    unsigned int stride = (unsigned long)pitch >> 1;
    unsigned int fontStride = (unsigned long)fontPitch >> 1;
    int end = 0;
    unsigned int px = x;
    unsigned int py = y;
    unsigned int i;

    for (i = 0; i < length; i++) {
        CharacterCell* character = currentFont->FindCell(text[i]);
        if (character) {
            cell = character->UnknownFunction50beb0();
            unsigned int u = (unsigned int)(fontWidth * cell.field_0x04.x);
            unsigned int v = (unsigned int)(fontHeight * cell.field_0x04.y);
            unsigned int w = (unsigned int)((cell.field_0x10.x - cell.field_0x04.x) * fontWidth);
            unsigned int h = (unsigned int)((cell.field_0x10.y - cell.field_0x04.y) * fontHeight);
            if (px + w >= width) {
                px = x;
                py += h;
            }
            unsigned int source = fontStride * v;
            unsigned int dest = py * stride;
            for (unsigned int row = 0; row < h; row++) {
                end = px;
                for (unsigned int column = u; column < u + w; column++) {
                    bits[dest + end] = UnknownFunction43b440(fontBits[source + column], bits[dest + end], color);
                    end++;
                }
                source += fontStride;
                dest += stride;
            }
            px += w;
        }
    }
    texture->UnknownVirtualSlot14(0);
    fontTexture->UnknownVirtualSlot14(0);
    texture->UnknownVirtualSlot9(0, 1);
    return end;
}

// 0x0050ba00: cell rectangles are in texture coordinates of a 255-texel
// font texture.
void* UnknownOverlayText::UnknownFunction50ba00(const UnknownOverlayRect* rect, const char* text,
                                                unsigned int color, int* count) {
    Rectangle2D cell(0, 0, 0, 0);
    Rectangle2D size(0, 0, 0, 0);
    float x = rect->left > 0.0f ? (float)rect->left : 0.0f;
    float y = rect->top > 0.0f ? (float)rect->top : 0.0f;
    unsigned int length = strlen(text);
    unsigned int i;

    *count = length * 6;
    for (i = 0; i < length; i++) {
        CharacterCell* character = currentFont->FindCell(text[i]);
        if (character) {
            cell = character->UnknownFunction50beb0();
            float height = (cell.field_0x10.y - cell.field_0x04.y) * 255.0f;
            float width = (cell.field_0x10.x - cell.field_0x04.x) * 255.0f;
            float top = cell.field_0x04.y * 255.0f;
            float left = cell.field_0x04.x * 255.0f;
            size.UnknownSet(left, top, left + width, top + height);
            float right = size.field_0x10.x - size.field_0x04.x + x;
            float bottom = size.field_0x10.y - size.field_0x04.y + y;

            field_0x2c[i * 6 + 0].sx = x;
            field_0x2c[i * 6 + 0].sy = y;
            field_0x2c[i * 6 + 0].sz = vertexDepth;
            field_0x2c[i * 6 + 0].rhw = 1.0f;
            field_0x2c[i * 6 + 0].color = color;
            field_0x2c[i * 6 + 0].specular = 0;
            field_0x2c[i * 6 + 0].tu = cell.field_0x04.x;
            field_0x2c[i * 6 + 0].tv = cell.field_0x04.y;

            field_0x2c[i * 6 + 2].sx = x;
            field_0x2c[i * 6 + 2].sy = bottom;
            field_0x2c[i * 6 + 2].sz = vertexDepth;
            field_0x2c[i * 6 + 2].rhw = 1.0f;
            field_0x2c[i * 6 + 2].color = color;
            field_0x2c[i * 6 + 2].specular = 0;
            field_0x2c[i * 6 + 2].tu = cell.field_0x04.x;
            field_0x2c[i * 6 + 2].tv = cell.field_0x10.y;

            field_0x2c[i * 6 + 1].sx = right;
            field_0x2c[i * 6 + 1].sy = bottom;
            field_0x2c[i * 6 + 1].sz = vertexDepth;
            field_0x2c[i * 6 + 1].rhw = 1.0f;
            field_0x2c[i * 6 + 1].color = color;
            field_0x2c[i * 6 + 1].specular = 0;
            field_0x2c[i * 6 + 1].tu = cell.field_0x10.x;
            field_0x2c[i * 6 + 1].tv = cell.field_0x10.y;

            field_0x2c[i * 6 + 3].sx = x;
            field_0x2c[i * 6 + 3].sy = y;
            field_0x2c[i * 6 + 3].sz = vertexDepth;
            field_0x2c[i * 6 + 3].rhw = 1.0f;
            field_0x2c[i * 6 + 3].color = color;
            field_0x2c[i * 6 + 3].specular = 0;
            field_0x2c[i * 6 + 3].tu = cell.field_0x04.x;
            field_0x2c[i * 6 + 3].tv = cell.field_0x04.y;

            field_0x2c[i * 6 + 5].sx = right;
            field_0x2c[i * 6 + 5].sy = bottom;
            field_0x2c[i * 6 + 5].sz = vertexDepth;
            field_0x2c[i * 6 + 5].rhw = 1.0f;
            field_0x2c[i * 6 + 5].color = color;
            field_0x2c[i * 6 + 5].specular = 0;
            field_0x2c[i * 6 + 5].tu = cell.field_0x10.x;
            field_0x2c[i * 6 + 5].tv = cell.field_0x10.y;

            field_0x2c[i * 6 + 4].sx = right;
            field_0x2c[i * 6 + 4].sy = y;
            field_0x2c[i * 6 + 4].sz = vertexDepth;
            field_0x2c[i * 6 + 4].rhw = 1.0f;
            field_0x2c[i * 6 + 4].color = color;
            field_0x2c[i * 6 + 4].specular = 0;
            field_0x2c[i * 6 + 4].tu = cell.field_0x10.x;
            field_0x2c[i * 6 + 4].tv = cell.field_0x04.y;

            x = right;
        }
    }
    return field_0x2c;
}


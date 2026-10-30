// Group2_OverlayText.cpp -- bring-up (link) definitions for TextService.cpp
// functions still missing from src/: UnknownOverlayText::DrawTextInto
// (0x0050b080), its 16-bit variants DrawText4444 (0x0050b400) and
// DrawText1555 (0x0050b700), LayoutText (0x0050ba00), and the four cdecl
// pixel helpers they call (0x0043b660, 0x0043b560, 0x0043b320, 0x0043b440;
// names from TextService.h, provisional).
//
// Status: best-guess-unreviewed. The UnknownOverlayText bodies are the
// near-miss bodies of samples/render/TextServiceNearMisses.cpp (register /
// scheduling differences only, per that file). The pixel helpers were
// written from the retail disassembly for this bring-up; not byte-matched
// and not differentially tested.

#include <string.h>

#include "TextService.h"

#include "FontTexture.h"
#include "TextureMap.h"

// 0x0043b660: converts a 32-bit ARGB colour to a 16-bit texel of `format`
// (4444 or 1555); any other format gives 0. Channels are scaled by 1/255
// and the 4-/5-bit maximum and truncated (_ftol); 1555's alpha bit is set
// when alpha > 127.
unsigned int ConvertColorToFormat(unsigned int color, int format) {
    unsigned char a = (unsigned char)(color >> 24);
    unsigned char r = (unsigned char)(color >> 16);
    unsigned char g = (unsigned char)(color >> 8);
    unsigned char b = (unsigned char)color;
    if (format == 4444) {
        unsigned int value = (unsigned char)(int)(a * 0.003921569f * 15.0f);
        value = (value << 4) | (unsigned char)(int)(r * 0.003921569f * 15.0f);
        value = (value << 4) | (unsigned char)(int)(g * 0.003921569f * 15.0f);
        value = (value << 4) | (unsigned char)(int)(b * 0.003921569f * 15.0f);
        return (unsigned short)value;
    }
    if (format == 1555) {
        unsigned int value = (unsigned char)(int)(r * 0.003921569f * 31.0f) | ((a > 0x7f) << 5);
        value = (value << 5) | (unsigned char)(int)(g * 0.003921569f * 31.0f);
        value = (value << 5) | (unsigned char)(int)(b * 0.003921569f * 31.0f);
        return (unsigned short)value;
    }
    return 0;
}

// 0x0043b560: blends a 32-bit ARGB font texel over `dest` with the texel's
// alpha; a nonzero `color` replaces the texel's RGB (keeping its alpha).
// All four channels (alpha too) are blended: d = t * a + d * (1 - a).
unsigned int BlendFontTexel32(unsigned int texel, unsigned int dest, unsigned int color) {
    unsigned char t[4];
    unsigned char d[4];
    t[0] = (unsigned char)(texel >> 24);
    t[1] = (unsigned char)(texel >> 16);
    t[2] = (unsigned char)(texel >> 8);
    t[3] = (unsigned char)texel;
    d[0] = (unsigned char)(dest >> 24);
    d[1] = (unsigned char)(dest >> 16);
    d[2] = (unsigned char)(dest >> 8);
    d[3] = (unsigned char)dest;
    float alpha = t[0] * 0.003921569f;
    if (color) {
        t[1] = (unsigned char)(color >> 16);
        t[2] = (unsigned char)(color >> 8);
        t[3] = (unsigned char)color;
    }
    float inverse = 1.0f - alpha;
    for (unsigned int i = 0; i < 4; i++)
        d[i] = (unsigned char)(int)(t[i] * alpha + d[i] * inverse);
    return ((unsigned int)d[0] << 24) | ((unsigned int)d[1] << 16) | ((unsigned int)d[2] << 8) | d[3];
}

// 0x0043b320: the 4444 version (alpha nibble scaled by 1/15).
unsigned short BlendFontTexel4444(unsigned short texel, unsigned short dest, unsigned short color) {
    unsigned char t[4];
    unsigned char d[4];
    t[0] = (texel >> 12) & 0xf;
    t[1] = (texel >> 8) & 0xf;
    t[2] = (texel >> 4) & 0xf;
    t[3] = texel & 0xf;
    d[0] = (dest >> 12) & 0xf;
    d[1] = (dest >> 8) & 0xf;
    d[2] = (dest >> 4) & 0xf;
    d[3] = dest & 0xf;
    float alpha = t[0] * 0.06666667f;
    if (color) {
        t[1] = (color >> 8) & 0xf;
        t[2] = (color >> 4) & 0xf;
        t[3] = color & 0xf;
    }
    float inverse = 1.0f - alpha;
    for (unsigned int i = 0; i < 4; i++)
        d[i] = (unsigned char)(int)(t[i] * alpha + d[i] * inverse);
    return (unsigned short)((d[0] << 12) | (d[1] << 8) | (d[2] << 4) | d[3]);
}

// 0x0043b440: the 1555 version; the alpha bit is used unscaled (0 or 1).
unsigned short BlendFontTexel1555(unsigned short texel, unsigned short dest, unsigned short color) {
    unsigned char t[4];
    unsigned char d[4];
    t[0] = (texel >> 15) & 1;
    t[1] = (texel >> 10) & 0x1f;
    t[2] = (texel >> 5) & 0x1f;
    t[3] = texel & 0x1f;
    d[0] = (dest >> 15) & 1;
    d[1] = (dest >> 10) & 0x1f;
    d[2] = (dest >> 5) & 0x1f;
    d[3] = dest & 0x1f;
    float alpha = (float)t[0];
    if (color) {
        t[1] = (color >> 10) & 0x1f;
        t[2] = (color >> 5) & 0x1f;
        t[3] = color & 0x1f;
    }
    float inverse = 1.0f - alpha;
    for (unsigned int i = 0; i < 4; i++)
        d[i] = (unsigned char)(int)(t[i] * alpha + d[i] * inverse);
    return (unsigned short)((d[0] << 15) | (d[1] << 10) | (d[2] << 5) | d[3]);
}

// 0x0050b080
int UnknownOverlayText::DrawTextInto(TextureMap* texture, int x, int y, const char* text, int color) {
    if (texture->mapFormat == 1555)
        return DrawText1555(texture, x, y, text, ConvertColorToFormat(color, texture->mapFormat));
    if (texture->mapFormat == 4444)
        return DrawText4444(texture, x, y, text, ConvertColorToFormat(color, texture->mapFormat));

    TextureMap* fontTexture = currentFont->GetTexture();
    unsigned int width = texture->textureWidth;
    long pitch = 0;
    long fontPitch = 0;
    unsigned int fontWidth = fontTexture->textureWidth;
    unsigned int fontHeight = fontTexture->textureHeight;
    unsigned int* bits = (unsigned int*)texture->LockSurface(0, &pitch, 0);
    unsigned int* fontBits = (unsigned int*)fontTexture->LockSurface(0, &fontPitch, 0);
    Rectangle2D cell(0, 0, 0, 0);
    unsigned int length = strlen(text);
    unsigned int stride = (unsigned long)pitch >> 2;
    int end = 0;
    unsigned int fontStride = (unsigned long)fontPitch >> 2;
    unsigned int px = x;
    unsigned int py = y;
    unsigned int i;

    for (i = 0; i < length; i++) {
        CharacterCell* character = currentFont->FindCell(text[i]);
        if (character) {
            cell = character->GetRect();
            unsigned int srcX = (unsigned int)(fontWidth * cell.field_0x04.x);
            unsigned int srcY = (unsigned int)(fontHeight * cell.field_0x04.y);
            unsigned int glyphWidth = (unsigned int)((cell.field_0x10.x - cell.field_0x04.x) * fontWidth);
            unsigned int glyphHeight = (unsigned int)((cell.field_0x10.y - cell.field_0x04.y) * fontHeight);
            if (px + glyphWidth >= width) {
                px = x;
                py += glyphHeight;
            }
            unsigned int source = fontStride * srcY;
            unsigned int dest = py * stride;
            for (unsigned int row = 0; row < glyphHeight; row++) {
                end = px;
                for (unsigned int column = srcX; column < srcX + glyphWidth; column++) {
                    bits[dest + end] = BlendFontTexel32(fontBits[source + column], bits[dest + end], color);
                    end++;
                }
                source += fontStride;
                dest += stride;
            }
            px += glyphWidth;
        }
    }
    texture->UnlockSurface(0);
    fontTexture->UnlockSurface(0);
    texture->Upload(0, 1);
    return end;
}

// 0x0050b400
int UnknownOverlayText::DrawText4444(TextureMap* texture, int x, int y, const char* text, int color) {
    TextureMap* fontTexture = currentFont->GetTexture();
    unsigned int width = texture->textureWidth;
    long pitch = 0;
    long fontPitch = 0;
    unsigned int fontWidth = fontTexture->textureWidth;
    unsigned int fontHeight = fontTexture->textureHeight;
    unsigned short* bits = (unsigned short*)texture->LockSurface(0, &pitch, 0);
    unsigned short* fontBits = (unsigned short*)fontTexture->LockSurface(0, &fontPitch, 0);
    Rectangle2D cell(0, 0, 0, 0);
    unsigned int length = strlen(text);
    unsigned int stride = (unsigned long)pitch >> 1;
    int end = 0;
    unsigned int fontStride = (unsigned long)fontPitch >> 1;
    unsigned int px = x;
    unsigned int py = y;
    unsigned int i;

    for (i = 0; i < length; i++) {
        CharacterCell* character = currentFont->FindCell(text[i]);
        if (character) {
            cell = character->GetRect();
            unsigned int srcX = (unsigned int)(fontWidth * cell.field_0x04.x);
            unsigned int srcY = (unsigned int)(fontHeight * cell.field_0x04.y);
            unsigned int glyphWidth = (unsigned int)((cell.field_0x10.x - cell.field_0x04.x) * fontWidth);
            unsigned int glyphHeight = (unsigned int)((cell.field_0x10.y - cell.field_0x04.y) * fontHeight);
            if (px + glyphWidth >= width) {
                px = x;
                py += glyphHeight;
            }
            unsigned int source = fontStride * srcY;
            unsigned int dest = py * stride;
            for (unsigned int row = 0; row < glyphHeight; row++) {
                end = px;
                for (unsigned int column = srcX; column < srcX + glyphWidth; column++) {
                    bits[dest + end] = BlendFontTexel4444(fontBits[source + column], bits[dest + end], color);
                    end++;
                }
                source += fontStride;
                dest += stride;
            }
            px += glyphWidth;
        }
    }
    texture->UnlockSurface(0);
    fontTexture->UnlockSurface(0);
    texture->Upload(0, 1);
    return end;
}

// 0x0050b700
int UnknownOverlayText::DrawText1555(TextureMap* texture, int x, int y, const char* text, int color) {
    TextureMap* fontTexture = currentFont->GetTexture();
    unsigned int width = texture->textureWidth;
    long pitch = 0;
    long fontPitch = 0;
    unsigned int fontWidth = fontTexture->textureWidth;
    unsigned int fontHeight = fontTexture->textureHeight;
    unsigned short* bits = (unsigned short*)texture->LockSurface(0, &pitch, 0);
    unsigned short* fontBits = (unsigned short*)fontTexture->LockSurface(0, &fontPitch, 0);
    Rectangle2D cell(0, 0, 0, 0);
    unsigned int length = strlen(text);
    unsigned int stride = (unsigned long)pitch >> 1;
    int end = 0;
    unsigned int fontStride = (unsigned long)fontPitch >> 1;
    unsigned int px = x;
    unsigned int py = y;
    unsigned int i;

    for (i = 0; i < length; i++) {
        CharacterCell* character = currentFont->FindCell(text[i]);
        if (character) {
            cell = character->GetRect();
            unsigned int srcX = (unsigned int)(fontWidth * cell.field_0x04.x);
            unsigned int srcY = (unsigned int)(fontHeight * cell.field_0x04.y);
            unsigned int glyphWidth = (unsigned int)((cell.field_0x10.x - cell.field_0x04.x) * fontWidth);
            unsigned int glyphHeight = (unsigned int)((cell.field_0x10.y - cell.field_0x04.y) * fontHeight);
            if (px + glyphWidth >= width) {
                px = x;
                py += glyphHeight;
            }
            unsigned int source = fontStride * srcY;
            unsigned int dest = py * stride;
            for (unsigned int row = 0; row < glyphHeight; row++) {
                end = px;
                for (unsigned int column = srcX; column < srcX + glyphWidth; column++) {
                    bits[dest + end] = BlendFontTexel1555(fontBits[source + column], bits[dest + end], color);
                    end++;
                }
                source += fontStride;
                dest += stride;
            }
            px += glyphWidth;
        }
    }
    texture->UnlockSurface(0);
    fontTexture->UnlockSurface(0);
    texture->Upload(0, 1);
    return end;
}

// 0x0050ba00: cell rectangles are in texture coordinates of a 255-texel
// font texture.
void* UnknownOverlayText::LayoutText(const UnknownOverlayRect* rect, const char* text,
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
            cell = character->GetRect();
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


// Group2_OverlayText.cpp -- bring-up (link) definitions for TextService.cpp
// functions still missing from src/: TextService::DrawTextInto
// (0x0050b080), its 16-bit variants DrawText4444 (0x0050b400) and
// DrawText1555 (0x0050b700) and LayoutText (0x0050ba00). The four cdecl
// pixel helpers they call (ConvertColorToFormat and the three blends) are
// strict exact in src/reconstructed/PixelBlend.cpp.
//
// Status: best-guess-unreviewed. The TextService bodies are the
// near-miss bodies of samples/render/TextServiceNearMisses.cpp (register /
// scheduling differences only, per that file).

#include <string.h>

#include "TextService.h"

#include "FontTexture.h"
#include "TextureMap.h"

// 0x0050b080
int TextService::DrawTextInto(TextureMap* texture, int x, int y, const char* text, int color) {
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
            unsigned int srcX = (unsigned int)(fontWidth * cell.topLeft.x);
            unsigned int srcY = (unsigned int)(fontHeight * cell.topLeft.y);
            unsigned int glyphWidth = (unsigned int)((cell.bottomRight.x - cell.topLeft.x) * fontWidth);
            unsigned int glyphHeight = (unsigned int)((cell.bottomRight.y - cell.topLeft.y) * fontHeight);
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
int TextService::DrawText4444(TextureMap* texture, int x, int y, const char* text, unsigned short color) {
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
            unsigned int srcX = (unsigned int)(fontWidth * cell.topLeft.x);
            unsigned int srcY = (unsigned int)(fontHeight * cell.topLeft.y);
            unsigned int glyphWidth = (unsigned int)((cell.bottomRight.x - cell.topLeft.x) * fontWidth);
            unsigned int glyphHeight = (unsigned int)((cell.bottomRight.y - cell.topLeft.y) * fontHeight);
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
int TextService::DrawText1555(TextureMap* texture, int x, int y, const char* text, unsigned short color) {
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
            unsigned int srcX = (unsigned int)(fontWidth * cell.topLeft.x);
            unsigned int srcY = (unsigned int)(fontHeight * cell.topLeft.y);
            unsigned int glyphWidth = (unsigned int)((cell.bottomRight.x - cell.topLeft.x) * fontWidth);
            unsigned int glyphHeight = (unsigned int)((cell.bottomRight.y - cell.topLeft.y) * fontHeight);
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
void* TextService::LayoutText(const UnknownOverlayRect* rect, const char* text,
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
            float height = (cell.bottomRight.y - cell.topLeft.y) * 255.0f;
            float width = (cell.bottomRight.x - cell.topLeft.x) * 255.0f;
            float top = cell.topLeft.y * 255.0f;
            float left = cell.topLeft.x * 255.0f;
            size.SetCorners(left, top, left + width, top + height);
            float right = size.bottomRight.x - size.topLeft.x + x;
            float bottom = size.bottomRight.y - size.topLeft.y + y;

            vertices[i * 6 + 0].sx = x;
            vertices[i * 6 + 0].sy = y;
            vertices[i * 6 + 0].sz = vertexDepth;
            vertices[i * 6 + 0].rhw = 1.0f;
            vertices[i * 6 + 0].color = color;
            vertices[i * 6 + 0].specular = 0;
            vertices[i * 6 + 0].tu = cell.topLeft.x;
            vertices[i * 6 + 0].tv = cell.topLeft.y;

            vertices[i * 6 + 2].sx = x;
            vertices[i * 6 + 2].sy = bottom;
            vertices[i * 6 + 2].sz = vertexDepth;
            vertices[i * 6 + 2].rhw = 1.0f;
            vertices[i * 6 + 2].color = color;
            vertices[i * 6 + 2].specular = 0;
            vertices[i * 6 + 2].tu = cell.topLeft.x;
            vertices[i * 6 + 2].tv = cell.bottomRight.y;

            vertices[i * 6 + 1].sx = right;
            vertices[i * 6 + 1].sy = bottom;
            vertices[i * 6 + 1].sz = vertexDepth;
            vertices[i * 6 + 1].rhw = 1.0f;
            vertices[i * 6 + 1].color = color;
            vertices[i * 6 + 1].specular = 0;
            vertices[i * 6 + 1].tu = cell.bottomRight.x;
            vertices[i * 6 + 1].tv = cell.bottomRight.y;

            vertices[i * 6 + 3].sx = x;
            vertices[i * 6 + 3].sy = y;
            vertices[i * 6 + 3].sz = vertexDepth;
            vertices[i * 6 + 3].rhw = 1.0f;
            vertices[i * 6 + 3].color = color;
            vertices[i * 6 + 3].specular = 0;
            vertices[i * 6 + 3].tu = cell.topLeft.x;
            vertices[i * 6 + 3].tv = cell.topLeft.y;

            vertices[i * 6 + 5].sx = right;
            vertices[i * 6 + 5].sy = bottom;
            vertices[i * 6 + 5].sz = vertexDepth;
            vertices[i * 6 + 5].rhw = 1.0f;
            vertices[i * 6 + 5].color = color;
            vertices[i * 6 + 5].specular = 0;
            vertices[i * 6 + 5].tu = cell.bottomRight.x;
            vertices[i * 6 + 5].tv = cell.bottomRight.y;

            vertices[i * 6 + 4].sx = right;
            vertices[i * 6 + 4].sy = y;
            vertices[i * 6 + 4].sz = vertexDepth;
            vertices[i * 6 + 4].rhw = 1.0f;
            vertices[i * 6 + 4].color = color;
            vertices[i * 6 + 4].specular = 0;
            vertices[i * 6 + 4].tu = cell.bottomRight.x;
            vertices[i * 6 + 4].tv = cell.topLeft.y;

            x = right;
        }
    }
    return vertices;
}


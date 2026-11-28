// TextService.cpp (see TextService.h). 0x0050b080, 0x0050b400 and 0x0050b700
// are near misses (see their status comments).

#include <string.h>

#include "TextService.h"

#include "DebugAlloc.h"
#include "EArray.h"
#include "FontTexture.h"
#include "RenderTarget.h"
#include "ResourceManager.h"
#include "TextureMap.h"
#include "D3DConstants.h"

// Copies at most sizeof(dest) - 1 characters of `source` and terminates
// `dest` (a macro: retail addresses `dest` at each use).
#define COPY_NAME(dest, source)                                                    \
    {                                                                              \
        int length = strlen(source);                                               \
        int copied = length > (int)sizeof(dest) - 1 ? (int)sizeof(dest) - 1 : length; \
        strncpy(dest, source, copied);                                             \
        (dest)[copied] = 0;                                                        \
    }

// A rectangle's extent. The layout's cell sizes go through these inline
// helpers: written out in place, VC6 schedules two of LayoutText's vertex
// stores one instruction away from retail's (inferred from the bytes; the
// helpers declared but unused change nothing). As Rectangle2D members they
// give the same code here, but the extra declarations in Rectangle2D.h move
// BikeRace::CreateRacers (0x00419970) in an unrelated unit.
static inline float RectWidth(const Rectangle2D& r) {
    return r.bottomRight.x - r.topLeft.x;
}

static inline float RectHeight(const Rectangle2D& r) {
    return r.bottomRight.y - r.topLeft.y;
}

// 0x0050ac30
TextService::TextService(unsigned int count, float x, float y, float width, float height)
    : currentFont(0), fonts(new(__FILE__, 22) EArray<FontTexture*>(count)), field_0x08(0.2f),
      vertexDepth(0.001f), bounds(0, 0, 0, 0) {
    unsigned int i;

    bounds.SetCorners(x, y, x + width, y + height);
    for (i = 0; i < count; i++)
        (*fonts)[i] = 0;
}

// 0x0050ad30
TextService::~TextService() {
    unsigned int count;
    unsigned int i;

    count = fonts->Count();
    for (i = 0; i < count; i++) {
        FontTexture* font = (*fonts)[i];
        (*fonts)[i] = 0;
        FontTexture::s_UnknownManager65b478.DeleteFont(font);
    }
    currentFont = 0;
    delete fonts;
    fonts = 0;
}

// 0x0050ade0: the count test and the match leave the loop by `return` and
// `break` (a `for` or `while (i < count)` loop is rotated).
void TextService::SelectFont(const char* font) {
    unsigned int count;
    unsigned int i;

    count = fonts->Count();
    i = 0;
    while (1) {
        if (i >= count)
            return;
        if (!strcmp(font, (*fonts)[i]->GetFontName()))
            break;
        i++;
    }
    currentFont = (*fonts)[i];
}

// 0x0050ae60
void TextService::SetFont(unsigned int index, FontTexture* font) {
    (*fonts)[index] = font;
}

// 0x0050ae80
TextService* TextService::Create(const char* archive, const char** names, int count,
                                                              int format, TextureMapManager* manager, float x,
                                                              float y, float width, float height, TextureContext* textures) {
    TextService* text;
    char path[80];
    char textureName[80];
    int i;

    text = new(__FILE__, 113) TextService(count, x, y, width, height);
    g_ResourceManager->AddArchive(archive, 0);
    path[0] = 0;
    textureName[0] = 0;
    for (i = 0; i < count; i++) {
        COPY_NAME(path, names[i]);
        strcat(path, ".cell");
        COPY_NAME(textureName, names[i]);
        strcat(textureName, ".tga");
        FontTexture* font = FontTexture::Load(names[i], path, textureName, format, manager, textures);
        if (font)
            text->SetFont(i, font);
    }
    text->SelectFont(names[0]);
    return text;
}

// 0x0050b080. Not exact (530 of 890 bytes: same instructions as retail, but VC6 keeps
// glyphWidth in a stack slot and srcY in esi where retail has the reverse;
// docs/TEXTSERVICE.md forms tried)
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
            unsigned int glyphWidth = (unsigned int)(RectWidth(cell) * fontWidth);
            unsigned int glyphHeight = (unsigned int)(RectHeight(cell) * fontHeight);
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

// 0x0050b400. Not exact (398 of 758 bytes: the same glyphWidth/srcY register and
// stack-slot swap as DrawTextInto;
// docs/TEXTSERVICE.md forms tried)
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
            unsigned int glyphWidth = (unsigned int)(RectWidth(cell) * fontWidth);
            unsigned int glyphHeight = (unsigned int)(RectHeight(cell) * fontHeight);
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

// 0x0050b700. Not exact (398 of 758 bytes: the same glyphWidth/srcY register and
// stack-slot swap as DrawTextInto;
// docs/TEXTSERVICE.md forms tried)
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
            unsigned int glyphWidth = (unsigned int)(RectWidth(cell) * fontWidth);
            unsigned int glyphHeight = (unsigned int)(RectHeight(cell) * fontHeight);
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

// 0x0050ba00. Cell rectangles are in texture coordinates of a 255-texel
// font texture.
void* TextService::LayoutText(const OverlayRect* rect, const char* text,
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
            float height = RectHeight(cell) * 255.0f;
            float width = RectWidth(cell) * 255.0f;
            float top = cell.topLeft.y * 255.0f;
            float left = cell.topLeft.x * 255.0f;
            size.SetCorners(left, top, left + width, top + height);
            float right = RectWidth(size) + x;
            float bottom = RectHeight(size) + y;

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

// 0x0050bd30: texture stage 0 colour (state 0xc, restored afterwards) and
// alpha blending per texture format: 4444 and 8888 also modulate alpha.
void TextService::DrawVertices(RenderTarget* target, void* vertices, int count) {
    if (!currentFont || !vertices)
        return;

    TextureMap* texture = currentFont->GetTexture();
    texture->CreateTextureSurface(1, 0, 0);
    texture->Bind();
    int saved;
    target->GetTextureStageState(0, 0xc, &saved);
    target->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
    target->SetRenderState(D3DRENDERSTATE_FOGENABLE, 0, 0);
    int format = texture->mapFormat;
    if (format == 4444 || format == 8888) {
        target->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        target->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        target->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        target->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        target->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        target->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
    } else {
        target->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        target->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        target->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        target->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    target->SetAlphaTest(0);
    target->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0, 0);
    target->SetRenderState(D3DRENDERSTATE_ZENABLE, 0, 0);
    target->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0, 0);
    target->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, (int)vertices, count, 0);
    target->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, 0, 0);
    target->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 1, 0);
    target->SetRenderState(D3DRENDERSTATE_ZENABLE, 1, 0);
    target->SetTextureStageState(0, D3DTSS_ADDRESS, saved);
}

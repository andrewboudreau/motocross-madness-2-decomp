// TextService.cpp (see TextService.h). 0x0050ade0, 0x0050b080, 0x0050b400,
// 0x0050b700 and 0x0050ba00 are near misses kept in
// samples/render/TextServiceNearMisses.cpp.

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

// 0x0050ac30
UnknownOverlayText::UnknownOverlayText(unsigned int count, float x, float y, float width, float height)
    : currentFont(0), fonts(new(__FILE__, 22) EArray<FontTexture*>(count)), field_0x08(0.2f),
      vertexDepth(0.001f), field_0x10(0, 0, 0, 0) {
    unsigned int i;

    field_0x10.UnknownSet(x, y, x + width, y + height);
    for (i = 0; i < count; i++)
        (*fonts)[i] = 0;
}

// 0x0050ad30
UnknownOverlayText::~UnknownOverlayText() {
    unsigned int count;
    unsigned int i;

    count = fonts->Count();
    for (i = 0; i < count; i++) {
        FontTexture* font = (*fonts)[i];
        (*fonts)[i] = 0;
        FontTexture::s_UnknownManager65b478.UnknownFunction467760(font);
    }
    currentFont = 0;
    delete fonts;
    fonts = 0;
}

// 0x0050ae60
void UnknownOverlayText::SetFont(unsigned int index, FontTexture* font) {
    (*fonts)[index] = font;
}

// 0x0050ae80
UnknownOverlayText* UnknownOverlayText::Create(const char* archive, const char** names, int count,
                                                              int format, TextureMapManager* manager, float x,
                                                              float y, float width, float height, int a10) {
    UnknownOverlayText* text;
    char path[80];
    char textureName[80];
    int i;

    text = new(__FILE__, 113) UnknownOverlayText(count, x, y, width, height);
    g_UnknownResourceManager572b44->UnknownFunction4e9030(archive, 0);
    path[0] = 0;
    textureName[0] = 0;
    for (i = 0; i < count; i++) {
        COPY_NAME(path, names[i]);
        strcat(path, ".cell");
        COPY_NAME(textureName, names[i]);
        strcat(textureName, ".tga");
        FontTexture* font = FontTexture::UnknownFunction4673d0(names[i], path, textureName, format, manager, a10);
        if (font)
            text->SetFont(i, font);
    }
    text->SelectFont(names[0]);
    return text;
}

// 0x0050bd30: texture stage 0 colour (state 0xc, restored afterwards) and
// alpha blending per texture format: 4444 and 8888 also modulate alpha.
void UnknownOverlayText::DrawVertices(RenderTarget* target, void* vertices, int count) {
    if (!currentFont || !vertices)
        return;

    TextureMap* texture = currentFont->GetTexture();
    texture->UnknownVirtualSlot8(1, 0, 0);
    texture->UnknownVirtualSlot19();
    int saved;
    target->UnknownVirtualSlot6(0, 0xc, &saved);
    target->UnknownVirtualSlot7(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
    target->UnknownVirtualSlot8(D3DRENDERSTATE_FOGENABLE, 0, 0);
    int format = texture->field_0x20;
    if (format == 4444 || format == 8888) {
        target->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        target->UnknownVirtualSlot7(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        target->UnknownVirtualSlot7(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        target->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        target->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        target->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
    } else {
        target->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        target->UnknownVirtualSlot7(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        target->UnknownVirtualSlot7(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        target->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    target->UnknownVirtualSlot18(0);
    target->UnknownVirtualSlot8(D3DRENDERSTATE_ZWRITEENABLE, 0, 0);
    target->UnknownVirtualSlot8(D3DRENDERSTATE_ZENABLE, 0, 0);
    target->UnknownVirtualSlot8(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 0, 0);
    target->UnknownVirtualSlot16(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, (int)vertices, count, 0);
    target->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHATESTENABLE, 0, 0);
    target->UnknownVirtualSlot8(D3DRENDERSTATE_ZWRITEENABLE, 1, 0);
    target->UnknownVirtualSlot8(D3DRENDERSTATE_ZENABLE, 1, 0);
    target->UnknownVirtualSlot7(0, D3DTSS_ADDRESS, saved);
}

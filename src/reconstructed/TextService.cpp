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
    : field_0x00(0), field_0x04(new(__FILE__, 22) EArray<FontTexture*>(count)), field_0x08(0.2f),
      field_0x0c(0.001f), field_0x10(0, 0, 0, 0) {
    unsigned int i;

    field_0x10.UnknownSet(x, y, x + width, y + height);
    for (i = 0; i < count; i++)
        (*field_0x04)[i] = 0;
}

// 0x0050ad30
UnknownOverlayText::~UnknownOverlayText() {
    unsigned int count;
    unsigned int i;

    count = field_0x04->Count();
    for (i = 0; i < count; i++) {
        FontTexture* font = (*field_0x04)[i];
        (*field_0x04)[i] = 0;
        FontTexture::s_UnknownManager65b478.UnknownFunction467760(font);
    }
    field_0x00 = 0;
    delete field_0x04;
    field_0x04 = 0;
}

// 0x0050ae60
void UnknownOverlayText::UnknownFunction50ae60(unsigned int index, FontTexture* font) {
    (*field_0x04)[index] = font;
}

// 0x0050ae80
UnknownOverlayText* UnknownOverlayText::UnknownFunction50ae80(const char* archive, const char** names, int count,
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
            text->UnknownFunction50ae60(i, font);
    }
    text->UnknownFunction50ade0(names[0]);
    return text;
}

// 0x0050bd30: texture stage 0 colour (state 0xc, restored afterwards) and
// alpha blending per texture format: 0x115c and 0x22b8 also modulate alpha.
void UnknownOverlayText::UnknownFunction50bd30(RenderTarget* target, void* vertices, int count) {
    if (!field_0x00 || !vertices)
        return;

    TextureMap* texture = field_0x00->UnknownFunction4673c0();
    texture->UnknownVirtualSlot8(1, 0, 0);
    texture->UnknownVirtualSlot19();
    int saved;
    target->UnknownVirtualSlot6(0, 0xc, &saved);
    target->UnknownVirtualSlot7(0, 0xc, 3);
    target->UnknownVirtualSlot8(0x1c, 0, 0);
    int format = texture->field_0x20;
    if (format == 0x115c || format == 0x22b8) {
        target->UnknownVirtualSlot7(0, 1, 4);
        target->UnknownVirtualSlot7(0, 2, 2);
        target->UnknownVirtualSlot7(0, 3, 0);
        target->UnknownVirtualSlot7(0, 4, 2);
        target->UnknownVirtualSlot7(0, 5, 2);
        target->UnknownVirtualSlot8(0x1b, 1, 0);
    } else {
        target->UnknownVirtualSlot7(0, 1, 4);
        target->UnknownVirtualSlot7(0, 2, 2);
        target->UnknownVirtualSlot7(0, 3, 0);
        target->UnknownVirtualSlot7(0, 4, 1);
    }
    target->UnknownVirtualSlot18(0);
    target->UnknownVirtualSlot8(0xe, 0, 0);
    target->UnknownVirtualSlot8(7, 0, 0);
    target->UnknownVirtualSlot8(4, 0, 0);
    target->UnknownVirtualSlot16(4, 0x1c4, (int)vertices, count, 0);
    target->UnknownVirtualSlot8(0xf, 0, 0);
    target->UnknownVirtualSlot8(0xe, 1, 0);
    target->UnknownVirtualSlot8(7, 1, 0);
    target->UnknownVirtualSlot7(0, 0xc, saved);
}

#include <windows.h>

#include "Palette8.h"

#include "DebugAlloc.h"
#include "Display.h"
#include "Quantize.h"
#include "TextureMap.h"
#include "TrackGame.h"
#include "UnknownResourceManager.h"

// 0x004b6b30
Palette8* Palette8::UnknownFunction4b6b30(const char* name) {
    UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
    Palette8* palette;

    if (!entry)
        return 0;
    if (entry->field_0x10) {
        ((Palette8*)entry->field_0x10)->AddRef();
        return (Palette8*)entry->field_0x10;
    }
    entry->field_0x14->UnknownFunction461340(entry->field_0x18, 0, 0);
    palette = new(__FILE__, 46) Palette8(entry->field_0x14);
    g_UnknownResourceManager572b44->UnknownFunction4e9010(entry, palette);
    return palette;
}

// 0x004b6c00: the stream holds the first entry, the count, the mapper's
// name (length first) and 256 RGB entries. With a first entry of 10 the
// palette keeps the system's 20 static colours; otherwise unused entries
// are white.
Palette8::Palette8(UnknownTextureStream* stream) {
    int length;
    char* name;
    int i;

    field_0x708 = 0;
    field_0x70c = 0;
    if (stream->UnknownFunction461640(&field_0x710, 4, 1) != 1)
        return;
    if (stream->UnknownFunction461640(&field_0x714, 4, 1) != 1)
        return;
    if (stream->UnknownFunction461640(&length, 4, 1) != 1)
        return;
    name = (char*)DebugRealloc(0, length, __FILE__, 67);
    if (!name)
        return;
    if (stream->UnknownFunction461640(name, length, 1) != 1
        || stream->UnknownFunction461640(field_0x008, sizeof(field_0x008), 1) != 1) {
        DebugFree(name, __FILE__, 73);
        return;
    }
    if (field_0x710 == 10) {
        HDC dc = GetDC(0);
        GetSystemPaletteEntries(dc, 0, 256, (PALETTEENTRY*)field_0x308);
        ReleaseDC(0, dc);
        for (i = 0; i < 10; i++)
            field_0x308[i].flags = 0x40;
        for (i = 10; i < 246; i++)
            field_0x308[i].flags = 0x40;
        for (i = 246; i < 256; i++)
            field_0x308[i].flags = 0x40;
        for (i = 10; i < field_0x714 + 10; i++) {
            field_0x308[i].red = field_0x008[i][0];
            field_0x308[i].green = field_0x008[i][1];
            field_0x308[i].blue = field_0x008[i][2];
        }
    } else {
        for (i = field_0x710; i < field_0x714 + field_0x710; i++) {
            field_0x308[i].red = field_0x008[i][0];
            field_0x308[i].green = field_0x008[i][1];
            field_0x308[i].blue = field_0x008[i][2];
            field_0x308[i].flags = 0x40;
        }
        for (i = field_0x714 + field_0x710; i < 256; i++) {
            field_0x308[i].blue = 0xff;
            field_0x308[i].green = 0xff;
            field_0x308[i].red = 0xff;
            field_0x308[i].flags = 0x40;
        }
    }
    if (g_UnknownGlobal56e26c->field_0x0c->field_0x190->UnknownMethod5(0x44, field_0x308, &field_0x70c, 0) == 0
        && *name)
        field_0x708 = ColorMapper::UnknownFunction4dddd0(name);
    DebugFree(name, __FILE__, 170);
}

// 0x004b6ea0
Palette8::~Palette8() {
    void* entry;

    if (field_0x70c)
        field_0x70c->UnknownMethod2();
    if (field_0x708)
        field_0x708->Release();
    entry = g_UnknownResourceManager572b44->UnknownFunction4e93f0(this);
    if (entry)
        g_UnknownResourceManager572b44->UnknownFunction4e9010(entry, 0);
}

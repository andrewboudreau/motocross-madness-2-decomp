#include <windows.h>

#include "Palette8.h"

#include "DebugAlloc.h"
#include "Display.h"
#include "Quantize.h"
#include "TextureMap.h"
#include "TrackGame.h"
#include "UnknownResourceManager.h"

// 0x004b6b30
Palette8* Palette8::LoadShared(const char* name) {
    UnknownResourceEntry* entry = g_ResourceManager->FindItem(name, 0);
    Palette8* palette;

    if (!entry)
        return 0;
    if (entry->object) {
        ((Palette8*)entry->object)->AddRef();
        return (Palette8*)entry->object;
    }
    entry->archive->Seek(entry->offset, 0, 0);
    palette = new(__FILE__, 46) Palette8(entry->archive);
    g_ResourceManager->SetItemObject(entry, palette);
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

    colorMapper = 0;
    ddPalette = 0;
    if (stream->Read(&firstEntry, 4, 1) != 1)
        return;
    if (stream->Read(&entryCount, 4, 1) != 1)
        return;
    if (stream->Read(&length, 4, 1) != 1)
        return;
    name = (char*)DebugRealloc(0, length, __FILE__, 67);
    if (!name)
        return;
    if (stream->Read(name, length, 1) != 1
        || stream->Read(rgbEntries, sizeof(rgbEntries), 1) != 1) {
        DebugFree(name, __FILE__, 73);
        return;
    }
    if (firstEntry == 10) {
        HDC dc = GetDC(0);
        GetSystemPaletteEntries(dc, 0, 256, (PALETTEENTRY*)paletteEntries);
        ReleaseDC(0, dc);
        for (i = 0; i < 10; i++)
            paletteEntries[i].flags = 0x40;
        for (i = 10; i < 246; i++)
            paletteEntries[i].flags = 0x40;
        for (i = 246; i < 256; i++)
            paletteEntries[i].flags = 0x40;
        for (i = 10; i < entryCount + 10; i++) {
            paletteEntries[i].red = rgbEntries[i][0];
            paletteEntries[i].green = rgbEntries[i][1];
            paletteEntries[i].blue = rgbEntries[i][2];
        }
    } else {
        for (i = firstEntry; i < entryCount + firstEntry; i++) {
            paletteEntries[i].red = rgbEntries[i][0];
            paletteEntries[i].green = rgbEntries[i][1];
            paletteEntries[i].blue = rgbEntries[i][2];
            paletteEntries[i].flags = 0x40;
        }
        for (i = entryCount + firstEntry; i < 256; i++) {
            paletteEntries[i].blue = 0xff;
            paletteEntries[i].green = 0xff;
            paletteEntries[i].red = 0xff;
            paletteEntries[i].flags = 0x40;
        }
    }
    if (g_TrackGame->display->directDraw->CreatePalette(0x44, paletteEntries, &ddPalette, 0) == 0
        && *name)
        colorMapper = ColorMapper::LoadShared(name);
    DebugFree(name, __FILE__, 170);
}

// 0x004b6ea0
Palette8::~Palette8() {
    void* entry;

    if (ddPalette)
        ddPalette->Release();
    if (colorMapper)
        colorMapper->Release();
    entry = g_ResourceManager->FindItemByObject(this);
    if (entry)
        g_ResourceManager->SetItemObject(entry, 0);
}

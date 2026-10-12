#include "Quantize.h"

#include "DebugAlloc.h"
#include "Lzw.h"
#include "TextureMap.h"
#include "UnknownResourceManager.h"

// 0x004dddd0
ColorMapper* ColorMapper::UnknownFunction4dddd0(const char* name) {
    UnknownResourceEntry* entry = g_UnknownResourceManager572b44->FindItem(name, 0);
    ColorMapper* mapper;

    if (!entry)
        return 0;
    if (entry->field_0x10) {
        ((ColorMapper*)entry->field_0x10)->AddRef();
        return (ColorMapper*)entry->field_0x10;
    }
    entry->field_0x14->Seek(entry->field_0x18, 0, 0);
    mapper = new(__FILE__, 148) ColorMapper(entry->field_0x14);
    g_UnknownResourceManager572b44->SetItemObject(entry, mapper);
    return mapper;
}

// 0x004ddec0
void ColorMapper::Build16BitTables() {
    int i;

    for (i = 0; i < 256; i++) {
        entries555[i] = ((entries[i][0] >> 3) << 10) | ((entries[i][1] >> 3) << 5) | (entries[i][2] >> 3);
        entries565[i] = ((entries[i][0] >> 3) << 11) | ((entries[i][1] >> 2) << 5) | (entries[i][2] >> 3);
    }
}

// 0x004ddf40: reads the palette and the two lookup tables, each stored raw
// or LZW-compressed, then reserves one more entry for magenta (the colour
// key) when the palette is not full.
ColorMapper::ColorMapper(UnknownTextureStream* stream) {
    int compressed;

    if (stream->Read(&firstIndex, 4, 1) != 1)
        return;
    if (stream->Read(&entryCount, 4, 1) != 1)
        return;
    if (stream->Read(&compressed, 4, 1) != 1)
        return;
    if (compressed) {
        unsigned char* buffer;
        int size;

        buffer = (unsigned char*)DebugMalloc(0x10000, __FILE__, 200);
        if (!buffer)
            return;
        if (stream->Read(&size, 4, 1) != 1)
            return;
        if (size == 0x300) {
            if (stream->Read(entries, 0x300, 1) != 1)
                return;
        } else {
            if (stream->Read(buffer, size, 1) != 1)
                return;
            ExpandLzw(entries[0], buffer, 0x300);
        }
        if (stream->Read(&size, 4, 1) != 1)
            return;
        if (size == 0x8000) {
            if (stream->Read(indexFrom555, 0x8000, 1) != 1)
                return;
        } else {
            if (stream->Read(buffer, size, 1) != 1)
                return;
            ExpandLzw(indexFrom555, buffer, 0x8000);
        }
        if (stream->Read(&size, 4, 1) != 1)
            return;
        if (size == 0x10000) {
            if (stream->Read(indexFrom565, 0x10000, 1) != 1)
                return;
        } else {
            if (stream->Read(buffer, size, 1) != 1)
                return;
            ExpandLzw(indexFrom565, buffer, 0x10000);
        }
        DebugFree(buffer, __FILE__, 261);
    } else {
        if (stream->Read(entries, 0x300, 1) != 1)
            return;
        if (stream->Read(indexFrom555, 0x8000, 1) != 1)
            return;
        if (stream->Read(indexFrom565, 0x10000, 1) != 1)
            return;
    }
    if (entryCount != 256) {
        entries[entryCount + firstIndex][0] = 0xff;
        entries[entryCount + firstIndex][1] = 0;
        entries[entryCount + firstIndex][2] = 0xff;
        indexFrom565[0xf81f] = indexFrom555[0x7c1f] = (unsigned char)(entryCount + firstIndex);
        entryCount++;
    }
    Build16BitTables();
}

// 0x004de200
ColorMapper::~ColorMapper() {
    void* entry = g_UnknownResourceManager572b44->FindItemByObject(this);
    if (entry)
        g_UnknownResourceManager572b44->SetItemObject(entry, 0);
}

// 0x004de270
unsigned char* ColorMapper::GetColors() {
    return entries[0];
}

// 0x004de280
unsigned char* ColorMapper::UnknownFunction4de280() {
    return indexFrom555;
}

// 0x004de290
unsigned char* ColorMapper::UnknownFunction4de290() {
    return indexFrom565;
}

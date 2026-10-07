#include "TextureMap.h"

#include "DebugAlloc.h"
#include "ManagedTexture.h"
#include "MatrixUtil.h"
#include "Palette8.h"
#include "Pixtrans.h"
#include "Tgafile.h"
#include "UnknownResourceManager.h"

// The four vector constants that open about 73 retail files (see
// src/krusty2/math/Math3D.h): 0x0068a340, 0x0068a350, 0x0068a360 and
// 0x0068a330, initialised by 0x0050a3a0..0x0050a4db.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// 0x0050a4e0
TextureMap::TextureMap(TextureMapManager* manager, int registered) {
    field_0x08 = 0;
    field_0x0c = 0;
    field_0x10 = manager;
    field_0x24 = 0;
    field_0x14 = 0;
    field_0x18 = 0;
    field_0x20 = 0;
    if (registered && manager)
        manager->UnknownFunction5112f0(this);
    field_0x44 = 0;
    field_0x30 = 0;
    field_0x28 = 0;
    field_0x40 = 0;
    field_0x68 = 0;
    field_0x6c = 0;
}

// 0x0050a590: see TextureMap.h.
TextureMap* UnknownFunction50a590(TextureMapManager* manager, const char* name, int format,
                                  Palette8* palette, int flags, int addressU, int addressV,
                                  UnknownTextureFormatChoice* choice, int alphaThreshold,
                                  unsigned int key, int addRef, int fromArchive)
{
    char* paletteName = 0;
    UnknownTexturePalette* texturePalette = 0;
    int fromFile = 1;
    void* surfacePalette = 0;
    UnknownTgaFile* file = 0;
    int width;
    int height;
    int fileFormat;
    int dataSize;
    if (choice)
        manager = choice->field_0x00;
    UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 0);
    if (entry) {
        if (entry->field_0x10) {
            if (addRef)
                ((TextureMap*)entry->field_0x10)->AddRef();
            return (TextureMap*)entry->field_0x10;
        }
        entry->field_0x14->UnknownFunction461340(entry->field_0x18, 0, 0);
    }
    if (entry && fromArchive) {
        fromFile = 0;
        int nameSize;
        if (entry->field_0x14->UnknownFunction461640(&fileFormat, 4, 1) != 1
            || entry->field_0x14->UnknownFunction461640(&dataSize, 4, 1) != 1
            || entry->field_0x14->UnknownFunction461640(&width, 4, 1) != 1
            || entry->field_0x14->UnknownFunction461640(&height, 4, 1) != 1
            || entry->field_0x14->UnknownFunction461640(&nameSize, 4, 1) != 1
            || (paletteName = (char*)DebugRealloc(0, nameSize, __FILE__, 123)) == 0)
            goto failed;    // retail shares one `return 0` with the failed name read
        if (entry->field_0x14->UnknownFunction461640(paletteName, nameSize, 1) == 1) {
            if (format == 8 && !palette && *paletteName) {
                int position = entry->field_0x14->UnknownFunction461600();
                char mode = entry->field_0x14->UnknownFunction43e9e0();
                palette = Palette8::UnknownFunction4b6b30(paletteName);
                if (!palette) {
                    texturePalette = 0;
                    surfacePalette = 0;
                } else {
                    texturePalette = (UnknownTexturePalette*)palette->field_0x708;
                    surfacePalette = palette->field_0x70c;
                }
                entry->field_0x14->UnknownFunction461340(position, 0, 0);
                entry->field_0x14->UnknownFunction43e9b0(mode);
            }
        } else {
            DebugFree(paletteName, __FILE__, 138);
failed:
            return 0;
        }
    } else {
        file = UnknownFunction5125c0(name, 0, (int)g_UnknownResourceManager572b44);
        if (!file)
            return 0;
        width = file->width;
        height = file->height;
    }

    int alpha = UnknownFunction511ad0(format);
    TextureMap* map;
    if (choice && (alpha && choice->field_0x08 || !alpha && choice->field_0x04))
        map = new(__FILE__, 207) ManagedTexture(manager);
    else
        map = new(__FILE__, 213) PCTextureMap(manager, 1);
    int minimumSize = (flags & 2) ? 1 : width;
    if (choice) {
        if (alpha) {
            if (choice->field_0x08)
                choice->field_0x08->UnknownFunction50c6c0((ManagedTexture*)map);
        } else if (choice->field_0x04) {
            choice->field_0x04->UnknownFunction50c6c0((ManagedTexture*)map);
        }
    }
    if (palette) {
        texturePalette = (UnknownTexturePalette*)palette->field_0x708;
        surfacePalette = palette->field_0x70c;
        map->field_0x28 = palette;
        if (addRef)
            palette->AddRef();
    }
    if (file && choice) {
        int halvings = choice->field_0x14;
        while (width > 32 && halvings--) {
            int sourceFormat = file->bitsPerPixel == 16 ? 0x22b : (file->bitsPerPixel == 24 ? 0x378 : 0x22b8);
            if (UnknownFunction4d1b90(file->bits, file->bits, width / 2, height / 2, width / 2, width, 1,
                                      sourceFormat, texturePalette, 2)) {
                width /= 2;
                height /= 2;
            }
        }
    }
    if (fromFile) {
        int sourceFormat = file->bitsPerPixel == 16 ? 0x22b : (file->bitsPerPixel == 24 ? 0x378 : 0x22b8);
        map->UnknownVirtualSlot4(file->bits, width, height, width, minimumSize, sourceFormat, format,
                                 texturePalette, flags, surfacePalette, 1, 0, addressU, addressV, choice,
                                 alphaThreshold, key);
        UnknownFunction512dd0(file);
        if (!entry)
            g_UnknownResourceManager572b44->UnknownFunction4e96b0(name, (int)map);
    } else if (map->UnknownVirtualSlot5(entry->field_0x14, width, height, minimumSize, fileFormat, dataSize,
                                        format, texturePalette, flags, surfacePalette, addressU, addressV,
                                        choice, alphaThreshold, key)) {
        g_UnknownResourceManager572b44->UnknownFunction4e9010(entry, map);
    }
    if (paletteName)
        DebugFree(paletteName, __FILE__, 320);
    return map;
}

// 0x0050ab40: releases +0x28, unregisters from the manager and drops the
// resource manager's entry for the texture.
TextureMap::~TextureMap() {
    if (field_0x28)
        field_0x28->Release();
    if (field_0x10)
        field_0x10->UnknownFunction511300(this);
    void* entry = g_UnknownResourceManager572b44->UnknownFunction4e93f0(this);
    if (entry)
        g_UnknownResourceManager572b44->UnknownFunction4e9010(entry, 0);
}

// 0x0050abd0
int TextureMap::UnknownFunction50abd0(int addressU, int addressV) {
    if (UnknownFunction511ad0(field_0x20)) {
        for (int i = 0; i < field_0x44; i++) {
            if (field_0x48[i].state == 0x13)
                field_0x48[i].value = addressU;
            else if (field_0x48[i].state == 0x14)
                field_0x48[i].value = addressV;
        }
        return 1;
    }
    return 0;
}

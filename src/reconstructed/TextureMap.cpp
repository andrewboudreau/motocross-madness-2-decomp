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
    nextTexture = 0;
    previousTexture = 0;
    ownerManager = manager;
    mipCount = 0;
    textureWidth = 0;
    textureHeight = 0;
    mapFormat = 0;
    if (registered && manager)
        manager->RegisterTexture(this);
    statePairCount = 0;
    usesColorKey = 0;
    sourcePalette = 0;
    field_0x40 = 0;
    mapFlags = 0;
    halvings = 0;
}

// 0x0050a590: see TextureMap.h.
TextureMap* TerrainLoadTexture(TextureMapManager* manager, const char* name, int format,
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
        manager = choice->manager;
    UnknownResourceEntry* entry = g_ResourceManager->FindItem(name, 0);
    if (entry) {
        if (entry->object) {
            if (addRef)
                ((TextureMap*)entry->object)->AddRef();
            return (TextureMap*)entry->object;
        }
        entry->archive->Seek(entry->offset, 0, 0);
    }
    if (entry && fromArchive) {
        fromFile = 0;
        int nameSize;
        if (entry->archive->Read(&fileFormat, 4, 1) != 1
            || entry->archive->Read(&dataSize, 4, 1) != 1
            || entry->archive->Read(&width, 4, 1) != 1
            || entry->archive->Read(&height, 4, 1) != 1
            || entry->archive->Read(&nameSize, 4, 1) != 1
            || (paletteName = (char*)DebugRealloc(0, nameSize, __FILE__, 123)) == 0)
            goto failed;    // retail shares one `return 0` with the failed name read
        if (entry->archive->Read(paletteName, nameSize, 1) == 1) {
            if (format == 8 && !palette && *paletteName) {
                int position = entry->archive->Tell();
                char mode = entry->archive->GetTextMode();
                palette = Palette8::LoadShared(paletteName);
                if (!palette) {
                    texturePalette = 0;
                    surfacePalette = 0;
                } else {
                    texturePalette = (UnknownTexturePalette*)palette->colorMapper;
                    surfacePalette = palette->ddPalette;
                }
                entry->archive->Seek(position, 0, 0);
                entry->archive->SetTextMode(mode);
            }
        } else {
            DebugFree(paletteName, __FILE__, 138);
failed:
            return 0;
        }
    } else {
        file = LoadFile(name, 0, (int)g_ResourceManager);
        if (!file)
            return 0;
        width = file->width;
        height = file->height;
    }

    int alpha = IsFormat4444Or8888(format);
    TextureMap* map;
    if (choice && (alpha && choice->alphaGroup || !alpha && choice->opaqueGroup))
        map = new(__FILE__, 207) ManagedTexture(manager);
    else
        map = new(__FILE__, 213) PCTextureMap(manager, 1);
    int minimumSize = (flags & 2) ? 1 : width;
    if (choice) {
        if (alpha) {
            if (choice->alphaGroup)
                choice->alphaGroup->AddTexture((ManagedTexture*)map);
        } else if (choice->opaqueGroup) {
            choice->opaqueGroup->AddTexture((ManagedTexture*)map);
        }
    }
    if (palette) {
        texturePalette = (UnknownTexturePalette*)palette->colorMapper;
        surfacePalette = palette->ddPalette;
        map->sourcePalette = palette;
        if (addRef)
            palette->AddRef();
    }
    if (file && choice) {
        int halvings = choice->halvings;
        while (width > 32 && halvings--) {
            int sourceFormat = file->bitsPerPixel == 16 ? 0x22b : (file->bitsPerPixel == 24 ? 0x378 : 0x22b8);
            if (Downsample(file->bits, file->bits, width / 2, height / 2, width / 2, width, 1,
                                      sourceFormat, texturePalette, 2)) {
                width /= 2;
                height /= 2;
            }
        }
    }
    if (fromFile) {
        int sourceFormat = file->bitsPerPixel == 16 ? 0x22b : (file->bitsPerPixel == 24 ? 0x378 : 0x22b8);
        map->CreateSurface(file->bits, width, height, width, minimumSize, sourceFormat, format,
                           texturePalette, flags, surfacePalette, 1, 0, addressU, addressV, choice,
                           alphaThreshold, key);
        FreeFile(file);
        if (!entry)
            g_ResourceManager->AddObject(name, map);
    } else if (map->LoadSurface(entry->archive, width, height, minimumSize, fileFormat, dataSize,
                                format, texturePalette, flags, surfacePalette, addressU, addressV,
                                choice, alphaThreshold, key)) {
        g_ResourceManager->SetItemObject(entry, map);
    }
    if (paletteName)
        DebugFree(paletteName, __FILE__, 320);
    return map;
}

// 0x0050ab40: releases +0x28, unregisters from the manager and drops the
// resource manager's entry for the texture.
TextureMap::~TextureMap() {
    if (sourcePalette)
        sourcePalette->Release();
    if (ownerManager)
        ownerManager->UnregisterTexture(this);
    void* entry = g_ResourceManager->FindItemByObject(this);
    if (entry)
        g_ResourceManager->SetItemObject(entry, 0);
}

// 0x0050abd0
int TextureMap::SetAddressModes(int addressU, int addressV) {
    if (IsFormat4444Or8888(mapFormat)) {
        for (int i = 0; i < statePairCount; i++) {
            if (statePairs[i].state == 0x13)
                statePairs[i].value = addressU;
            else if (statePairs[i].state == 0x14)
                statePairs[i].value = addressV;
        }
        return 1;
    }
    return 0;
}

#include "TextureMap.h"

#include "UnknownResourceManager.h"

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

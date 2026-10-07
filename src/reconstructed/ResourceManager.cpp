// D:\aardvark\VC\krusty2\ResourceManager.cpp: the named-resource table.
//
// Evidence: the __FILE__ literal 0x00572b64 is passed by every allocation
// from 0x004e8de6 to 0x004e975f (lines 24..478 below) and by the unwind
// funclets 0x0054daa0..0x0054db26. The file's code runs from the global
// object's initializer 0x004e8d30 to the out-of-line inline 0x004e9960;
// SceneManager.cpp starts at 0x004e9980. The data run 0x00572b44 (the
// manager pointer), 0x00572b48 (ResourceItem's type descriptor), the
// __FILE__ literal and "RS2" (0x00572b90) are this file's.

#include <string.h>

#include "ResourceManager.h"

#include "DebugAlloc.h"
#include "TextureMap.h"

// 0x00689c78; constructed by 0x004e8d40, destroyed by 0x004e8d60.
UnknownResourceManager g_UnknownResourceManager689c78;
UnknownResourceManager* g_UnknownResourceManager572b44 = &g_UnknownResourceManager689c78;

// 0x004e8d70
ResourceItem::ResourceItem()
{
    UnknownFunction4e8de0();
}

// 0x004e8de0
void ResourceItem::UnknownFunction4e8de0()
{
    field_0x0c = 1;
    field_0x08 = (char*)DebugMalloc(1, __FILE__, 24);
    field_0x08[0] = 0;
    field_0x10 = 0;
    field_0x14 = 0;
    field_0x18 = 0;
    field_0x1c = 0;
}

// 0x004e8e10
ResourceItem::~ResourceItem()
{
    if (field_0x08)
        DebugFree(field_0x08, __FILE__, 42);
}

// 0x004e8e80
UnknownResourceManager::UnknownResourceManager()
{
    field_0x08 = 0;
    field_0x14 = 0;
    field_0x0c = 0;
    field_0x10 = 0;
    field_0x00 = 0;
    field_0x04 = 0;
}

// 0x004e8ea0
UnknownResourceManager::~UnknownResourceManager()
{
    int i;

    if (field_0x10) {
        for (i = 0; i < field_0x0c; i++) {
            if (field_0x10[i])
                delete field_0x10[i];
        }
        DebugFree(field_0x10, __FILE__, 135);
    }
    if (field_0x14) {
        for (i = 0; i < field_0x08; i++) {
            if (field_0x14[i])
                field_0x14[i]->Release();
        }
        DebugFree(field_0x14, __FILE__, 139);
    }
    field_0x08 = 0;
    field_0x14 = 0;
    field_0x0c = 0;
    field_0x10 = 0;
    field_0x00 = 0;
    field_0x04 = 0;
}

// 0x004e8f40
int UnknownResourceManager::UnknownFunction4e8f40(ResourceItem* item)
{
    for (int i = 0; i < field_0x08; i++) {
        if (!_stricmp(item->field_0x08, field_0x14[i]->field_0x08)) {
            UnknownTextureStream* archive = field_0x14[i]->field_0x14;
            if (archive && archive->UnknownFunction4e9960())
                return 0;

            field_0x14[i]->field_0x18 = item->field_0x18;
            field_0x14[i]->field_0x1c = item->field_0x1c;
            ResourceItem* existing = field_0x14[i];
            UnknownTextureStream* old = existing->field_0x14;
            existing->field_0x14 = item->field_0x14;
            if (old != item->field_0x14) {
                if (!UnknownInlineArchiveUsed(old) && old)
                    delete old;
            }
            return 1;
        }
    }
    return 0;
}

// 0x004e9010
void UnknownResourceManager::UnknownFunction4e9010(void* entry, void* object)
{
    ResourceItem* item = (ResourceItem*)entry;

    item->field_0x10 = object;
    if (!object && !item->field_0x14)
        UnknownFunction4e9900(item);
}

// 0x004e9030 (opens an "RS2" archive and adds its items) is a near miss:
// samples/render/ResourceManagerNearMisses.cpp.

// 0x004e9360
ResourceItem* UnknownResourceManager::UnknownFunction4e9360(const char* name, int a)
{
    for (int i = 0; i < field_0x08; i++) {
        if (!_stricmp(field_0x14[i]->field_0x08, name)) {
            UnknownTextureStream* archive = field_0x14[i]->field_0x14;
            if (archive && (a || archive->UnknownFunction4e9960()))
                field_0x14[i]->field_0x14->UnknownFunction461340(field_0x14[i]->field_0x18, 0, 0);
            return field_0x14[i];
        }
    }
    return 0;
}

// 0x004e93f0
void* UnknownResourceManager::UnknownFunction4e93f0(void* object)
{
    for (int i = 0; i < field_0x08; i++) {
        if (field_0x14[i]->field_0x10 == object)
            return field_0x14[i];
    }
    return 0;
}

// 0x004e9430
int UnknownResourceManager::UnknownFunction4e9430(const char* name, const char* path)
{
    char buffer[0x104];

    if (path)
        strncpy(buffer, path, sizeof(buffer));
    else
        strncpy(buffer, name, sizeof(buffer));

    UnknownTextureStream* archive = new(__FILE__, 387) UnknownTextureStream((int)this);
    if (archive->UnknownFunction460f50(buffer, "rb", archive->UnknownFunction460e70(buffer))) {
        if (field_0x08 + 1 > field_0x00) {
            field_0x14 = (ResourceItem**)DebugRealloc(field_0x14, (field_0x08 + 1) * 4, __FILE__, 393);
            if (field_0x14) {
                field_0x14[field_0x08] = 0;
                field_0x00 = field_0x08 + 1;
            }
        }
        if (field_0x0c + 1 > field_0x04) {
            field_0x10 = (UnknownTextureStream**)DebugRealloc(field_0x10, (field_0x0c + 8) * 4, __FILE__, 405);
            if (field_0x10) {
                field_0x04 = field_0x0c + 8;
                field_0x10[field_0x0c++] = archive;
            }
        }
        if (field_0x10 && field_0x14) {
            field_0x14[field_0x08] = new(__FILE__, 418) ResourceItem;
            field_0x14[field_0x08]->field_0x0c = strlen(name) + 1;
            field_0x14[field_0x08]->field_0x08 =
                (char*)DebugRealloc(field_0x14[field_0x08]->field_0x08, field_0x14[field_0x08]->field_0x0c,
                                    __FILE__, 420);
            strcpy(field_0x14[field_0x08]->field_0x08, name);
            field_0x14[field_0x08]->field_0x18 = 0;
            field_0x14[field_0x08]->field_0x1c = 0;
            field_0x14[field_0x08]->field_0x14 = archive;
            if (UnknownFunction4e8f40(field_0x14[field_0x08])) {
                field_0x14[field_0x08]->Release();
                return 1;
            }
            field_0x08++;
        }
        return 1;
    }
    if (archive)
        delete archive;
    return 0;
}

// 0x004e96b0
int UnknownResourceManager::UnknownFunction4e96b0(const char* name, void* object)
{
    if (field_0x08 + 1 > field_0x00) {
        field_0x14 = (ResourceItem**)DebugRealloc(field_0x14, (field_0x08 + 1) * 4, __FILE__, 464);
        if (field_0x14) {
            field_0x14[field_0x08] = 0;
            field_0x00 = field_0x08 + 1;
        }
    }
    if (field_0x14) {
        field_0x14[field_0x08] = new(__FILE__, 476) ResourceItem;
        field_0x14[field_0x08]->field_0x0c = strlen(name) + 1;
        field_0x14[field_0x08]->field_0x08 =
            (char*)DebugRealloc(field_0x14[field_0x08]->field_0x08, field_0x14[field_0x08]->field_0x0c,
                                __FILE__, 478);
        strcpy(field_0x14[field_0x08]->field_0x08, name);
        field_0x14[field_0x08]->field_0x18 = 0;
        field_0x14[field_0x08]->field_0x1c = 0;
        field_0x14[field_0x08]->field_0x14 = 0;
        field_0x14[field_0x08]->field_0x10 = object;
        if (UnknownFunction4e8f40(field_0x14[field_0x08]))
            field_0x14[field_0x08]->Release();
        else
            field_0x08++;
    }
    return 1;
}

// 0x004e9830: refuses (0) while another holder references one of the
// archive's items; the archive itself is not deleted here.
int UnknownResourceManager::UnknownFunction4e9830(UnknownTextureStream* archive)
{
    int used = 0;
    int i;

    for (i = 0; i < field_0x08; i++) {
        if (field_0x14[i]->field_0x14 == archive) {
            used++;
            // Retail compares unsigned (`ja`): held by anyone but the table.
            if ((unsigned int)field_0x14[i]->GetRefCount() > 1)
                return 0;
        }
    }
    if (used) {
        for (i = 0; i < field_0x0c; i++) {
            if (field_0x10[i] == archive) {
                field_0x10[i] = field_0x10[field_0x0c - 1];
                field_0x0c--;
                break;
            }
        }
        for (i = 0; i < field_0x08; i++) {
            if (field_0x14[i]->field_0x14 == archive && !field_0x14[i]->Release()) {
                field_0x14[i] = field_0x14[field_0x08 - 1];
                field_0x14[field_0x08 - 1] = 0;
                field_0x08--;
                i--;
            }
        }
    }
    return 1;
}

// 0x004e9900
void UnknownResourceManager::UnknownFunction4e9900(ResourceItem* item)
{
    for (int i = 0; i < field_0x08; i++) {
        if (field_0x14[i] == item && !field_0x14[i]->Release()) {
            field_0x14[i] = field_0x14[field_0x08 - 1];
            field_0x14[field_0x08 - 1] = 0;
            field_0x08--;
            return;
        }
    }
}

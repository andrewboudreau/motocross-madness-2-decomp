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
    nameSize = 1;
    name = (char*)DebugMalloc(1, __FILE__, 24);
    name[0] = 0;
    object = 0;
    archive = 0;
    offset = 0;
    size = 0;
}

// 0x004e8e10
ResourceItem::~ResourceItem()
{
    if (name)
        DebugFree(name, __FILE__, 42);
}

// 0x004e8e80
UnknownResourceManager::UnknownResourceManager()
{
    itemCount = 0;
    items = 0;
    archiveCount = 0;
    archives = 0;
    itemCapacity = 0;
    archiveCapacity = 0;
}

// 0x004e8ea0
UnknownResourceManager::~UnknownResourceManager()
{
    int i;

    if (archives) {
        for (i = 0; i < archiveCount; i++) {
            if (archives[i])
                delete archives[i];
        }
        DebugFree(archives, __FILE__, 135);
    }
    if (items) {
        for (i = 0; i < itemCount; i++) {
            if (items[i])
                items[i]->Release();
        }
        DebugFree(items, __FILE__, 139);
    }
    itemCount = 0;
    items = 0;
    archiveCount = 0;
    archives = 0;
    itemCapacity = 0;
    archiveCapacity = 0;
}

// 0x004e8f40
int UnknownResourceManager::MergeWithExisting(ResourceItem* item)
{
    for (int i = 0; i < itemCount; i++) {
        if (!_stricmp(item->name, items[i]->name)) {
            UnknownTextureStream* archive = items[i]->archive;
            if (archive && archive->UnknownFunction4e9960())
                return 0;

            items[i]->offset = item->offset;
            items[i]->size = item->size;
            ResourceItem* existing = items[i];
            UnknownTextureStream* old = existing->archive;
            existing->archive = item->archive;
            if (old != item->archive) {
                if (!UnknownInlineArchiveUsed(old) && old)
                    delete old;
            }
            return 1;
        }
    }
    return 0;
}

// 0x004e9010
void UnknownResourceManager::SetItemObject(void* entry, void* object)
{
    ResourceItem* item = (ResourceItem*)entry;

    item->object = object;
    if (!object && !item->archive)
        ReleaseItem(item);
}

// 0x004e9030 (opens an "RS2" archive and adds its items) is a near miss:
// samples/render/ResourceManagerNearMisses.cpp.

// 0x004e9360
ResourceItem* UnknownResourceManager::FindItem(const char* name, int a)
{
    for (int i = 0; i < itemCount; i++) {
        if (!_stricmp(items[i]->name, name)) {
            UnknownTextureStream* archive = items[i]->archive;
            if (archive && (a || archive->UnknownFunction4e9960()))
                items[i]->archive->Seek(items[i]->offset, 0, 0);
            return items[i];
        }
    }
    return 0;
}

// 0x004e93f0
void* UnknownResourceManager::FindItemByObject(void* object)
{
    for (int i = 0; i < itemCount; i++) {
        if (items[i]->object == object)
            return items[i];
    }
    return 0;
}

// 0x004e9430
int UnknownResourceManager::AddLooseFile(const char* name, const char* path)
{
    char buffer[0x104];

    if (path)
        strncpy(buffer, path, sizeof(buffer));
    else
        strncpy(buffer, name, sizeof(buffer));

    UnknownTextureStream* archive = new(__FILE__, 387) UnknownTextureStream((int)this);
    if (archive->Open(buffer, "rb", archive->UnknownFunction460e70(buffer))) {
        if (itemCount + 1 > itemCapacity) {
            items = (ResourceItem**)DebugRealloc(items, (itemCount + 1) * 4, __FILE__, 393);
            if (items) {
                items[itemCount] = 0;
                itemCapacity = itemCount + 1;
            }
        }
        if (archiveCount + 1 > archiveCapacity) {
            archives = (UnknownTextureStream**)DebugRealloc(archives, (archiveCount + 8) * 4, __FILE__, 405);
            if (archives) {
                archiveCapacity = archiveCount + 8;
                archives[archiveCount++] = archive;
            }
        }
        if (archives && items) {
            items[itemCount] = new(__FILE__, 418) ResourceItem;
            items[itemCount]->nameSize = strlen(name) + 1;
            items[itemCount]->name =
                (char*)DebugRealloc(items[itemCount]->name, items[itemCount]->nameSize,
                                    __FILE__, 420);
            strcpy(items[itemCount]->name, name);
            items[itemCount]->offset = 0;
            items[itemCount]->size = 0;
            items[itemCount]->archive = archive;
            if (MergeWithExisting(items[itemCount])) {
                items[itemCount]->Release();
                return 1;
            }
            itemCount++;
        }
        return 1;
    }
    if (archive)
        delete archive;
    return 0;
}

// 0x004e96b0
int UnknownResourceManager::AddObject(const char* name, void* object)
{
    if (itemCount + 1 > itemCapacity) {
        items = (ResourceItem**)DebugRealloc(items, (itemCount + 1) * 4, __FILE__, 464);
        if (items) {
            items[itemCount] = 0;
            itemCapacity = itemCount + 1;
        }
    }
    if (items) {
        items[itemCount] = new(__FILE__, 476) ResourceItem;
        items[itemCount]->nameSize = strlen(name) + 1;
        items[itemCount]->name =
            (char*)DebugRealloc(items[itemCount]->name, items[itemCount]->nameSize,
                                __FILE__, 478);
        strcpy(items[itemCount]->name, name);
        items[itemCount]->offset = 0;
        items[itemCount]->size = 0;
        items[itemCount]->archive = 0;
        items[itemCount]->object = object;
        if (MergeWithExisting(items[itemCount]))
            items[itemCount]->Release();
        else
            itemCount++;
    }
    return 1;
}

// 0x004e9830: refuses (0) while another holder references one of the
// archive's items; the archive itself is not deleted here.
int UnknownResourceManager::RemoveArchiveItems(UnknownTextureStream* archive)
{
    int used = 0;
    int i;

    for (i = 0; i < itemCount; i++) {
        if (items[i]->archive == archive) {
            used++;
            // Retail compares unsigned (`ja`): held by anyone but the table.
            if ((unsigned int)items[i]->GetRefCount() > 1)
                return 0;
        }
    }
    if (used) {
        for (i = 0; i < archiveCount; i++) {
            if (archives[i] == archive) {
                archives[i] = archives[archiveCount - 1];
                archiveCount--;
                break;
            }
        }
        for (i = 0; i < itemCount; i++) {
            if (items[i]->archive == archive && !items[i]->Release()) {
                items[i] = items[itemCount - 1];
                items[itemCount - 1] = 0;
                itemCount--;
                i--;
            }
        }
    }
    return 1;
}

// 0x004e9900
void UnknownResourceManager::ReleaseItem(ResourceItem* item)
{
    for (int i = 0; i < itemCount; i++) {
        if (items[i] == item && !items[i]->Release()) {
            items[i] = items[itemCount - 1];
            items[itemCount - 1] = 0;
            itemCount--;
            return;
        }
    }
}

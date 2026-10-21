#include "UnknownResourceManager.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include <stddef.h>

// Both historical include paths must agree on the concrete storage used by
// Scene's stack objects and by resources returned from FindItem.
typedef char ManagerSize[sizeof(UnknownResourceManager) == 0x18 ? 1 : -1];
typedef char SceneManagerSize[sizeof(UnknownSceneResourceManager) == 0x18 ? 1 : -1];
typedef char ItemSize[sizeof(UnknownResourceEntry) == 0x20 ? 1 : -1];
typedef char ManagerArchives[offsetof(UnknownResourceManager, archives) == 0x10 ? 1 : -1];
typedef char ManagerItems[offsetof(UnknownResourceManager, items) == 0x14 ? 1 : -1];
typedef char ItemObject[offsetof(ResourceItem, object) == 0x10 ? 1 : -1];
typedef char ItemArchive[offsetof(ResourceItem, archive) == 0x14 ? 1 : -1];
typedef char ItemOffset[offsetof(ResourceItem, offset) == 0x18 ? 1 : -1];

// Compile-time type identity, including lifecycle and pointer return types.
void CheckResourceTypes(UnknownSceneResourceManager* manager)
{
    UnknownResourceManager* same = manager;
    ResourceItem* (UnknownResourceManager::*find)(const char*, int) =
        &UnknownResourceManager::FindItem;
    int (UnknownResourceManager::*remove)(UnknownTextureStream*) =
        &UnknownResourceManager::RemoveArchiveItems;
    int (UnknownResourceManager::*add)(const char*, void*) =
        &UnknownResourceManager::AddObject;
    UnknownResourceEntry* entry = (same->*find)("", 0);
    ResourceItem* item = entry;
}

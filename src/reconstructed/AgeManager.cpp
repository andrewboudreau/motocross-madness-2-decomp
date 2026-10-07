#include "AgeManager.h"

#include <stdlib.h>

#include "DebugAlloc.h"

// 0x00401000
AgeManager::AgeManager() {
    entryCount = 0;
    entryCapacity = 0;
    entryList = 0;
    currentAge = 0;
    totalBytes = 0;
}

// 0x00401020
AgeManager::~AgeManager() {
    if (entryList)
        DebugFree(entryList, __FILE__, 17);
}

// 0x00401040
void AgeManager::AdvanceAge() {
    currentAge++;
}

// 0x00401050
void AgeManager::Register(AgeEntry* entry, int (*callback)(void* owner, int context),
                                       void* owner, int context, int size) {
    if (entryCount == entryCapacity) {
        AgeEntry** entries = (AgeEntry**)DebugRealloc(entryList, (entryCapacity + 1000) * sizeof(AgeEntry*),
                                                      __FILE__, 38);
        if (!entries)
            return;
        entryList = entries;
        entryCapacity += 1000;
    }
    entryList[entryCount] = entry;
    entry->callback = callback;
    entry->context = context;
    entry->owner = owner;
    entry->age = currentAge;
    entry->size = size;
    entryCount++;
    totalBytes += size;
}

// 0x004010d0
void AgeManager::Unregister(AgeEntry* entry) {
    if (entryList && entryCount) {
        for (int i = 0; i < entryCount; i++) {
            if (entryList[i] == entry) {
                if (--entryCount) {
                    entryList[i] = entryList[entryCount];
                    entryList[entryCount] = 0;
                } else {
                    entryList[0] = 0;
                }
                return;
            }
        }
    }
}

// 0x00401130
int AgeManager::TotalSize(int* stale) {
    if (stale) {
        *stale = 0;
        for (int i = 0; i < entryCount; i++) {
            AgeEntry* entry = entryList[i];
            if (entry->age < currentAge)
                *stale += entry->size;
        }
    }
    return totalBytes;
}

// 0x00401180: qsort comparator, most recently used first.
static int UnknownFunction401180(const void* a, const void* b) {
    int delta = (*(AgeEntry**)b)->age - (*(AgeEntry**)a)->age;
    if (delta < 0)
        return -1;
    return delta != 0;
}

// 0x004011b0
int AgeManager::EvictStale(int limit) {
    if (totalBytes < limit)
        return 0;
    qsort(entryList, entryCount, sizeof(AgeEntry*), UnknownFunction401180);
    int freed = 0;
    int evicted = 0;
    for (int i = entryCount - 1; i >= 0; i--) {
        AgeEntry* entry = entryList[i];
        if (entry->age < currentAge) {
            int size = entry->size;
            if (entry->callback(entry->owner, entry->context)) {
                freed += size;
                evicted++;
                if (totalBytes - freed <= limit)
                    break;
            }
        }
    }
    entryCount -= evicted;
    totalBytes -= freed;
    return freed;
}

// 0x00401250
void AgeManager::MarkUsed(AgeEntry* entry) {
    entry->age = currentAge;
}

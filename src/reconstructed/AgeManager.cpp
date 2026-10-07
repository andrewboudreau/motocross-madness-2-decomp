#include "AgeManager.h"

#include <stdlib.h>

#include "DebugAlloc.h"

// 0x00401000
AgeManager::AgeManager() {
    field_0x04 = 0;
    field_0x08 = 0;
    field_0x0c = 0;
    field_0x00 = 0;
    field_0x10 = 0;
}

// 0x00401020
AgeManager::~AgeManager() {
    if (field_0x0c)
        DebugFree(field_0x0c, __FILE__, 17);
}

// 0x00401040
void AgeManager::UnknownFunction401040() {
    field_0x00++;
}

// 0x00401050
void AgeManager::UnknownFunction401050(AgeEntry* entry, int (*callback)(void* owner, int context),
                                       void* owner, int context, int size) {
    if (field_0x04 == field_0x08) {
        AgeEntry** entries = (AgeEntry**)DebugRealloc(field_0x0c, (field_0x08 + 1000) * sizeof(AgeEntry*),
                                                      __FILE__, 38);
        if (!entries)
            return;
        field_0x0c = entries;
        field_0x08 += 1000;
    }
    field_0x0c[field_0x04] = entry;
    entry->callback = callback;
    entry->context = context;
    entry->owner = owner;
    entry->age = field_0x00;
    entry->size = size;
    field_0x04++;
    field_0x10 += size;
}

// 0x004010d0
void AgeManager::UnknownFunction4010d0(AgeEntry* entry) {
    if (field_0x0c && field_0x04) {
        for (int i = 0; i < field_0x04; i++) {
            if (field_0x0c[i] == entry) {
                if (--field_0x04) {
                    field_0x0c[i] = field_0x0c[field_0x04];
                    field_0x0c[field_0x04] = 0;
                } else {
                    field_0x0c[0] = 0;
                }
                return;
            }
        }
    }
}

// 0x00401130
int AgeManager::UnknownFunction401130(int* stale) {
    if (stale) {
        *stale = 0;
        for (int i = 0; i < field_0x04; i++) {
            AgeEntry* entry = field_0x0c[i];
            if (entry->age < field_0x00)
                *stale += entry->size;
        }
    }
    return field_0x10;
}

// 0x00401180: qsort comparator, most recently used first.
static int UnknownFunction401180(const void* a, const void* b) {
    int delta = (*(AgeEntry**)b)->age - (*(AgeEntry**)a)->age;
    if (delta < 0)
        return -1;
    return delta != 0;
}

// 0x004011b0
int AgeManager::UnknownFunction4011b0(int limit) {
    if (field_0x10 < limit)
        return 0;
    qsort(field_0x0c, field_0x04, sizeof(AgeEntry*), UnknownFunction401180);
    int freed = 0;
    int evicted = 0;
    for (int i = field_0x04 - 1; i >= 0; i--) {
        AgeEntry* entry = field_0x0c[i];
        if (entry->age < field_0x00) {
            int size = entry->size;
            if (entry->callback(entry->owner, entry->context)) {
                freed += size;
                evicted++;
                if (field_0x10 - freed <= limit)
                    break;
            }
        }
    }
    field_0x04 -= evicted;
    field_0x10 -= freed;
    return freed;
}

// 0x00401250
void AgeManager::UnknownFunction401250(AgeEntry* entry) {
    entry->age = field_0x00;
}

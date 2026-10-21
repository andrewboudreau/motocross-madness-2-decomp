// Native execution of the separately compiled production AgeManager object.
// Only debug allocation boundaries and resource callbacks are test-owned.
#include "../../src/reconstructed/AgeEntry.h"
#include "../../src/reconstructed/AgeManager.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char ManagerSize[sizeof(AgeManager) == 0x14 ? 1 : -1];
typedef char EntrySize[sizeof(AgeEntry) == 0x14 ? 1 : -1];
typedef char ManagerList[offsetof(AgeManager, entryList) == 0x0c ? 1 : -1];
typedef char EntryCallback[offsetof(AgeEntry, callback) == 4 ? 1 : -1];
typedef char EntryOwner[offsetof(AgeEntry, owner) == 8 ? 1 : -1];
typedef char EntryContext[offsetof(AgeEntry, context) == 12 ? 1 : -1];
typedef char EntryBytes[offsetof(AgeEntry, size) == 16 ? 1 : -1];

static unsigned allocations, frees, requested, checks;
static int failAllocation;
static void* allocated;
static void Require(int value, int line) {
    if (!value) { printf("AgeManager check failed at line %d\n", line); exit(2); }
}
#define CHECK(value) Require((value), __LINE__)
static void CheckFile(const char* path) {
    const char* leaf = strrchr(path, '\\');
    if (!leaf) leaf = strrchr(path, '/');
    CHECK(!_stricmp(leaf ? leaf + 1 : path, "AgeManager.cpp"));
}
void* DebugRealloc(void* pointer, unsigned size, const char* file, int line) {
    CheckFile(file); CHECK(line == 38); CHECK(pointer == allocated);
    ++allocations; requested = size;
    if (failAllocation) return 0;
    void* result = realloc(pointer, size);
    CHECK(result != 0); allocated = result; return result;
}
void DebugFree(void* pointer, const char* file, int line) {
    CheckFile(file); CHECK(line == 17); CHECK(pointer == allocated);
    ++frees; free(pointer); allocated = 0;
}

struct Owner { AgeEntry* entry; int index, context, succeed; };
static int calls, order[8], contexts[8];
static AgeManager* active;
static int expectedCount, expectedBytes;
static int Evict(void* value, int context) {
    Owner* owner = (Owner*)value;
    CHECK(owner->context == context);
    CHECK(owner->entry->owner == value && owner->entry->context == context);
    CHECK(active->entryCount == expectedCount && active->totalBytes == expectedBytes);
    CHECK(calls < 8); order[calls] = owner->index; contexts[calls++] = context;
    // Real grid callbacks clear entry.size. The manager must debit the size
    // captured before the callback, rather than reload this zero afterward.
    if (owner->succeed) owner->entry->size = 0;
    return owner->succeed ? -3 : 0;
}
static const int sizes[4] = {3, 5, 7, 11};
static const int values[4] = {0, -7, 0x12345678, (int)0x80000000};

static void EvictionCases() {
    const int limits[] = {-1, 0, 3, 11, 18, 25, 26, 27, 0x7fffffff};
    for (int used = 0; used < 16; ++used)
    for (int success = 0; success < 16; ++success)
    for (int limit = 0; limit < 9; ++limit) {
        unsigned oldFrees = frees;
        {
            AgeManager manager; active = &manager;
            AgeEntry entries[4]; Owner owners[4];
            CHECK(!manager.currentAge && !manager.entryCount && !manager.entryCapacity &&
                  !manager.entryList && !manager.totalBytes);
            for (int n = 0; n < 4; ++n) {
                memset(&entries[n], 0xa5, sizeof(entries[n]));
                owners[n].entry = &entries[n]; owners[n].index = n;
                owners[n].context = values[n]; owners[n].succeed = (success >> n) & 1;
                manager.currentAge = 10 + n;
                manager.Register(&entries[n], Evict, &owners[n], values[n], sizes[n]);
                CHECK(entries[n].age == 10u + n && entries[n].callback == Evict &&
                      entries[n].owner == &owners[n] && entries[n].context == values[n] &&
                      entries[n].size == sizes[n]);
            }
            CHECK(manager.entryCount == 4 && manager.entryCapacity == 1000 && requested == 4000);
            manager.AdvanceAge(); CHECK(manager.currentAge == 14);
            int staleExpected = 26;
            for (n = 0; n < 4; ++n) if (used & (1 << n)) {
                manager.MarkUsed(&entries[n]); staleExpected -= sizes[n];
            }
            int stale = -99;
            CHECK(manager.TotalSize(&stale) == 26 && stale == staleExpected);
            CHECK(manager.TotalSize(0) == 26);
            int expectedOrder[4], expectedCalls = 0, freed = 0, removed = 0, changed = 0;
            if (limits[limit] <= 26) {
                for (n = 0; n < 4; ++n) if (!(used & (1 << n))) {
                    expectedOrder[expectedCalls++] = n;
                    if (success & (1 << n)) {
                        freed += sizes[n]; ++removed; changed |= 1 << n;
                        if (26 - freed <= limits[limit]) break;
                    }
                }
            }
            calls = 0; expectedCount = 4; expectedBytes = 26;
            CHECK(manager.EvictStale(limits[limit]) == freed);
            CHECK(calls == expectedCalls && manager.entryCount == 4 - removed &&
                  manager.totalBytes == 26 - freed && manager.currentAge == 14);
            for (n = 0; n < calls; ++n) {
                CHECK(order[n] == expectedOrder[n]);
                CHECK(contexts[n] == values[order[n]]);
            }
            unsigned seen = 0;
            for (n = 0; n < 4; ++n) {
                CHECK(entries[n].size == ((changed & (1 << n)) ? 0 : sizes[n]));
                CHECK(entries[n].context == values[n] && entries[n].owner == &owners[n] &&
                      entries[n].callback == Evict && entries[n].age == ((used & (1 << n)) ? 14u : 10u + n));
                int index = manager.entryList[n] - entries;
                CHECK(index >= 0 && index < 4 && !(seen & (1u << index)));
                seen |= 1u << index;
                if (limits[limit] > 26) CHECK(index == n); // early return does not sort
                else if (n) CHECK(manager.entryList[n - 1]->age >= manager.entryList[n]->age);
            }
            CHECK(seen == 15);
            ++checks;
        }
        CHECK(frees == oldFrees + 1 && !allocated);
    }
}

static void RegistrationAndLifetime() {
    unsigned oldFrees = frees;
    {
        AgeManager manager;
        AgeEntry entry, untouched;
        memset(&entry, 0x7b, sizeof(entry)); untouched = entry;
        failAllocation = 1;
        manager.Register(&entry, Evict, &entry, -99, 27);
        CHECK(!manager.entryList && !manager.entryCount && !manager.entryCapacity && !manager.totalBytes);
        CHECK(!memcmp(&entry, &untouched, sizeof(entry)) && requested == 4000);
        failAllocation = 0;
        ++checks;
    }
    CHECK(frees == oldFrees); // no successful allocation, nothing to free
    {
        AgeManager manager;
        AgeEntry entries[1001]; memset(entries, 0x3a, sizeof(entries));
        for (int n = 0; n < 1000; ++n) manager.Register(&entries[n], Evict, &entries[n], n, 1);
        CHECK(manager.entryCount == 1000 && manager.entryCapacity == 1000 && manager.totalBytes == 1000);
        AgeEntry untouched = entries[1000]; AgeEntry** old = manager.entryList;
        failAllocation = 1;
        manager.Register(&entries[1000], Evict, &entries[1000], 1000, 1);
        CHECK(requested == 8000 && manager.entryList == old && manager.entryCount == 1000 &&
              manager.entryCapacity == 1000 && manager.totalBytes == 1000);
        CHECK(!memcmp(&entries[1000], &untouched, sizeof(untouched)));
        failAllocation = 0;
        manager.Register(&entries[1000], Evict, &entries[1000], 1000, 1);
        CHECK(manager.entryCount == 1001 && manager.entryCapacity == 2000 && manager.totalBytes == 1001);
        for (n = 0; n < 1001; ++n) CHECK(manager.entryList[n] == &entries[n]);
        ++checks;
    }
    CHECK(frees == oldFrees + 1 && !allocated);
    {
        AgeManager manager;
        AgeEntry entries[4], absent;
        calls = 0;
        for (int n = 0; n < 4; ++n) manager.Register(&entries[n], Evict, &entries[n], n, sizes[n]);
        manager.Unregister(&absent);
        CHECK(manager.entryCount == 4 && manager.totalBytes == 26);
        manager.Unregister(&entries[1]);
        CHECK(manager.entryCount == 3 && manager.entryList[1] == &entries[3] && !manager.entryList[3]);
        manager.Unregister(&entries[0]);
        CHECK(manager.entryCount == 2 && manager.entryList[0] == &entries[2] && !manager.entryList[2]);
        manager.Unregister(&entries[3]);
        CHECK(manager.entryCount == 1 && manager.entryList[0] == &entries[2] && !manager.entryList[1]);
        manager.Unregister(&entries[2]); manager.Unregister(&absent);
        CHECK(!manager.entryCount && !manager.entryList[0] && manager.TotalSize(0) == 26 && !calls);
        ++checks;
    }
    CHECK(frees == oldFrees + 2 && !allocated);
    {
        AgeManager manager; AgeEntry entries[2];
        manager.currentAge = 0xffffffff;
        manager.Register(&entries[0], Evict, 0, -1, 5);
        manager.AdvanceAge(); CHECK(manager.currentAge == 0);
        manager.Register(&entries[1], Evict, 0, 0, 7);
        int stale;
        CHECK(manager.TotalSize(&stale) == 12 && stale == 0);
        manager.currentAge = 1;
        CHECK(manager.TotalSize(&stale) == 12 && stale == 7);
        manager.MarkUsed(&entries[0]);
        CHECK(entries[0].age == 1 && entries[0].size == 5 && entries[0].context == -1);
        ++checks;
    }
    CHECK(frees == oldFrees + 3 && !allocated);
}

int main() {
    RegistrationAndLifetime(); EvictionCases();
    CHECK(!allocated && !failAllocation);
    printf("AgeManager: %u native allocation, lifetime, callback and accounting cases passed\n", checks);
    return 0;
}

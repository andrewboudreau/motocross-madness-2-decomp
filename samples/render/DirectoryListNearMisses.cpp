// Near-miss dirlist.cpp candidates, kept out of src/reconstructed until they
// match. See docs/DIRLIST.md. The canonical file is included first so the
// TU-local macro and inline helpers are the same.
//
// 0x0044a600 (104 bytes, the found-file filter): everything matches except
// the attribute load: retail loads the whole dword and masks it with 0xff
// before `>> 4 & 1`; with an unsigned char or a cast VC6 here loads only
// the low byte.
//
// 0x0044ac30 (1130 bytes, CombinedDirectoryList slot 1): the same calls,
// allocations and merge order; retail keeps the merged count in ebp and the
// second list in ebx with a 0x20-byte frame, VC6 here allocates one more
// stack slot and different registers.

#include "../../src/reconstructed/DirectoryList.cpp"

// 0x0044a600
int DirectoryList::UnknownFunction44a600(const char* name, unsigned int attributes) {
    if (!_stricmp("", name) || !_stricmp(".", name) || !_stricmp("..", name))
        return 0;
    if (!includeFiles)
        return ((unsigned char)attributes >> 4) & 1;
    return 1;
}

// 0x0044ac30: lists both directories, then merges them: the second
// directory's entries tagged 5, each first-directory name found there
// retagged 3 and the others appended with 3.
int CombinedDirectoryList::UnknownVirtualSlot1() {
    DirectoryList* first;
    DirectoryList* second;
    UnknownDirectoryEntry* merged;
    int total;
    int found;
    int i;
    int j;

    count = 0;
    current = 0;
    if (entries) {
        delete entries;
        entries = 0;
    }
    first = new (__FILE__, 464) DirectoryList;
    second = new (__FILE__, 465) DirectoryList;
    if (!first || !second)
        return 0;
    first->UnknownFunction44a220(pattern, 1);
    first->UnknownFunction44a1d0(field_0x21c);
    first->UnknownVirtualSlot1();
    first->UnknownFunction44aab0();
    second->UnknownFunction44a220(pattern, 1);
    second->UnknownFunction44a1d0(field_0x320);
    second->UnknownVirtualSlot1();
    second->UnknownFunction44aab0();

    total = second->count;
    merged = new (__FILE__, 480) UnknownDirectoryEntry[total + first->count];
    for (i = 0; i < second->count; i++) {
        COPY_NAME(merged[i].name, second->entries[i].name);
        merged[i].attributes = 5;
    }
    for (i = 0; i < first->count; i++) {
        found = 0;
        for (j = 0; j < total; j++) {
            if (!strcmp(first->entries[i].name, merged[j].name)) {
                merged[j].field_0x108 = 3;
                found = 1;
                total--;
            }
        }
        if (!found) {
            COPY_NAME(merged[total].name, first->entries[i].name);
            merged[total].attributes = 3;
            total++;
        }
    }

    count = total;
    entries = new (__FILE__, 512) UnknownDirectoryEntry[total];
    if (!entries) {
        delete first;
        delete second;
        delete merged;
        return 0;
    }
    for (i = 0; i < count; i++) {
        COPY_NAME(entries[i].name, merged[i].name);
        entries[i].attributes = merged[i].field_0x108;
    }
    delete first;
    delete second;
    delete merged;
    return 1;
}

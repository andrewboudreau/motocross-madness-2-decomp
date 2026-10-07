#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "DirectoryList.h"

#include "DebugAlloc.h"

// Copies at most MAX_PATH - 1 characters of `source` and terminates `dest`
// (a macro: retail addresses `dest` at each use).
#define COPY_NAME(dest, source)                                      \
    {                                                               \
        int length = strlen(source);                                \
        int copied = length > MAX_PATH - 1 ? MAX_PATH - 1 : length; \
        strncpy(dest, source, copied);                              \
        (dest)[copied] = 0;                                         \
    }

// Inline; sets the entry's name (truncated) and attributes.
inline void UnknownDirectoryEntry::UnknownFunctionSet(const char* fileName, unsigned int fileAttributes) {
    COPY_NAME(name, fileName);
    attributes = fileAttributes;
}

// 0x00449e60
int UnknownDriveList::UnknownFunction449e60() {
    return UnknownFunction449e70();
}

// 0x00449e70
int UnknownDriveList::UnknownFunction449e70() {
    char root[4];
    char* strings;
    int size;
    int length;
    int found;
    char fileSystem[MAX_PATH];

    count = 0;
    if (drives)
        delete drives;
    size = GetLogicalDriveStringsA(0, 0) * 2;
    strings = (char*)DebugMalloc(size, __FILE__, 32);
    if (!strings)
        return 0;
    GetLogicalDriveStringsA(size, strings);
    found = 0;
    for (length = 0; strings[length]; length += 4)
        ;
    drives = new (__FILE__, 44) UnknownDriveEntry[length];
    if (!drives)
        return 0;
    for (length = 0; strings[length]; length += 4) {
        sprintf(root, "%c:\\", strings[length]);
        strcpy(drives[found].root, root);
        drives[found].type = GetDriveTypeA(drives[found].root);
        if (drives[found].type == DRIVE_FIXED || drives[found].type == DRIVE_CDROM) {
            GetVolumeInformationA(drives[found].root, drives[found].volumeName, MAX_PATH, 0, 0, 0, fileSystem,
                                  MAX_PATH);
            found++;
        }
    }
    count = found;
    DebugFree(strings, __FILE__, 63);
    return 1;
}

// 0x0044a000
UnknownDriveList::~UnknownDriveList() {
    if (drives)
        delete drives;
}

// 0x0044a010
int UnknownDriveList::UnknownFunction44a010(const char* volumeName, int* index, unsigned int type) {
    int i;

    *index = 0;
    if (!drives)
        return 0;
    for (i = 0; i < count; i++) {
        if (!strcmp(volumeName, drives[i].volumeName)) {
            if (type) {
                if (type == drives[i].type) {
                    *index = i;
                    return 1;
                }
            } else {
                *index = i;
                return 1;
            }
        }
    }
    return 0;
}

// 0x0044a0b0
int UnknownDriveList::UnknownFunction44a0b0(int index, char* root) {
    strcpy(root, "");
    if (!drives)
        return 0;
    strcpy(root, drives[index].root);
    return 1;
}

// 0x0044a130
DirectoryList::DirectoryList() {
    GetCurrentDirectoryA(MAX_PATH, directory);
    COPY_NAME(pattern, "");
    entries = 0;
    count = 0;
    current = 0;
    includeFiles = 1;
    DirectoryList::UnknownVirtualSlot1();
}

// 0x0044a1d0
void DirectoryList::UnknownFunction44a1d0(const char* directory) {
    COPY_NAME(this->directory, directory);
}

// 0x0044a220
void DirectoryList::UnknownFunction44a220(const char* pattern, int includeFiles) {
    COPY_NAME(this->pattern, pattern);
    this->includeFiles = includeFiles;
}

// 0x0044a270
int DirectoryList::UnknownFunction44a270(int index, char* path) {
    strcpy(path, "");
    if (!entries || index < 0 || index >= count)
        return 0;
    strcpy(path, entries[index].name);
    return 1;
}

// 0x0044a2f0
int DirectoryList::UnknownFunction44a2f0(int index, char* name) {
    char drive[_MAX_DRIVE];
    char fileName[_MAX_FNAME];
    char path[MAX_PATH];
    char extension[_MAX_EXT];
    char dir[_MAX_DIR];

    strcpy(name, "");
    if (!UnknownFunction44a270(index, path))
        return 0;
    _splitpath(path, drive, dir, fileName, extension);
    strcpy(name, fileName);
    return 1;
}

// 0x0044a3b0
int DirectoryList::UnknownFunction44a3b0(char* path) {
    strcpy(path, "");
    if (++current >= count) {
        current = count - 1;
        return 0;
    }
    strcpy(path, entries[current].name);
    return 1;
}

// 0x0044a440
int DirectoryList::UnknownFunction44a440(char* path) {
    strcpy(path, "");
    if (count > 0) {
        strcpy(path, entries[current].name);
        return 1;
    }
    return 0;
}

// 0x0044a4c0
int DirectoryList::UnknownFunction44a4c0(char* name) {
    char drive[_MAX_DRIVE];
    char fileName[_MAX_FNAME];
    char path[MAX_PATH];
    char extension[_MAX_EXT];
    char dir[_MAX_DIR];

    if (!UnknownFunction44a3b0(path))
        return 0;
    _splitpath(path, drive, dir, fileName, extension);
    strcpy(name, fileName);
    return 1;
}

// 0x0044a550
int DirectoryList::UnknownFunction44a550(char* name) {
    char drive[_MAX_DRIVE];
    char fileName[_MAX_FNAME];
    char path[MAX_PATH];
    char extension[_MAX_EXT];
    char dir[_MAX_DIR];

    strcpy(name, "");
    if (!UnknownFunction44a440(path))
        return 0;
    _splitpath(path, drive, dir, fileName, extension);
    strcpy(name, fileName);
    return 1;
}

// 0x0044a670
int DirectoryList::UnknownVirtualSlot1() {
    char search[MAX_PATH];
    WIN32_FIND_DATAA data;
    HANDLE find;
    int found;

    count = 0;
    current = 0;
    if (entries) {
        delete entries;
        entries = 0;
    }
    if (directory[strlen(directory) - 1] == '\\')
        sprintf(search, "%s%s", directory, pattern);
    else
        sprintf(search, "%s\\%s", directory, pattern);

    found = 0;
    find = FindFirstFileA(search, &data);
    if (find == INVALID_HANDLE_VALUE)
        return 0;
    if (UnknownFunction44a600(data.cFileName, data.dwFileAttributes))
        found = 1;
    while (FindNextFileA(find, &data)) {
        if (UnknownFunction44a600(data.cFileName, data.dwFileAttributes))
            found++;
    }
    FindClose(find);
    count = found;

    found = 0;
    if (count > 0) {
        entries = new (__FILE__, 328) UnknownDirectoryEntry[count];
        find = FindFirstFileA(search, &data);
        if (find == INVALID_HANDLE_VALUE)
            return 0;
        if (UnknownFunction44a600(data.cFileName, data.dwFileAttributes)) {
            entries[0].UnknownFunctionSet(data.cFileName, data.dwFileAttributes);
            found = 1;
        }
        while (FindNextFileA(find, &data)) {
            if (UnknownFunction44a600(data.cFileName, data.dwFileAttributes)) {
                entries[found].UnknownFunctionSet(data.cFileName, data.dwFileAttributes);
                found++;
            }
        }
        FindClose(find);
    }
    return 1;
}

// 0x0044a910
int DirectoryList::UnknownFunction44a910(const char* name) {
    int i;

    for (i = 0; i < count; i++) {
        if (!_stricmp(name, entries[i].name))
            return 1;
    }
    return 0;
}

// 0x0044a960
int DirectoryList::UnknownFunction44a960(const char* path) {
    char child[MAX_PATH];
    DirectoryList* list = new (__FILE__, 374) DirectoryList;
    int i;

    if (!list)
        return 0;
    list->UnknownFunction44a1d0(path);
    list->UnknownFunction44a220("*", 1);
    list->UnknownVirtualSlot1();
    for (i = 0; i < list->count; i++) {
        sprintf(child, "%s\\%s", path, list->entries[i].name);
        if (list->entries[i].attributes & FILE_ATTRIBUTE_DIRECTORY) {
            UnknownFunction44a960(child);
        } else if (!DeleteFileA(child)) {
            delete list;
            return 0;
        }
    }
    delete list;
    return RemoveDirectoryA(path);
}

// 0x0044aa90
int DirectoryList::UnknownFunction44aa90(const void* a, const void* b) {
    return _stricmp((const char*)a, (const char*)b);
}

// 0x0044aab0
void DirectoryList::UnknownFunction44aab0() {
    if (count > 0)
        qsort(entries, count, sizeof(UnknownDirectoryEntry), UnknownFunction44aa90);
}

// 0x0044aae0
DirectoryList::~DirectoryList() {
    if (entries) {
        delete entries;
        entries = 0;
    }
}

// 0x0044ab10
CombinedDirectoryList::CombinedDirectoryList() {
    strcpy(field_0x21c, "");
    strcpy(field_0x320, "");
}

// 0x0044aba0
CombinedDirectoryList::~CombinedDirectoryList() {
}

// 0x0044abb0
void CombinedDirectoryList::UnknownFunction44abb0(const char* first, const char* second) {
    COPY_NAME(field_0x21c, first);
    COPY_NAME(field_0x320, second);
}

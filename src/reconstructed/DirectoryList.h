#pragma once

// Reconstruction of D:\aardvark\VC\krusty2\dirlist.cpp (literal __FILE__ at
// 0x00569204; code 0x00449e60..0x0044b09f). Evidence and the per-function
// table are in docs/DIRLIST.md. Field roles follow the Win32 calls that fill
// them; method names are provisional.

// One logical drive (0x10c bytes): its root ("%c:\\"), GetDriveTypeA's
// result and, for fixed (3) and CD-ROM (5) drives, GetVolumeInformationA's
// volume name.
struct UnknownDriveEntry {
    char root[4];
    unsigned int type;
    char volumeName[0x104];
};

// The logical drives. The game creates one (0x005221e7, 8 bytes; the
// constructor is a folded two-field reset) and looks for the CD-ROM whose
// volume is "MCM2" (0x00523bf0).
class UnknownDriveList {
public:
    UnknownDriveList();                                   // called at 0x004676a0 (folded)
    ~UnknownDriveList();                                  // 0x0044a000
    int UnknownFunction449e60();                          // 0x00449e60: rescans
    // 0x00449e70: lists the fixed and CD-ROM drives; 0 when out of memory.
    int UnknownFunction449e70();
    // 0x0044a010: the index of the drive whose volume is `volumeName` (and
    // whose type is `type`, unless 0); 1 when found.
    int UnknownFunction44a010(const char* volumeName, int* index, unsigned int type);
    int UnknownFunction44a0b0(int index, char* root);     // 0x0044a0b0

    UnknownDriveEntry* drives;
    int count;
};

// One directory entry (0x10c bytes): the found file's name and attributes
// (FILE_ATTRIBUTE_NORMAL until set) and a tag CombinedDirectoryList uses.
struct UnknownDirectoryEntry {
    UnknownDirectoryEntry() {
        name[0] = 0;
        field_0x108 = 3;
        attributes = 0x80;                                 // FILE_ATTRIBUTE_NORMAL
    }
    void UnknownFunctionSet(const char* fileName, unsigned int fileAttributes);

    char name[0x104];
    unsigned int attributes;
    int field_0x108;
};

// RTTI: DirectoryList (root; CombinedDirectoryList derives from it; vtable
// 0x005516e0; 0x21c bytes, the size TrackGame slot 4 allocates).
class DirectoryList {
public:
    DirectoryList();                                       // 0x0044a130
    virtual ~DirectoryList();                              // 0x0044aae0 (deleting wrapper 0x0044a1b0)
    // 0x0044a670: lists `pattern` in `directory` (FindFirstFileA/FindNextFileA)
    // into `entries`; 0 when nothing matches.
    virtual int UnknownVirtualSlot1();
    void UnknownFunction44a1d0(const char* directory);     // 0x0044a1d0
    void UnknownFunction44a220(const char* pattern, int includeFiles); // 0x0044a220
    int UnknownFunction44a270(int index, char* path);      // 0x0044a270: entry `index`'s name
    int UnknownFunction44a2f0(int index, char* name);      // 0x0044a2f0: ... without its extension
    int UnknownFunction44a3b0(char* path);                 // 0x0044a3b0: advances to the next entry
    int UnknownFunction44a440(char* path);                 // 0x0044a440: the current entry
    int UnknownFunction44a4c0(char* name);                 // 0x0044a4c0: next listed name
    int UnknownFunction44a550(char* name);                 // 0x0044a550: current listed name
    // 0x0044a600: whether a found file is listed: not "", "." or "..", and
    // a directory unless `includeFiles` is set.
    int UnknownFunction44a600(const char* name, unsigned int attributes);
    int UnknownFunction44a910(const char* name);           // 0x0044a910: whether `name` is listed
    // 0x0044a960: deletes the directory `path` and everything below it.
    int UnknownFunction44a960(const char* path);
    void UnknownFunction44aab0();                          // 0x0044aab0: sorts the entries by name
    static int UnknownFunction44aa90(const void* a, const void* b); // 0x0044aa90: qsort order

    int count;
    int current;
    char pattern[0x104];
    char directory[0x104];
    UnknownDirectoryEntry* entries;
    int includeFiles;
};

// RTTI: CombinedDirectoryList : DirectoryList (vtable 0x005516ec): lists
// the union of two directories.
class CombinedDirectoryList : public DirectoryList {
public:
    CombinedDirectoryList();                               // 0x0044ab10
    virtual ~CombinedDirectoryList();                      // 0x0044aba0 (deleting wrapper 0x0044ab80)
    virtual int UnknownVirtualSlot1();                     // 0x0044ac30
    void UnknownFunction44abb0(const char* first, const char* second); // 0x0044abb0

    char field_0x21c[0x104];
    char field_0x320[0x104];
};

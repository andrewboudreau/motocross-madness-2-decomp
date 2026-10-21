#pragma once

// AgeManager.h -- the reconstructed D:\aardvark\VC\krusty2\AgeManager.cpp
// (literal __FILE__ at 0x005665c0, xrefs 0x0040102a and 0x0040106a; code
// 0x00401000..0x0040125a, ArcadeObject.cpp follows at 0x00401260).
//
// No RTTI: the class has no virtual functions. The class name follows the
// file name (tier 2); method and member names are provisional. The 0x14-byte
// layout is decoded from the constructor 0x00401000 and the users below.
// Terrain and grid caches use this manager at Terrain+0xc84 and +0xc88.

struct AgeEntry;

class AgeManager {
public:
    AgeManager();                              // 0x00401000
    ~AgeManager();                             // 0x00401020 (line 17 free)

    void AdvanceAge();                         // 0x00401040: advances the age
    // 0x00401050: registers `entry` (grows the pointer array by 1000 at line 38).
    void Register(AgeEntry* entry, int (*callback)(void* owner, int context),
                  void* owner, int context, int size);
    void Unregister(AgeEntry* entry);          // 0x004010d0: unregisters `entry`
    // 0x00401130: the accounted total; `stale` receives the bytes of the
    // entries not used during the current age.
    int TotalSize(int* stale);
    // 0x004011b0: evicts stale entries, least recently used first, until at
    // most `limit` bytes remain; returns the bytes freed.
    int EvictStale(int limit);
    void MarkUsed(AgeEntry* entry);            // 0x00401250: marks `entry` used

    unsigned int currentAge;                   // current age
    int entryCount;                            // entry count
    int entryCapacity;                            // capacity
    AgeEntry** entryList;                     // entries
    int totalBytes;                            // accounted bytes
};

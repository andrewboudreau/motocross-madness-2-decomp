#pragma once

// AgeManager.h -- the reconstructed D:\aardvark\VC\krusty2\AgeManager.cpp
// (literal __FILE__ at 0x005665c0, xrefs 0x0040102a and 0x0040106a; code
// 0x00401000..0x0040125a, ArcadeObject.cpp follows at 0x00401260).
//
// No RTTI: the class has no virtual functions. The class name follows the
// file name (tier 2); method and member names are provisional. The 0x14-byte
// layout is decoded from the constructor 0x00401000 and the users below.
// Griddraw.h's GridAgeManager/GridAgeEntry are the terrain's view of the
// same objects (Terrain+0xc88).

// One registered resource. The manager keeps pointers to entries owned by
// the callers; an entry is stamped with the manager's current age when it is
// registered or used (0x00401250).
struct AgeEntry {
    unsigned int age;                          // +0x00 last use
    int (*callback)(void* owner, int context); // +0x04 evicts the resource, nonzero on success
    void* owner;                               // +0x08
    int context;                               // +0x0c second callback argument
    int size;                                  // +0x10 accounted bytes
};

class AgeManager {
public:
    AgeManager();                              // 0x00401000
    ~AgeManager();                             // 0x00401020 (line 17 free)

    void UnknownFunction401040();              // 0x00401040: advances the age
    // 0x00401050: registers `entry` (grows the pointer array by 1000 at line 38).
    void UnknownFunction401050(AgeEntry* entry, int (*callback)(void* owner, int context),
                               void* owner, int context, int size);
    void UnknownFunction4010d0(AgeEntry* entry);    // 0x004010d0: unregisters `entry`
    // 0x00401130: the accounted total; `stale` receives the bytes of the
    // entries not used during the current age.
    int UnknownFunction401130(int* stale);
    // 0x004011b0: evicts stale entries, least recently used first, until at
    // most `limit` bytes remain; returns the bytes freed.
    int UnknownFunction4011b0(int limit);
    void UnknownFunction401250(AgeEntry* entry);    // 0x00401250: marks `entry` used

    unsigned int field_0x00;                   // current age
    int field_0x04;                            // entry count
    int field_0x08;                            // capacity
    AgeEntry** field_0x0c;                     // entries
    int field_0x10;                            // accounted bytes
};

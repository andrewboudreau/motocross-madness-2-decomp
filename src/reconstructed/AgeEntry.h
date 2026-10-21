#pragma once

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

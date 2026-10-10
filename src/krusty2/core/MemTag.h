// MemTag.h -- the allocation-category tracker reached through the global pointer at
// 0x0056df04.  Callers bracket allocations with Push("Terrain"/"Collision"/...) and
// Pop(previous) (Terrain dtor 0x005079f0, SoultreePhysicsBaseObject slot 2,
// CollisionCharacter ctor 0x004318d0).  Tier 1: both are thiscall with one stack
// argument (ret 4) and Push's eax result is what the caller later passes to Pop.
// The class and method names are tier 3.
#ifndef MCM2_PHYSICS_COMMON_MEMTAG_H
#define MCM2_PHYSICS_COMMON_MEMTAG_H

class MemTagStack {
public:
    int Push(const char* tag);      // 0x004a2d00: returns the previous tag
    void Pop(int previous);         // 0x004a2d90
    int UnknownFunction4a2d20(const char* tag);  // 0x004a2d20 (ret 4): Terrain slot 14 prints it as "Memory %d"
};

extern MemTagStack* g_MemTagStack;  // 0x0056df04

#endif

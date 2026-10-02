#pragma once

// The allocation-category tracker reached through the global pointer at
// 0x0056df04 (see docs/ALLOCATION.md). Callers bracket allocations with
// Push(name) and Pop(previous). Names are provisional.
class MemTagStack {
public:
    int Push(const char* tag);      // 0x004a2d00: returns the previous category
    void Pop(int previous);         // 0x004a2d90
};

extern MemTagStack* g_MemTagStack;  // 0x0056df04

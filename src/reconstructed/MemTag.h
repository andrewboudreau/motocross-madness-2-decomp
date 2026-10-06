#pragma once

// The allocation-category tracker reached through the global pointer at
// 0x0056df04 (see docs/ALLOCATION.md). Callers bracket allocations with
// Push(name) and Pop(previous). Names are provisional.
class MemTagStack {
public:
    int Push(const char* tag);      // 0x004a2d00: returns the previous category
    void Pop(int previous);         // 0x004a2d90
    void UnknownFunction4a2da0(char* name);       // 0x004a2da0: copies the current category name
    void UnknownFunction4a2bc0(const char* tag);  // 0x004a2bc0 (TrackGame slot 14: "In Game")
    void UnknownFunction4a2de0(int bytes);        // 0x004a2de0: adds to the current "in DirectX" count
    void UnknownFunction4a2e00(int bytes);        // 0x004a2e00: subtracts from the current "in DirectX" count

    int count;                      // categories
    char (*names)[0x80];            // category names
    int* ours;                      // bytes allocated through the tracker
    int* directx;                   // Game slot 8 prints these as "in DirectX"
};

extern MemTagStack* g_MemTagStack;  // 0x0056df04

#pragma once

// RTTI: DirectoryList (root; CombinedDirectoryList derives from it; vtable
// 0x005516e0; 0x21c bytes, the size TrackGame slot 4 allocates). Its
// constructor 0x0044a130 writes the vtable. Names are provisional.
class DirectoryList {
public:
    DirectoryList();                                       // 0x0044a130
    virtual ~DirectoryList();
    virtual void UnknownVirtualSlot1();                    // scans
    void UnknownFunction44a1d0(const char* directory);     // 0x0044a1d0
    void UnknownFunction44a220(const char* pattern, int value); // 0x0044a220
    int UnknownFunction44a910(const char* name);           // 0x0044a910

    unsigned char field_0x04[0x21c - 4];
};

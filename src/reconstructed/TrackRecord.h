#pragma once

// TrackRecord.cpp (literal __FILE__ "D:\aardvark\VC\krusty2\TrackRecord.cpp",
// 0x0057536c). Names are provisional (docs/TRACKRECORD.md).

class DirectoryList;

// A high-score entry, 0x14 bytes: ten of them are read and written as one
// block. 0x0051ee90 is its constructor.
struct UnknownTrackRecord {
    UnknownTrackRecord();                     // 0x0051ee90

    char field_0x00[16];                      // racer name
    float field_0x10;                         // time or score
};

// qsort orders for the records: 0x0051eed0 puts the lower value first,
// 0x0051ef00 the higher one.
int UnknownFunction51eed0(const void* a, const void* b);
int UnknownFunction51ef00(const void* a, const void* b);

// The high-score table at TrackGame+0x3400 (0xe8 bytes, the size TrackGame
// slot 4 allocates); its constructor references TrackRecord.cpp's __FILE__.
class UnknownTrackGameObject3400 {
public:
    UnknownTrackGameObject3400();             // 0x0051ef30
    ~UnknownTrackGameObject3400();            // 0x0051efc0
    void UnknownFunction51efe0();             // 0x0051efe0: clears the table
    int UnknownFunction51f0b0(short directory, const char* name); // 0x0051f0b0
    int UnknownFunction51f110(const char* path);                  // 0x0051f110: reads
    int UnknownFunction51f1b0(const char* path, int value);       // 0x0051f1b0: writes
    int UnknownFunction51f260(short directory, const char* name, int value); // 0x0051f260
    void UnknownFunction51f2c0(int unused, const char* name);    // 0x0051f2c0
    int UnknownFunction51f3c0(short kind, int racer);             // 0x0051f3c0: adds a racer

    int field_0x00;                           // selects the file extension
    char field_0x04[3][5];                    // ".hs1", ".hs2", ".hs3"
    UnknownTrackRecord field_0x14[10];
    DirectoryList* field_0xdc;
    int field_0xe0;                           // record count
    int field_0xe4;                           // stored with the records
};

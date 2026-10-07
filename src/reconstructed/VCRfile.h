#pragma once

#include <stdio.h>

// Nonpolymorphic recording file object whose code is attributed by the retail
// D:\aardvark\VC\krusty2\VCRfile.cpp literal (0x00575868). bikerace.cpp code
// constructs it with (1, 2) and stores it at the view's +0x1a8 (0x00418fab),
// loads a recording into it (0x00418fda), and destroys and frees it at
// 0x0041d020; EventManager saves it with 0x00524d00. recorder.cpp code opens,
// reads, writes, seeks and closes handles through it. It has no vtable or
// RTTI, so the class name and every member name are provisional.
//
// Each handle is either an in-memory file (a chain of 0x19000-byte blocks)
// or a stdio FILE, chosen by the constructor's first argument.
struct UnknownVcrFileBlock {
    char data[0x19000];
    UnknownVcrFileBlock* next;          // +0x19000
};

struct UnknownVcrFileEntry {
    int field_0x00;                     // tested and cleared by 0x00525000
    char name[0x104];                   // +0x004
    char mode[8];                       // +0x108, fopen-style mode string
    int position;                       // +0x110
    int size;                           // +0x114
};

class UnknownVcrFile {
public:
    UnknownVcrFile(int memory, int count);                                  // 0x00524a50
    ~UnknownVcrFile();                                                      // 0x00524b10
    void Load(char* path, char* name);                     // 0x00524b80: loads `path`
    void Save(char* path);                                 // 0x00524d00: saves to `path`
    int Open(char* name, char* mode);                      // 0x00524dd0: open
    void Close(int handle);                                 // 0x00525000: close
    int Read(void* buffer, int size, int count, int handle);       // 0x00525070: read
    int Write(const void* buffer, int size, int count, int handle); // 0x005251f0: write
    int Seek(int handle, int offset, int origin);          // 0x005253d0: seek
    int Tell(int handle);                                  // 0x00525440: tell
    int IsEndOfFile(int handle);                                  // 0x005254a0: end of file

    int count;                          // +0x000, at most 2
    UnknownVcrFileEntry entries[2];     // +0x004
    UnknownVcrFileBlock* blocks[2];     // +0x234
    int memory;                         // +0x23c
    FILE* files[2];                     // +0x240
};

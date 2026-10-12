#pragma once

// Nonpolymorphic 0x101c-byte object whose code is attributed by the retail
// D:\aardvark\VC\krusty2\VCR.cpp literal (0x00575848). recorder.cpp code
// allocates it with operator new(0x101c) and constructs it at 0x004e7ad9,
// and destroys and frees it at 0x004e79b0. It has no vtable or RTTI, so the
// class name and every member name are provisional.
//
// The object is a ring of 256 16-byte buffer slots with three cursors, plus
// a chained list of 0x804-byte blocks of (file position, time) pairs. The
// member names describe how recorder.cpp's thread (samples/race/
// RecorderNearMisses.cpp) uses them: recording fills slots that the thread
// writes to the VCRtape file; playback has the thread read records from the
// file into slots that the game takes. Slot states: 0 free, 1 filled, 2 end
// of the file reached forward, 3 no earlier position (reverse playback).
struct UnknownVcrSlot {
    void* data;                 // +0x00, DebugMalloc'd buffer
    unsigned int size;          // +0x04, capacity recorded for data
    int filePosition;           // +0x08: file offset of a record read back (-1: marker)
    char inUse;                 // +0x0c: set to 1 while the slot is handed out
    char state;                 // +0x0d: 0 free, 1 filled, 2/3 end markers
};

struct UnknownVcrBlock {
    int pairs[256][2];          // +0x000: (file position, time in ms); -1 positions are markers
    UnknownVcrBlock* next;      // +0x800
};

class UnknownVcr {
public:
    UnknownVcr(unsigned int size);                                           // 0x00524110
    ~UnknownVcr();                                                           // 0x005241b0
    int ClaimRecordSlot(unsigned int size, int* index, void** data);        // 0x00524220: next fill slot, behind a size header
    void CommitRecordSlot(int index);                                        // 0x005242e0: state 1
    void TakeSlotToWrite(void** data, int* size, int* index);                // 0x00524300: advances the write cursor
    int HasSlotToWrite();                                                    // 0x00524350
    int ClaimReadSlot(unsigned int size, int* index, void** data);          // 0x00524390: next fill slot, no header
    void CommitReadSlot(int position, int index);                            // 0x00524440: state 1 with the file position
    int TakePlaybackSlot(void** data, unsigned int* size, int* position, int* index); // 0x00524460: 0, or 1/2 at an end, 4 when empty
    void ReleaseSlot(int index);                                             // 0x00524540: state 0
    int HasRoomToRead();                                                     // 0x00524560
    void AppendPosition(int position, int timeMs);                           // 0x00524590
    void ClearPositions();                                                   // 0x00524640
    int StepPositionBack(int* timeMs);                                       // 0x005246c0
    int PeekPosition(int* timeMs);                                           // 0x005247c0: skips one -1 marker
    void SeekPosition(int position);                                         // 0x00524870
    void SetFirstPosition(int position);                                     // 0x005248f0: offset after the file header
    int GetFirstPosition();                                                  // 0x00524900
    void ResetSlots();                                                       // 0x00524910
    void MarkForwardEnd();                                                   // 0x00524940: state 2 on the next fill slot
    void MarkReverseEnd();                                                   // 0x00524970: state 3 on the next fill slot
    int AdvanceWriteCursor();                                                // 0x005249a0
    int AdvancePlayCursor();                                                 // 0x005249d0
    int AdvanceFillCursor();                                                 // 0x00524a00
    int NextSlot(int index);                                                 // 0x00524a30: ring successor

    UnknownVcrSlot slots[256];          // +0x0000
    int writeCursor;                    // +0x1000: last slot handed to the file writer
    int playCursor;                     // +0x1004: last slot handed to playback
    int fillCursor;                     // +0x1008: last slot claimed for filling
    UnknownVcrBlock* positionBlocks;    // +0x100c: first pair block
    int positionIndex;                  // +0x1010: pair index in the current block
    int positionBlock;                  // +0x1014: current block number
    int firstPosition;                  // +0x1018
};

#pragma once

#include "KrustyVCRHeader.h"
#include "Recorder.h"

class VCRFile;

// KrustyVCR.cpp's class (bikerace.cpp keeps a replay and a ghost recorder;
// InGameProcs.cpp reads the replay's recorded length).

// RTTI: KrustyVCR : VCRInterface (vtable 0x00550e9c, 0xf8c bytes). Its
// constructor is inline at bikerace.cpp's `new` sites (lines 0x2b9, 0x10e7,
// 0x1103 and others), which also emit its implicit destructor 0x00419960
// and the deleting wrapper 0x00419940. BikeRace keeps the replay being
// recorded or played at +0x1a0 and the ghost at +0x1a4. Its methods sit
// after krustyui.cpp's code; only the members used here are declared.
class KrustyVCR : public VCRInterface {
public:
    KrustyVCR() {}

    // 0x0049bf10: opens `name` in `file` through `callback` for recording
    // (mode 0) or playback (mode 1); 0 on failure.
    int OpenRecording(RecorderCallbackProc callback, int mode, char* name,
                              VCRFile* file);
    float RecordedLength();            // 0x0049c000: the recorded length
    void SetReplayLength(float time, int flag); // 0x0049bff0
    // 0x0049c010: saves the recording with its length and description.
    void SaveRecording(float length, char* description, int flag);
    // 0x0049c070: describes a racer of the recording: its network id, AI
    // index (a char), record slot, name, model files, bike and rider names,
    // engine size, four-stroke flag and plate number. GetRacer (0x0049c1e0) copies
    // record `index`'s fields back out.
    void SetRacer(int id, char ai, int slot, char* name, char* model,
                               char* rider, char* bikeName, char* riderName, int engineSize,
                               int fourStroke, int value);
    void GetRacer(int* id, char* ai, int index, char* name, char* model, char* rider,
                               char* bikeName, char* riderName, int* engineSize, int* fourStroke,
                               int* value);

    float replayLength;                       // +0x0d8 SetReplayLength (BikeRace slot 10 each frame)
    // +0xdc..+0xf8c: the header 0x0049bf10 and 0x0049c010 hand to the
    // recorder as one 0xeb0-byte block.
    KrustyVCRHeader header;                   // +0x0dc
};

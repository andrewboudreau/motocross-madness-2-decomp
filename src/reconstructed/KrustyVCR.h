#pragma once

#include "Recorder.h"

class VCRFile;

// KrustyVCR.cpp's class (bikerace.cpp keeps a replay and a ghost recorder;
// InGameProcs.cpp reads the replay's recorded length).

// One of KrustyVCR's eight 8-byte records at +0x25c (cleared by its inline
// constructor).
struct KrustyVcrGridEntry {
    KrustyVcrGridEntry() {
        networkPlayerId = 0;
        points = 0;
    }
    int networkPlayerId;  // +0x0  same 8-byte grid entry as TrackGame.h GridEntry (+0 compared with racers' networkPlayerId)
    char points;  // +0x4  same grid entry: TrackGame.h GridEntry::points (championship points)
};

// One racer of a recording (KrustyVCR+0x300, 0x124 bytes; 0x0049c070 writes
// one and 0x0049c1e0 reads it back). The strings are copied with their
// lengths clamped to the array size less one.
struct KrustyVcrRacerRecord {
    int networkId;                            // +0x000 network id
    char aiIndex;                             // +0x004 AI index
    char name[0x10];                          // +0x005 name
    char bikeFile[0x40];  // +0x15  SetRacer model argument (Res\Ghost.mcf); the loader reads it back into bikeFile
    char riderFile[0x40];  // +0x55  SetRacer rider argument (Res\GhostRider.mcf); the loader reads it back into riderFile
    char bikeName[0x40];  // +0x95  bike name; the loader passes it back to KrustyUI::ResolveBikeFile
    char riderName[0x40];  // +0xD5  rider name; the loader passes it back to KrustyUI::ResolveRiderFile
    int engineSize;                           // +0x118 engine size
    int fourStroke;                           // +0x11c four-stroke flag (KrustyUI "BikeFourStroke_%02d")
    int plateNumber;  // +0x120  SetRacer's last argument is TrackGameMode plateNumber / slots[j].plateNumber; read back into plate
};

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
    // +0xdc..+0xf8c (0xeb0 bytes) is the block 0x0049bf10 and 0x0049c010 hand
    // to the recorder.
    char magic[8];                            // +0x0dc "MCMVCR" (0x0049bf10, mode 0)
    char version[8];                          // +0x0e4 version "2.0" plus 0x1a (the string at 0x0056da64)
    char description[0x20];                   // +0x0ec description (0x0049c010)
    float recordedLength;                     // +0x10c recorded length: SaveRecording 0x0049c010 stores it, RecordedLength 0x0049c000 returns it
    // +0x110 TrackGame+0x2d70's 0x1ec bytes, the race settings from
    // raceMode on (copied by 0x0049bf10, mode 0).
    unsigned char raceSettings[0x25c - 0x110];
    KrustyVcrGridEntry gridEntries[8];  // +0x25C  raceSettings+0x14c (TrackGame+0x2ebc) copied with the settings: per grid slot player id (TrackGame.h GridEntry)
    unsigned char field_0x29c[0x2fc - 0x29c];
    int racerCount;                           // +0x2fc racers recorded (the loader)
    KrustyVcrRacerRecord racers[11];         // +0x300 0x0049c070 / 0x0049c1e0
};

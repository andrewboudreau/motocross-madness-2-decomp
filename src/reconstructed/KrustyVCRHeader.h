#pragma once

#include "RaceSettings.h"

// One racer of a recording (KrustyVCRHeader+0x224, 0x124 bytes; 0x0049c070 writes
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

// The 0xeb0-byte header of a ghost or replay file. KrustyVCR keeps the one it
// records at +0xdc and hands it whole to the recorder (0x0049bf10,
// 0x0049c010); GhostFilesDlg and ReplayFilesDlg read the files' headers
// (0x00454640), SPRaceInfoDlg reads VCRtape.dat's (0x004507a0), and KrustyUI
// keeps the header of the ghost raced against at +0x4b4.
struct KrustyVCRHeader {
    char magic[8];                            // +0x000 "MCMVCR" (0x0049bf10, mode 0)
    char version[8];                          // +0x008 "2.0" plus 0x1a (the string at 0x0056da64)
    char description[0x20];                   // +0x010 description (SaveRecording 0x0049c010)
    float lengthSeconds;                      // +0x030 SaveRecording 0x0049c010 stores it, RecordedLength 0x0049c000 returns it; the menus show it as a time
    RaceSettings raceSettings;                // +0x034 TrackGame+0x2d70's settings (copied by 0x0049bf10, mode 0); a ghost race copies them back
    int racerCount;                           // +0x220 racers recorded (the loader)
    KrustyVcrRacerRecord racers[11];          // +0x224 SetRacer 0x0049c070 / GetRacer 0x0049c1e0; the menus show racers[0]'s name, engine size and four-stroke flag
};

// KrustyVCR.cpp -- KrustyVCR's out-of-line methods 0x0049bf10..0x0049c2e9
// (RTTI KrustyVCR : VCRInterface, vtable 0x00550e9c; class in KrustyVCR.h).
// The code sits after krustyui.cpp's and has no __FILE__ literal: the file
// name is ours (tier 3), as are the member names. bikerace.cpp calls these
// methods on its replay and ghost recorders.

#include <string.h>

#include "KrustyVCR.h"
#include "BikeRace.h"
#include "TrackGame.h"

// Copies at most sizeof(destination) - 1 characters of `source` into the
// character array `destination` and terminates the copy. Retail addresses the
// array afresh for the terminator (no shared destination pointer), which an
// inline helper function does not reproduce.
#define COPY_TRUNCATED(destination, source)                                   \
    {                                                                         \
        int length = strlen(source);                                          \
        int count = length > (int)sizeof(destination) - 1                     \
                        ? (int)sizeof(destination) - 1                        \
                        : length;                                             \
        strncpy(destination, source, count);                                  \
        destination[count] = 0;                                               \
    }

// 0x0049bf10: starts the recorder on `file` (mode 0 records, 1 plays back).
// A new recording first stamps the "MCMVCR" signature, the version string and
// the current race settings into the block it hands over.
int KrustyVCR::OpenRecording(RecorderCallbackProc callback, int mode, char* name,
                                     VCRFile* file)
{
    replayLength = 0;
    if (mode == 0) {
        COPY_TRUNCATED(header.magic, "MCMVCR")
        COPY_TRUNCATED(header.version, "2.0\x1a")
        header.lengthSeconds = 0;
        memcpy(&header.raceSettings, &g_TrackGame->mode.raceSettings, sizeof(RaceSettings));
    }
    return Start((int)callback, (int)&header, sizeof(header), mode, (int)name, 0x58,
                 file);
}

// 0x0049bff0
void KrustyVCR::SetReplayLength(float time, int flag)
{
    replayLength = time;
}

// 0x0049c000
float KrustyVCR::RecordedLength()
{
    return header.lengthSeconds;
}

// 0x0049c010
void KrustyVCR::SaveRecording(float duration, char* description, int flag)
{
    COPY_TRUNCATED(header.description, description)
    header.lengthSeconds = duration;
    CloseFile((int)&header, sizeof(header), flag);
}

// 0x0049c070
void KrustyVCR::SetRacer(int id, char ai, int slot, char* name, char* model,
                                      char* rider, char* bikeName, char* riderName,
                                      int engineSize, int fourStroke, int value)
{
    header.racers[slot].networkId = id;
    header.racers[slot].aiIndex = ai;
    COPY_TRUNCATED(header.racers[slot].name, name)
    COPY_TRUNCATED(header.racers[slot].bikeFile, model)
    COPY_TRUNCATED(header.racers[slot].riderFile, rider)
    COPY_TRUNCATED(header.racers[slot].bikeName, bikeName)
    COPY_TRUNCATED(header.racers[slot].riderName, riderName)
    header.racers[slot].engineSize = engineSize;
    header.racers[slot].fourStroke = fourStroke;
    header.racers[slot].plateNumber = value;
}

// 0x0049c1e0
void KrustyVCR::GetRacer(int* id, char* ai, int index, char* name, char* model,
                                      char* rider, char* bikeName, char* riderName,
                                      int* engineSize, int* fourStroke, int* value)
{
    *id = header.racers[index].networkId;
    *ai = header.racers[index].aiIndex;
    strcpy(name, header.racers[index].name);
    strcpy(model, header.racers[index].bikeFile);
    strcpy(rider, header.racers[index].riderFile);
    strcpy(bikeName, header.racers[index].bikeName);
    strcpy(riderName, header.racers[index].riderName);
    *engineSize = header.racers[index].engineSize;
    *fourStroke = header.racers[index].fourStroke;
    *value = header.racers[index].plateNumber;
}

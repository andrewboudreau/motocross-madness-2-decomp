// KrustyVCR.cpp -- KrustyVCR's out-of-line methods 0x0049bf10..0x0049c2e9
// (RTTI KrustyVCR : VCRInterface, vtable 0x00550e9c; class in BikeRace.h).
// The code sits after krustyui.cpp's and has no __FILE__ literal: the file
// name is ours (tier 3), as are the member names. bikerace.cpp calls these
// methods on its replay and ghost recorders.

#include <string.h>

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
int KrustyVCR::OpenRecording(UnknownRecorderCallback callback, int mode, char* name,
                                     UnknownVcrFile* file)
{
    replayLength = 0;
    if (mode == 0) {
        COPY_TRUNCATED(magic, "MCMVCR")
        COPY_TRUNCATED(version, "2.0\x1a")
        timePlayed = 0;
        memcpy(raceSettings, &g_TrackGame->mode.raceSettings.raceMode, 0x1ec);
    }
    return Start((int)callback, (int)magic, 0xeb0, mode, (int)name, 0x58,
                 (UnknownRecorderOwner*)file);
}

// 0x0049bff0
void KrustyVCR::SetReplayLength(float time, int flag)
{
    replayLength = time;
}

// 0x0049c000
float KrustyVCR::RecordedLength()
{
    return timePlayed;
}

// 0x0049c010
void KrustyVCR::SaveRecording(float duration, char* description, int flag)
{
    COPY_TRUNCATED(this->description, description)
    timePlayed = duration;
    CloseFile((int)magic, 0xeb0, flag);
}

// 0x0049c070
void KrustyVCR::SetRacer(int id, char ai, int slot, char* name, char* model,
                                      char* rider, char* bikeName, char* riderName,
                                      int engineSize, int fourStroke, int value)
{
    racers[slot].networkId = id;
    racers[slot].aiIndex = ai;
    COPY_TRUNCATED(racers[slot].name, name)
    COPY_TRUNCATED(racers[slot].bikeFile, model)
    COPY_TRUNCATED(racers[slot].riderFile, rider)
    COPY_TRUNCATED(racers[slot].bikeName, bikeName)
    COPY_TRUNCATED(racers[slot].riderName, riderName)
    racers[slot].engineSize = engineSize;
    racers[slot].fourStroke = fourStroke;
    racers[slot].plateNumber = value;
}

// 0x0049c1e0
void KrustyVCR::GetRacer(int* id, char* ai, int index, char* name, char* model,
                                      char* rider, char* bikeName, char* riderName,
                                      int* engineSize, int* fourStroke, int* value)
{
    *id = racers[index].networkId;
    *ai = racers[index].aiIndex;
    strcpy(name, racers[index].name);
    strcpy(model, racers[index].bikeFile);
    strcpy(rider, racers[index].riderFile);
    strcpy(bikeName, racers[index].bikeName);
    strcpy(riderName, racers[index].riderName);
    *engineSize = racers[index].engineSize;
    *fourStroke = racers[index].fourStroke;
    *value = racers[index].plateNumber;
}

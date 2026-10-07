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
int KrustyVCR::UnknownFunction49bf10(UnknownRecorderCallback callback, int mode, char* name,
                                     UnknownVcrFile* file)
{
    field_0x0d8 = 0;
    if (mode == 0) {
        COPY_TRUNCATED(field_0x0dc, "MCMVCR")
        COPY_TRUNCATED(field_0x0e4, "2.0\x1a")
        field_0x10c = 0;
        memcpy(field_0x110, &g_TrackGame->mode.field_0x27f8.field_0x00, 0x1ec);
    }
    return Start((int)callback, (int)field_0x0dc, 0xeb0, mode, (int)name, 0x58,
                 (UnknownRecorderOwner*)file);
}

// 0x0049bff0
void KrustyVCR::UnknownFunction49bff0(float time, int flag)
{
    field_0x0d8 = time;
}

// 0x0049c000
float KrustyVCR::UnknownFunction49c000()
{
    return field_0x10c;
}

// 0x0049c010
void KrustyVCR::UnknownFunction49c010(float duration, char* description, int flag)
{
    COPY_TRUNCATED(field_0x0ec, description)
    field_0x10c = duration;
    UnknownFunction4e86d0((int)field_0x0dc, 0xeb0, flag);
}

// 0x0049c070
void KrustyVCR::UnknownFunction49c070(int id, char ai, int slot, char* name, char* model,
                                      char* rider, char* riderName, char* bikeName,
                                      int engineSize, int engineKind, int value)
{
    field_0x300[slot].field_0x000 = id;
    field_0x300[slot].field_0x004 = ai;
    COPY_TRUNCATED(field_0x300[slot].field_0x005, name)
    COPY_TRUNCATED(field_0x300[slot].field_0x015, model)
    COPY_TRUNCATED(field_0x300[slot].field_0x055, rider)
    COPY_TRUNCATED(field_0x300[slot].field_0x095, riderName)
    COPY_TRUNCATED(field_0x300[slot].field_0x0d5, bikeName)
    field_0x300[slot].field_0x118 = engineSize;
    field_0x300[slot].field_0x11c = engineKind;
    field_0x300[slot].field_0x120 = value;
}

// 0x0049c1e0
void KrustyVCR::UnknownFunction49c1e0(int* slot, char* ai, int a, char* name, char* model,
                                      char* rider, char* bikeName, char* riderName,
                                      int* engineSize, int* engineKind, int* value)
{
    *slot = field_0x300[a].field_0x000;
    *ai = field_0x300[a].field_0x004;
    strcpy(name, field_0x300[a].field_0x005);
    strcpy(model, field_0x300[a].field_0x015);
    strcpy(rider, field_0x300[a].field_0x055);
    strcpy(bikeName, field_0x300[a].field_0x095);
    strcpy(riderName, field_0x300[a].field_0x0d5);
    *engineSize = field_0x300[a].field_0x118;
    *engineKind = field_0x300[a].field_0x11c;
    *value = field_0x300[a].field_0x120;
}

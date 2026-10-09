// Near miss of the unit 0x0049c2f0..0x0049dd2f (canonical file:
// src/reconstructed/ZoneReport.cpp, whose other three
// functions are strict exact).
//
// 0x0049ca60 (4810 bytes, 4792 match): the track table is six separate
// arrays initialised in the order quarry, baja, national, supercross, tag,
// stunt (the order the string pool 0x0056dc04..0x0056dda4 records, in
// reverse); VC6's frame rule then gives retail's layout (quarry, baja,
// supercross, tag, stunt, national, lists). Direction 2 adds the 1 of the
// 1-based type after the join: written inside the else branch
// (`index - first + 1`, any spelling) VC6 computes `index + (1 - first)`
// with the register that holds the table's constant 1, runs out of
// registers and spills `index` (frame 0x5ac). What still differs: retail
// folds the 1 into the >= 30 branch (`add ebx, -0x1a`, jumping past the
// `inc ebx` of the else branch); VC6 here subtracts 27 and increments after
// the join. In-branch -26/+1 forms (`+= -26`, `- 27 + 1`, `++` first or
// last), `++index`/`1 + index` joins, stores in both branches (worse) and
// moving `first = 0` into the else branch (worse) were tried; a
// `(short)(index - first) + 1` cast also avoids the spill but adds a movsx.
// Check: compile this file and compare ?UnknownFunction49ca60@... with
// samples/net/ZoneReportNearMisses.bindings.json.

#include <string.h>

#include "../../src/reconstructed/TrackGame.h"

// 0x0056db00 (in the unit's .data): bike types per manufacturer.
static int kBikeTypeCounts[7] = {3, 3, 5, 4, 5, 5, 5};

// One track of an event type: its name and variant (0x14 bytes).
struct UnknownTrackNameEntry {
    char name[16];
    int variant;
};

struct UnknownTrackNameList {
    int count;
    UnknownTrackNameEntry* entries;
};

// 0x0049ca60: converts between the lobby's numbering and the game's.
// Direction 1: bike manufacturer/type to bike index (+0x2a0) and event
// type/location to track name and variant; direction 2: the reverse, and
// +0x2a5 gets the track's entry number (1-based; 0 when not found).
void UnknownTrackGameObject3410::UnknownFunction49ca60(int direction)
{
    UnknownTrackNameEntry quarry[5] = {
        {"Quarry05", 0}, {"Quarry01", 0}, {"Quarry04", 0}, {"Quarry03", 0}, {"Quarry02", 0},
    };
    UnknownTrackNameEntry baja[15] = {
        {"baja01", 1}, {"baja01", 2}, {"baja01", 3}, {"baja03", 1}, {"baja03", 2},
        {"baja03", 3}, {"baja02", 1}, {"baja02", 2}, {"baja02", 3}, {"baja04", 1},
        {"baja04", 2}, {"baja04", 3}, {"baja05", 1}, {"baja05", 2}, {"baja05", 3},
    };
    UnknownTrackNameEntry national[16] = {
        {"Nat12", 0}, {"Nat01", 0}, {"Nat05", 0}, {"Nat08", 0}, {"Nat10", 0}, {"Nat03", 0},
        {"Nat09", 0}, {"iffendic", 0}, {"hillside", 0}, {"Nat11", 0}, {"Nat07", 0},
        {"Nat06", 0}, {"SkyLab2", 0}, {"Nat02", 0}, {"VV", 0}, {"Nat04", 0},
    };
    UnknownTrackNameEntry supercross[16] = {
        {"SX01", 0}, {"SX02", 0}, {"SX03", 0}, {"SX04", 0}, {"SX05", 0}, {"SX06", 0},
        {"SX07", 0}, {"SX08", 0}, {"SX09", 0}, {"SX10", 0}, {"SX11", 0}, {"SX12", 0},
        {"SX13", 0}, {"SX14", 0}, {"SX15", 0}, {"SX16", 0},
    };
    UnknownTrackNameEntry tag[3] = {
        {"Tag01", 0}, {"Tag02", 0}, {"Tag03", 0},
    };
    UnknownTrackNameEntry stunt[15] = {
        {"SkiLodge", 1}, {"SkiLodge", 2}, {"SkiLodge", 3}, {"OpenPit", 1}, {"OpenPit", 2},
        {"OpenPit", 3}, {"Farm", 1}, {"Farm", 2}, {"Farm", 3}, {"Trailer", 1},
        {"Trailer", 2}, {"Trailer", 3}, {"Airfield", 1}, {"Airfield", 2}, {"Airfield", 3},
    };
    UnknownTrackNameList lists[6] = {
        {5, quarry}, {15, baja}, {16, national}, {16, supercross}, {3, tag}, {15, stunt},
    };
    int i;
    int index;
    int first;
    UnknownTrackNameEntry* entry;

    switch (direction) {
    case 1:
        if (field_0x298 >= 0 && field_0x298 <= 4) {
            first = 0;
            if (field_0x298 == 4)
                field_0x298 = 6;
            if (field_0x298 == 0 && field_0x29c >= 3) {
                field_0x2a0 = field_0x29c + 27;
            } else {
                for (i = 0; i < field_0x298; i++)
                    first += kBikeTypeCounts[i];
                if (i < 7)
                    field_0x2a0 = field_0x29c + first;
                else
                    field_0x2a0 = -1;
            }
        } else {
            field_0x2a0 = -1;
        }
        if (field_0x24c >= 0 && field_0x24c <= 5) {
            entry = &lists[field_0x24c].entries[field_0x250];
            field_0x254 = entry->variant;
            strcpy(field_0x258, entry->name);
        } else {
            field_0x24c = -1;
        }
        break;
    case 2:
        index = field_0x2a0;
        first = 0;
        if (index >= 30) {
            field_0x298 = 1;
            index -= 27;
        } else {
            for (i = 0; i < 7; i++) {
                if (index < first + kBikeTypeCounts[i])
                    break;
                first += kBikeTypeCounts[i];
            }
            field_0x298 = i + 1;
            if (field_0x298 == 7)
                field_0x298 = 5;
            index -= first;
        }
        field_0x29c = index + 1;
        field_0x2a4.field_0x01 = 0;
        entry = lists[field_0x24c].entries;
        for (i = 0; i < 16; i++, entry++) {
            if (!_stricmp(entry->name, field_0x258) && field_0x254 == entry->variant) {
                field_0x2a4.field_0x01 = i + 1;
                return;
            }
        }
        break;
    }
}

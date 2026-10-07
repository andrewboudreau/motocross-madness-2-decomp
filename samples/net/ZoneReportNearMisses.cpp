// Near miss of the unit 0x0049c2f0..0x0049dd2f (canonical file:
// src/reconstructed/ZoneReport.cpp, whose other three
// functions are strict exact).
//
// 0x0049ca60 (4810 bytes): the 70-entry track table is one local array
// (one array gives retail's frame layout: quarry, baja, supercross, tag,
// stunt, national, then the six {count, entries} lists; six separate arrays
// are laid out in another order whatever their declaration order or names).
// The conversion logic matches instruction for instruction (_stricmp, the
// 1/2 direction switch, the bike-type loops), but VC6 SP3 spills `index`
// of direction 2 to a stack slot (frame 0x5ac, retail 0x5a8, which keeps it
// in ebx and the loop counter in esi), which shifts every table store by 4,
// and the table stores are scheduled differently from about the 190th
// instruction on. Variants tried (separate loop counters, direct field
// arithmetic, a `next` temporary, indexed search, scoped index) keep a spill.
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
    UnknownTrackNameEntry tracks[70] = {
        // quarry (event type 0)
        {"Quarry05", 0}, {"Quarry01", 0}, {"Quarry04", 0}, {"Quarry03", 0}, {"Quarry02", 0},
        // baja (1)
        {"baja01", 1}, {"baja01", 2}, {"baja01", 3}, {"baja03", 1}, {"baja03", 2},
        {"baja03", 3}, {"baja02", 1}, {"baja02", 2}, {"baja02", 3}, {"baja04", 1},
        {"baja04", 2}, {"baja04", 3}, {"baja05", 1}, {"baja05", 2}, {"baja05", 3},
        // supercross (3)
        {"SX01", 0}, {"SX02", 0}, {"SX03", 0}, {"SX04", 0}, {"SX05", 0}, {"SX06", 0},
        {"SX07", 0}, {"SX08", 0}, {"SX09", 0}, {"SX10", 0}, {"SX11", 0}, {"SX12", 0},
        {"SX13", 0}, {"SX14", 0}, {"SX15", 0}, {"SX16", 0},
        // tag (4)
        {"Tag01", 0}, {"Tag02", 0}, {"Tag03", 0},
        // stunt (5)
        {"SkiLodge", 1}, {"SkiLodge", 2}, {"SkiLodge", 3}, {"OpenPit", 1}, {"OpenPit", 2},
        {"OpenPit", 3}, {"Farm", 1}, {"Farm", 2}, {"Farm", 3}, {"Trailer", 1},
        {"Trailer", 2}, {"Trailer", 3}, {"Airfield", 1}, {"Airfield", 2}, {"Airfield", 3},
        // national (2)
        {"Nat12", 0}, {"Nat01", 0}, {"Nat05", 0}, {"Nat08", 0}, {"Nat10", 0}, {"Nat03", 0},
        {"Nat09", 0}, {"iffendic", 0}, {"hillside", 0}, {"Nat11", 0}, {"Nat07", 0},
        {"Nat06", 0}, {"SkyLab2", 0}, {"Nat02", 0}, {"VV", 0}, {"Nat04", 0},
    };
    UnknownTrackNameList lists[6] = {
        {5, &tracks[0]}, {15, &tracks[5]}, {16, &tracks[54]}, {16, &tracks[20]}, {3, &tracks[36]}, {15, &tracks[39]},
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
            index -= 26;
        } else {
            for (i = 0; i < 7; i++) {
                if (index < first + kBikeTypeCounts[i])
                    break;
                first += kBikeTypeCounts[i];
            }
            field_0x298 = i + 1;
            if (field_0x298 == 7)
                field_0x298 = 5;
            index = index - first + 1;
        }
        field_0x29c = index;
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

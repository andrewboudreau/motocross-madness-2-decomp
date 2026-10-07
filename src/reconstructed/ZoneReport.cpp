// The Zone lobby reporting methods of the TrackGame+0x3410 object
// (0x0049c2f0..0x0049dd30). No __FILE__ literal or RTTI reaches this range,
// so the file name is ours (tier 3). The range lies between KrustyVCR and
// LightEmitter.cpp. It is separate from MSZoneInterface.cpp
// (0x004aa350..), which holds the COM calls these methods use.
// 0x0049ca60 (4810 bytes) is not reconstructed yet. It builds local tables
// of track names ("Quarry01"..) and maps the lobby's indices.

#include <stdio.h>
#include <string.h>
#include "TrackGame.h"
#include "Parser.h"

unsigned int UnknownFunction4bfa80();   // 0x004bfa80: milliseconds

// 0x0049c2f0: reads the lobby preset (0x004aa360) and applies it to the
// race settings. `*flags` gets a bit per tag found (1 event type, 2
// location, 4 manufacturer, 8 bike type), cleared again when 0x0049ca60
// cannot map them. The parser's implicit destructor is emitted out of line
// at 0x0049c5f0 for the unwind funclets; the normal paths inline it.
// Writing the body inside `if (...) { ... return 1; } return 0;` gives
// retail's explicit `xor eax, eax` on the failure path.
int UnknownTrackGameObject3410::UnknownFunction49c2f0(int* flags)
{
    char response[0x800];
    char name[0x40];
    unsigned long value;
    *flags = 0;
    if (UnknownFunction4aa360(response, 0x800)) {
        UnknownParser parser;
        parser.UnknownFunction4b87f0(response, lstrlen(response));
        if (parser.UnknownFunction4b89d0("EventTypeIndex", &value) != 0x80070005) {
            field_0x24c = value - 1;
            *flags |= 1;
        } else {
            field_0x24c = -1;
        }
        if (parser.UnknownFunction4b89d0("EventTypeLocation", &value) != 0x80070005) {
            field_0x250 = value - 1;
            *flags |= 2;
        } else {
            field_0x250 = -1;
        }
        if (parser.UnknownFunction4b89d0("BikeManufacturer", &value) != 0x80070005) {
            field_0x298 = value - 1;
            *flags |= 4;
        } else {
            field_0x298 = -1;
        }
        if (parser.UnknownFunction4b89d0("BikeType", &value) != 0x80070005) {
            field_0x29c = value - 1;
            *flags |= 8;
        } else {
            field_0x29c = -1;
        }
        UnknownFunction49ca60(1);
        if (field_0x2a0 == -1) {
            *flags &= ~0xc;
        } else {
            g_TrackGame->mode.field_0x1974.field_0xc0 = field_0x298;
            g_TrackGame->mode.field_0x1974.field_0xc4 = field_0x2a0;
        }
        if (field_0x24c == -1)
            *flags &= ~3;
        else
            g_TrackGame->mode.field_0x27f8.field_0x04 = field_0x24c;
        g_TrackGame->mode.field_0x27f8.field_0x34 = field_0x254;
        if (field_0x250 != -1)
            strcpy(g_TrackGame->mode.field_0x27f8.field_0x36, field_0x258);
        if (g_TrackGame->network->isHost)
            g_TrackGame->sceneObject->UnknownFunction4ea010(name, g_TrackGame->mode.field_0x27f8.field_0x36, 0, "scn",
                                                             &g_TrackGame->mode.field_0x27f8.field_0x138,
                                                             &g_TrackGame->mode.field_0x27f8.field_0x13c);
        g_TrackGame->mode.field_0x27f8.field_0x10 = 1;
        g_TrackGame->mode.field_0x27f8.field_0x14 = 1;
        g_TrackGame->mode.field_0x27f8.field_0x24 = 0;
        g_TrackGame->mode.field_0x27f8.field_0x00 = 1;
        g_TrackGame->mode.field_0x27f8.field_0x0c = 0;
        g_TrackGame->mode.field_0x27f8.field_0x18 = 0;
        g_TrackGame->mode.field_0x27f8.field_0x20 = 5;
        g_TrackGame->mode.field_0x27f8.field_0x140 = 5.0f;
        g_TrackGame->mode.field_0x27f8.field_0x148 = 0;
        g_TrackGame->mode.field_0x27f8.field_0x2d = 0;
        g_TrackGame->mode.field_0x27f8.field_0x144 = 0;
        g_TrackGame->mode.field_0x98 = 3;
        g_TrackGame->mode.field_0x94 = 3;
        return 1;
    }
    return 0;
}

// 0x0049c600: polls the lobby's rank property (0x004aa4e0) for up to 20
// seconds. The value keyed by the player's name is "%d,%d,%d,%d"; the first
// number is stored in TrackGameMode+0x1bd0. -1 means not ready yet; it
// returns 0 on failure, -1 or -2. The `else return 0` inside the loop keeps
// one shared epilogue; an early `if (!...) return 0;` duplicates it.
int UnknownTrackGameObject3410::UnknownFunction49c600()
{
    int value;
    int b;
    int c;
    int d;
    char response[0x800];
    char text[0x800];
    unsigned int start = UnknownFunction4bfa80();
    while (UnknownFunction4bfa80() - start < 20000) {
        if (UnknownFunction4aa4e0(response, 0x800)) {
            UnknownParser parser;
            parser.UnknownFunction4b87f0(response, lstrlen(response));
            parser.UnknownFunction4b8990(g_TrackGame->mode.field_0x00, text, 0x800);
            sscanf(text, "%d,%d,%d,%d", &value, &b, &c, &d);
            if (value == -1) {
                Sleep(1000);
            } else {
                g_TrackGame->mode.field_0x1bd0 = value;
                break;
            }
        } else {
            return 0;
        }
    }
    if (value == -1)
        return 0;
    return value != -2;
}

// 0x0049c770 (EventManager 0x0045e550): fills the race status from the
// event entries and sends it to the Zone (0x004aa670). Returns 1 without
// a lobby connection. Indexing field_0x2a4.field_0x08[i] directly (not
// through a record pointer) gives retail's induction register.
int UnknownTrackGameObject3410::UnknownFunction49c770()
{
    int longest;
    int i;
    int j;
    if (g_TrackGame->network->lobbyConnected) {
        field_0x2a4.field_0x00 = g_TrackGame->mode.field_0x27f8.field_0x04 + 1;
        longest = 0;
        i = 0;
        field_0x24c = g_TrackGame->mode.field_0x27f8.field_0x04;
        strcpy(field_0x258, g_TrackGame->mode.field_0x27f8.field_0x36);
        field_0x254 = g_TrackGame->mode.field_0x27f8.field_0x34;
        field_0x2a4.field_0x02 = g_TrackGame->mode.field_0x27f8.field_0x20;
        for (; i < g_TrackGame->field_0x18; i++) {
            field_0x2a4.field_0x08[i].field_0x02 = g_TrackGame->eventManager->field_0x50[i].field_0x04;
            field_0x2a4.field_0x08[i].field_0x04 = (int)(g_TrackGame->eventManager->field_0x50[i].field_0x08 * 100.0f);
            field_0x2a4.field_0x08[i].field_0x08 = (int)(g_TrackGame->eventManager->field_0x50[i].field_0x14 * 100.0f);
            if (longest < field_0x2a4.field_0x08[i].field_0x08)
                longest = field_0x2a4.field_0x08[i].field_0x08;
            field_0x2a4.field_0x08[i].field_0x0c = g_TrackGame->eventManager->field_0x50[i].field_0x0c;
            field_0x08[i].field_0x40 = field_0x2a4.field_0x08[i].field_0x02;
            strcpy(field_0x08[i].field_0x00, g_TrackGame->eventManager->field_0x50[i].field_0x40);
            if (g_TrackGame->eventManager->field_0x50[i].field_0x20) {
                if (g_TrackGame->mode.field_0x27f8.field_0x04 != 0 && g_TrackGame->mode.field_0x27f8.field_0x04 != 4)
                    field_0x08[i].field_0x44 = (int)(g_TrackGame->eventManager->field_0x50[i].field_0x08 * 100.0f);
                else
                    field_0x08[i].field_0x44 = (int)g_TrackGame->eventManager->field_0x50[i].field_0x2c;
            }
            for (j = 0; j < g_TrackGame->mode.field_0x1be0; j++) {
                if (g_TrackGame->eventManager->field_0x50[i].field_0x00 == g_TrackGame->mode.field_0x1be4[j].field_0xd4) {
                    field_0x2a0 = g_TrackGame->mode.field_0x1be4[j].field_0xc4;
                    break;
                }
            }
            UnknownFunction49ca60(2);
            field_0x2a4.field_0x08[i].field_0x00 = field_0x298;
            field_0x2a4.field_0x08[i].field_0x01 = field_0x29c;
            SendDebugMessage("\nPlayer %d\n", i + 1);
            SendDebugMessage("Bike %d-%d (%d)\n", field_0x2a4.field_0x08[i].field_0x00, field_0x2a4.field_0x08[i].field_0x01, field_0x2a0);
            SendDebugMessage("Position %d\n", field_0x2a4.field_0x08[i].field_0x02);
            SendDebugMessage("FastestLap %d (scaled)\n", field_0x2a4.field_0x08[i].field_0x04);
            SendDebugMessage("TotalrunningTime %d (scaled)\n", field_0x2a4.field_0x08[i].field_0x08);
            SendDebugMessage("LargestSingleStuntPoints %d\n", field_0x2a4.field_0x08[i].field_0x0c);
        }
        field_0x2a4.field_0x04 = longest;
        return UnknownFunction4aa670(g_TrackGame->field_0x18, &field_0x2a4, (void*)sizeof(field_0x2a4));
    }
    return 1;
}

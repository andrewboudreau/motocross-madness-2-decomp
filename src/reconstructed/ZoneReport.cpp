// The Zone lobby reporting methods of the TrackGame+0x3410 object
// (0x0049c2f0..0x0049dd30). No __FILE__ literal or RTTI reaches this range,
// so the file name is ours (tier 3). The range lies between KrustyVCR and
// LightEmitter.cpp. It is separate from MSZoneInterface.cpp
// (0x004aa350..), which holds the COM calls these methods use.

#include <stdio.h>
#include <string.h>
#include "TrackGame.h"
#include "Parser.h"

unsigned int ReadClock();   // 0x004bfa80: milliseconds

// 0x0049c2f0: reads the lobby preset (0x004aa360) and applies it to the
// race settings. `*flags` gets a bit per tag found (1 event type, 2
// location, 4 manufacturer, 8 bike type), cleared again when 0x0049ca60
// cannot map them. The parser's implicit destructor is emitted out of line
// at 0x0049c5f0 for the unwind funclets; the normal paths inline it.
// Writing the body inside `if (...) { ... return 1; } return 0;` gives
// retail's explicit `xor eax, eax` on the failure path.
int UnknownTrackGameObject3410::ReadLobbyPreset(int* flags)
{
    char response[0x800];
    char name[0x40];
    unsigned long value;
    *flags = 0;
    if (GetLobbyPreset(response, 0x800)) {
        UnknownParser parser;
        parser.Parse(response, lstrlen(response));
        if (parser.GetNumber("EventTypeIndex", &value) != 0x80070005) {
            eventTypeIndex = value - 1;
            *flags |= 1;
        } else {
            eventTypeIndex = -1;
        }
        if (parser.GetNumber("EventTypeLocation", &value) != 0x80070005) {
            eventTypeLocation = value - 1;
            *flags |= 2;
        } else {
            eventTypeLocation = -1;
        }
        if (parser.GetNumber("BikeManufacturer", &value) != 0x80070005) {
            bikeManufacturer = value - 1;
            *flags |= 4;
        } else {
            bikeManufacturer = -1;
        }
        if (parser.GetNumber("BikeType", &value) != 0x80070005) {
            bikeType = value - 1;
            *flags |= 8;
        } else {
            bikeType = -1;
        }
        ConvertLobbyNumbering(1);
        if (bikeIndex == -1) {
            *flags &= ~0xc;
        } else {
            g_TrackGame->mode.racerChoice.modelIndex = bikeManufacturer;
            g_TrackGame->mode.racerChoice.bikeIndex = bikeIndex;
        }
        if (eventTypeIndex == -1)
            *flags &= ~3;
        else
            g_TrackGame->mode.raceSettings.eventType = eventTypeIndex;
        g_TrackGame->mode.raceSettings.trackNumber = trackVariant;
        if (eventTypeLocation != -1)
            strcpy(g_TrackGame->mode.raceSettings.trackName, trackName);
        if (g_TrackGame->network->isHost)
            g_TrackGame->sceneObject->ReadSceneName(name, g_TrackGame->mode.raceSettings.trackName, 0, "scn",
                                                             &g_TrackGame->mode.raceSettings.field_0x138,
                                                             &g_TrackGame->mode.raceSettings.field_0x13c);
        g_TrackGame->mode.raceSettings.treeCollision = 1;
        g_TrackGame->mode.raceSettings.riderCollision = 1;
        g_TrackGame->mode.raceSettings.aiRacerCount = 0;
        g_TrackGame->mode.raceSettings.raceMode = 1;
        g_TrackGame->mode.raceSettings.races = 0;
        g_TrackGame->mode.raceSettings.shortTimeLimit = 0;
        g_TrackGame->mode.raceSettings.lapCount = 5;
        g_TrackGame->mode.raceSettings.timeLimitMinutes = 5.0f;
        g_TrackGame->mode.raceSettings.stuntMode = 0;
        g_TrackGame->mode.raceSettings.gateCount = 0;
        g_TrackGame->mode.raceSettings.tagBall = 0;
        g_TrackGame->mode.savedDifficulty = 3;
        g_TrackGame->mode.difficulty = 3;
        return 1;
    }
    return 0;
}

// 0x0049c600: polls the lobby's rank property (0x004aa4e0) for up to 20
// seconds. The value keyed by the player's name is "%d,%d,%d,%d"; the first
// number is stored in TrackGameMode+0x1bd0. -1 means not ready yet; it
// returns 0 on failure, -1 or -2. The `else return 0` inside the loop keeps
// one shared epilogue; an early `if (!...) return 0;` duplicates it.
int UnknownTrackGameObject3410::WaitForLobbyRank()
{
    int value;
    int b;
    int c;
    int d;
    char response[0x800];
    char text[0x800];
    unsigned int start = ReadClock();
    while (ReadClock() - start < 20000) {
        if (GetLobbyRank(response, 0x800)) {
            UnknownParser parser;
            parser.Parse(response, lstrlen(response));
            parser.GetText(g_TrackGame->mode.field_0x00, text, 0x800);
            sscanf(text, "%d,%d,%d,%d", &value, &b, &c, &d);
            if (value == -1) {
                Sleep(1000);
            } else {
                g_TrackGame->mode.lobbyRank = value;
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
// a lobby connection. Indexing raceStatus.records[i] directly (not
// through a record pointer) gives retail's induction register.
int UnknownTrackGameObject3410::ReportRaceToZone()
{
    int longest;
    int i;
    int j;
    if (g_TrackGame->network->lobbyConnected) {
        raceStatus.eventType = g_TrackGame->mode.raceSettings.eventType + 1;
        longest = 0;
        i = 0;
        eventTypeIndex = g_TrackGame->mode.raceSettings.eventType;
        strcpy(trackName, g_TrackGame->mode.raceSettings.trackName);
        trackVariant = g_TrackGame->mode.raceSettings.trackNumber;
        raceStatus.laps = g_TrackGame->mode.raceSettings.lapCount;
        for (; i < g_TrackGame->playerCount; i++) {
            raceStatus.records[i].position = g_TrackGame->eventManager->results[i].position;
            raceStatus.records[i].fastestLap = (int)(g_TrackGame->eventManager->results[i].fastestLap * 100.0f);
            raceStatus.records[i].totalRunningTime = (int)(g_TrackGame->eventManager->results[i].totalRunningTime * 100.0f);
            if (longest < raceStatus.records[i].totalRunningTime)
                longest = raceStatus.records[i].totalRunningTime;
            raceStatus.records[i].largestStuntPoints = g_TrackGame->eventManager->results[i].largestStuntPoints;
            players[i].position = raceStatus.records[i].position;
            strcpy(players[i].name, g_TrackGame->eventManager->results[i].name);
            if (g_TrackGame->eventManager->results[i].finished) {
                if (g_TrackGame->mode.raceSettings.eventType != 0 && g_TrackGame->mode.raceSettings.eventType != 4)
                    players[i].score = (int)(g_TrackGame->eventManager->results[i].fastestLap * 100.0f);
                else
                    players[i].score = (int)g_TrackGame->eventManager->results[i].score;
            }
            for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                if (g_TrackGame->eventManager->results[i].playerId == g_TrackGame->mode.racers[j].networkPlayerId) {
                    bikeIndex = g_TrackGame->mode.racers[j].bikeIndex;
                    break;
                }
            }
            ConvertLobbyNumbering(2);
            raceStatus.records[i].bikeManufacturer = bikeManufacturer;
            raceStatus.records[i].bikeType = bikeType;
            SendDebugMessage("\nPlayer %d\n", i + 1);
            SendDebugMessage("Bike %d-%d (%d)\n", raceStatus.records[i].bikeManufacturer, raceStatus.records[i].bikeType, bikeIndex);
            SendDebugMessage("Position %d\n", raceStatus.records[i].position);
            SendDebugMessage("FastestLap %d (scaled)\n", raceStatus.records[i].fastestLap);
            SendDebugMessage("TotalrunningTime %d (scaled)\n", raceStatus.records[i].totalRunningTime);
            SendDebugMessage("LargestSingleStuntPoints %d\n", raceStatus.records[i].largestStuntPoints);
        }
        raceStatus.longestRunningTime = longest;
        return SubmitScores(g_TrackGame->playerCount, &raceStatus, (void*)sizeof(raceStatus));
    }
    return 1;
}

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
// The six tables are separate arrays initialised quarry, baja, national,
// supercross, tag, stunt; VC6's frame rule then lays them out as retail.
// Direction 2 computes the 1-based type into its own local in each branch
// (retail folds the +1 into the >= 30 branch as `add ebx, -0x1a`).
void UnknownTrackGameObject3410::ConvertLobbyNumbering(int direction)
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
    int type;
    UnknownTrackNameEntry* entry;

    switch (direction) {
    case 1:
        if (bikeManufacturer >= 0 && bikeManufacturer <= 4) {
            first = 0;
            if (bikeManufacturer == 4)
                bikeManufacturer = 6;
            if (bikeManufacturer == 0 && bikeType >= 3) {
                bikeIndex = bikeType + 27;
            } else {
                for (i = 0; i < bikeManufacturer; i++)
                    first += kBikeTypeCounts[i];
                if (i < 7)
                    bikeIndex = bikeType + first;
                else
                    bikeIndex = -1;
            }
        } else {
            bikeIndex = -1;
        }
        if (eventTypeIndex >= 0 && eventTypeIndex <= 5) {
            entry = &lists[eventTypeIndex].entries[eventTypeLocation];
            trackVariant = entry->variant;
            strcpy(trackName, entry->name);
        } else {
            eventTypeIndex = -1;
        }
        break;
    case 2:
        index = bikeIndex;
        first = 0;
        if (index >= 30) {
            bikeManufacturer = 1;
            type = index - 27 + 1;
        } else {
            for (i = 0; i < 7; i++) {
                if (index < first + kBikeTypeCounts[i])
                    break;
                first += kBikeTypeCounts[i];
            }
            bikeManufacturer = i + 1;
            if (bikeManufacturer == 7)
                bikeManufacturer = 5;
            type = index - first + 1;
        }
        bikeType = type;
        raceStatus.trackEntry = 0;
        entry = lists[eventTypeIndex].entries;
        for (i = 0; i < 16; i++, entry++) {
            if (!_stricmp(entry->name, trackName) && trackVariant == entry->variant) {
                raceStatus.trackEntry = i + 1;
                return;
            }
        }
        break;
    }
}

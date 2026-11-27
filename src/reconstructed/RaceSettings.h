#pragma once

// Eight of these at +0x14c in each RaceSettings. TrackGameMode's
// constructor (0x00522060) clears all three arrays with inline loops, which
// gives this record an inline constructor.
struct GridEntry {
    GridEntry() {
        playerId = 0;
        points = 0;
    }

    int playerId;  // +0x0  network player id (BikeRace compares it; SelectGamePicProcs fills the grid)
    char points;  // +0x4  championship points, the start grid sort key
    char racerIndex;  // +0x5  racer index; BikeRace treats nonzero as an AI racer that player's host runs
};

// The 0x1ec-byte race settings block. TrackGameMode holds three: the current
// settings (+0x27f8) and two saved copies (+0x29e4, +0x2bd0). 0x005231f0 reads
// the copies whole and assigns +0x29e4 to +0x27f8 (rep movsd, 0x7b dwords).
struct RaceSettings {
    int raceMode;                             // +0x00 EventManager 0x0045e600 compares it with 2
    int eventType;                            // +0x04 event type (SPEventDlg EventTypeDropDown; ZoneReport sends it plus one); picks the event object
    int series;                               // +0x08 series (SelectGamePicProcs.cpp; uiinfo.cpp 0x005240e0)
    int races;                                // +0x0c RacesListBox row data, shown as "%d/%d" with racesRun (DlgProcs 0x0044dfc0); cleared by EventManager 0x0045e520
    int treeCollision;                        // +0x10 tree collision (SelectGamePicProcs.cpp)
    int riderCollision;                       // +0x14 rider collision (SelectGamePicProcs.cpp)
    int shortTimeLimit;                       // +0x18 selects a 30 (else 120) second limit (EventManager 0x0045eef0)
    int maxBikeClass;                         // +0x1c largest bike class allowed (SelectGamePicProcs.cpp)
    int lapCount;                             // +0x20 laps (SelectGamePicProcs.cpp)
    int aiRacerCount;                         // +0x24 AI racer count (EventManager 0x0045f9a0)
    int multiplayerAiRacerCount;              // +0x28 MPEventDlg OpponentsListBox row data; multiplayer racer count is this plus playerCount (BikeRace 0x0041d2a0); cleared by NetProcs.cpp 0x004aef40
    unsigned char randomGates;                // +0x2c ChkRandomGates (DlgProcs 0x0044e6c0); cleared by TrackGameMode 0x00522440
    unsigned char gateCount;                  // +0x2d LstNumGates row data (DlgProcs 0x0044e6c0); 30 (TrackGameMode 0x00522440)
    unsigned char field_0x2e[0x30 - 0x2e];
    int gateSeed;                             // +0x30 atoi of the seed text (DlgProcs 0x0044e6c0); 1 (TrackGameMode 0x00522440)
    unsigned char trackNumber;                // +0x34 the selected track entry's scene (DlgProcs 0x0044e6c0)
    unsigned char networkPlayerCount;         // +0x35 lobby message players; decremented when a player leaves (EventManager 0x0045fbd0)
    char trackName[0x20];                     // +0x36 track name (length not established)
    unsigned char field_0x56[0x138 - 0x56];
    int trackFileTime;  // +0x138  ReadSceneName's GetFileTimes: st_mtime of the track scene (ZoneReport, lobby message)
    int trackFileSize;  // +0x13C  GetFileTimes st_size of the scene; a client with another size reports status 3
    float timeLimitMinutes;                   // +0x140 "%dm" rows 2..5 (DlgProcs 0x0044e3e0), SelectGamePicProcs.cpp's minutes
    int tagBall;                              // +0x144 tag ball (SelectGamePicProcs.cpp)
    int stuntMode;                            // +0x148 ChkStuntMode (MPEventDlg 0x004f6370); mode 4 ranks by score, else by tag run (EventManager 0x0045e9d0)
    GridEntry gridOrder[8];  // +0x14C  start grid entries, qsorted by championship points (SelectGamePicProcs); reset by 0x00522cd0
    unsigned char lobbySlots[0x1ec - 0x18c];
};

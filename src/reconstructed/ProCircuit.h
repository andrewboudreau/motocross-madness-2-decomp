#pragma once

// ProCircuit.cpp (D:\aardvark\VC\krusty2\ProCircuit.cpp): the pro circuit
// career at TrackGame+0x3444. No RTTI names the object; its size (0x12e5
// bytes, ProCircuitProcs.cpp allocates it at 0x004d50ed) and the odd
// offsets of every member after the first 0x465 bytes show a
// byte-packed layout. The first 0x1225 bytes are the saved career (0x004d4100
// reads them, 0x004d4150 writes them); the rest come from PCTables.pb,
// PCSched.pb and ui\PCNames.txt. Names are provisional.

class UnknownParameterBlock;

#pragma pack(push, 1)

// Six counters a racer record keeps twice (both cleared by 0x004d3b00).
struct UnknownProCircuitStats {
    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
    int field_0x14;
};

// One racer of the career (0x140 bytes; record 0 is the player).
struct UnknownProCircuitRacer {
    UnknownProCircuitStats field_0x00;
    UnknownProCircuitStats field_0x18;        // +0x28 is compared with 1 and 3 when a series ends
    int money;                                // +0x30 money (starts at StartingCapital)
    int field_0x34;
    char bikeModel[0x40];                     // +0x38 bike model name
    char bikeName[0x40];                      // +0x78 bike name
    char riderModel[0x40];                    // +0xb8 rider model name
    char racerName[0x20];                     // +0xf8 racer name
    char bikeDisplayName[0x20];               // +0x118 bike display name
    int engineSize;                           // +0x138 engine size
    int field_0x13c;
};

// One PCSched.pb track (12 bytes; strings are DebugMalloc'd).
struct UnknownProCircuitTrack {
    char* trackKey;                           // +0x00 the "Track_%d" value
    char* displayName;                        // +0x04 its display name
    char* keyCopy;                            // +0x08 a copy of field_0x00
};

// One PCSched.pb section (16 bytes). ProCircuit.cpp fills Baja (1),
// Nationals (2), Supercross (3) and Enduro (5).
struct UnknownProCircuitSchedule {
    UnknownProCircuitTrack* tracks;           // +0x00 tracks
    int trackCount;                           // +0x04 "NumberOfTracks"
    int bonusTrack;                           // +0x08 "BonusTrack"
    int laps;                                 // +0x0c "Laps"
};

class UnknownTrackGameObject3444 {
public:
    UnknownTrackGameObject3444();             // 0x004d3480: reads PCTables.pb, PCNames.txt and PCSched.pb
    ~UnknownTrackGameObject3444();            // 0x004d39f0
    void FreeSchedule(UnknownProCircuitSchedule* schedule); // 0x004d3a60: frees a schedule
    // 0x004d3b00: starts a career for `name` with `count` racers.
    void StartCareer(const char* name, int a, int b, int count);
    int LoadSaved(const char* path); // 0x004d4100: loads the saved part
    int Save(const char* path); // 0x004d4150: saves it
    int AdvanceAfterRace();              // 0x004d41a0: advances after a race
    // 0x004d4420: reads the selected PCSched.pb section into `schedule`.
    int ReadSchedule(UnknownParameterBlock* block, UnknownProCircuitSchedule* schedule,
                              const char* directory);
    void PickComputerRacers();             // 0x004d4670: picks the computer racers

    char careerName[0x40];                    // +0x00 career name
    int series;                               // +0x40 series (1 Baja, 2 Nationals, 3 Supercross, 5 Enduro)
    int raceIndex;                            // +0x44 race in the series
    int maxModelKind;                         // +0x48 highest model kind the racers may use
    int bikeClassRule;                        // +0x4c bike class rule (1..3)
    int field_0x50;
    char messageText[0x400];                  // +0x54 message text
    int field_0x454;
    int riderRow;                             // +0x458 chosen rider row (ProCircuitProcs.cpp)
    int bikeRow;                              // +0x45c chosen bike row
    int racerCount;                           // +0x460 racer count
    unsigned char careerFlags;                // +0x464 flags
    UnknownProCircuitRacer racers[11];        // +0x465 record 0 is the player
    int startingCapital;                      // +0x1225 "StartingCapital"
    int entryFees[6];                         // +0x1229 entry fees by series ("BajaFee" is [1])
    float purseMultiplier;                    // +0x1241 "PurseMultiplier"
    float repairCapPct;                       // +0x1245 "RepairCapPct"
    float medicalCapPct;                      // +0x1249 "MedicalCapPct"
    float stuntCapPct;                        // +0x124d "StuntCapPct"
    float placePct[11];                       // +0x1251 "%d_PlacePct"
    char** racerNames;                        // +0x127d racer names (ui\PCNames.txt)
    int racerNameCount;                       // +0x1281 their count
    UnknownProCircuitSchedule schedules[6];   // +0x1285 by series
};

#pragma pack(pop)

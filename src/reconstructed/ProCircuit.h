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
    int field_0x30;                           // money (starts at StartingCapital)
    int field_0x34;
    char field_0x38[0x40];                    // bike model name
    char field_0x78[0x40];                    // bike name
    char field_0xb8[0x40];                    // rider model name
    char field_0xf8[0x20];                    // racer name
    char field_0x118[0x20];                   // bike display name
    int field_0x138;                          // engine size
    int field_0x13c;
};

// One PCSched.pb track (12 bytes; strings are DebugMalloc'd).
struct UnknownProCircuitTrack {
    char* field_0x00;                         // the "Track_%d" value
    char* field_0x04;                         // its display name
    char* field_0x08;                         // a copy of field_0x00
};

// One PCSched.pb section (16 bytes). ProCircuit.cpp fills Baja (1),
// Nationals (2), Supercross (3) and Enduro (5).
struct UnknownProCircuitSchedule {
    UnknownProCircuitTrack* field_0x00;       // tracks
    int field_0x04;                           // "NumberOfTracks"
    int field_0x08;                           // "BonusTrack"
    int field_0x0c;                           // "Laps"
};

class UnknownTrackGameObject3444 {
public:
    UnknownTrackGameObject3444();             // 0x004d3480: reads PCTables.pb, PCNames.txt and PCSched.pb
    ~UnknownTrackGameObject3444();            // 0x004d39f0
    void FreeSchedule(UnknownProCircuitSchedule* schedule); // 0x004d3a60: frees a schedule
    // 0x004d3b00: starts a career for `name` with `count` racers.
    void UnknownFunction4d3b00(const char* name, int a, int b, int count);
    int LoadSaved(const char* path); // 0x004d4100: loads the saved part
    int Save(const char* path); // 0x004d4150: saves it
    int AdvanceAfterRace();              // 0x004d41a0: advances after a race
    // 0x004d4420: reads the selected PCSched.pb section into `schedule`.
    int UnknownFunction4d4420(UnknownParameterBlock* block, UnknownProCircuitSchedule* schedule,
                              const char* directory);
    void PickComputerRacers();             // 0x004d4670: picks the computer racers

    char field_0x00[0x40];                    // career name
    int field_0x40;                           // series (1 Baja, 2 Nationals, 3 Supercross, 5 Enduro)
    int field_0x44;                           // race in the series
    int field_0x48;                           // highest model kind the racers may use
    int field_0x4c;                           // bike class rule (1..3)
    int field_0x50;
    char field_0x54[0x400];                   // message text
    int field_0x454;
    int field_0x458;                          // chosen rider row (ProCircuitProcs.cpp)
    int field_0x45c;                          // chosen bike row
    int field_0x460;                          // racer count
    unsigned char field_0x464;                // flags
    UnknownProCircuitRacer field_0x465[11];
    int field_0x1225;                         // "StartingCapital"
    int field_0x1229[6];                      // entry fees by series ("BajaFee" is [1])
    float field_0x1241;                       // "PurseMultiplier"
    float field_0x1245;                       // "RepairCapPct"
    float field_0x1249;                       // "MedicalCapPct"
    float field_0x124d;                       // "StuntCapPct"
    float field_0x1251[11];                   // "%d_PlacePct"
    char** field_0x127d;                      // racer names (ui\PCNames.txt)
    int field_0x1281;                         // their count
    UnknownProCircuitSchedule field_0x1285[6]; // by series
};

#pragma pack(pop)

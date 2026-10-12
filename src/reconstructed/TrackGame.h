#pragma once

#include "CDAudio.h"
#include "ControlInterface.h"
#include "DirectoryList.h"
#include "EventManager.h"
#include "KrustyUI.h"
#include "MemTag.h"
#include "PCGame.h"
#include "UnknownResourceManager.h"
#include "RaceView.h"
#include "TrackRecord.h"

struct UnknownInputEntry;
class FollowCamera;

// RTTI: TrackGame : PCGame (vtable 0x00558938; trkgame.cpp). The global
// pointer at 0x0056e26c is statically initialised to the object at
// 0x006851a0, which a dynamic initializer (0x004aa7c0) constructs with
// 0x00520870 (it writes TrackGame's vtable) and destroys at exit with
// 0x00521ae0. The camera and input code reads it throughout. Only members
// that reconstructed functions touch are declared; names are provisional.

// The race-mode objects at TrackGame+0x558..+0x568 (EventManager uses the
// first one present). They are GameObjects (slots 4 and 5, the +0x25 flag
// bits); their classes are not established.
struct UnknownTrackGameViewPart {
    unsigned char field_0x00[0xc4];
    GameObject* field_0xc4;                   // racesnd.cpp switches it with slot 16
};

// The object at TrackGameViewOwner+0xdc: bikerace.cpp 0x00420650 passes it
// the new replay recorder (0x004a9d10). It is the ghost bike BikeRace.h
// declares as UnknownBikeRaceGhost (FollowRacer.cpp implements both views).
class KrustyVCR;
class UnknownTrackGameViewOwnerDc {
public:
    void SetRecorder(KrustyVCR* vcr); // 0x004a9d10: keeps the recorder at +0x708

    unsigned char field_0x000[0x10a];
    unsigned char field_0x10a;                // cleared when the followed racer changes (0x004a9e80)
    unsigned char field_0x10b[0x708 - 0x10b];
    KrustyVCR* recorder;                      // +0x708
};

// The entries at TrackGameViewOwner+0x420 (BikeRace.h walks them as
// UnknownBikeRaceNodeEntry): +0x58 points at an object whose +0x44 flag
// 0x00404df0 raises for the two entries it leaves and clears for the two
// it picks.
struct UnknownTrackGameViewEntryTarget {
    unsigned char field_0x00[0x44];
    int field_0x44;
};
struct UnknownTrackGameViewEntry {
    unsigned char field_0x00[0x58];
    UnknownTrackGameViewEntryTarget* field_0x58;
};

struct TrackGameViewOwner : public GameObject {
    // 0x004e0c30 (among QuarryStuntEvent.cpp's code): shows the on/off
    // message for string `id` (racesnd.cpp passes 0x1429 and 0x14c3).
    void ShowOnOffMessage(int id, int value);
    void ResetClock();             // 0x004e1f00 (dlgprocs.cpp LoadingDlg)
    // 0x004a9d20 (dlgprocs.cpp 0x004526b0): the host picks the racer to
    // follow (+0xa8) and tells the players and the recorder (FollowRacer.cpp).
    void PickFollowedRacer();
    // 0x004a9e80: follows `racer` after `previous` (KrustyBike.cpp 0x00497986
    // passes itself, 0, 1); `tell` lets the view's own racer know.
    void SetFollowedRacer(UnknownEventRacer* racer, UnknownEventRacer* previous, int tell);
    // 0x004a9f90: lets every other racer know about the switch.
    void AnnounceFollowedRacer(UnknownEventRacer* racer, UnknownEventRacer* previous);
    // 0x00404df0: bikerace.cpp passes a racer's +0x7b8, +0x7bc and the racer
    // (0x00420650 passes 0, 1): picks entries `a` and `b` at +0x420 after
    // flagging the two it leaves; `racer` is unused.
    void UnknownFunction404df0(int a, int b, void* racer);

    UnknownTrackGameViewPart* field_0x2c;
    unsigned char field_0x30[0x34 - 0x30];
    UnknownKrustyBikeView* raceView;          // +0x34
    unsigned char field_0x38[0x3c - 0x38];
    FollowCamera* raceCamera;                 // +0x3c the race camera (InGameProcs.cpp VCRDlg)
    unsigned char field_0x40[0x5c - 0x40];
    GameObject* field_0x5c;                   // EventManager 0x0045cdc0 calls slot 4 on 0x5c..0x68
    GameObject* field_0x60;
    GameObject* field_0x64;
    GameObject* field_0x68;
    TextQueueOverlay* textQueue;              // +0x6c
    float elapsedMinutes;                     // +0x70 compared with the settings' time limit (EventManager 0x0045eef0)
    float elapsedSeconds;                     // +0x74 RaceStatus.cpp 0x004e62d0 adds it to 60 times +0x70
    unsigned char field_0x78[0x9c - 0x78];
    GameObject* field_0x9c;                   // racesnd.cpp calls slot 4 (sound off) or 5 (on)
    unsigned char field_0xa0[0xa8 - 0xa0];
    UnknownEventRacer* followedRacer;         // +0xa8 the followed racer (0x004a9d20 picks it, 0x004a9e80 switches it)
    // One struct stands in for the three owners at TrackGame +0x55c/+0x560/
    // +0x568; +0xac is a gate count in the quarry/course owners and a float
    // (stored through a cast: a float member here moves TrackGame slot 1 and
    // TrackOverlay 0x0051a560) in the bike race owner.
    int field_0xac;                           // RaceStatus.cpp 0x004e6a50 wraps a racer's next gate by it
    int field_0xb0;                           // the two picked entries (0x00404df0)
    int field_0xb4;
    unsigned char field_0xb8[0xdc - 0xb8];
    UnknownTrackGameViewOwnerDc* ball;        // +0xdc string 0x14b7 "FREE BALL!" while set and no racer "is IT!" (StatsOverlay 0x0051aa40); BikeRace.cpp reads it as a ghost bike
    unsigned char field_0xe0[0x420 - 0xe0];
    UnknownTrackGameViewEntry* field_0x420[1];  // +0xac counts them (BikeRace.h)
};

// Settings blocks of TrackGameMode, passed to its reset helpers (0x00522720,
// 0x00522780, 0x005227d0, 0x00522800, 0x00522cb0); 0x005231f0 saves each one
// whole, which gives their sizes.
// The last 0x14 bytes of UnknownTrackGameModeOptions6ac. 0x00522720 clears
// them through a pointer advanced to +0x360 with a fresh zero register, which
// suggests an inline helper (provisional; that function is still a near miss).
struct UnknownTrackGameModeOptions6ac360 {
    void UnknownReset() {
        field_0x00 = 0;
        field_0x04 = 0;
        field_0x08 = 0;
        field_0x0c = 0;
        field_0x10 = 0;
    }

    int field_0x00;
    int field_0x04;
    int field_0x08;
    int field_0x0c;
    int field_0x10;
};

struct UnknownTrackGameModeOptions6ac {       // TrackGameMode+0x6ac, 0x374 bytes
    int instrumentOverlayOn;                  // +0x00 (TrackGameMode +0x6ac)
    int showRaceStatistics;                   // +0x04 (+0x6b0: TrackOverlay 0x00519980)
    int chatInputLine;                        // +0x08 (+0x6b4: 1 shows the chat input line)
    int showNameTags;                         // +0x0c (+0x6b8: TrackOverlay 0x005190e0)
    int field_0x10;
    int field_0x14;
    int field_0x18;
    int field_0x1c;
    int introMovieOn;                         // +0x20 (+0x6cc: Intro1Dlg)
    int field_0x24;
    int field_0x28;
    int field_0x2c;
    int field_0x30;
    int field_0x34;
    int field_0x38;
    int field_0x3c;
    int field_0x40;
    int field_0x44;
    unsigned char field_0x48[0x360 - 0x48];
    UnknownTrackGameModeOptions6ac360 field_0x360;
};

struct UnknownTrackGameModeOptionsA20 {       // TrackGameMode+0xa20, 0x2c bytes
    int field_0x00;
    int field_0x04;
    int soundOn;                              // +0x08 (TrackGameMode+0xa28): sound on
    int field_0x0c;
    int field_0x10;
    int eaxEnabled;                           // +0x14 (TrackGameMode+0xa34): LoadProfile passes it to allowEAXExtension
    int positionSounds;                       // +0x18 (TrackGameMode+0xa38): position sounds on
    int field_0x1c;
    int soundVolume;                          // +0x20 (TrackGameMode+0xa40): sound volume
    int field_0x24;
    int use16BitSound;                        // +0x28 (TrackGameMode+0xa48): 16-bit (else 8-bit) primary format in LoadProfile
};

struct UnknownTrackGameModeOptionsA4c {       // TrackGameMode+0xa4c, 0x1c bytes
    int displayMode;                          // +0x00 (TrackGameMode+0xa4c): display mode index
    int shadows;                              // +0x04 (TrackGameMode+0xa50): shadows on
    int particles;                            // +0x08 (TrackGameMode+0xa54): particles on
    int field_0x0c;
    int skyCube;                              // +0x10 (TrackGameMode+0xa5c): sky cube on
    int field_0x14;
    int detailLevel;                          // +0x18 (TrackGameMode+0xa64): detail level 0..9
};

struct UnknownTrackGameModeOptionsA68 {       // TrackGameMode+0xa68, 0x30 bytes (ResetControlOptions)
    int field_0x00[7];                        // 50 each
    int field_0x1c;
    int joystickOn;                           // +0x20 TrackGameMode+0xa88 joystickOn, reset to 1
    int field_0x24;
    int field_0x28;
    int field_0x2c;
};

// TrackGameMode+0xfd8, +0x1034 and +0x1090 (0x5c bytes each).
struct UnknownTrackGameModeOptionsFd8 {
    int engineSize;                           // +0x00
    int fourStroke;                           // +0x04
    int field_0x08;
    float suspension[6];                      // +0x0c
    int eqBands[11];                          // +0x24 from KrustyUI's +0xec table
    unsigned char field_0x50[0x5c - 0x50];
};

// Eight of these at +0x14c in each UnknownTrackGameModeSettings. TrackGameMode's
// constructor (0x00522060) clears all three arrays with inline loops, which
// gives this record an inline constructor.
struct UnknownTrackGameModeEntry {
    UnknownTrackGameModeEntry() {
        field_0x00 = 0;
        field_0x04 = 0;
    }

    int field_0x00;
    char field_0x04;
};

// The 0x1ec-byte race settings block. TrackGameMode holds three: the current
// settings (+0x27f8) and two saved copies (+0x29e4, +0x2bd0). 0x005231f0 reads
// the copies whole and assigns +0x29e4 to +0x27f8 (rep movsd, 0x7b dwords).
struct UnknownTrackGameModeSettings {
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
    int field_0x138;                          // SelectGamePicProcs.cpp
    int field_0x13c;                          // SelectGamePicProcs.cpp
    float timeLimitMinutes;                   // +0x140 "%dm" rows 2..5 (DlgProcs 0x0044e3e0), SelectGamePicProcs.cpp's minutes
    int tagBall;                              // +0x144 tag ball (SelectGamePicProcs.cpp)
    int stuntMode;                            // +0x148 ChkStuntMode (MPEventDlg 0x004f6370); mode 4 ranks by score, else by tag run (EventManager 0x0045e9d0)
    UnknownTrackGameModeEntry field_0x14c[8]; // reset by TrackGameMode 0x00522cd0
    unsigned char field_0x18c[0x1ec - 0x18c];
};

// The five enumerated network sessions at TrackGameMode+0xa98 (0x53c bytes).
// The constructor builds them with the eh vector constructor iterator. Their
// unwind actions (in the constructor's and destructor's handlers) call an
// out-of-line destroyer (0x00522420, `this` in ecx) that the destructor
// inlines, which is the shape of an implicit destructor of an enclosing
// class (strong inference).
struct UnknownTrackGameSessionList {
    SessionInfoType field_0x00[5];
};

// The player's bike and rider choice at TrackGameMode+0x1974 and its saved
// copy at +0x1a3c (0xc8 bytes each; 0x005231f0 copies it whole). Its layout
// is the first 0xc8 bytes of UnknownTrackGameRacerSlot: SelectGamePicProcs.cpp
// passes its three names to 0x00521fb0 and copies +0xc4 into a racer slot,
// and the constructor sets +0xc0 (strong inference).
struct UnknownTrackGameRacerChoice {
    char bikeModel[0x40];                     // +0x00 bike model name (dlgprocs.cpp SPBikeRiderDlg)
    char bikeName[0x40];                      // +0x40 bike name
    char riderName[0x40];                     // +0x80 rider name
    int modelIndex;                           // +0xc0 -1 (constructor)
    int bikeIndex;                            // +0xc4 SelectGamePicProcs.cpp
};

// Per-racer record (0xf8 bytes); TrackGameMode holds eight at +0x1be4, after
// the count at +0x1be0 (EventManager 0x0045fbd0 copies whole records).
// TrackGameMode's constructor builds them with 0x00521f30 (no destructor).
struct UnknownTrackGameRacerSlot {
    UnknownTrackGameRacerSlot();              // 0x00521f30
    // 0x00521fb0 (SelectGamePicProcs.cpp passes TrackGameMode+0x1974, +0x19b4 and +0x19f4).
    void SetBikeAndRiderNames(void* modelText, void* bikeText, void* riderText);
    void ClearLeft();                         // 0x00522050 (dlgprocs.cpp LoadingDlg): clears +0xc8

    char bikeModelName[0x40];                 // +0x00 0x00521fb0 copies three names (at most 63 chars)
    char bikeName[0x40];                      // +0x40
    char riderName[0x40];                     // +0x80
    int plateNumber;                          // +0xc0 SelectGamePicProcs.cpp
    int bikeIndex;                            // +0xc4 SelectGamePicProcs.cpp
    int hasLeft;                              // +0xc8 set when the player leaves (EventManager slot 24)
    int isReady;                              // +0xcc ready
    float lowestMessageDelay;                 // +0xd0 lowest message delay (bikerace.cpp slot 24); FLT_MAX initially
    int networkPlayerId;                      // +0xd4 network player id
    int racerIndex;                           // +0xd8 player's racer index (EventManager slot 24)
    char playerName[0x10];                    // +0xdc player name (SelectGamePicProcs.cpp)
    int engineSize;                           // +0xec bike record +0x8c (SelectGamePicProcs.cpp 0x004f2340)
    int fourStroke;                           // +0xf0 bike record +0x90
    unsigned char field_0xf4;                 // 1 (constructor)
    unsigned char field_0xf5[0xf8 - 0xf5];
};

// Object embedded at TrackGame+0x578; KrustyBikeCamera slot 52 reads its
// mode. Its methods are uiinfo.cpp's (0x00521f30..0x00524106). It is 0x2dc0
// bytes: 0x005231f0 constructs one on its stack, and the constructor and the
// save/load code use offsets up to +0x2dbc (TrackGame+0x578..+0x3337).
class TrackGameMode {
public:
    TrackGameMode();             // 0x00522060
    int FindCdDirectory(); // 0x00523bf0 (TrackGame slot 1 retries until it passes)
    int FindDataDirectory(); // 0x00523c90 (TrackGame slot 1)
    ~TrackGameMode();            // 0x005225f0
    int GetSelectedSeries(); // 0x00524100
    void SelectSeries(int series); // 0x005240e0 (TrackRecord.cpp 0x00520390)
    // 0x00523d30 (OptionProcs.cpp: "MCM2HELP"): opens `topic`'s .hlp file
    // with ShellExecuteA (passing `parameters`); 1 when found.
    int OpenHelp(const char* topic, const char* parameters);
    void ResetDefaults();             // 0x00522440 (OptionProcs.cpp)
    void LoadControllerChoice();             // 0x00523000 (OptionProcs.cpp)
    void SaveControllerChoice();             // 0x00523130: saves the controller choice
    int WaitForCd();              // 0x00523800: asks for the CD until it is found; 0 on cancel
    void SaveProfile();                           // 0x00523580 (TrackGame slot 15)
    int FindDataFile(const char* name, char* path); // 0x005238f0 (TrackGame slot 18)
    // 0x00522680 (TrackGame slot 4; SelectGamePicProcs.cpp's network start):
    // clears the selected session, the racers and the entries.
    int ResetNetworkRace();
    // Reset helpers called by 0x00522440.
    void ResetInterfaceOptions(UnknownTrackGameModeOptions6ac* options);
    void ResetSoundOptions(UnknownTrackGameModeOptionsA20* options);
    void ResetGraphicsOptions(UnknownTrackGameModeOptionsA4c* options);
    void ResetControlOptions(UnknownTrackGameModeOptionsA68* options); // 0x00522800: +0xa68..+0xa94
    void ReloadResourceStrings(char (*names)[0x80]);   // reloads resourceStrings
    void CopyGarageTables(UnknownTrackGameModeOptionsFd8* options);
    void ResetSuspension(UnknownTrackGameModeOptionsFd8* options); // 0x00522c70: all six at 0.5f
    void ResetEngineOptions(UnknownTrackGameModeOptionsFd8* options); // 0x00522c90: 250, two stroke, tables
    void ResetGarageOptions(UnknownTrackGameModeOptionsFd8* options); // 0x00522cb0: the two above
    void ResetSettingsEntries();              // 0x00522cd0: raceSettings +0x14c
    // 0x00522e20: picks the display mode (640x480x16 preferred) on the
    // adapter whose GUID is `guid`, and stores the adapter GUID.
    void ChooseDisplayMode(const void* guid, int* mode);  // 0x00522e20
    int CreateDirectoryLists();              // 0x00522d00 (TrackGame slot 4): creates the directory lists
    int LoadProfile();              // 0x005231f0 (TrackGame slot 4)
    void LoadBonusTrackNames();             // 0x00523e50 (KrustyUI 0x004988a0)
    void CopyCurrentSeriesDirectory(char* name);   // 0x00523b70 (SelectGamePicProcs.cpp)
    int CopySeriesDirectory(short a, short b, char* name); // 0x00523bb0: copies the series' directory
    // 0x00523a60 (near uiinfo.cpp's literals): finds `name`.`kind` (under
    // directory `value` when nonempty) and stores the directory it is in
    // (EventManager 0x0045cb70 asks for "env", OpenHelp for "hlp").
    // Returns 0 when the file is not found.
    int FindFileDirectory(int value, char* name, const char* kind, char* path);

    char profileName[16];                     // +0x00 "ui\profile\%s\control.ctl", string 0x1466 "Current Profile:"; slot 4 sets it from the network object
    char playerName[0x80];                // +0x10: player name (NetProcs.cpp opens sessions with it)
    int nameTagShade;                         // +0x90 name tag grey step 0..5 (ChatOverlay 0x0051e7c0, 0x0051e910)
    int difficulty;                           // +0x94 "RadLODEasy" group (PCNewDlg reads the same group as the difficulty)
    int savedDifficulty;                      // +0x98 saved +0x94 (dlgprocs.cpp SinglePlayerDlg)
    int showNewbieDialog;                 // +0x9c: cleared by MPBikeRiderDlg (SelectGamePicProcs.cpp); opens NewbieDlg (dlgprocs.cpp)
    char seriesDirectories[6][0x100];     // +0xa0: directories (TrackRecord.cpp 0x0051f0b0)
    int currentSeriesDirectory;           // +0x6a0: a directory name pointer for TrackRecord.cpp 0x0051f2c0; EventManager 0x0045cb70 passes it to 0x00523a60
    int field_0x6a4;                          // cleared when a network race fails to load (dlgprocs.cpp LoadingDlg)
    int visualCuesOn;                         // +0x6a8 visual cue and runway lights on (QuarryStuntEvent.cpp)
    int instrumentOverlayOn;                  // +0x6ac: instrument overlay on (QuarryStuntEvent.cpp)
    int showRaceStatistics;                   // +0x6b0 shows the race statistics (TrackOverlay 0x00519980)
    int chatInputLine;                        // +0x6b4 1 shows the chat input line (QuarryStuntEvent.cpp)
    int showNameTags;                         // +0x6b8 shows the racers' name tags (TrackOverlay 0x005190e0)
    int field_0x6bc;                          // TrackOverlay 0x0051e390
    int field_0x6c0;                          // TrackOverlay 0x0051e390
    unsigned char field_0x6c4[0x6c8 - 0x6c4];
    int field_0x6c8;                          // copied to the GUI's +0xec (KrustyUI 0x004988a0)
    int introMovieOn;                         // +0x6cc: intro movie on (dlgprocs.cpp Intro1Dlg)
    unsigned char field_0x6d0[0x6d4 - 0x6d0];
    int field_0x6d4;                          // copied into GUI layer 0 (EventManager 0x0045e710)
    unsigned char field_0x6d8[0x6dc - 0x6d8];
    int maxAIBikes;                           // +0x6dc: most AI bikes ("EditMaxAIBikes", SelectGamePicProcs.cpp)
    unsigned char field_0x6e0[0x6f4 - 0x6e0];
    char trackNames[6][0x80];             // +0x6f4: a track name per game type (SelectGamePicProcs.cpp)
    int trackIndices[6];                  // +0x9f4: a track index per game type (SelectGamePicProcs.cpp)
    int garageCurveChoice[5];             // +0xa0c: the garage curve chosen per bike class (OptionProcs.cpp's +0x360)
    int field_0xa20;                          // EventManager 0x0045e710 calls TrackGame 0x00521a40 when clear
    int field_0xa24;                          // selects +0xa3c for the GUI's +0x34c (KrustyUI 0x004988a0)
    int soundOn;                              // +0xa28 sound on (racesnd.cpp)
    unsigned char field_0xa2c[0xa34 - 0xa2c];
    int eaxEnabled;                           // +0xa34 TrackGame slot 4 audio argument
    int positionSounds;                       // +0xa38 position sounds on (racesnd.cpp 0x004e39b0)
    int field_0xa3c;
    int soundVolume;                          // +0xa40 sound volume (racesnd.cpp 0x004e3430)
    unsigned char field_0xa44[0xa48 - 0xa44];
    int use16BitSound;                        // +0xa48 selects 16 (else 8) in TrackGame slot 4
    int displayModeIndex;                     // +0xa4c: display mode index (EventManager 0x0045e710)
    int shadowsOn;                            // +0xa50 shadows on (QuarryStuntEvent.cpp)
    int particlesOn;                          // +0xa54 particles on (QuarryStuntEvent.cpp)
    unsigned char field_0xa58[0xa5c - 0xa58];
    int skyCubeOn;                            // +0xa5c: sky cube on (QuarryStuntEvent.cpp)
    unsigned char field_0xa60[0xa64 - 0xa60];
    int detailLevel;                          // +0xa64: detail level 0..9 (QuarryStuntEvent.cpp)
    unsigned char field_0xa68[0xa88 - 0xa68];
    int joystickOn;                           // +0xa88 joystick input on (bikerace.cpp slot 16)
    int gasGyro;                              // +0xa8c toggled by key 0x22, shown as string 0x140d "Gas Gyro" ON/OFF (bikerace.cpp slot 23)
    int brakeGyro;                            // +0xa90 toggled by key 0x30, shown as string 0x142d "Brake Gyro" ON/OFF (bikerace.cpp slot 23)
    unsigned char field_0xa94[0xa98 - 0xa94];
    UnknownTrackGameSessionList sessions; // +0xa98: enumerated sessions (NetProcs.cpp)
    int selectedSession;                  // +0xfd4: selected session (NetProcs.cpp)
    unsigned char garageOptions[0x5c];       // +0xfd8 garage settings (UNKNOWN_GARAGE_SETTINGS); restored from +0x1034 when a replay is left (InGameProcs.cpp)
    unsigned char optionsBackup[0x5c];        // +0x1034: copy of +0xfd8 (DlgProcs.cpp, InGameProcs.cpp)
    unsigned char field_0x1090[0x10ec - 0x1090];
    int championshipInProgress;               // +0x10ec: set when a race-mode 2 series starts (points reset); hides the race count
    int customGarageCurves[5][3][11];     // +0x10f0: custom garage curves per bike class (OptionProcs.cpp)
    int bonusTrackUnlocked[6];            // +0x1384: a bonus track is listed only when set (SelectGamePicProcs.cpp)
    char bonusTrackNames[6][0x20];        // +0x139c: each series' BonusTrack from PCSched.pb (LoadBonusTrackNames)
    char resourceStrings[10][0x80];           // +0x145c string resources 0x1420, 0x1417..0x141f (uiinfo.cpp 0x00522840)
    int videoSettings[5];                 // +0x195c: video settings: forced video memory, then four detail levels (OptionProcs.cpp)
    int bikeUnlockFlags;                      // +0x1970 bit 0 unlocks kind 10 bikes (SelectGamePicProcs.cpp)
    UnknownTrackGameRacerChoice racerChoice;  // +0x1974 the player's bike and rider; restored from +0x1a3c when a replay is left (InGameProcs.cpp)
    UnknownTrackGameRacerChoice racerChoiceBackup; // +0x1a3c
    unsigned char field_0x1b04[0x1bcc - 0x1b04];
    int plateNumber;                          // +0x1bcc SelectGamePicProcs.cpp
    int lobbyRank;                            // +0x1bd0: 0x0049c600 WaitForLobbyRank; 1..100 is copied to +0x1bcc
    int lobbyPresetFlags;                     // +0x1bd4: 0x0049c2f0 ReadLobbyPreset (-1 until read)
    int maxOpponents;                         // +0x1bd8: largest opponent count (SelectGamePicProcs.cpp)
    int field_0x1bdc;                         // cleared by TrackGameMode 0x00522440
    int racerCount;                           // +0x1be0 racer count (EventManager 0x0045e550)
    UnknownTrackGameRacerSlot racers[8];      // +0x1be4 the race's racer slots
    int networkGame;                          // +0x23a4 network game (racesnd.cpp 0x004e5780)
    UnknownGuid displayAdapterGuid;       // +0x23a8: display adapter GUID (TrackGameMode 0x00522e20)
    int savedCameraState;                     // +0x23b8 KrustyBikeCamera state (slots 61, 62); saved in uiinfo
    float savedCameraDistance;                // +0x23bc KrustyBikeCamera presets (slots 59, 60): follow distance (10.3)
    float savedCameraPitch;                   // +0x23c0 follow pitch (0.471)
    float savedCameraYaw;                     // +0x23c4 follow yaw (1.065)
    float savedCameraFieldOfView;             // +0x23c8 target field of view (85)
    int installType;                      // +0x23cc: 2 when the game runs without the CD (uiinfo.cpp 0x00523c90)
    char installDirectory[0x104];         // +0x23d0: installed data directory (uiinfo.cpp 0x00523c90)
    char cdDataDirectory[0x104];          // +0x24d4: CD data directory (uiinfo.cpp 0x00523bf0)
    char cdDriveRoot[4];                  // +0x25d8: CD drive root (uiinfo.cpp 0x00523bf0)
    UnknownDriveList* driveList;          // +0x25dc: created by the constructor (0x005221e7)
    DirectoryList* highScoreDirectoryList;    // +0x25e0 scanned by TrackRecord.cpp 0x00520390
    DirectoryList* trackDirectoryList;    // +0x25e4: track directories (SelectGamePicProcs.cpp)
    CombinedDirectoryList* field_0x25e8;      // created by 0x00522d00
    char ghostFile[0x104];                // +0x25ec: the ghost file raced against (dlgprocs.cpp GhostFilesDlg)
    int saveGhost;                            // +0x26f0 mode 4 saves a ghost (EventManager 0x0045cdc0)
    char replayFile[0x104];               // +0x26f4: the replay file played (dlgprocs.cpp ReplayFilesDlg)
    UnknownTrackGameModeSettings raceSettings; // +0x27f8 current race settings
    UnknownTrackGameModeSettings savedSettings; // +0x29e4: saved copy, restored when a replay is left (InGameProcs.cpp)
    UnknownTrackGameModeSettings field_0x2bd0;
    int saveReplay;                           // +0x2dbc saves a replay (EventManager 0x0045cdc0)
};

// Object returned by (TrackGame+0x56c)->+0x2c->0x00485df0.
class UnknownTrackGameObject56cItem {
public:
    void UnknownFunction46ff30(int value);    // 0x0046ff30 (EventManager 0x0045e710)
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual void UnknownVirtualSlot5();
    virtual void UnknownVirtualSlot6();
    virtual void UnknownVirtualSlot7();
    virtual void UnknownVirtualSlot8();
    virtual void UnknownVirtualSlot9();
    virtual void UnknownVirtualSlot10();
    virtual void UnknownVirtualSlot11();
    virtual void UnknownVirtualSlot12();
    virtual void UnknownVirtualSlot13();
    virtual void UnknownVirtualSlot14();
    virtual void UnknownVirtualSlot15();
    virtual void UnknownVirtualSlot16();
    virtual void UnknownVirtualSlot17();
    virtual void UnknownVirtualSlot18();
    virtual void UnknownVirtualSlot19();
    virtual void UnknownVirtualSlot20();
    virtual void UnknownVirtualSlot21();
    virtual void UnknownVirtualSlot22();
    virtual void UnknownVirtualSlot23();
    virtual void UnknownVirtualSlot24();
    virtual void UnknownVirtualSlot25();
    virtual void UnknownVirtualSlot26();
};

// Object at TrackGame+0x574, deleted by slot 15. Its destructor is
// out-of-line and empty (the shared body 0x00464e90).
class UnknownTrackGameObject574 {
public:
    UnknownTrackGameObject574();              // 0x004e99d0 (defined in SceneManager.cpp)
    ~UnknownTrackGameObject574();             // 0x00464e90
    // 0x004e9a10 / 0x004e9ac0 (SceneManager.cpp): run a temporary Scene.
    int UnknownFunction4e9a10(int* flag);
    int UnknownFunction4e9ac0(unsigned long* info, char* a, char* b, int c);
    // 0x004e9ba0: opens the scene archive `name` (default sceneFile).
    UnknownTextureStream* OpenSceneArchive(const char* name);
    // 0x004e9cd0: opens `name` from the archive in `stream` (racesnd.cpp
    // reads Audio.res samples through it); 0 when not found.
    int OpenArchiveFile(UnknownTextureStream* stream, const char* name, const char* mode, int a);
    void SetSceneFileNames(const char* name); // 0x004e9f70: derived file names
    void SetSceneDirectory(char* path);   // 0x004e9b80 (EventManager 0x0045cb70)
    void SelectScene(char* name, const char* kind, int value); // 0x004e9e30
    // 0x004ea390: writes the scene name for `sceneFile` into `name`.
    void ReadCurrentSceneName(char* name, char* scene, int value);
    // 0x004ea010 (TrackRecord.cpp 0x0051ffe0): a track's display name.
    // Clears *a and *b when given; the archive's 0x00461cb0 fills them.
    void ReadSceneName(char* out, char* name, int index, const char* kind, int* a, int* b);

    // Layout from the constructor's stores; array lengths are inferred
    // from the gaps between them (and the 0x103-byte copy into +0x44).
    UnknownTextureStream* sceneArchive;       // +0x00 open scene archive
    char sceneArchiveName[0x40];              // +0x04 its name
    char sceneDirectory[0x104];               // +0x44 scene directory (SetSceneDirectory)
    char waypointPath[0x104];                 // +0x148 directory\waypoint file
    char sceneFile[0x40];                     // +0x24c scene file name
    char estFile[0x40];                       // +0x28c "%s.est"
    char trnFile[0x40];                       // +0x2cc "%s.trn"
    char field_0x30c[0x40];                   // +0x30c emptied by SetSceneFileNames
    char waypointFile[0x40];                  // +0x34c "%s01.wpt"
    int sceneNamed;                           // +0x38c 0 when SelectScene read "NO NAME"
    int waypointRaceCount;                    // +0x390 SceneInfo "NumberOfWaypointRaces"
};

// Object at TrackGame+0x3338, deleted through its virtual destructor.
// The object at TrackGame+0x3340 is CDAudio (CDAudio.h).

// UnknownTrackGameObject3400, the object at TrackGame+0x3400, is in TrackRecord.h.

// One control assignment of UnknownTrackGameObject33fc: the device kind
// (0 keyboard, 1 mouse, 2 joystick; -1 unassigned) and the key, button or
// negative axis direction code. Saved as "kind,code" (DeviceSetup.cpp).
struct UnknownControlAssignment {
    int kind;
    int code;
};

// Object at TrackGame+0x33fc (constructor near DebugOverlay.cpp's literals;
// no destructor). Its methods are in DeviceSetup.cpp (provisional name).
class UnknownTrackGameObject33fc {
public:
    UnknownTrackGameObject33fc();             // 0x00448960
    // OptionProcs.cpp's control mapping dialog (provisional roles).
    void SetAssignment(int device, int row, int kind, int code); // 0x00448c90
    int FindDuplicateAssignment(int row, int kind, int code, int* other);  // 0x00448cc0
    void ApplyDevice(const char* path, int device);               // 0x00448e90
    void LoadControlFile(const char* path);                        // 0x00448990 (uiinfo.cpp 0x00523000)
    void SaveControlFile(const char* path);                        // 0x004489e0 (uiinfo.cpp 0x00523130)
    int ChooseDefaultDevice();                                         // 0x00449220 (uiinfo.cpp 0x00523000)
    void GetAssignmentName(int device, int row, char* text);         // 0x00449350
    int FormatAssignmentName(int kind, int code, char* text);            // 0x00449380
    // 0x00448a50 / 0x00448bc0: reads / writes "Controller<device>",
    // "Key<row>" of the control file `path`.
    void LoadAssignment(int device, int row, const char* path);
    void SaveAssignment(int device, int row, const char* path);
    // 0x00448d30: binds an axis binding to the device of `kind`.
    int BindAxis(UnknownControlBinding* binding, int kind, int code, int code2);
    // 0x00448df0: enters `code` for `control` in TrackGame's control mapping.
    int BindButton(int control, int code, int kind);

    int currentDevice;                        // +0x00: input device (OptionProcs.cpp "InputDeviceDDL")
    UnknownControlAssignment assignments[8][14]; // +0x04: per input device, per control row
};

// One player reported by 0x004aa670 (0x48 bytes).
struct UnknownZonePlayerRecord {
    char name[0x40];                          // +0x00
    unsigned char position;                   // +0x40: 1, other nonzero or zero: flags 2, 4 or 16
    int score;                                // +0x44
};

// One racer's line of the race status (0x10 bytes). The names are the
// labels of 0x0049c770's debug messages.
struct UnknownZoneRaceRecord {
    unsigned char bikeManufacturer;           // +0x00: "Bike %d-%d": the bike manufacturer
    unsigned char bikeType;                   // +0x01: and the bike type
    unsigned char position;                   // +0x02: "Position"
    unsigned char field_0x03;
    int fastestLap;                           // +0x04: "FastestLap" (x100)
    int totalRunningTime;                     // +0x08: "TotalrunningTime" (x100)
    int largestStuntPoints;                   // +0x0c: "LargestSingleStuntPoints"
};

// The race status 0x0049c770 passes to 0x004aa670 with its size (0x88).
struct UnknownZoneRaceStatus {
    unsigned char eventType;                  // +0x00: event type index plus one
    unsigned char trackEntry;                 // +0x01: 0x0049ca60(2): the track's 1-based entry, 0 when not found
    unsigned char laps;                       // +0x02: laps
    unsigned char field_0x03;
    int longestRunningTime;                   // +0x04: the longest total running time
    UnknownZoneRaceRecord records[8];         // +0x08
};

// Base of the +0x3410 object; its constructor sits among
// FontTextureManager.cpp's literals.
class UnknownTrackGameObject3410Base {
public:
    UnknownTrackGameObject3410Base();         // 0x004676a0

    // The constructor clears +0 and +4; 0x004aa350 sets them
    // (MSZoneInterface.cpp).
    UnknownDirectPlay4A* directPlay;          // +0x00
    UnknownDirectPlayLobby3A* lobby;          // +0x04
    UnknownZonePlayerRecord players[8];       // +0x08: reported by 0x004aa670
    unsigned char field_0x248[0x24c - 0x248];
    // 0x0049c2f0 fills these from the lobby preset: a tag's value minus
    // one, or -1 when the tag is missing.
    int eventTypeIndex;                       // +0x24c: "EventTypeIndex"
    int eventTypeLocation;                    // +0x250: "EventTypeLocation"
    int trackVariant;                         // +0x254: the race settings' +0x34 (a byte there)
    char trackName[0x40];                     // +0x258: track name copied when +0x250 is set
    int bikeManufacturer;                     // +0x298: the lobby's bike model (SelectGamePicProcs.cpp); "BikeManufacturer"
    int bikeType;                             // +0x29c: "BikeType"
    int bikeIndex;                            // +0x2a0: tested after 0x0049ca60(1)
    UnknownZoneRaceStatus raceStatus;         // +0x2a4: sent through 0x004aa670 by 0x0049c770
};

// Object at TrackGame+0x3410, created for network games. It has no
// constructor of its own: `new` calls the base constructor directly and uses
// the allocation (not the constructor's result), and its method sits among
// MSZoneInterface.cpp's literals.
class UnknownTrackGameObject3410 : public UnknownTrackGameObject3410Base {
public:
    void SetDirectPlay(UnknownDirectPlay4A* a, UnknownDirectPlayLobby3A* b); // 0x004aa350
    // MSZoneInterface.cpp: 0x004aa360 / 0x004aa4e0 ask the lobby for the
    // preset / rank property and copy it into `buffer` (at most `size`
    // bytes); 0x004aa670 reports `count` players through a COM object.
    int GetLobbyPreset(char* buffer, unsigned int size);
    int GetLobbyRank(char* buffer, unsigned int size);
    int SubmitScores(unsigned int count, void* a, void* b);
    int ReportRaceToZone();                   // 0x0049c770 (EventManager 0x0045e550): reports the race to the Zone
    int ReadLobbyPreset(int* flags);          // 0x0049c2f0 (SelectGamePicProcs.cpp: TrackGameMode+0x1bd4)
    void ConvertLobbyNumbering(int kind);     // 0x0049ca60
    int WaitForLobbyRank();                   // 0x0049c600 (SelectGamePicProcs.cpp): waits up to 20 s for the lobby rank
};

// Object at TrackGame+0x3444 (0x00521cd0 tests that it exists): the pro
// circuit career, declared in ProCircuit.h.
#include "ProCircuit.h"

// cdecl 0x00520820 (near TrackRecord.cpp's literals): formats a message
// (_vsnprintf into 0x200 bytes) and sends it through 0x0068a48c.
void SendDebugMessage(const char* format, ...);

// Global object at 0x0068a48c, deleted by TrackGame's destructor. Slot 4
// connects it to the "debugIP"/"debugPort" registry settings (name tier 3).
class DebugSocket {
public:
    DebugSocket();           // 0x004ad3b0 (defined in Net.cpp)
    ~DebugSocket();          // 0x004ad3d0
    int Connect(const char* address, int port); // 0x004ad3e0: 1 on failure
    int SendData(const char* data, int length);  // 0x004ad530
    int SendText(const char* text);              // 0x004ad570

    unsigned int socketHandle;                // +0x00 WinSock socket
    int lastResult;                           // +0x04 last WinSock result
    int field_0x08;
    int isConnected;                          // +0x0c connected
    int socketPort;                           // +0x10 port
};
extern DebugSocket* g_DebugSocket;

class TrackGame : public PCGame {
public:
    TrackGame();                              // 0x00520870
    virtual ~TrackGame();                     // 0x00521ae0 (deleting wrapper 0x00520a90)
    virtual int UnknownVirtualSlot1();        // 0x00520ab0: start-up checks and the EULA
    virtual int UnknownVirtualSlot2();        // 0x00520d00: PCGame's slot 2
    virtual int UnknownVirtualSlot3();        // 0x00520d10: opens the resource archives
    virtual int UnknownVirtualSlot4();        // 0x00521050: creates the game objects
    virtual int UnknownVirtualSlot5();        // 0x00521a50
    virtual int UnknownVirtualSlot10();       // 0x00521660: Game's slot 10 as a flag
    virtual int UnknownVirtualSlot13(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00521840
    virtual int UnknownVirtualSlot14(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00521670
    virtual int UnknownVirtualSlot15();       // 0x00521a70: shutdown
    virtual int UnknownVirtualSlot18(const char* name, char* path); // 0x00521cb0

    void MuteSound();             // 0x00521a30
    void StopCDAudio();             // 0x00521a40
    int InProCircuitCareer();              // 0x00521cd0: proCircuit set and KrustyUI +0x4a8 clear

    // 0x00521970: loads string resource `id` into buffer (size bytes), or
    // "Resource String Unavailable"; 1 when loaded.
    int LoadResourceString(int id, char* buffer, int size);
    // 0x00521860: opens (`open`) or closes the in-game menu `id`.
    void SetMenuOpen(int open, int id, int sound);

    // The event objects, one per mode.raceSettings.eventType: KrustyBikeCamera
    // slot 10 (0x00497e20) takes +0x34 of the one for the event type, and
    // the type also picks the series directory (0x005240e0: Quarries, Baja,
    // National, SX, Tag, Enduro). 0x0045cb70 creates types 0 and 2.
    TrackGameViewOwner* supercrossEvent;      // +0x558 event type 3 (SX)
    TrackGameViewOwner* quarryEvent;          // +0x55c event type 0 (BaseQuarryEvent)
    TrackGameViewOwner* bajaEnduroEvent;      // +0x560 event types 1 and 5 (Baja, Enduro)
    TrackGameViewOwner* nationalEvent;        // +0x564 event type 2 (NationalRace)
    TrackGameViewOwner* tagEvent;             // +0x568 event type 4 (Tag)
    KrustyUI* ui;
    EventManager* eventManager;
    UnknownTrackGameObject574* sceneObject;
    TrackGameMode mode;                       // +0x578..+0x3337
    DirectoryList* profileDirectory;
    int menuIsOpen;
    CDAudio* cdAudio;                         // +0x3340
    UnknownControlBinding field_0x3344;
    UnknownControlBinding field_0x3380;
    UnknownControlBinding field_0x33bc;
    UnknownControlMapping* controlMapping;
    UnknownTrackGameObject33fc* controlSetup; // +0x33fc control assignments: the profile's "control.ctl", OptControlsDlg InputDeviceDDL
    UnknownTrackGameObject3400* highScores;   // +0x3400 the high-score table (TrackRecord.cpp)
    char savedDecimalSeparator[8];
    void* languageModule;
    UnknownTrackGameObject3410* networkGameObject;
    float fullNetPacketIntervalSeconds;
    float shortNetPacketIntervalSeconds;
    float fullRecordPacketIntervalSeconds;
    float shortRecordPacketIntervalSeconds;
    int field_0x3424;                          // EventManager 0x0045fbd0 decrements it for a leaving player whose entry has racer +0x11c0 set (Game+0x18 otherwise)
    int playingReplay;                        // +0x3428
    int useReplayTape;                        // +0x342c
    int uiInteractionBlocked;
    float uiBlockedSeconds;                   // +0x3434 seconds UI interaction has been blocked (EventManager slot 10)
    int field_0x3438;
    int openLocalizedWebPageOnExit;
    int openStorePageOnExit;
    UnknownTrackGameObject3444* proCircuit;   // +0x3444 the pro circuit career (ProCircuit.h)
    char careerFile[0x34c8 - 0x3448];         // +0x3448: the pro circuit career file loaded/saved (ProCircuitProcs.cpp)
    int screenSaverWasActive;
};

extern TrackGame* g_TrackGame;   // 0x0056e26c: initialised to the TrackGame singleton 0x006851a0

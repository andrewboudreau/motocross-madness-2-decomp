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
// bits). Their members at +0x2c..+0x9c sit where BaseQuarryEvent
// (QuarryEvent.h) keeps its scene, overlays and AuralScape; the exact
// class of each owner is not established.
struct UnknownTrackGameViewPart {
    unsigned char field_0x00[0xc4];
    GameObject* soundGroup;  // +0xC4  racesnd.cpp mutes it with slot 16 beside the engine/effects groups; the podium's sound group
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
    void SelectLapGates(int a, int b, void* racer);

    UnknownTrackGameViewPart* eventScene;     // +0x2C  BaseQuarryEvent::eventScene (0x004df108 stores the Scene that 0x004ea7e0 sets up)
    unsigned char field_0x30[0x34 - 0x30];
    UnknownKrustyBikeView* raceView;          // +0x34
    unsigned char field_0x38[0x3c - 0x38];
    FollowCamera* raceCamera;                 // +0x3c the race camera (InGameProcs.cpp VCRDlg)
    unsigned char field_0x40[0x5c - 0x40];
    GameObject* statsOverlay;                 // +0x5C  BaseQuarryEvent::statsOverlay (StatsOverlay ctor 0x00519370, stored by 0x004e07c0)
    GameObject* radarOverlay;                 // +0x60  BaseQuarryEvent::radarOverlay (RadarOverlay ctor 0x0051b690, stored by 0x004e07c0)
    GameObject* chatOverlay;                  // +0x64  BaseQuarryEvent::chatOverlay (ChatOverlay ctor 0x0051cda0); BikeRace copies it to its chatOverlay
    GameObject* instrumentOverlay;            // +0x68  BaseQuarryEvent::instrumentOverlay (InstrumentOverlay ctor 0x00518720, stored by 0x004e07c0)
    TextQueueOverlay* textQueue;              // +0x6c
    float elapsedMinutes;                     // +0x70 compared with the settings' time limit (EventManager 0x0045eef0)
    float elapsedSeconds;                     // +0x74 RaceStatus.cpp 0x004e62d0 adds it to 60 times +0x70
    unsigned char field_0x78[0x9c - 0x78];
    GameObject* auralScape;                   // +0x9C  BaseQuarryEvent::auralScape (0x004dee7d stores AuralScape::Create); racesnd.cpp enables it with sound on
    unsigned char field_0xa0[0xa8 - 0xa0];
    UnknownEventRacer* followedRacer;         // +0xa8 the followed racer (0x004a9d20 picks it, 0x004a9e80 switches it)
    // One struct stands in for the three owners at TrackGame +0x55c/+0x560/
    // +0x568; +0xac is a gate count in the quarry/course owners and a float
    // (stored through a cast: a float member here moves TrackGame slot 1 and
    // TrackOverlay 0x0051a560) in the bike race owner.
    int field_0xac;                           // RaceStatus.cpp 0x004e6a50 wraps a racer's next gate by it
    int lapGate;  // +0xB0  picked gate: bikerace/KrustyBike pass the racer's lapGate (0x00404df0)
    int nextLapGate;  // +0xB4  picked gate: the racer's nextLapGate = (lapGate + 1) % count (0x00404df0)
    unsigned char field_0xb8[0xdc - 0xb8];
    UnknownTrackGameViewOwnerDc* ball;        // +0xdc string 0x14b7 "FREE BALL!" while set and no racer "is IT!" (StatsOverlay 0x0051aa40); BikeRace.cpp reads it as a ghost bike
    unsigned char field_0xe0[0x420 - 0xe0];
    UnknownTrackGameViewEntry* gates[1];  // +0x420  BikeRace 0x0041d170 builds gateList from the first +0xac entries
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
    int showRiderNames;                       // +0x10 RiderNamesCheckBox (OptionProcs.h)
    int showRiderPositions;                   // +0x14 RiderPositionCheckBox
    int showPodiums;                          // +0x18 ChkPodiums
    int uiAnimations;                         // +0x1c ChkUIAnims
    int introMovieOn;                         // +0x20 (+0x6cc: Intro1Dlg)
    int playerIndicator;                      // +0x24 ChkPlayerIndicator
    int toolTips;                             // +0x28 ChkToolTips
    int wreckResetOnTrack;                    // +0x2c ChkWreckResetOnTrack
    int maxAIBikes;                           // +0x30  (TrackGameMode +0x6dc maxAIBikes, EditMaxAIBikes); 0x00522720 resets it to 2
    int field_0x34;
    int field_0x38;
    int field_0x3c;
    int field_0x40;
    int field_0x44;
    unsigned char field_0x48[0x360 - 0x48];
    UnknownTrackGameModeOptions6ac360 garageCurveChoice;  // +0x360  TrackGameMode+0xa0c garageCurveChoice[5] (0x6ac + 0x360); OptionProcs.h curveRows
};

struct UnknownTrackGameModeOptionsA20 {       // TrackGameMode+0xa20, 0x2c bytes
    int field_0x00;
    int uiSounds;                             // +0x04 UserInterfaceSoundsCheckBox (OptionProcs.h)
    int soundOn;                              // +0x08 (TrackGameMode+0xa28): sound on
    int field_0x0c;
    int soundHardware;                        // +0x10 SoundHardwareCheckBox
    int eaxEnabled;                           // +0x14 (TrackGameMode+0xa34): LoadProfile passes it to allowEAXExtension
    int positionSounds;                       // +0x18 (TrackGameMode+0xa38): position sounds on
    int uiVolume;                             // +0x1c UserInterfaceVolumeSlider
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
    int responseSliders[7];                   // +0x00 50 each: Steering, Crossup, Pitch, PressThrottle, PressBrakes, ReleaseThrottle, ReleaseBrakes sliders (OptionProcs.h)
    int feedback;                             // +0x1c FeedbackSlider
    int joystickOn;                           // +0x20 TrackGameMode+0xa88 joystickOn, reset to 1
    int gasGyro;                              // +0x24 ChkGasGyro, reset to 0
    int brakeGyro;                            // +0x28 ChkBrakeGyro, reset to 1
    int analogAxes;                           // +0x2c ChkAnalogAxes
};

// TrackGameMode+0xfd8, +0x1034 and +0x1090 (0x5c bytes each).
struct UnknownTrackGameModeOptionsFd8 {
    int engineSize;                           // +0x00
    int fourStroke;                           // +0x04
    int weight;                               // +0x08 KrustyUI "Weight" (OptionProcs.h)
    float suspension[6];                      // +0x0c
    int eqBands[11];                          // +0x24 from KrustyUI's +0xec table
    int firstBand;                            // +0x50 KrustyUI "RPMLowerLimit" (OptionProcs.h)
    int lastBand;                             // +0x54 KrustyUI "RPMUpperLimit"
    int bandStep;                             // +0x58 a tenth of their range
};

// Eight of these at +0x14c in each UnknownTrackGameModeSettings. TrackGameMode's
// constructor (0x00522060) clears all three arrays with inline loops, which
// gives this record an inline constructor.
struct UnknownTrackGameModeEntry {
    UnknownTrackGameModeEntry() {
        playerId = 0;
        points = 0;
    }

    int playerId;  // +0x0  SelectGamePicProcs' UnknownGridEntry view: network player id (BikeRace compares it)
    char points;  // +0x4  UnknownGridEntry view: championship points, the start grid sort key
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
    int trackFileTime;  // +0x138  ReadSceneName's GetFileTimes: st_mtime of the track scene (ZoneReport, lobby message)
    int trackFileSize;  // +0x13C  GetFileTimes st_size of the scene; a client with another size reports status 3
    float timeLimitMinutes;                   // +0x140 "%dm" rows 2..5 (DlgProcs 0x0044e3e0), SelectGamePicProcs.cpp's minutes
    int tagBall;                              // +0x144 tag ball (SelectGamePicProcs.cpp)
    int stuntMode;                            // +0x148 ChkStuntMode (MPEventDlg 0x004f6370); mode 4 ranks by score, else by tag run (EventManager 0x0045e9d0)
    UnknownTrackGameModeEntry gridOrder[8];  // +0x14C  start grid entries, qsorted by championship points (SelectGamePicProcs); reset by 0x00522cd0
    unsigned char field_0x18c[0x1ec - 0x18c];
};

// The five enumerated network sessions at TrackGameMode+0xa98 (0x53c bytes).
// The constructor builds them with the eh vector constructor iterator. Their
// unwind actions (in the constructor's and destructor's handlers) call an
// out-of-line destroyer (0x00522420, `this` in ecx) that the destructor
// inlines, which is the shape of an implicit destructor of an enclosing
// class (strong inference).
struct UnknownTrackGameSessionList {
    SessionInfoType entries[5];  // +0x0  NetProcs.cpp enumerates sessions into it and indexes it by selectedSession
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
    unsigned char hasMprSkin;  // +0xF4  with registry MPR == 123 BikeRace gives the rider "MPR.tga"; 1 (constructor)
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
    int showRiderNames;                       // +0x6bc RiderNamesCheckBox (OptionProcs.h's TrackGame+0xc24 block); TrackOverlay 0x0051e390
    int showRiderPositions;                   // +0x6c0 RiderPositionCheckBox; TrackOverlay 0x0051e390
    int showPodiums;                          // +0x6c4 ChkPodiums (OptionProcs.h's TrackGame+0xc24 block +0x18); EventManager 0x0045d480
    int uiAnimations;                         // +0x6c8 ChkUIAnims; copied to the GUI's +0xec (KrustyUI 0x004988a0)
    int introMovieOn;                         // +0x6cc: intro movie on (dlgprocs.cpp Intro1Dlg)
    int playerIndicator;                      // +0x6d0 ChkPlayerIndicator (block +0x24); BikeRace 0x00417ed0
    int toolTips;                             // +0x6d4 ChkToolTips; copied into GUI layer 0's tool tip (EventManager 0x0045e710)
    int wreckResetOnTrack;                    // +0x6d8 ChkWreckResetOnTrack (block +0x2c)
    int maxAIBikes;                           // +0x6dc: most AI bikes ("EditMaxAIBikes", SelectGamePicProcs.cpp)
    unsigned char field_0x6e0[0x6f4 - 0x6e0];
    char trackNames[6][0x80];             // +0x6f4: a track name per game type (SelectGamePicProcs.cpp)
    int trackIndices[6];                  // +0x9f4: a track index per game type (SelectGamePicProcs.cpp)
    int garageCurveChoice[5];             // +0xa0c: the garage curve chosen per bike class (OptionProcs.cpp's +0x360)
    int field_0xa20;                          // EventManager 0x0045e710 calls TrackGame 0x00521a40 when clear
    int uiSounds;                             // +0xa24 UserInterfaceSoundsCheckBox (OptionProcs.h's TrackGame+0xf98 block); selects uiVolume for the GUI's +0x34c (KrustyUI 0x004988a0)
    int soundOn;                              // +0xa28 sound on (racesnd.cpp)
    int field_0xa2c;
    int soundHardware;                        // +0xa30 SoundHardwareCheckBox (OptionProcs.h's TrackGame+0xf98 block +0x10)
    int eaxEnabled;                           // +0xa34 TrackGame slot 4 audio argument
    int positionSounds;                       // +0xa38 position sounds on (racesnd.cpp 0x004e39b0)
    int uiVolume;                             // +0xa3c UserInterfaceVolumeSlider
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
    int responseSliders[7];                   // +0xa68 Steering, Crossup, Pitch, PressThrottle, PressBrakes, ReleaseThrottle, ReleaseBrakes sliders (OptionProcs.h's TrackGame+0xfe0 block)
    int feedback;                             // +0xa84 FeedbackSlider
    int joystickOn;                           // +0xa88 joystick input on (bikerace.cpp slot 16)
    int gasGyro;                              // +0xa8c toggled by key 0x22, shown as string 0x140d "Gas Gyro" ON/OFF (bikerace.cpp slot 23)
    int brakeGyro;                            // +0xa90 toggled by key 0x30, shown as string 0x142d "Brake Gyro" ON/OFF (bikerace.cpp slot 23)
    int analogAxes;                           // +0xa94 ChkAnalogAxes (OptionProcs.h's TrackGame+0xfe0 block +0x2c)
    UnknownTrackGameSessionList sessions; // +0xa98: enumerated sessions (NetProcs.cpp)
    int selectedSession;                  // +0xfd4: selected session (NetProcs.cpp)
    unsigned char garageOptions[0x5c];       // +0xfd8 garage settings (UNKNOWN_GARAGE_SETTINGS); restored from +0x1034 when a replay is left (InGameProcs.cpp)
    unsigned char optionsBackup[0x5c];        // +0x1034: copy of +0xfd8 (DlgProcs.cpp, InGameProcs.cpp)
    unsigned char multiplayerGarageOptions[0x10ec - 0x1090]; // +0x1090 garage settings kept for multiplayer: saved when a multiplayer race starts (0x004536e0), restored by MultiPlayerDlg
    int championshipInProgress;               // +0x10ec: set when a race-mode 2 series starts (points reset); hides the race count
    int customGarageCurves[5][3][11];     // +0x10f0: custom garage curves per bike class (OptionProcs.cpp)
    int bonusTrackUnlocked[6];            // +0x1384: a bonus track is listed only when set (SelectGamePicProcs.cpp)
    char bonusTrackNames[6][0x20];        // +0x139c: each series' BonusTrack from PCSched.pb (LoadBonusTrackNames)
    char resourceStrings[10][0x80];           // +0x145c string resources 0x1420, 0x1417..0x141f (uiinfo.cpp 0x00522840)
    int videoSettings[5];                 // +0x195c: video settings: forced video memory, then four detail levels (OptionProcs.cpp)
    int bikeUnlockFlags;                      // +0x1970 bit 0 unlocks kind 10 bikes (SelectGamePicProcs.cpp)
    UnknownTrackGameRacerChoice racerChoice;  // +0x1974 the player's bike and rider; restored from +0x1a3c when a replay is left (InGameProcs.cpp)
    UnknownTrackGameRacerChoice racerChoiceBackup; // +0x1a3c
    unsigned char multiplayerRacerChoice[0x1bcc - 0x1b04]; // +0x1b04 racerChoice kept for multiplayer (same paths as +0x1090)
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
    UnknownTrackGameModeSettings multiplayerSettings; // +0x2bd0 race settings kept for multiplayer (same paths as +0x1090)
    int saveReplay;                           // +0x2dbc saves a replay (EventManager 0x0045cdc0)
};

// Object returned by (TrackGame+0x56c)->+0x2c->0x00485df0.
class UnknownTrackGameObject56cItem {
public:
    void EndDialog(int value);    // 0x0046ff30 (EventManager 0x0045e710)
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

#include "SceneArchiveFinder.h"

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
    virtual int Startup();                    // 0x00520ab0: start-up checks and the EULA
    virtual int Initialize();                 // 0x00520d00: PCGame's slot 2
    virtual int OpenResources();              // 0x00520d10: opens the resource archives
    virtual int CreateGameObjects();          // 0x00521050: creates the game objects
    virtual int OnActivate();                 // 0x00521a50
    virtual int Tick();                       // 0x00521660: Game's slot 10 as a flag
    virtual int OnControlRelease(UnknownControlEvent* event, UnknownInputEntry* entry);     // 0x00521840
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);       // 0x00521670
    virtual int Shutdown();                   // 0x00521a70: shutdown
    virtual int FindDataFile(const char* name, char* path);         // 0x00521cb0

    void MuteSound();             // 0x00521a30
    void StopCDAudio();             // 0x00521a40
    int InProCircuitCareer();              // 0x00521cd0: proCircuit set and KrustyUI playingDemo (+0x4a8) clear

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
    UnknownControlBinding steerBinding;  // +0x3344  axis of control rows 0/1 (dead zone 0.04); LoadBike's steer -> Vehicle steerAxis
    UnknownControlBinding leanBinding;  // +0x3380  axis of control rows 2/3; LoadBike's lean -> Vehicle leanAxis (controlInput.y)
    UnknownControlBinding throttleBinding;  // +0x33BC  optional 0..1 axis of row 4; LoadBike's throttle -> Vehicle throttleAxis
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
    int networkAiRacerCount;                   // +0x3424 AI racers in a multiplayer race (multiplayerAiRacerCount at the start; racerCount = it + playerCount); EventManager 0x0045fbd0 decrements it for a leaving player whose entry has racer +0x11c0 set (Game+0x18 otherwise)
    int playingReplay;                        // +0x3428
    int useReplayTape;                        // +0x342c
    int uiInteractionBlocked;
    float uiBlockedSeconds;                   // +0x3434 seconds UI interaction has been blocked (EventManager slot 10)
    int podiumFinished;                       // +0x3438 set when the podium ends, is skipped or fails to load; the next EndRace closes the race and clears it
    int openLocalizedWebPageOnExit;
    int openStorePageOnExit;
    UnknownTrackGameObject3444* proCircuit;   // +0x3444 the pro circuit career (ProCircuit.h)
    char careerFile[0x34c8 - 0x3448];         // +0x3448: the pro circuit career file loaded/saved (ProCircuitProcs.cpp)
    int screenSaverWasActive;
};

extern TrackGame* g_TrackGame;   // 0x0056e26c: initialised to the TrackGame singleton 0x006851a0

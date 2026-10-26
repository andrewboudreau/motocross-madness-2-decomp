#pragma once

#include "GameObject.h"
#include "MatrixUtil.h"

class Camera;
class RenderTarget;
struct UnknownKrustyUIModelObject;

// Shared GUI manager and user declarations. Legacy view names are aliases;
// object storage and methods belong to the existing GUI implementations.
class GUIManager;
class GUIUser;
typedef GUIManager UnknownKrustyUIGui;
typedef GUIUser UnknownKrustyUIGuiLayer;

// Tooltip option boundary used without importing the Windows drawing types.
struct UnknownKrustyUIToolTipView {
    unsigned char field_0x00[0x5c];
    int enabled;
};

// LoadingDlg publishes its UIDialog base at +0x490.
class UIDialog;

// 8-byte record; KrustyUI keeps eight at +0x634 (the constructor's loop
// clears both fields of each).
struct UnknownKrustyUISlot {
    UnknownKrustyUISlot() : field_0x00(0), field_0x04(0) {}

    int field_0x00;
    char field_0x04;
};

// 0x54-byte record in KrustyUI's +0x60 list (appended by 0x0049ba70): a
// track list row; SelectGamePicProcs.cpp passes the scene number, series and
// .env name.
struct UnknownKrustyUIEntry {
    int directoryIndex;                       // +0x0  the track directory entry (SelectGamePicProcs.cpp passes it to GetEntryStem)
    int scene;                                // +0x04: scene number; copied to the settings' trackNumber
    int series;                               // +0x08
    int trackFileTime;                        // +0xC  copied to the race settings' trackFileTime (SelectGamePicProcs.cpp); the host passes ReadSceneName's x
    int trackFileSize;                        // +0x10  copied to the race settings' trackFileSize; the host passes ReadSceneName's y
    char fileName[64];                        // +0x14: the track's .env name
};

// RTTI: KrustyUI : GameObject (vtable 0x0055488c; 0x1364 bytes, the size
// TrackGame slot 4 allocates). Its constructor 0x004987f0 writes the vtable.
// Only the members TrackGame calls are declared.
class KrustyUI : public GameObject {
public:
    explicit KrustyUI(int flags);                                  // 0x004987f0
    virtual ~KrustyUI();                      // 0x0049b470 (deleting wrapper 0x00498880)
    virtual void Disable();                   // 0x00499ad0
    virtual void Enable();                    // 0x00499ac0
    virtual int Tick(float frameTime);                 // 0x00499ae0
    virtual int OnActivate();                 // 0x00499af0
    virtual int OnControlPress(UnknownControlEvent* event, UnknownInputEntry* entry);       // 0x00499a40
    virtual int OnNetworkMessage(int type, void* data, int from, int to, int flags);     // 0x00499a70
    virtual int AttachRenderContext(void* context);  // 0x00499980
    void ShowScene(GameObject* parent); // 0x004999f0: shows the +0x464 scene
    void HideScene();             // 0x00499a20: hides it
    // 0x0049b560 / 0x0049b7f0 (bikerace.cpp passes TrackGame+0x1f2c and
    // +0x1f6c): copy `name` to `text` (`size` bytes) and return 1 when it is
    // an archive entry or a file (in "Res" when it has no directory; then
    // `text` gets that path). Otherwise `text` gets a random bike (+0x50) or
    // rider (+0x58) name and they return 0.
    int ResolveBikeFile(const char* name, char* text, int size);
    int ResolveRiderFile(const char* name, char* text, int size);
    void ShowCursors();             // 0x00499b00
    void HideCursors();             // 0x00499b10
    void OpenExitDialog();             // 0x0049a4a0: opens the exit dialog
    // 0x0049b020: fills `names` with `count` distinct random short strings.
    void MakeRandomNames(const char** names, int count);
    // 0x0049b0d0: picks `count` distinct random bikes and riders for the AI
    // racers (bikes near the player's class).
    void PickRandomRacers(int* bikeChoices, int count, int* riderChoices);
    // 0x0049ba70: appends an entry to +0x60; returns its index.
    int AddEntry(int directoryIndex, int scene, int series, const char* name, int fileTime, int fileSize);
    void Shutdown();             // 0x004999b0: shutdown (the destructor's first step)
    void ClearEntries();             // 0x0049bb80: frees +0x60
    void LoadGarageTables();             // 0x0049a540: loads the garage tables
    void LoadBikesAndRiders();                // 0x0049a8b0: bikes.pb and riders.pb
    GameObject* Create(RenderTarget* target, int value); // 0x004988a0
    void BuildGarageScene(int value);                         // 0x00498cf0
    void OpenMenu(int menu);                                       // 0x00499b20: opens a menu
    void StepProgressBar();                                  // 0x0049b530
    // 0x0049bbb0: records the finished race in the high-score tables
    // (TrackGame+0x3400): table 0, then 1 and 2 by laps (5, 10) or, in
    // modes 0 and 4, by the +0x140 setting (5.0, 10.0).
    void RecordRaceResults();
    void RecordHighScoreTable(int table);                          // 0x0049bc50

    UnknownKrustyUIGui* gui;                  // +0x2c: the GUIManager (constructor 0x00485190)
    int returnMenu;                           // +0x30: DlgProcs.cpp saves currentMenu here (events 0x104/0x105) and reopens it
    int previousMenu;                         // +0x34: OpenMenu saves currentMenu here before opening menu 0xd0
    int field_0x38;                           // reopened by the 0x911 track-record dialog
    int currentMenu;                          // +0x3c: set by OpenMenu
    int replayAvailable;                      // +0x40: BikeRace 0x00420b00 sets it after closing a replay VCR; shows "ButViewReplay"
    int demoEnded;                            // +0x44: BikeRace sets it at race end while playingDemo is set; DemoDlg slot 10 closes on it; cleared by EventManager 0x0045cdc0
    // Lists that LoadBikesAndRiders reads from bikes.pb and riders.pb.
    void* manufacturers;                      // +0x48: "Manufacturer_%02d" entries; freed by the destructor
    int manufacturerCount;                    // +0x4c
    void* bikes;                              // +0x50: each manufacturer's bikes
    int bikeCount;                            // +0x54
    void* riders;                             // +0x58: "Rider_%02d" entries
    int riderCount;                           // +0x5c
    UnknownKrustyUIEntry* entries;            // +0x60: also freed by 0x0049bb80
    int entryCount;                           // +0x64: entries in +0x60
    // Garage tables per bike class (OptionProcs.cpp's OptGarageDlg), read
    // from presets.pb by 0x0049a540 (keys in the comments).
    int standardCurves[5][3][11];             // +0x68: standard curves ("Category_%dPreset_%d" RPM%05d)
    int rpmLowerLimit[5];                     // +0x2fc: "RPMLowerLimit"
    int rpmUpperLimit[5];                     // +0x310: "RPMUpperLimit"
    int bandMaxima[5][11];                    // +0x324: band maxima ("Category_%d" RPM%05d)
    int hpSum[5];                             // +0x400: total of the bands ("HPSum")
    int rpmStep[5];                           // +0x414: RPM step: a tenth of the limits' range
    int weight[5];                            // +0x428: "Weight"
    int minRange[5];                          // +0x43c: slider minimum ("MinRange")
    int maxRange[5];                          // +0x450: slider maximum (the largest band maximum)
    GameObject* garageScene;                  // +0x464: the garage scene (built by 0x00498cf0); released by 0x004999b0
    Camera* garageCamera;                     // +0x468: made current while +0x464 is shown
    UnknownKrustyUIModelObject* riderPreview; // +0x46c: the rider preview (dlgprocs.cpp SPBikeRiderDlg slot 13)
    UnknownKrustyUIModelObject* garageCharacter; // +0x470: the garage character (dlgprocs.cpp SPBikeRiderDlg slot 10)
    Vector3 garageFocus;                      // +0x474: the +0x464 scene's focus (SelectGamePicProcs.cpp)
    unsigned char field_0x480[0x48c - 0x480];
    int profileLoaded;                        // +0x48c: set once a profile is loaded or saved (UiInfo.cpp LoadProfile/SaveProfile)
    UIDialog* progressDialog;                 // +0x490: the dialog holding "ProgressBar" (DlgProcs.cpp sets and clears it)
    void* savedInputContext;                  // +0x494: ImmAssociateContext's result (DlgProcs.cpp), restored to the window (EventManager 0x0045e710)
    int allowIme;                             // +0x498: "AllowIME" registry flag (0x00498cf0)
    int field_0x49c;
    int field_0x4a0;
    int field_0x4a4;
    int playingDemo;                          // +0x4a8: the attract demo plays "ui\demo.rpl" with the credits.dtm dialog (bikerace.cpp 0x00417ed0); LeaveMenusForRace clears it
    int field_0x4ac;
    int userNameDialogOpen;                   // +0x4b0: set while UserNameDlg is open (DlgProcs.cpp)
    unsigned char field_0x4b4[0x634 - 0x4b4];
    UnknownKrustyUISlot field_0x634[8];
    unsigned char field_0x674[0x1364 - 0x674];
};

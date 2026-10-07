#pragma once

#include "DialogProc.h"
#include "KrustyUI.h"

// dlgprocs.cpp (__FILE__ 0x0056963c): the front-end dialogs' message
// procedures (their vtable slot 29), a few other slots and their helpers.
// Names are provisional; the dialog classes are RTTI names.

// 0x0044b0a0 / 0x0044b0e0: qsort orders of list rows by the number in their
// text (ascending / descending).
int CompareRowNumbersAscending(const void* a, const void* b);
int CompareRowNumbersDescending(const void* a, const void* b);

// 0x0044b120: makes the profile selected in "LstProfiles" current; 0 when
// none is selected.
int UseSelectedProfile(UnknownDialogEvent* event);

// 0x0044b1f0: closes `dialog` (CreditsVidDlg passes it as a callback).
void CloseDialogCallback(UIDialog* dialog);

// 0x0044c930 / 0x0044c9f0: label the four "ButStats" buttons from string
// ids / with texts (a zero argument leaves the button).
void SetStatsButtonResources(UnknownDialogEvent* event, int a, int b, int c, int d);
void SetStatsButtonTexts(UnknownDialogEvent* event, const char* a, const char* b, const char* c,
                         const char* d);
// 0x0044ca80: empties the four "LstStats" lists; 0x0044cae0 adds a row to
// each (a zero argument skips the list).
void ClearStatsLists(UnknownDialogEvent* event);
void AddStatsRows(UnknownDialogEvent* event, const char* a, const char* b, const char* c,
                  const char* d);

// RTTI: MainDlg : UIDialog (vtable 0x00554fb8; 0x7f80 bytes, the size
// KrustyUI 0x00499b20 allocates).
class MainDlg : public UIDialog {
public:
    MainDlg() : UIDialog(1, "mainmenu.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x0044b200
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0044c6b0
    void FillProfileList();             // 0x0044c7a0: fills "LstProfiles"
    void ShowCurrentProfile();             // 0x0044c8c0: shows the current profile

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
    UIVideoStatic* movieControl;              // +0x7f58: the movie control
    unsigned char field_0x7f5c[0x7f60 - 0x7f5c];
    UIControl* menuSingleAnimation; // +0x7f60: "MenuSingleAnimation"
    UIControl* menuMultiAnimation; // +0x7f64: "MenuMultiAnimation"
    UIControl* menuUserAnimation;  // +0x7f68: "MenuUserAnimation"
    int field_0x7f6c;
    int field_0x7f70;
    int field_0x7f74;
    int field_0x7f78;
    float field_0x7f7c;
};

// RTTI: ChooseTCPMethodDlg : UIDialog (vtable 0x00551884).
class ChooseTCPMethodDlg : public UIDialog {
public:
    ChooseTCPMethodDlg() : UIDialog(1, "messbox2.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x0044cb80

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: NewbieDlg : UIDialog (vtable 0x00551c2c; 0x7f58 bytes).
class NewbieDlg : public UIDialog {
public:
    NewbieDlg() : UIDialog(1, "messbox2.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x0044ccf0

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: NoDelCurProfileDlg : UIDialog (vtable 0x0055198c).
class NoDelCurProfileDlg : public UIDialog {
public:
    NoDelCurProfileDlg() : UIDialog(1, "messbox2.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x0044ce10

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

class BackgroundImage;
class SPEventDlg;
class SPBikeRiderDlg;
class SPRaceInfoDlg;

// The texture a bike or rider model object holds at +0x1a0 (0x00444c70
// names it from `name`).
class UnknownModelTexture {
public:
    void UnknownFunction444c70(int a, const char* name, TextureMapManager** textures);
};

// RTTI: SinglePlayerDlg : UIDialog (vtable 0x00551a94; 0x7f64 bytes, the
// size KrustyUI 0x00499b20 allocates). It hosts three pages.
class SinglePlayerDlg : public UIDialog {
public:
    SinglePlayerDlg() : UIDialog(1, "SPBase.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x0044cf20
    void OpenPage(int page);     // 0x0044d740: opens page `page` (0..2)

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0xc4 - 0x34];
    int openingMenu;                          // +0xc4: 0x88e when opened on the race info page
    unsigned char field_0xc8[0x7f58 - 0xc8];
    SPEventDlg* field_0x7f58;
    SPBikeRiderDlg* field_0x7f5c;
    SPRaceInfoDlg* field_0x7f60;
};

struct UnknownRecordFileHeader;

// RTTI: SPEventDlg : UIDialog (vtable 0x00551d34; 0x7f68 bytes).
class SPEventDlg : public UIDialog {
public:
    SPEventDlg() : UIDialog(1, "SPEvent.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x0044d950
    void ShowRaceModeControls();             // 0x0044dfc0: shows the controls for the race mode
    void ApplyEventType();             // 0x0044e3e0: applies the chosen event type
    void UnknownVirtualSlot31(int apply);     // 0x0044e6c0: stores (or shows) the event settings
    void ListGhosts();             // 0x0044eb40: lists the ghosts for the track
    int AddGhost(const char* path); // 0x0044ec50: adds a ghost; 0 when unusable
    void ForgetGhosts();             // 0x0044eed0: forgets the ghosts

    unsigned char field_0x2c[0x34 - 0x2c];
    GUIUser* guiUser;                         // +0x34
    unsigned char field_0x38[0x7f58 - 0x38];
    UnknownRecordFileHeader* ghostHeaders;    // +0x7f58: the ghost files' headers
    int ghostHeaderCount;                     // +0x7f5c: their count
    char** ghostPaths;                        // +0x7f60: the ghost files' paths
    int ghostPathCount;                       // +0x7f64: their count
};

// RTTI: SPBikeRiderDlg : UIDialog (vtable 0x00551db8; 0x7f9c bytes).
class SPBikeRiderDlg : public UIDialog {
public:
    SPBikeRiderDlg() : UIDialog(1, "SPBikeR.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x0044ef70
    // 0x0044fab0 / 0x0044fa00 (a near miss): MPBikeRiderDlg's and
    // PCCentralBikeRiderDlg's slot 22 and PCCentralBikeRiderDlg's slot 23
    // share these bodies.
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry);
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0044fae0: turns the bike view
    virtual int UnknownVirtualSlot13();       // 0x0044ffc0: names the chosen bike's and rider's textures
    void UnknownVirtualSlot31(int apply);     // 0x004505d0: stores the chosen bike and rider
    void FillBikeRiderLists();             // 0x0044f750: fills the bike and rider lists (a near miss)
    void PaintPlateNumber(int number);   // 0x0044f950: paints the plate number on every bike
    // 0x004500d0: MPBikeRiderDlg's identical method is folded into it.
    void ApplyChosenRider();
    void UnknownFunction4500e0();             // 0x004500e0

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    GUIUser* guiUser;                         // +0x34
    unsigned char field_0x38[0x110 - 0x38];
    BackgroundImage* dialogBackground;        // +0x110
    unsigned char field_0x114[0x7f58 - 0x114];
    Vector3 viewEye;                          // +0x7f58: the bike view's eye
    Vector3 viewTarget;                       // +0x7f64: and target
    float viewDistance;                       // +0x7f70: their distance
    int previewRegion;                        // +0x7f74: background region
    int previewDragged;                       // +0x7f78: the bike preview was dragged
    int previewDragging;                      // +0x7f7c: the bike preview is being dragged
    int idleMotionPlaying;                    // +0x7f80: the rider's idle motion is playing
    int bikeChanged;                          // +0x7f84: the bike changed
    int riderChanged;                         // +0x7f88: the rider changed
    CameraRect previewArea;                   // +0x7f8c: the bike preview's area
};

// RTTI: SPRaceInfoDlg : UIDialog (vtable 0x00551e3c; 0x7f58 bytes).
class SPRaceInfoDlg : public UIDialog {
public:
    SPRaceInfoDlg() : UIDialog(1, "SPRInfo.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004507a0

    SinglePlayerDlg* parentDialog;            // +0x2c: parent dialog
    unsigned char field_0x30[0x7f58 - 0x30];
};

// RTTI: CreditsVidDlg : UIDialog (vtable 0x005516f8).
class CreditsVidDlg : public UIDialog {
public:
    CreditsVidDlg() : UIDialog(1, "credits.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00453170

    unsigned char field_0x2c[0x7f58 - 0x2c];
    UIVideoStatic* field_0x7f58;
};

// 0x0059ae00: set when the player skips the intro (or the intro movie is
// turned off).
extern int g_UnknownGlobal59ae00;

// RTTI: Intro2Dlg : UIDialog (vtable 0x00552260; 0x7f64 bytes).
class Intro2Dlg : public UIDialog {
public:
    Intro2Dlg() : UIDialog(1, "intro2.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00453430
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x00453570
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004535f0

    unsigned char field_0x2c[0x7f58 - 0x2c];
    int framesShown;                          // +0x7f58: frames shown
    float secondsShown;                       // +0x7f5c: seconds shown
    int frameRan;                             // +0x7f60: set once a frame has run
};

// RTTI: Intro3Dlg : UIDialog (vtable 0x005522e4; 0x7f60 bytes).
class Intro3Dlg : public UIDialog {
public:
    Intro3Dlg() : UIDialog(1, "intro3.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00453610
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x00453670
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004536c0

    unsigned char field_0x2c[0x7f58 - 0x2c];
    float secondsShown;                       // +0x7f58: seconds shown
    int frameRan;                             // +0x7f5c: set once a frame has run
};

// RTTI: EditBoxDlg : UIDialog (vtable 0x00552470; 0x7f60 bytes).
class EditBoxDlg : public UIDialog {
public:
    EditBoxDlg() : UIDialog(1, "messbox1.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00455840
    // 0x004558f0: sets the title and prompt and edits `buffer` (`size` bytes).
    void Edit(const char* title, const char* prompt, char* buffer, int size);
    // 0x00455970: the same with string resource ids (used when nonzero).
    void EditWithResources(const char* title, int titleId, const char* prompt, int promptId, char* buffer,
                           int size);

    unsigned char field_0x2c[0x34 - 0x2c];
    GUIUser* guiUser;                         // +0x34
    unsigned char field_0x38[0x7f58 - 0x38];
    char* editBuffer;                         // +0x7f58: the edited text
    int editBufferSize;                       // +0x7f5c: its size
};

// RTTI: DemoDlg : UIDialog (vtable 0x00550e18; 0x7f58 bytes).
class DemoDlg : public UIDialog {
public:
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00455a10
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x00455b20
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00455ae0

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* guiManager;                   // +0x30
};

// The image a dialog resource names (UIDialog 0x0046e9a0 returns its entry).
class UnknownDialogImage {
public:
    int GetCurrentTexture();              // 0x00472f90: the current frame's texture
};

// 0x0059adfc: frames the loading dialog has waited; 0x0059ae84: set once
// the race is loaded; 0x0059ae88: cleared with them.
extern int g_UnknownGlobal59adfc;
extern int g_UnknownGlobal59ae84;
extern int g_UnknownGlobal59ae88;

// The 8-byte message 0xcc a network player sends when its race fails to
// load.
struct UnknownLoadFailedMessage {
    int field_0x00;
    int field_0x04;                           // the sender's player id
};

// RTTI: LoadingDlg : UIDialog (vtable 0x00554f34; 0x7f5c bytes): loads the
// race while it is shown.
class LoadingDlg : public UIDialog {
public:
    explicit LoadingDlg(int flags) : UIDialog(flags, "loading.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00451020
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x00451270

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x38 - 0x34];
    char field_0x38[0x110 - 0x38];            // the dialog resource (UIDialog's constructor copies it)
    unsigned char field_0x110[0x7f58 - 0x110];
    int framesShown;                          // +0x7f58: frames shown
};

// The loading screens per event type; each only names its own resource.
class LoadSupercrossDlg : public LoadingDlg {
public:
    explicit LoadSupercrossDlg(int flags);    // 0x00450e30 (vtable 0x00551ec4)
};
class LoadEnduroDlg : public LoadingDlg {
public:
    explicit LoadEnduroDlg(int flags);        // 0x00450e80 (vtable 0x00551f48)
};
class LoadQuarryDlg : public LoadingDlg {
public:
    explicit LoadQuarryDlg(int flags);        // 0x00450ed0 (vtable 0x00551fcc)
};
class LoadNationalsDlg : public LoadingDlg {
public:
    explicit LoadNationalsDlg(int flags);     // 0x00450f20 (vtable 0x00552050)
};
class LoadTagDlg : public LoadingDlg {
public:
    explicit LoadTagDlg(int flags);           // 0x00450f70 (vtable 0x005520d4)
};
class LoadBajaDlg : public LoadingDlg {
public:
    explicit LoadBajaDlg(int flags);          // 0x00450fd0 (vtable 0x00552158)
};

// The DirectPlay serial connection (DPCOMPORTADDRESS's layout) that
// 0x004ae410 (between Net.cpp and NetProcs.cpp) fills from SerialPopupDlg's
// selections.
struct UnknownComPortAddress {
    int port;                                 // +0x00: port
    int baudRate;                             // +0x04: baud rate
    int stopBits;                             // +0x08: stop bits
    int parity;                               // +0x0c: parity
    int flowControl;                          // +0x10: flow control
};
void UnknownFunction4ae410(struct UnknownSerialSettings* settings, UnknownComPortAddress* address);

// 0x00451b80: creates the profile named in "EditBox" (or reports that it
// exists).
void CreateProfile(UnknownDialogEvent* event);

// RTTI: UserNameDlg : UIDialog (vtable 0x00554988): asks for the profile name.
class UserNameDlg : public UIDialog {
public:
    UserNameDlg() : UIDialog(1, "messbox1.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00451ff0

    unsigned char field_0x2c[0x34 - 0x2c];
    GUIUser* guiUser;                         // +0x34
    unsigned char field_0x38[0x7f58 - 0x38];
};

// RTTI: ProfileExistsDlg : UIDialog (vtable 0x005521dc; 0x7f58 bytes).
class ProfileExistsDlg : public UIDialog {
public:
    ProfileExistsDlg() : UIDialog(1, "messbox2.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00452370

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: RemoveProfileDlg : UIDialog (vtable 0x00551908).
class RemoveProfileDlg : public UIDialog {
public:
    RemoveProfileDlg() : UIDialog(1, "messbox2.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004524f0

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// cdecl 0x0047b570 (gameui.cpp): resizes a DebugMalloc'd block (also
// declared in ProCircuitProcs.h).
void* UnknownFunction47b570(void* block, unsigned int size);

// A ghost or replay file's 0xeb0-byte header, as GhostFilesDlg and
// ReplayFilesDlg read it (0x00454640). +0x34..+0x220 is a copy of the race
// settings at TrackGame+0x2d70 (KrustyUI+0x4e8 keeps another copy, whose
// eight records at +0x14c are KrustyUI+0x634).
struct UnknownRecordFileHeader {
    unsigned char field_0x00[0x10];
    char description[0x20];                   // +0x10: description
    float lengthSeconds;                      // +0x30: length in seconds
    int eventType;                            // +0x34: the event type (TrackGame+0x2d70)
    int raceKind;                             // +0x38: the race kind (TrackGame+0x2d74)
    int series;                               // +0x3c: the series (TrackGame+0x2d78)
    unsigned char field_0x40[0x54 - 0x40];
    int laps;                                 // +0x54: laps (TrackGame+0x2d90)
    unsigned char field_0x58[0x60 - 0x58];
    unsigned char field_0x60;
    unsigned char field_0x61[0x64 - 0x61];
    int eventNumber;                          // +0x64: the event number shown with the track
    unsigned char trackIndex;                 // +0x68: the track index (TrackGame+0x2da4)
    unsigned char field_0x69;
    char trackName[0x180 - 0x6a];             // +0x6a: the track (TrackGame+0x2da6)
    UnknownKrustyUISlot field_0x180[8];       // TrackGame+0x2ebc
    unsigned char field_0x1c0[0x229 - 0x1c0];
    char riderName[0x33c - 0x229];            // +0x229: the rider
    int field_0x33c;                          // copied to TrackGame+0x1550 (ReplayFilesDlg)
    int field_0x340;
    unsigned char field_0x344[0xeb0 - 0x344];
};

class GhostFilesDlg;
class ReplayFilesDlg;

// RTTI: GhostReplayDlg : UIDialog (vtable 0x00551800): hosts the ghost and
// replay file pages.
class GhostReplayDlg : public UIDialog {
public:
    GhostReplayDlg() : UIDialog(1, "GRBase.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00453b20
    void OpenPage(int page);     // 0x00453e90: opens page `page` (0 ghosts, 1 replays)

    unsigned char field_0x2c[0x7f58 - 0x2c];
    GhostFilesDlg* field_0x7f58;
    ReplayFilesDlg* field_0x7f5c;
};

// RTTI: GhostFilesDlg : UIDialog (vtable 0x00552368; 0x7f88 bytes).
class GhostFilesDlg : public UIDialog {
public:
    GhostFilesDlg() : UIDialog(1, "GhostFil.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00454030
    void ListGhostFiles();             // 0x00454470: lists the ghost files
    int AddGhostFile(const char* path); // 0x00454640: adds a ghost file; 0 when unusable
    void RaceSelectedGhost();             // 0x00454970: races the selected ghost

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
    UnknownRecordFileHeader* fileHeaders;     // +0x7f58: the files' headers
    int fileHeaderCount;                      // +0x7f5c: their count
    char** filePaths;                         // +0x7f60: the files' paths
    int filePathCount;                        // +0x7f64: their count
    char editedDescription[0x20];             // +0x7f68: the description being edited
};

// RTTI: ReplayFilesDlg : UIDialog (vtable 0x005523ec; 0x7f88 bytes).
class ReplayFilesDlg : public UIDialog {
public:
    ReplayFilesDlg() : UIDialog(1, "ReplyFil.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00454a50
    void ListReplayFiles();             // 0x00454e60: lists the replay files
    int AddReplayFile(const char* path); // 0x00455040: adds a replay file; 0 when unusable
    void PlaySelectedReplay();             // 0x00455490: plays the selected replay

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
    UnknownRecordFileHeader* field_0x7f58;
    int field_0x7f5c;
    char** field_0x7f60;
    int field_0x7f64;
    char field_0x7f68[0x20];
};

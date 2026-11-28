#pragma once

#include "DialogProc.h"

// OptionProcs.cpp (__FILE__ 0x0056f57c, line references 0x004b4734..
// 0x004b5119): the options dialogs' message procedures (vtable slot 29,
// one DialogEvent) and their slot 31 (one flag: 1 reads the controls
// back into the settings, 0 shows the settings). The classes are RTTI names;
// every one derives directly from UIDialog. Member names are provisional.

struct TrackGameInterfaceOptions;
struct TrackGameSoundOptions;
struct TrackGameGraphicsOptions;
struct TrackGameControlOptions;
struct TrackGameGarageOptions;

// The copies the options dialogs edit (TrackGame.h's settings blocks; the
// sizes are OptionsDlg's rep movsd counts): OptionsDlg's init (0x004b4280)
// copies TrackGame's garage, game, sound, graphics and control settings and
// its Message0..9 texts into them, its close copies them back, and the pages
// bind their controls to them. 0x00688898 (game), 0x00688798 (sound),
// 0x006887d8 (graphics), 0x00688868 (controls), 0x00688808 (garage) and
// 0x00688c10 (messages). HandleMenuOk (0x00452930) also commits the game,
// sound and graphics copies when their page's OK is pressed.
extern TrackGameInterfaceOptions g_EditedGameSettings;
extern TrackGameSoundOptions g_EditedSoundSettings;
extern TrackGameGraphicsOptions g_EditedGraphicsSettings;
extern TrackGameControlOptions g_EditedControlSettings;
extern TrackGameGarageOptions g_EditedGarageSettings;
extern char g_EditedMessages[10][128]; // Message0..9 (TrackGame+0x19d4)

// 0x0056cb6c: the engine sizes (cc) of the five bike classes, 125, 250,
// 400, 500 and 600 in retail .data (the "125cc 2-stroke" .. "600cc
// 4-stroke" rows); initialised data shared with other files and read by
// the inline class lookup below.
extern int g_BikeClassEngineSizes[5];

// The bike class of an engine size. Inline: the same loop over
// 0x0056cb6c appears in many files (0x00419bfe, 0x0044f00c, ...).
inline int BikeClassOf(int cc)
{
    if (cc <= g_BikeClassEngineSizes[0])
        return 0;
    if (cc >= g_BikeClassEngineSizes[4])
        return 4;
    for (int i = 1; i < 4; i++) {
        if (cc > g_BikeClassEngineSizes[i - 1] && cc < g_BikeClassEngineSizes[i + 1])
            return i;
    }
    return 1;
}

// Every class declares the dialog slots 27-31 (0x6c..0x7c); only 29 (the
// message procedure) and 31 have signatures. The constructors are inline:
// OptionsDlg (0x004b4280, 0x004b4ab0) and GlobalSettingsDlg allocate the
// dialogs and write their vtables after UIDialog's constructor.

// RTTI: OptGameSettingsDlg : UIDialog (vtable 0x0055591c; 0x7f58 bytes).
class OptGameSettingsDlg : public UIDialog {
public:
    OptGameSettingsDlg() : UIDialog(1, "OptGame.dtm") {}
    virtual void HandleEvent(DialogEvent* event);   // 0x004b2000
    virtual void SyncValues(int save);                    // 0x004b22d0
};

// RTTI: OptGraphicsDlg : UIDialog (vtable 0x00555898; 0x7f58 bytes).
class OptGraphicsDlg : public UIDialog {
public:
    OptGraphicsDlg() : UIDialog(1, "OptGraph.dtm") {}
    virtual void HandleEvent(DialogEvent* event);   // 0x004b23b0
    virtual void SyncValues(int save);                    // 0x004b2670
};

// RTTI: OptAdvancedGraphicsDlg : UIDialog (vtable 0x00555580; 0x7f58
// bytes; slot 31 is UIDialog's).
class OptAdvancedGraphicsDlg : public UIDialog {
public:
    OptAdvancedGraphicsDlg() : UIDialog(1, "OptAdvG.dtm") {}
    virtual void HandleEvent(DialogEvent* event);   // 0x004b26c0
    // Slot 31 inherits UIDialog::SyncValues (0x00470050).
};

// RTTI: OptSoundDlg : UIDialog (vtable 0x00555814; 0x7f58 bytes).
class OptSoundDlg : public UIDialog {
public:
    OptSoundDlg() : UIDialog(1, "OptSound.dtm") {}
    virtual void HandleEvent(DialogEvent* event);   // 0x004b2b20
    virtual void SyncValues(int save);                    // 0x004b2ce0
    void ApplySoundSettings();             // 0x004b2d40: applies the sound settings
};

// 0x004b5570 (cdecl): the text of input `code` of `kind` for list row
// `row` (empty, and 0, when a row below 4 is given a kind other than 0).
int GetInputText(int row, int kind, int code, char* text);

// RTTI: OptControlsDlg : UIDialog (vtable 0x00555790; 0x7f7c bytes). While +0x7f70 is
// set the dialog waits for a key, button or axis to map (its slots 22 and
// 23 take the input).
class OptControlsDlg : public UIDialog {
public:
    OptControlsDlg() : UIDialog(1, "OptCont.dtm") {}
    virtual int OnControlRelease(ControlEvent* event, InputButtonState* entry);     // 0x004b5600
    virtual int OnControlPress(ControlEvent* event, InputButtonState* entry);       // 0x004b55d0
    virtual void HandleEvent(DialogEvent* event);   // 0x004b2e00
    virtual void SyncValues(int save);                    // 0x004b3470
    void ListMappedKeys();             // 0x004b3410: lists the mapped keys
    void ColorListRows();             // 0x004b54c0: colours the list rows
    void MapMovedInput();             // 0x004b5760: maps a moved axis or mouse direction
    void EndInputWait();             // 0x004b5a20: ends the wait for input

    float axisValuesAtWait[6];                // +0x7f58: joystick axis values when the wait began
    int waitingForInput;                      // +0x7f70: waiting for input to map
    int inputWaitPending;                     // +0x7f74: the 250 ms timer before the input wait runs
    int joystickIsKind3;                      // +0x7f78: the active joystick has device kind 3
};

// RTTI: OptGarageDlg : UIDialog (vtable 0x00555688; 0x7f5c bytes).
class OptGarageDlg : public UIDialog {
public:
    OptGarageDlg() : UIDialog(1, "OptGar.dtm") {}
    virtual void HandleEvent(DialogEvent* event);   // 0x004b3500
    virtual void SyncValues(int save);                    // 0x004b3e70
    void FillCurveList();             // 0x004b38f0: fills DDLCurves
    void EqSliderMoved(UIControl* slider); // 0x004b3aa0: an SldEQ slider moved
    void ShowCurve();             // 0x004b3bc0: shows the curve

    int curveSum;                             // +0x7f58: sum of the curve values shown
};

// RTTI: OptMessagesDlg : UIDialog (vtable 0x0055570c; 0x7f58 bytes; slot
// 31 is UIDialog's).
class OptMessagesDlg : public UIDialog {
public:
    OptMessagesDlg() : UIDialog(1, "OptMess.dtm") {}
    virtual void HandleEvent(DialogEvent* event);   // 0x004b40a0
    // Slot 31 inherits UIDialog::SyncValues (0x00470050).
};

// The 0x94-byte records at KrustyUI+0x50 (+0x54 counts them) as OptionsDlg
// reads them.
struct OptionsBikeRecord {
    unsigned char field_0x00[0x8c];
    int engineSize;                           // +0x8c: engine size (cc), as KrustyUI's bike table
    unsigned char field_0x90[0x94 - 0x90];
};

// RTTI: OptionsDlg : UIDialog (vtable 0x00551b18): the options pages. Its
// +0x7f58..+0x7f6c are the open page dialogs.
class OptionsDlg : public UIDialog {
public:
    OptionsDlg() : UIDialog(1, "OptBase.dtm") {} // SelectGamePicProcs.cpp 0x004f8047 allocates 0x7f70 bytes
    virtual void HandleEvent(DialogEvent* event);   // 0x004b4280
    // Slot 31 inherits UIDialog::SyncValues (0x00470050).
    void OpenPage(int page);     // 0x004b4ab0: opens page `page` (0..5)

    OptGameSettingsDlg* gameSettingsPage;     // +0x7f58
    OptGraphicsDlg* graphicsPage;             // +0x7f5c
    OptSoundDlg* soundPage;                   // +0x7f60
    OptControlsDlg* controlsPage;             // +0x7f64
    OptMessagesDlg* messagesPage;             // +0x7f68
    OptGarageDlg* garagePage;                 // +0x7f6c
};

// RTTI: GlobalSettingsDlg : UIDialog (vtable 0x00555604; 0x7f58 bytes):
// restores the player profile.
class GlobalSettingsDlg : public UIDialog {
public:
    GlobalSettingsDlg() : UIDialog(1, "GlobSet.dtm") {}
    virtual void HandleEvent(DialogEvent* event);   // 0x004b4e70
    // Slot 31 inherits UIDialog::SyncValues (0x00470050).
    void ListOtherProfiles();             // 0x004b51a0: lists the other profiles
};

// RTTI: ConfirmRestoreDlg : UIDialog (vtable 0x005559a0; 0x7f58 bytes).
class ConfirmRestoreDlg : public UIDialog {
public:
    ConfirmRestoreDlg() : UIDialog(1, "messbox2.dtm") {}
    virtual void HandleEvent(DialogEvent* event);   // 0x004b5380
    // Slot 31 inherits UIDialog::SyncValues (0x00470050).
};

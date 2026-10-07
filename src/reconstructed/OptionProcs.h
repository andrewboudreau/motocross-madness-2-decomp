#pragma once

#include "DialogProc.h"

// OptionProcs.cpp (__FILE__ 0x0056f57c, line references 0x004b4734..
// 0x004b5119): the options dialogs' message procedures (vtable slot 29,
// one UnknownDialogEvent) and their slot 31 (one flag: 1 reads the controls
// back into the settings, 0 shows the settings). The classes are RTTI names;
// every one derives directly from UIDialog. Member names are provisional.

// The settings blocks. OptionsDlg (0x004b4280) copies each from the global
// game object (TrackGame +0xc24, +0xf98, +0xfc4, +0xfe0, +0x1550 and
// +0x19d4; the sizes are its rep movsd counts) into this file's globals when
// the options open, and back when they are accepted; the controls are bound
// to the copies (their slot 54 keeps a pointer). Field roles are the control
// names.
struct UnknownOptGameSettings {               // 0x374 bytes (TrackGame+0xc24)
    int showGauges;                           // +0x00: GuagesCheckBox
    int showRaceStats;                        // +0x04: RaceStatsCheckBox
    int chatMode;                             // +0x08: MultChatMode state (0..2)
    int showOverview;                         // +0x0c: OverviewCheckBox
    int showRiderNames;                       // +0x10: RiderNamesCheckBox
    int showRiderPositions;                   // +0x14: RiderPositionCheckBox
    int showPodiums;                          // +0x18: ChkPodiums
    int uiAnimations;                         // +0x1c: ChkUIAnims
    int introMovie;                           // +0x20: ChkIntroMovie
    int playerIndicator;                      // +0x24: ChkPlayerIndicator
    int toolTips;                             // +0x28: ChkToolTips
    int wreckResetOnTrack;                    // +0x2c: ChkWreckResetOnTrack
    unsigned char field_0x30[0x360 - 0x30];
    int curveRows[5];                         // +0x360: chosen DDLCurves row per bike class
};

struct UnknownOptSoundSettings {              // 0x2c bytes (TrackGame+0xf98)
    int field_0x00;
    int uiSounds;                             // +0x04: UserInterfaceSoundsCheckBox
    int inGameSoundEffects;                   // +0x08: InGameSoundEffectsCheck
    int field_0x0c;
    int soundHardware;                        // +0x10: SoundHardwareCheckBox
    int eax;                                  // +0x14: ChkEAX
    int waypointChime;                        // +0x18: ChkWaypointChime
    int uiVolume;                             // +0x1c: UserInterfaceVolumeSlider
    int inGameVolume;                         // +0x20: InGameVolumeSlider
    int field_0x24;
    int sound16Bit;                           // +0x28: 16-bit sound (else 8-bit)
};

struct UnknownOptGraphicsSettings {           // 0x1c bytes (TrackGame+0xfc4)
    int resolutionRow;                        // +0x00: display resolution row
    int shadows;                              // +0x04: ShadowsCheckBox
    int particles;                            // +0x08: ParticlesCheckBox
    int field_0x0c;
    int sky;                                  // +0x10: SkyCheckBox
    int field_0x14;
    int terrainQuality;                       // +0x18: TerrainQualitySliderBar
};

struct UnknownOptControlSettings {            // 0x30 bytes (TrackGame+0xfe0)
    int steering;                             // +0x00: SteeringSlider
    int crossup;                              // +0x04: CrossupSlider
    int pitch;                                // +0x08: PitchSlider
    int pressThrottle;                        // +0x0c: PressThrottleSlider
    int pressBrakes;                          // +0x10: PressBrakesSlider
    int releaseThrottle;                      // +0x14: ReleaseThrottleSlider
    int releaseBrakes;                        // +0x18: ReleaseBrakesSlider
    int feedback;                             // +0x1c: FeedbackSlider
    int forceFeedback;                        // +0x20: ForceFeedCheck
    int gasGyro;                              // +0x24: ChkGasGyro
    int brakeGyro;                            // +0x28: ChkBrakeGyro
    int analogAxes;                           // +0x2c: ChkAnalogAxes
};

struct UnknownOptGarageSettings {             // 0x5c bytes (TrackGame+0x1550)
    int engineSize;                           // +0x00: engine size in cc ("%dcc")
    int field_0x04;
    int field_0x08;
    float suspension[6];                      // +0x0c: suspension sliders (SldFrontComp ...)
    int eqBands[11];                          // +0x24: SldEQ1..11
    int firstBand;                            // +0x50: first band ("%2.1fk" after * 0.001)
    int field_0x54;
    int bandStep;                             // +0x58: band step
};

// 0x00688898, 0x00688798, 0x006887d8, 0x00688868, 0x00688808 and
// 0x00688c10. The first is also read at 0x00452980, the second and third
// from dlgprocs.cpp's code.
extern UnknownOptGameSettings g_UnknownGlobal688898;
extern UnknownOptSoundSettings g_UnknownGlobal688798;
extern UnknownOptGraphicsSettings g_UnknownGlobal6887d8;
extern UnknownOptControlSettings g_UnknownGlobal688868;
extern UnknownOptGarageSettings g_UnknownGlobal688808;
extern char g_UnknownGlobal688c10[10][128]; // Message0..9 (TrackGame+0x19d4)

// 0x0056cb6c: the engine sizes (cc) of the five bike classes (initialised
// data shared with other files; read by the inline class lookup below).
extern int g_UnknownGlobal56cb6c[5];

// The bike class of an engine size. Inline: the same loop over
// 0x0056cb6c appears in many files (0x00419bfe, 0x0044f00c, ...).
inline int UnknownBikeClassOf(int cc)
{
    if (cc <= g_UnknownGlobal56cb6c[0])
        return 0;
    if (cc >= g_UnknownGlobal56cb6c[4])
        return 4;
    for (int i = 1; i < 4; i++) {
        if (cc > g_UnknownGlobal56cb6c[i - 1] && cc < g_UnknownGlobal56cb6c[i + 1])
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
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b2000
    virtual void UnknownVirtualSlot31(int save);                    // 0x004b22d0
};

// RTTI: OptGraphicsDlg : UIDialog (vtable 0x00555898; 0x7f58 bytes).
class OptGraphicsDlg : public UIDialog {
public:
    OptGraphicsDlg() : UIDialog(1, "OptGraph.dtm") {}
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b23b0
    virtual void UnknownVirtualSlot31(int save);                    // 0x004b2670
};

// RTTI: OptAdvancedGraphicsDlg : UIDialog (vtable 0x00555580; 0x7f58
// bytes; slot 31 is UIDialog's).
class OptAdvancedGraphicsDlg : public UIDialog {
public:
    OptAdvancedGraphicsDlg() : UIDialog(1, "OptAdvG.dtm") {}
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b26c0
    virtual void UnknownVirtualSlot31(int save);
};

// RTTI: OptSoundDlg : UIDialog (vtable 0x00555814; 0x7f58 bytes).
class OptSoundDlg : public UIDialog {
public:
    OptSoundDlg() : UIDialog(1, "OptSound.dtm") {}
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b2b20
    virtual void UnknownVirtualSlot31(int save);                    // 0x004b2ce0
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
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004b5600 (near miss in samples/ui)
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x004b55d0
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b2e00
    virtual void UnknownVirtualSlot31(int save);                    // 0x004b3470
    void ListMappedKeys();             // 0x004b3410: lists the mapped keys
    void ColorListRows();             // 0x004b54c0: colours the list rows
    void MapMovedInput();             // 0x004b5760: maps a moved axis or mouse direction (near miss in samples/ui)
    void EndInputWait();             // 0x004b5a20: ends the wait for input

    float axisValuesAtWait[6];                // +0x7f58: joystick axis values when the wait began
    int waitingForInput;                      // +0x7f70: waiting for input to map
    int field_0x7f74;
    int field_0x7f78;                         // the active joystick has device kind 3
};

// RTTI: OptGarageDlg : UIDialog (vtable 0x00555688; 0x7f5c bytes).
class OptGarageDlg : public UIDialog {
public:
    OptGarageDlg() : UIDialog(1, "OptGar.dtm") {}
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b3500
    virtual void UnknownVirtualSlot31(int save);                    // 0x004b3e70
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
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b40a0
    virtual void UnknownVirtualSlot31(int save);
};

// The 0x94-byte records at KrustyUI+0x50 (+0x54 counts them) as OptionsDlg
// reads them.
struct UnknownOptKrustyUIRecord {
    unsigned char field_0x00[0x8c];
    int field_0x8c;
    unsigned char field_0x90[0x94 - 0x90];
};

// RTTI: OptionsDlg : UIDialog (vtable 0x00551b18): the options pages. Its
// +0x7f58..+0x7f6c are the open page dialogs.
class OptionsDlg : public UIDialog {
public:
    OptionsDlg() : UIDialog(1, "OptBase.dtm") {} // SelectGamePicProcs.cpp 0x004f8047 allocates 0x7f70 bytes
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b4280
    virtual void UnknownVirtualSlot31(int save);
    void OpenPage(int page);     // 0x004b4ab0: opens page `page` (0..5)

    OptGameSettingsDlg* field_0x7f58;
    OptGraphicsDlg* field_0x7f5c;
    OptSoundDlg* field_0x7f60;
    OptControlsDlg* field_0x7f64;
    OptMessagesDlg* field_0x7f68;
    OptGarageDlg* field_0x7f6c;
};

// RTTI: GlobalSettingsDlg : UIDialog (vtable 0x00555604; 0x7f58 bytes):
// restores the player profile.
class GlobalSettingsDlg : public UIDialog {
public:
    GlobalSettingsDlg() : UIDialog(1, "GlobSet.dtm") {}
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b4e70
    virtual void UnknownVirtualSlot31(int save);
    void ListOtherProfiles();             // 0x004b51a0: lists the other profiles
};

// RTTI: ConfirmRestoreDlg : UIDialog (vtable 0x005559a0; 0x7f58 bytes).
class ConfirmRestoreDlg : public UIDialog {
public:
    ConfirmRestoreDlg() : UIDialog(1, "messbox2.dtm") {}
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event);   // 0x004b5380
    virtual void UnknownVirtualSlot31(int save);
};

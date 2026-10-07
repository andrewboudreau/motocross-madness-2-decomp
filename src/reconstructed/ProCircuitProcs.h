#pragma once

#include "DialogProc.h"
#include "MatrixUtil.h"
#include "ProCircuit.h"

class BackgroundImage;

// ProCircuitProcs.cpp (D:\aardvark\VC\krusty2\ProCircuitProcs.cpp): the pro
// circuit dialogs' message procedures (their vtable slot 29) and helpers.
// The dialog classes are RTTI names (all : UIDialog); members, sizes and
// method names are provisional. PCStartupDlg, PCCentralDlg and PCLastRaceDlg
// have their type descriptors in other files' data; the others in this
// file's.

// cdecl 0x0047b570 (gameui.cpp): resizes a DebugMalloc'd block (also
// declared by KrustyUI.cpp).
void* UnknownFunction47b570(void* block, unsigned int size);

// dlgprocs.cpp's 0x0044b0a0 / 0x0044b0e0 (declared in DlgProcs.h too):
// qsort orders of list rows by their number, ascending / descending.
int CompareRowNumbersAscending(const void* a, const void* b);
int CompareRowNumbersDescending(const void* a, const void* b);

// 0x004d4b20 / 0x004d4b60: qsort orders of list rows by the number after
// their first character ("$%d" cash; ascending / descending).
int CompareCashAscending(const void* a, const void* b);
int CompareCashDescending(const void* a, const void* b);

// 0x004d4ba0: opens the dialog the career's state calls for (KrustyUI
// 0x0049a105 calls it too).
void OpenCareerDialog();

// A view of EventManager's 0x50-byte race entries (EventManager.h's
// UnknownEventEntry) as the results code reads them: +0x34..+0x3c are loaded
// as floats.
struct UnknownProCircuitResult {
    int racer;                                // +0x00: the circuit racer
    int position;                             // +0x04: finishing position
    unsigned char field_0x08[0x28 - 0x08];
    int points;                               // +0x28: points
    unsigned char field_0x2c[0x34 - 0x2c];
    float repairs;                            // +0x34: repairs (times 500 is the cost)
    float medical;                            // +0x38: medical (times 250)
    float stunts;                             // +0x3c: stunts (times 400)
    char field_0x40[16];
};

// One row of the rankings 0x004d8c40 sorts (with 0x004d8c20).
struct UnknownProCircuitRank {
    int points;                               // +0x00: points
    int racer;                                // +0x04: racer
};

// 0x004d8c20: qsort order of UnknownProCircuitRank rows, most points first.
int CompareRankPoints(const void* a, const void* b);

// The skinned models the garage shows (a bike model's +0xc0->+0x1a0, and the
// object KrustyUI+0x46c points to): 0x00444c70 (among D3DIMSoulTree.CPP's
// literals) applies the skin `name` with the game's textures.
class UnknownProCircuitSkinned {
public:
    void UnknownFunction444c70(int a, const char* name, TextureMapManager** textures);
};

// RTTI: PCBonusTrackDlg : UIDialog (vtable 0x00557434; 0x7f58 bytes).
class PCBonusTrackDlg : public UIDialog {
public:
    PCBonusTrackDlg() : UIDialog(1, "PCDeal.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d9cd0

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
};

// RTTI: PCBailoutDlg : UIDialog (vtable 0x005573b0; 0x7f64 bytes): offers
// to cover a missing entry fee.
class PCBailoutDlg : public UIDialog {
public:
    PCBailoutDlg() : UIDialog(1, "PCDeal.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d8fc0

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
    int entryFee;                             // +0x7f58: the entry fee
    int playerCash;                           // +0x7f5c: the player's cash
    int cashShortfall;                        // +0x7f60: the difference
};

// RTTI: PCFinishedDlg : UIDialog (vtable 0x005574bc; 0x7f58 bytes): the
// final standings.
class PCFinishedDlg : public UIDialog {
public:
    PCFinishedDlg() : UIDialog(1, "PCFinish.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d94e0

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
};

// RTTI: PCBunnyDlg : UIDialog (vtable 0x0055732c; 0x7f64 bytes).
class PCBunnyDlg : public UIDialog {
public:
    PCBunnyDlg() : UIDialog(1, "PCDeal.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d9860

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
    int field_0x7f58;                         // the entry fee
    int field_0x7f5c;                         // the player's cash
    int field_0x7f60;                         // what accepting adds
};

// RTTI: PCFailedDlg : UIDialog (vtable 0x005572a8; 0x7f58 bytes).
class PCFailedDlg : public UIDialog {
public:
    PCFailedDlg() : UIDialog(1, "PCTrans.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d9340

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
};

// RTTI: PCLastRaceDlg : UIDialog (vtable 0x00554a90): the last race's
// results.
class PCLastRaceDlg : public UIDialog {
public:
    PCLastRaceDlg() : UIDialog(1, "PCLRace.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d8660
    void PayOutRace();             // 0x004d8c40: pays out the race

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
};

// RTTI: PCCompleteDlg : UIDialog (vtable 0x00557014; 0x7f58 bytes).
class PCCompleteDlg : public UIDialog {
public:
    PCCompleteDlg() : UIDialog(1, "PCTrans.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004da1b0

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
};

// RTTI: PCNewEventDlg : UIDialog (vtable 0x00556f90; 0x7f58 bytes).
class PCNewEventDlg : public UIDialog {
public:
    PCNewEventDlg() : UIDialog(1, "PCTrans.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d9fd0

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
};

// RTTI: PCCentralNextDlg : UIDialog (vtable 0x0055711c; 0x7f58 bytes): the
// next race page of PCCentralDlg.
class PCCentralNextDlg : public UIDialog {
public:
    PCCentralNextDlg() : UIDialog(1, "PCCNext.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d7f40

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: PCCentralBikeRiderDlg : UIDialog (vtable 0x005571a0; 0x7f9c bytes):
// the bike and rider page of PCCentralDlg.
// Its layout follows MPBikeRiderDlg's (SelectGamePicProcs.h). Slots 13 and 26
// (0x004d77d0, 0x004d72a0) are shared with MPBikeRiderDlg (26 also with
// SPBikeRiderDlg) by identical-code folding; the kept copies sit among this
// file's code.
class PCCentralBikeRiderDlg : public UIDialog {
public:
    PCCentralBikeRiderDlg() : UIDialog(1, "PCCBikeR.dtm") {}
    virtual int UnknownVirtualSlot10(float frameTime); // 0x004d72c0: turns the bike view
    virtual int UnknownVirtualSlot13();       // 0x004d77d0: applies the chosen bike and rider skins
    virtual void UnknownVirtualSlot26();      // 0x004d72a0: hides the bike view
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d6770
    void UnknownVirtualSlot31(int apply);     // 0x004d7d70: stores the chosen bike and rider
    void FillBikeRiderLists();             // 0x004d6fc0: fills the bike and rider lists
    void PaintPlateNumber(int number);   // 0x004d71f0: paints the plate number on every bike
    void ApplyChosenRider();             // 0x004d78c0: takes the chosen rider
    void ApplyChosenBike();             // 0x004d7900: applies the chosen bike

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    GUIUser* guiUser;                         // +0x34
    unsigned char field_0x38[0x110 - 0x38];
    BackgroundImage* dialogBackground;        // +0x110
    unsigned char field_0x114[0x7f58 - 0x114];
    Vector3 field_0x7f58;                     // the bike view's eye
    Vector3 field_0x7f64;                     // and target
    float field_0x7f70;                       // their distance
    int field_0x7f74;                         // background region
    int field_0x7f78;                         // auto-rotate
    int field_0x7f7c;
    int field_0x7f80;
    int field_0x7f84;                         // the bike changed
    int field_0x7f88;                         // the rider changed
    RECT field_0x7f8c;                        // the bike view
};

// RTTI: PCCentralStandingsDlg : UIDialog (vtable 0x00557224; 0x7f58 bytes):
// the standings page of PCCentralDlg.
class PCCentralStandingsDlg : public UIDialog {
public:
    PCCentralStandingsDlg() : UIDialog(1, "PCCStand.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d8380

    unsigned char field_0x2c[0x7f58 - 0x2c];
};

// RTTI: PCCentralDlg : UIDialog (vtable 0x00554a0c; 0x7f64 bytes): the
// career's hub with its three pages.
class PCCentralDlg : public UIDialog {
public:
    PCCentralDlg() : UIDialog(1, "PCC.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d5f70
    void OpenPage(int page);     // 0x004d6570: opens page `page` (0..2)

    UnknownGuiDialog* parentDialog;           // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
    PCCentralNextDlg* field_0x7f58;
    PCCentralBikeRiderDlg* field_0x7f5c;
    PCCentralStandingsDlg* field_0x7f60;
};

// RTTI: PCNewDlg : UIDialog (vtable 0x00557098; 0x7f68 bytes): asks for a
// new career's name, class, difficulty and number of opponents.
class PCNewDlg : public UIDialog {
public:
    PCNewDlg() : UIDialog(1, "PCNew.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d5d20
    // 0x004d5f40: where the answers go.
    void SetAnswerTargets(char* name, int* difficulty, int* bikeClass, int* opponents);

    unsigned char field_0x2c[0x34 - 0x2c];
    GUIUser* guiUser;                         // +0x34
    unsigned char field_0x38[0x7f58 - 0x38];
    char* field_0x7f58;                       // name (64 bytes)
    int* field_0x7f5c;                        // "RadLODEasy" group
    int* field_0x7f60;                        // "RadClass125" group
    int* field_0x7f64;                        // "OpponentsListBox" row
};

// RTTI: PCStartupDlg : UIDialog (vtable 0x00551a10): lists the saved
// careers (ui\profile\<player>\*.pc) and starts or continues one.
class PCStartupDlg : public UIDialog {
public:
    PCStartupDlg() : UIDialog(1, "PCStart.dtm") {} // 0x8074 bytes (PCCentralDlg 0x004d6125)
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004d4d20
    void EnableButtons();             // 0x004d5600: enables the buttons
    void FillLists();             // 0x004d56c0: fills the lists
    // 0x004d59a0: adds `circuit` as row `row` of the lists.
    void AddCircuitRow(UnknownTrackGameObject3444* circuit, int row);
    void FreeFileNames();             // 0x004d5ca0: frees the file names

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
    char** field_0x7f58;                      // career file names
    int field_0x7f5c;                         // their count
    char field_0x7f60[0x100];                 // new career's name
    int field_0x8060;                         // difficulty
    int field_0x8064;                         // bike class
    int field_0x8068;                         // opponents row
    ChoiceDlg* field_0x806c;                  // delete confirmation (running careers)
    ChoiceDlg* field_0x8070;                  // delete confirmation (finished careers)
};

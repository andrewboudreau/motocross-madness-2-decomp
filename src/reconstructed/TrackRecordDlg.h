#pragma once

#include "DirectoryList.h"
#include "GameUi.h"
#include "UIDialog.h"

// The message TrackRecordDlg's slot 29 (0x0051f600) receives: its +0x08 is
// the type (1..6), and +0x0c the dialog whose controls the list helpers
// fill. The layout matches UnknownDialogEvent (DialogProc.h).
struct UnknownTrackRecordEvent {
    int field_0x00;
    const char* field_0x04;                   // control name
    int field_0x08;
    UIDialog* field_0x0c;
    void* field_0x10;
    UnknownGameUiControl* field_0x14;         // the control ("LstTrack")
    int field_0x18;
    int field_0x1c;
    int field_0x20;                           // set to 1 once handled
};

// An 8-byte list row (0x0051ffe0, TrackRecord.cpp line 535): a strdup'd
// name and a value.
struct UnknownTrackRecordRow {
    char* field_0x00;
    int field_0x04;
};

// The "LstTrack" control. Of the gameui.cpp classes only UIListBox
// (vtable 0x00553100) and UIDDLListBox have a slot 66, so this is probably
// UIListBox (strong inference). Only slot 66 is called.
class UnknownTrackRecordListBox {
public:
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
    virtual void UnknownVirtualSlot27();
    virtual void UnknownVirtualSlot28();
    virtual void UnknownVirtualSlot29();
    virtual void UnknownVirtualSlot30();
    virtual void UnknownVirtualSlot31();
    virtual void UnknownVirtualSlot32();
    virtual void UnknownVirtualSlot33();
    virtual void UnknownVirtualSlot34();
    virtual void UnknownVirtualSlot35();
    virtual void UnknownVirtualSlot36();
    virtual void UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot38();
    virtual void UnknownVirtualSlot39();
    virtual void UnknownVirtualSlot40();
    virtual void UnknownVirtualSlot41();
    virtual void UnknownVirtualSlot42();
    virtual void UnknownVirtualSlot43();
    virtual void UnknownVirtualSlot44();
    virtual void UnknownVirtualSlot45();
    virtual void UnknownVirtualSlot46();
    virtual void UnknownVirtualSlot47();
    virtual void UnknownVirtualSlot48();
    virtual void UnknownVirtualSlot49();
    virtual void UnknownVirtualSlot50();
    virtual void UnknownVirtualSlot51();
    virtual void UnknownVirtualSlot52();
    virtual void UnknownVirtualSlot53();
    virtual void UnknownVirtualSlot54();
    virtual void UnknownVirtualSlot55();
    virtual void UnknownVirtualSlot56();
    virtual void UnknownVirtualSlot57();
    virtual void UnknownVirtualSlot58();
    virtual void UnknownVirtualSlot59();
    virtual void UnknownVirtualSlot60();
    virtual void UnknownVirtualSlot61();
    virtual void UnknownVirtualSlot62();
    virtual void UnknownVirtualSlot63();
    virtual void UnknownVirtualSlot64();
    virtual void UnknownVirtualSlot65();
    virtual void UnknownVirtualSlot66(int a);
};

// RTTI: TrackRecordDlg : UIDialog (vtable 0x0055177c; 0x7f68 bytes, the size
// allocated at 0x0044c27b and 0x0049a2e6, with an inline constructor).
// Names are provisional (docs/TRACKRECORD.md).
class TrackRecordDlg : public UIDialog {
public:
    TrackRecordDlg() : UIDialog(1, "HiScores.dtm") {} // inline (KrustyUI 0x00499b20)
    void UnknownVirtualSlot29(UnknownTrackRecordEvent* event);             // 0x0051f600
    void UnknownFunction51fc40(int series);                                 // 0x0051fc40: labels the tabs
    void UnknownFunction51fe80(UnknownTrackRecordEvent* event, int series); // 0x0051fe80: shows a tab
    void UnknownFunction51ff40();
    void UnknownFunction51ffe0(UnknownGameUiControl* list, DirectoryList* directory, int series); // 0x0051ffe0
    void UnknownFunction520390(UnknownTrackRecordEvent* event, int series); // 0x00520390: fills the lists                                           // 0x0051ff40: frees the rows
    void UnknownFunction520480(UnknownTrackRecordEvent* event, const char* name, float value,
                               const char* text);                           // 0x00520480
    void UnknownFunction5204e0(UnknownTrackRecordEvent* event, int times, const char* name); // 0x005204e0

    unsigned char field_0x2c[0x7f58 - 0x2c];
    int field_0x7f58;                         // selected series (0..5)
    int field_0x7f5c;
    UnknownTrackRecordRow** field_0x7f60;
    int field_0x7f64;                         // row count
};

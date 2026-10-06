#pragma once

// GUI page and control objects (their code sits among gameui.cpp's
// literals). Kept out of KrustyUI.h: declaring them there changed VC6's
// register choice in TrackGame slot 1 (see docs/TRACKGAME.md).

#include "GameObject.h"

class MediaControl;

// A 0x38-byte list box row; +0x14 is its text (OptionProcs.cpp 0x004b4e70).
struct UnknownGameUiListRow {
    unsigned char field_0x00[0x14];
    char* field_0x14;
    unsigned char field_0x18[0x38 - 0x18];
};

// A control found by name (its +0x1f0 is a progress bar's value). Every
// control a dialog finds is a UIControl (RTTI UIControl : GameObject,
// vtable 0x00552ba4, 65 slots); only the slots the dialog procedures call
// are given signatures.
class UnknownGameUiControl : public GameObject {
public:
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
    virtual void UnknownVirtualSlot49(int enable); // NetProcs.cpp, InGameProcs.cpp
    virtual void UnknownVirtualSlot50();
    virtual void UnknownVirtualSlot51();
    virtual void UnknownVirtualSlot52();
    virtual void UnknownVirtualSlot53();
    virtual void UnknownVirtualSlot54(int* value); // NetProcs.cpp: binds a value
    virtual void UnknownVirtualSlot55();
    virtual void UnknownVirtualSlot56();
    virtual void UnknownVirtualSlot57();
    virtual void UnknownVirtualSlot58();
    virtual void UnknownVirtualSlot59(int value); // SelectGamePicProcs.cpp ("ChkRecordRace")
    virtual void UnknownVirtualSlot60();
    virtual void UnknownVirtualSlot61();
    virtual void UnknownVirtualSlot62();
    virtual void UnknownVirtualSlot63();
    virtual void UnknownVirtualSlot64();
    virtual void UnknownVirtualSlot65();
    virtual void UnknownVirtualSlot66(int value); // OptionProcs.cpp (a list box's DDLCurves row change)

    void UnknownFunction47b370(int value);    // 0x0047b370
    void UnknownFunction470b20(const char* text); // 0x00470b20 (TrackRecord.cpp)
    void UnknownFunction476d80(const char* text, int a, int b); // 0x00476d80: adds a list row
    void UnknownFunction4775f0();             // 0x004775f0
    void UnknownFunction477900(int a);        // 0x00477900
    void UnknownFunction477bb0(int a);        // 0x00477bb0
    void UnknownFunction479310(int a);        // 0x00479310
    void UnknownFunction478a50(int index, void* module, int id); // 0x00478a50: a state's resource string
    void UnknownFunction478ad0(int index, const char* text);    // 0x00478ad0: a state's text

    // NetProcs.cpp and InGameProcs.cpp (provisional names).
    void UnknownFunction470660(int a, int b); // 0x00470660
    void UnknownFunction470a80(void* module, int id); // 0x00470a80: text from string resource `id`
    void UnknownFunction470d40(unsigned int color); // 0x00470d40
    void UnknownFunction470da0(int value);    // 0x00470da0
    void UnknownFunction473c70(int value);    // 0x00473c70
    void UnknownFunction473da0(char* text);   // 0x00473da0
    void UnknownFunction473f30(const char* characters); // 0x00473f30
    void UnknownFunction4751c0(int range);    // 0x004751c0
    void UnknownFunction4753c0(int a, int b); // 0x004753c0
    int UnknownFunction475500();              // 0x00475500
    int UnknownFunction4755c0();              // 0x004755c0
    int UnknownFunction4768d0(int value);     // 0x004768d0
    int UnknownFunction476950();              // 0x00476950: selected row, -1 when none
    void UnknownFunction476a60(int value);    // 0x00476a60
    void UnknownFunction473820(char* buffer, int size); // 0x00473820: an edit field's text buffer (OptionProcs.cpp)
    void UnknownFunction476ad0(const char* text); // 0x00476ad0: selects the row `text` (OptionProcs.cpp)
    int UnknownFunction475300(int value);     // 0x00475300 (OptionProcs.cpp)
    void UnknownFunction4754d0(int range);    // 0x004754d0 (OptionProcs.cpp)
    void UnknownFunction476b30(int row);      // 0x00476b30
    void UnknownFunction476ba0(unsigned int color, int row); // 0x00476ba0 (OptionProcs.cpp)
    void UnknownFunction476ff0(int row, const char* text); // 0x00476ff0 (OptionProcs.cpp)
    void UnknownFunction476b80(unsigned int color); // 0x00476b80
    void UnknownFunction476c70(unsigned int color, int a); // 0x00476c70
    void UnknownFunction476cd0(unsigned int color); // 0x00476cd0
    void UnknownFunction478860(int count);    // 0x00478860: number of states
    void UnknownFunction478cf0(int value);    // 0x00478cf0
    int UnknownFunction4793f0();              // 0x004793f0 (OptionProcs.cpp)
    // SelectGamePicProcs.cpp (provisional names).
    char* UnknownFunction473ef0(char* buffer, int size); // 0x00473ef0: copies an edit field's text
    void UnknownFunction476860(int row, int a); // 0x00476860
    void UnknownFunction477490(int row);      // 0x00477490: removes a list row
    void UnknownFunction477e60(int a);        // 0x00477e60
    void UnknownFunction477110(const char* image, int a, int b, int c); // 0x00477110: shows an image
    char* UnknownFunction476d20(int row);     // 0x00476d20: a row's text (-1: the selected one)
    // ProCircuitProcs.cpp (provisional names).
    void UnknownFunction473390(UnknownGameUiControl* list); // 0x00473390: links a column button to its list
    void UnknownFunction4777f0(int (*compare)(const void* a, const void* b)); // 0x004777f0: the rows' sort order
    void UnknownFunction470760(int a, const char* image); // 0x00470760: shows an image file
    void UnknownFunction470730(int a, void* image); // 0x00470730: shows a dialog resource image
    // Inline: a drop-down list's button.
    UnknownGameUiControl* UnknownInlineButton() { return field_0x1ec_control; }

    unsigned char field_0x02c[0x3c - 0x2c];
    int field_0x3c[4];                        // the control's area, a CameraRect (dlgprocs.cpp LoadingDlg)
    unsigned char field_0x04c[0x5c - 0x4c];
    int field_0x5c;                           // an edit box's state (dlgprocs.cpp UserNameDlg)
    unsigned char field_0x060[0x68 - 0x60];
    int field_0x68;                           // a movie control's playing flag (dlgprocs.cpp MainDlg slot 10)
    unsigned char field_0x06c[0x7c - 0x6c];
    int field_0x7c;                           // control id (UIControl slot 52 sets it; OptionProcs.cpp's SldEQ band)
    unsigned char field_0x080[0x1d8 - 0x80];
    int field_0x1d8;
    unsigned char field_0x1dc[0x1e8 - 0x1dc];
    int field_0x1e8;
    union {
        int field_0x1ec;                      // a list box's visible rows (OptionProcs.cpp)
        UnknownGameUiControl* field_0x1ec_control; // a drop-down list's button (SelectGamePicProcs.cpp)
        MediaControl* field_0x1ec_movie;      // a movie control's player (dlgprocs.cpp)
    };
    int field_0x1f0;
    unsigned char field_0x1f4[0x1fc - 0x1f4];
    UnknownGameUiControl* field_0x1fc;        // a drop-down list's list box (OptionProcs.cpp)
    unsigned char field_0x200[0x214 - 0x200];
    UnknownGameUiListRow* field_0x214;        // a list box's rows (OptionProcs.cpp)
    unsigned char field_0x218[0x21c - 0x218];
    int field_0x21c;                          // set on the SldEQ sliders (OptionProcs.cpp)
};

// Page object at KrustyUI+0x490.
class UnknownGameUiPage {
public:
    UnknownGameUiControl* UnknownFunction46ebf0(const char* name, int flags); // 0x0046ebf0
};

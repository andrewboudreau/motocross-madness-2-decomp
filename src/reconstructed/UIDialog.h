#pragma once

#include "GameObject.h"

// RTTI: UIDialog : GameObject (complete object locator 0x0055c280). Its
// constructor 0x00469db0 takes a flag and the dialog resource and writes
// vtable 0x00552a9c. Only what EventManager uses is declared.
class UnknownGameUiControl;
class Sound;
class GUIManager;
struct UnknownDialogEvent;

class UIDialog : public GameObject {
public:
    UIDialog(int flags, const char* resource);    // 0x00469db0
    // 0x0046ebf0: the control whose .dtm section is `name` ("LstProfiles",
    // "ButLeft", ...), of control type `type` when nonzero (GameUi.h declares
    // the same function on UnknownGameUiPage).
    UnknownGameUiControl* FindControl(const char* name, int type);

    virtual int Release();                    // 0x0046a010 (gameui.cpp)

    // NetProcs.cpp and InGameProcs.cpp (provisional names).
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0046ef00
    virtual int UnknownVirtualSlot13();       // 0x0046f1c0 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot14();       // 0x0046f300 (gameui.cpp)
    virtual int UnknownVirtualSlot15();       // 0x0046f320 (gameui.cpp)
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0046a7d0 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0046a780 (SelectGamePicProcs.cpp)
    void UpdateBoundValues(int value);        // 0x0046ecc0: slot 59 on every control with a bound value
    // 0x0046e9a0: the object (a control, image or sound) built for the .dtm
    // section `name`, else the GUI's (the control loader resolves "SoundNorm",
    // "FXAnimIn" and "Anchor" references through it).
    Sound* FindSectionObject(const char* name);
    // 0x0046fce0: a UITimer `id` running every `time` ms; it tells `control`
    // (slot 60) or, when 0, sends the dialog a kDialogTimer event.
    void AddTimer(int id, int time, int control);
    void RemoveTimers(int id);                // 0x0046fe40: removes the timers `id`
    void EnableGroup(int group, int enable);  // 0x0046ea80: slot 49 on the controls of "GroupId" `group`
    void ShowGroup(int group, int show);      // 0x0046eb30: Show on the controls of "GroupId" `group`
    void EndDialog(int result);               // 0x0046ff30: stores `result` (+0x17c) and starts closing
    void NotifyParent(int code, int kind);    // 0x0046ff70: sends the parent dialog an event
    int UnknownFunction470000(UnknownGameUiControl* control, int a, int b); // 0x00470000
    GameObject* AddControl(GameObject* control, int group, int region); // 0x0046a8a0

    // gameui.cpp (provisional names).
    UnknownGameUiControl* UnknownFunction46a840(UnknownGameUiControl* control, int group, int region); // 0x0046a840
    void UnknownFunction46ea60(int value);    // 0x0046ea60
    void EndControlDraw(void* dc);            // 0x0046eeb0: restores the font and releases `dc`
    int LoadDialogResource(const char* name); // 0x0046a8e0: the .dtm `name` from the resource manager
    int UnknownFunction46a920(void* stream, int offset); // 0x0046a920
    // 0x0046f3c0: TextOutA, with a shadow one pixel up and left when `shadow`.
    void DrawShadowText(void* dc, int x, int y, const char* text, int length,
                        unsigned int color, int shadow, unsigned int shadowColor);
    // 0x0046f420: the same with colored runs (`runs` ends with a null text).
    void DrawTextRuns(void* dc, int x, int y, const char* text, int length,
                      struct UnknownGameUiTextRun* runs, unsigned int color, int shadow,
                      unsigned int shadowColor);
    // 0x0046f550: aligns the text in `rect` by `flags` and draws it.
    void DrawAlignedText(void* dc, struct CameraRect* rect, const char* text, int length,
                         unsigned int color, int height, struct UnknownGameUiTextRun* runs, int shadow,
                         unsigned int shadowColor, int flags, int transparent,
                         UnknownGameUiControl* control);
    // 0x0046f6c0: draws `control`'s text inside its margins.
    void DrawControlText(void* dc, UnknownGameUiControl* control, int transparent);
    // 0x0046ed70: a DC for drawing `control` inside `rect`, with its font and clip region.
    int BeginControlDraw(void** dc, struct CameraRect* rect, int* a, UnknownGameUiControl* control);
    // 0x0046fc80: whether `position` is in a double-byte character of `text`.
    int IsInDoubleByteCharacter(const char* text, const char* position, int length);
    void UnknownFunction46f120();             // 0x0046f120
    void RemoveTimer(void* timer);            // 0x0046fec0: removes `timer`
    void UnknownFunction46ff60();             // 0x0046ff60
    void UnknownFunction46ffc0(int value);    // 0x0046ffc0
    void UnknownFunction46ffd0(void* background); // 0x0046ffd0: the background, also for its parents
    void UnknownFunction470070(int a, int b, CameraRect* rect); // 0x00470070
    void GrabBackground(int dim, CameraRect* rect); // 0x004700b0: grabs the screen behind it
    void ReleaseBackgroundGrab();             // 0x00470110: releases the grab
    // 0x0046f890: splits `text` into lines for a control; returns 0 when it fits on one.
    char* SplitTextLines(char* text, unsigned int color, void* font, void* a, int* size);
};

// RTTI: TransDlg : UIDialog (vtable 0x00552624; 0x7f64 bytes, the size
// EventManager 0x0045e710 allocates). Its constructor is inline.
class TransDlg : public UIDialog {
public:
    TransDlg() : UIDialog(1, "Trans.dtm") {}
    void UnknownFunction455c40(int menu);         // 0x00455c40: stores +0x7f60
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00455b60 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x00455bd0 (dlgprocs.cpp)

    unsigned char field_0x2c[0x7f58 - 0x2c];
    int framesShown;                          // +0x7f58: frames shown (dlgprocs.cpp)
    float secondsShown;                       // +0x7f5c: seconds shown (dlgprocs.cpp)
    int nextMenu;                             // +0x7f60: the menu to open next
};

// RTTI: Intro1Dlg : UIDialog (vtable 0x005548fc; 0x7f5c bytes, the size
// KrustyUI 0x004988a0 allocates). Its constructor is inline.
class Intro1Dlg : public UIDialog {
public:
    Intro1Dlg() : UIDialog(1, "intro1.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x004532c0 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x004533d0 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00453420 (dlgprocs.cpp)

    unsigned char field_0x2c[0x7f58 - 0x2c];
    int framesShown;                          // +0x7f58: frames shown (dlgprocs.cpp)
};

// RTTI: Exit1Dlg : UIDialog (vtable 0x0055503c; 0x7f68 bytes, the size
// KrustyUI 0x0049a4a0 allocates). Its constructor is inline.
class Exit1Dlg : public UIDialog {
public:
    Exit1Dlg() : UIDialog(1, "Exit1.dtm") {}
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00455c50 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x00455d00 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00455d70 (dlgprocs.cpp)

    unsigned char field_0x2c[0x30 - 0x2c];
    GUIManager* guiManager;                   // +0x30
    unsigned char field_0x34[0x7f58 - 0x34];
    int framesShown;                          // +0x7f58: frames shown (dlgprocs.cpp)
    float secondsShown;                       // +0x7f5c: seconds shown (dlgprocs.cpp)
    int frameRan;                             // +0x7f60: set once a frame has run (dlgprocs.cpp)
    int field_0x7f64;                             // a key or the "GoLink" button closes it (dlgprocs.cpp)
};

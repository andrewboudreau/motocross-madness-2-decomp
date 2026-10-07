#pragma once

#include "CameraRect.h"
#include "ContainerList.h"
#include "GameObject.h"

// RTTI: UIDialog : GameObject (complete object locator 0x0055c280; vtable
// 0x00552a9c): a 0x7f58-byte dialog built from a .dtm resource. Its
// constructor 0x00469db0 (flags, resource) and destructor 0x0046a070 are in
// gameui.cpp; every member below is initialised by the constructor or read
// by the gameui.cpp methods. Member names are provisional.
class UIControl;
class UIAnim;
class UIListBox;
class UITimer;
class Sound;
class SoundGroup;
class GUIManager;
class GUIUser;
class BackgroundImage;
class PCTextureMap;
class UnknownTextureStream;
struct UnknownCursorAnimation;
struct UnknownDialogEvent;

// One .dtm section of a dialog (0x3c bytes; the table is at UIDialog+0x9cc):
// its name and the control, image or sound built for it
// (UIDialog::FindSectionObject).
struct UnknownGameUiSection {
    char sectionName[0x34];                   // +0x00
    Sound* sectionObject;                     // +0x34 (typed as the sound case)
    int field_0x38;
};

class UIDialog : public GameObject {
public:
    UIDialog(int flags, const char* resource);    // 0x00469db0
    virtual ~UIDialog();                      // 0x0046a070 (deleting wrapper 0x00469ff0)
    // 0x0046ebf0: the control whose .dtm section is `name` ("LstProfiles",
    // "ButLeft", ...), of control type `type` when nonzero (GameUi.h declares
    // the same function on UnknownGameUiPage).
    UIControl* FindControl(const char* name, int type);

    virtual int Release();                    // 0x0046a010 (gameui.cpp)

    // NetProcs.cpp and InGameProcs.cpp (provisional names).
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0046ef00
    virtual int UnknownVirtualSlot13();       // 0x0046f1c0 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot14();       // 0x0046f300 (gameui.cpp)
    virtual int UnknownVirtualSlot15();       // 0x0046f320 (gameui.cpp)
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0046a7d0 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0046a780 (SelectGamePicProcs.cpp)

    // The dialog's own slots (gameui.cpp).
    // 0x0046a300: creates the controls from the resource; this, or 0 when it fails.
    virtual UIDialog* UnknownVirtualSlot27(void* target, CameraRect* area, int a, int flags,
                                           SoundGroup* sound, void* textures, const char* directory,
                                           UIDialog* parent, BackgroundImage* background,
                                           const char* font, int fontSize, GUIManager* gui,
                                           GUIUser* user, const char* name);
    virtual void UnknownVirtualSlot28(int value); // 0x0046e8c0
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00470020: the dialog's procedure
    virtual void UnknownVirtualSlot30();      // 0x00470040
    virtual void UnknownVirtualSlot31(int value); // 0x00470050

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
    int UnknownFunction470000(UIControl* control, int a, int b); // 0x00470000
    GameObject* AddControl(GameObject* control, int group, int region); // 0x0046a8a0

    // gameui.cpp (provisional names).
    UIControl* UnknownFunction46a840(UIControl* control, int group, int region); // 0x0046a840
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
    void DrawAlignedText(void* dc, CameraRect* rect, const char* text, int length,
                         unsigned int color, int height, struct UnknownGameUiTextRun* runs, int shadow,
                         unsigned int shadowColor, int flags, int transparent,
                         UIControl* control);
    // 0x0046f6c0: draws `control`'s text inside its margins.
    void DrawControlText(void* dc, UIControl* control, int transparent);
    // 0x0046ed70: a DC for drawing `control` inside `rect`, with its font and clip region.
    int BeginControlDraw(void** dc, CameraRect* rect, int* a, UIControl* control);
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

    // The render target (GameObject +0x18) as the controls read it.
    void* UnknownInlineField18() { return field_0x18; }

    UIDialog* parentDialog;                   // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    GUIUser* guiUser;                         // +0x34
    char resourceName[0x80];                  // +0x38: the dialog resource's name
    UnknownTextureStream* resourceArchive;    // +0xb8: its archive (slot 27)
    int isShown;                              // +0xbc
    int field_0xc0;
    int openingMenu;                          // +0xc4: the menu id it was shown for (GUIManager::ShowDialog's `a`)
    int animatesControls;                     // +0xc8: animates its controls
    void (*field_0xcc)(UnknownDialogEvent* event); // the procedure (slot 29)
    void (*field_0xd0)(UIDialog* dialog, int value); // slot 31
    void (*field_0xd4)();                     // slot 30
    void* dialogFont;                         // +0xd8
    int dialogFontHeight;                     // +0xdc
    char dialogFontFace[0x108 - 0xe0];        // +0xe0
    int field_0x108;                          // bold (GUIManager.cpp)
    int field_0x10c;
    BackgroundImage* dialogBackground;        // +0x110
    SoundGroup* soundGroup;                   // +0x114
    unsigned int textColors[10];              // +0x118: the text colors "~0".."~9" select
    void* dialogPalette;                      // +0x140
    int field_0x144;                          // flags from GUIManager 0x00485a70
    int field_0x148;                          // takes the input first (GUIManager 0x00485df0)
    int field_0x14c;                          // GUIManager 0x00485ee0's previous value
    int field_0x150;
    int field_0x154;                          // notifies the parent when destroyed
    int scaleToScreen;                        // +0x158: scales to the screen (not "NoScale")
    int isPopup;                              // +0x15c: "Popup"
    CameraRect screenArea;                    // +0x160
    int popupAlignment;                       // +0x170: a popup's alignment ("DlgAlignV" | "DlgAlignH"); 9 otherwise
    int screenWidth;                          // +0x174: "ScreenWidth"; where controls slide in from (x)
    int screenHeight;                         // +0x178: "ScreenHeight"; (y)
    int dialogResult;                         // +0x17c
    int field_0x180;                          // 7 by default
    int field_0x184;
    int field_0x188;
    UIAnim* imageTable[500];                  // +0x18c: the "Set_Anim" sections' images
    UnknownCursorAnimation* cursorAnimation;  // +0x95c: the cursor over it
    Sound* soundTable[20];                    // +0x960: the "Set_Sound" sections' sounds
    int imageCount;                           // +0x9b0
    int soundCount;                           // +0x9b4
    PCTextureMap* parentBackground;           // +0x9b8: the parent's background image ("BackgroundFile")
    UIListBox* sortingList;                   // +0x9bc: the list box being sorted
    float lastFrameTime;                      // +0x9c0
    float scaleX;                             // +0x9c4: horizontal scale
    float scaleY;                             // +0x9c8: vertical scale
    UnknownGameUiSection sectionTable[500];   // +0x9cc
    int sectionCount;                         // +0x7efc
    void* field_0x7f00;                       // the clip region of a control draw
    void* field_0x7f04;                       // the font it replaced
    int field_0x7f08;
    int ownsPalette;                          // +0x7f0c: owns the palette +0x140
    int ownsBackground;                       // +0x7f10: has its own background image
    int field_0x7f14;
    int field_0x7f18;                         // 1 by default; the controls' post3D
    int joystickCentred;                      // +0x7f1c: the joystick is centred
    void* dialogTextures;                     // +0x7f20
    ContainerList<UITimer*> timerList;        // +0x7f24 (0x14 bytes)
    int field_0x7f38;
    GameObject* controlContainer;             // +0x7f3c: the controls' container (a UICtlContainer)
    int sendFrameEvent;                       // +0x7f40: sends kind 0x12 on the next frame
    PCTextureMap* screenGrab;                 // +0x7f44: the screen behind it
    int grabRegion;                           // +0x7f48: -1 by default
    int field_0x7f4c;
    int isClosing;                            // +0x7f50
    int closeFrame;                           // +0x7f54: the frame it closed on
};

// RTTI: TransDlg : UIDialog (vtable 0x00552624; 0x7f64 bytes, the size
// EventManager 0x0045e710 allocates). Its constructor is inline.
class TransDlg : public UIDialog {
public:
    TransDlg() : UIDialog(1, "Trans.dtm") {}
    void SetNextMenu(int menu);                   // 0x00455c40: stores +0x7f60
    void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00455b60 (dlgprocs.cpp)
    virtual int UnknownVirtualSlot10(float frameTime);    // 0x00455bd0 (dlgprocs.cpp)

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

    int framesShown;                          // +0x7f58: frames shown (dlgprocs.cpp)
    float secondsShown;                       // +0x7f5c: seconds shown (dlgprocs.cpp)
    int frameRan;                             // +0x7f60: set once a frame has run (dlgprocs.cpp)
    int field_0x7f64;                             // a key or the "GoLink" button closes it (dlgprocs.cpp)
};

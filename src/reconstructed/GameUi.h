#pragma once

// GUI page and control objects of D:\aardvark\VC\krusty2\gameui.cpp (literal
// __FILE__ at 0x0056b738). Kept out of KrustyUI.h: declaring them there
// changed VC6's register choice in TrackGame slot 1 (see docs/TRACKGAME.md).
// Reconstructed bodies are in GameUi.cpp.

#include "GameObject.h"
#include "MediaControl.h"

class GUIManager;
struct UnknownSurfaceInterface;
class Sound;
class GameObjectIterator;
class UnknownGameUiDialog;
struct CameraRect;
struct tagPOINT;

class TextureMap;
class UIAnim;
class UIDropDownList;
class UIDialog;
class UIButton;
class UIEditBox;
class UIScrollBar;
class UIListBox;
class UIMultiState;
class UIProgressBar;

// A 0x38-byte list box row; +0x14 is its text (OptionProcs.cpp 0x004b4e70).
struct UnknownGameUiListRow {
    int kind;                                 // +0x00: 1 a text row, 2 an image row
    int flags;                                // +0x04
    int width;                                // +0x08
    int height;                               // +0x0c
    int sortIndex;                            // +0x10
    char* text;                               // +0x14
    char* imageFile;                          // +0x18
    int data;                                 // +0x1c: the caller's data
    UIAnim* image;                            // +0x20
    char* textLines;                          // +0x24
    unsigned int textColor;                   // +0x28
    int field_0x2c;
    int field_0x30;
    void* brush;                              // +0x34: background brush (HBRUSH, "ItemBoxColor")
};

// RTTI: UIFrame : BaseObject (vtable 0x00552cb8; 0x24 bytes, new'd at
// gameui.cpp line 0x14d1). One image of a UIAnim: a texture or a sound.
class UIFrame : public BaseObject {
public:
    // 0x00472960: loads image `file`.
    UIFrame(const char* file, void* textures, int a, int b, void* palette);
    // 0x00472d00: loads image `id` from a resource module.
    UIFrame(void* module, int id, void* textures, int a, int b, int c, void* palette);
    virtual ~UIFrame();                       // 0x00472dc0 (deleting wrapper 0x00472ba0)
    // 0x00472bc0: loads the image at `offset` of `stream`; 0 when it fails.
    int UnknownFunction472bc0(void* stream, int offset, void* palette);

    int isSound;                              // +0x08: 1: a sound, not an image
    int frameWidth;                           // +0x0c
    int frameHeight;                          // +0x10
    Sound* frameSound;                        // +0x14
    TextureMap* frameTexture;                 // +0x18
    int ownsSound;                            // +0x1c: owns +0x14
    int field_0x20;
};

// RTTI: UIAnim : BaseObject (vtable 0x00552ccc; 0xf8 bytes, new'd at
// gameui.cpp line 0xc10). A control's image for one state: up to 49
// UIFrames played in turn. DlgProcs.h's UnknownDialogImage is the same
// object.
class UIAnim : public BaseObject {
public:
    UIAnim(void* textures, void* palette);    // 0x00472e30
    virtual ~UIAnim();                        // 0x00472eb0 (deleting wrapper 0x00472e90)

    void SetFrameCount(int count);    // 0x00472f20: the frame count to play
    void SetFrameDelay(int delay);    // 0x00472f40: milliseconds per frame
    void Rewind();             // 0x00472f50: rewinds
    UIFrame* GetCurrentFrame();         // 0x00472f80: the current frame
    UIFrame* UnknownInlineFrame(int index) { return frameList[index]; }
    TextureMap* GetCurrentTexture();      // 0x00472f90: the current frame's texture
    int AddFrame(UIFrame* frame); // 0x00472fb0: appends a frame
    UIFrame* Advance();         // 0x00472fe0: advances
    TextureMap* AdvancePastSounds();      // 0x004730b0: advances past sound frames
    void LoadFile(const char* file, void* palette); // 0x004730e0
    void LoadFromModule(void* module, int id, int a, void* palette); // 0x00473160

    int currentFrame;                         // +0x08
    int playForwards;                         // +0x0c: 1: playing forwards
    int frameCount;                           // +0x10
    int passesLeft;                           // +0x14: passes left (0x7fff: forever)
    int field_0x18;
    int frameDelay;                           // +0x1c
    int lastStepTime;                         // +0x20
    int playBackwards;                        // +0x24: 1: plays backwards
    UIFrame* frameList[50];                   // +0x28
    void* animTextures;                       // +0xf0
    void* animPalette;                        // +0xf4
};

class UnknownGameUiControl;

// RTTI: UITimer : BaseObject (vtable 0x00552b90; 0x18 bytes).
class UITimer : public BaseObject {
public:
    // Inline in UIDialog 0x0046fce0.
    UITimer(int id, int time, UnknownGameUiControl* control) {
        timerId = id;
        periodTime = time;
        elapsedTime = 0;
        targetControl = control;
    }
    virtual ~UITimer();                       // 0x0046fe30 (deleting wrapper 0x0046fe10)

    int timerId;                              // +0x08
    int elapsedTime;                          // +0x0c
    int periodTime;                           // +0x10
    UnknownGameUiControl* targetControl;      // +0x14: the control told
};

// RTTI: UICtlContainer : GameObject (vtable 0x00552b20; 0x2c bytes): the
// parent of a dialog's controls.
class UICtlContainer : public GameObject {
public:
    UICtlContainer() : GameObject(1) {}
    virtual ~UICtlContainer();                // 0x0046e860 (deleting wrapper 0x0046e840)
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0047b440
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x0047b3f0
};

// One state of a UIMultiState (32 bytes; the slots 34-37 overrides read
// it the way UIControl's read its own +0xc0..+0xe4).
struct UnknownGameUiState {
    UIAnim* image;                            // +0x00
    UIAnim* focusImage;                       // +0x04
    char* text;                               // +0x08
    int field_0x0c;
    int selectable;                           // +0x10
    int textExtent[2];                        // +0x14: the text's length and width
    char* textLines;                          // +0x1c
};

// A control found by name. Every control a dialog finds is a UIControl
// (RTTI UIControl : GameObject, vtable 0x00552ba4, 65 slots, 0x1ec bytes;
// the derived controls' members follow it). The dialog procedures find
// controls through this type and cast them to the derived control a method
// belongs to.
class UnknownGameUiControl : public GameObject {
public:
    // 0x00470170: `type` is the control kind (5 a static, 12 a static text).
    UnknownGameUiControl(int type, int id, CameraRect* area, UnknownGameUiDialog* owner);
    virtual ~UnknownGameUiControl();          // 0x00470450 (deleting wrapper 0x00470430)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00470e00
    virtual int UnknownVirtualSlot13();       // 0x00471140
    virtual int UnknownVirtualSlot15();       // 0x004711a0
    virtual int UnknownVirtualSlot20(int value); // 0x00472320
    virtual int UnknownVirtualSlot21(int value); // 0x004723d0
    virtual int UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00472130
    virtual int UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry); // 0x00471fd0

    virtual void UnknownVirtualSlot27();      // 0x00471070: advances the state's image
    virtual int UnknownVirtualSlot28(tagPOINT point, int state); // 0x00470990: whether `point` is inside
    virtual void UnknownVirtualSlot29(int state); // 0x00470970: sets the state
    virtual int UnknownVirtualSlot30();       // 0x00467ae0 (shared body: returns 1)
    virtual int UnknownVirtualSlot31();       // 0x004725b0: takes the focus
    virtual void UnknownVirtualSlot32(int value); // 0x00472630: loses the focus
    virtual void UnknownVirtualSlot33(int value); // 0x00464e80 (shared empty body)
    virtual int UnknownVirtualSlot34();       // 0x004703c0
    virtual int UnknownVirtualSlot35();       // 0x004703d0
    virtual int UnknownVirtualSlot36();       // 0x004703e0
    virtual void* UnknownVirtualSlot37();     // 0x004703f0
    virtual void UnknownVirtualSlot38();      // 0x004710c0
    virtual void UnknownVirtualSlot39();      // 0x00470f10: scales the area
    virtual int UnknownVirtualSlot40();       // 0x00471240: draws
    virtual int UnknownVirtualSlot41();       // 0x004715d0
    virtual int UnknownVirtualSlot42();       // 0x00471850
    virtual int UnknownVirtualSlot43();       // 0x00471b30
    virtual int UnknownVirtualSlot44();       // 0x004719c0
    virtual int UnknownVirtualSlot45();       // 0x00471c90
    virtual int UnknownVirtualSlot46();       // 0x00471df0
    virtual int UnknownVirtualSlot47();       // 0x00471eb0
    virtual TextureMap* UnknownVirtualSlot48(int state); // 0x00471fa0: the state's texture
    virtual void UnknownVirtualSlot49(int enable); // 0x004705d0 (NetProcs.cpp, InGameProcs.cpp)
    virtual void UnknownVirtualSlot50();      // 0x00470640: redraws
    virtual int UnknownVirtualSlot51(tagPOINT point); // 0x004709f0
    virtual void UnknownVirtualSlot52(int id); // 0x00470400
    virtual int UnknownVirtualSlot53();       // 0x004726b0
    virtual void UnknownVirtualSlot54(int* value); // 0x00470a70 (NetProcs.cpp: binds a value)
    virtual int UnknownVirtualSlot55(int a, int b); // 0x004721b0
    virtual int UnknownVirtualSlot56(int a, int* position); // 0x00472250
    virtual void UnknownVirtualSlot57(int a, int* position); // 0x004722b0
    virtual void UnknownVirtualSlot58(int index, int a); // 0x00472670: plays sound `index`
    virtual void UnknownVirtualSlot59(int value); // 0x00464e80 (SelectGamePicProcs.cpp "ChkRecordRace")
    virtual void UnknownVirtualSlot60(int value); // 0x00464e80
    virtual int UnknownVirtualSlot61();       // 0x00470410: width
    virtual int UnknownVirtualSlot62();       // 0x00470420: height
    virtual void UnknownVirtualSlot63(CameraRect* in, CameraRect* out); // 0x00472860
    virtual void UnknownVirtualSlot64(CameraRect* in, CameraRect* out); // 0x004728e0

    // Names from the bodies and from the .dtm keys the dialog loader
    // (UIDialog slot 27) passes to them.
    void SetText(const char* text);           // 0x00470b20: copies `text`, measures it
    void SetTextFromResource(void* module, int id); // 0x00470a80: LoadStringA(`module`, `id`)
    void Show(int show, int animate);         // 0x00470660: the loader's "Show"; ChoiceDlg hides empty buttons
    void SetFontColor(unsigned int color);    // 0x00470d40: "TextColor"
    void SetTextAlign(int flags);             // 0x00470da0: "TextAlign" and "TextAlignV" bits
    void SetImageFile(int state, const char* file); // 0x00470760: a new UIAnim from `file`
    void SetImage(int state, void* image);    // 0x00470730: a state's UIAnim
    int IsEnabled();                          // 0x00470720: returns +0x6c
    void SetSound(int index, Sound* sound);   // 0x00470810: "SoundNorm", "SoundFocus", "SoundPush", "SoundClick"
    void SetAnchor(UnknownGameUiControl* anchor, int relative); // 0x00470830: "Anchor" and "RelAnchor"
    UnknownGameUiControl* UnknownFunction470850(UnknownGameUiControl* none); // 0x00470850: the last linked control
    void SetShapeBounds(int value);           // 0x004709d0: "ShapeBounds"
    void SetTextDrop(int value);              // 0x00470d60: "TextDrop"
    void SetDropColor(int value);             // 0x00470d80: "DropColor"
    void SetName(const char* name);           // 0x00470dc0: the .dtm section name
    char* GetName();                          // 0x00470df0
    void UnknownInlineSetField18(void* value) { field_0x18 = value; }
    // 0x00470870: starts transition `type` (100-106: the loader's "FX" GROW,
    // LSLIDE, RSLIDE, TSLIDE, BSLIDE and ANIM).
    void StartTransition(int type, int a, int b, int delay, int c);
    int UnknownFunction471500();              // 0x00471500
    GameObjectIterator* UnknownFunction4726b0(); // slot 53's body: an iterator over the owner's controls
    void UnknownFunction472730(GameObjectIterator* iterator); // 0x00472730: deletes it
    // 0x00472750: the next control with this control's id.
    UnknownGameUiControl* UnknownFunction472750(GameObjectIterator* iterator);
    // 0x00472790: the next control with this control's id and another type.
    UnknownGameUiControl* UnknownFunction472790(GameObjectIterator* iterator);
    UnknownGameUiControl* UnknownFunction4727c0(); // 0x004727c0: the previous focusable control
    UnknownGameUiControl* UnknownFunction472810(); // 0x00472810: the next focusable control
    int UnknownFunction472480(int* position); // 0x00472480

    int field_0x2c[4];                        // screen area (scaled +0x3c)
    union {
        int field_0x3c[4];                    // the control's area, a CameraRect (dlgprocs.cpp LoadingDlg)
        struct {
            int left;
            int top;
            int right;
            int bottom;
        } field_0x3c_rect;
    };
    int field_0x4c[4];                        // the visible part: left and top clipped, width, height
    // Member names follow the .dtm keys the dialog loader (UIDialog slot 27)
    // stores in them ("GroupId", "AttachId", "TextColor", ...).
    int controlType;                          // +0x5c: 5 static, 7/8 scroll bar, 11 edit box, 12 static text (FindControl's filter)
    int currentState;                         // +0x60: state (image index)
    int savedState;                           // +0x64: state before disabling
    int field_0x68;                           // a movie control's playing flag (dlgprocs.cpp MainDlg slot 10)
    int enabled;                              // +0x6c: "Enable" (slot 49)
    int field_0x70;
    int eventCode;                            // +0x74
    int groupId;                              // +0x78: "GroupId" (UIDialog::EnableGroup, ShowGroup)
    int attachId;                             // +0x7c: "AttachId" (slot 52; lists and their scroll bars share it)
    float field_0x80;                         // sliding position (x)
    float field_0x84;                         // sliding position (y)
    float field_0x88;                         // zooming width
    float field_0x8c;                         // zooming height
    float field_0x90;                         // x step per frame
    float field_0x94;                         // y step per frame
    float field_0x98;                         // width step per frame
    float field_0x9c;                         // height step per frame
    int shapeBounds;                          // +0xa0: "ShapeBounds"
    int sourceBlit;                           // +0xa4: "SourceBlit"
    unsigned int transitionTime;              // +0xa8
    unsigned int transitionDelay;             // +0xac: "FXDelay"
    int field_0xb0;
    int field_0xb4;
    UnknownGameUiDialog* ownerDialog;         // +0xb8
    GUIManager* ownerGui;                     // +0xbc
    char* controlText;                        // +0xc0: "Text" (SetText)
    unsigned int textColor;                   // +0xc4: "TextColor"
    int dropColor;                            // +0xc8: "DropColor"
    unsigned int currentTextColor;            // +0xcc
    int field_0xd0;
    int textDrop;                             // +0xd4: "TextDrop"
    int textAlign;                            // +0xd8: "TextAlign"/"TextAlignV" bits
    int textExtent[2];                        // +0xdc: the text's length and width
    char* textLines;                          // +0xe4: UIDialog::SplitTextLines' result, 0 for one line
    int textLeftMargin;                       // +0xe8: "TextLMargin"
    int textTopMargin;                        // +0xec: "TextTMargin"
    char* toolTipText;                        // +0xf0: "ToolTipText" / "LocalToolTipId"
    char controlName[0x32];                   // +0xf4: the .dtm section name (FindControl)
    unsigned char field_0x126[0x128 - 0x126];
    int fontHandle;                           // +0x128: its own HFONT ("FontName", "FontHeight")
    int ownsFont;                             // +0x12c
    char fontFace[0x158 - 0x130];             // +0x130
    int bold;                                 // +0x158
    int italic;                               // +0x15c
    int fontHeight;                           // +0x160
    int field_0x164;
    int moveable;                             // +0x168: "Moveable"
    UIAnim* stateImages[5];                   // +0x16c: "AnimNorm", "AnimFocus", "AnimPush", ...
    int mouseAnim;                            // +0x180: "MouseAnim" (a UnknownCursorAnimation*)
    UIAnim* fxAnimIn;                         // +0x184: "FXAnimIn"
    UIAnim* fxAnimOut;                        // +0x188: "FXAnimOut"
    UnknownGameUiControl* anchorControl;      // +0x18c: "Anchor"
    int relAnchor;                            // +0x190: "RelAnchor"
    Sound* sounds[5];                         // +0x194: "SoundNorm", "SoundFocus", "SoundPush", "SoundClick"
    Sound* slideSound;                        // +0x1a8
    Sound* fxSoundIn;                         // +0x1ac: "FXSoundIn"
    Sound* fxSoundOut;                        // +0x1b0: "FXSoundOut"
    int* boundValue;                          // +0x1b4: slot 54; written back by slot 59
    int backgroundRegion;                     // +0x1b8
    int redrawFrames;                         // +0x1bc: frames to redraw
    int needsRedraw;                          // +0x1c0
    void* field_0x1c4;
    int field_0x1c8;
    TextureMap* drawnTexture;                 // +0x1cc: the texture drawn
    int (UnknownGameUiControl::*field_0x1d0)(); // the running transition (slots 41-47)
    int field_0x1d4;
    int keyBind;                              // +0x1d8: "KeyBind"
    int post3D;                               // +0x1dc: "Pre3D" == 0
    int permanent;                            // +0x1e0: "Permanent"
    unsigned char field_0x1e4[0x1e8 - 0x1e4];
    int field_0x1e8;
};

// RTTI: UIButton : UIControl (vtable 0x00552ce0; 0x1f0 bytes: gameui.cpp's
// `new` at 0x0046c019 and the UIScrollCtl ones push 0x1f0).
class UIButton : public UnknownGameUiControl {
public:
    UIButton(int id, CameraRect* area, UnknownGameUiDialog* owner); // 0x004731f0
    virtual ~UIButton();                      // 0x004732c0 (deleting wrapper 0x004732a0)
    virtual int UnknownVirtualSlot28(tagPOINT point, int state); // 0x004734c0
    virtual void UnknownVirtualSlot49(int enable); // 0x004733a0
    virtual int UnknownVirtualSlot56(int a, int* position); // 0x00473410

    // 0x004732d0: the images for the up, over, down, disabled and focus states.
    void UnknownFunction4732d0(UIAnim* up, UIAnim* over, UIAnim* down, UIAnim* disabled);
    void SetNormalImage(UIAnim* image); // 0x00473310
    void SetFocusImage(UIAnim* image); // 0x00473330
    void SetPushImage(UIAnim* image); // 0x00473350
    void SetDisabledImage(UIAnim* image); // 0x00473370
    void SetSortList(UIListBox* list); // 0x00473390: links a column button to its list (ProCircuitProcs.cpp)

    UIListBox* field_0x1ec;                   // the list a column button sorts (slot 56 sorts it)
};

// RTTI: UIStatic : UIControl (vtable 0x00553318; 0x1ec bytes, `new` at
// 0x0046cace): no members of its own.
class UIStatic : public UnknownGameUiControl {
public:
    UIStatic(int id, CameraRect* area, UnknownGameUiDialog* owner); // 0x00478f60
    virtual ~UIStatic();                      // 0x00478fb0 (deleting wrapper 0x00478f90)
    virtual int UnknownVirtualSlot30();       // 0x00478fe0
    virtual TextureMap* UnknownVirtualSlot48(int state); // 0x00478fc0
};

// RTTI: UIEditBox : UIControl (vtable 0x00552de8; 0x230 bytes, `new` at
// 0x0046ce50).
class UIEditBox : public UnknownGameUiControl {
public:
    // 0x00473630: `size` is the text's capacity.
    UIEditBox(int id, CameraRect* area, UnknownGameUiDialog* owner, int size, int a, int b);
    virtual ~UIEditBox();                     // 0x00473770 (deleting wrapper 0x00473750)
    virtual int UnknownVirtualSlot20(int value); // 0x00474150
    virtual int UnknownVirtualSlot21(int value); // 0x00474570
    virtual void UnknownVirtualSlot33(int value); // 0x004740b0
    virtual void UnknownVirtualSlot38();      // 0x00479100 (shared with UIStaticText)
    virtual int UnknownVirtualSlot40();       // 0x004738a0
    virtual int UnknownVirtualSlot55(int a, int b); // 0x00474120
    virtual int UnknownVirtualSlot56(int a, int* position); // 0x00474b10 (shared with UIScrollCtl)
    virtual void UnknownVirtualSlot59(int value); // 0x00473840: writes the text back

    void UnknownFunction473d80(int value);    // 0x00473d80
    void UnknownFunction473d90(int value);    // 0x00473d90
    void BindTextBuffer(char* buffer, int size); // 0x00473820: binds a text buffer (OptionProcs.cpp)
    void SetCapacity(int size);     // 0x00473c70: the text's capacity
    void SetEditText(char* text);   // 0x00473da0: sets the text
    char* GetEditText(char* buffer, int size); // 0x00473ef0: copies the text
    void SetAcceptedCharacters(const char* characters); // 0x00473f30: the characters it accepts
    char FilterCharacter(char c);       // 0x00473fc0: `c` if accepted (either case)
    void SetBackgroundColor(unsigned long color); // 0x00474060: the background brush

    int textLength;                           // +0x1ec
    int textCapacity;                         // +0x1f0
    int field_0x1f4;
    int field_0x1f8;
    int textWidth;                            // +0x1fc
    int field_0x200;
    union {
        int field_0x204;
        Sound* field_0x204_sound;             // the key click
    };
    union {
        int field_0x208;
        Sound* field_0x208_sound;             // the "full" sound
    };
    void* backgroundBrush;                    // +0x20c: the background brush (HBRUSH)
    void* acceptedCharacters;                 // +0x210
    int field_0x214;
    int field_0x218;
    int field_0x21c;
    int field_0x220;
    int boundBufferSize;                      // +0x224: the bound buffer's size (0x00473820)
    int field_0x228;
    int field_0x22c;
};

// RTTI: UIScrollCtl : UIButton (vtable 0x00552ef0; 0x1f0 bytes, `new` at
// 0x0046c070 and 0x0046c0c3): a scroll arrow; `type` 9 scrolls back, 10
// forwards. No members of its own.
class UIScrollCtl : public UIButton {
public:
    UIScrollCtl(int type, int id, CameraRect* area, UnknownGameUiDialog* owner); // 0x00474820
    virtual ~UIScrollCtl();                   // 0x00474870 (deleting wrapper 0x00474850)
    virtual int UnknownVirtualSlot55(int a, int b); // 0x00474880
    virtual int UnknownVirtualSlot56(int a, int* position); // 0x00474b10
    virtual void UnknownVirtualSlot60(int value); // 0x004749f0
};

// RTTI: UIScrollBar : UIControl (vtable 0x00552ff8; 0x220 bytes, `new` at
// 0x0046cc82; its constructor clears +0x1ec..+0x21c).
class UIScrollBar : public UnknownGameUiControl {
public:
    UIScrollBar(int type, int id, CameraRect* area, UnknownGameUiDialog* owner); // 0x00474ba0
    virtual ~UIScrollBar();                   // 0x00474c50 (deleting wrapper 0x00474c30)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00474cf0
    virtual int UnknownVirtualSlot40();       // 0x00474ed0
    virtual int UnknownVirtualSlot55(int a, int b); // 0x004755d0
    virtual int UnknownVirtualSlot56(int a, int* position); // 0x00475880
    virtual void UnknownVirtualSlot57(int a, int* position); // 0x00475970
    virtual void UnknownVirtualSlot59(int value); // 0x00474cb0

    void UnknownFunction475160(int state);    // 0x00475160: the thumb size from a state image
    void UnknownFunction4751c0(int range);    // 0x004751c0
    int UnknownFunction4751e0(UIListBox* list); // 0x004751e0: `list`'s scroll range
    int UnknownFunction475200(UIListBox* list); // 0x00475200: follows `list`
    int UnknownFunction475300(int value);     // 0x00475300 (OptionProcs.cpp)
    int UnknownFunction4753c0(int value, int range); // 0x004753c0: sets the position to `value` of `range`
    void UnknownFunction4754d0(int range);    // 0x004754d0 (OptionProcs.cpp)
    int UnknownFunction475500();              // 0x00475500
    int UnknownFunction4755c0();              // 0x004755c0: +0x1f0

    union {
        int field_0x1ec;
        float field_0x1ec_float;              // the thumb's position (0..1)
    };
    int field_0x1f0;
    int thumbWidth;                           // +0x1f4
    int thumbHeight;                          // +0x1f8
    int field_0x1fc;
    int scrollRange;                          // +0x200
    union {
        int field_0x204;
        UIAnim* field_0x204_image;            // the track image
    };
    int field_0x208;
    int dragX;                                // +0x20c: the last drag position (x)
    int dragY;                                // +0x210: (y)
    float field_0x214;
    union {
        int field_0x218;
        float field_0x218_float;              // time held at one position
    };
    int field_0x21c;                          // set on the SldEQ sliders (OptionProcs.cpp)
};

// RTTI: UIListBox : UIControl (vtable 0x00553100; 0x250 bytes, `new` at
// 0x0046c5b0).
class UIListBox : public UnknownGameUiControl {
public:
    // 0x00475c70: `rows` is the row capacity (it passes (id, area, owner) on
    // to UIControl from its first, third and fourth arguments).
    UIListBox(int id, int rows, CameraRect* area, UnknownGameUiDialog* owner);
    virtual ~UIListBox();                     // 0x00475e20 (deleting wrapper 0x00475e00)
    virtual int UnknownVirtualSlot10(float frameTime); // 0x00476020: joystick selection and the row under the cursor
    virtual int UnknownVirtualSlot40();       // 0x004761f0: draws the rows
    virtual void UnknownVirtualSlot27();      // 0x00475fa0
    virtual void UnknownVirtualSlot29(int state); // 0x00479220
    virtual void UnknownVirtualSlot50();      // 0x00477ce0
    virtual int UnknownVirtualSlot21(int key); // 0x00478040
    virtual int UnknownVirtualSlot55(int a, int b); // 0x00477e90
    virtual int UnknownVirtualSlot56(int a, int* position); // 0x00477ff0
    virtual void UnknownVirtualSlot57(int a, int* position); // 0x00478190
    virtual void UnknownVirtualSlot59(int value); // 0x00475fe0
    // Slots 65 and 66 are UIListBox's own (UIControl's vtable has 65 slots,
    // UIListBox's and UIDDLListBox's 67).
    virtual int UnknownVirtualSlot65(int row); // 0x004769e0: selects `row`
    virtual void UnknownVirtualSlot66(int* handled); // 0x00477d30: reports a click (and a double click)

    int UnknownFunction4755c0();              // 0x004755c0 (folded with UIScrollBar's): the first row shown
    int ScrollToRow(int row, int updateScrollBars); // 0x00476860
    int GetRowData(int row);                  // 0x004768d0: a row's data (-1: the selected one)
    int FindRowByData(int data);              // 0x00476900: -1 when none
    void SetRowData(int row, int data);       // 0x00476930
    int GetSelectedRow();                     // 0x00476950: -1 when none
    int SelectRow(int row);                   // 0x00476a60: also on the lists attached to it
    int SelectRowByText(const char* text);    // 0x00476ad0
    int SelectRowByData(int data);            // 0x00476b30
    void SetSelectColor(unsigned int color);  // 0x00476b80: "SelectColor"
    void SetItemBoxColor(unsigned int color, int row); // 0x00476ba0: "ItemBoxColor" (row -1: all rows)
    void SetRowTextColor(unsigned int color, int row); // 0x00476c70 (row -1: the control's)
    void SetSelectBoxColor(unsigned int color); // 0x00476cd0: "SelectBoxColor"
    char* GetRowText(int row);                // 0x00476d20: (-1: the selected one)
    int UnknownFunction476d80(const char* text, int data, int a); // 0x00476d80: adds a row ("Item%d")
    int CountVisibleRows();                   // 0x00476ee0: the rows that fit from the first shown
    int UnknownFunction476f50(int rows);      // 0x00476f50: grows the rows to `rows`
    void SetRowText(int row, const char* text); // 0x00476ff0
    int AddImageFileRow(const char* image, int a, int b, int c); // 0x00477110
    int AddImageRow(UIAnim* image, int data, int a); // 0x004773a0 ("Item%d" starting with '@')
    int RemoveRow(int row);                   // 0x00477490
    void RemoveAllRows();                     // 0x004775f0
    void SetSortCompare(int (*compare)(const void* a, const void* b)); // 0x004777f0: qsort callback
    void UnknownFunction477900(int a);        // 0x00477900: sorts the rows
    void UnknownFunction477bb0(int a);        // 0x00477bb0: "Selectable" (+0x224)
    void UpdateScrollBars();                  // 0x00477bc0: the attached scroll bars follow
    void UnknownFunction477e60(int a);        // 0x00477e60
    char* UnknownFunction476970(int row);     // 0x00476970
    int GetRowHeight(int row);                // 0x004769a0
    void SetAutoSort(int value);              // 0x00477b90: "AutoSort"
    void ScrollBy(int delta);                 // 0x00477730
    void SetAllowWScroll(int value);          // 0x00477ba0: "AllowWScroll"

    int rowCount;                             // +0x1ec
    int firstVisibleRow;                      // +0x1f0
    int selectedRow;                          // +0x1f4
    int field_0x1f8;
    int rowCapacity;                          // +0x1fc
    int visibleRowCount;                      // +0x200
    int lastPageRowCount;                     // +0x204
    int selectColor;                          // +0x208: "SelectColor"
    void* itemBrush;                          // +0x20c: "ItemBoxColor" brush (HBRUSH)
    void* selectBrush;                        // +0x210: "SelectBoxColor" brush
    UnknownGameUiListRow* rowTable;           // +0x214: the rows (OptionProcs.cpp)
    union {
        int field_0x218;
        UIListBox* field_0x218_control;       // itself
    };
    int autoSort;                             // +0x21c: "AutoSort"
    int field_0x220;
    int selectable;                           // +0x224: "Selectable"
    int allowWScroll;                         // +0x228: "AllowWScroll"
    int field_0x22c;
    int field_0x230;
    int field_0x234;
    int lastClickRow;                         // +0x238: for the double click (slot 66)
    int lastClickTime;                        // +0x23c
    int field_0x240;
    int field_0x244;
    int field_0x248;
    int (*field_0x24c)(const void* a, const void* b); // the rows' sort order
};

// RTTI: UIMultiState : UIControl (vtable 0x00553210; 0x1f8 bytes, `new` at
// 0x0046c2b4).
class UIMultiState : public UnknownGameUiControl {
public:
    UIMultiState(int id, CameraRect* area, UnknownGameUiDialog* owner); // 0x004781f0
    virtual ~UIMultiState();                  // 0x00478300 (deleting wrapper 0x004782e0)
    virtual void UnknownVirtualSlot27();      // 0x004784e0
    virtual int UnknownVirtualSlot28(tagPOINT point, int state); // 0x00478e10: whether `point` is on an opaque pixel
    virtual int UnknownVirtualSlot40();       // 0x00478570: draws
    virtual void UnknownVirtualSlot29(int state); // 0x00478520
    virtual int UnknownVirtualSlot34();       // 0x00478260
    virtual int UnknownVirtualSlot35();       // 0x00478280
    virtual int UnknownVirtualSlot36();       // 0x004782a0
    virtual void* UnknownVirtualSlot37();     // 0x004782c0
    virtual void UnknownVirtualSlot38();      // 0x00478450
    virtual TextureMap* UnknownVirtualSlot48(int state); // 0x00478810
    virtual void UnknownVirtualSlot59(int value); // 0x00478540

    int UnknownFunction4755c0();              // 0x004755c0 (folded with UIScrollBar's): the current state
    int SetStateCount(int count);     // 0x00478860: number of states
    void SetStateEntry(int index, UIAnim* image, const char* text); // 0x004789f0
    void SetStateTextFromResource(int index, void* module, int id); // 0x00478a50: a state's resource string
    void SetStateText(int index, const char* text);    // 0x00478ad0: a state's text
    void SetCurrentState(int value);    // 0x00478cf0: sets the current state
    void SelectNextState();             // 0x00478d10: the next selectable state
    virtual int UnknownVirtualSlot55(int a, int b); // 0x00478d70

    int stateCount;                           // +0x1ec
    int selectedState;                        // +0x1f0
    UnknownGameUiState* stateTable;           // +0x1f4: the states
};

// RTTI: UIStaticText : UIControl (vtable 0x00553420; 0x1ec bytes, `new` at
// 0x0046d033): no members of its own.
class UIStaticText : public UnknownGameUiControl {
public:
    UIStaticText(int id, CameraRect* area, UnknownGameUiDialog* owner, const char* text,
                 unsigned int color);         // 0x00478ff0
    virtual ~UIStaticText();                  // 0x004790f0 (deleting wrapper 0x004790d0)
    virtual void UnknownVirtualSlot29(int state); // 0x00479220 (shared with UIListBox)
    virtual int UnknownVirtualSlot30();       // 0x00478fe0 (shared with UIStatic)
    virtual void UnknownVirtualSlot38();      // 0x00479100
    virtual int UnknownVirtualSlot40();       // 0x00479170
    virtual int UnknownVirtualSlot55(int a, int b); // 0x00479190 (shared with UIStatic)
};

// RTTI: UIRadioButton : UIMultiState (vtable 0x00553528; 0x1f8 bytes, `new`
// at 0x0046c982): no members of its own.
class UIRadioButton : public UIMultiState {
public:
    UIRadioButton(int id, CameraRect* area, UnknownGameUiDialog* owner); // 0x00479240
    virtual ~UIRadioButton();                 // 0x004792d0 (deleting wrapper 0x004792b0)
    virtual int UnknownVirtualSlot53();       // 0x004795c0
    virtual int UnknownVirtualSlot55(int a, int b); // 0x004794c0
    virtual void UnknownVirtualSlot59(int value); // 0x004792e0

    int SelectInGroup(int index);     // 0x00479310: selects radio button `index` (1-based) of the group
    int GetGroupSelection();              // 0x004793f0: the group's selected button (1-based), 0 when none
};

// RTTI: UIDDLScrollBar : UIScrollBar (vtable 0x00553630; 0x224 bytes, `new`
// at 0x00479fae): a drop-down list's scroll bar. Its destructor is the
// compiler's.
class UIDDLScrollBar : public UIScrollBar {
public:
    UIDDLScrollBar(int type, int id, CameraRect* area, UnknownGameUiDialog* owner,
                   UIDropDownList* list);     // 0x00479640
    virtual void UnknownVirtualSlot33(int value); // 0x004796e0
    virtual int UnknownVirtualSlot55(int a, int b); // 0x00479a00
    virtual void UnknownVirtualSlot57(int a, int* position); // 0x00479710

    UIDropDownList* ownerList;                // +0x220
};

// RTTI: UIDDLStatic : UIStatic (vtable 0x00553738; 0x1f0 bytes, new'd three
// times by UIDropDownList's constructor): a drop-down list's text. Its
// destructor is the compiler's (0x0047ae80, which UIVideoStatic shares).
class UIDDLStatic : public UIStatic {
public:
    UIDDLStatic(int id, CameraRect* area, UnknownGameUiDialog* owner, UIDropDownList* list); // 0x00479b50
    virtual void UnknownVirtualSlot33(int value); // 0x00479bc0

    UIDropDownList* ownerList;                // +0x1ec
};

// RTTI: UIDDLButton : UIButton (vtable 0x00553840; 0x1f4 bytes, `new` at
// 0x00479f2a): a drop-down list's button. Its destructor is the compiler's.
class UIDDLButton : public UIButton {
public:
    UIDDLButton(int id, CameraRect* area, UnknownGameUiDialog* owner, UIDropDownList* list); // 0x00479bf0
    virtual void UnknownVirtualSlot33(int value); // 0x00479ce0
    virtual int UnknownVirtualSlot55(int a, int b); // 0x00479c90

    UIDropDownList* ownerList;                // +0x1f0
};

// RTTI: UIDDLListBox : UIListBox (vtable 0x00553948; 0x254 bytes, `new` at
// 0x00479f6b): a drop-down list's list. Its destructor is the compiler's.
class UIDDLListBox : public UIListBox {
public:
    UIDDLListBox(int id, int rows, CameraRect* area, UnknownGameUiDialog* owner,
                 UIDropDownList* list);       // 0x00479d10
    virtual void UnknownVirtualSlot33(int value); // 0x00479e70
    virtual int UnknownVirtualSlot65(int row); // 0x00479db0
    virtual void UnknownVirtualSlot66(int* handled); // 0x00479df0

    UIDropDownList* ownerList;                // +0x250
};

// RTTI: UIDropDownList : UIStaticText (vtable 0x00553a58; 0x21c bytes, `new`
// at 0x0046d109).
class UIDropDownList : public UIStaticText {
public:
    // 0x00479ea0: creates the parts (button, list box, scroll bar and three statics).
    UIDropDownList(int id, CameraRect* area, UnknownGameUiDialog* owner, const char* text, unsigned int color);
    virtual ~UIDropDownList();                // 0x0047a1f0 (deleting wrapper 0x0047a1d0)
    virtual int UnknownVirtualSlot30();       // 0x00467ae0 (shared body: returns 1)
    virtual void UnknownVirtualSlot33(int value); // 0x0047a7d0
    virtual void UnknownVirtualSlot52(int id); // 0x0047a290
    virtual int UnknownVirtualSlot55(int a, int b); // 0x0047add0
    virtual void UnknownVirtualSlot59(int value); // 0x0047ad80
    virtual int UnknownVirtualSlot61();       // 0x0047a850
    virtual int UnknownVirtualSlot62();       // 0x0047a870

    void UnknownFunction47a2d0(int open);     // 0x0047a2d0: opens or closes the list
    void UnknownFunction47a400();             // 0x0047a400: names and lays out the parts
    void UnknownFunction47a880(int height);   // 0x0047a880: lays the parts out for a row height
    void UnknownFunction47a970(UIAnim* image); // 0x0047a970: the static part's image
    void UnknownFunction47aa40(UIAnim* image); // 0x0047aa40: the button's image
    void UnknownFunction47ab20(UIAnim* image); // 0x0047ab20: the corner image; lays the parts out around it
    int UnknownFunction47a800(UnknownGameUiControl* control); // 0x0047a800: whether `control` is a part
    // Inline: the button. SelectGamePicProcs.cpp reads it through this
    // accessor; reading field_0x1ec directly lets VC6 merge the two branches
    // of 0x004f3260 and 0x004f8700, which retail does not. Defined after the
    // class (an in-class body flips ProCircuitProcsNearMisses' 0x004d59a0).
    UIDDLButton* UnknownInlineButton();

    UIDDLButton* buttonPart;                  // +0x1ec
    UIDDLStatic* staticPart0;                 // +0x1f0: the static parts
    UIDDLStatic* staticPart1;                 // +0x1f4
    UIDDLStatic* staticPart2;                 // +0x1f8
    UIDDLListBox* listPart;                   // +0x1fc: the list box (OptionProcs.cpp)
    UIDDLScrollBar* scrollBarPart;            // +0x200
    int rowHeight;                            // +0x204
    int isOpen;                               // +0x208
    int field_0x20c;
    int field_0x210;
    int field_0x214;
    int field_0x218;
};

inline UIDDLButton* UIDropDownList::UnknownInlineButton() { return buttonPart; }

// MediaControl (RTTI, vtable 0x005551c0; 0x80 bytes, new'd by UIVideoStatic
// 0x0047ae90), the movie player, is declared in MediaControl.h.

// RTTI: UIVideoStatic : UIStatic (vtable 0x00553b60; 0x200 bytes, new'd by
// dlgprocs.cpp's CreditsVidDlg): a control that plays a movie. Its
// destructor is the compiler's (slot 0 is 0x0047ae60, shared with
// UIDDLStatic).
class UIVideoStatic : public UIStatic {
public:
    UIVideoStatic(int flags, CameraRect* area, UIDialog* owner); // 0x0047ae30
    virtual int UnknownVirtualSlot10(float frameTime); // 0x0047af90
    virtual int UnknownVirtualSlot40();       // 0x0047afa0: copies the movie's frame
    // 0x0047ae90: plays `file`; `done` is called with `owner` at its end.
    int UnknownFunction47ae90(const char* file, int a, void (*done)(UIDialog* dialog), UIDialog* owner);

    MediaControl* field_0x1ec;                // the player
    int field_0x1f0[4];                       // the frame's source rectangle
};

// RTTI: UIProgressBar : UIStatic (vtable 0x00553c68; 0x204 bytes, new'd by
// dlgprocs.cpp's LoadingDlg).
class UIProgressBar : public UIStatic {
public:
    UIProgressBar(int flags, CameraRect* area, UIDialog* owner); // 0x0047b020
    virtual ~UIProgressBar();                 // 0x0047b090 (deleting wrapper 0x0047b070)
    virtual int UnknownVirtualSlot40();       // 0x0047b110: draws the bar and its percentage
    virtual TextureMap* UnknownVirtualSlot48(int state); // 0x0047b100
    void UnknownFunction47b370(int redraw);   // 0x0047b370: one step on
    void UnknownFunction47b3d0(int texture, int owned); // 0x0047b3d0: the bar's texture (a TextureMap*)

    float progressFraction;                   // +0x1ec: the part done (0..1)
    int field_0x1f0;                          // steps
    int stepsDone;                            // +0x1f4: steps done (0x0047b370)
    TextureMap* barTexture;                   // +0x1f8
    int showPercentage;                       // +0x1fc
    int ownsBarTexture;                       // +0x200: owns +0x1f8
};

// Page object at KrustyUI+0x490.
class UnknownGameUiPage {
public:
    UnknownGameUiControl* UnknownFunction46ebf0(const char* name, int flags); // 0x0046ebf0
};

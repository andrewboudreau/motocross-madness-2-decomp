// Part of D:\aardvark\VC\krusty2\gameui.cpp (literal __FILE__ at
// 0x0056b738): the dialog (UIDialog), its control container and the
// UIControl family. Member names are provisional; see GameUi.h.

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GameUi.h"

#include "BackgroundImage.h"
#include "ContainerList.h"
#include "DebugAlloc.h"
#include "DialogEventKind.h"
#include "DialogProc.h"
#include "GUIManager.h"
#include "InputDevice.h"
#include "KeyboardDevice.h"
#include "MatrixUtil.h"
#include "PCAudio.h"
#include "PCTextureMap.h"
#include "Palette8.h"
#include "PCRenderTarget.h"
#include "RenderTarget.h"
#include "RenderInterfaces.h"
#include "TextureMap.h"
#include "Tgafile.h"
#include "TrackGame.h"
#include "UIDialog.h"
#include "UnknownResourceManager.h"

// The four vector constants that open many retail files (see
// src/krusty2/math/Math3D.h): 0x0065b568, 0x0065b578, 0x0065b590 and
// 0x0065b550, initialised by 0x0046e870..0x0046ea5b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);

// The smaller and the larger of two ints (inline helpers; the drag and
// progress code computes each operand once).
static inline int UnknownMinInt(int a, int b) { return a < b ? a : b; }
static inline int UnknownMaxInt(int a, int b) { return a < b ? b : a; }

// The same as macros, evaluating an operand twice (0x00479a00 calls slot 62
// again for the chosen operand).
#define UNKNOWN_MIN(a, b) ((a) < (b) ? (a) : (b))
#define UNKNOWN_MAX(a, b) ((a) < (b) ? (b) : (a))

// cdecl 0x0047b570: resizes a DebugMalloc'd block (KrustyUI.cpp).
void* UnknownFunction47b570(void* block, unsigned int size);
// 0x0065b608: the dialog whose list box is being sorted (0x00477900).
UnknownGameUiDialog* g_UnknownGlobal65b608;
// 0x0065b60c: the frames of the zoom transition (UIControl slot 41).
int g_UnknownGlobal65b60c;

// cdecl 0x00477b60: the list rows' default order (by text).
int UnknownFunction477b60(const void* a, const void* b);
// cdecl 0x00477800: the qsort comparison 0x00477900 sorts with
// (samples/ui/GameUiNearMisses.cpp).
int UnknownFunction477800(const void* a, const void* b);
// cdecl 0x0047b490: whether `texture` has a pixel of colour `key`
// (samples/ui/GameUiNearMisses.cpp).
int UnknownFunction47b490(TextureMap* texture, int key);

// One .dtm section of a dialog (0x3c bytes; the table is at UIDialog+0x9cc):
// its name and the control, image or sound built for it
// (UIDialog::FindSectionObject).
struct UnknownGameUiSection {
    char sectionName[0x34];                   // +0x00
    Sound* sectionObject;                     // +0x34 (typed as the sound case)
    int field_0x38;
};

// A dialog as its controls see it: UIDialog (0x7f58 bytes) with the
// members read here. Never constructed as such.
class UnknownGameUiDialog : public UIDialog {
public:
    // 0x0046a300: creates the controls from the resource; this, or 0 when it fails.
    virtual UnknownGameUiDialog* UnknownVirtualSlot27(void* target, CameraRect* area, int a, int flags,
                                                      SoundGroup* sound, void* textures, const char* directory,
                                                      UnknownGameUiDialog* parent, BackgroundImage* background,
                                                      const char* font, int fontSize, GUIManager* gui,
                                                      GUIUser* user, const char* name);
    virtual void UnknownVirtualSlot28(int value); // 0x0046e8c0
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00470020: the dialog's procedure
    virtual void UnknownVirtualSlot30();      // 0x00470040
    virtual void UnknownVirtualSlot31(int value); // 0x00470050

    void* UnknownInlineField18() { return field_0x18; }

    UnknownGameUiDialog* parentDialog;        // +0x2c: parent dialog
    GUIManager* guiManager;                   // +0x30
    GUIUser* guiUser;                         // +0x34
    char resourceName[0x80];                  // +0x38: the dialog resource's name
    UnknownTextureStream* resourceArchive;    // +0xb8: its archive (slot 27)
    int isShown;                              // +0xbc
    int field_0xc0;
    int field_0xc4;
    int animatesControls;                     // +0xc8: animates its controls
    void (*field_0xcc)(UnknownDialogEvent* event); // the procedure (slot 29)
    void (*field_0xd0)(UIDialog* dialog, int value); // slot 31
    void (*field_0xd4)();                     // slot 30
    void* dialogFont;                         // +0xd8
    int dialogFontHeight;                     // +0xdc
    char dialogFontFace[0x108 - 0xe0];        // +0xe0
    int field_0x108;
    int field_0x10c;
    BackgroundImage* dialogBackground;        // +0x110
    SoundGroup* soundGroup;                   // +0x114
    unsigned int textColors[10];              // +0x118: the text colors "~0".."~9" select
    void* dialogPalette;                      // +0x140
    int field_0x144;
    int field_0x148;
    int field_0x14c;
    unsigned char field_0x150[0x154 - 0x150];
    int field_0x154;
    int scaleToScreen;                        // +0x158: scales to the screen (not "NoScale")
    int isPopup;                              // +0x15c: "Popup"
    CameraRect screenArea;                    // +0x160
    int popupAlignment;                       // +0x170: a popup's alignment ("DlgAlignV" | "DlgAlignH"); 9 otherwise
    int screenWidth;                          // +0x174: "ScreenWidth"; where controls slide in from (x)
    int screenHeight;                         // +0x178: "ScreenHeight"; (y)
    int dialogResult;                         // +0x17c
    int field_0x180;
    unsigned char field_0x184[0x18c - 0x184];
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
    UnknownGameUiSection sectionTable[500]; // +0x9cc
    int sectionCount;                         // +0x7efc
    void* field_0x7f00;
    void* field_0x7f04;
    unsigned char field_0x7f08[0x7f0c - 0x7f08];
    int ownsPalette;                          // +0x7f0c: owns the palette +0x140
    int ownsBackground;                       // +0x7f10: has its own background image
    unsigned char field_0x7f14[0x7f18 - 0x7f14];
    int field_0x7f18;
    int joystickCentred;                      // +0x7f1c: the joystick is centred
    void* dialogTextures;                     // +0x7f20
    ContainerList<UITimer*> timerList;        // +0x7f24
    unsigned char field_0x7f38[0x7f3c - 0x7f38];
    GameObject* controlContainer;             // +0x7f3c: the controls' container
    int sendFrameEvent;                       // +0x7f40: sends kind 0x12 on the next frame
    PCTextureMap* screenGrab;                 // +0x7f44: the screen behind it
    int grabRegion;                           // +0x7f48
    int field_0x7f4c;
    int isClosing;                            // +0x7f50
    int closeFrame;                           // +0x7f54: the frame it closed on
};

// ---------------------------------------------------------------------------
// UICtlContainer

// 0x0046e860
UICtlContainer::~UICtlContainer() {
}

// ---------------------------------------------------------------------------
// UITimer

// 0x0046fe30
UITimer::~UITimer() {
}

// ---------------------------------------------------------------------------
// UIControl

// 0x004703c0
int UIControl::UnknownVirtualSlot34() {
    return (int)controlText;
}

// 0x004703d0
int UIControl::UnknownVirtualSlot35() {
    return field_0xd0;
}

// 0x004703e0
int UIControl::UnknownVirtualSlot36() {
    return (int)textLines;
}

// 0x004703f0
void* UIControl::UnknownVirtualSlot37() {
    return textExtent;
}

// 0x00470400
void UIControl::UnknownVirtualSlot52(int id) {
    attachId = id;
}

// 0x00470410
int UIControl::UnknownVirtualSlot61() {
    return field_0x2c[2] - field_0x2c[0];
}

// 0x00470420
int UIControl::UnknownVirtualSlot62() {
    return field_0x2c[3] - field_0x2c[1];
}

// 0x00470640
void UIControl::UnknownVirtualSlot50() {
    needsRedraw = 0;
    redrawFrames = 3;
}

// 0x00470720
int UIControl::IsEnabled() {
    return enabled;
}

// 0x00470730
void UIControl::SetImage(int a, void* image) {
    stateImages[a] = (UIAnim*)image;
    if (a == 1)
        stateImages[3] = (UIAnim*)image;
    UnknownVirtualSlot50();
}

// 0x00470810
void UIControl::SetSound(int index, Sound* sound) {
    sounds[index] = sound;
}

// 0x00470830
void UIControl::SetAnchor(UIControl* next, int a) {
    anchorControl = next;
    relAnchor = a;
}

// 0x00470850
UIControl* UIControl::UnknownFunction470850(UIControl* none) {
    UIControl* control = anchorControl;
    if (control) {
        UIControl* last;
        do {
            last = control;
            control = control->anchorControl;
        } while (control);
        return last;
    }
    return none;
}

// 0x00470970
void UIControl::UnknownVirtualSlot29(int state) {
    currentState = state;
    UnknownVirtualSlot50();
}

// 0x004709d0
void UIControl::SetShapeBounds(int value) {
    shapeBounds = value;
    UnknownVirtualSlot50();
}

// 0x00470a70
void UIControl::UnknownVirtualSlot54(int* value) {
    boundValue = value;
}

// 0x00470d40
void UIControl::SetFontColor(unsigned int color) {
    textColor = color;
    UnknownVirtualSlot50();
}

// 0x00470d60
void UIControl::SetTextDrop(int value) {
    textDrop = value;
    UnknownVirtualSlot50();
}

// 0x00470d80
void UIControl::SetDropColor(int value) {
    dropColor = value;
    UnknownVirtualSlot50();
}

// 0x00470da0
void UIControl::SetTextAlign(int value) {
    textAlign = value;
    UnknownVirtualSlot50();
}

// 0x00470dc0
void UIControl::SetName(const char* name) {
    strncpy(controlName, name, 0x31);
    controlName[0x31] = 0;
}

// 0x00470df0
char* UIControl::GetName() {
    return controlName;
}

// 0x00471fa0
TextureMap* UIControl::UnknownVirtualSlot48(int state) {
    if (state == -1)
        state = currentState;
    if (stateImages[state])
        return stateImages[state]->GetCurrentTexture();
    return 0;
}

// ---------------------------------------------------------------------------
// UIAnim

// 0x00472f20
void UIAnim::SetFrameCount(int count) {
    passesLeft = count;
    field_0x18 = count;
    Rewind();
}

// 0x00472f40
void UIAnim::SetFrameDelay(int delay) {
    frameDelay = delay;
}

// 0x00472f50
void UIAnim::Rewind() {
    if (playBackwards) {
        currentFrame = frameCount - 1;
        lastStepTime = 0;
        playForwards = 1;
    } else {
        currentFrame = 0;
        lastStepTime = 0;
        playForwards = 1;
    }
}

// 0x00472f80
UIFrame* UIAnim::GetCurrentFrame() {
    return frameList[currentFrame];
}

// 0x00472f90
TextureMap* UIAnim::GetCurrentTexture() {
    UIFrame* frame = frameList[currentFrame];
    if (frame->isSound == 1)
        return 0;
    return frame->frameTexture;
}

// 0x00472fb0
int UIAnim::AddFrame(UIFrame* frame) {
    if (frameCount < 0x31) {
        frameList[frameCount] = frame;
        frameCount++;
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// UIButton

// 0x004732c0
UIButton::~UIButton() {
}

// 0x004732d0
void UIButton::UnknownFunction4732d0(UIAnim* up, UIAnim* over, UIAnim* down, UIAnim* disabled) {
    stateImages[0] = up;
    stateImages[1] = over;
    stateImages[2] = down;
    stateImages[3] = over;
    stateImages[4] = disabled;
    UnknownVirtualSlot50();
}

// 0x00473310
void UIButton::SetNormalImage(UIAnim* image) {
    stateImages[0] = image;
    UnknownVirtualSlot50();
}

// 0x00473330
void UIButton::SetFocusImage(UIAnim* image) {
    stateImages[1] = image;
    stateImages[3] = image;
    UnknownVirtualSlot50();
}

// 0x00473350
void UIButton::SetPushImage(UIAnim* image) {
    stateImages[2] = image;
    UnknownVirtualSlot50();
}

// 0x00473370
void UIButton::SetDisabledImage(UIAnim* image) {
    stateImages[4] = image;
    UnknownVirtualSlot50();
}

// 0x00473390
void UIButton::SetSortList(UIListBox* list) {
    field_0x1ec = list;
}

// ---------------------------------------------------------------------------
// UIStatic

// 0x00478f60
UIStatic::UIStatic(int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UIControl(5, id, area, owner) {
}

// 0x00478fb0
UIStatic::~UIStatic() {
}

// 0x00478fc0
TextureMap* UIStatic::UnknownVirtualSlot48(int state) {
    if (stateImages[0])
        return stateImages[0]->GetCurrentTexture();
    return 0;
}

// 0x00478fe0
int UIStatic::UnknownVirtualSlot30() {
    return 0;
}

// ---------------------------------------------------------------------------
// UIControl (continued)

// 0x00470990
int UIControl::UnknownVirtualSlot28(POINT point, int state) {
    RECT rect = *(RECT*)field_0x2c;
    return PtInRect(&rect, point);
}

// 0x004709f0
int UIControl::UnknownVirtualSlot51(POINT point) {
    RECT rect = *(RECT*)field_0x2c;
    if (!shapeBounds) {
        if (PtInRect(&rect, point))
            return 1;
    } else if (UnknownVirtualSlot28(point, currentState)) {
        return 1;
    }
    return 0;
}

// 0x00471070
void UIControl::UnknownVirtualSlot27() {
    if (stateImages[currentState])
        stateImages[currentState]->Advance();
    if (field_0x68 == 1 || field_0x68 == 2) {
        if (fxAnimIn)
            fxAnimIn->Advance();
        if (fxAnimOut)
            fxAnimIn->Advance();
    }
}

// 0x004710c0
void UIControl::UnknownVirtualSlot38() {
    UIControl* last = UnknownFunction470850(0);
    if (!field_0x70 || (last && (!last->field_0x70 || last->field_0x68))) {
        if (--redrawFrames < 0) {
            redrawFrames = 0;
            needsRedraw = 1;
        }
        return;
    }
    UIAnim* image = stateImages[currentState];
    if (!image) {
        needsRedraw = 1;
        return;
    }
    if (image->frameCount == 1)
        needsRedraw = 1;
    else
        needsRedraw = 0;
}

// 0x00471140
int UIControl::UnknownVirtualSlot13() {
    int result = GameObject::UnknownVirtualSlot13();
    UIControl* last = UnknownFunction470850(0);
    if ((!last || (last->field_0x70 && !last->field_0x68)) && !post3D) {
        if (field_0x70)
            return UnknownVirtualSlot40();
        needsRedraw = 1;
    }
    return result;
}

// 0x00472250
int UIControl::UnknownVirtualSlot56(int a, int* position) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a && currentState == 2) {
        UnknownVirtualSlot29(3);
        UnknownVirtualSlot58(3, ownerDialog->field_0x180);
    }
    field_0x1d4 = -1;
    return 0;
}

// 0x004722b0
void UIControl::UnknownVirtualSlot57(int a, int* position) {
    if (moveable && a == 1) {
        UnknownVirtualSlot50();
        int width = UnknownVirtualSlot61();
        int height = UnknownVirtualSlot62();
        UnknownGameUiDialog* owner = ownerDialog;
        field_0x3c[0] = position[0] - owner->screenArea.left;
        field_0x3c[2] = field_0x3c[0] + width;
        field_0x3c[1] = position[1] - owner->screenArea.top;
        field_0x3c[3] = field_0x3c[1] + height;
    }
}

// 0x004725b0
int UIControl::UnknownVirtualSlot31() {
    if (UnknownVirtualSlot30() && ownerDialog->guiUser->field_0x1d8 != (UnknownGuiControl*)this) {
        UnknownVirtualSlot29(1);
        if (stateImages[currentState])
            stateImages[currentState]->Rewind();
        UnknownVirtualSlot58(1, ownerDialog->field_0x180);
        ToolTip* tip = ownerDialog->guiUser->userToolTip;
        if (tip)
            tip->UnknownFunction486b10((UnknownGuiControl*)this);
    }
    return 1;
}

// 0x00472630
void UIControl::UnknownVirtualSlot32(int value) {
    if (enabled)
        UnknownVirtualSlot29(0);
    ToolTip* tip = ownerDialog->guiUser->userToolTip;
    if (tip)
        tip->ShowText(0, 0, 1.0f);
}

// 0x00472670
void UIControl::UnknownVirtualSlot58(int index, int a) {
    if (sounds[index]) {
        sounds[index]->SetVolume(ownerGui->field_0x34c, 0);
        sounds[index]->PlayWithOptions(1, 0, 0);
    }
}

// 0x004726b0
int UIControl::UnknownVirtualSlot53() {
    return (int)new(__FILE__, 0x1267) GameObjectIterator(ownerDialog->controlContainer, 1, "UIControl");
}

// 0x00472730
void UIControl::UnknownFunction472730(GameObjectIterator* iterator) {
    delete iterator;
}

// 0x00472750
UIControl* UIControl::UnknownFunction472750(GameObjectIterator* iterator) {
    if (!attachId)
        return 0;
    UIControl* control;
    do {
        control = (UIControl*)iterator->Next();
    } while ((control && control->attachId != attachId) || control == this);
    return control;
}

// 0x00472790
UIControl* UIControl::UnknownFunction472790(GameObjectIterator* iterator) {
    UIControl* control = UnknownFunction472750(iterator);
    while (control && control->controlType != controlType)
        control = UnknownFunction472750(iterator);
    return control;
}

// 0x00472860
void UIControl::UnknownVirtualSlot63(CameraRect* in, CameraRect* out) {
    *out = *in;
    if (sourceBlit && drawnTexture) {
        out->right = (int)(drawnTexture->field_0x14 * ownerDialog->scaleX) + out->left;
        out->bottom = (int)(drawnTexture->field_0x18 * ownerDialog->scaleY) + out->top;
    }
}

// 0x004728e0
void UIControl::UnknownVirtualSlot64(CameraRect* in, CameraRect* out) {
    *out = *in;
    if (currentState == 2) {
        out->top++;
        out->left++;
        out->bottom++;
        out->right++;
    }
    out->left += textLeftMargin;
    out->top += textTopMargin;
    out->right -= textLeftMargin;
    out->bottom -= textTopMargin;
}

// ---------------------------------------------------------------------------
// UIButton (continued)

// 0x004731f0
UIButton::UIButton(int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UIControl(1, id, area, owner) {
    field_0x1ec = 0;
    UnknownFunction4732d0(0, 0, 0, 0);
    SetSound(0, 0);
    SetSound(1, 0);
    SetSound(2, 0);
    SetSound(3, 0);
    SetSound(4, 0);
}

// 0x00473410
int UIButton::UnknownVirtualSlot56(int a, int* position) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a && currentState == 2) {
        UnknownDialogEvent event;
        event.handled = a;
        if (field_0x1ec)
            field_0x1ec->Sort(1);
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->UnknownVirtualSlot29(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::UnknownVirtualSlot56(a, position);
}

// ---------------------------------------------------------------------------
// UIEditBox

// 0x00473630
UIEditBox::UIEditBox(int id, CameraRect* area, UnknownGameUiDialog* owner, int size, int a, int b)
    : UIControl(0xb, id, area, owner) {
    char* text = controlText;
    if (!text) {
        int capacity = size + 1;
        if (capacity >= 1000)
            capacity = 1000;
        else if (capacity < 1)
            capacity = 1;
        textCapacity = capacity;
    } else {
        int capacity = size + 1;
        int length = strlen(text);
        textCapacity = length > capacity ? length : capacity;
    }
    field_0x204 = a;
    field_0x208 = b;
    field_0x200 = 0;
    textLength = 0;
    field_0xd0 = 0;
    textWidth = 0;
    acceptedCharacters = 0;
    backgroundBrush = 0;
    field_0x228 = 0;
    field_0x22c = 0;
    if (!text)
        controlText = (char*)DebugMalloc(textCapacity, __FILE__, 0x15c7);
    controlText[0] = 0;
}

// 0x00473770
UIEditBox::~UIEditBox() {
    if (boundValue)
        UnknownVirtualSlot59(1);
    if (controlText) {
        DebugFree(controlText, __FILE__, 0x15d2);
        controlText = 0;
    }
    if (acceptedCharacters)
        DebugFree(acceptedCharacters, __FILE__, 0x15d6);
    if (backgroundBrush)
        DeleteObject((HGDIOBJ)backgroundBrush);
}

// 0x00473820
void UIEditBox::BindTextBuffer(char* buffer, int size) {
    UIControl::UnknownVirtualSlot54((int*)buffer);
    boundBufferSize = size;
}

// 0x00473840
void UIEditBox::UnknownVirtualSlot59(int value) {
    if (boundValue && boundBufferSize) {
        if (value) {
            strncpy((char*)boundValue, controlText, boundBufferSize);
            ((char*)boundValue)[boundBufferSize - 1] = 0;
        } else {
            SetEditText((char*)boundValue);
        }
    }
}

// 0x00473d80
void UIEditBox::UnknownFunction473d80(int value) {
    field_0x204 = value;
}

// 0x00473d90
void UIEditBox::UnknownFunction473d90(int value) {
    field_0x208 = value;
}

// 0x00473ef0
char* UIEditBox::GetEditText(char* buffer, int size) {
    if (field_0xd0 + 1 < size)
        size = field_0xd0 + 1;
    strncpy(buffer, controlText, size);
    buffer[size - 1] = 0;
    return buffer;
}

// 0x00474060
void UIEditBox::SetBackgroundColor(unsigned long color) {
    if (backgroundBrush)
        DeleteObject((HGDIOBJ)backgroundBrush);
    if (color)
        backgroundBrush = CreateSolidBrush(color);
    else
        backgroundBrush = 0;
    UnknownVirtualSlot50();
}

// 0x004740b0
void UIEditBox::UnknownVirtualSlot33(int value) {
    UnknownDialogEvent event;
    event.handled = 0;
    event.kind = kDialogEditDone;
    event.code = eventCode;
    event.controlName = GetName();
    event.dialog = ownerDialog;
    event.gui = ownerGui;
    event.control = this;
    ownerDialog->UnknownVirtualSlot29(&event);
    UnknownVirtualSlot50();
    UIControl::UnknownVirtualSlot33(value);
}

// 0x00474120
int UIEditBox::UnknownVirtualSlot55(int a, int b) {
    if (ownerDialog->guiUser->focusControl != (UnknownGuiControl*)this)
        return UIControl::UnknownVirtualSlot55(a, b);
    return 0;
}

// ---------------------------------------------------------------------------
// UIScrollCtl

// 0x00474820
UIScrollCtl::UIScrollCtl(int type, int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UIButton(id, area, owner) {
    controlType = type;
}

// 0x00474870
UIScrollCtl::~UIScrollCtl() {
}

// 0x00474b10
int UIScrollCtl::UnknownVirtualSlot56(int a, int* position) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a && currentState == 2) {
        UnknownDialogEvent event;
        event.handled = a;
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->UnknownVirtualSlot29(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::UnknownVirtualSlot56(a, position);
}

// ---------------------------------------------------------------------------
// UIScrollBar

// 0x00474ba0
UIScrollBar::UIScrollBar(int type, int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UIControl(type, id, area, owner) {
    field_0x1ec = 0;
    field_0x1f0 = 0;
    thumbWidth = 0;
    thumbHeight = 0;
    field_0x1fc = 0;
    field_0x208 = 0;
    field_0x204 = 0;
    dragY = 0;
    dragX = 0;
    field_0x218 = 0;
    field_0x21c = 0;
    scrollRange = 100;
    field_0x214 = 0.2f;
}

// 0x00474c50
UIScrollBar::~UIScrollBar() {
    if (boundValue)
        UnknownVirtualSlot59(1);
}

// 0x00474cb0
void UIScrollBar::UnknownVirtualSlot59(int value) {
    if (value)
        *boundValue = UnknownFunction475500();
    else
        UnknownFunction4753c0(*boundValue, scrollRange);
}

// 0x00475160
void UIScrollBar::UnknownFunction475160(int state) {
    UnknownGameUiDialog* owner = ownerDialog;
    thumbWidth = (int)(stateImages[state]->frameList[0]->frameWidth * owner->scaleX);
    thumbHeight = (int)(stateImages[state]->frameList[0]->frameHeight * owner->scaleY);
}

// 0x004751c0
void UIScrollBar::UnknownFunction4751c0(int range) {
    scrollRange = range;
    UnknownVirtualSlot50();
}

// 0x004754d0
void UIScrollBar::UnknownFunction4754d0(int range) {
    if (range > 0)
        field_0x1f0 = range - 1;
    else
        field_0x1f0 = 0;
    UnknownVirtualSlot50();
}

// 0x004755c0
int UIScrollBar::UnknownFunction4755c0() {
    return field_0x1f0;
}

// UIListBox's first shown row and UIMultiState's current state are read
// through the same body: 0x00475200 and UIScrollBar slot 55 call 0x004755c0
// on list boxes, UIRadioButton 0x004793f0 and the dialog procedures on
// multi-states and radio buttons. All three read +0x1f0; the linker folded
// the identical bodies.
int UIListBox::UnknownFunction4755c0() {
    return firstVisibleRow;
}

int UIMultiState::UnknownFunction4755c0() {
    return selectedState;
}

// ---------------------------------------------------------------------------
// UIListBox

// 0x00475fa0
void UIListBox::UnknownVirtualSlot27() {
    for (int i = 0; i < rowCount; i++) {
        UIAnim* image = rowTable[i].image;
        if (image)
            image->Advance();
    }
}

// 0x00475fe0
void UIListBox::UnknownVirtualSlot59(int value) {
    if (value) {
        *boundValue = GetSelectedRow();
    } else {
        lastClickTime = 0;
        SelectRow(*boundValue);
    }
}

// ---------------------------------------------------------------------------
// UIFrame and UIAnim

// 0x00472d00: `module` is the image's name and `id` the resource manager
// holding it; `a` is the sound group a missing image is loaded from as a sound.
UIFrame::UIFrame(void* module, int id, void* textures, int a, int b, int c, void* palette) {
    if (!b) {
        ownsSound = 0;
        isSound = 0;
        frameSound = 0;
        field_0x20 = (int)textures;
        UnknownResourceEntry* entry =
            ((UnknownResourceManager*)id)->UnknownFunction4e9360((const char*)module, 1);
        if (!UnknownFunction472bc0(entry->field_0x14, entry->field_0x18, palette) && a) {
            UnknownFunction4bb890((SoundGroup*)a, (const char*)module, 1, 3, c, -1);
            ownsSound = 1;
            isSound = 1;
            frameWidth = 0;
            frameHeight = 0;
            frameTexture = 0;
        }
    }
}

// 0x00472dc0
UIFrame::~UIFrame() {
    if (frameTexture)
        frameTexture->Release();
    if (ownsSound)
        frameSound->Release();
}

// 0x00472eb0
UIAnim::~UIAnim() {
    for (int i = 0; i < frameCount; i++) {
        if (frameList[i])
            frameList[i]->Release();
    }
}

// 0x004730b0
TextureMap* UIAnim::AdvancePastSounds() {
    UIFrame* frame = Advance();
    while (frame->isSound == 1) {
        frame->frameSound->PlayWithOptions(1, 0, 0);
        frame = Advance();
    }
    return frame->frameTexture;
}

// 0x004730e0
void UIAnim::LoadFile(const char* file, void* palette) {
    AddFrame(new(__FILE__, 0x14d1) UIFrame(file, animTextures, 0, 0, palette));
}

// 0x00473160
void UIAnim::LoadFromModule(void* module, int id, int a, void* palette) {
    AddFrame(new(__FILE__, 0x14e6) UIFrame(module, id, animTextures, a, 0, 0, palette));
}

// ---------------------------------------------------------------------------
// UIListBox (continued)

// 0x00475e20
UIListBox::~UIListBox() {
    if (boundValue)
        UnknownVirtualSlot59(1);
    if (itemBrush)
        DeleteObject((HGDIOBJ)itemBrush);
    if (selectBrush)
        DeleteObject((HGDIOBJ)selectBrush);
    for (int i = 0; i < rowCount; i++) {
        DebugFree(rowTable[i].text, __FILE__, 0x1b79);
        DebugFree(rowTable[i].imageFile, __FILE__, 0x1b7a);
        DebugFree(rowTable[i].textLines, __FILE__, 0x1b7b);
        UIAnim* image = rowTable[i].image;
        if (image && !image->animPalette)
            image->Release();
        if (rowTable[i].brush) {
            DeleteObject((HGDIOBJ)rowTable[i].brush);
            for (int j = 0; j < rowCount; j++) {
                if (rowTable[j].brush == rowTable[i].brush)
                    rowTable[j].brush = 0;
            }
        }
    }
    DebugFree(rowTable, __FILE__, 0x1b88);
}

// 0x00476860
int UIListBox::ScrollToRow(int row, int a) {
    int last = rowCount - lastPageRowCount;
    if (row < last)
        last = row;
    firstVisibleRow = last < 0 ? 0 : last;
    if (visibleRowCount == 1)
        UnknownVirtualSlot65(firstVisibleRow);
    UnknownVirtualSlot50();
    if (a)
        UpdateScrollBars();
    return 1;
}

// 0x004768d0
int UIListBox::GetRowData(int row) {
    if (row == -1) {
        row = selectedRow;
        if (row == -1)
            return 0;
    }
    return rowTable[row].data;
}

// 0x00476900
int UIListBox::FindRowByData(int data) {
    for (int i = 0; i < rowCount; i++) {
        if (rowTable[i].data == data)
            return i;
    }
    return -1;
}

// 0x00476930
void UIListBox::SetRowData(int row, int data) {
    rowTable[row].data = data;
}

// 0x00476950
int UIListBox::GetSelectedRow() {
    if (rowCount > 0)
        return selectedRow;
    return -1;
}

// 0x00476970
char* UIListBox::UnknownFunction476970(int row) {
    if (rowCount && row >= 0 && row < rowCount)
        return (char*)rowTable[row].kind;
    return 0;
}

// 0x004769a0
int UIListBox::GetRowHeight(int row) {
    if (rowCount && row >= 0 && row < rowCount)
        return rowTable[row].height;
    return 0;
}

// 0x00476b80
void UIListBox::SetSelectColor(unsigned int color) {
    selectColor = color;
    UnknownVirtualSlot50();
}

// 0x00476d20
char* UIListBox::GetRowText(int row) {
    if (selectedRow >= 0 && row == -1)
        return rowTable[selectedRow].text;
    if (rowCount && row >= 0 && row < rowCount)
        return rowTable[row].text;
    return 0;
}

// 0x004777f0
void UIListBox::SetSortCompare(int (*compare)(const void* a, const void* b)) {
    field_0x24c = compare;
}

// 0x00477b60
int UnknownFunction477b60(const void* a, const void* b) {
    const char* left = ((const UnknownGameUiListRow*)a)->text;
    if (!left)
        return 1;
    const char* right = ((const UnknownGameUiListRow*)b)->text;
    if (!right)
        return -1;
    return _stricmp(left, right);
}

// 0x00477b90
void UIListBox::SetAutoSort(int value) {
    autoSort = value;
}

// 0x00477ba0
void UIListBox::SetAllowWScroll(int value) {
    allowWScroll = value;
}

// 0x00477bb0
void UIListBox::SetSelectable(int a) {
    selectable = a;
}

// 0x00477ce0
void UIListBox::UnknownVirtualSlot50() {
    UIControl::UnknownVirtualSlot50();
    if (ownerDialog && ownerDialog->dialogBackground) {
        for (int i = 0; i < rowCount; i++)
            rowTable[i].field_0x30 = 0;
    }
}

// 0x00477e60
void UIListBox::UnknownFunction477e60(int a) {
    field_0x240 = a;
    if (!visibleRowCount)
        visibleRowCount = 1;
}

// 0x00478190
void UIListBox::UnknownVirtualSlot57(int a, int* position) {
    if (a == 1 && position) {
        int delta = position[0] - field_0x22c;
        field_0x234 = delta;
        if (delta >= -field_0x230)
            delta = -field_0x230;
        field_0x234 = delta;
        UnknownVirtualSlot50();
    }
    UIControl::UnknownVirtualSlot57(a, position);
}

// 0x00479220
void UIListBox::UnknownVirtualSlot29(int state) {
    needsRedraw = 1;
    currentState = state;
}

// ---------------------------------------------------------------------------
// UIMultiState

// 0x004781f0
UIMultiState::UIMultiState(int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UIControl(2, id, area, owner) {
    stateCount = 0;
    selectedState = 0;
    stateTable = 0;
    SetStateCount(2);
}

// 0x00478260
int UIMultiState::UnknownVirtualSlot34() {
    return (int)stateTable[selectedState].text;
}

// 0x00478280
int UIMultiState::UnknownVirtualSlot35() {
    return stateTable[selectedState].field_0x0c;
}

// 0x004782a0
int UIMultiState::UnknownVirtualSlot36() {
    return (int)stateTable[selectedState].textLines;
}

// 0x004782c0
void* UIMultiState::UnknownVirtualSlot37() {
    return stateTable[selectedState].textExtent;
}

// 0x00478300
UIMultiState::~UIMultiState() {
    if (stateCount && (!ownerDialog || !ownerDialog->imageCount)) {
        for (int i = 0; i < stateCount; i++) {
            UIAnim* image = stateTable[i].image;
            if (image && !image->animPalette)
                DebugFree(image, __FILE__, 0x210e);
            image = stateTable[i].focusImage;
            if (image && !image->animPalette)
                DebugFree(image, __FILE__, 0x210f);
            if (stateTable[i].text)
                DebugFree(stateTable[i].text, __FILE__, 0x2110);
            if (stateTable[i].textLines)
                DebugFree(stateTable[i].textLines, __FILE__, 0x2111);
        }
    }
    if (stateTable)
        DebugFree(stateTable, __FILE__, 0x2114);
}

// 0x00478450
void UIMultiState::UnknownVirtualSlot38() {
    UIControl* last = UnknownFunction470850(0);
    if (!field_0x70 || (last && (!last->field_0x70 || last->field_0x68))) {
        if (--redrawFrames < 0) {
            redrawFrames = 0;
            needsRedraw = 1;
        }
        return;
    }
    UIAnim* image;
    if (!stateTable || !(image = stateTable[selectedState].image)) {
        needsRedraw = 1;
        return;
    }
    if (image->frameCount == 1)
        needsRedraw = 1;
    else
        needsRedraw = 0;
}

// 0x004784e0
void UIMultiState::UnknownVirtualSlot27() {
    if (!stateTable)
        return;
    UIAnim* image;
    if (currentState == 4)
        image = stateTable[selectedState].focusImage;
    else
        image = stateTable[selectedState].image;
    if (image)
        image->Advance();
}

// 0x00478520
void UIMultiState::UnknownVirtualSlot29(int state) {
    redrawFrames = 3;
    currentState = state;
}

// 0x00478540
void UIMultiState::UnknownVirtualSlot59(int value) {
    if (value)
        *boundValue = UnknownFunction4755c0();
    else
        SetCurrentState(*boundValue);
}

// 0x004789f0
void UIMultiState::SetStateEntry(int index, UIAnim* image, const char* text) {
    if (index < stateCount && image) {
        UIAnim* old = stateTable[index].image;
        if (old && !old->animPalette)
            old->Release();
        stateTable[index].image = image;
    }
    if (text)
        SetStateText(index, text);
}

// 0x00478cf0
void UIMultiState::SetCurrentState(int value) {
    if (value < stateCount)
        selectedState = value;
    UnknownVirtualSlot50();
}

// 0x00478d10
void UIMultiState::SelectNextState() {
    UnknownGameUiState* states = stateTable;
    if (!states)
        return;
    int start = selectedState;
    int count = stateCount;
    int index = (start + 1) % count;
    selectedState = index;
    while (!states[index].selectable && index != start) {
        index = (index + 1) % count;
        selectedState = index;
    }
    UnknownVirtualSlot50();
}

// ---------------------------------------------------------------------------
// UIStaticText

// 0x00478ff0
UIStaticText::UIStaticText(int id, CameraRect* area, UnknownGameUiDialog* owner, const char* text,
                           unsigned int color)
    : UIControl(0xc, id, area, owner) {
    textColor = color;
    if (text) {
        controlText = (char*)DebugMalloc(strlen(text) + 1, __FILE__, 0x22ec);
        strcpy(controlText, text);
        textDrop = 0;
    } else {
        controlText = 0;
        textDrop = 0;
    }
}

// 0x004790f0
UIStaticText::~UIStaticText() {
}

// 0x00479100
void UIStaticText::UnknownVirtualSlot38() {
    int empty = 0;
    if (!controlText || !field_0xd0)
        empty = 1;
    UIControl* last = UnknownFunction470850(0);
    if (!field_0x70 || empty || (last && (!last->field_0x70 || last->field_0x68))) {
        if (--redrawFrames < 0) {
            redrawFrames = 0;
            needsRedraw = 1;
        }
    }
}

// 0x00479170
int UIStaticText::UnknownVirtualSlot40() {
    UIControl::UnknownVirtualSlot40();
    needsRedraw = 1;
    return 1;
}

// 0x00479190
int UIStaticText::UnknownVirtualSlot55(int a, int b) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.handled = a;
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->UnknownVirtualSlot29(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::UnknownVirtualSlot55(a, b);
}

// ---------------------------------------------------------------------------
// UIRadioButton

// 0x00479240
UIRadioButton::UIRadioButton(int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UIMultiState(id, area, owner) {
    AppendClassName(this);
    controlType = 4;
}

// 0x004792d0
UIRadioButton::~UIRadioButton() {
}

// 0x004792e0
void UIRadioButton::UnknownVirtualSlot59(int value) {
    if (value)
        *boundValue = GetGroupSelection();
    else
        SelectInGroup(*boundValue);
}

// 0x004795c0
int UIRadioButton::UnknownVirtualSlot53() {
    return (int)new(__FILE__, 0x23b9) GameObjectIterator(ownerDialog->controlContainer, 1, "UIRadioButton");
}

// ---------------------------------------------------------------------------
// The drop-down list's parts

// 0x00479640
UIDDLScrollBar::UIDDLScrollBar(int type, int id, CameraRect* area, UnknownGameUiDialog* owner,
                               UIDropDownList* list)
    : UIScrollBar(type, id, area, owner) {
    ownerList = list;
    AppendClassName(this);
}

// 0x004796e0
void UIDDLScrollBar::UnknownVirtualSlot33(int value) {
    UIControl::UnknownVirtualSlot33(value);
    if (!ownerList->UnknownFunction47a800((UIControl*)value))
        ownerList->UnknownFunction47a2d0(0);
}

// 0x00479b50
UIDDLStatic::UIDDLStatic(int id, CameraRect* area, UnknownGameUiDialog* owner, UIDropDownList* list)
    : UIStatic(id, area, owner), ownerList(list) {
    AppendClassName(this);
}

// 0x00479bc0
void UIDDLStatic::UnknownVirtualSlot33(int value) {
    UIControl::UnknownVirtualSlot33(value);
    if (!ownerList->UnknownFunction47a800((UIControl*)value))
        ownerList->UnknownFunction47a2d0(0);
}

// 0x00479bf0
UIDDLButton::UIDDLButton(int id, CameraRect* area, UnknownGameUiDialog* owner, UIDropDownList* list)
    : UIButton(id, area, owner), ownerList(list) {
    AppendClassName(this);
}

// 0x00479c90
int UIDDLButton::UnknownVirtualSlot55(int a, int b) {
    UIControl::UnknownVirtualSlot55(a, b);
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this)
        ownerList->UnknownFunction47a2d0(1 - ownerList->isOpen);
    return 0;
}

// 0x00479ce0
void UIDDLButton::UnknownVirtualSlot33(int value) {
    UIControl::UnknownVirtualSlot33(value);
    if (!ownerList->UnknownFunction47a800((UIControl*)value))
        ownerList->UnknownFunction47a2d0(0);
}

// 0x00479d10
UIDDLListBox::UIDDLListBox(int id, int rows, CameraRect* area, UnknownGameUiDialog* owner,
                           UIDropDownList* list)
    : UIListBox(id, rows, area, owner) {
    ownerList = list;
    AppendClassName(this);
}

// 0x00479db0
int UIDDLListBox::UnknownVirtualSlot65(int row) {
    int result = UIListBox::UnknownVirtualSlot65(row);
    if (result)
        ownerList->SetText(GetRowText(row));
    return result;
}

// 0x00479e70
void UIDDLListBox::UnknownVirtualSlot33(int value) {
    UIControl::UnknownVirtualSlot33(value);
    if (!ownerList->UnknownFunction47a800((UIControl*)value))
        ownerList->UnknownFunction47a2d0(0);
}

// ---------------------------------------------------------------------------
// UIDropDownList

// 0x00479ea0
UIDropDownList::UIDropDownList(int id, CameraRect* area, UnknownGameUiDialog* owner, const char* text,
                               unsigned int color)
    : UIStaticText(id, area, owner, text, color) {
    AppendClassName(this);
    CameraRect rect;
    controlType = 6;
    rowHeight = 0x14;
    rect.left = 0;
    rect.top = 0;
    rect.right = 10;
    rect.bottom = 10;
    buttonPart = new(__FILE__, 0x24e3) UIDDLButton(0, &rect, owner, this);
    listPart = new(__FILE__, 0x24e4) UIDDLListBox(0, 100, &rect, owner, this);
    scrollBarPart = new(__FILE__, 0x24e5) UIDDLScrollBar(7, 0, &rect, owner, this);
    staticPart0 = new(__FILE__, 0x24e6) UIDDLStatic(0, &rect, owner, this);
    staticPart1 = new(__FILE__, 0x24e7) UIDDLStatic(0, &rect, owner, this);
    staticPart2 = new(__FILE__, 0x24e8) UIDDLStatic(0, &rect, owner, this);
    staticPart0->SetAnchor(this, 1);
    staticPart1->SetAnchor(this, 1);
    staticPart2->SetAnchor(this, 1);
    buttonPart->SetAnchor(this, 1);
    listPart->SetAnchor(this, 1);
    scrollBarPart->SetAnchor(this, 1);
    listPart->UnknownVirtualSlot52(0x4d43);
    scrollBarPart->UnknownVirtualSlot52(0x4d43);
    AppendChild(buttonPart, -1);
    AppendChild(staticPart2, -1);
    isOpen = 0;
    UnknownFunction47a2d0(0);
    owner->UnknownFunction46a840(buttonPart, 0, 0);
    owner->UnknownFunction46a840(listPart, 0, 0);
    owner->UnknownFunction46a840(scrollBarPart, 0, 0);
    owner->UnknownFunction46a840(staticPart0, 0, 0);
    owner->UnknownFunction46a840(staticPart2, 0, 0);
    owner->UnknownFunction46a840(staticPart1, 0, 0);
}

// 0x0047a1f0
UIDropDownList::~UIDropDownList() {
    if (boundValue)
        UnknownVirtualSlot59(1);
    if (!isOpen) {
        staticPart0->Release();
        staticPart1->Release();
        listPart->Release();
        scrollBarPart->Release();
    }
}

// 0x0047a290
void UIDropDownList::UnknownVirtualSlot52(int id) {
    attachId = id;
    if (!id)
        id = 0x4d43;
    listPart->UnknownVirtualSlot52(id);
    scrollBarPart->UnknownVirtualSlot52(id);
}

// 0x0047a7d0
void UIDropDownList::UnknownVirtualSlot33(int value) {
    UIControl::UnknownVirtualSlot33(value);
    if (!UnknownFunction47a800((UIControl*)value))
        UnknownFunction47a2d0(0);
}

// 0x0047a800
int UIDropDownList::UnknownFunction47a800(UIControl* control) {
    if (control == this || control == scrollBarPart || control == buttonPart ||
        control == listPart || control == staticPart0 || control == staticPart1 ||
        control == staticPart2)
        return 1;
    return 0;
}

// 0x0047a850
int UIDropDownList::UnknownVirtualSlot61() {
    if (!buttonPart->field_0x3c[0])
        return field_0x3c[2] - field_0x3c[0];
    return buttonPart->field_0x3c[2] - field_0x3c[0];
}

// 0x0047a870
int UIDropDownList::UnknownVirtualSlot62() {
    return listPart->field_0x3c[3] - field_0x3c[1];
}

// 0x0047ad80
void UIDropDownList::UnknownVirtualSlot59(int value) {
    if (value) {
        *boundValue = listPart->GetSelectedRow();
    } else {
        listPart->SelectRow(*boundValue);
        listPart->UnknownVirtualSlot66(0);
    }
}

// 0x0047add0
int UIDropDownList::UnknownVirtualSlot55(int a, int b) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this) {
        UnknownFunction47a2d0(isOpen == 0);
        ownerDialog->guiUser->UnknownFunction487790((UnknownGuiControl*)this, 0, 0);
        return 1;
    }
    return UIControl::UnknownVirtualSlot55(a, b);
}

// ---------------------------------------------------------------------------
// UIProgressBar's slot 48 and UICtlContainer's input slots

// 0x0047b3f0
int UICtlContainer::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    GameObject* child = field_0x10;
    if (child) {
        while (child->field_0x0C)
            child = child->field_0x0C;
        for (; child; child = child->field_0x08) {
            if (child->UnknownVirtualSlot23(event, entry))
                return 1;
        }
    }
    return 0;
}

// 0x0047b440
int UICtlContainer::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    GameObject* child = field_0x10;
    if (child) {
        while (child->field_0x0C)
            child = child->field_0x0C;
        for (; child; child = child->field_0x08) {
            if (child->UnknownVirtualSlot22(event, entry))
                return 1;
        }
    }
    return 0;
}

// 0x0047b570
void* UnknownFunction47b570(void* block, unsigned int size) {
    if (block && size <= 0) {
        DebugFree(block, __FILE__, 0x2782);
        return 0;
    }
    void* resized = DebugMalloc(size, __FILE__, 0x2787);
    if (resized && block) {
        int length = _msize(block);
        memcpy(resized, block, length < (int)size ? length : size);
        DebugFree(block, __FILE__, 0x278e);
    }
    return resized;
}

// ---------------------------------------------------------------------------
// UIDialog

// 0x0046a010
int UIDialog::Release() {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    UnknownDialogEvent event;
    event.handled = 0;
    UpdateBoundValues(1);
    dialog->UnknownVirtualSlot31(1);
    event.gui = (GUIManager*)dialog->guiManager;
    event.kind = kDialogClose;
    event.code = 0;
    event.controlName = 0;
    event.dialog = this;
    event.control = 0;
    dialog->UnknownVirtualSlot29(&event);
    return GameObject::Release();
}

// 0x0046a780
int UIDialog::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (event->kind && (unsigned int)event->control < 0x20)
        dialog->guiUser->field_0x3c[event->control] = 1;
    if (!field_0x25_bit2) {
        if (event->kind == 1)
            UnknownFunction46f120();
        return GameObject::UnknownVirtualSlot23(event, entry);
    }
    return 0;
}

// 0x0046a7d0
int UIDialog::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (event->kind && (unsigned int)event->control < 0x20)
        dialog->guiUser->field_0x3c[event->control] = 0;
    if (!field_0x25_bit2) {
        if (event->kind == 1)
            UnknownFunction46f120();
        if (event->kind) {
            RemoveTimers(0x101);
            RemoveTimers(0x102);
        }
        return GameObject::UnknownVirtualSlot22(event, entry);
    }
    return 0;
}

// 0x0046a840
UIControl* UIDialog::UnknownFunction46a840(UIControl* control, int group, int region) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (control) {
        control->UnknownInlineSetField18(field_0x18);
        control->post3D = dialog->field_0x7f18;
        control->ownerGui = dialog->guiManager;
        control->groupId = group;
        control->fontHeight = dialog->dialogFontHeight;
        if (dialog->dialogBackground)
            control->backgroundRegion = dialog->dialogBackground->UnknownFunction4040f0(region);
        return control;
    }
    return 0;
}

// 0x0046a8a0
GameObject* UIDialog::AddControl(GameObject* control, int a, int b) {
    if (control) {
        UnknownFunction46a840((UIControl*)control, a, b);
        ((UnknownGameUiDialog*)this)->controlContainer->AppendChild(control, -1);
        return control;
    }
    return 0;
}

// 0x0046e9a0
Sound* UIDialog::FindSectionObject(const char* name) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (!name)
        return 0;
    for (int i = 0; i < dialog->sectionCount; i++) {
        if (!_stricmp(name, dialog->sectionTable[i].sectionName))
            return dialog->sectionTable[i].sectionObject;
    }
    if (dialog->guiManager)
        return (Sound*)dialog->guiManager->FindSectionObject((int)name);
    return 0;
}

// 0x0046ea60
void UIDialog::UnknownFunction46ea60(int value) {
    ((UnknownGameUiDialog*)this)->isShown = value;
    if (value)
        GameObject::UnknownVirtualSlot5();
    else
        GameObject::UnknownVirtualSlot4();
}

// 0x0046eb30
void UIDialog::ShowGroup(int id, int value) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    GameObjectIterator iterator(dialog->controlContainer, 1, "UIControl");
    UIControl* control;
    while ((control = (UIControl*)iterator.Next()) != 0) {
        if (control->groupId == id)
            control->Show(value, 1);
    }
    if (!value && dialog->dialogBackground)
        dialog->dialogBackground->UnknownFunction404da0();
}

// 0x0046ebf0
UIControl* UIDialog::FindControl(const char* name, int flags) {
    UIControl* found = 0;
    GameObjectIterator iterator(((UnknownGameUiDialog*)this)->controlContainer, 1, "UIControl");
    UIControl* control = (UIControl*)iterator.Next();
    while (control) {
        if (!_stricmp(control->GetName(), name)) {
            if (!flags || control->controlType == flags || (flags == 8 && control->controlType == 7))
                found = control;
            break;
        }
        control = (UIControl*)iterator.Next();
    }
    return found;
}

// 0x0046ecc0
void UIDialog::UpdateBoundValues(int value) {
    GameObjectIterator iterator(((UnknownGameUiDialog*)this)->controlContainer, 1, "UIControl");
    UIControl* control;
    while ((control = (UIControl*)iterator.Next()) != 0) {
        if (control->boundValue)
            control->UnknownVirtualSlot59(value);
    }
    ((UnknownGameUiDialog*)this)->UnknownVirtualSlot31(value);
}

// 0x0046f1c0
int UIDialog::UnknownVirtualSlot13() {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (!field_0x25_bit2) {
        GameObjectIterator iterator(dialog->controlContainer, 1, "UIControl");
        if (dialog->dialogBackground) {
            UIControl* control;
            while ((control = (UIControl*)iterator.Next()) != 0) {
                if (control->backgroundRegion >= 0 && (!control->needsRedraw || control->redrawFrames))
                    dialog->dialogBackground->UnknownFunction404240(control->backgroundRegion,
                                                               (CameraRect*)&control->field_0x2c);
            }
        }
        GameObject::UnknownVirtualSlot13();
    } else if (dialog->screenGrab && !dialog->field_0x7f18) {
        if (dialog->dialogBackground)
            dialog->dialogBackground->UnknownFunction404480(dialog->screenGrab, &dialog->screenArea, 0, 0x1000000,
                                                       dialog->grabRegion, 1, &dialog->field_0x7f4c, 0);
        else
            ((RenderTarget*)dialog->UnknownInlineField18())->UnknownVirtualSlot3(&dialog->screenArea,
                                                                                 dialog->screenGrab, 0, 0x1000000);
    }
    return 1;
}

// 0x0046f300
int UIDialog::UnknownVirtualSlot14() {
    if (!field_0x25_bit2)
        return GameObject::UnknownVirtualSlot14();
    return 1;
}

// 0x0046f320
int UIDialog::UnknownVirtualSlot15() {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (!field_0x25_bit2) {
        GameObject::UnknownVirtualSlot15();
    } else if (dialog->screenGrab && dialog->field_0x7f18) {
        if (dialog->dialogBackground)
            dialog->dialogBackground->UnknownFunction404480(dialog->screenGrab, &dialog->screenArea, 0, 0x1000000,
                                                       dialog->grabRegion, 1, &dialog->field_0x7f4c, 0);
        else
            ((RenderTarget*)dialog->UnknownInlineField18())->UnknownVirtualSlot3(&dialog->screenArea,
                                                                                 dialog->screenGrab, 0, 0x1000000);
    }
    if (dialog->isClosing && dialog->closeFrame != ((RenderTarget*)dialog->UnknownInlineField18())->field_0x1c)
        UnknownVirtualSlot26();
    return 1;
}

// 0x0046fe40
void UIDialog::RemoveTimers(int id) {
    UITimer* timer;
    for (int i = 0; (timer = ((UnknownGameUiDialog*)this)->timerList.Get(i)) != 0; i++) {
        if (timer->timerId == id)
            ((UnknownGameUiDialog*)this)->timerList.Remove(timer);
    }
}

// 0x0046fec0
void UIDialog::RemoveTimer(void* timer) {
    UITimer* entry;
    for (int i = 0; (entry = ((UnknownGameUiDialog*)this)->timerList.Get(i)) != 0; i++) {
        if (entry == timer)
            ((UnknownGameUiDialog*)this)->timerList.Remove((UITimer*)timer);
    }
}

// 0x0046ff30
void UIDialog::EndDialog(int result) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (!dialog->isClosing) {
        dialog->dialogResult = result;
        dialog->isClosing = 1;
        dialog->closeFrame = ((RenderTarget*)field_0x18)->field_0x1c;
    }
}

// 0x0046ff60
void UIDialog::UnknownFunction46ff60() {
    ((UnknownGameUiDialog*)this)->field_0x154 = 0;
    UnknownVirtualSlot26();
}

// 0x0046ffc0
void UIDialog::UnknownFunction46ffc0(int value) {
    ((UnknownGameUiDialog*)this)->popupAlignment = value;
}

// 0x0046ffd0
void UIDialog::UnknownFunction46ffd0(void* background) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (dialog->parentDialog) {
        dialog->parentDialog->UnknownFunction46ffd0(background);
        dialog->dialogBackground = (BackgroundImage*)background;
    } else {
        dialog->dialogBackground = (BackgroundImage*)background;
    }
}

// 0x00470000
int UIDialog::UnknownFunction470000(UIControl* control, int a, int b) {
    return ((UnknownGameUiDialog*)this)->guiUser->UnknownFunction487730((UnknownGuiControl*)control,
                                                                    (UnknownGuiControl**)a, b);
}

// 0x00470020
void UnknownGameUiDialog::UnknownVirtualSlot29(UnknownDialogEvent* event) {
    if (field_0xcc)
        field_0xcc(event);
}

// 0x00470040
void UnknownGameUiDialog::UnknownVirtualSlot30() {
    if (field_0xd4)
        field_0xd4();
}

// 0x00470050
void UnknownGameUiDialog::UnknownVirtualSlot31(int value) {
    if (field_0xd0)
        field_0xd0(this, value);
}

// 0x00470070
void UIDialog::UnknownFunction470070(int a, int b, CameraRect* rect) {
    if (a)
        GrabBackground(b, rect);
    else
        ReleaseBackgroundGrab();
    GameObject::UnknownVirtualSlot16(a);
}

// 0x004700b0
void UIDialog::GrabBackground(int dim, CameraRect* rect) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    ReleaseBackgroundGrab();
    if (!rect)
        rect = &dialog->screenArea;
    dialog->screenGrab = dialog->guiManager->CopyScreenToTexture(dim, rect);
    if (dialog->dialogBackground) {
        dialog->grabRegion = dialog->dialogBackground->UnknownFunction4040f0(1);
        dialog->dialogBackground->UnknownFunction404da0();
        dialog->dialogBackground->field_0x30 = 0;
    }
}

// 0x00470110
void UIDialog::ReleaseBackgroundGrab() {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (dialog->screenGrab)
        dialog->screenGrab->Release();
    if (dialog->dialogBackground && dialog->grabRegion >= 0) {
        dialog->dialogBackground->UnknownFunction404200(dialog->grabRegion);
        dialog->dialogBackground->field_0x30 = 1;
    }
    dialog->screenGrab = 0;
    dialog->grabRegion = -1;
    dialog->field_0x7f4c = 0;
}

// ---------------------------------------------------------------------------
// UIControl: text and transitions

// 0x00470660
void UIControl::Show(int a, int b) {
    if (field_0x1d0) {
        if (a) {
            field_0x68 = 1;
            if (field_0xb4 == 0x6a)
                field_0x1d0 = &UIControl::UnknownVirtualSlot46;
            drawnTexture = 0;
            field_0x70 = a;
            return;
        }
        if (field_0xb4 == 0x6a && b && ownerDialog->animatesControls) {
            drawnTexture = 0;
            field_0x68 = 2;
            field_0x1d0 = &UIControl::UnknownVirtualSlot47;
            return;
        }
        field_0x68 = 2;
        field_0x70 = 0;
        UnknownVirtualSlot50();
        drawnTexture = 0;
        return;
    }
    field_0x68 = 0;
    field_0x70 = a;
    UnknownVirtualSlot50();
}

// 0x00470760
void UIControl::SetImageFile(int a, const char* image) {
    if (ownerDialog) {
        UIAnim* anim = new(__FILE__, 0xc10) UIAnim(ownerDialog->dialogTextures, 0);
        anim->LoadFile(image, ownerDialog->dialogPalette);
        SetImage(a, anim);
    }
    UnknownVirtualSlot50();
}

// 0x00470870
void UIControl::StartTransition(int type, int a, int b, int delay, int c) {
    if (b) {
        Show(1, 1);
        field_0x68 = 1;
    }
    field_0xb4 = type;
    switch (type) {
    case 100:
        field_0x1d0 = &UIControl::UnknownVirtualSlot41;
        break;
    case 101:
        field_0x1d0 = &UIControl::UnknownVirtualSlot42;
        break;
    case 102:
        field_0x1d0 = &UIControl::UnknownVirtualSlot43;
        break;
    case 103:
        field_0x1d0 = &UIControl::UnknownVirtualSlot44;
        break;
    case 104:
        field_0x1d0 = &UIControl::UnknownVirtualSlot45;
        break;
    case 106:
        field_0x1d0 = &UIControl::UnknownVirtualSlot46;
        break;
    default:
        field_0x1d0 = 0;
        break;
    }
    transitionTime = 0;
    field_0xb0 = c;
    if (delay == 0xffff)
        transitionDelay = rand() % 2000;
    else
        transitionDelay = delay;
    slideSound = (Sound*)a;
    UnknownVirtualSlot50();
}

// 0x00470a80
void UIControl::SetTextFromResource(void* module, int id) {
    char text[0x400];
    if (LoadStringA((HINSTANCE)module, id, text, 0x400)) {
        if (controlType == 0xb)
            static_cast<UIEditBox*>(this)->SetEditText(text);
        else
            SetText(text);
    } else {
        sprintf(text, "Resource string '%d' load fail\n", id);
        if (controlType == 0xb)
            static_cast<UIEditBox*>(this)->SetEditText(text);
        else
            SetText("Resource String Unavailable");
    }
    UnknownVirtualSlot50();
}

// 0x00470b20
void UIControl::SetText(const char* text) {
    if (text) {
        if (controlType == 0xb) {
            static_cast<UIEditBox*>(this)->SetEditText((char*)text);
        } else {
            if (controlText)
                DebugFree(controlText, __FILE__, 0xcc2);
            if (textLines)
                DebugFree(textLines, __FILE__, 0xcc3);
            controlText = (char*)DebugMalloc(strlen(text) + 1, __FILE__, 0xcc4);
            strcpy(controlText, text);
            textLines = ownerDialog->SplitTextLines(controlText, textColor, (void*)fontHandle,
                                                    field_0x1c4, textExtent);
            if (!textLines) {
                field_0xd0 = textExtent[0] = strlen(controlText);
                HDC dc;
                if (!g_TrackGame->PCTarget()->renderSurface->GetDC((void**)&dc)) {
                    HGDIOBJ font =
                        (HGDIOBJ)(fontHandle ? fontHandle : (ownerDialog ? (int)ownerDialog->dialogFont : 0));
                    HGDIOBJ old = SelectObject(dc, font);
                    SIZE size;
                    GetTextExtentPoint32A(dc, controlText, field_0xd0, &size);
                    SelectObject(dc, old);
                    g_TrackGame->PCTarget()->renderSurface->ReleaseDC(dc);
                    textExtent[1] = size.cx;
                } else {
                    textExtent[1] = 0;
                }
            }
        }
        UnknownVirtualSlot50();
    } else {
        if (controlText)
            DebugFree(controlText, __FILE__, 0xcde);
        if (textLines)
            DebugFree(textLines, __FILE__, 0xcdf);
        controlText = 0;
        textLines = 0;
    }
}

// 0x00470e00
int UIControl::UnknownVirtualSlot10(float frameTime) {
    UnknownVirtualSlot27();
    int draw = 1;
    if (field_0x70) {
        UIControl* last = UnknownFunction470850(0);
        if (!last || (last->field_0x70 && !last->field_0x68))
            draw = !UnknownFunction471500();
    }
    if (draw) {
        drawnTexture = UnknownVirtualSlot48(-1);
        UnknownVirtualSlot38();
        UnknownVirtualSlot39();
        if (ownerDialog->guiUser->pointerDevice && field_0x1d4 != -1) {
            POINT position = ownerDialog->guiUser->pointerDevice->pointerPosition;
            UnknownVirtualSlot57(field_0x1d4, (int*)&position);
        }
    }
    field_0x2c[0] = field_0x2c[0] < 0 ? 0 : field_0x2c[0];
    field_0x2c[1] = field_0x2c[1] < 0 ? 0 : field_0x2c[1];
    int right = field_0x2c[2];
    field_0x2c[2] = g_TrackGame->renderTarget->field_0x0c < right ? g_TrackGame->renderTarget->field_0x0c : right;
    int bottom = field_0x2c[3];
    field_0x2c[3] = g_TrackGame->renderTarget->field_0x10 < bottom ? g_TrackGame->renderTarget->field_0x10 : bottom;
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x004711a0
int UIControl::UnknownVirtualSlot15() {
    int result = GameObject::UnknownVirtualSlot15();
    UIControl* last = UnknownFunction470850(0);
    if ((!last || (last->field_0x70 && !last->field_0x68)) && post3D) {
        if (field_0x70)
            result = UnknownVirtualSlot40();
        else
            needsRedraw = 1;
    }
    BackgroundImage* background = ownerDialog->dialogBackground;
    if (background && backgroundRegion > -1 &&
        (!field_0x70 || (last && (!last->field_0x70 || last->field_0x68))))
        background->UnknownFunction404cb0(backgroundRegion);
    return result;
}

// 0x00471500
int UIControl::UnknownFunction471500() {
    int done = 1;
    UnknownGameUiDialog* owner = ownerDialog;
    int (UIControl::*step)();
    if (owner->animatesControls && (step = field_0x1d0) != 0 && field_0x68 && controlType != 3 && controlType != 0xc) {
        transitionTime += (int)(owner->lastFrameTime * 1000.0f);
        done = 0;
        if (transitionTime < transitionDelay)
            goto end;
        done = (this->*step)();
        if (!done)
            goto end;
        field_0x164 = 0;
        field_0x68 = 0;
    } else if (field_0x68) {
        field_0x68 = 0;
        done = 1;
        UnknownVirtualSlot50();
    }
    *(CameraRect*)field_0x2c = *(CameraRect*)field_0x3c;
end:
    return !done;
}

// ---------------------------------------------------------------------------
// UIControl: input

// 0x004721b0
int UIControl::UnknownVirtualSlot55(int a, int b) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this) {
        if (!a) {
            UnknownVirtualSlot29(2);
            UnknownVirtualSlot58(2, ownerDialog->field_0x180);
        }
        field_0x1d4 = a;
        if (a == 1 && ownerDialog->dialogBackground)
            ownerDialog->dialogBackground->UnknownFunction404da0();
        ownerDialog->guiUser->UnknownFunction487790((UnknownGuiControl*)this, 0, 0);
        ToolTip* tip = ownerDialog->guiUser->userToolTip;
        if (tip) {
            tip->shown = 0;
            tip->showDelay = -1.0f;
        }
    }
    return 0;
}

// 0x00472320
int UIControl::UnknownVirtualSlot20(int value) {
    UnknownDialogEvent event;
    event.handled = 0;
    if (ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
        event.kind = 0xb;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        event.key = value;
        event.field_0x18 = 1;
        ownerDialog->UnknownVirtualSlot29(&event);
        if (event.handled)
            return 1;
        if (!event.field_0x18)
            return 1;
    }
    return GameObject::UnknownVirtualSlot20(value);
}

// 0x004723d0
int UIControl::UnknownVirtualSlot21(int value) {
    UnknownDialogEvent event;
    event.handled = 0;
    if (ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
        event.kind = 0xc;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        event.key = value;
        event.field_0x18 = 1;
        ownerDialog->UnknownVirtualSlot29(&event);
        if (event.handled)
            return 1;
        if (!event.field_0x18)
            return 1;
    }
    return GameObject::UnknownVirtualSlot21(value);
}

// 0x00472480
int UIControl::UnknownFunction472480(int* position) {
    int result = 0;
    GameObject* next = field_0x0C;
    while (next) {
        if (strstr(next->field_0x28, "UIControl,"))
            break;
        next = next->field_0x0C;
    }
    if (next) {
        result = ((UIControl*)next)->UnknownFunction472480(position);
        if (result)
            return result;
    }
    if (field_0x10 && strstr(field_0x10->field_0x28, "UIControl,")) {
        result = ((UIControl*)field_0x10)->UnknownFunction472480(position);
        if (result)
            return result;
    }
    if (UnknownVirtualSlot51(*(POINT*)position) && UnknownVirtualSlot30()) {
        UIControl* last = UnknownFunction470850(0);
        if (enabled && field_0x70 && (!last || (last->field_0x70 && !last->field_0x68))) {
            GUIUser* user = ownerDialog->guiUser;
            UnknownGuiControl* hover = user->field_0x1d8;
            if (!user->field_0x3c[0] && !user->field_0x3c[1] && hover != (UnknownGuiControl*)this) {
                result = ownerDialog->UnknownFunction470000(this, 0, 0);
            } else if (hover == (UnknownGuiControl*)this) {
                if (user->field_0x1cc && user->field_0x1cc != (UnknownGuiControl*)this)
                    user->UnknownFunction487730((UnknownGuiControl*)this, 0, 0);
                result = 1;
            }
        }
    }
    return result;
}

// 0x004727c0
UIControl* UIControl::UnknownFunction4727c0() {
    UIControl* control = this;
    while (control) {
        if (control != this && control->enabled && control->field_0x70 && control->controlType != 5 &&
            control->controlType != 0xc)
            return control;
        if (control->field_0x08) {
            control = (UIControl*)control->field_0x08;
        } else {
            while (control->field_0x0C)
                control = (UIControl*)control->field_0x0C;
        }
        if (control == this)
            return 0;
    }
    return control;
}

// 0x00472810
UIControl* UIControl::UnknownFunction472810() {
    UIControl* control = this;
    while (control) {
        if (control != this && control->enabled && control->field_0x70 && control->controlType != 5 &&
            control->controlType != 0xc)
            return control;
        control = (UIControl*)control->field_0x0C;
        if (!control)
            control = (UIControl*)ownerDialog->controlContainer->field_0x10;
        if (control == this)
            return 0;
    }
    return control;
}

// ---------------------------------------------------------------------------
// UIListBox: rows

// 0x004769e0
int UIListBox::UnknownVirtualSlot65(int row) {
    if ((rowCount && row < rowCount && row >= 0) || (!rowCount && !row)) {
        selectedRow = row;
        if (row >= firstVisibleRow) {
            int last = visibleRowCount + firstVisibleRow - 1;
            if (row <= (last < 0 ? 0 : last))
                goto shown;
            int first = row - visibleRowCount + 1;
            row = first < 0 ? 0 : first;
        }
        firstVisibleRow = row;
    shown:
        UnknownVirtualSlot50();
        return 1;
    }
    return 1;
}

// 0x00476a60
int UIListBox::SelectRow(int row) {
    int result = UnknownVirtualSlot65(row);
    UpdateScrollBars();
    if (ownerDialog) {
        GameObjectIterator* iterator = (GameObjectIterator*)UnknownVirtualSlot53();
        UIListBox* control;
        while ((control = static_cast<UIListBox*>(UnknownFunction472790(iterator))) != 0)
            control->UnknownVirtualSlot65(row);
        UnknownFunction472730(iterator);
    }
    return result;
}

// 0x00476ad0
int UIListBox::SelectRowByText(const char* text) {
    for (int i = 0; i < rowCount; i++) {
        if (rowTable[i].kind == 1 && !_stricmp(text, rowTable[i].text))
            return SelectRow(i);
    }
    return 0;
}

// 0x00476b30
int UIListBox::SelectRowByData(int data) {
    for (int i = 0; i < rowCount; i++) {
        if (GetRowData(i) == data)
            return SelectRow(i);
    }
    return 0;
}

// 0x00476ba0
void UIListBox::SetItemBoxColor(unsigned int color, int row) {
    if (row == -1) {
        if (itemBrush)
            DeleteObject((HGDIOBJ)itemBrush);
        itemBrush = color != 0xff000000 ? CreateSolidBrush(color) : 0;
    } else if (row < rowCount) {
        void* brush = rowTable[row].brush;
        if (brush) {
            for (int i = 0; i < rowCount; i++) {
                if (rowTable[i].brush == brush)
                    goto shared;
            }
            DeleteObject((HGDIOBJ)brush);
        }
    shared:
        rowTable[row].brush = color != 0xff000000 ? CreateSolidBrush(color) : 0;
    }
    UnknownVirtualSlot50();
}

// 0x00476c70
void UIListBox::SetRowTextColor(unsigned int color, int row) {
    if (row == -1) {
        SetFontColor(color);
        UnknownVirtualSlot50();
        return;
    }
    if (row < rowCount)
        rowTable[row].textColor = color;
    UnknownVirtualSlot50();
}

// 0x00476cd0
void UIListBox::SetSelectBoxColor(unsigned int color) {
    if (selectBrush)
        DeleteObject((HGDIOBJ)selectBrush);
    selectBrush = color != 0xff000000 ? CreateSolidBrush(color) : 0;
    UnknownVirtualSlot50();
}

// 0x00476ee0
int UIListBox::CountVisibleRows() {
    int height = 0;
    int rows = 0;
    int i = firstVisibleRow;
    int count = rowCount;
    if (i < count) {
        int space = field_0x2c[3] - field_0x2c[1];
        for (; i < count; i++) {
            height += rowTable[i].height;
            if (height > space) {
                if (!rows && field_0x240)
                    rows = 1;
                return rows;
            }
            rows++;
        }
    }
    return rows;
}

// 0x00476f50
int UIListBox::UnknownFunction476f50(int rows) {
    if (rows > rowCapacity) {
        UnknownGameUiListRow* resized =
            (UnknownGameUiListRow*)UnknownFunction47b570(rowTable, rows * sizeof(UnknownGameUiListRow));
        if (resized) {
            memset(&resized[rowCapacity], 0, (rows - rowCapacity) * sizeof(UnknownGameUiListRow));
            rowCapacity = rows;
            rowTable = resized;
            return 1;
        }
        return 0;
    }
    return 1;
}

// 0x00476ff0
void UIListBox::SetRowText(int row, const char* text) {
    if (row < rowCount && row >= 0) {
        if (rowTable[row].text)
            DebugFree(rowTable[row].text, __FILE__, 0x1e0b);
        if (rowTable[row].textLines)
            DebugFree(rowTable[row].textLines, __FILE__, 0x1e0c);
        rowTable[row].width = strlen(text);
        rowTable[row].text = (char*)DebugMalloc(rowTable[row].width + 1, __FILE__, 0x1e0e);
        strcpy(rowTable[row].text, text);
        rowTable[row].textLines = ownerDialog->SplitTextLines(
            rowTable[row].text, textColor, (void*)fontHandle, field_0x1c4, 0);
    }
    UnknownVirtualSlot50();
}

// 0x00476d80
int UIListBox::AddRow(const char* text, int data, int a) {
    if (text && (rowCount < rowCapacity || UnknownFunction476f50(rowCount + 1))) {
        UnknownGameUiListRow* row = &rowTable[rowCount];
        row->width = strlen(text);
        int height = fontHeight;
        if (!height)
            height = ownerDialog->dialogFontHeight;
        row->image = 0;
        row->imageFile = 0;
        row->textColor = 0;
        row->brush = 0;
        row->height = height;
        row->kind = 1;
        row->flags = a;
        row->text = (char*)DebugMalloc(row->width + 1, __FILE__, 0x1dc7);
        strcpy(row->text, text);
        row->textLines = ownerDialog->SplitTextLines(row->text, textColor, (void*)fontHandle,
                                                     field_0x1c4, 0);
        SetRowData(rowCount, data);
        row->field_0x30 = 0;
        rowCount++;
        visibleRowCount = CountVisibleRows();
        UpdateScrollBars();
        if (autoSort)
            Sort(1);
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// UIMultiState and UIRadioButton: states

// 0x00478860
int UIMultiState::SetStateCount(int count) {
    if (stateCount > count) {
        for (int i = count; i < stateCount; i++) {
            UIAnim* image = stateTable[i].image;
            if (image && !image->animPalette)
                DebugFree(image, __FILE__, 0x21b6);
            image = stateTable[i].focusImage;
            if (image && !image->animPalette)
                DebugFree(image, __FILE__, 0x21b7);
            if (stateTable[i].text)
                DebugFree(stateTable[i].text, __FILE__, 0x21b8);
            if (stateTable[i].textLines)
                DebugFree(stateTable[i].textLines, __FILE__, 0x21b9);
        }
        stateTable = (UnknownGameUiState*)UnknownFunction47b570(stateTable,
                                                                         count * sizeof(UnknownGameUiState));
    } else if (stateCount < count) {
        stateTable = (UnknownGameUiState*)UnknownFunction47b570(stateTable,
                                                                         count * sizeof(UnknownGameUiState));
        for (int i = stateCount; i < count; i++) {
            stateTable[i].image = 0;
            stateTable[i].focusImage = 0;
            stateTable[i].text = 0;
            stateTable[i].textLines = 0;
            stateTable[i].selectable = 1;
        }
    }
    selectedState = 0;
    stateCount = count;
    UnknownVirtualSlot50();
    return 1;
}

// 0x00478a50
void UIMultiState::SetStateTextFromResource(int index, void* module, int id) {
    char text[0x400];
    if (LoadStringA((HINSTANCE)module, id, text, 0x400)) {
        SetStateText(index, text);
    } else {
        sprintf(text, "Resource string '%d' load fail\n", id);
        SetStateText(index, "Resource String Unavailable");
    }
    UnknownVirtualSlot50();
}

// 0x00478ad0
void UIMultiState::SetStateText(int index, const char* text) {
    int current = selectedState;
    if (index < stateCount) {
        selectedState = index;
        if (stateTable[index].text)
            DebugFree(stateTable[index].text, __FILE__, 0x2210);
        if (stateTable[index].textLines)
            DebugFree(stateTable[index].textLines, __FILE__, 0x2211);
        if (text) {
            stateTable[index].text = (char*)DebugMalloc(strlen(text) + 1, __FILE__, 0x2213);
            strcpy(stateTable[index].text, text);
            stateTable[index].textLines = ownerDialog->SplitTextLines(
                stateTable[index].text, textColor, (void*)fontHandle, field_0x1c4,
                stateTable[index].textExtent);
            if (!stateTable[index].textLines) {
                stateTable[index].textExtent[0] = strlen(stateTable[index].text);
                stateTable[index].field_0x0c = stateTable[index].textExtent[0];
                HDC dc;
                if (!g_TrackGame->PCTarget()->renderSurface->GetDC((void**)&dc)) {
                    HGDIOBJ font =
                        (HGDIOBJ)(fontHandle ? fontHandle : (ownerDialog ? (int)ownerDialog->dialogFont : 0));
                    HGDIOBJ old = SelectObject(dc, font);
                    SIZE size;
                    GetTextExtentPoint32A(dc, stateTable[index].text,
                                          stateTable[index].field_0x0c, &size);
                    SelectObject(dc, old);
                    g_TrackGame->PCTarget()->renderSurface->ReleaseDC(dc);
                    stateTable[index].textExtent[1] = size.cx;
                } else {
                    stateTable[index].textExtent[1] = 0;
                }
            }
        } else {
            stateTable[index].text = 0;
            stateTable[index].textLines = 0;
        }
        selectedState = current;
    }
    UnknownVirtualSlot50();
}

// 0x00478d70
int UIMultiState::UnknownVirtualSlot55(int a, int b) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.handled = a;
        SelectNextState();
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->UnknownVirtualSlot29(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::UnknownVirtualSlot55(a, b);
}

// 0x00479310
int UIRadioButton::SelectInGroup(int index) {
    int count = 0;
    GameObjectIterator iterator(ownerDialog->controlContainer, 1, "UIRadioButton");
    if (!index)
        SetCurrentState(1);
    UIRadioButton* control;
    while ((control = (UIRadioButton*)iterator.Next()) != 0) {
        if (control->attachId == attachId) {
            count++;
            if (index) {
                if (count == index)
                    control->SetCurrentState(1);
                else
                    control->SetCurrentState(0);
            } else if (control != this) {
                control->UnknownVirtualSlot29(0);
                control->SetCurrentState(0);
            }
        }
    }
    return 1;
}

// 0x004793f0
int UIRadioButton::GetGroupSelection() {
    int index = 0;
    GameObjectIterator iterator(ownerDialog->controlContainer, 1, "UIRadioButton");
    UIRadioButton* control;
    while ((control = (UIRadioButton*)iterator.Next()) != 0) {
        if (control->attachId == attachId) {
            index++;
            if (control->UnknownFunction4755c0() == 1)
                return index;
        }
    }
    return 0;
}

// 0x004794c0
int UIRadioButton::UnknownVirtualSlot55(int a, int b) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.handled = a;
        if (!UnknownFunction4755c0()) {
            SetCurrentState(1);
            GameObjectIterator* iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            UIRadioButton* control;
            while ((control = static_cast<UIRadioButton*>(UnknownFunction472790(iterator))) != 0) {
                if (control != this && control->UnknownFunction4755c0() == 1) {
                    control->UnknownVirtualSlot29(0);
                    control->SetCurrentState(0);
                }
            }
            UnknownFunction472730(iterator);
        }
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.control = this;
        event.gui = ownerDialog->guiManager;
        ownerDialog->UnknownVirtualSlot29(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::UnknownVirtualSlot55(a, b);
}

// 0x00477490
int UIListBox::RemoveRow(int row) {
    if (row < rowCount && row >= 0) {
        DebugFree(rowTable[row].text, __FILE__, 0x1e80);
        DebugFree(rowTable[row].textLines, __FILE__, 0x1e81);
        DebugFree(rowTable[row].imageFile, __FILE__, 0x1e82);
        void* brush = rowTable[row].brush;
        if (brush) {
            for (int i = 0; i < rowCount; i++) {
                if (rowTable[i].brush == brush)
                    goto shared;
            }
            DeleteObject((HGDIOBJ)brush);
        }
    shared:
        if (row < rowCount - 1) {
            for (int i = row; i < rowCount; i++)
                rowTable[i] = rowTable[i + 1];
        }
        memset(&rowTable[rowCount - 1], 0, sizeof(UnknownGameUiListRow));
        int count = rowCount - 1;
        rowCount = count <= 0 ? 0 : count;
        if (selectedRow == row && selectedRow == rowCount) {
            UnknownVirtualSlot65(selectedRow - 1 <= 0 ? 0 : selectedRow - 1);
        }
        UpdateScrollBars();
        return 1;
    }
    return 0;
}

// 0x004775f0
void UIListBox::RemoveAllRows() {
    for (int i = 0; i < rowCount; i++) {
        DebugFree(rowTable[i].text, __FILE__, 0x1ecf);
        DebugFree(rowTable[i].imageFile, __FILE__, 0x1ed0);
        DebugFree(rowTable[i].textLines, __FILE__, 0x1ed1);
        if (rowTable[i].brush) {
            DeleteObject((HGDIOBJ)rowTable[i].brush);
            for (int j = 0; j < rowCount; j++) {
                if (rowTable[j].brush == rowTable[i].brush)
                    rowTable[j].brush = 0;
            }
        }
        UIAnim* image = rowTable[i].image;
        if (image && !image->animPalette)
            image->Release();
    }
    rowCount = 0;
    firstVisibleRow = 0;
    SelectRow(0);
    lastPageRowCount = 0;
    memset(rowTable, 0, rowCapacity * sizeof(UnknownGameUiListRow));
    UpdateScrollBars();
}

// 0x00477900
void UIListBox::Sort(int a) {
    if (a || !ownerDialog->sortingList || !g_UnknownGlobal65b608) {
        ownerDialog->sortingList = this;
        g_UnknownGlobal65b608 = ownerDialog;
    }
    if (!rowCount)
        return;
    field_0x220 = 1;
    for (int i = 0; i < rowCount; i++)
        rowTable[i].sortIndex = i;
    int selected = selectedRow;
    qsort(rowTable, rowCount, sizeof(UnknownGameUiListRow), UnknownFunction477800);
    UnknownVirtualSlot50();
    if (attachId > 0) {
        GameObjectIterator iterator(ownerDialog->controlContainer, 1, "UIControl");
        UIControl* control;
        while ((control = (UIControl*)iterator.Next()) != 0) {
            UIListBox* other = static_cast<UIListBox*>(control);
            if (control->attachId == attachId && control->controlType == 3 && control != this &&
                other->rowCount == rowCount) {
                UnknownGameUiListRow* rows = (UnknownGameUiListRow*)DebugMalloc(
                    other->rowCapacity * sizeof(UnknownGameUiListRow), __FILE__, 0x1f87);
                for (int j = 0; j < rowCount; j++)
                    rows[j] = other->rowTable[rowTable[j].sortIndex];
                DebugFree(other->rowTable, __FILE__, 0x1f8e);
                other->rowTable = rows;
                control->UnknownVirtualSlot50();
            }
        }
    }
    for (int k = 0; k < rowCount; k++) {
        if (rowTable[k].sortIndex == selected) {
            SelectRow(k);
            break;
        }
    }
    field_0x220 = 0;
}

// ---------------------------------------------------------------------------
// UIScrollBar: position

// 0x004751e0
int UIScrollBar::UnknownFunction4751e0(UIListBox* list) {
    return list->rowCount - list->lastPageRowCount;
}

// 0x00475200
int UIScrollBar::UnknownFunction475200(UIListBox* list) {
    UnknownVirtualSlot50();
    if (field_0x21c) {
        int count = list->rowCount;
        field_0x1fc = count - list->UnknownFunction4755c0();
    } else {
        field_0x1fc = list->UnknownFunction4755c0();
    }
    if (UnknownFunction4751e0(list) == field_0x1fc)
        return 1;
    unsigned int travel;
    if (controlType == 8)
        travel = UnknownVirtualSlot61() - thumbWidth;
    else
        travel = UnknownVirtualSlot62() - thumbHeight;
    unsigned int range = list->rowCount - list->lastPageRowCount;
    if (range) {
        float position = field_0x1fc * (float)travel / range;
        field_0x1ec_float = position < travel ? position : travel;
        return 1;
    }
    return 0;
}

// 0x00475300
int UIScrollBar::UnknownFunction475300(int value) {
    int width = UnknownVirtualSlot61();
    int height = UnknownVirtualSlot62();
    unsigned int position;
    int travel;
    if (controlType == 8) {
        position = (int)(field_0x21c ? width - field_0x1ec_float : field_0x1ec_float);
    } else {
        position = (int)(field_0x21c ? height - field_0x1ec_float : field_0x1ec_float);
    }
    travel = controlType == 8 ? width - thumbWidth : height - thumbHeight;
    return (int)(travel ? (double)(position * value) / travel : 0.0);
}

// 0x00475500
int UIScrollBar::UnknownFunction475500() {
    int width = UnknownVirtualSlot61();
    int height = UnknownVirtualSlot62();
    int position;
    if (!field_0x21c) {
        position = (int)field_0x1ec_float;
        if (position < field_0x1ec_float)
            position++;
    } else {
        if (controlType == 8)
            position = (int)(width - field_0x1ec_float);
        else
            position = (int)(height - field_0x1ec_float);
    }
    if (scrollRange && width) {
        if (controlType == 8)
            return (unsigned int)(scrollRange * position) / (unsigned int)(width - thumbWidth);
        return (unsigned int)(scrollRange * position) / (unsigned int)(height - thumbHeight);
    }
    return position;
}

// ---------------------------------------------------------------------------
// UIEditBox: text

// 0x00473c70
void UIEditBox::SetCapacity(int size) {
    char saved[1000];
    int kept = 0;
    if (controlText) {
        strcpy(saved, controlText);
        DebugFree(controlText, __FILE__, 0x169a);
        kept = 1;
    }
    int capacity = size + 1;
    if (capacity >= 1000)
        capacity = 1000;
    else if (capacity < 1)
        capacity = 1;
    textCapacity = capacity;
    field_0x200 = 0;
    textLength = 0;
    textWidth = 0;
    field_0xd0 = 0;
    controlText = (char*)DebugMalloc(capacity, __FILE__, 0x16a3);
    if (kept) {
        int max = textCapacity - 1;
        int n = strlen(saved);
        int length;
        if (n < max)
            length = n;
        else
            length = max;
        strncpy(controlText, saved, length);
        controlText[length] = 0;
    }
    UnknownVirtualSlot50();
}

// 0x00473da0
void UIEditBox::SetEditText(char* text) {
    if (!text)
        text = "";
    if (!controlText)
        controlText = (char*)DebugMalloc(textCapacity, __FILE__, 0x16c0);
    int max = textCapacity - 1;
    int n = strlen(text);
    int length;
    if (n < max)
        length = n;
    else
        length = max;
    strncpy(controlText, text, length);
    controlText[length] = 0;
    textLength = length;
    field_0xd0 = length;
    HDC dc;
    if (!g_TrackGame->PCTarget()->renderSurface->GetDC((void**)&dc)) {
        HGDIOBJ font = (HGDIOBJ)(fontHandle ? fontHandle : (ownerDialog ? (int)ownerDialog->dialogFont : 0));
        HGDIOBJ old = SelectObject(dc, font);
        SIZE size;
        GetTextExtentPoint32A(dc, controlText, field_0xd0, &size);
        SelectObject(dc, old);
        g_TrackGame->PCTarget()->renderSurface->ReleaseDC(dc);
        textExtent[1] = size.cx;
        textWidth = size.cx;
    } else {
        textExtent[1] = 0;
        textWidth = 0;
    }
    UnknownVirtualSlot50();
}

// 0x00473f30
void UIEditBox::SetAcceptedCharacters(const char* characters) {
    if (characters) {
        int length = strlen(characters);
        acceptedCharacters = DebugMalloc(length + 1, __FILE__, 0x16f7);
        if (acceptedCharacters)
            strcpy((char*)acceptedCharacters, characters);
    } else {
        if (acceptedCharacters)
            DebugFree(acceptedCharacters, __FILE__, 0x16fb);
        acceptedCharacters = 0;
    }
}

// 0x00473fc0
char UIEditBox::FilterCharacter(char c) {
    if (!acceptedCharacters)
        return c;
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        char other = c <= 'Z' ? c + 0x20 : c - 0x20;
        if (strchr((char*)acceptedCharacters, c))
            return c;
        if (strchr((char*)acceptedCharacters, other))
            return other;
        return 0;
    }
    if (strchr((char*)acceptedCharacters, c))
        return c;
    return 0;
}

// ---------------------------------------------------------------------------
// UIDialog: resources and text

// 0x0046a8e0
int UIDialog::LoadDialogResource(const char* name) {
    int result = 0;
    UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 1);
    if (entry)
        result = UnknownFunction46a920(entry->field_0x14, entry->field_0x18);
    return result;
}

// 0x0046eeb0
void UIDialog::EndControlDraw(void* dc) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (dialog->field_0x7f04)
        SelectObject((HDC)dc, (HGDIOBJ)dialog->field_0x7f04);
    if (!dialog->dialogBackground && dc)
        ((PCRenderTarget*)dialog->UnknownInlineField18())->renderSurface->ReleaseDC(dc);
    if (dialog->field_0x7f00)
        DeleteObject((HGDIOBJ)dialog->field_0x7f00);
}

// 0x0046f3c0
void UIDialog::DrawShadowText(void* dc, int x, int y, const char* text, int length,
                              unsigned int color, int shadow, unsigned int shadowColor) {
    if (shadow) {
        SetTextColor((HDC)dc, shadowColor);
        TextOutA((HDC)dc, x - 1, y - 1, text, length);
    }
    SetTextColor((HDC)dc, color);
    TextOutA((HDC)dc, x, y, text, length);
}

// 0x0046fc80
int UIDialog::IsInDoubleByteCharacter(const char* text, const char* position, int length) {
    for (int i = 0; i < length; i++) {
        if (IsDBCSLeadByte(text[i])) {
            if (text + i == position || text + i + 1 == position)
                return 1;
            i++;
        } else if (text + i > position) {
            return 0;
        }
    }
    return 0;
}

// 0x0046f120
void UIDialog::UnknownFunction46f120() {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (dialog->guiUser->pointerDevice && dialog->controlContainer->field_0x10) {
        GUIUser* user = dialog->guiUser;
        UnknownGuiControl* focus = user->field_0x1d8;
        if (focus && ((UIControl*)focus)->ownerDialog != (UnknownGameUiDialog*)this)
            focus = 0;
        POINT position = user->pointerDevice->pointerPosition;
        if (dialog->controlContainer->field_0x10) {
            user->field_0x1dc = 0;
            int result = ((UIControl*)dialog->controlContainer->field_0x10)->UnknownFunction472480((int*)&position);
            if (focus && !dialog->guiUser->field_0x1cc && !result)
                dialog->guiUser->UnknownFunction487730(0, 0, 1);
        }
    }
}

// A colored run of dialog text (0x18 bytes).
struct UnknownGameUiTextRun {
    const char* field_0x00;                   // text
    int field_0x04;                           // 1: sets the color, 2: sets the position
    int field_0x08;                           // length
    unsigned int field_0x0c;                  // color
    int field_0x10;                           // width
    int field_0x14;                           // position
};

int g_UnknownGlobal65b604;                    // the run being drawn
int g_UnknownGlobal65b564;                    // its position
unsigned int g_UnknownGlobal65b574;           // its color
SIZE g_UnknownGlobal65b5e0;                   // the text's size
int g_UnknownGlobal65b5a8;                    // the text's position
int g_UnknownGlobal65b584;

// 0x0046f420
void UIDialog::DrawTextRuns(void* dc, int x, int y, const char* text, int length,
                            UnknownGameUiTextRun* runs, unsigned int color, int shadow,
                            unsigned int shadowColor) {
    g_UnknownGlobal65b604 = 0;
    g_UnknownGlobal65b564 = x;
    SetTextColor((HDC)dc, color);
    g_UnknownGlobal65b574 = color;
    while (runs[g_UnknownGlobal65b604].field_0x00) {
        if (runs[g_UnknownGlobal65b604].field_0x04 == 2)
            g_UnknownGlobal65b564 = runs[g_UnknownGlobal65b604].field_0x14 + x;
        if (shadow) {
            SetTextColor((HDC)dc, shadowColor);
            TextOutA((HDC)dc, g_UnknownGlobal65b564 + 1, y + 1, runs[g_UnknownGlobal65b604].field_0x00,
                     runs[g_UnknownGlobal65b604].field_0x08);
            SetTextColor((HDC)dc, g_UnknownGlobal65b574);
        }
        if (runs[g_UnknownGlobal65b604].field_0x04 == 1) {
            g_UnknownGlobal65b574 = runs[g_UnknownGlobal65b604].field_0x0c;
            SetTextColor((HDC)dc, g_UnknownGlobal65b574);
        }
        TextOutA((HDC)dc, g_UnknownGlobal65b564, y, runs[g_UnknownGlobal65b604].field_0x00,
                 runs[g_UnknownGlobal65b604].field_0x08);
        g_UnknownGlobal65b564 += runs[g_UnknownGlobal65b604].field_0x10;
        g_UnknownGlobal65b604++;
    }
}

// 0x0046f550
void UIDialog::DrawAlignedText(void* dc, CameraRect* rect, const char* text, int length,
                               unsigned int color, int height, UnknownGameUiTextRun* runs, int shadow,
                               unsigned int shadowColor, int flags, int transparent,
                               UIControl* control) {
    if (transparent)
        SetBkMode((HDC)dc, TRANSPARENT);
    else
        SetBkMode((HDC)dc, OPAQUE);
    if (flags != 9) {
        if (control) {
            g_UnknownGlobal65b5e0.cx = ((int*)control->UnknownVirtualSlot37())[1];
            g_UnknownGlobal65b5e0.cy = control->fontHeight;
        } else {
            GetTextExtentPoint32A((HDC)dc, text, length, &g_UnknownGlobal65b5e0);
        }
    }
    if (flags & 2)
        g_UnknownGlobal65b5a8 = (rect->right - rect->left) / 2 - g_UnknownGlobal65b5e0.cx / 2 + rect->left;
    else if (flags & 4)
        g_UnknownGlobal65b5a8 = rect->right - g_UnknownGlobal65b5e0.cx;
    else
        g_UnknownGlobal65b5a8 = rect->left;
    if (!height)
        height = ((UnknownGameUiDialog*)this)->dialogFontHeight;
    if (flags & 0x10)
        g_UnknownGlobal65b584 = (rect->bottom - rect->top) / 2 - height / 2 + rect->top;
    else if (flags & 0x20)
        g_UnknownGlobal65b584 = rect->bottom - height;
    else
        g_UnknownGlobal65b584 = rect->top;
    if (runs)
        DrawTextRuns(dc, g_UnknownGlobal65b5a8, g_UnknownGlobal65b584, text, length, runs, color, shadow,
                     shadowColor);
    else
        DrawShadowText(dc, g_UnknownGlobal65b5a8, g_UnknownGlobal65b584, text, length, color, shadow,
                       shadowColor);
}

SIZE g_UnknownGlobal65b5a0;                   // the control text's size
int g_UnknownGlobal65b5e8;                    // its position
int g_UnknownGlobal65b5c0;

// 0x0046f6c0
void UIDialog::DrawControlText(void* dc, UIControl* control, int transparent) {
    CameraRect rect = *(CameraRect*)control->field_0x2c;
    rect.left += control->textLeftMargin;
    rect.top += control->textTopMargin;
    rect.right -= control->textLeftMargin;
    rect.bottom -= control->textTopMargin;
    if (transparent)
        SetBkMode((HDC)dc, TRANSPARENT);
    else
        SetBkMode((HDC)dc, OPAQUE);
    if (control->textAlign != 9) {
        g_UnknownGlobal65b5a0.cx = ((int*)control->UnknownVirtualSlot37())[1];
        g_UnknownGlobal65b5a0.cy = control->fontHeight;
    }
    if (control->textAlign & 2)
        g_UnknownGlobal65b5e8 = (rect.right - rect.left) / 2 - g_UnknownGlobal65b5a0.cx / 2 + rect.left;
    else if (control->textAlign & 4)
        g_UnknownGlobal65b5e8 = rect.right - g_UnknownGlobal65b5a0.cx;
    else
        g_UnknownGlobal65b5e8 = rect.left;
    if (control->textAlign & 0x10)
        g_UnknownGlobal65b5c0 =
            (rect.bottom - rect.top) / 2 - ((UnknownGameUiDialog*)this)->dialogFontHeight / 2 + rect.top;
    else if (control->textAlign & 0x20)
        g_UnknownGlobal65b5c0 = rect.bottom - ((UnknownGameUiDialog*)this)->dialogFontHeight;
    else
        g_UnknownGlobal65b5c0 = rect.top;
    if (control->UnknownVirtualSlot36())
        DrawTextRuns(dc, g_UnknownGlobal65b5e8, g_UnknownGlobal65b5c0,
                     (const char*)control->UnknownVirtualSlot34(), control->UnknownVirtualSlot35(),
                     (UnknownGameUiTextRun*)control->UnknownVirtualSlot36(), control->textColor,
                     control->textColor, control->dropColor);
    else
        DrawShadowText(dc, g_UnknownGlobal65b5e8, g_UnknownGlobal65b5c0,
                       (const char*)control->UnknownVirtualSlot34(), control->UnknownVirtualSlot35(),
                       control->textColor, control->textDrop, control->dropColor);
}

CameraRect* g_UnknownGlobal65b588;            // the clip rectangle
CameraRect g_UnknownGlobal65b5f0;
int g_UnknownGlobal65b5ec;

// 0x0046ed70
int UIDialog::BeginControlDraw(void** dc, CameraRect* rect, int* a, UIControl* control) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    g_UnknownGlobal65b588 = rect;
    *a = 0;
    dialog->field_0x7f04 = 0;
    if (dialog->dialogBackground) {
        g_UnknownGlobal65b588 = &g_UnknownGlobal65b5f0;
        g_UnknownGlobal65b5f0 = *rect;
        if (control) {
            g_UnknownGlobal65b5ec = dialog->dialogBackground->UnknownFunction4049d0(
                dc, rect, control->backgroundRegion, control->needsRedraw, &control->redrawFrames, a,
                &g_UnknownGlobal65b5f0);
        } else {
            int frames = 1;
            g_UnknownGlobal65b5ec =
                dialog->dialogBackground->UnknownFunction4049d0(dc, rect, -1, 0, &frames, a, &g_UnknownGlobal65b5f0);
        }
    } else {
        ((PCRenderTarget*)dialog->UnknownInlineField18())->renderSurface->GetDC(dc);
        g_UnknownGlobal65b5ec = 1;
    }
    if (*dc && control) {
        dialog->field_0x7f04 =
            SelectObject((HDC)*dc, (HGDIOBJ)(control->fontHandle ? control->fontHandle : (int)dialog->dialogFont));
        dialog->field_0x7f00 = CreateRectRgn(g_UnknownGlobal65b588->left, g_UnknownGlobal65b588->top,
                                             g_UnknownGlobal65b588->right, g_UnknownGlobal65b588->bottom);
        SelectClipRgn((HDC)*dc, (HRGN)dialog->field_0x7f00);
    } else {
        dialog->field_0x7f00 = 0;
    }
    return g_UnknownGlobal65b5ec;
}

// 0x0046fce0
void UIDialog::AddTimer(int a, int time, int b) {
    if (time)
        ((UnknownGameUiDialog*)this)->timerList.Add(
            new(__FILE__, 0xa76) UITimer(a, time, (UIControl*)b));
}

// ---------------------------------------------------------------------------
// UIControl: transitions

// 0x00471df0
int UIControl::UnknownVirtualSlot46() {
    if (field_0x164 == 0) {
        fxAnimIn->playBackwards = 0;
        fxAnimIn->Rewind();
        field_0x164 = 1;
        if (fxSoundIn) {
            fxSoundIn->SetVolume(ownerGui->field_0x34c, 0);
            fxSoundIn->PlayWithOptions(1, 0, 0);
        }
    }
    if (field_0x164 == 1 && fxAnimIn->currentFrame <= fxAnimIn->frameCount - 1) {
        drawnTexture = fxAnimIn->GetCurrentTexture();
        UnknownVirtualSlot50();
        if (fxAnimIn->currentFrame == fxAnimIn->frameCount - 1)
            field_0x164 = 2;
        return 0;
    }
    field_0x164 = 0;
    return 1;
}

// 0x00471eb0
int UIControl::UnknownVirtualSlot47() {
    if (field_0x164 == 0) {
        if (fxAnimOut == fxAnimIn) {
            fxAnimOut->playBackwards = 1;
            field_0x164 = 2;
        } else {
            field_0x164 = 1;
        }
        fxAnimOut->Rewind();
        if (fxSoundOut) {
            fxSoundOut->SetVolume(ownerGui->field_0x34c, 0);
            fxSoundOut->PlayWithOptions(1, 0, 0);
        }
    }
    if ((field_0x164 == 1 && fxAnimOut->currentFrame < fxAnimOut->frameCount - 1) ||
        (field_0x164 == 2 && fxAnimOut->currentFrame >= 0)) {
        drawnTexture = fxAnimOut->GetCurrentTexture();
        UnknownVirtualSlot50();
        if (fxAnimOut->currentFrame == 0)
            field_0x164 = 3;
        return 0;
    }
    field_0x164 = 0;
    field_0x70 = 0;
    return 1;
}

int g_UnknownGlobal65b610;                       // frames of the slide

// 0x00471850
int UIControl::UnknownVirtualSlot42() {
    if (field_0x164 == 0 && field_0xb0) {
        g_UnknownGlobal65b610 = field_0xb0 < 0 ? 25 : field_0xb0;
        int left = field_0x3c[0];
        field_0x80 = (float)(left - field_0x3c[2] - 1);
        field_0x90 = ((float)left - field_0x80) / g_UnknownGlobal65b610;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        field_0x80 += field_0x90;
    }
    if (field_0x164 == g_UnknownGlobal65b610)
        return 1;
    UnknownVirtualSlot50();
    *(CameraRect*)field_0x2c = *(CameraRect*)field_0x3c;
    field_0x2c[0] = (int)field_0x80;
    field_0x2c[2] = field_0x3c[2] - field_0x3c[0] + field_0x2c[0];
    if (field_0x2c[2] > 0) {
        switch (controlType) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
            drawnTexture = stateImages[currentState]->GetCurrentTexture();
            break;
        default:
            drawnTexture = 0;
            return 1;
        }
    }
    field_0x164++;
    return 0;
}

int g_UnknownGlobal65b614;                       // frames of the slide

// 0x004719c0
int UIControl::UnknownVirtualSlot44() {
    if (field_0x164 == 0 && field_0xb0) {
        g_UnknownGlobal65b614 = field_0xb0 < 0 ? 25 : field_0xb0;
        int top = field_0x3c[1];
        field_0x84 = (float)(top - field_0x3c[3]);
        field_0x94 = ((float)top - field_0x84) / g_UnknownGlobal65b614;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        field_0x84 += field_0x94;
    }
    if (field_0x164 == g_UnknownGlobal65b614)
        return 1;
    UnknownVirtualSlot50();
    *(CameraRect*)field_0x2c = *(CameraRect*)field_0x3c;
    field_0x2c[1] = (int)field_0x84;
    field_0x2c[3] = field_0x3c[3] - field_0x3c[1] + field_0x2c[1];
    if (field_0x2c[3] > 0) {
        switch (controlType) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
            drawnTexture = stateImages[currentState]->GetCurrentTexture();
            break;
        default:
            drawnTexture = 0;
            return 1;
        }
    }
    field_0x164++;
    return 0;
}

int g_UnknownGlobal65b618;                       // frames of the slide

// 0x00471b30
int UIControl::UnknownVirtualSlot43() {
    if (field_0x164 == 0 && field_0xb0) {
        g_UnknownGlobal65b618 = field_0xb0 < 0 ? 25 : field_0xb0;
        field_0x80 = (float)ownerDialog->screenWidth;
        field_0x90 = ((float)field_0x3c[0] - field_0x80) / g_UnknownGlobal65b618;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        field_0x80 += field_0x90;
    }
    if (field_0x164 == g_UnknownGlobal65b618)
        return 1;
    UnknownVirtualSlot50();
    *(CameraRect*)field_0x2c = *(CameraRect*)field_0x3c;
    field_0x2c[0] = (int)field_0x80;
    field_0x2c[2] = field_0x3c[2] - field_0x3c[0] + field_0x2c[0];
    if (field_0x2c[0] < ownerDialog->screenWidth) {
        switch (controlType) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
            drawnTexture = stateImages[currentState]->GetCurrentTexture();
            break;
        default:
            drawnTexture = 0;
            return 1;
        }
    }
    field_0x164++;
    return 0;
}

int g_UnknownGlobal65b61c;                       // frames of the slide

// 0x00471c90
int UIControl::UnknownVirtualSlot45() {
    if (field_0x164 == 0 && field_0xb0) {
        g_UnknownGlobal65b61c = field_0xb0 < 0 ? 25 : field_0xb0;
        field_0x84 = (float)ownerDialog->screenHeight;
        field_0x94 = ((float)field_0x3c[1] - field_0x84) / g_UnknownGlobal65b61c;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        field_0x84 += field_0x94;
    }
    if (field_0x164 == g_UnknownGlobal65b61c)
        return 1;
    UnknownVirtualSlot50();
    *(CameraRect*)field_0x2c = *(CameraRect*)field_0x3c;
    field_0x2c[1] = (int)field_0x84;
    field_0x2c[3] = field_0x3c[3] - field_0x3c[1] + field_0x2c[1];
    if (field_0x2c[1] < ownerDialog->screenHeight) {
        switch (controlType) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
            drawnTexture = stateImages[currentState]->GetCurrentTexture();
            break;
        default:
            drawnTexture = 0;
            return 1;
        }
    }
    field_0x164++;
    return 0;
}

// ---------------------------------------------------------------------------
// UIScrollBar

// 0x00475880
int UIScrollBar::UnknownVirtualSlot56(int a, int* position) {
    UnknownDialogEvent event;
    event.handled = 0;
    if (field_0x208) {
        event.kind = 0x10;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->UnknownVirtualSlot29(&event);
    }
    field_0x208 = 0;
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a && currentState == 2) {
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->UnknownVirtualSlot29(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::UnknownVirtualSlot56(a, position);
}

// ---------------------------------------------------------------------------
// UIListBox: rows

unsigned int g_UnknownGlobal65b59c;           // time of the last list box click

// 0x00477d30
void UIListBox::UnknownVirtualSlot66(int* handled) {
    UnknownDialogEvent event;
    event.handled = 0;
    event.kind = kDialogListSelect;
    event.code = eventCode;
    event.controlName = controlName;
    event.dialog = ownerDialog;
    event.control = this;
    event.gui = ownerDialog->guiManager;
    ownerDialog->UnknownVirtualSlot29(&event);
    if (handled)
        *handled = 0;
    if (!event.handled) {
        static unsigned int s_doubleClickTime = GetDoubleClickTime();
        g_UnknownGlobal65b59c = UnknownFunction4bfa80();
        if (selectedRow == lastClickRow && g_UnknownGlobal65b59c - lastClickTime <= s_doubleClickTime) {
            event.kind = kDialogListDoubleClick;
            event.code = eventCode;
            event.controlName = controlName;
            event.dialog = ownerDialog;
            event.control = this;
            event.gui = ownerDialog->guiManager;
            ownerDialog->UnknownVirtualSlot29(&event);
            if (event.handled) {
                if (handled)
                    *handled = 1;
                return;
            }
            lastClickTime = 0;
            return;
        } else {
            lastClickTime = g_UnknownGlobal65b59c;
            lastClickRow = selectedRow;
            return;
        }
    }
    if (handled)
        *handled = 1;
}

// ---------------------------------------------------------------------------
// UIDropDownList: layout

// 0x0047a880
void UIDropDownList::UnknownFunction47a880(int height) {
    listPart->field_0x2c[3] = listPart->field_0x2c[1] + height;
    *(CameraRect*)listPart->field_0x3c = *(CameraRect*)listPart->field_0x2c;
    staticPart0->field_0x2c[3] = staticPart0->field_0x2c[1] + height;
    *(CameraRect*)staticPart0->field_0x3c = *(CameraRect*)staticPart0->field_0x2c;
    scrollBarPart->field_0x2c[3] = scrollBarPart->field_0x2c[1] + height;
    scrollBarPart->field_0x2c[1] += 2;
    scrollBarPart->field_0x2c[3] -= 2;
    *(CameraRect*)scrollBarPart->field_0x3c = *(CameraRect*)scrollBarPart->field_0x2c;
    staticPart1->field_0x2c[3] = staticPart1->field_0x2c[1] + height;
    *(CameraRect*)staticPart1->field_0x3c = *(CameraRect*)staticPart1->field_0x2c;
    rowHeight = height;
}

// 0x0047a970
void UIDropDownList::UnknownFunction47a970(UIAnim* image) {
    staticPart0->SetImage(0, image);
    staticPart0->field_0x2c[2] = image->GetCurrentFrame()->frameWidth + staticPart0->field_0x2c[0];
    staticPart0->field_0x2c[3] = image->GetCurrentFrame()->frameHeight + staticPart0->field_0x2c[1];
    *(CameraRect*)staticPart0->field_0x3c = *(CameraRect*)staticPart0->field_0x2c;
    *(CameraRect*)listPart->field_0x3c = *(CameraRect*)staticPart0->field_0x2c;
    *(CameraRect*)listPart->field_0x2c = *(CameraRect*)listPart->field_0x3c;
    UnknownFunction47a880(image->GetCurrentFrame()->frameHeight);
}

// 0x0047aa40
void UIDropDownList::UnknownFunction47aa40(UIAnim* image) {
    staticPart1->SetImage(0, image);
    staticPart1->field_0x2c[2] = image->GetCurrentFrame()->frameWidth + staticPart1->field_0x2c[0];
    staticPart1->field_0x2c[3] = image->GetCurrentFrame()->frameHeight + staticPart1->field_0x2c[1];
    *(CameraRect*)staticPart1->field_0x3c = *(CameraRect*)staticPart1->field_0x2c;
    *(CameraRect*)scrollBarPart->field_0x2c = *(CameraRect*)staticPart1->field_0x2c;
    scrollBarPart->field_0x2c[1] += 2;
    scrollBarPart->field_0x2c[3] -= 2;
    *(CameraRect*)scrollBarPart->field_0x3c = *(CameraRect*)scrollBarPart->field_0x2c;
}

// 0x0047a2d0
void UIDropDownList::UnknownFunction47a2d0(int open) {
    staticPart0->Show(open, 1);
    staticPart1->Show(open, 1);
    listPart->Show(open, 1);
    scrollBarPart->Show(open, 1);
    if (!isOpen && open) {
        ownerDialog->controlContainer->AppendChild(staticPart0, -1);
        ownerDialog->controlContainer->AppendChild(staticPart1, -1);
        ownerDialog->controlContainer->AppendChild(listPart, -1);
        ownerDialog->controlContainer->AppendChild(scrollBarPart, -1);
        listPart->UpdateScrollBars();
    } else if (isOpen && !open) {
        staticPart0->UnknownFunction4691f0();
        staticPart1->UnknownFunction4691f0();
        listPart->UnknownFunction4691f0();
        scrollBarPart->UnknownFunction4691f0();
        if (ownerDialog->dialogBackground)
            ownerDialog->dialogBackground->UnknownFunction404da0();
    }
    isOpen = open;
}

// ---------------------------------------------------------------------------
// Drawing and input slots

// 0x0046a300
UnknownGameUiDialog* UnknownGameUiDialog::UnknownVirtualSlot27(void* target, CameraRect* area, int a, int flags,
                                                               SoundGroup* sound, void* textures,
                                                               const char* directory, UnknownGameUiDialog* parent,
                                                               BackgroundImage* background, const char* font,
                                                               int fontSize, GUIManager* gui, GUIUser* user,
                                                               const char* name) {
    GameObject::UnknownVirtualSlot8(target);
    if (area) {
        screenArea = *area;
    } else {
        screenArea.top = 0;
        screenArea.left = 0;
        screenArea.right = ((RenderTarget*)field_0x18)->field_0x0c;
        screenArea.bottom = ((RenderTarget*)field_0x18)->field_0x10;
    }
    parentDialog = parent;
    if (!(flags & 6))
        field_0x148 = 0;
    else
        field_0x148 = 1;
    guiManager = gui;
    if (gui && field_0x148)
        field_0x14c = gui->UnknownFunction485ee0((int)this);
    if (parent)
        dialogPalette = parent->dialogPalette;
    field_0xc4 = a;
    dialogBackground = background;
    soundGroup = sound;
    dialogTextures = textures;
    guiUser = user;
    dialogFontHeight = fontSize;
    strcpy(dialogFontFace, font);
    dialogFont = CreateFontA(dialogFontHeight, 0, 0, 0, FW_MEDIUM, 0, 0, 0, DEFAULT_CHARSET, 0, 0, PROOF_QUALITY,
                             VARIABLE_PITCH, dialogFontFace);
    field_0x108 = 0;
    field_0x10c = 0;
    controlContainer = (GameObject*)AppendChild(new(__FILE__, 0x16e) UICtlContainer, -1);
    UnknownDialogEvent event;
    event.handled = 0;
    event.kind = kDialogCreate;
    event.code = 0;
    event.controlName = 0;
    event.dialog = this;
    event.control = 0;
    event.gui = guiManager;
    UnknownVirtualSlot29(&event);
    if (resourceName[0]) {
        char path[0x100];
        char* dot = strrchr(resourceName, '.');
        if (!name) {
            if (dot) {
                char base[0x100];
                int length = dot - resourceName;
                strncpy(base, resourceName, length);
                base[length] = 0;
                sprintf(path, "%s%s.dat", directory ? directory : "", base);
            } else {
                sprintf(path, "%s%s.dat", directory ? directory : "", resourceName);
            }
        } else {
            sprintf(path, "%s%s", directory ? directory : "", name);
        }
        resourceArchive = g_UnknownResourceManager572b44->UnknownFunction4e9030(path, 0);
        sprintf(path, "%s%s", directory ? directory : "", resourceName);
        UnknownTextureStream* stream = new(__FILE__, 0x194) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        if (stream->UnknownFunction460f50(path, "r", 0)) {
            if (!UnknownFunction46a920(stream, 0)) {
                delete stream;
                UnknownFunction46ff60();
                return 0;
            }
            delete stream;
        } else {
            delete stream;
            if (!LoadDialogResource(resourceName)) {
                UnknownFunction46ff60();
                return 0;
            }
        }
    }
    if (parent && !dialogBackground)
        dialogBackground = parent->dialogBackground;
    if (dialogBackground)
        dialogBackground->UnknownFunction404da0();
    if (user && !user->pointerDevice) {
        GameObjectIterator iterator(controlContainer, 1, "UIControl");
        UnknownFunction470000((UIControl*)iterator.Next(), 0, 0);
    }
    UpdateBoundValues(0);
    sendFrameEvent = 1;
    return this;
}

// 0x0046f890: splits `text` at its "~N" (color N) and "|x" (position
// table[x]) codes into runs, each measured with `font`; 0 when there are no
// codes. `size` receives the visible length and the total width.
char* UIDialog::SplitTextLines(char* text, unsigned int color, void* font, void* table, int* size) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (g_TrackGame->imeLibrary)
        return 0;
    if (!strchr(text, '~') && (!strchr(text, '|') || !table))
        return 0;
    void* dc;
    SIZE extent;
    UnknownGameUiTextRun runs[100];
    int length = strlen(text);
    int i = 0;
    int count = 0;
    if (text[0] != '~' && text[0] != '|') {
        runs[0].field_0x00 = text;
        runs[0].field_0x0c = color;
        runs[0].field_0x04 = 1;
        count = 1;
    }
    char* start = text;
    char* mark = 0;
    while (text[i]) {
        char c = text[i];
        if (c != '~' && c != '|') {
            i++;
            continue;
        }
        if (!mark)
            mark = &text[i];
        if (i + 2 >= length)
            break;
        if (text[i + 2] == '~' || text[i + 2] == '|') {
            i += 2;
            continue;
        }
        UnknownGameUiTextRun* run = &runs[count];
        run->field_0x00 = &text[i + 2];
        if (c == '~') {
            int index = text[i + 1] - '0';
            run->field_0x04 = 1;
            if (index > 9)
                index = 9;
            else if (index < 0)
                index = 0;
            run->field_0x0c = dialog->textColors[index];
        } else {
            run->field_0x04 = 2;
            if (table) {
                char key = text[i + 1];
                char digit;
                if (key >= '0' && key <= '9')
                    digit = key - '0';
                else if (key >= 'a' && key <= 'z')
                    digit = key - 'a' + 10;
                else if (key >= 'A' && key <= 'Z')
                    digit = key - 'A' + 10;
                else
                    digit = 0;
                run->field_0x14 = ((int*)table)[digit];
            } else {
                run->field_0x14 = 0;
            }
        }
        i += 2;
        if (count) {
            UnknownGameUiTextRun* last = run - 1;
            if (mark)
                last->field_0x08 = mark - start;
            else
                last->field_0x08 = run->field_0x00 - start - 2;
            if (!((PCRenderTarget*)dialog->UnknownInlineField18())->renderSurface->GetDC(&dc)) {
                HGDIOBJ old = SelectObject((HDC)dc, (HGDIOBJ)(font ? font : dialog->dialogFont));
                GetTextExtentPoint32A((HDC)dc, last->field_0x00, last->field_0x08, &extent);
                SelectObject((HDC)dc, old);
                ((PCRenderTarget*)dialog->UnknownInlineField18())->renderSurface->ReleaseDC(dc);
            }
            last->field_0x10 = extent.cx;
        }
        start = (char*)run->field_0x00;
        mark = 0;
        count++;
    }
    if (count) {
        if (mark)
            runs[count - 1].field_0x08 = mark - runs[count - 1].field_0x00;
        else
            runs[count - 1].field_0x08 = length - (runs[count - 1].field_0x00 - text);
        if (!((PCRenderTarget*)dialog->UnknownInlineField18())->renderSurface->GetDC(&dc)) {
            HGDIOBJ old = SelectObject((HDC)dc, (HGDIOBJ)(font ? font : dialog->dialogFont));
            GetTextExtentPoint32A((HDC)dc, runs[count - 1].field_0x00, runs[count - 1].field_0x08, &extent);
            SelectObject((HDC)dc, old);
            ((PCRenderTarget*)dialog->UnknownInlineField18())->renderSurface->ReleaseDC(dc);
            runs[count - 1].field_0x10 = extent.cx;
        } else {
            runs[count - 1].field_0x10 = 0;
        }
    }
    if (size) {
        size[1] = 0;
        size[0] = length - count * 2;
        for (int j = 0; j < count; j++)
            size[1] += runs[j].field_0x10;
    }
    runs[count].field_0x00 = 0;
    runs[count].field_0x08 = 0;
    runs[count].field_0x04 = 0;
    UnknownGameUiTextRun* result =
        (UnknownGameUiTextRun*)DebugMalloc((count + 1) * sizeof(UnknownGameUiTextRun), __FILE__, 0xa5b);
    if (result)
        memcpy(result, runs, (count + 1) * sizeof(UnknownGameUiTextRun));
    return (char*)result;
}

// 0x00471240: draws the state's image, then the text.
int UIControl::UnknownVirtualSlot40() {
    int frames = -1;
    int redraw = redrawFrames;
    CameraRect rect;
    if (drawnTexture) {
        UnknownVirtualSlot63((CameraRect*)field_0x2c, &rect);
        if (ownerDialog->dialogBackground) {
            ownerDialog->dialogBackground->UnknownFunction404480((PCTextureMap*)drawnTexture, &rect, field_0x4c,
                                                           drawnTexture->field_0x30 ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            frames = redrawFrames;
        } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(
                       &rect, drawnTexture, field_0x4c, drawnTexture->field_0x30 ? 0x1008000 : 0x1000000)) {
            return 0;
        }
    }
    if (controlText && field_0xd0) {
        redrawFrames = redraw;
        UnknownVirtualSlot64((CameraRect*)field_0x2c, &rect);
        void* dc;
        int more;
        if (ownerDialog->BeginControlDraw(&dc, &rect, &more, this)) {
            do {
                if (field_0x1e8) {
                    unsigned int format = DT_WORDBREAK | DT_NOPREFIX;
                    if (textAlign & 2)
                        format = DT_WORDBREAK | DT_NOPREFIX | DT_CENTER;
                    else if (!(textAlign & 1) && (textAlign & 4))
                        format = DT_WORDBREAK | DT_NOPREFIX | DT_RIGHT;
                    SetBkMode((HDC)dc, TRANSPARENT);
                    if (textDrop) {
                        rect.left--;
                        rect.top--;
                        rect.right--;
                        rect.bottom--;
                        SetTextColor((HDC)dc, dropColor);
                        DrawTextA((HDC)dc, controlText, field_0xd0, (RECT*)&rect, format);
                        rect.left++;
                        rect.top++;
                        rect.right++;
                        rect.bottom++;
                    }
                    SetTextColor((HDC)dc, textColor);
                    DrawTextA((HDC)dc, controlText, field_0xd0, (RECT*)&rect, format);
                } else {
                    ownerDialog->DrawAlignedText(dc, &rect, controlText, field_0xd0, textColor, fontHeight,
                                                      (UnknownGameUiTextRun*)textLines, textDrop, dropColor,
                                                      textAlign, 1, 0);
                }
                ownerDialog->EndControlDraw(dc);
            } while (more && ownerDialog->BeginControlDraw(&dc, &rect, &more, this));
        }
        if (frames >= 0)
            redrawFrames = frames;
    }
    return 1;
}

// 0x00474570: the caret keys (Home, End, Left, Right, Delete); measures the
// text and the caret's offset.
int UIEditBox::UnknownVirtualSlot21(int value) {
    if (!UIControl::UnknownVirtualSlot21(value) &&
        ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
        if (field_0x204_sound) {
            field_0x204_sound->SetVolume(ownerGui->field_0x34c, 0);
            field_0x204_sound->PlayWithOptions(1, 0, 0);
        }
        switch (value) {
        case VK_HOME:
            textLength = 0;
            textWidth = 0;
            break;
        case VK_END:
            textLength = field_0xd0;
            break;
        case VK_DELETE:
            if (textLength < field_0xd0) {
                if (IsDBCSLeadByte(controlText[textLength])) {
                    memmove(controlText + textLength, controlText + textLength + 2, field_0xd0 - textLength - 1);
                    field_0xd0 -= 2;
                } else {
                    memmove(controlText + textLength, controlText + textLength + 1, field_0xd0 - textLength);
                    field_0xd0--;
                }
            }
            break;
        case VK_LEFT:
            if (textLength > 0) {
                if (ownerDialog->IsInDoubleByteCharacter(controlText, controlText + textLength - 1, strlen(controlText)))
                    textLength -= 2;
                else
                    textLength--;
            }
            break;
        case VK_RIGHT:
            if (textLength < field_0xd0) {
                if (IsDBCSLeadByte(controlText[textLength]))
                    textLength += 2;
                else
                    textLength++;
            }
            break;
        }
        controlText[field_0xd0] = 0;
        void* dc;
        if (!((PCRenderTarget*)ownerDialog->UnknownInlineField18())->renderSurface->GetDC(&dc)) {
            HGDIOBJ font = SelectObject((HDC)dc, (HGDIOBJ)(fontHandle ? fontHandle : (int)ownerDialog->dialogFont));
            SIZE size;
            GetTextExtentPoint32A((HDC)dc, controlText, field_0xd0, &size);
            textExtent[1] = size.cx;
            GetTextExtentPoint32A((HDC)dc, controlText, textLength, &size);
            textWidth = size.cx;
            SelectObject((HDC)dc, font);
            ((PCRenderTarget*)ownerDialog->UnknownInlineField18())->renderSurface->ReleaseDC(dc);
        }
        UnknownVirtualSlot50();
        return 1;
    }
    return 0;
}

// 0x00474cf0: the joystick moves the focused scroll bar; then hides the
// thumb while every list it follows shows all its rows.
int UIScrollBar::UnknownVirtualSlot10(float frameTime) {
    int range = UnknownVirtualSlot61();
    GUIInputDevice* device;
    for (int i = 0; (device = ownerDialog->guiUser->UnknownFunction488310(i++)) != 0;) {
        if (device->inputDevice->deviceKind == 2 && device->inputDevice->UnknownFunction4897e0(4)) {
            POINT position = device->pointerPosition;
            if (ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
                if (position.y <= -1) {
                    if (ownerDialog->joystickCentred) {
                        if (UnknownFunction4755c0()) {
                            if (UnknownFunction475500())
                                UnknownFunction4753c0(UnknownFunction475500() - 1, UnknownFunction4755c0());
                        } else {
                            if (UnknownFunction475500())
                                UnknownFunction4753c0(UnknownFunction475500() - 1, range);
                        }
                        ownerDialog->joystickCentred = 0;
                    }
                } else if (position.y >= 1) {
                    if (ownerDialog->joystickCentred) {
                        if (UnknownFunction4755c0()) {
                            if ((unsigned int)UnknownFunction475500() < (unsigned int)UnknownFunction4755c0())
                                UnknownFunction4753c0(UnknownFunction475500() + 1, UnknownFunction4755c0());
                        } else {
                            if ((unsigned int)UnknownFunction475500() < (unsigned int)range)
                                UnknownFunction4753c0(UnknownFunction475500() + 1, range);
                        }
                        ownerDialog->joystickCentred = 0;
                    }
                } else {
                    ownerDialog->joystickCentred = 1;
                }
            }
        }
    }
    int result = UIControl::UnknownVirtualSlot10(frameTime);
    if (result) {
        GameObjectIterator* iterator = (GameObjectIterator*)UnknownVirtualSlot53();
        int full = 0;
        for (UIControl* control = UnknownFunction472750(iterator); control;
             control = UnknownFunction472750(iterator)) {
            if (control->controlType == 3 &&
                static_cast<UIListBox*>(control)->visibleRowCount >= static_cast<UIListBox*>(control)->rowCount)
                full = 1;
        }
        UnknownFunction472730(iterator);
        if (full)
            drawnTexture = 0;
    }
    return result;
}

// 0x00474ed0: draws the track centred across the bar, then the thumb.
int UIScrollBar::UnknownVirtualSlot40() {
    CameraRect rect;
    TextureMap* thumb;
    if (field_0x204_image && drawnTexture) {
        TextureMap* track = field_0x204_image->GetCurrentTexture();
        int redraw = redrawFrames;
        rect = *(CameraRect*)field_0x2c;
        if (controlType == 8) {
            rect.top += UnknownMaxInt((field_0x2c[3] - field_0x2c[1] - track->field_0x18) / 2, 0);
            rect.bottom = track->field_0x18 + rect.top;
        } else {
            rect.left += UnknownMaxInt((field_0x2c[2] - field_0x2c[0] - track->field_0x14) / 2, 0);
            rect.right = track->field_0x14 + rect.left;
        }
        if (ownerDialog->dialogBackground)
            ownerDialog->dialogBackground->UnknownFunction404480((PCTextureMap*)track, &rect, 0,
                                                           track->field_0x30 ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
        else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, track, 0,
                                                                   track->field_0x30 ? 0x1008000 : 0x1000000))
            goto failed;
        redrawFrames = redraw;
    }
    thumb = drawnTexture;
    if (thumb) {
        if (controlType == 8) {
            rect.top = (field_0x2c[3] - thumbHeight - field_0x2c[1]) / 2 + field_0x2c[1];
            rect.left = (int)field_0x1ec_float + field_0x2c[0];
            rect.bottom = thumbHeight + rect.top;
            rect.right = thumbWidth + rect.left;
        } else {
            rect.top = (int)field_0x1ec_float + field_0x2c[1];
            rect.left = (field_0x2c[2] - thumbWidth - field_0x2c[0]) / 2 + field_0x2c[0];
            rect.bottom = thumbHeight + rect.top;
            rect.right = thumbWidth + rect.left;
        }
        if (ownerDialog->dialogBackground) {
            ownerDialog->dialogBackground->UnknownFunction404480((PCTextureMap*)thumb, &rect, 0,
                                                           thumb->field_0x30 ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            if (stateImages[currentState]->frameCount == 1)
                needsRedraw = 1;
            else
                UnknownVirtualSlot50();
        } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, thumb, 0,
                                                                     thumb->field_0x30 ? 0x1008000 : 0x1000000)) {
            goto failed;
        }
    }
    return 1;

failed:
    return 0;
}

// 0x004755d0: a click on the bar moves the thumb (slider) or pages the lists.
int UIScrollBar::UnknownVirtualSlot55(int a, int b) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a && b) {
        GameObjectIterator* iterator;
        UIControl* list;
        if (controlType == 8) {
            if (field_0x1f0) {
                field_0x1ec_float = (float)UnknownMinInt(UnknownVirtualSlot61() - thumbWidth,
                                                         UnknownMaxInt(((int*)b)[0] - field_0x2c[0], 0));
            } else {
                float x = (float)((int*)b)[0];
                float left = (float)field_0x2c[0];
                if (x > field_0x1ec_float + (left + thumbWidth)) {
                    iterator = (GameObjectIterator*)UnknownVirtualSlot53();
                    for (list = UnknownFunction472750(iterator); list; list = UnknownFunction472750(iterator)) {
                        if (list->controlType == 3) {
                            UIListBox* listBox = static_cast<UIListBox*>(list);
                            int page = listBox->visibleRowCount;
                            listBox->ScrollToRow(listBox->UnknownFunction4755c0() + page, 1);
                        }
                    }
                    UnknownFunction472730(iterator);
                } else if (x < left + field_0x1ec_float) {
                    iterator = (GameObjectIterator*)UnknownVirtualSlot53();
                    for (list = UnknownFunction472750(iterator); list; list = UnknownFunction472750(iterator)) {
                        if (list->controlType == 3) {
                            UIListBox* listBox = static_cast<UIListBox*>(list);
                            int page = listBox->visibleRowCount;
                            listBox->ScrollToRow(listBox->UnknownFunction4755c0() - page, 1);
                        }
                    }
                    UnknownFunction472730(iterator);
                }
            }
        } else {
            if (field_0x1f0) {
                field_0x1ec_float = (float)UnknownMinInt(UnknownVirtualSlot62() - thumbHeight,
                                                         UnknownMaxInt(((int*)b)[1] - field_0x2c[1], 0));
            } else {
                float y = (float)((int*)b)[1];
                float top = (float)field_0x2c[1];
                if (y > field_0x1ec_float + (top + thumbHeight)) {
                    iterator = (GameObjectIterator*)UnknownVirtualSlot53();
                    for (list = UnknownFunction472750(iterator); list; list = UnknownFunction472750(iterator)) {
                        if (list->controlType == 3) {
                            UIListBox* listBox = static_cast<UIListBox*>(list);
                            int page = listBox->visibleRowCount;
                            listBox->ScrollToRow(listBox->UnknownFunction4755c0() + page, 1);
                        }
                    }
                    UnknownFunction472730(iterator);
                } else if (y < top + field_0x1ec_float) {
                    iterator = (GameObjectIterator*)UnknownVirtualSlot53();
                    for (list = UnknownFunction472750(iterator); list; list = UnknownFunction472750(iterator)) {
                        if (list->controlType == 3) {
                            UIListBox* listBox = static_cast<UIListBox*>(list);
                            int page = listBox->visibleRowCount;
                            listBox->ScrollToRow(listBox->UnknownFunction4755c0() - page, 1);
                        }
                    }
                    UnknownFunction472730(iterator);
                }
            }
        }
        if (field_0x1f0)
            UnknownFunction4753c0(UnknownFunction475300(field_0x1f0), field_0x1f0);
    }
    return UIControl::UnknownVirtualSlot55(a, b);
}

// 0x00475970: dragging the thumb; holding it still repeats kind 0x11.
void UIScrollBar::UnknownVirtualSlot57(int a, int* position) {
    UIControl::UnknownVirtualSlot57(a, position);
    UnknownDialogEvent event;
    event.handled = 0;
    if (!a && position) {
        int width = UnknownVirtualSlot61();
        int height = UnknownVirtualSlot62();
        UnknownVirtualSlot50();
        if (controlType == 8) {
            if (field_0x1f0)
                field_0x1ec_float = (float)UnknownMinInt(width - thumbWidth,
                                                       UnknownMaxInt(position[0] - field_0x2c[0], 0));
            else
                field_0x1ec_float = (float)UnknownMinInt(width - thumbWidth,
                                                       UnknownMaxInt(position[0] - field_0x2c[0] - thumbWidth / 2, 0));
        } else {
            if (field_0x1f0 != 0.0f)
                field_0x1ec_float = (float)UnknownMinInt(height - thumbHeight,
                                                       UnknownMaxInt(position[1] - field_0x2c[1], 0));
            else
                field_0x1ec_float = (float)UnknownMinInt(height - thumbHeight,
                                                       UnknownMaxInt(position[1] - field_0x2c[1] - thumbHeight / 2, 0));
        }
        if (field_0x1f0)
            UnknownFunction4753c0(UnknownFunction475300(field_0x1f0), field_0x1f0);
        if (!field_0x208) {
            event.kind = 0xf;
            event.code = eventCode;
            event.controlName = GetName();
            event.dialog = ownerDialog;
            event.gui = ownerGui;
            event.control = this;
            ownerDialog->UnknownVirtualSlot29(&event);
            field_0x208 = 1;
        }
        if (!event.handled) {
            if (position[0] == dragX && position[1] == dragY) {
                field_0x218_float += ownerDialog->lastFrameTime;
                if (field_0x218_float < field_0x214)
                    goto scroll;
                event.kind = 0x11;
                event.code = eventCode;
                event.controlName = GetName();
                event.dialog = ownerDialog;
                event.gui = ownerGui;
                event.control = this;
                ownerDialog->UnknownVirtualSlot29(&event);
            } else {
                event.kind = 3;
                event.code = eventCode;
                event.controlName = GetName();
                event.dialog = ownerDialog;
                event.gui = ownerGui;
                event.control = this;
                ownerDialog->UnknownVirtualSlot29(&event);
                dragX = position[0];
                dragY = position[1];
                field_0x218_float = 0.0f;
            }
            if (!event.handled) {
            scroll:
                GameObjectIterator* iterator = (GameObjectIterator*)UnknownVirtualSlot53();
                for (UIControl* list = UnknownFunction472750(iterator); list;
                     list = UnknownFunction472750(iterator)) {
                    if (list->controlType == 3) {
                        UIListBox* listBox = static_cast<UIListBox*>(list);
                        listBox->ScrollToRow(
                            UnknownFunction475300(listBox->rowCount - listBox->lastPageRowCount), 0);
                    }
                }
                UnknownFunction472730(iterator);
            }
        }
    }
}

// 0x00476020: the joystick moves the selection; the row under the cursor
// is +0x248.
int UIListBox::UnknownVirtualSlot10(float frameTime) {
    GUIInputDevice* device;
    for (int i = 0; (device = ownerDialog->guiUser->UnknownFunction488310(i++)) != 0;) {
        if (device->inputDevice->deviceKind == 2 && device->inputDevice->UnknownFunction4897e0(4)) {
            POINT position = device->pointerPosition;
            if (ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
                if (position.y <= -1) {
                    if (GetSelectedRow() > 0)
                        SelectRow(GetSelectedRow() - 1);
                } else if (position.y >= 1) {
                    if (GetSelectedRow() < rowCount - 1)
                        SelectRow(GetSelectedRow() + 1);
                }
            }
        }
    }
    field_0x248 = -1;
    GUIUser* user = ownerDialog->guiUser;
    if (user->pointerDevice) {
        POINT cursor = user->pointerDevice->pointerPosition;
        if (user->field_0x1d8 == (UnknownGuiControl*)this && selectable) {
            RECT rect = *(RECT*)field_0x2c;
            for (int row = firstVisibleRow; row < firstVisibleRow + visibleRowCount; row++) {
                rect.bottom = rect.top + GetRowHeight(row);
                if (PtInRect(&rect, cursor) && !(rowTable[row].flags & 1)) {
                    field_0x248 = row;
                    break;
                }
                rect.top += GetRowHeight(row);
            }
        }
    }
    return UIControl::UnknownVirtualSlot10(frameTime);
}

// 0x004761f0: draws the background, then the rows: the text rows on a DC
// (pass 0), then the image rows (pass 1, loading the images of kind 3).
int UIListBox::UnknownVirtualSlot40() {
    void* dc;
    int more;
    CameraRect rect;
    CameraRect area;
    int pass;
    int frames = -1;
    int redraw = redrawFrames;
    int visible = 0;
    int done = 1;
    if (drawnTexture) {
        if (ownerDialog->dialogBackground) {
            ownerDialog->dialogBackground->UnknownFunction404480((PCTextureMap*)drawnTexture, (CameraRect*)field_0x2c,
                                                           field_0x4c,
                                                           drawnTexture->field_0x30 ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            redrawFrames = redraw;
        } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(
                       field_0x2c, drawnTexture, field_0x4c, drawnTexture->field_0x30 ? 0x1008000 : 0x1000000)) {
            goto failed;
        }
    }
    area = *(CameraRect*)field_0x2c;
    area.left += textLeftMargin;
    area.right -= textLeftMargin;
    area.top += textTopMargin;
    area.bottom -= textTopMargin;
    for (pass = 0; pass < 2; pass++) {
        do {
            if (pass == 0) {
                ownerDialog->BeginControlDraw(&dc, &area, &more, this);
                if (itemBrush)
                    FillRect((HDC)dc, (RECT*)&area, (HBRUSH)itemBrush);
            } else {
                more = 0;
                frames = redrawFrames;
                redrawFrames = redraw;
            }
            int y = area.top;
            for (int row = firstVisibleRow; row < rowCount; row++) {
                int height = GetRowHeight(row);
                int whole;
                if (field_0x240 && visible <= 0) {
                    whole = 0;
                } else {
                    whole = 1;
                    if (height + y > area.bottom)
                        break;
                }
                switch ((int)UnknownFunction476970(row)) {
                case 3:
                    if (pass == 1) {
                        if (rowTable[row].imageFile) {
                            if (rowTable[row].image)
                                rowTable[row].image->Release();
                            rowTable[row].image =
                                new(__FILE__, 0x1c62) UIAnim(ownerDialog->dialogTextures, 0);
                            rowTable[row].image->LoadFile(rowTable[row].imageFile,
                                                                                ownerDialog->dialogPalette);
                            rowTable[row].width = rowTable[row].image->frameList[0]->frameWidth;
                            rowTable[row].height = rowTable[row].image->frameList[0]->frameHeight;
                            height = rowTable[row].height;
                        } else {
                            rowTable[row].image = 0;
                        }
                        rowTable[row].kind = 2;
                        UnknownVirtualSlot50();
                    }
                    if (whole && height + y > area.bottom)
                        break;
                case 2:
                    if (pass == 1 && rowTable[row].image) {
                        int width = rowTable[row].image->GetCurrentFrame()->frameWidth;
                        int imageHeight = rowTable[row].image->GetCurrentFrame()->frameHeight;
                        rect.left = field_0x230 + area.left;
                        rect.top = y;
                        rect.right = rect.left + width;
                        if (rect.right >= area.right)
                            rect.right = area.right;
                        if (field_0x240) {
                            if (imageHeight + y >= area.bottom)
                                rect.bottom = area.bottom;
                            else
                                rect.bottom = imageHeight + y;
                        } else {
                            rect.bottom = imageHeight + y;
                        }
                        TextureMap* texture = rowTable[row].image->GetCurrentFrame()->frameTexture;
                        if (ownerDialog->dialogBackground) {
                            ownerDialog->dialogBackground->UnknownFunction404480(
                                (PCTextureMap*)texture, &rect, 0, texture->field_0x30 ? 0x1008000 : 0x1000000,
                                backgroundRegion, needsRedraw, &redrawFrames, 0);
                            if (rowTable[row].image->frameCount > 1) {
                                done = 0;
                                frames = redrawFrames;
                            }
                        } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(
                                       &rect, texture, 0, texture->field_0x30 ? 0x1008000 : 0x1000000)) {
                            goto failed;
                        }
                        visible++;
                    }
                    break;
                case 1:
                    if (pass == 0 && rowTable[row].text) {
                        rect.left = area.left;
                        rect.top = y;
                        rect.bottom = height + y;
                        rect.right = area.right;
                        SetBkMode((HDC)dc, TRANSPARENT);
                        unsigned int color;
                        if (row == selectedRow && selectable) {
                            if (selectBrush)
                                FillRect((HDC)dc, (RECT*)&rect, (HBRUSH)selectBrush);
                            color = selectColor;
                        } else {
                            if (rowTable[row].brush)
                                FillRect((HDC)dc, (RECT*)&rect, (HBRUSH)rowTable[row].brush);
                            color = rowTable[row].textColor;
                            if (!color)
                                color = textColor;
                        }
                        if (rowTable[row].width) {
                            rect.left += field_0x234 + field_0x230;
                            rect.right += field_0x234 + field_0x230;
                            ownerDialog->DrawAlignedText(dc, &rect, rowTable[row].text,
                                                              rowTable[row].width, color, fontHeight,
                                                              (UnknownGameUiTextRun*)rowTable[row].textLines,
                                                              textDrop, dropColor, textAlign, 1, 0);
                        }
                        if (!more)
                            visible++;
                    }
                    break;
                }
                if (field_0x248 == row && pass == 0 && visibleRowCount > 1) {
                    rect.left = area.left;
                    rect.top = y;
                    rect.bottom = height + y;
                    rect.right = area.right;
                    if (itemBrush)
                        FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)itemBrush);
                    else
                        FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
                    UnknownVirtualSlot50();
                }
                y += height;
            }
            if (pass == 0)
                ownerDialog->EndControlDraw(dc);
        } while (more);
    }
    if (frames >= 0)
        redrawFrames = frames;
    visibleRowCount = visible;
    needsRedraw = done;
    return 1;

failed:
    return 0;
}

// 0x00479a00: a click on the bar moves the thumb (slider) or pages the list.
int UIDDLScrollBar::UnknownVirtualSlot55(int a, int b) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a && b) {
        UIListBox* list = ownerList->listPart;
        if (field_0x1f0) {
            field_0x1ec_float = (float)UNKNOWN_MIN(UnknownVirtualSlot62() - thumbHeight,
                                                   UNKNOWN_MAX(((int*)b)[1] - field_0x2c[1], 0));
        } else {
            float y = (float)((int*)b)[1];
            float top = (float)field_0x2c[1];
            if (y > field_0x1ec_float + (top + thumbHeight))
            {
                int page = list->visibleRowCount;
                list->ScrollToRow(list->UnknownFunction4755c0() + page, 1);
            } else if (y < top + field_0x1ec_float) {
                int page = list->visibleRowCount;
                list->ScrollToRow(list->UnknownFunction4755c0() - page, 1);
            }
        }
    }
    if (field_0x1f0)
        UnknownFunction4753c0(UnknownFunction475300(field_0x1f0), field_0x1f0);
    return UIControl::UnknownVirtualSlot55(a, b);
}

// 0x0047ab20
void UIDropDownList::UnknownFunction47ab20(UIAnim* image) {
    staticPart2->SetImage(0, image);
    staticPart2->field_0x2c[1] = 0;
    staticPart2->field_0x2c[0] = 0;
    staticPart2->field_0x2c[2] = image->GetCurrentFrame()->frameWidth;
    staticPart2->field_0x2c[3] = image->GetCurrentFrame()->frameHeight;
    *(CameraRect*)staticPart2->field_0x3c = *(CameraRect*)staticPart2->field_0x2c;
    field_0x2c[2] = staticPart2->field_0x2c[2] + field_0x2c[0];
    field_0x2c[3] = staticPart2->field_0x2c[3] + field_0x2c[1];
    *(CameraRect*)field_0x3c = *(CameraRect*)field_0x2c;
    staticPart0->field_0x2c[0] = 0;
    staticPart0->field_0x2c[1] = staticPart2->field_0x2c[3];
    *(CameraRect*)staticPart0->field_0x3c = *(CameraRect*)staticPart0->field_0x2c;
    *(CameraRect*)listPart->field_0x3c = *(CameraRect*)staticPart0->field_0x2c;
    *(CameraRect*)listPart->field_0x2c = *(CameraRect*)listPart->field_0x3c;
    if (buttonPart->field_0x3c[0]) {
        buttonPart->field_0x2c[1] = 0;
        buttonPart->field_0x2c[0] = staticPart2->field_0x2c[2];
        buttonPart->field_0x2c[2] =
            buttonPart->UnknownVirtualSlot48(-1)->field_0x14 + buttonPart->field_0x2c[0];
        buttonPart->field_0x2c[3] =
            buttonPart->UnknownVirtualSlot48(-1)->field_0x18 + buttonPart->field_0x2c[1];
        *(CameraRect*)buttonPart->field_0x3c = *(CameraRect*)buttonPart->field_0x2c;
    }
    staticPart1->field_0x2c[0] = staticPart2->field_0x2c[2];
    staticPart1->field_0x2c[1] = staticPart2->field_0x2c[3];
    *(CameraRect*)staticPart1->field_0x3c = *(CameraRect*)staticPart1->field_0x2c;
    *(CameraRect*)scrollBarPart->field_0x2c = *(CameraRect*)staticPart1->field_0x2c;
    scrollBarPart->field_0x2c[1] += 2;
    scrollBarPart->field_0x2c[3] -= 2;
    *(CameraRect*)scrollBarPart->field_0x3c = *(CameraRect*)scrollBarPart->field_0x2c;
}

// 0x0047b370 (UIProgressBar): one step on; redraws when `redraw`.
void UIProgressBar::UnknownFunction47b370(int redraw) {
    int total = field_0x1f0;
    if (total) {
        stepsDone = UnknownMinInt(stepsDone + 1, total);
        progressFraction = (float)stepsDone / total;
    }
    if (redraw && ownerGui)
        ownerGui->RedrawFrame();
}

// ---------------------------------------------------------------------------
// UIVideoStatic

// 0x0047ae30
UIVideoStatic::UIVideoStatic(int flags, CameraRect* area, UIDialog* owner)
    : UIStatic(flags, area, (UnknownGameUiDialog*)owner) {
    field_0x1ec = 0;
}

// 0x0047ae90
int UIVideoStatic::UnknownFunction47ae90(const char* file, int a, void (*done)(UIDialog* dialog), UIDialog* owner) {
    if (field_0x1ec)
        field_0x1ec->Release();
    field_0x1ec = new(__FILE__, 0x264d) MediaControl(1);
    if (!field_0x1ec->UnknownFunction4a2560(field_0x18, file, done, owner)) {
        field_0x1ec = 0;
        return 0;
    }
    AppendChild(field_0x1ec, -1);
    field_0x1f0[0] = 0;
    field_0x1f0[1] = 0;
    field_0x1f0[2] = field_0x1ec->field_0x30;
    field_0x1f0[3] = field_0x1ec->field_0x34;
    if (a)
        field_0x1ec->Restart();
    return 1;
}

// 0x0047af90
int UIVideoStatic::UnknownVirtualSlot10(float frameTime) {
    return UIControl::UnknownVirtualSlot10(frameTime);
}

// 0x0047afa0
int UIVideoStatic::UnknownVirtualSlot40() {
    CameraRect rect;
    if (!field_0x1ec)
        return 0;
    if (ownerDialog->dialogBackground) {
        ownerDialog->dialogBackground->UnknownFunction404c80();
        UnknownVirtualSlot50();
    }
    UnknownVirtualSlot63((CameraRect*)field_0x2c, &rect);
    return ((PCRenderTarget*)field_0x18)->renderSurface->Blt(&rect, field_0x1ec->field_0x2c, field_0x1f0,
                                                                      0x1000000, 0) == 0;
}

// ---------------------------------------------------------------------------
// UIProgressBar

// 0x0047b020
UIProgressBar::UIProgressBar(int flags, CameraRect* area, UIDialog* owner)
    : UIStatic(flags, area, (UnknownGameUiDialog*)owner) {
    progressFraction = 0;
    field_0x1f0 = 0;
    stepsDone = 0;
    barTexture = 0;
    showPercentage = 1;
}

// 0x0047b090
UIProgressBar::~UIProgressBar() {
    if (barTexture && ownsBarTexture)
        barTexture->Release();
}

// 0x0047b100
TextureMap* UIProgressBar::UnknownVirtualSlot48(int state) {
    return barTexture;
}

// 0x0047b110
int UIProgressBar::UnknownVirtualSlot40() {
    int redraw = redrawFrames;
    CameraRect rect;
    void* dc;
    int more;
    if (barTexture && progressFraction != 0.0f) {
        UnknownVirtualSlot63((CameraRect*)field_0x2c, &rect);
        CameraRect source = *(CameraRect*)field_0x4c;
        source.right = (int)((field_0x4c[2] - field_0x4c[0]) * progressFraction);
        UnknownGameUiDialog* owner = ownerDialog;
        rect.right = (int)(source.right * owner->scaleX + rect.left);
        rect.bottom = (int)(source.bottom * owner->scaleY + rect.top);
        if (owner->dialogBackground) {
            owner->dialogBackground->UnknownFunction404480((PCTextureMap*)barTexture, &rect, &source,
                                                      barTexture->field_0x30 ? 0x1008000 : 0x1000000,
                                                      backgroundRegion, needsRedraw, &redraw, 0);
        } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(
                       &rect, barTexture, &source, barTexture->field_0x30 ? 0x1008000 : 0x1000000)) {
            return 0;
        }
    }
    if (showPercentage) {
        redraw = redrawFrames;
        UnknownVirtualSlot64((CameraRect*)field_0x2c, &rect);
        char text[16];
        sprintf(text, "%3.0f%%", progressFraction * 100.0f);
        if (ownerDialog->BeginControlDraw(&dc, &rect, &more, this)) {
            do {
                ownerDialog->DrawAlignedText(dc, &rect, text, strlen(text), textColor, fontHeight, 0,
                                                  textDrop, dropColor, 2, 1, 0);
                ownerDialog->EndControlDraw(dc);
            } while (more && ownerDialog->BeginControlDraw(&dc, &rect, &more, this));
        }
        redrawFrames = redraw;
    }
    return 1;
}

// 0x0047b3d0
void UIProgressBar::UnknownFunction47b3d0(int texture, int owned) {
    barTexture = (TextureMap*)texture;
    ownsBarTexture = owned;
}

// ---------------------------------------------------------------------------
// Input, image and list slots promoted from the near-miss sample

// 0x004715d0
int UIControl::UnknownVirtualSlot41() {
    if (field_0x164 == 0 && field_0xb0 && !field_0x25_bit2) {
        g_UnknownGlobal65b60c = field_0xb0 < 0 ? 7 : field_0xb0;
        field_0x88 = 21.0f;
        field_0x8c = 11.0f;
        int left = field_0x3c[0];
        int width = field_0x3c[2] - left;
        field_0x80 = max(0.0f, (float)(width / 2 + left) - 10.5f);
        int top = field_0x3c[1];
        int height = field_0x3c[3] - top;
        field_0x84 = max(0.0f, (float)(height / 2 + top) - 5.5f);
        field_0x90 = ((float)left - field_0x80) / g_UnknownGlobal65b60c;
        field_0x94 = ((float)top - field_0x84) / g_UnknownGlobal65b60c;
        field_0x98 = ((float)width - 21.0f) / g_UnknownGlobal65b60c;
        field_0x9c = ((float)height - 11.0f) / g_UnknownGlobal65b60c;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        field_0x80 += field_0x90;
        field_0x84 += field_0x94;
        field_0x88 += field_0x98;
        field_0x8c += field_0x9c;
    }
    if (field_0x164 == g_UnknownGlobal65b60c)
        return 1;
    UnknownVirtualSlot50();
    if (!field_0x25_bit2 || field_0x164) {
        float width = field_0x88;
        field_0x2c[0] = (int)field_0x80;
        field_0x2c[1] = (int)field_0x84;
        field_0x2c[2] = (int)(width + field_0x80);
        field_0x2c[3] = (int)(field_0x84 + field_0x8c);
        switch (controlType) {
        case 1:
        case 4:
        case 5:
        case 9:
        case 10:
            drawnTexture = stateImages[currentState]->GetCurrentTexture();
            field_0x164++;
            break;
        default:
            drawnTexture = 0;
            return 1;
        }
    }
    return 0;
}

// 0x00472130
int UIControl::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int handled = 0;
    GUIInputDevice* device = ownerDialog->guiUser->UnknownFunction4882a0(event);
    if (device) {
        POINT position = device->pointerPosition;
        if (event->kind)
            handled = UnknownVirtualSlot56(event->control, (int*)&position);
    }
    if (!handled)
        return GameObject::UnknownVirtualSlot22(event, entry);
    return 1;
}

// 0x004753c0
int UIScrollBar::UnknownFunction4753c0(int value, int range) {
    unsigned int position = field_0x21c ? range - value : value;
    UnknownVirtualSlot50();
    if (!field_0x1f0 && UnknownFunction475300(range) == position)
        return 1;
    if (!range) {
        field_0x1ec_float = position;
        return 1;
    }
    unsigned int travel = controlType == 8 ? UnknownVirtualSlot61() - thumbWidth : UnknownVirtualSlot62() - thumbHeight;
    if ((unsigned int)range > 0) {
        float limit = travel;
        float scaled = (double)(travel * position) / (unsigned int)range;
        field_0x1ec_float = scaled < limit ? scaled : limit;
        return 1;
    }
    return 0;
}

// 0x00477730
void UIListBox::ScrollBy(int delta) {
    int handled = 0;
    int count = rowCount;
    if (!count)
        return;
    int first = firstVisibleRow += delta;
    if (field_0x244) {
        if (first < 0)
            firstVisibleRow = first + count;
        int index = firstVisibleRow % count;
        if (index < 0)
            index = -index;
        firstVisibleRow = index;
    } else {
        int last = count - visibleRowCount;
        if (last < first)
            first = last;
        firstVisibleRow = UnknownMaxInt(first, 0);
    }
    if (visibleRowCount == 1) {
        UnknownVirtualSlot65(firstVisibleRow);
        UnknownVirtualSlot66(&handled);
        if (handled)
            return;
    }
    UnknownVirtualSlot50();
    UpdateScrollBars();
}

// 0x00477ff0
int UIListBox::UnknownVirtualSlot56(int a, int* position) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && a == 1) {
        field_0x230 += field_0x234;
        field_0x234 = 0;
    }
    return UIControl::UnknownVirtualSlot56(a, position);
}

// 0x00478040
int UIListBox::UnknownVirtualSlot21(int key) {
    if (!UIControl::UnknownVirtualSlot21(key) &&
        ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this && selectable) {
        int row;
        switch (key) {
        case VK_LEFT:
        case VK_UP:
            if (GetSelectedRow() <= 0)
                return 0;
            SelectRow(GetSelectedRow() - 1);
            break;
        case VK_RIGHT:
        case VK_DOWN:
            if (GetSelectedRow() >= rowCount - 1)
                return 0;
            SelectRow(GetSelectedRow() + 1);
            break;
        case VK_HOME:
            if (GetSelectedRow() <= 0)
                return 0;
            SelectRow(0);
            break;
        case VK_END:
            if (GetSelectedRow() >= rowCount - 1)
                return 0;
            SelectRow(rowCount - 1);
            break;
        case VK_PRIOR:
            row = GetSelectedRow() - visibleRowCount;
            if (row < 0)
                row = 0;
            if (row == GetSelectedRow())
                return 0;
            SelectRow(row);
            break;
        case VK_NEXT:
            row = GetSelectedRow() + visibleRowCount;
            if (row > rowCount - 1)
                row = rowCount - 1;
            if (row == GetSelectedRow())
                return 0;
            SelectRow(row);
            break;
        default:
            return 0;
        }
        int handled;
        UnknownVirtualSlot66(&handled);
        return 1;
    }
    return 0;
}

// 0x00478810
TextureMap* UIMultiState::UnknownVirtualSlot48(int state) {
    UIAnim* image;
    if (currentState == 4 && stateTable[selectedState].focusImage)
        image = stateTable[selectedState].focusImage;
    else
        image = stateTable[selectedState].image;
    if (image)
        return image->GetCurrentTexture();
    return 0;
}

// 0x00479df0
void UIDDLListBox::UnknownVirtualSlot66(int* handled) {
    UnknownDialogEvent event;
    event.handled = 0;
    ownerList->SetText(GetRowText(GetSelectedRow()));
    ownerList->UnknownFunction47a2d0(0);
    event.kind = kDialogListSelect;
    event.code = eventCode;
    event.controlName = ownerList->GetName();
    event.dialog = ownerDialog;
    event.control = this;
    event.gui = ownerDialog->guiManager;
    ownerDialog->UnknownVirtualSlot29(&event);
}

// 0x00472bc0
int UIFrame::UnknownFunction472bc0(void* stream, int offset, void* palette) {
    if (stream) {
        UnknownTgaFile* image = UnknownFunction511dd0((UnknownTextureStream*)stream, 0, offset);
        if (image) {
            int format = g_TrackGame->renderTarget->field_0x28;
            frameWidth = image->width;
            frameHeight = image->height;
            frameTexture = new(__FILE__, 0x13d6) PCTextureMap((TextureMapManager*)field_0x20, 1);
            Palette8* pal = (Palette8*)palette;
            frameTexture->UnknownVirtualSlot4(image->bits, image->width, image->height, image->width,
                                            image->width, 0x22b, format,
                                            (UnknownTexturePalette*)(pal ? pal->field_0x708 : 0), 4,
                                            pal ? pal->field_0x70c : 0, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
            if (UnknownFunction47b490(frameTexture, 0xff00ff))
                frameTexture->UnknownVirtualSlot18(0xff00ff);
            UnknownFunction512dd0(image);
            return 1;
        }
    }
    return 0;
}

// 0x00471fd0: a key or button press. A keyboard press of this control's
// "KeyBind" without Shift focuses it and presses it (slots 55 and 56);
// other presses go to slot 55 with the pointer position.
int UIControl::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    int handled = 0;
    if (ownerDialog->guiUser->UnknownFunction4881d0(event)) {
        if (event->kind == 0) {
            if (keyBind && keyBind == event->control &&
                !g_TrackGame->controlInterface->keyboard->UnknownVirtualSlot5(0x2a, 0x3f, 0) &&
                !g_TrackGame->controlInterface->keyboard->UnknownVirtualSlot5(0x36, 0x3f, 0) &&
                IsEnabled() && field_0x70) {
                ownerDialog->guiUser->UnknownFunction487730((UnknownGuiControl*)this, 0, 1);
                UnknownVirtualSlot55(0, 0);
                UnknownVirtualSlot56(0, 0);
                return 1;
            }
            if (UnknownFunction43caa0(0x32, event->kind, event, 0x80) &&
                ownerDialog->guiUser->field_0x1dc == (int)this)
                moveable = 1;
        }
        if (event->kind) {
            POINT position;
            GUIInputDevice* device = ownerDialog->guiUser->pointerDevice;
            if (device)
                position = device->pointerPosition;
            handled = UnknownVirtualSlot55(event->control, ownerDialog->guiUser->pointerDevice ? (int)&position : 0);
        }
    }
    if (!handled)
        return GameObject::UnknownVirtualSlot23(event, entry);
    return 1;
}

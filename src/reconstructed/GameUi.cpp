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
void* ResizeDebugBlock(void* block, unsigned int size);
// 0x0065b5c8: the time stamp of the last image step (UIAnim 0x00472fe0).
int g_UnknownGlobal65b5c8;
// 0x0065b608: the dialog whose list box is being sorted (0x00477900).
UIDialog* g_SortingDialog;
// 0x0065b60c: the frames of the zoom transition (UIControl slot 41).
int g_UnknownGlobal65b60c;

// cdecl 0x00477b60: the list rows' default order (by text).
int CompareRowText(const void* a, const void* b);
// cdecl 0x00477800: the qsort comparison 0x00477900 sorts with.
int CompareSortedRows(const void* a, const void* b);
// cdecl 0x0047b490: whether `texture` has a pixel of colour `key`
// (samples/ui/GameUiNearMisses.cpp).
int TextureHasColor(TextureMap* texture, int key);

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
int UIControl::GetControlText() {
    return (int)controlText;
}

// 0x004703d0
int UIControl::GetTextLength() {
    return controlTextLength;
}

// 0x004703e0
int UIControl::GetTextLines() {
    return (int)textLines;
}

// 0x004703f0
void* UIControl::GetTextExtent() {
    return textExtent;
}

// 0x00470400
void UIControl::SetAttachId(int id) {
    attachId = id;
}

// 0x00470410
int UIControl::GetWidth() {
    return screenArea[2] - screenArea[0];
}

// 0x00470420
int UIControl::GetHeight() {
    return screenArea[3] - screenArea[1];
}

// 0x00470640
void UIControl::RequestRedraw() {
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
    RequestRedraw();
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
UIControl* UIControl::FindLastAnchor(UIControl* none) {
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
void UIControl::SetState(int state) {
    currentState = state;
    RequestRedraw();
}

// 0x004709d0
void UIControl::SetShapeBounds(int value) {
    shapeBounds = value;
    RequestRedraw();
}

// 0x00470a70
void UIControl::BindValue(int* value) {
    boundValue = value;
}

// 0x00470d40
void UIControl::SetFontColor(unsigned int color) {
    textColor = color;
    RequestRedraw();
}

// 0x00470d60
void UIControl::SetTextDrop(int value) {
    textDrop = value;
    RequestRedraw();
}

// 0x00470d80
void UIControl::SetDropColor(int value) {
    dropColor = value;
    RequestRedraw();
}

// 0x00470da0
void UIControl::SetTextAlign(int value) {
    textAlign = value;
    RequestRedraw();
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

// 0x00470f10: scales the design rect by the dialog's scale into the screen
// rect (+0x2c), offsets it by the anchor control or the dialog, and clips
// the visible part (+0x4c) to the render target.
void UIControl::ScaleArea() {
    UIDialog* owner = ownerDialog;
    int left = bounds[0];
    screenArea[0] = (int)(left * owner->scaleX);
    int top = bounds[1];
    screenArea[1] = (int)(top * owner->scaleY);
    int right = bounds[2];
    screenArea[2] = (int)(right * owner->scaleX);
    int bottom = bounds[3];
    screenArea[3] = (int)(bottom * owner->scaleY);
    if (anchorControl && relAnchor) {
        screenArea[1] += anchorControl->screenArea[1];
        screenArea[3] += anchorControl->screenArea[1];
        screenArea[0] += anchorControl->screenArea[0];
        screenArea[2] += anchorControl->screenArea[0];
    } else {
        screenArea[1] += owner->screenArea.top;
        screenArea[3] += owner->screenArea.top;
        screenArea[0] += owner->screenArea.left;
        screenArea[2] += owner->screenArea.left;
    }
    visibleArea[0] = -screenArea[0] < 0 ? 0 : -screenArea[0];
    visibleArea[1] = -screenArea[1] < 0 ? 0 : -screenArea[1];
    int width = right - left;
    int height = bottom - top;
    int over = screenArea[2] - g_TrackGame->renderTarget->targetWidth;
    int under = screenArea[3] - g_TrackGame->renderTarget->targetHeight;
    if (over > 0)
        width -= over;
    visibleArea[2] = width;
    if (under > 0)
        height -= under;
    visibleArea[3] = height;
}

// 0x00471fa0
TextureMap* UIControl::GetStateTexture(int state) {
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
    passCountdown = count;
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

// 0x00472fe0: steps to the next frame once the frame delay has passed (or at
// once past a sound frame), bouncing or looping as the pass count says.
UIFrame* UIAnim::Advance() {
    int now = ReadClock();
    g_UnknownGlobal65b5c8 = now;
    if (abs(now - lastStepTime) > frameDelay || frameList[currentFrame]->isSound == 1) {
        lastStepTime = now;
        if (playBackwards) {
            if (!playForwards && currentFrame > 0) {
                currentFrame--;
            } else {
                playForwards = 0;
                if (passesLeft == 0x7fff || (passesLeft && --passCountdown > 0))
                    currentFrame = frameCount;
            }
        } else {
            if (!playForwards && currentFrame + 1 < frameCount) {
                currentFrame++;
            } else {
                playForwards = 0;
                if (passesLeft == 0x7fff || (passesLeft && --passCountdown > 0))
                    currentFrame = 0;
            }
        }
    }
    return frameList[currentFrame];
}

// ---------------------------------------------------------------------------
// UIButton

// 0x004732c0
UIButton::~UIButton() {
}

// 0x004732d0
void UIButton::SetStateImages(UIAnim* up, UIAnim* over, UIAnim* down, UIAnim* disabled) {
    stateImages[0] = up;
    stateImages[1] = over;
    stateImages[2] = down;
    stateImages[3] = over;
    stateImages[4] = disabled;
    RequestRedraw();
}

// 0x00473310
void UIButton::SetNormalImage(UIAnim* image) {
    stateImages[0] = image;
    RequestRedraw();
}

// 0x00473330
void UIButton::SetFocusImage(UIAnim* image) {
    stateImages[1] = image;
    stateImages[3] = image;
    RequestRedraw();
}

// 0x00473350
void UIButton::SetPushImage(UIAnim* image) {
    stateImages[2] = image;
    RequestRedraw();
}

// 0x00473370
void UIButton::SetDisabledImage(UIAnim* image) {
    stateImages[4] = image;
    RequestRedraw();
}

// 0x00473390
void UIButton::SetSortList(UIListBox* list) {
    sortList = list;
}

// ---------------------------------------------------------------------------
// UIStatic

// 0x00478f60
UIStatic::UIStatic(int id, CameraRect* area, UIDialog* owner)
    : UIControl(5, id, area, owner) {
}

// 0x00478fb0
UIStatic::~UIStatic() {
}

// 0x00478fc0
TextureMap* UIStatic::GetStateTexture(int state) {
    if (stateImages[0])
        return stateImages[0]->GetCurrentTexture();
    return 0;
}

// 0x00478fe0
int UIStatic::IsInteractive() {
    return 0;
}

// ---------------------------------------------------------------------------
// UIControl (continued)

// 0x00470990
int UIControl::ContainsPoint(POINT point, int state) {
    RECT rect = *(RECT*)screenArea;
    return PtInRect(&rect, point);
}

// 0x004709f0
int UIControl::HitTest(POINT point) {
    RECT rect = *(RECT*)screenArea;
    if (!shapeBounds) {
        if (PtInRect(&rect, point))
            return 1;
    } else if (ContainsPoint(point, currentState)) {
        return 1;
    }
    return 0;
}

// 0x00471070
void UIControl::AdvanceImages() {
    if (stateImages[currentState])
        stateImages[currentState]->Advance();
    if (transitionState == 1 || transitionState == 2) {
        if (fxAnimIn)
            fxAnimIn->Advance();
        if (fxAnimOut)
            fxAnimIn->Advance();
    }
}

// 0x004710c0
void UIControl::UpdateNeedsRedraw() {
    UIControl* last = FindLastAnchor(0);
    if (!shown || (last && (!last->shown || last->transitionState))) {
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
    UIControl* last = FindLastAnchor(0);
    if ((!last || (last->shown && !last->transitionState)) && !post3D) {
        if (shown)
            return Draw();
        needsRedraw = 1;
    }
    return result;
}

// 0x00472250
int UIControl::ButtonUp(int a, int* position) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a && currentState == 2) {
        SetState(3);
        PlayControlSound(3, ownerDialog->field_0x180);
    }
    heldButton = -1;
    return 0;
}

// 0x004722b0
void UIControl::ButtonHeld(int a, int* position) {
    if (moveable && a == 1) {
        RequestRedraw();
        int width = GetWidth();
        int height = GetHeight();
        UIDialog* owner = ownerDialog;
        bounds[0] = position[0] - owner->screenArea.left;
        bounds[2] = bounds[0] + width;
        bounds[1] = position[1] - owner->screenArea.top;
        bounds[3] = bounds[1] + height;
    }
}

// 0x004725b0
int UIControl::EnterHover() {
    if (IsInteractive() && ownerDialog->guiUser->hoverControl != (UnknownGuiControl*)this) {
        SetState(1);
        if (stateImages[currentState])
            stateImages[currentState]->Rewind();
        PlayControlSound(1, ownerDialog->field_0x180);
        ToolTip* tip = ownerDialog->guiUser->userToolTip;
        if (tip)
            tip->UnknownFunction486b10((UnknownGuiControl*)this);
    }
    return 1;
}

// 0x00472630
void UIControl::LeaveHover(int value) {
    if (enabled)
        SetState(0);
    ToolTip* tip = ownerDialog->guiUser->userToolTip;
    if (tip)
        tip->ShowText(0, 0, 1.0f);
}

// 0x00472670
void UIControl::PlayControlSound(int index, int a) {
    if (sounds[index]) {
        sounds[index]->SetVolume(ownerGui->field_0x34c, 0);
        sounds[index]->PlayWithOptions(1, 0, 0);
    }
}

// 0x004726b0
int UIControl::CreateControlIterator() {
    return (int)new(__FILE__, 0x1267) GameObjectIterator(ownerDialog->controlContainer, 1, "UIControl");
}

// 0x00472730
void UIControl::DeleteControlIterator(GameObjectIterator* iterator) {
    delete iterator;
}

// 0x00472750
UIControl* UIControl::NextAttachedControl(GameObjectIterator* iterator) {
    if (!attachId)
        return 0;
    UIControl* control;
    do {
        control = (UIControl*)iterator->Next();
    } while ((control && control->attachId != attachId) || control == this);
    return control;
}

// 0x00472790
UIControl* UIControl::NextAttachedControlOfType(GameObjectIterator* iterator) {
    UIControl* control = NextAttachedControl(iterator);
    while (control && control->controlType != controlType)
        control = NextAttachedControl(iterator);
    return control;
}

// 0x00472860
void UIControl::GetImageRect(CameraRect* in, CameraRect* out) {
    *out = *in;
    if (sourceBlit && drawnTexture) {
        out->right = (int)(drawnTexture->textureWidth * ownerDialog->scaleX) + out->left;
        out->bottom = (int)(drawnTexture->textureHeight * ownerDialog->scaleY) + out->top;
    }
}

// 0x004728e0
void UIControl::GetTextRect(CameraRect* in, CameraRect* out) {
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
UIButton::UIButton(int id, CameraRect* area, UIDialog* owner)
    : UIControl(1, id, area, owner) {
    sortList = 0;
    SetStateImages(0, 0, 0, 0);
    SetSound(0, 0);
    SetSound(1, 0);
    SetSound(2, 0);
    SetSound(3, 0);
    SetSound(4, 0);
}

// 0x00473410
int UIButton::ButtonUp(int a, int* position) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a && currentState == 2) {
        UnknownDialogEvent event;
        event.handled = a;
        if (sortList)
            sortList->Sort(1);
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->HandleEvent(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::ButtonUp(a, position);
}

// ---------------------------------------------------------------------------
// UIEditBox

// 0x00473630
UIEditBox::UIEditBox(int id, CameraRect* area, UIDialog* owner, int size, int a, int b)
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
    textScrollX = 0;
    textLength = 0;
    controlTextLength = 0;
    textWidth = 0;
    acceptedCharacters = 0;
    backgroundBrush = 0;
    leadBytePending = 0;
    dropTrailByte = 0;
    if (!text)
        controlText = (char*)DebugMalloc(textCapacity, __FILE__, 0x15c7);
    controlText[0] = 0;
}

// 0x00473770
UIEditBox::~UIEditBox() {
    if (boundValue)
        SyncBoundValue(1);
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
    UIControl::BindValue((int*)buffer);
    boundBufferSize = size;
}

// 0x00473840
void UIEditBox::SyncBoundValue(int value) {
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
void UIEditBox::SetClickSound(int value) {
    field_0x204 = value;
}

// 0x00473d90
void UIEditBox::SetFullSound(int value) {
    field_0x208 = value;
}

// 0x00473ef0
char* UIEditBox::GetEditText(char* buffer, int size) {
    if (controlTextLength + 1 < size)
        size = controlTextLength + 1;
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
    RequestRedraw();
}

// 0x004740b0
void UIEditBox::KillFocus(int value) {
    UnknownDialogEvent event;
    event.handled = 0;
    event.kind = kDialogEditDone;
    event.code = eventCode;
    event.controlName = GetName();
    event.dialog = ownerDialog;
    event.gui = ownerGui;
    event.control = this;
    ownerDialog->HandleEvent(&event);
    RequestRedraw();
    UIControl::KillFocus(value);
}

// 0x00474120
int UIEditBox::ButtonDown(int a, int b) {
    if (ownerDialog->guiUser->focusControl != (UnknownGuiControl*)this)
        return UIControl::ButtonDown(a, b);
    return 0;
}

// ---------------------------------------------------------------------------
// UIScrollCtl

// 0x00474820
UIScrollCtl::UIScrollCtl(int type, int id, CameraRect* area, UIDialog* owner)
    : UIButton(id, area, owner) {
    controlType = type;
}

// 0x00474870
UIScrollCtl::~UIScrollCtl() {
}

// 0x00474880: a click scrolls the arrow's lists by a row, then repeats on a timer.
int UIScrollCtl::ButtonDown(int a, int b) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.handled = 0;
        GameObjectIterator* iterator;
        UIControl* control;
        if (controlType == 9) {
            iterator = (GameObjectIterator*)CreateControlIterator();
            for (control = NextAttachedControl(iterator); control; control = NextAttachedControl(iterator)) {
                if (control->controlType == 3) {
                    ((UIListBox*)control)->lastClickTime = 0;
                    ((UIListBox*)control)->ScrollBy(-1);
                }
                if (control->controlType == 6) {
                    ((UIDropDownList*)control)->listPart->lastClickTime = 0;
                    ((UIDropDownList*)control)->listPart->ScrollBy(-1);
                }
            }
            DeleteControlIterator(iterator);
            event.code = eventCode;
            event.kind = 0xe;
            event.controlName = controlName;
            event.dialog = ownerDialog;
            event.control = this;
            event.gui = ownerDialog->guiManager;
            ownerDialog->HandleEvent(&event);
            ownerDialog->AddTimer(0x101, 0xfa, (int)this);
        } else {
            iterator = (GameObjectIterator*)CreateControlIterator();
            for (control = NextAttachedControl(iterator); control; control = NextAttachedControl(iterator)) {
                if (control->controlType == 3) {
                    ((UIListBox*)control)->lastClickTime = 0;
                    ((UIListBox*)control)->ScrollBy(1);
                }
                if (control->controlType == 6) {
                    ((UIDropDownList*)control)->listPart->lastClickTime = 0;
                    ((UIDropDownList*)control)->listPart->ScrollBy(1);
                }
            }
            DeleteControlIterator(iterator);
            event.code = eventCode;
            event.kind = 0xe;
            event.controlName = controlName;
            event.dialog = ownerDialog;
            event.control = this;
            event.gui = ownerDialog->guiManager;
            ownerDialog->HandleEvent(&event);
            ownerDialog->AddTimer(0x101, 0xfa, (int)this);
        }
    }
    return UIControl::ButtonDown(a, b);
}

// 0x004749f0: the scroll timer (0x101 starts the repeat timer 0x102, which
// scrolls the list boxes by one row). Each case has its own copy of the event
// tail; VC6 cross-jumps case 9's copy into case 10's.
void UIScrollCtl::HandleTimer(int value) {
    UITimer* timer = (UITimer*)value;
    UnknownDialogEvent event;
    event.handled = 0;
    GameObjectIterator* iterator;
    UIControl* control;
    switch (timer->timerId) {
    case 0x101:
        ownerDialog->RemoveTimer(timer);
        ownerDialog->AddTimer(0x102, 100, (int)this);
        break;
    case 0x102:
        switch (controlType) {
        case 9:
            iterator = (GameObjectIterator*)CreateControlIterator();
            for (control = NextAttachedControl(iterator); control; control = NextAttachedControl(iterator)) {
                if (control->controlType == 3)
                    ((UIListBox*)control)->ScrollBy(-1);
            }
            DeleteControlIterator(iterator);
            event.kind = 0xe;
            event.code = eventCode;
            event.controlName = GetName();
            event.dialog = ownerDialog;
            event.gui = ownerGui;
            event.control = this;
            ownerDialog->HandleEvent(&event);
            break;
        case 10:
            iterator = (GameObjectIterator*)CreateControlIterator();
            for (control = NextAttachedControl(iterator); control; control = NextAttachedControl(iterator)) {
                if (control->controlType == 3)
                    ((UIListBox*)control)->ScrollBy(1);
            }
            DeleteControlIterator(iterator);
            event.kind = 0xe;
            event.code = eventCode;
            event.controlName = GetName();
            event.dialog = ownerDialog;
            event.gui = ownerGui;
            event.control = this;
            ownerDialog->HandleEvent(&event);
            break;
        }
        break;
    }
}

// 0x00474b10
int UIScrollCtl::ButtonUp(int a, int* position) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a && currentState == 2) {
        UnknownDialogEvent event;
        event.handled = a;
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->HandleEvent(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::ButtonUp(a, position);
}

// ---------------------------------------------------------------------------
// UIScrollBar

// 0x00474ba0
UIScrollBar::UIScrollBar(int type, int id, CameraRect* area, UIDialog* owner)
    : UIControl(type, id, area, owner) {
    field_0x1ec = 0;
    snapSteps = 0;
    thumbWidth = 0;
    thumbHeight = 0;
    followedRow = 0;
    dragging = 0;
    field_0x204 = 0;
    dragY = 0;
    dragX = 0;
    field_0x218 = 0;
    reversed = 0;
    scrollRange = 100;
    holdDelay = 0.2f;
}

// 0x00474c50
UIScrollBar::~UIScrollBar() {
    if (boundValue)
        SyncBoundValue(1);
}

// 0x00474cb0
void UIScrollBar::SyncBoundValue(int value) {
    if (value)
        *boundValue = GetScrollValue();
    else
        SetThumbPosition(*boundValue, scrollRange);
}

// 0x00475160
void UIScrollBar::SetThumbSizeFromState(int state) {
    UIDialog* owner = ownerDialog;
    thumbWidth = (int)(stateImages[state]->frameList[0]->frameWidth * owner->scaleX);
    thumbHeight = (int)(stateImages[state]->frameList[0]->frameHeight * owner->scaleY);
}

// 0x004751c0
void UIScrollBar::SetScrollRange(int range) {
    scrollRange = range;
    RequestRedraw();
}

// 0x004754d0
void UIScrollBar::SetSnapStepCount(int count) {
    if (count > 0)
        snapSteps = count - 1;
    else
        snapSteps = 0;
    RequestRedraw();
}

// 0x004755c0
int UIScrollBar::GetSnapSteps() {
    return snapSteps;
}

// UIListBox's first shown row and UIMultiState's current state are read
// through the same body: 0x00475200 and UIScrollBar slot 55 call 0x004755c0
// on list boxes, UIRadioButton 0x004793f0 and the dialog procedures on
// multi-states and radio buttons. All three read +0x1f0; the linker folded
// the identical bodies.
int UIListBox::GetFirstVisibleRow() {
    return firstVisibleRow;
}

int UIMultiState::GetSelectedState() {
    return selectedState;
}

// ---------------------------------------------------------------------------
// UIListBox

// 0x00475fa0
void UIListBox::AdvanceImages() {
    for (int i = 0; i < rowCount; i++) {
        UIAnim* image = rowTable[i].image;
        if (image)
            image->Advance();
    }
}

// 0x00475fe0
void UIListBox::SyncBoundValue(int value) {
    if (value) {
        *boundValue = GetSelectedRow();
    } else {
        lastClickTime = 0;
        SelectRow(*boundValue);
    }
}

// ---------------------------------------------------------------------------
// UIFrame and UIAnim

// 0x00472960: a frame from `file`: a .wav file becomes a sound of group `a`
// (played with `b`), anything else an image texture keyed on magenta. The
// image is freed after both the loaded and the missing path, which retail
// tests once after the whole if/else; the missing-image warning is formatted
// but never shown.
UIFrame::UIFrame(const char* file, void* textures, int a, int b, void* palette) {
    UnknownTgaFile* image = 0;
    if (_stricmp(".wav", file + strlen(file) - 4)) {
        int format = g_TrackGame->renderTarget->surfacePixelFormat;
        ownsSound = 0;
        isSound = 0;
        frameSound = 0;
        frameTextures = (int)textures;
        image = LoadFile(file, 0, (int)g_UnknownResourceManager572b44);
        if (image) {
            frameWidth = image->width;
            frameHeight = image->height;
            frameTexture = new(__FILE__, 0x1393) PCTextureMap((TextureMapManager*)frameTextures, 1);
            Palette8* pal = (Palette8*)palette;
            frameTexture->UnknownVirtualSlot4(image->bits, image->width, image->height, image->width,
                                            image->width, 0x22b, format,
                                            (UnknownTexturePalette*)(pal ? pal->field_0x708 : 0), 4,
                                            pal ? pal->field_0x70c : 0, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
            if (TextureHasColor(frameTexture, 0xff00ff))
                frameTexture->UnknownVirtualSlot18(0xff00ff);
        } else {
            char message[100];
            frameTexture = 0;
            sprintf(message, "UIFrame(): Warning! Error loading frame: %s\n", file);
        }
    } else {
        frameSound = new(__FILE__, 0x13b9) Sound((SoundGroup*)a, 1);
        frameSound->Load(file, 0, 1, 3, b, -1);
        ownsSound = 1;
        isSound = 1;
        frameWidth = 0;
        frameHeight = 0;
        frameTexture = 0;
    }
    if (image)
        FreeFile(image);
}

// 0x00472d00: `module` is the image's name and `id` the resource manager
// holding it; `a` is the sound group a missing image is loaded from as a sound.
UIFrame::UIFrame(void* module, int id, void* textures, int a, int b, int c, void* palette) {
    if (!b) {
        ownsSound = 0;
        isSound = 0;
        frameSound = 0;
        frameTextures = (int)textures;
        UnknownResourceEntry* entry =
            ((UnknownResourceManager*)id)->FindItem((const char*)module, 1);
        if (!LoadFromStream(entry->archive, entry->offset, palette) && a) {
            LoadEntrySound((SoundGroup*)a, (const char*)module, 1, 3, c, -1);
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

// 0x00472e30. The zeroed counters come first and the textures after them;
// the frame list is cleared by a loop (memset puts the `lea edi` elsewhere).
// This store order was found by an exhaustive sweep (tools/sweep_variants.py).
UIAnim::UIAnim(void* textures, void* palette) {
    currentFrame = 0;
    frameCount = 0;
    passesLeft = 0;
    passCountdown = 0;
    lastStepTime = 0;
    animTextures = textures;
    playForwards = 1;
    frameDelay = 0;
    playBackwards = 0;
    animPalette = palette;
    for (int i = 0; i < 50; i++)
        frameList[i] = 0;
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

// 0x00475c70: the font height is read as members in both the test and the
// divisor (a local for it makes VC6 reuse the test's load as the divisor).
UIListBox::UIListBox(int id, int rows, CameraRect* area, UIDialog* owner)
    : UIControl(3, id, area, owner) {
    selectBrush = 0;
    itemBrush = 0;
    rowCount = 0;
    selectedRow = 0;
    firstVisibleRow = 0;
    rowCapacity = rows;
    lastPageRowCount = 0;
    selectColor = 0xffffff;
    rowTable = (UnknownGameUiListRow*)DebugMalloc(rows * sizeof(UnknownGameUiListRow), __FILE__, 0x1b50);
    sortCompare = CompareRowText;
    field_0x218 = (int)this;
    autoSort = 0;
    sorting = 0;
    selectable = 1;
    allowWScroll = 1;
    dragOffsetX = 0;
    scrollOffsetX = 0;
    dragStartX = 0;
    lastClickTime = 0;
    partialRows = 0;
    rowCount = 0;
    firstVisibleRow = 0;
    lastPageRowCount = 0;
    memset(rowTable, 0, rowCapacity * sizeof(UnknownGameUiListRow));
    wrapScroll = 1;
    if (!(fontHeight || (ownerDialog && ownerDialog->dialogFontHeight))) {
        visibleRowCount = 1;
    } else {
        int rowsShown = (screenArea[3] - screenArea[1]) / (fontHeight ? fontHeight : ownerDialog->dialogFontHeight);
        visibleRowCount = rowsShown < 1 ? 1 : rowsShown;
    }
}

// 0x00475e20
UIListBox::~UIListBox() {
    if (boundValue)
        SyncBoundValue(1);
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
        SetSelectedRow(firstVisibleRow);
    RequestRedraw();
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
char* UIListBox::GetRowKind(int row) {
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
    RequestRedraw();
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
    sortCompare = compare;
}

// 0x00477b60
int CompareRowText(const void* a, const void* b) {
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

// 0x00477bc0: counts the rows of the last page (the rows that fit from the
// end) and updates the scroll bars attached to the list.
void UIListBox::UpdateScrollBars() {
    UIDialog* owner = ownerDialog;
    if (!owner)
        return;
    int height = 0;
    lastPageRowCount = 0;
    for (int i = rowCount - 1; i >= 0; i--) {
        if (height + rowTable[i].height > screenArea[3] - screenArea[1])
            break;
        height += rowTable[i].height;
        lastPageRowCount++;
    }
    lastPageRowCount = lastPageRowCount < 0 ? 0 : lastPageRowCount;
    GameObjectIterator iterator(owner->controlContainer, 1, "UIControl");
    UIControl* control;
    while ((control = (UIControl*)iterator.Next()) != 0) {
        if (control->attachId == attachId && (control->controlType == 8 || control->controlType == 7))
            static_cast<UIScrollBar*>(control)->FollowList(this);
    }
    RequestRedraw();
}

// 0x00477ce0
void UIListBox::RequestRedraw() {
    UIControl::RequestRedraw();
    if (ownerDialog && ownerDialog->dialogBackground) {
        for (int i = 0; i < rowCount; i++)
            rowTable[i].field_0x30 = 0;
    }
}

// 0x00477e60
void UIListBox::SetPartialRows(int value) {
    partialRows = value;
    if (!visibleRowCount)
        visibleRowCount = 1;
}

// 0x00478190
void UIListBox::ButtonHeld(int a, int* position) {
    if (a == 1 && position) {
        int delta = position[0] - dragStartX;
        dragOffsetX = delta;
        if (delta >= -scrollOffsetX)
            delta = -scrollOffsetX;
        dragOffsetX = delta;
        RequestRedraw();
    }
    UIControl::ButtonHeld(a, position);
}

// 0x00479220
void UIListBox::SetState(int state) {
    needsRedraw = 1;
    currentState = state;
}

// ---------------------------------------------------------------------------
// UIMultiState

// 0x004781f0
UIMultiState::UIMultiState(int id, CameraRect* area, UIDialog* owner)
    : UIControl(2, id, area, owner) {
    stateCount = 0;
    selectedState = 0;
    stateTable = 0;
    SetStateCount(2);
}

// 0x00478260
int UIMultiState::GetControlText() {
    return (int)stateTable[selectedState].text;
}

// 0x00478280
int UIMultiState::GetTextLength() {
    return stateTable[selectedState].textLength;
}

// 0x004782a0
int UIMultiState::GetTextLines() {
    return (int)stateTable[selectedState].textLines;
}

// 0x004782c0
void* UIMultiState::GetTextExtent() {
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
void UIMultiState::UpdateNeedsRedraw() {
    UIControl* last = FindLastAnchor(0);
    if (!shown || (last && (!last->shown || last->transitionState))) {
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
void UIMultiState::AdvanceImages() {
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
void UIMultiState::SetState(int state) {
    redrawFrames = 3;
    currentState = state;
}

// 0x00478540
void UIMultiState::SyncBoundValue(int value) {
    if (value)
        *boundValue = GetSelectedState();
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
    RequestRedraw();
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
    RequestRedraw();
}

// ---------------------------------------------------------------------------
// UIStaticText

// 0x00478ff0
UIStaticText::UIStaticText(int id, CameraRect* area, UIDialog* owner, const char* text,
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
void UIStaticText::UpdateNeedsRedraw() {
    int empty = 0;
    if (!controlText || !controlTextLength)
        empty = 1;
    UIControl* last = FindLastAnchor(0);
    if (!shown || empty || (last && (!last->shown || last->transitionState))) {
        if (--redrawFrames < 0) {
            redrawFrames = 0;
            needsRedraw = 1;
        }
    }
}

// 0x00479170
int UIStaticText::Draw() {
    UIControl::Draw();
    needsRedraw = 1;
    return 1;
}

// 0x00479190
int UIStaticText::ButtonDown(int a, int b) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.handled = a;
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->HandleEvent(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::ButtonDown(a, b);
}

// ---------------------------------------------------------------------------
// UIRadioButton

// 0x00479240
UIRadioButton::UIRadioButton(int id, CameraRect* area, UIDialog* owner)
    : UIMultiState(id, area, owner) {
    AppendClassName(this);
    controlType = 4;
}

// 0x004792d0
UIRadioButton::~UIRadioButton() {
}

// 0x004792e0
void UIRadioButton::SyncBoundValue(int value) {
    if (value)
        *boundValue = GetGroupSelection();
    else
        SelectInGroup(*boundValue);
}

// 0x004795c0
int UIRadioButton::CreateControlIterator() {
    return (int)new(__FILE__, 0x23b9) GameObjectIterator(ownerDialog->controlContainer, 1, "UIRadioButton");
}

// ---------------------------------------------------------------------------
// The drop-down list's parts

// 0x00479640
UIDDLScrollBar::UIDDLScrollBar(int type, int id, CameraRect* area, UIDialog* owner,
                               UIDropDownList* list)
    : UIScrollBar(type, id, area, owner) {
    ownerList = list;
    AppendClassName(this);
}

// 0x004796e0
void UIDDLScrollBar::KillFocus(int value) {
    UIControl::KillFocus(value);
    if (!ownerList->IsPart((UIControl*)value))
        ownerList->SetOpen(0);
}

// 0x00479b50
UIDDLStatic::UIDDLStatic(int id, CameraRect* area, UIDialog* owner, UIDropDownList* list)
    : UIStatic(id, area, owner), ownerList(list) {
    AppendClassName(this);
}

// 0x00479bc0
void UIDDLStatic::KillFocus(int value) {
    UIControl::KillFocus(value);
    if (!ownerList->IsPart((UIControl*)value))
        ownerList->SetOpen(0);
}

// 0x00479bf0
UIDDLButton::UIDDLButton(int id, CameraRect* area, UIDialog* owner, UIDropDownList* list)
    : UIButton(id, area, owner), ownerList(list) {
    AppendClassName(this);
}

// 0x00479c90
int UIDDLButton::ButtonDown(int a, int b) {
    UIControl::ButtonDown(a, b);
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this)
        ownerList->SetOpen(1 - ownerList->isOpen);
    return 0;
}

// 0x00479ce0
void UIDDLButton::KillFocus(int value) {
    UIControl::KillFocus(value);
    if (!ownerList->IsPart((UIControl*)value))
        ownerList->SetOpen(0);
}

// 0x00479d10
UIDDLListBox::UIDDLListBox(int id, int rows, CameraRect* area, UIDialog* owner,
                           UIDropDownList* list)
    : UIListBox(id, rows, area, owner) {
    ownerList = list;
    AppendClassName(this);
}

// 0x00479db0
int UIDDLListBox::SetSelectedRow(int row) {
    int result = UIListBox::SetSelectedRow(row);
    if (result)
        ownerList->SetText(GetRowText(row));
    return result;
}

// 0x00479e70
void UIDDLListBox::KillFocus(int value) {
    UIControl::KillFocus(value);
    if (!ownerList->IsPart((UIControl*)value))
        ownerList->SetOpen(0);
}

// ---------------------------------------------------------------------------
// UIDropDownList

// 0x00479ea0
UIDropDownList::UIDropDownList(int id, CameraRect* area, UIDialog* owner, const char* text,
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
    listPart->SetAttachId(0x4d43);
    scrollBarPart->SetAttachId(0x4d43);
    AppendChild(buttonPart, -1);
    AppendChild(staticPart2, -1);
    isOpen = 0;
    SetOpen(0);
    owner->SetupControl(buttonPart, 0, 0);
    owner->SetupControl(listPart, 0, 0);
    owner->SetupControl(scrollBarPart, 0, 0);
    owner->SetupControl(staticPart0, 0, 0);
    owner->SetupControl(staticPart2, 0, 0);
    owner->SetupControl(staticPart1, 0, 0);
}

// 0x0047a1f0
UIDropDownList::~UIDropDownList() {
    if (boundValue)
        SyncBoundValue(1);
    if (!isOpen) {
        staticPart0->Release();
        staticPart1->Release();
        listPart->Release();
        scrollBarPart->Release();
    }
}

// 0x0047a290
void UIDropDownList::SetAttachId(int id) {
    attachId = id;
    if (!id)
        id = 0x4d43;
    listPart->SetAttachId(id);
    scrollBarPart->SetAttachId(id);
}

// 0x0047a7d0
void UIDropDownList::KillFocus(int value) {
    UIControl::KillFocus(value);
    if (!IsPart((UIControl*)value))
        SetOpen(0);
}

// 0x0047a800
int UIDropDownList::IsPart(UIControl* control) {
    if (control == this || control == scrollBarPart || control == buttonPart ||
        control == listPart || control == staticPart0 || control == staticPart1 ||
        control == staticPart2)
        return 1;
    return 0;
}

// 0x0047a850
int UIDropDownList::GetWidth() {
    if (!buttonPart->bounds[0])
        return bounds[2] - bounds[0];
    return buttonPart->bounds[2] - bounds[0];
}

// 0x0047a870
int UIDropDownList::GetHeight() {
    return listPart->bounds[3] - bounds[1];
}

// 0x0047ad80
void UIDropDownList::SyncBoundValue(int value) {
    if (value) {
        *boundValue = listPart->GetSelectedRow();
    } else {
        listPart->SelectRow(*boundValue);
        listPart->ReportClick(0);
    }
}

// 0x0047add0
int UIDropDownList::ButtonDown(int a, int b) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this) {
        SetOpen(isOpen == 0);
        ownerDialog->guiUser->RequestFocus((UnknownGuiControl*)this, 0, 0);
        return 1;
    }
    return UIControl::ButtonDown(a, b);
}

// ---------------------------------------------------------------------------
// UIProgressBar's slot 48 and UICtlContainer's input slots

// 0x0047b3f0
int UICtlContainer::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    GameObject* child = firstChild;
    if (child) {
        while (child->nextSibling)
            child = child->nextSibling;
        for (; child; child = child->prevSibling) {
            if (child->UnknownVirtualSlot23(event, entry))
                return 1;
        }
    }
    return 0;
}

// 0x0047b440
int UICtlContainer::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    GameObject* child = firstChild;
    if (child) {
        while (child->nextSibling)
            child = child->nextSibling;
        for (; child; child = child->prevSibling) {
            if (child->UnknownVirtualSlot22(event, entry))
                return 1;
        }
    }
    return 0;
}

// 0x0047b570
void* ResizeDebugBlock(void* block, unsigned int size) {
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

// 0x00469db0: `resource` is the .dtm the dialog is built from.
UIDialog::UIDialog(int flags, const char* resource) : GameObject(flags) {
    AppendClassName(this);
    if (resource)
        strcpy(resourceName, resource);
    else
        strcpy(resourceName, "");
    parentDialog = 0;
    guiManager = 0;
    dialogResult = 0;
    dialogFont = 0;
    field_0x108 = 0;
    field_0x10c = 0;
    dialogFontFace[0] = 0;
    screenWidth = 640;
    screenHeight = 480;
    isShown = 1;
    openingMenu = 0;
    eventProc = 0;
    syncProc = 0;
    field_0xd4 = 0;
    cursorAnimation = 0;
    field_0x184 = 0;
    field_0x188 = 0;
    imageCount = 0;
    soundCount = 0;
    dialogTextures = 0;
    dialogBackground = 0;
    soundGroup = 0;
    field_0x7f14 = 0;
    field_0xc0 = 0;
    field_0x148 = 0;
    for (int i = 0; i < 10; i++)
        textColors[i] = 0xffffff;
    scaleY = 1.0f;
    scaleX = 1.0f;
    dialogPalette = 0;
    ownsPalette = 0;
    parentBackground = 0;
    ownsBackground = 0;
    scaleToScreen = 1;
    isPopup = 0;
    popupAlignment = 0x12;
    field_0x180 = 7;
    animatesControls = 1;
    memset(sectionTable, 0, sizeof(sectionTable));
    sectionCount = 0;
    field_0x154 = 1;
    lastFrameTime = 0.0f;
    controlsPost3D = 1;
    timerList.Init(2, 1);
    joystickCentred = 1;
    guiUser = 0;
    resourceArchive = 0;
    controlContainer = 0;
    field_0x14c = 0;
    screenGrab = 0;
    field_0x144 = 0;
    isClosing = 0;
    grabRegion = -1;
}

// 0x0046a070 (deleting wrapper 0x00469ff0): tells the GUI, releases the
// images, the palette and background it owns, and the timers.
UIDialog::~UIDialog() {
    if (guiManager) {
        if (field_0x148)
            guiManager->UnknownFunction485ee0(field_0x14c);
        if (g_TrackGame->renderTarget->surfacePixelFormat == 8) {
            if (!guiManager || !guiManager->UnknownFunction4864f0())
                guiManager->UnknownFunction464e90();
        }
        if (field_0x148 && parentDialog)
            parentDialog->UnknownFunction470070(0, 0, 0);
        if ((field_0x144 & 4) && (field_0x144 & 0x10))
            guiManager->ReleaseBackgroundGrab();
    }
    if (dialogBackground && !g_TrackGame->field_0x2d5_bit1)
        dialogBackground->MarkAllDirty();
    if (parentDialog) {
        if (parentDialog->dialogPalette && parentDialog->dialogPalette != dialogPalette &&
            parentDialog->guiManager && !parentDialog->guiManager->UnknownFunction4864f0())
            parentDialog->guiManager->UnknownFunction486150((Palette8*)parentDialog->dialogPalette);
        if (field_0x154)
            NotifyParent(dialogResult, 9);
    }
    if (dialogFont)
        DeleteObject((HGDIOBJ)dialogFont);
    if (imageCount) {
        for (int i = 0; i < imageCount; i++) {
            if (imageTable[i])
                imageTable[i]->Release();
        }
    }
    if (!g_TrackGame->field_0x2d5_bit1 && g_UnknownResourceManager572b44 && resourceArchive)
        g_UnknownResourceManager572b44->RemoveArchiveItems(resourceArchive);
    if ((ownsPalette && dialogPalette && !parentDialog) ||
        (dialogPalette && parentDialog && parentDialog->dialogPalette != dialogPalette))
        ((Palette8*)dialogPalette)->Release();
    if (!g_TrackGame->field_0x2d5_bit1 && ownsBackground && parentBackground) {
        dialogBackground->field_0x2c->Release();
        dialogBackground->SetImage(parentBackground);
    }
    if (!g_TrackGame->field_0x2d5_bit1 && parentDialog && parentDialog->dialogBackground)
        parentDialog->dialogBackground->MarkAllDirty();
    int i = 0;
    UITimer* timer;
    while ((timer = timerList.Get(i++)) != 0)
        timer->Release();
}

// 0x0046a010
int UIDialog::Release() {
    UnknownDialogEvent event;
    event.handled = 0;
    UpdateBoundValues(1);
    SyncValues(1);
    event.gui = guiManager;
    event.kind = kDialogClose;
    event.code = 0;
    event.controlName = 0;
    event.dialog = this;
    event.control = 0;
    HandleEvent(&event);
    return GameObject::Release();
}

// 0x0046a780
int UIDialog::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind && (unsigned int)event->control < 0x20)
        guiUser->controlDown[event->control] = 1;
    if (!isPaused) {
        if (event->kind == 1)
            UnknownFunction46f120();
        return GameObject::UnknownVirtualSlot23(event, entry);
    }
    return 0;
}

// 0x0046a7d0
int UIDialog::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    if (event->kind && (unsigned int)event->control < 0x20)
        guiUser->controlDown[event->control] = 0;
    if (!isPaused) {
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
UIControl* UIDialog::SetupControl(UIControl* control, int group, int region) {
    if (control) {
        control->UnknownInlineSetField18(renderContext);
        control->post3D = controlsPost3D;
        control->ownerGui = guiManager;
        control->groupId = group;
        control->fontHeight = dialogFontHeight;
        if (dialogBackground)
            control->backgroundRegion = dialogBackground->AllocRegion(region);
        return control;
    }
    return 0;
}

// 0x0046a8a0
GameObject* UIDialog::AddControl(GameObject* control, int a, int b) {
    if (control) {
        SetupControl((UIControl*)control, a, b);
        controlContainer->AppendChild(control, -1);
        return control;
    }
    return 0;
}

// 0x0046e8c0
void UIDialog::UnknownVirtualSlot28(int value) {
    UnknownDialogEvent event;
    event.handled = 0;
    event.kind = kDialogInit;
    event.code = 0;
    event.controlName = 0;
    event.control = 0;
    event.field_0x18 = value;
    event.gui = guiManager;
    event.dialog = this;
    HandleEvent(&event);
}

// 0x0046e9a0
Sound* UIDialog::FindSectionObject(const char* name) {
    if (!name)
        return 0;
    for (int i = 0; i < sectionCount; i++) {
        if (!_stricmp(name, sectionTable[i].sectionName))
            return sectionTable[i].sectionObject;
    }
    if (guiManager)
        return guiManager->FindSectionObject(name);
    return 0;
}

// 0x0046ea60
void UIDialog::SetShown(int value) {
    isShown = value;
    if (value)
        GameObject::UnknownVirtualSlot5();
    else
        GameObject::UnknownVirtualSlot4();
}

// 0x0046ea80
void UIDialog::EnableGroup(int id, int value) {
    GameObjectIterator iterator(controlContainer, 1, "UIControl");
    UIControl* control;
    while ((control = (UIControl*)iterator.Next()) != 0) {
        int group = control->groupId;
        if (group == id)
            control->SetEnabled(value);
    }
}

// 0x0046eb30
void UIDialog::ShowGroup(int id, int value) {
    GameObjectIterator iterator(controlContainer, 1, "UIControl");
    UIControl* control;
    while ((control = (UIControl*)iterator.Next()) != 0) {
        if (control->groupId == id)
            control->Show(value, 1);
    }
    if (!value && dialogBackground)
        dialogBackground->MarkAllDirty();
}

// 0x0046ebf0
UIControl* UIDialog::FindControl(const char* name, int flags) {
    UIControl* found = 0;
    GameObjectIterator iterator(controlContainer, 1, "UIControl");
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
    GameObjectIterator iterator(controlContainer, 1, "UIControl");
    UIControl* control;
    while ((control = (UIControl*)iterator.Next()) != 0) {
        if (control->boundValue)
            control->SyncBoundValue(value);
    }
    SyncValues(value);
}

// 0x0046f1c0
int UIDialog::UnknownVirtualSlot13() {
    if (!isPaused) {
        GameObjectIterator iterator(controlContainer, 1, "UIControl");
        if (dialogBackground) {
            UIControl* control;
            while ((control = (UIControl*)iterator.Next()) != 0) {
                if (control->backgroundRegion >= 0 && (!control->needsRedraw || control->redrawFrames))
                    dialogBackground->SetRegionRect(control->backgroundRegion,
                                                               (CameraRect*)&control->screenArea);
            }
        }
        GameObject::UnknownVirtualSlot13();
    } else if (screenGrab && !controlsPost3D) {
        if (dialogBackground)
            dialogBackground->DrawImage(screenGrab, &screenArea, 0, 0x1000000,
                                                       grabRegion, 1, &field_0x7f4c, 0);
        else
            ((RenderTarget*)UnknownInlineField18())->UnknownVirtualSlot3(&screenArea,
                                                                                 screenGrab, 0, 0x1000000);
    }
    return 1;
}

// 0x0046f300
int UIDialog::UnknownVirtualSlot14() {
    if (!isPaused)
        return GameObject::UnknownVirtualSlot14();
    return 1;
}

// 0x0046f320
int UIDialog::UnknownVirtualSlot15() {
    if (!isPaused) {
        GameObject::UnknownVirtualSlot15();
    } else if (screenGrab && controlsPost3D) {
        if (dialogBackground)
            dialogBackground->DrawImage(screenGrab, &screenArea, 0, 0x1000000,
                                                       grabRegion, 1, &field_0x7f4c, 0);
        else
            ((RenderTarget*)UnknownInlineField18())->UnknownVirtualSlot3(&screenArea,
                                                                                 screenGrab, 0, 0x1000000);
    }
    if (isClosing && closeFrame != ((RenderTarget*)UnknownInlineField18())->frameCount)
        UnknownVirtualSlot26();
    return 1;
}

// 0x0046fe40
void UIDialog::RemoveTimers(int id) {
    UITimer* timer;
    for (int i = 0; (timer = timerList.Get(i)) != 0; i++) {
        if (timer->timerId == id)
            timerList.Remove(timer);
    }
}

// 0x0046fec0
void UIDialog::RemoveTimer(void* timer) {
    UITimer* entry;
    for (int i = 0; (entry = timerList.Get(i)) != 0; i++) {
        if (entry == timer)
            timerList.Remove((UITimer*)timer);
    }
}

// 0x0046ff70: sends the parent dialog a `kind` event with `code`; every
// parent read goes through the member (a local copy hoists the vtable load).
void UIDialog::NotifyParent(int code, int kind) {
    if (parentDialog) {
        UnknownDialogEvent event;
        event.kind = kind;
        event.handled = 0;
        event.code = code;
        event.controlName = 0;
        event.dialog = parentDialog;
        event.control = (UIControl*)this;
        event.gui = parentDialog->guiManager;
        parentDialog->HandleEvent(&event);
    }
}

// 0x0046ff30
void UIDialog::EndDialog(int result) {
    if (!isClosing) {
        dialogResult = result;
        isClosing = 1;
        closeFrame = ((RenderTarget*)renderContext)->frameCount;
    }
}

// 0x0046ff60
void UIDialog::UnknownFunction46ff60() {
    field_0x154 = 0;
    UnknownVirtualSlot26();
}

// 0x0046ffc0
void UIDialog::UnknownFunction46ffc0(int value) {
    popupAlignment = value;
}

// 0x0046ffd0
void UIDialog::SetBackground(void* background) {
    if (parentDialog) {
        parentDialog->SetBackground(background);
        dialogBackground = (BackgroundImage*)background;
    } else {
        dialogBackground = (BackgroundImage*)background;
    }
}

// 0x00470000
int UIDialog::RequestHover(UIControl* control, int a, int b) {
    return guiUser->RequestHover((UnknownGuiControl*)control,
                                                                    (UnknownGuiControl**)a, b);
}

// 0x00470020
void UIDialog::HandleEvent(UnknownDialogEvent* event) {
    if (eventProc)
        eventProc(event);
}

// 0x00470040
void UIDialog::UnknownVirtualSlot30() {
    if (field_0xd4)
        field_0xd4();
}

// 0x00470050
void UIDialog::SyncValues(int value) {
    if (syncProc)
        syncProc(this, value);
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
    ReleaseBackgroundGrab();
    if (!rect)
        rect = &screenArea;
    screenGrab = guiManager->CopyScreenToTexture(dim, rect);
    if (dialogBackground) {
        grabRegion = dialogBackground->AllocRegion(1);
        dialogBackground->MarkAllDirty();
        dialogBackground->field_0x30 = 0;
    }
}

// 0x00470110
void UIDialog::ReleaseBackgroundGrab() {
    if (screenGrab)
        screenGrab->Release();
    if (dialogBackground && grabRegion >= 0) {
        dialogBackground->ReleaseRegion(grabRegion);
        dialogBackground->field_0x30 = 1;
    }
    screenGrab = 0;
    grabRegion = -1;
    field_0x7f4c = 0;
}

// ---------------------------------------------------------------------------
// UIControl: text and transitions

// 0x00470170: the owner is tested twice (post3D, then +0x18); VC6 threads
// the second test and places the ownerless store after the epilogue.
UIControl::UIControl(int type, int id, CameraRect* area, UIDialog* owner)
    : GameObject(1) {
    AppendClassName(this);
    controlType = type;
    eventCode = id;
    if (area) {
        *(CameraRect*)screenArea = *area;
        *(CameraRect*)bounds = *area;
    } else {
        bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0;
        *(CameraRect*)screenArea = *(CameraRect*)bounds;
    }
    savedState = 0;
    currentState = 0;
    transitionState = 0;
    enabled = 1;
    shown = 1;
    groupId = 0;
    attachId = 0;
    ownerDialog = owner;
    ownerGui = owner ? owner->guiManager : 0;
    anchorControl = 0;
    relAnchor = 0;
    shapeBounds = 0;
    sourceBlit = 0;
    boundValue = 0;
    backgroundRegion = -1;
    redrawFrames = 0;
    controlText = 0;
    toolTipText = 0;
    textColor = 0xffff;
    currentTextColor = 0xffff;
    textDrop = 1;
    dropColor = 0;
    textAlign = 1;
    controlTextLength = 0;
    textLines = 0;
    needsRedraw = 1;
    moveable = 0;
    keyBind = 0;
    field_0x1c8 = 0;
    tabStops = 0;
    mouseAnim = 0;
    for (int i = 0; i < 5; i++) {
        stateImages[i] = 0;
        sounds[i] = 0;
    }
    transitionFrame = 0;
    transitionDelay = 0;
    transitionTime = 0;
    slideSound = 0;
    fxAnimOut = 0;
    fxAnimIn = 0;
    fxSoundOut = 0;
    fxSoundIn = 0;
    controlName[0] = 0;
    ownsFont = 0;
    fontHandle = 0;
    fontFace[0] = 0;
    fontHeight = 0;
    bold = 0;
    italic = 0;
    textTopMargin = 0;
    textLeftMargin = 0;
    drawnTexture = 0;
    transitionStep = 0;
    transitionType = 0;
    heldButton = -1;
    if (owner)
        post3D = owner->controlsPost3D;
    else
        post3D = 1;
    if (owner)
        renderContext = owner->UnknownInlineField18();
    permanent = 0;
    wordWrap = 0;
}

// 0x00470450: two independent guiUser tests (VC6 threads the second one);
// nesting them inside one test keeps the focus control in a register.
UIControl::~UIControl() {
    if (ownerDialog && !g_TrackGame->field_0x2d5_bit1) {
        pendingRelease = 1;
        ToolTip* tip = 0;
        if (ownerDialog->guiUser && ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
            ownerDialog->guiUser->UnknownFunction487bf0(0);
            tip = ownerDialog->guiUser->userToolTip;
        }
        if (ownerDialog->guiUser && ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this) {
            ownerDialog->RequestHover(0, 0, 1);
            tip = ownerDialog->guiUser->userToolTip;
        }
        if (tip)
            tip->ShowText(0, 0, 1.0f);
    }
    if (backgroundRegion != -1 && ownerDialog && ownerDialog->dialogBackground)
        ownerDialog->dialogBackground->ReleaseRegion(backgroundRegion);
    if (controlText)
        DebugFree(controlText, __FILE__, 0xba3);
    if (toolTipText)
        DebugFree(toolTipText, __FILE__, 0xba4);
    if (ownsFont)
        DeleteObject((HGDIOBJ)fontHandle);
    if (textLines)
        DebugFree(textLines, __FILE__, 0xba8);
    if (tabStops)
        DebugFree(tabStops, __FILE__, 0xba9);
}

// 0x00470660
void UIControl::Show(int a, int b) {
    if (transitionStep) {
        if (a) {
            transitionState = 1;
            if (transitionType == 0x6a)
                transitionStep = &UIControl::AnimInStep;
            drawnTexture = 0;
            shown = a;
            return;
        }
        if (transitionType == 0x6a && b && ownerDialog->animatesControls) {
            drawnTexture = 0;
            transitionState = 2;
            transitionStep = &UIControl::AnimOutStep;
            return;
        }
        transitionState = 2;
        shown = 0;
        RequestRedraw();
        drawnTexture = 0;
        return;
    }
    transitionState = 0;
    shown = a;
    RequestRedraw();
}

// 0x00470760
void UIControl::SetImageFile(int a, const char* image) {
    if (ownerDialog) {
        UIAnim* anim = new(__FILE__, 0xc10) UIAnim(ownerDialog->dialogTextures, 0);
        anim->LoadFile(image, ownerDialog->dialogPalette);
        SetImage(a, anim);
    }
    RequestRedraw();
}

// 0x00470870
void UIControl::StartTransition(int type, int a, int b, int delay, int c) {
    if (b) {
        Show(1, 1);
        transitionState = 1;
    }
    transitionType = type;
    switch (type) {
    case 100:
        transitionStep = &UIControl::GrowStep;
        break;
    case 101:
        transitionStep = &UIControl::SlideLeftStep;
        break;
    case 102:
        transitionStep = &UIControl::SlideRightStep;
        break;
    case 103:
        transitionStep = &UIControl::SlideTopStep;
        break;
    case 104:
        transitionStep = &UIControl::SlideBottomStep;
        break;
    case 106:
        transitionStep = &UIControl::AnimInStep;
        break;
    default:
        transitionStep = 0;
        break;
    }
    transitionTime = 0;
    transitionFrameCount = c;
    if (delay == 0xffff)
        transitionDelay = rand() % 2000;
    else
        transitionDelay = delay;
    slideSound = (Sound*)a;
    RequestRedraw();
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
    RequestRedraw();
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
                                                    tabStops, textExtent);
            if (!textLines) {
                controlTextLength = textExtent[0] = strlen(controlText);
                HDC dc;
                if (!g_TrackGame->PCTarget()->renderSurface->GetDC((void**)&dc)) {
                    HGDIOBJ font =
                        (HGDIOBJ)(fontHandle ? fontHandle : (ownerDialog ? (int)ownerDialog->dialogFont : 0));
                    HGDIOBJ old = SelectObject(dc, font);
                    SIZE size;
                    GetTextExtentPoint32A(dc, controlText, controlTextLength, &size);
                    SelectObject(dc, old);
                    g_TrackGame->PCTarget()->renderSurface->ReleaseDC(dc);
                    textExtent[1] = size.cx;
                } else {
                    textExtent[1] = 0;
                }
            }
        }
        RequestRedraw();
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
    AdvanceImages();
    int draw = 1;
    if (shown) {
        UIControl* last = FindLastAnchor(0);
        if (!last || (last->shown && !last->transitionState))
            draw = !RunTransition();
    }
    if (draw) {
        drawnTexture = GetStateTexture(-1);
        UpdateNeedsRedraw();
        ScaleArea();
        if (ownerDialog->guiUser->pointerDevice && heldButton != -1) {
            POINT position = ownerDialog->guiUser->pointerDevice->pointerPosition;
            ButtonHeld(heldButton, (int*)&position);
        }
    }
    screenArea[0] = screenArea[0] < 0 ? 0 : screenArea[0];
    screenArea[1] = screenArea[1] < 0 ? 0 : screenArea[1];
    int right = screenArea[2];
    screenArea[2] = g_TrackGame->renderTarget->targetWidth < right ? g_TrackGame->renderTarget->targetWidth : right;
    int bottom = screenArea[3];
    screenArea[3] = g_TrackGame->renderTarget->targetHeight < bottom ? g_TrackGame->renderTarget->targetHeight : bottom;
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x004711a0
int UIControl::UnknownVirtualSlot15() {
    int result = GameObject::UnknownVirtualSlot15();
    UIControl* last = FindLastAnchor(0);
    if ((!last || (last->shown && !last->transitionState)) && post3D) {
        if (shown)
            result = Draw();
        else
            needsRedraw = 1;
    }
    BackgroundImage* background = ownerDialog->dialogBackground;
    if (background && backgroundRegion > -1 &&
        (!shown || (last && (!last->shown || last->transitionState))))
        background->MarkRegionPending(backgroundRegion);
    return result;
}

// 0x00471500
int UIControl::RunTransition() {
    int done = 1;
    UIDialog* owner = ownerDialog;
    int (UIControl::*step)();
    if (owner->animatesControls && (step = transitionStep) != 0 && transitionState && controlType != 3 && controlType != 0xc) {
        transitionTime += (int)(owner->lastFrameTime * 1000.0f);
        done = 0;
        if (transitionTime < transitionDelay)
            goto end;
        done = (this->*step)();
        if (!done)
            goto end;
        transitionFrame = 0;
        transitionState = 0;
    } else if (transitionState) {
        transitionState = 0;
        done = 1;
        RequestRedraw();
    }
    *(CameraRect*)screenArea = *(CameraRect*)bounds;
end:
    return !done;
}

// ---------------------------------------------------------------------------
// UIControl: input

// 0x004721b0
int UIControl::ButtonDown(int a, int b) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this) {
        if (!a) {
            SetState(2);
            PlayControlSound(2, ownerDialog->field_0x180);
        }
        heldButton = a;
        if (a == 1 && ownerDialog->dialogBackground)
            ownerDialog->dialogBackground->MarkAllDirty();
        ownerDialog->guiUser->RequestFocus((UnknownGuiControl*)this, 0, 0);
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
        ownerDialog->HandleEvent(&event);
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
        ownerDialog->HandleEvent(&event);
        if (event.handled)
            return 1;
        if (!event.field_0x18)
            return 1;
    }
    return GameObject::UnknownVirtualSlot21(value);
}

// 0x00472480
int UIControl::UpdateHoverAt(int* position) {
    int result = 0;
    GameObject* next = nextSibling;
    while (next) {
        if (strstr(next->classNames, "UIControl,"))
            break;
        next = next->nextSibling;
    }
    if (next) {
        result = ((UIControl*)next)->UpdateHoverAt(position);
        if (result)
            return result;
    }
    if (firstChild && strstr(firstChild->classNames, "UIControl,")) {
        result = ((UIControl*)firstChild)->UpdateHoverAt(position);
        if (result)
            return result;
    }
    if (HitTest(*(POINT*)position) && IsInteractive()) {
        UIControl* last = FindLastAnchor(0);
        if (enabled && shown && (!last || (last->shown && !last->transitionState))) {
            GUIUser* user = ownerDialog->guiUser;
            UnknownGuiControl* hover = user->hoverControl;
            if (!user->controlDown[0] && !user->controlDown[1] && hover != (UnknownGuiControl*)this) {
                result = ownerDialog->RequestHover(this, 0, 0);
            } else if (hover == (UnknownGuiControl*)this) {
                if (user->pendingHoverControl && user->pendingHoverControl != (UnknownGuiControl*)this)
                    user->RequestHover((UnknownGuiControl*)this, 0, 0);
                result = 1;
            }
        }
    }
    return result;
}

// 0x004727c0
UIControl* UIControl::PreviousFocusableControl() {
    UIControl* control = this;
    while (control) {
        if (control != this && control->enabled && control->shown && control->controlType != 5 &&
            control->controlType != 0xc)
            return control;
        if (control->prevSibling) {
            control = (UIControl*)control->prevSibling;
        } else {
            while (control->nextSibling)
                control = (UIControl*)control->nextSibling;
        }
        if (control == this)
            return 0;
    }
    return control;
}

// 0x00472810
UIControl* UIControl::NextFocusableControl() {
    UIControl* control = this;
    while (control) {
        if (control != this && control->enabled && control->shown && control->controlType != 5 &&
            control->controlType != 0xc)
            return control;
        control = (UIControl*)control->nextSibling;
        if (!control)
            control = (UIControl*)ownerDialog->controlContainer->firstChild;
        if (control == this)
            return 0;
    }
    return control;
}

// ---------------------------------------------------------------------------
// UIListBox: rows

// 0x004769e0
int UIListBox::SetSelectedRow(int row) {
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
        RequestRedraw();
        return 1;
    }
    return 1;
}

// 0x00476a60
int UIListBox::SelectRow(int row) {
    int result = SetSelectedRow(row);
    UpdateScrollBars();
    if (ownerDialog) {
        GameObjectIterator* iterator = (GameObjectIterator*)CreateControlIterator();
        UIListBox* control;
        while ((control = static_cast<UIListBox*>(NextAttachedControlOfType(iterator))) != 0)
            control->SetSelectedRow(row);
        DeleteControlIterator(iterator);
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
    RequestRedraw();
}

// 0x00476c70
void UIListBox::SetRowTextColor(unsigned int color, int row) {
    if (row == -1) {
        SetFontColor(color);
        RequestRedraw();
        return;
    }
    if (row < rowCount)
        rowTable[row].textColor = color;
    RequestRedraw();
}

// 0x00476cd0
void UIListBox::SetSelectBoxColor(unsigned int color) {
    if (selectBrush)
        DeleteObject((HGDIOBJ)selectBrush);
    selectBrush = color != 0xff000000 ? CreateSolidBrush(color) : 0;
    RequestRedraw();
}

// 0x00476ee0
int UIListBox::CountVisibleRows() {
    int height = 0;
    int rows = 0;
    int i = firstVisibleRow;
    int count = rowCount;
    if (i < count) {
        int space = screenArea[3] - screenArea[1];
        for (; i < count; i++) {
            height += rowTable[i].height;
            if (height > space) {
                if (!rows && partialRows)
                    rows = 1;
                return rows;
            }
            rows++;
        }
    }
    return rows;
}

// 0x00476f50
int UIListBox::ReserveRows(int rows) {
    if (rows > rowCapacity) {
        UnknownGameUiListRow* resized =
            (UnknownGameUiListRow*)ResizeDebugBlock(rowTable, rows * sizeof(UnknownGameUiListRow));
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
            rowTable[row].text, textColor, (void*)fontHandle, tabStops, 0);
    }
    RequestRedraw();
}

// 0x00476d80
int UIListBox::AddRow(const char* text, int data, int a) {
    if (text && (rowCount < rowCapacity || ReserveRows(rowCount + 1))) {
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
                                                     tabStops, 0);
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

// 0x00477110: adds a row for an image file: with `a` the file is loaded as a
// UIAnim (only when the stream can open it), otherwise the row keeps the TGA
// file's name and the size from its header.
int UIListBox::AddImageFileRow(const char* file, int data, int a, int b) {
    if (rowCount >= rowCapacity && !ReserveRows(rowCount + 1))
        return 0;
    UnknownGameUiListRow row;
    if (a) {
        UnknownTextureStream* stream =
            new(__FILE__, 0x1e20) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        if (stream->Open(file, "r", 0)) {
            row.kind = 2;
            row.text = 0;
            row.imageFile = 0;
            row.textLines = 0;
            UIAnim* image = new(__FILE__, 0x1e27) UIAnim(ownerDialog->dialogTextures, 0);
            row.image = image;
            image->LoadFile(file, 0);
            UIFrame* frame = image->frameList[0];
            row.width = frame->frameWidth;
            row.height = frame->frameHeight;
            row.flags = b;
            row.field_0x30 = 0;
            delete stream;
        } else {
            delete stream;
            return 0;
        }
    } else {
        row.kind = 3;
        row.text = 0;
        row.image = 0;
        char* name = (char*)DebugMalloc(strlen(file) + 1, __FILE__, 0x1e37);
        row.flags = b;
        row.imageFile = name;
        strcpy(name, file);
        row.textLines = 0;
        row.field_0x30 = 0;
        UnknownTgaFile* header = OpenAndReadHeader(file, 0, (int)g_UnknownResourceManager572b44);
        if (header) {
            row.width = header->width;
            row.height = header->height;
            FreeFile(header);
        } else {
            return 0;
        }
    }
    rowTable[rowCount] = row;
    SetRowData(rowCount, data);
    rowCount++;
    visibleRowCount = CountVisibleRows();
    UpdateScrollBars();
    if (autoSort)
        Sort(1);
    return 1;
}

// 0x004773a0
int UIListBox::AddImageRow(UIAnim* image, int data, int a) {
    if (rowCount >= rowCapacity && !ReserveRows(rowCount + 1))
        return 0;
    UnknownGameUiListRow row;
    row.kind = 2;
    row.text = 0;
    row.image = image;
    image->AddRef();
    UIFrame* frame = image->frameList[0];
    row.width = frame->frameWidth;
    row.height = frame->frameHeight;
    row.flags = a;
    row.imageFile = 0;
    row.textLines = 0;
    row.field_0x30 = 0;
    rowTable[rowCount] = row;
    SetRowData(rowCount, data);
    rowCount++;
    visibleRowCount = CountVisibleRows();
    UpdateScrollBars();
    if (autoSort)
        Sort(1);
    return 1;
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
        stateTable = (UnknownGameUiState*)ResizeDebugBlock(stateTable,
                                                                         count * sizeof(UnknownGameUiState));
    } else if (stateCount < count) {
        stateTable = (UnknownGameUiState*)ResizeDebugBlock(stateTable,
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
    RequestRedraw();
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
    RequestRedraw();
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
                stateTable[index].text, textColor, (void*)fontHandle, tabStops,
                stateTable[index].textExtent);
            if (!stateTable[index].textLines) {
                stateTable[index].textExtent[0] = strlen(stateTable[index].text);
                stateTable[index].textLength = stateTable[index].textExtent[0];
                HDC dc;
                if (!g_TrackGame->PCTarget()->renderSurface->GetDC((void**)&dc)) {
                    HGDIOBJ font =
                        (HGDIOBJ)(fontHandle ? fontHandle : (ownerDialog ? (int)ownerDialog->dialogFont : 0));
                    HGDIOBJ old = SelectObject(dc, font);
                    SIZE size;
                    GetTextExtentPoint32A(dc, stateTable[index].text,
                                          stateTable[index].textLength, &size);
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
    RequestRedraw();
}

// 0x00478d70
int UIMultiState::ButtonDown(int a, int b) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.handled = a;
        SelectNextState();
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->HandleEvent(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::ButtonDown(a, b);
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
                control->SetState(0);
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
            if (control->GetSelectedState() == 1)
                return index;
        }
    }
    return 0;
}

// 0x004794c0
int UIRadioButton::ButtonDown(int a, int b) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.handled = a;
        if (!GetSelectedState()) {
            SetCurrentState(1);
            GameObjectIterator* iterator = (GameObjectIterator*)CreateControlIterator();
            UIRadioButton* control;
            while ((control = static_cast<UIRadioButton*>(NextAttachedControlOfType(iterator))) != 0) {
                if (control != this && control->GetSelectedState() == 1) {
                    control->SetState(0);
                    control->SetCurrentState(0);
                }
            }
            DeleteControlIterator(iterator);
        }
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.control = this;
        event.gui = ownerDialog->guiManager;
        ownerDialog->HandleEvent(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::ButtonDown(a, b);
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
            SetSelectedRow(selectedRow - 1 <= 0 ? 0 : selectedRow - 1);
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

// 0x00477800: the order 0x00477900 sorts by, then the linked lists' orders.
// The first comparator call goes through a `control` local: without it VC6
// swaps eax and edx for that temporary and the next global load.
int CompareSortedRows(const void* a, const void* b) {
    int result = 0;
    if (g_SortingDialog->sortingList) {
        UIListBox* control = g_SortingDialog->sortingList->selfList;
        result = control->sortCompare(a, b);
        if (!result) {
            int left = ((const UnknownGameUiListRow*)a)->sortIndex;
            int right = ((const UnknownGameUiListRow*)b)->sortIndex;
            GameObjectIterator* iterator =
                (GameObjectIterator*)g_SortingDialog->sortingList->selfList->CreateControlIterator();
            while (1) {
                UIListBox* other = static_cast<UIListBox*>(
                    g_SortingDialog->sortingList->selfList->NextAttachedControlOfType(iterator));
                if (!other)
                    break;
                if (other->rowCount != g_SortingDialog->sortingList->selfList->rowCount) {
                    result = -1;
                    break;
                }
                result = other->sortCompare(&other->rowTable[left], &other->rowTable[right]);
                if (result)
                    break;
            }
            g_SortingDialog->sortingList->selfList->DeleteControlIterator(iterator);
        }
    }
    return result;
}

// 0x00477900
void UIListBox::Sort(int a) {
    if (a || !ownerDialog->sortingList || !g_SortingDialog) {
        ownerDialog->sortingList = this;
        g_SortingDialog = ownerDialog;
    }
    if (!rowCount)
        return;
    sorting = 1;
    for (int i = 0; i < rowCount; i++)
        rowTable[i].sortIndex = i;
    int selected = selectedRow;
    qsort(rowTable, rowCount, sizeof(UnknownGameUiListRow), CompareSortedRows);
    RequestRedraw();
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
                control->RequestRedraw();
            }
        }
    }
    for (int k = 0; k < rowCount; k++) {
        if (rowTable[k].sortIndex == selected) {
            SelectRow(k);
            break;
        }
    }
    sorting = 0;
}

// ---------------------------------------------------------------------------
// UIScrollBar: position

// 0x004751e0
int UIScrollBar::GetListScrollRange(UIListBox* list) {
    return list->rowCount - list->lastPageRowCount;
}

// 0x00475200
int UIScrollBar::FollowList(UIListBox* list) {
    RequestRedraw();
    if (reversed) {
        int count = list->rowCount;
        followedRow = count - list->GetFirstVisibleRow();
    } else {
        followedRow = list->GetFirstVisibleRow();
    }
    if (GetListScrollRange(list) == followedRow)
        return 1;
    unsigned int travel;
    if (controlType == 8)
        travel = GetWidth() - thumbWidth;
    else
        travel = GetHeight() - thumbHeight;
    unsigned int range = list->rowCount - list->lastPageRowCount;
    if (range) {
        float position = followedRow * (float)travel / range;
        thumbOffset = position < travel ? position : travel;
        return 1;
    }
    return 0;
}

// 0x00475300
int UIScrollBar::ScaleThumbPosition(int range) {
    int width = GetWidth();
    int height = GetHeight();
    unsigned int position;
    int travel;
    if (controlType == 8) {
        position = (int)(reversed ? width - thumbOffset : thumbOffset);
    } else {
        position = (int)(reversed ? height - thumbOffset : thumbOffset);
    }
    travel = controlType == 8 ? width - thumbWidth : height - thumbHeight;
    return (int)(travel ? (double)(position * range) / travel : 0.0);
}

// 0x00475500
int UIScrollBar::GetScrollValue() {
    int width = GetWidth();
    int height = GetHeight();
    int position;
    if (!reversed) {
        position = (int)thumbOffset;
        if (position < thumbOffset)
            position++;
    } else {
        if (controlType == 8)
            position = (int)(width - thumbOffset);
        else
            position = (int)(height - thumbOffset);
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
    textScrollX = 0;
    textLength = 0;
    textWidth = 0;
    controlTextLength = 0;
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
    RequestRedraw();
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
    controlTextLength = length;
    HDC dc;
    if (!g_TrackGame->PCTarget()->renderSurface->GetDC((void**)&dc)) {
        HGDIOBJ font = (HGDIOBJ)(fontHandle ? fontHandle : (ownerDialog ? (int)ownerDialog->dialogFont : 0));
        HGDIOBJ old = SelectObject(dc, font);
        SIZE size;
        GetTextExtentPoint32A(dc, controlText, controlTextLength, &size);
        SelectObject(dc, old);
        g_TrackGame->PCTarget()->renderSurface->ReleaseDC(dc);
        textExtent[1] = size.cx;
        textWidth = size.cx;
    } else {
        textExtent[1] = 0;
        textWidth = 0;
    }
    RequestRedraw();
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
    UnknownResourceEntry* entry = g_UnknownResourceManager572b44->FindItem(name, 1);
    if (entry)
        result = ReadDialogResource(entry->archive, entry->offset);
    return result;
}

// 0x0046eeb0
void UIDialog::EndControlDraw(void* dc) {
    if (previousFont)
        SelectObject((HDC)dc, (HGDIOBJ)previousFont);
    if (!dialogBackground && dc)
        ((PCRenderTarget*)UnknownInlineField18())->renderSurface->ReleaseDC(dc);
    if (drawClipRegion)
        DeleteObject((HGDIOBJ)drawClipRegion);
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
    if (guiUser->pointerDevice && controlContainer->firstChild) {
        GUIUser* user = guiUser;
        UnknownGuiControl* focus = user->hoverControl;
        if (focus && ((UIControl*)focus)->ownerDialog != (UIDialog*)this)
            focus = 0;
        POINT position = user->pointerDevice->pointerPosition;
        if (controlContainer->firstChild) {
            user->field_0x1dc = 0;
            int result = ((UIControl*)controlContainer->firstChild)->UpdateHoverAt((int*)&position);
            if (focus && !guiUser->pendingHoverControl && !result)
                guiUser->RequestHover(0, 0, 1);
        }
    }
}

// A colored run of dialog text (0x18 bytes).
struct UnknownGameUiTextRun {
    const char* text;                         // +0x00
    int kind;                                 // +0x04: 1 sets the color, 2 the position
    int length;                               // +0x08
    unsigned int color;                       // +0x0c: "~N" picks textColors[N]
    int width;                                // +0x10: GetTextExtentPoint32A cx
    int position;                             // +0x14: "|x" picks the tab-stop table entry x
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
    while (runs[g_UnknownGlobal65b604].text) {
        if (runs[g_UnknownGlobal65b604].kind == 2)
            g_UnknownGlobal65b564 = runs[g_UnknownGlobal65b604].position + x;
        if (shadow) {
            SetTextColor((HDC)dc, shadowColor);
            TextOutA((HDC)dc, g_UnknownGlobal65b564 + 1, y + 1, runs[g_UnknownGlobal65b604].text,
                     runs[g_UnknownGlobal65b604].length);
            SetTextColor((HDC)dc, g_UnknownGlobal65b574);
        }
        if (runs[g_UnknownGlobal65b604].kind == 1) {
            g_UnknownGlobal65b574 = runs[g_UnknownGlobal65b604].color;
            SetTextColor((HDC)dc, g_UnknownGlobal65b574);
        }
        TextOutA((HDC)dc, g_UnknownGlobal65b564, y, runs[g_UnknownGlobal65b604].text,
                 runs[g_UnknownGlobal65b604].length);
        g_UnknownGlobal65b564 += runs[g_UnknownGlobal65b604].width;
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
            g_UnknownGlobal65b5e0.cx = ((int*)control->GetTextExtent())[1];
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
        height = dialogFontHeight;
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
    CameraRect rect = *(CameraRect*)control->screenArea;
    rect.left += control->textLeftMargin;
    rect.top += control->textTopMargin;
    rect.right -= control->textLeftMargin;
    rect.bottom -= control->textTopMargin;
    if (transparent)
        SetBkMode((HDC)dc, TRANSPARENT);
    else
        SetBkMode((HDC)dc, OPAQUE);
    if (control->textAlign != 9) {
        g_UnknownGlobal65b5a0.cx = ((int*)control->GetTextExtent())[1];
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
            (rect.bottom - rect.top) / 2 - dialogFontHeight / 2 + rect.top;
    else if (control->textAlign & 0x20)
        g_UnknownGlobal65b5c0 = rect.bottom - dialogFontHeight;
    else
        g_UnknownGlobal65b5c0 = rect.top;
    if (control->GetTextLines())
        DrawTextRuns(dc, g_UnknownGlobal65b5e8, g_UnknownGlobal65b5c0,
                     (const char*)control->GetControlText(), control->GetTextLength(),
                     (UnknownGameUiTextRun*)control->GetTextLines(), control->textColor,
                     control->textColor, control->dropColor);
    else
        DrawShadowText(dc, g_UnknownGlobal65b5e8, g_UnknownGlobal65b5c0,
                       (const char*)control->GetControlText(), control->GetTextLength(),
                       control->textColor, control->textDrop, control->dropColor);
}

CameraRect* g_UnknownGlobal65b588;            // the clip rectangle
CameraRect g_UnknownGlobal65b5f0;
int g_UnknownGlobal65b5ec;

// 0x0046ed70
int UIDialog::BeginControlDraw(void** dc, CameraRect* rect, int* a, UIControl* control) {
    g_UnknownGlobal65b588 = rect;
    *a = 0;
    previousFont = 0;
    if (dialogBackground) {
        g_UnknownGlobal65b588 = &g_UnknownGlobal65b5f0;
        g_UnknownGlobal65b5f0 = *rect;
        if (control) {
            g_UnknownGlobal65b5ec = dialogBackground->GetDc(
                dc, rect, control->backgroundRegion, control->needsRedraw, &control->redrawFrames, a,
                &g_UnknownGlobal65b5f0);
        } else {
            int frames = 1;
            g_UnknownGlobal65b5ec =
                dialogBackground->GetDc(dc, rect, -1, 0, &frames, a, &g_UnknownGlobal65b5f0);
        }
    } else {
        ((PCRenderTarget*)UnknownInlineField18())->renderSurface->GetDC(dc);
        g_UnknownGlobal65b5ec = 1;
    }
    if (*dc && control) {
        previousFont =
            SelectObject((HDC)*dc, (HGDIOBJ)(control->fontHandle ? control->fontHandle : (int)dialogFont));
        drawClipRegion = CreateRectRgn(g_UnknownGlobal65b588->left, g_UnknownGlobal65b588->top,
                                             g_UnknownGlobal65b588->right, g_UnknownGlobal65b588->bottom);
        SelectClipRgn((HDC)*dc, (HRGN)drawClipRegion);
    } else {
        drawClipRegion = 0;
    }
    return g_UnknownGlobal65b5ec;
}

// 0x0046fce0
void UIDialog::AddTimer(int a, int time, int b) {
    if (time)
        timerList.Add(
            new(__FILE__, 0xa76) UITimer(a, time, (UIControl*)b));
}

// ---------------------------------------------------------------------------
// UIControl: transitions

// 0x00471df0
int UIControl::AnimInStep() {
    if (transitionFrame == 0) {
        fxAnimIn->playBackwards = 0;
        fxAnimIn->Rewind();
        transitionFrame = 1;
        if (fxSoundIn) {
            fxSoundIn->SetVolume(ownerGui->field_0x34c, 0);
            fxSoundIn->PlayWithOptions(1, 0, 0);
        }
    }
    if (transitionFrame == 1 && fxAnimIn->currentFrame <= fxAnimIn->frameCount - 1) {
        drawnTexture = fxAnimIn->GetCurrentTexture();
        RequestRedraw();
        if (fxAnimIn->currentFrame == fxAnimIn->frameCount - 1)
            transitionFrame = 2;
        return 0;
    }
    transitionFrame = 0;
    return 1;
}

// 0x00471eb0
int UIControl::AnimOutStep() {
    if (transitionFrame == 0) {
        if (fxAnimOut == fxAnimIn) {
            fxAnimOut->playBackwards = 1;
            transitionFrame = 2;
        } else {
            transitionFrame = 1;
        }
        fxAnimOut->Rewind();
        if (fxSoundOut) {
            fxSoundOut->SetVolume(ownerGui->field_0x34c, 0);
            fxSoundOut->PlayWithOptions(1, 0, 0);
        }
    }
    if ((transitionFrame == 1 && fxAnimOut->currentFrame < fxAnimOut->frameCount - 1) ||
        (transitionFrame == 2 && fxAnimOut->currentFrame >= 0)) {
        drawnTexture = fxAnimOut->GetCurrentTexture();
        RequestRedraw();
        if (fxAnimOut->currentFrame == 0)
            transitionFrame = 3;
        return 0;
    }
    transitionFrame = 0;
    shown = 0;
    return 1;
}

int g_UnknownGlobal65b610;                       // frames of the slide

// 0x00471850
int UIControl::SlideLeftStep() {
    if (transitionFrame == 0 && transitionFrameCount) {
        g_UnknownGlobal65b610 = transitionFrameCount < 0 ? 25 : transitionFrameCount;
        int left = bounds[0];
        slideX = (float)(left - bounds[2] - 1);
        slideStepX = ((float)left - slideX) / g_UnknownGlobal65b610;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        slideX += slideStepX;
    }
    if (transitionFrame == g_UnknownGlobal65b610)
        return 1;
    RequestRedraw();
    *(CameraRect*)screenArea = *(CameraRect*)bounds;
    screenArea[0] = (int)slideX;
    screenArea[2] = bounds[2] - bounds[0] + screenArea[0];
    if (screenArea[2] > 0) {
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
    transitionFrame++;
    return 0;
}

int g_UnknownGlobal65b614;                       // frames of the slide

// 0x004719c0
int UIControl::SlideTopStep() {
    if (transitionFrame == 0 && transitionFrameCount) {
        g_UnknownGlobal65b614 = transitionFrameCount < 0 ? 25 : transitionFrameCount;
        int top = bounds[1];
        slideY = (float)(top - bounds[3]);
        slideStepY = ((float)top - slideY) / g_UnknownGlobal65b614;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        slideY += slideStepY;
    }
    if (transitionFrame == g_UnknownGlobal65b614)
        return 1;
    RequestRedraw();
    *(CameraRect*)screenArea = *(CameraRect*)bounds;
    screenArea[1] = (int)slideY;
    screenArea[3] = bounds[3] - bounds[1] + screenArea[1];
    if (screenArea[3] > 0) {
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
    transitionFrame++;
    return 0;
}

int g_UnknownGlobal65b618;                       // frames of the slide

// 0x00471b30
int UIControl::SlideRightStep() {
    if (transitionFrame == 0 && transitionFrameCount) {
        g_UnknownGlobal65b618 = transitionFrameCount < 0 ? 25 : transitionFrameCount;
        slideX = (float)ownerDialog->screenWidth;
        slideStepX = ((float)bounds[0] - slideX) / g_UnknownGlobal65b618;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        slideX += slideStepX;
    }
    if (transitionFrame == g_UnknownGlobal65b618)
        return 1;
    RequestRedraw();
    *(CameraRect*)screenArea = *(CameraRect*)bounds;
    screenArea[0] = (int)slideX;
    screenArea[2] = bounds[2] - bounds[0] + screenArea[0];
    if (screenArea[0] < ownerDialog->screenWidth) {
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
    transitionFrame++;
    return 0;
}

int g_UnknownGlobal65b61c;                       // frames of the slide

// 0x00471c90
int UIControl::SlideBottomStep() {
    if (transitionFrame == 0 && transitionFrameCount) {
        g_UnknownGlobal65b61c = transitionFrameCount < 0 ? 25 : transitionFrameCount;
        slideY = (float)ownerDialog->screenHeight;
        slideStepY = ((float)bounds[1] - slideY) / g_UnknownGlobal65b61c;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        slideY += slideStepY;
    }
    if (transitionFrame == g_UnknownGlobal65b61c)
        return 1;
    RequestRedraw();
    *(CameraRect*)screenArea = *(CameraRect*)bounds;
    screenArea[1] = (int)slideY;
    screenArea[3] = bounds[3] - bounds[1] + screenArea[1];
    if (screenArea[1] < ownerDialog->screenHeight) {
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
    transitionFrame++;
    return 0;
}

// ---------------------------------------------------------------------------
// UIScrollBar

// 0x00475880
int UIScrollBar::ButtonUp(int a, int* position) {
    UnknownDialogEvent event;
    event.handled = 0;
    if (dragging) {
        event.kind = 0x10;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->HandleEvent(&event);
    }
    dragging = 0;
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a && currentState == 2) {
        event.kind = kDialogCommand;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->HandleEvent(&event);
        if (event.handled)
            return 1;
    }
    return UIControl::ButtonUp(a, position);
}

// ---------------------------------------------------------------------------
// UIListBox: rows

unsigned int g_UnknownGlobal65b59c;           // time of the last list box click

// 0x00477d30
void UIListBox::ReportClick(int* handled) {
    UnknownDialogEvent event;
    event.handled = 0;
    event.kind = kDialogListSelect;
    event.code = eventCode;
    event.controlName = controlName;
    event.dialog = ownerDialog;
    event.control = this;
    event.gui = ownerDialog->guiManager;
    ownerDialog->HandleEvent(&event);
    if (handled)
        *handled = 0;
    if (!event.handled) {
        static unsigned int s_doubleClickTime = GetDoubleClickTime();
        g_UnknownGlobal65b59c = ReadClock();
        if (selectedRow == lastClickRow && g_UnknownGlobal65b59c - lastClickTime <= s_doubleClickTime) {
            event.kind = kDialogListDoubleClick;
            event.code = eventCode;
            event.controlName = controlName;
            event.dialog = ownerDialog;
            event.control = this;
            event.gui = ownerDialog->guiManager;
            ownerDialog->HandleEvent(&event);
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
void UIDropDownList::LayoutForRowHeight(int height) {
    listPart->screenArea[3] = listPart->screenArea[1] + height;
    *(CameraRect*)listPart->bounds = *(CameraRect*)listPart->screenArea;
    staticPart0->screenArea[3] = staticPart0->screenArea[1] + height;
    *(CameraRect*)staticPart0->bounds = *(CameraRect*)staticPart0->screenArea;
    scrollBarPart->screenArea[3] = scrollBarPart->screenArea[1] + height;
    scrollBarPart->screenArea[1] += 2;
    scrollBarPart->screenArea[3] -= 2;
    *(CameraRect*)scrollBarPart->bounds = *(CameraRect*)scrollBarPart->screenArea;
    staticPart1->screenArea[3] = staticPart1->screenArea[1] + height;
    *(CameraRect*)staticPart1->bounds = *(CameraRect*)staticPart1->screenArea;
    rowHeight = height;
}

// 0x0047a970
void UIDropDownList::SetStaticImage(UIAnim* image) {
    staticPart0->SetImage(0, image);
    staticPart0->screenArea[2] = image->GetCurrentFrame()->frameWidth + staticPart0->screenArea[0];
    staticPart0->screenArea[3] = image->GetCurrentFrame()->frameHeight + staticPart0->screenArea[1];
    *(CameraRect*)staticPart0->bounds = *(CameraRect*)staticPart0->screenArea;
    *(CameraRect*)listPart->bounds = *(CameraRect*)staticPart0->screenArea;
    *(CameraRect*)listPart->screenArea = *(CameraRect*)listPart->bounds;
    LayoutForRowHeight(image->GetCurrentFrame()->frameHeight);
}

// 0x0047aa40
void UIDropDownList::SetButtonImage(UIAnim* image) {
    staticPart1->SetImage(0, image);
    staticPart1->screenArea[2] = image->GetCurrentFrame()->frameWidth + staticPart1->screenArea[0];
    staticPart1->screenArea[3] = image->GetCurrentFrame()->frameHeight + staticPart1->screenArea[1];
    *(CameraRect*)staticPart1->bounds = *(CameraRect*)staticPart1->screenArea;
    *(CameraRect*)scrollBarPart->screenArea = *(CameraRect*)staticPart1->screenArea;
    scrollBarPart->screenArea[1] += 2;
    scrollBarPart->screenArea[3] -= 2;
    *(CameraRect*)scrollBarPart->bounds = *(CameraRect*)scrollBarPart->screenArea;
}

// 0x0047a2d0
void UIDropDownList::SetOpen(int open) {
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
        staticPart0->Unlink();
        staticPart1->Unlink();
        listPart->Unlink();
        scrollBarPart->Unlink();
        if (ownerDialog->dialogBackground)
            ownerDialog->dialogBackground->MarkAllDirty();
    }
    isOpen = open;
}

// ---------------------------------------------------------------------------
// Drawing and input slots

// 0x0046a300
UIDialog* UIDialog::UnknownVirtualSlot27(void* target, CameraRect* area, int a, int flags,
                                                               SoundGroup* sound, void* textures,
                                                               const char* directory, UIDialog* parent,
                                                               BackgroundImage* background, const char* font,
                                                               int fontSize, GUIManager* gui, GUIUser* user,
                                                               const char* name) {
    GameObject::UnknownVirtualSlot8(target);
    if (area) {
        screenArea = *area;
    } else {
        screenArea.top = 0;
        screenArea.left = 0;
        screenArea.right = ((RenderTarget*)renderContext)->targetWidth;
        screenArea.bottom = ((RenderTarget*)renderContext)->targetHeight;
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
    openingMenu = a;
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
    HandleEvent(&event);
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
        resourceArchive = g_UnknownResourceManager572b44->AddArchive(path, 0);
        sprintf(path, "%s%s", directory ? directory : "", resourceName);
        UnknownTextureStream* stream = new(__FILE__, 0x194) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        if (stream->Open(path, "r", 0)) {
            if (!ReadDialogResource(stream, 0)) {
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
        dialogBackground->MarkAllDirty();
    if (user && !user->pointerDevice) {
        GameObjectIterator iterator(controlContainer, 1, "UIControl");
        RequestHover((UIControl*)iterator.Next(), 0, 0);
    }
    UpdateBoundValues(0);
    sendFrameEvent = 1;
    return this;
}

// 0x0046f890: splits `text` at its "~N" (color N) and "|x" (position
// table[x]) codes into runs, each measured with `font`; 0 when there are no
// codes. `size` receives the visible length and the total width.
char* UIDialog::SplitTextLines(char* text, unsigned int color, void* font, void* table, int* size) {
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
        runs[0].text = text;
        runs[0].color = color;
        runs[0].kind = 1;
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
        run->text = &text[i + 2];
        if (c == '~') {
            int index = text[i + 1] - '0';
            run->kind = 1;
            if (index > 9)
                index = 9;
            else if (index < 0)
                index = 0;
            run->color = textColors[index];
        } else {
            run->kind = 2;
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
                run->position = ((int*)table)[digit];
            } else {
                run->position = 0;
            }
        }
        i += 2;
        if (count) {
            UnknownGameUiTextRun* last = run - 1;
            if (mark)
                last->length = mark - start;
            else
                last->length = run->text - start - 2;
            if (!((PCRenderTarget*)UnknownInlineField18())->renderSurface->GetDC(&dc)) {
                HGDIOBJ old = SelectObject((HDC)dc, (HGDIOBJ)(font ? font : dialogFont));
                GetTextExtentPoint32A((HDC)dc, last->text, last->length, &extent);
                SelectObject((HDC)dc, old);
                ((PCRenderTarget*)UnknownInlineField18())->renderSurface->ReleaseDC(dc);
            }
            last->width = extent.cx;
        }
        start = (char*)run->text;
        mark = 0;
        count++;
    }
    if (count) {
        if (mark)
            runs[count - 1].length = mark - runs[count - 1].text;
        else
            runs[count - 1].length = length - (runs[count - 1].text - text);
        if (!((PCRenderTarget*)UnknownInlineField18())->renderSurface->GetDC(&dc)) {
            HGDIOBJ old = SelectObject((HDC)dc, (HGDIOBJ)(font ? font : dialogFont));
            GetTextExtentPoint32A((HDC)dc, runs[count - 1].text, runs[count - 1].length, &extent);
            SelectObject((HDC)dc, old);
            ((PCRenderTarget*)UnknownInlineField18())->renderSurface->ReleaseDC(dc);
            runs[count - 1].width = extent.cx;
        } else {
            runs[count - 1].width = 0;
        }
    }
    if (size) {
        size[1] = 0;
        size[0] = length - count * 2;
        for (int j = 0; j < count; j++)
            size[1] += runs[j].width;
    }
    runs[count].text = 0;
    runs[count].length = 0;
    runs[count].kind = 0;
    UnknownGameUiTextRun* result =
        (UnknownGameUiTextRun*)DebugMalloc((count + 1) * sizeof(UnknownGameUiTextRun), __FILE__, 0xa5b);
    if (result)
        memcpy(result, runs, (count + 1) * sizeof(UnknownGameUiTextRun));
    return (char*)result;
}

// 0x00471240: draws the state's image, then the text.
int UIControl::Draw() {
    int frames = -1;
    int redraw = redrawFrames;
    CameraRect rect;
    if (drawnTexture) {
        GetImageRect((CameraRect*)screenArea, &rect);
        if (ownerDialog->dialogBackground) {
            ownerDialog->dialogBackground->DrawImage((PCTextureMap*)drawnTexture, &rect, visibleArea,
                                                           drawnTexture->usesColorKey ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            frames = redrawFrames;
        } else if (!((RenderTarget*)renderContext)->UnknownVirtualSlot3(
                       &rect, drawnTexture, visibleArea, drawnTexture->usesColorKey ? 0x1008000 : 0x1000000)) {
            return 0;
        }
    }
    if (controlText && controlTextLength) {
        redrawFrames = redraw;
        GetTextRect((CameraRect*)screenArea, &rect);
        void* dc;
        int more;
        if (ownerDialog->BeginControlDraw(&dc, &rect, &more, this)) {
            do {
                if (wordWrap) {
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
                        DrawTextA((HDC)dc, controlText, controlTextLength, (RECT*)&rect, format);
                        rect.left++;
                        rect.top++;
                        rect.right++;
                        rect.bottom++;
                    }
                    SetTextColor((HDC)dc, textColor);
                    DrawTextA((HDC)dc, controlText, controlTextLength, (RECT*)&rect, format);
                } else {
                    ownerDialog->DrawAlignedText(dc, &rect, controlText, controlTextLength, textColor, fontHeight,
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
        if (clickSound) {
            clickSound->SetVolume(ownerGui->field_0x34c, 0);
            clickSound->PlayWithOptions(1, 0, 0);
        }
        switch (value) {
        case VK_HOME:
            textLength = 0;
            textWidth = 0;
            break;
        case VK_END:
            textLength = controlTextLength;
            break;
        case VK_DELETE:
            if (textLength < controlTextLength) {
                if (IsDBCSLeadByte(controlText[textLength])) {
                    memmove(controlText + textLength, controlText + textLength + 2, controlTextLength - textLength - 1);
                    controlTextLength -= 2;
                } else {
                    memmove(controlText + textLength, controlText + textLength + 1, controlTextLength - textLength);
                    controlTextLength--;
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
            if (textLength < controlTextLength) {
                if (IsDBCSLeadByte(controlText[textLength]))
                    textLength += 2;
                else
                    textLength++;
            }
            break;
        }
        controlText[controlTextLength] = 0;
        void* dc;
        if (!((PCRenderTarget*)ownerDialog->UnknownInlineField18())->renderSurface->GetDC(&dc)) {
            HGDIOBJ font = SelectObject((HDC)dc, (HGDIOBJ)(fontHandle ? fontHandle : (int)ownerDialog->dialogFont));
            SIZE size;
            GetTextExtentPoint32A((HDC)dc, controlText, controlTextLength, &size);
            textExtent[1] = size.cx;
            GetTextExtentPoint32A((HDC)dc, controlText, textLength, &size);
            textWidth = size.cx;
            SelectObject((HDC)dc, font);
            ((PCRenderTarget*)ownerDialog->UnknownInlineField18())->renderSurface->ReleaseDC(dc);
        }
        RequestRedraw();
        return 1;
    }
    return 0;
}

// 0x00474cf0: the joystick moves the focused scroll bar; then hides the
// thumb while every list it follows shows all its rows.
int UIScrollBar::UnknownVirtualSlot10(float frameTime) {
    int range = GetWidth();
    GUIInputDevice* device;
    for (int i = 0; (device = ownerDialog->guiUser->GetAcceptedDevice(i++)) != 0;) {
        if (device->inputDevice->deviceKind == 2 && device->inputDevice->UnknownFunction4897e0(4)) {
            POINT position = device->pointerPosition;
            if (ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
                if (position.y <= -1) {
                    if (ownerDialog->joystickCentred) {
                        if (GetSnapSteps()) {
                            if (GetScrollValue())
                                SetThumbPosition(GetScrollValue() - 1, GetSnapSteps());
                        } else {
                            if (GetScrollValue())
                                SetThumbPosition(GetScrollValue() - 1, range);
                        }
                        ownerDialog->joystickCentred = 0;
                    }
                } else if (position.y >= 1) {
                    if (ownerDialog->joystickCentred) {
                        if (GetSnapSteps()) {
                            if ((unsigned int)GetScrollValue() < (unsigned int)GetSnapSteps())
                                SetThumbPosition(GetScrollValue() + 1, GetSnapSteps());
                        } else {
                            if ((unsigned int)GetScrollValue() < (unsigned int)range)
                                SetThumbPosition(GetScrollValue() + 1, range);
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
        GameObjectIterator* iterator = (GameObjectIterator*)CreateControlIterator();
        int full = 0;
        for (UIControl* control = NextAttachedControl(iterator); control;
             control = NextAttachedControl(iterator)) {
            if (control->controlType == 3 &&
                static_cast<UIListBox*>(control)->visibleRowCount >= static_cast<UIListBox*>(control)->rowCount)
                full = 1;
        }
        DeleteControlIterator(iterator);
        if (full)
            drawnTexture = 0;
    }
    return result;
}

// 0x00474ed0: draws the track centred across the bar, then the thumb.
int UIScrollBar::Draw() {
    CameraRect rect;
    TextureMap* thumb;
    if (trackImage && drawnTexture) {
        TextureMap* track = trackImage->GetCurrentTexture();
        int redraw = redrawFrames;
        rect = *(CameraRect*)screenArea;
        if (controlType == 8) {
            rect.top += UnknownMaxInt((screenArea[3] - screenArea[1] - track->textureHeight) / 2, 0);
            rect.bottom = track->textureHeight + rect.top;
        } else {
            rect.left += UnknownMaxInt((screenArea[2] - screenArea[0] - track->textureWidth) / 2, 0);
            rect.right = track->textureWidth + rect.left;
        }
        if (ownerDialog->dialogBackground)
            ownerDialog->dialogBackground->DrawImage((PCTextureMap*)track, &rect, 0,
                                                           track->usesColorKey ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
        else if (!((RenderTarget*)renderContext)->UnknownVirtualSlot3(&rect, track, 0,
                                                                   track->usesColorKey ? 0x1008000 : 0x1000000))
            goto failed;
        redrawFrames = redraw;
    }
    thumb = drawnTexture;
    if (thumb) {
        if (controlType == 8) {
            rect.top = (screenArea[3] - thumbHeight - screenArea[1]) / 2 + screenArea[1];
            rect.left = (int)thumbOffset + screenArea[0];
            rect.bottom = thumbHeight + rect.top;
            rect.right = thumbWidth + rect.left;
        } else {
            rect.top = (int)thumbOffset + screenArea[1];
            rect.left = (screenArea[2] - thumbWidth - screenArea[0]) / 2 + screenArea[0];
            rect.bottom = thumbHeight + rect.top;
            rect.right = thumbWidth + rect.left;
        }
        if (ownerDialog->dialogBackground) {
            ownerDialog->dialogBackground->DrawImage((PCTextureMap*)thumb, &rect, 0,
                                                           thumb->usesColorKey ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            if (stateImages[currentState]->frameCount == 1)
                needsRedraw = 1;
            else
                RequestRedraw();
        } else if (!((RenderTarget*)renderContext)->UnknownVirtualSlot3(&rect, thumb, 0,
                                                                     thumb->usesColorKey ? 0x1008000 : 0x1000000)) {
            goto failed;
        }
    }
    return 1;

failed:
    return 0;
}

// 0x004755d0: a click on the bar moves the thumb (slider) or pages the lists.
int UIScrollBar::ButtonDown(int a, int b) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a && b) {
        GameObjectIterator* iterator;
        UIControl* list;
        if (controlType == 8) {
            if (snapSteps) {
                thumbOffset = (float)UnknownMinInt(GetWidth() - thumbWidth,
                                                         UnknownMaxInt(((int*)b)[0] - screenArea[0], 0));
            } else {
                float x = (float)((int*)b)[0];
                float left = (float)screenArea[0];
                if (x > thumbOffset + (left + thumbWidth)) {
                    iterator = (GameObjectIterator*)CreateControlIterator();
                    for (list = NextAttachedControl(iterator); list; list = NextAttachedControl(iterator)) {
                        if (list->controlType == 3) {
                            UIListBox* listBox = static_cast<UIListBox*>(list);
                            int page = listBox->visibleRowCount;
                            listBox->ScrollToRow(listBox->GetFirstVisibleRow() + page, 1);
                        }
                    }
                    DeleteControlIterator(iterator);
                } else if (x < left + thumbOffset) {
                    iterator = (GameObjectIterator*)CreateControlIterator();
                    for (list = NextAttachedControl(iterator); list; list = NextAttachedControl(iterator)) {
                        if (list->controlType == 3) {
                            UIListBox* listBox = static_cast<UIListBox*>(list);
                            int page = listBox->visibleRowCount;
                            listBox->ScrollToRow(listBox->GetFirstVisibleRow() - page, 1);
                        }
                    }
                    DeleteControlIterator(iterator);
                }
            }
        } else {
            if (snapSteps) {
                thumbOffset = (float)UnknownMinInt(GetHeight() - thumbHeight,
                                                         UnknownMaxInt(((int*)b)[1] - screenArea[1], 0));
            } else {
                float y = (float)((int*)b)[1];
                float top = (float)screenArea[1];
                if (y > thumbOffset + (top + thumbHeight)) {
                    iterator = (GameObjectIterator*)CreateControlIterator();
                    for (list = NextAttachedControl(iterator); list; list = NextAttachedControl(iterator)) {
                        if (list->controlType == 3) {
                            UIListBox* listBox = static_cast<UIListBox*>(list);
                            int page = listBox->visibleRowCount;
                            listBox->ScrollToRow(listBox->GetFirstVisibleRow() + page, 1);
                        }
                    }
                    DeleteControlIterator(iterator);
                } else if (y < top + thumbOffset) {
                    iterator = (GameObjectIterator*)CreateControlIterator();
                    for (list = NextAttachedControl(iterator); list; list = NextAttachedControl(iterator)) {
                        if (list->controlType == 3) {
                            UIListBox* listBox = static_cast<UIListBox*>(list);
                            int page = listBox->visibleRowCount;
                            listBox->ScrollToRow(listBox->GetFirstVisibleRow() - page, 1);
                        }
                    }
                    DeleteControlIterator(iterator);
                }
            }
        }
        if (snapSteps)
            SetThumbPosition(ScaleThumbPosition(snapSteps), snapSteps);
    }
    return UIControl::ButtonDown(a, b);
}

// 0x00475970: dragging the thumb; holding it still repeats kind 0x11.
void UIScrollBar::ButtonHeld(int a, int* position) {
    UIControl::ButtonHeld(a, position);
    UnknownDialogEvent event;
    event.handled = 0;
    if (!a && position) {
        int width = GetWidth();
        int height = GetHeight();
        RequestRedraw();
        if (controlType == 8) {
            if (snapSteps)
                thumbOffset = (float)UnknownMinInt(width - thumbWidth,
                                                       UnknownMaxInt(position[0] - screenArea[0], 0));
            else
                thumbOffset = (float)UnknownMinInt(width - thumbWidth,
                                                       UnknownMaxInt(position[0] - screenArea[0] - thumbWidth / 2, 0));
        } else {
            if (snapSteps != 0.0f)
                thumbOffset = (float)UnknownMinInt(height - thumbHeight,
                                                       UnknownMaxInt(position[1] - screenArea[1], 0));
            else
                thumbOffset = (float)UnknownMinInt(height - thumbHeight,
                                                       UnknownMaxInt(position[1] - screenArea[1] - thumbHeight / 2, 0));
        }
        if (snapSteps)
            SetThumbPosition(ScaleThumbPosition(snapSteps), snapSteps);
        if (!dragging) {
            event.kind = 0xf;
            event.code = eventCode;
            event.controlName = GetName();
            event.dialog = ownerDialog;
            event.gui = ownerGui;
            event.control = this;
            ownerDialog->HandleEvent(&event);
            dragging = 1;
        }
        if (!event.handled) {
            if (position[0] == dragX && position[1] == dragY) {
                holdTime += ownerDialog->lastFrameTime;
                if (holdTime < holdDelay)
                    goto scroll;
                event.kind = 0x11;
                event.code = eventCode;
                event.controlName = GetName();
                event.dialog = ownerDialog;
                event.gui = ownerGui;
                event.control = this;
                ownerDialog->HandleEvent(&event);
            } else {
                event.kind = 3;
                event.code = eventCode;
                event.controlName = GetName();
                event.dialog = ownerDialog;
                event.gui = ownerGui;
                event.control = this;
                ownerDialog->HandleEvent(&event);
                dragX = position[0];
                dragY = position[1];
                holdTime = 0.0f;
            }
            if (!event.handled) {
            scroll:
                GameObjectIterator* iterator = (GameObjectIterator*)CreateControlIterator();
                for (UIControl* list = NextAttachedControl(iterator); list;
                     list = NextAttachedControl(iterator)) {
                    if (list->controlType == 3) {
                        UIListBox* listBox = static_cast<UIListBox*>(list);
                        listBox->ScrollToRow(
                            ScaleThumbPosition(listBox->rowCount - listBox->lastPageRowCount), 0);
                    }
                }
                DeleteControlIterator(iterator);
            }
        }
    }
}

// 0x00476020: the joystick moves the selection; the row under the cursor
// is +0x248.
int UIListBox::UnknownVirtualSlot10(float frameTime) {
    GUIInputDevice* device;
    for (int i = 0; (device = ownerDialog->guiUser->GetAcceptedDevice(i++)) != 0;) {
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
    hoverRow = -1;
    GUIUser* user = ownerDialog->guiUser;
    if (user->pointerDevice) {
        POINT cursor = user->pointerDevice->pointerPosition;
        if (user->hoverControl == (UnknownGuiControl*)this && selectable) {
            RECT rect = *(RECT*)screenArea;
            for (int row = firstVisibleRow; row < firstVisibleRow + visibleRowCount; row++) {
                rect.bottom = rect.top + GetRowHeight(row);
                if (PtInRect(&rect, cursor) && !(rowTable[row].flags & 1)) {
                    hoverRow = row;
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
int UIListBox::Draw() {
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
            ownerDialog->dialogBackground->DrawImage((PCTextureMap*)drawnTexture, (CameraRect*)screenArea,
                                                           visibleArea,
                                                           drawnTexture->usesColorKey ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            redrawFrames = redraw;
        } else if (!((RenderTarget*)renderContext)->UnknownVirtualSlot3(
                       screenArea, drawnTexture, visibleArea, drawnTexture->usesColorKey ? 0x1008000 : 0x1000000)) {
            goto failed;
        }
    }
    area = *(CameraRect*)screenArea;
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
                if (partialRows && visible <= 0) {
                    whole = 0;
                } else {
                    whole = 1;
                    if (height + y > area.bottom)
                        break;
                }
                switch ((int)GetRowKind(row)) {
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
                        RequestRedraw();
                    }
                    if (whole && height + y > area.bottom)
                        break;
                case 2:
                    if (pass == 1 && rowTable[row].image) {
                        int width = rowTable[row].image->GetCurrentFrame()->frameWidth;
                        int imageHeight = rowTable[row].image->GetCurrentFrame()->frameHeight;
                        rect.left = scrollOffsetX + area.left;
                        rect.top = y;
                        rect.right = rect.left + width;
                        if (rect.right >= area.right)
                            rect.right = area.right;
                        if (partialRows) {
                            if (imageHeight + y >= area.bottom)
                                rect.bottom = area.bottom;
                            else
                                rect.bottom = imageHeight + y;
                        } else {
                            rect.bottom = imageHeight + y;
                        }
                        TextureMap* texture = rowTable[row].image->GetCurrentFrame()->frameTexture;
                        if (ownerDialog->dialogBackground) {
                            ownerDialog->dialogBackground->DrawImage(
                                (PCTextureMap*)texture, &rect, 0, texture->usesColorKey ? 0x1008000 : 0x1000000,
                                backgroundRegion, needsRedraw, &redrawFrames, 0);
                            if (rowTable[row].image->frameCount > 1) {
                                done = 0;
                                frames = redrawFrames;
                            }
                        } else if (!((RenderTarget*)renderContext)->UnknownVirtualSlot3(
                                       &rect, texture, 0, texture->usesColorKey ? 0x1008000 : 0x1000000)) {
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
                            rect.left += dragOffsetX + scrollOffsetX;
                            rect.right += dragOffsetX + scrollOffsetX;
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
                if (hoverRow == row && pass == 0 && visibleRowCount > 1) {
                    rect.left = area.left;
                    rect.top = y;
                    rect.bottom = height + y;
                    rect.right = area.right;
                    if (itemBrush)
                        FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)itemBrush);
                    else
                        FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
                    RequestRedraw();
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

// 0x00479710: dragging the thumb; holding it still repeats kind 0x11.
void UIDDLScrollBar::ButtonHeld(int a, int* position) {
    UIControl::ButtonHeld(a, position);
    UnknownDialogEvent event;
    event.handled = 0;
    if (!a && position) {
        int width = GetWidth();
        int height = GetHeight();
        RequestRedraw();
        if (controlType == 8) {
            if (snapSteps)
                thumbOffset = (float)UnknownMinInt(width - thumbWidth,
                                                       UnknownMaxInt(position[0] - screenArea[0], 0));
            else
                thumbOffset = (float)UnknownMinInt(width - thumbWidth,
                                                       UnknownMaxInt(position[0] - screenArea[0] - thumbWidth / 2, 0));
        } else {
            if (snapSteps != 0.0f)
                thumbOffset = (float)UnknownMinInt(height - thumbHeight,
                                                       UnknownMaxInt(position[1] - screenArea[1], 0));
            else
                thumbOffset = (float)UnknownMinInt(height - thumbHeight,
                                                       UnknownMaxInt(position[1] - screenArea[1] - thumbHeight / 2, 0));
        }
        if (snapSteps)
            SetThumbPosition(ScaleThumbPosition(snapSteps), snapSteps);
        if (!dragging) {
            event.kind = 0xf;
            event.code = eventCode;
            event.controlName = ownerList->GetName();
            event.dialog = ownerDialog;
            event.gui = ownerGui;
            event.control = this;
            ownerDialog->HandleEvent(&event);
            dragging = 1;
        }
        if (!event.handled) {
            if (position[0] == dragX && position[1] == dragY) {
                holdTime += ownerDialog->lastFrameTime;
                if (holdTime < holdDelay)
                    goto scroll;
                event.kind = 0x11;
                event.code = eventCode;
                event.controlName = ownerList->GetName();
                event.dialog = ownerDialog;
                event.gui = ownerGui;
                event.control = this;
                ownerDialog->HandleEvent(&event);
            } else {
                event.kind = 3;
                event.code = eventCode;
                event.controlName = ownerList->GetName();
                event.dialog = ownerDialog;
                event.gui = ownerGui;
                event.control = this;
                ownerDialog->HandleEvent(&event);
                dragX = position[0];
                dragY = position[1];
                holdTime = 0.0f;
            }
            if (!event.handled) {
            scroll:
                UIListBox* list = ownerList->listPart;
                if (list != 0) {
                    int count = list->rowCount;
                    int page = list->lastPageRowCount;
                    list->ScrollToRow(ScaleThumbPosition(count - page), 0);
                }
            }
        }
    }
}

// 0x00479a00: a click on the bar moves the thumb (slider) or pages the list.
int UIDDLScrollBar::ButtonDown(int a, int b) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && !a && b) {
        UIListBox* list = ownerList->listPart;
        if (snapSteps) {
            thumbOffset = (float)UNKNOWN_MIN(GetHeight() - thumbHeight,
                                                   UNKNOWN_MAX(((int*)b)[1] - screenArea[1], 0));
        } else {
            float y = (float)((int*)b)[1];
            float top = (float)screenArea[1];
            if (y > thumbOffset + (top + thumbHeight))
            {
                int page = list->visibleRowCount;
                list->ScrollToRow(list->GetFirstVisibleRow() + page, 1);
            } else if (y < top + thumbOffset) {
                int page = list->visibleRowCount;
                list->ScrollToRow(list->GetFirstVisibleRow() - page, 1);
            }
        }
    }
    if (snapSteps)
        SetThumbPosition(ScaleThumbPosition(snapSteps), snapSteps);
    return UIControl::ButtonDown(a, b);
}

// 0x0047ab20
void UIDropDownList::SetCornerImage(UIAnim* image) {
    staticPart2->SetImage(0, image);
    staticPart2->screenArea[1] = 0;
    staticPart2->screenArea[0] = 0;
    staticPart2->screenArea[2] = image->GetCurrentFrame()->frameWidth;
    staticPart2->screenArea[3] = image->GetCurrentFrame()->frameHeight;
    *(CameraRect*)staticPart2->bounds = *(CameraRect*)staticPart2->screenArea;
    screenArea[2] = staticPart2->screenArea[2] + screenArea[0];
    screenArea[3] = staticPart2->screenArea[3] + screenArea[1];
    *(CameraRect*)bounds = *(CameraRect*)screenArea;
    staticPart0->screenArea[0] = 0;
    staticPart0->screenArea[1] = staticPart2->screenArea[3];
    *(CameraRect*)staticPart0->bounds = *(CameraRect*)staticPart0->screenArea;
    *(CameraRect*)listPart->bounds = *(CameraRect*)staticPart0->screenArea;
    *(CameraRect*)listPart->screenArea = *(CameraRect*)listPart->bounds;
    if (buttonPart->bounds[0]) {
        buttonPart->screenArea[1] = 0;
        buttonPart->screenArea[0] = staticPart2->screenArea[2];
        buttonPart->screenArea[2] =
            buttonPart->GetStateTexture(-1)->textureWidth + buttonPart->screenArea[0];
        buttonPart->screenArea[3] =
            buttonPart->GetStateTexture(-1)->textureHeight + buttonPart->screenArea[1];
        *(CameraRect*)buttonPart->bounds = *(CameraRect*)buttonPart->screenArea;
    }
    staticPart1->screenArea[0] = staticPart2->screenArea[2];
    staticPart1->screenArea[1] = staticPart2->screenArea[3];
    *(CameraRect*)staticPart1->bounds = *(CameraRect*)staticPart1->screenArea;
    *(CameraRect*)scrollBarPart->screenArea = *(CameraRect*)staticPart1->screenArea;
    scrollBarPart->screenArea[1] += 2;
    scrollBarPart->screenArea[3] -= 2;
    *(CameraRect*)scrollBarPart->bounds = *(CameraRect*)scrollBarPart->screenArea;
}

// 0x0047b370 (UIProgressBar): one step on; redraws when `redraw`.
void UIProgressBar::StepProgress(int redraw) {
    int total = stepCount;
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
    : UIStatic(flags, area, owner) {
    moviePlayer = 0;
}

// 0x0047ae90
int UIVideoStatic::PlayMovie(const char* file, int a, void (*done)(UIDialog* dialog), UIDialog* owner) {
    if (moviePlayer)
        moviePlayer->Release();
    moviePlayer = new(__FILE__, 0x264d) MediaControl(1);
    if (!moviePlayer->Open(renderContext, file, done, owner)) {
        moviePlayer = 0;
        return 0;
    }
    AppendChild(moviePlayer, -1);
    frameRect[0] = 0;
    frameRect[1] = 0;
    frameRect[2] = moviePlayer->frameWidth;
    frameRect[3] = moviePlayer->frameHeight;
    if (a)
        moviePlayer->Restart();
    return 1;
}

// 0x0047af90
int UIVideoStatic::UnknownVirtualSlot10(float frameTime) {
    return UIControl::UnknownVirtualSlot10(frameTime);
}

// 0x0047afa0
int UIVideoStatic::Draw() {
    CameraRect rect;
    if (!moviePlayer)
        return 0;
    if (ownerDialog->dialogBackground) {
        ownerDialog->dialogBackground->ReleaseDc();
        RequestRedraw();
    }
    GetImageRect((CameraRect*)screenArea, &rect);
    return ((PCRenderTarget*)renderContext)->renderSurface->Blt(&rect, moviePlayer->frameSurface, frameRect,
                                                                      0x1000000, 0) == 0;
}

// ---------------------------------------------------------------------------
// UIProgressBar

// 0x0047b020
UIProgressBar::UIProgressBar(int flags, CameraRect* area, UIDialog* owner)
    : UIStatic(flags, area, owner) {
    progressFraction = 0;
    stepCount = 0;
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
TextureMap* UIProgressBar::GetStateTexture(int state) {
    return barTexture;
}

// 0x0047b110
int UIProgressBar::Draw() {
    int redraw = redrawFrames;
    CameraRect rect;
    void* dc;
    int more;
    if (barTexture && progressFraction != 0.0f) {
        GetImageRect((CameraRect*)screenArea, &rect);
        CameraRect source = *(CameraRect*)visibleArea;
        source.right = (int)((visibleArea[2] - visibleArea[0]) * progressFraction);
        UIDialog* owner = ownerDialog;
        rect.right = (int)(source.right * owner->scaleX + rect.left);
        rect.bottom = (int)(source.bottom * owner->scaleY + rect.top);
        if (owner->dialogBackground) {
            owner->dialogBackground->DrawImage((PCTextureMap*)barTexture, &rect, &source,
                                                      barTexture->usesColorKey ? 0x1008000 : 0x1000000,
                                                      backgroundRegion, needsRedraw, &redraw, 0);
        } else if (!((RenderTarget*)renderContext)->UnknownVirtualSlot3(
                       &rect, barTexture, &source, barTexture->usesColorKey ? 0x1008000 : 0x1000000)) {
            return 0;
        }
    }
    if (showPercentage) {
        redraw = redrawFrames;
        GetTextRect((CameraRect*)screenArea, &rect);
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
void UIProgressBar::SetBarTexture(int texture, int owned) {
    barTexture = (TextureMap*)texture;
    ownsBarTexture = owned;
}

// ---------------------------------------------------------------------------
// Input, image and list slots promoted from the near-miss sample

// 0x004715d0
int UIControl::GrowStep() {
    if (transitionFrame == 0 && transitionFrameCount && !isPaused) {
        g_UnknownGlobal65b60c = transitionFrameCount < 0 ? 7 : transitionFrameCount;
        zoomWidth = 21.0f;
        zoomHeight = 11.0f;
        int left = bounds[0];
        int width = bounds[2] - left;
        slideX = max(0.0f, (float)(width / 2 + left) - 10.5f);
        int top = bounds[1];
        int height = bounds[3] - top;
        slideY = max(0.0f, (float)(height / 2 + top) - 5.5f);
        slideStepX = ((float)left - slideX) / g_UnknownGlobal65b60c;
        slideStepY = ((float)top - slideY) / g_UnknownGlobal65b60c;
        zoomStepWidth = ((float)width - 21.0f) / g_UnknownGlobal65b60c;
        zoomStepHeight = ((float)height - 11.0f) / g_UnknownGlobal65b60c;
        if (slideSound) {
            slideSound->SetVolume(ownerGui->field_0x34c, 0);
            slideSound->PlayWithOptions(1, 0, 0);
        }
    } else {
        slideX += slideStepX;
        slideY += slideStepY;
        zoomWidth += zoomStepWidth;
        zoomHeight += zoomStepHeight;
    }
    if (transitionFrame == g_UnknownGlobal65b60c)
        return 1;
    RequestRedraw();
    if (!isPaused || transitionFrame) {
        float width = zoomWidth;
        screenArea[0] = (int)slideX;
        screenArea[1] = (int)slideY;
        screenArea[2] = (int)(width + slideX);
        screenArea[3] = (int)(slideY + zoomHeight);
        switch (controlType) {
        case 1:
        case 4:
        case 5:
        case 9:
        case 10:
            drawnTexture = stateImages[currentState]->GetCurrentTexture();
            transitionFrame++;
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
            handled = ButtonUp(event->control, (int*)&position);
    }
    if (!handled)
        return GameObject::UnknownVirtualSlot22(event, entry);
    return 1;
}

// 0x004753c0
int UIScrollBar::SetThumbPosition(int value, int range) {
    unsigned int position = reversed ? range - value : value;
    RequestRedraw();
    if (!snapSteps && ScaleThumbPosition(range) == position)
        return 1;
    if (!range) {
        thumbOffset = position;
        return 1;
    }
    unsigned int travel = controlType == 8 ? GetWidth() - thumbWidth : GetHeight() - thumbHeight;
    if ((unsigned int)range > 0) {
        float limit = travel;
        float scaled = (double)(travel * position) / (unsigned int)range;
        thumbOffset = scaled < limit ? scaled : limit;
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
    if (wrapScroll) {
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
        SetSelectedRow(firstVisibleRow);
        ReportClick(&handled);
        if (handled)
            return;
    }
    RequestRedraw();
    UpdateScrollBars();
}

// 0x00477ff0
int UIListBox::ButtonUp(int a, int* position) {
    if (ownerDialog->guiUser->hoverControl == (UnknownGuiControl*)this && a == 1) {
        scrollOffsetX += dragOffsetX;
        dragOffsetX = 0;
    }
    return UIControl::ButtonUp(a, position);
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
        ReportClick(&handled);
        return 1;
    }
    return 0;
}

// 0x00478810
TextureMap* UIMultiState::GetStateTexture(int state) {
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
void UIDDLListBox::ReportClick(int* handled) {
    UnknownDialogEvent event;
    event.handled = 0;
    ownerList->SetText(GetRowText(GetSelectedRow()));
    ownerList->SetOpen(0);
    event.kind = kDialogListSelect;
    event.code = eventCode;
    event.controlName = ownerList->GetName();
    event.dialog = ownerDialog;
    event.control = this;
    event.gui = ownerDialog->guiManager;
    ownerDialog->HandleEvent(&event);
}

// 0x00472bc0
int UIFrame::LoadFromStream(void* stream, int offset, void* palette) {
    if (stream) {
        UnknownTgaFile* image = ReadWholeFile((UnknownTextureStream*)stream, 0, offset);
        if (image) {
            int format = g_TrackGame->renderTarget->surfacePixelFormat;
            frameWidth = image->width;
            frameHeight = image->height;
            frameTexture = new(__FILE__, 0x13d6) PCTextureMap((TextureMapManager*)frameTextures, 1);
            Palette8* pal = (Palette8*)palette;
            frameTexture->UnknownVirtualSlot4(image->bits, image->width, image->height, image->width,
                                            image->width, 0x22b, format,
                                            (UnknownTexturePalette*)(pal ? pal->field_0x708 : 0), 4,
                                            pal ? pal->field_0x70c : 0, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
            if (TextureHasColor(frameTexture, 0xff00ff))
                frameTexture->UnknownVirtualSlot18(0xff00ff);
            FreeFile(image);
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
                IsEnabled() && shown) {
                ownerDialog->guiUser->RequestHover((UnknownGuiControl*)this, 0, 1);
                ButtonDown(0, 0);
                ButtonUp(0, 0);
                return 1;
            }
            if (TestInputEvent(0x32, event->kind, event, 0x80) &&
                ownerDialog->guiUser->field_0x1dc == (int)this)
                moveable = 1;
        }
        if (event->kind) {
            POINT position;
            GUIInputDevice* device = ownerDialog->guiUser->pointerDevice;
            if (device)
                position = device->pointerPosition;
            handled = ButtonDown(event->control, ownerDialog->guiUser->pointerDevice ? (int)&position : 0);
        }
    }
    if (!handled)
        return GameObject::UnknownVirtualSlot23(event, entry);
    return 1;
}

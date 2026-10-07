// Near misses for gameui.cpp (canonical file: src/reconstructed/GameUi.cpp,
// included below for its types and matched functions). Check: compile this
// file and compare each function with GameUiNearMisses.bindings.json.
//
// 0x0046e8c0 dialog slot 28 (55/64): retail stores the value before loading
//   the vtable (and so keeps the vtable in eax); every store order gives the
//   hoisted load.
// 0x0046ea80 (160/164): the two arguments are loaded into the opposite
//   registers (id in esi in retail); a local copy of either does not change it.
// 0x0046ff70 (23/71): the same event stores; retail schedules them in field
//   order, VC6 here hoists `this` and the parent's GUI. Seven store orders were
//   tried, plus every position of the control store.
// 0x00470170 UIControl constructor (535/578): retail places the owner == 0
//   block differently.
// 0x00470450 UIControl destructor (88/386): retail threads the tool-tip jumps
//   and keeps a value in edi.
// 0x004705d0 UIControl slot 49 (91/101) and 0x004733a0 UIButton slot 49
//   (85/99): the value loaded for each call lands in another register
//   (retail: ecx/edx, VC6 here: edx/eax); an int SetFontColor, a local for the
//   state, `if (enabled)` and the inverted branch order give the same bytes.
// 0x00470f10 UIControl slot 39 (226/332): a register permutation.
// 0x00472e30 UIAnim constructor (34/82): retail stores the palette (+0xf4)
//   after the zeroed members, right before the frame-list memset; VC6 here
//   hoists it next to the texture store whatever the statement, initializer
//   list or loop form.
// 0x00472fe0 UIAnim advance (77/194): retail copies the frame index into ecx
//   before indexing the frame list (`mov ecx, eax`); a local index, a
//   reloaded member and GetCurrentFrame() all index with eax.
// 0x004749f0 UIScrollCtl slot 60 (52/274): retail lets case 10 fall into the
//   shared event tail (ending in its own ret) and places case 9 after it,
//   then case 0x101; VC6 here places the tail after case 9. Switch (either
//   case order, `break` or `return` after the tail), if/else and goto forms
//   all give the same layout.
// 0x00475c70 UIListBox constructor (319/382): retail re-tests the row height
//   after the owner's font height test and reloads the owner for it; VC6
//   here threads both tests (five if/ternary forms tried).
// 0x00477110 adds an image row from a file (356/646): with `a` a UIAnim
//   loaded from `file`, otherwise a row naming the TGA file with its header's
//   size. Calls, constants and the row stores follow retail. Retail places
//   the function's epilogue after the stream-failure block and sends every
//   `return 0` (and the final `return 1`) there; VC6 here keeps the epilogue
//   last, which shifts every later offset. Retail also loads `b` into eax and
//   the frame's size into ecx/edx (VC6 here: ecx, edx/eax). Fail-first,
//   if-block and goto forms were tried.
// 0x004773a0 adds an image row (169/232): retail loads the frame's width
//   before its height and stores the row's +0x18 later; 40 store orders were
//   tried.
// 0x00477800 qsort compare (5/262).
// 0x00477bc0 (89/286): retail keeps zero in a different register.
// 0x00477e90 UIListBox slot 55 (155/344): retail keeps the point in ebx and
//   the row offset in ebp; VC6 here swaps them.
// 0x0047b490 colour-key test (116/210): retail keeps the key pixel in esi,
//   the width in edx and spills the row counter into the key's argument
//   slot; VC6 here spills the pixel instead. Hoisting the width or
//   reordering the red/green/blue terms moves the register pressure
//   elsewhere.
// 0x00472960 UIFrame constructor from a file (352/562): retail places the
//   sprintf (image missing) block after the epilogue, jumping back to the
//   `if (image)` free test; VC6 here lays it inline.
// 0x0046ef00 UIDialog slot 10 (352/555): the timer's +0x0c is cleared before
//   +0x14 is loaded, and the joystick branches sit after the epilogue.
// 0x004734c0 UIButton slot 28 (256/328) and 0x00478e10 UIMultiState slot 28
//   (28/328): retail keeps the 8-bit result in edi and the 16-bit one in
//   memory, tests Unlock's result and returns separately when Lock fails.
//   Ternary, separate-result and goto forms keep the result in memory.
// 0x004738a0 UIEditBox slot 40 (206/975): retail keeps the width and the
//   redraw count in memory.
// 0x00474150 UIEditBox slot 20 (339/1048): Backspace: retail loads the
//   length before the lead-byte test.
// 0x00474880 UIScrollCtl slot 55 (254/355): retail pushes every register in
//   the prologue and keeps `this` in esi.
// 0x00479710 UIDDLScrollBar slot 57 (736/738): the list's +0x204 goes through
//   ebp instead of ecx (retail keeps `position`'s register busy there).
//   Inlining the row expression costs a prologue push instead.
// 0x00478570 UIMultiState slot 40 (139/682): retail keeps &+0x1bc in ebx and
//   shares its spill slot with the DC.
// 0x0047a400 drop-down layout (320/960): store scheduling of the part rects.
// 0x0046a920 UIDialog's resource parser (16 KB; 92% of instructions equal
//   with stack offsets and relocations ignored, 74% raw): the control flow,
//   calls and constants follow retail. Retail keeps the current control in
//   ebx across the section loop and the pass counter in memory; VC6 here
//   gives ebx to the pass counter (and to the state/item counters), so the
//   control is spilled. Retail's frame is 0x80 bytes larger: VC6 lays locals
//   out by use count (most used nearest esp; ties by size, then by a fixed
//   permutation of the use order), and retail has an extra, never-referenced
//   0x80-byte local between `toolTip` and `fxSoundOut` (esp+0x2184 of the
//   0x9c74 frame) and a different use-count order for the key buffers, so
//   most stack offsets differ. The jump table makes ckm report an unresolved
//   $L label; compare with sdiff.

#include "../../src/reconstructed/GameUi.cpp"

#include <imm.h>

#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/KeyboardDevice.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/Palette8.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x0065b5c8: the time stamp of the last image step (UIAnim 0x00472fe0).
int g_UnknownGlobal65b5c8;

// 0x0046e8c0
void UnknownGameUiDialog::UnknownVirtualSlot28(int value) {
    UnknownDialogEvent event;
    event.handled = 0;
    event.gui = guiManager;
    event.code = 0;
    event.controlName = 0;
    event.control = 0;
    event.field_0x18 = value;
    event.kind = kDialogInit;
    event.dialog = this;
    UnknownVirtualSlot29(&event);
}

// 0x0046ea80
void UIDialog::EnableGroup(int id, int value) {
    GameObjectIterator iterator(((UnknownGameUiDialog*)this)->controlContainer, 1, "UIControl");
    UIControl* control;
    while ((control = (UIControl*)iterator.Next()) != 0) {
        if (control->groupId == id)
            control->UnknownVirtualSlot49(value);
    }
}

// 0x0046ff70
void UIDialog::NotifyParent(int a, int b) {
    UnknownGameUiDialog* parent = ((UnknownGameUiDialog*)this)->parentDialog;
    if (parent) {
        UnknownDialogEvent event;
        event.kind = b;
        event.handled = 0;
        event.code = a;
        event.controlName = 0;
        event.dialog = parent;
        event.control = (UIControl*)this;
        event.gui = parent->guiManager;
        parent->UnknownVirtualSlot29(&event);
    }
}

// 0x00470170
UIControl::UIControl(int type, int id, CameraRect* area, UnknownGameUiDialog* owner)
    : GameObject(1) {
    AppendClassName(this);
    controlType = type;
    eventCode = id;
    if (area) {
        *(CameraRect*)field_0x2c = *area;
        *(CameraRect*)field_0x3c = *area;
    } else {
        field_0x3c[0] = field_0x3c[1] = field_0x3c[2] = field_0x3c[3] = 0;
        *(CameraRect*)field_0x2c = *(CameraRect*)field_0x3c;
    }
    savedState = 0;
    currentState = 0;
    field_0x68 = 0;
    enabled = 1;
    field_0x70 = 1;
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
    field_0xd0 = 0;
    textLines = 0;
    needsRedraw = 1;
    moveable = 0;
    keyBind = 0;
    field_0x1c8 = 0;
    field_0x1c4 = 0;
    mouseAnim = 0;
    for (int i = 0; i < 5; i++) {
        stateImages[i] = 0;
        sounds[i] = 0;
    }
    field_0x164 = 0;
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
    field_0x1d0 = 0;
    field_0xb4 = 0;
    field_0x1d4 = -1;
    if (owner) {
        post3D = owner->field_0x7f18;
        field_0x18 = owner->UnknownInlineField18();
    } else {
        post3D = 1;
    }
    permanent = 0;
    field_0x1e8 = 0;
}

// 0x00470450
UIControl::~UIControl() {
    if (ownerDialog && !g_TrackGame->field_0x2d5_bit1) {
        field_0x25_bit3 = 1;
        ToolTip* tip = 0;
        if (ownerDialog->guiUser) {
            if (ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
                ownerDialog->guiUser->UnknownFunction487bf0(0);
                tip = ownerDialog->guiUser->userToolTip;
            }
            if (ownerDialog->guiUser && ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this) {
                ownerDialog->UnknownFunction470000(0, 0, 1);
                tip = ownerDialog->guiUser->userToolTip;
            }
        }
        if (tip)
            tip->ShowText(0, 0, 1.0f);
    }
    if (backgroundRegion != -1 && ownerDialog && ownerDialog->dialogBackground)
        ownerDialog->dialogBackground->UnknownFunction404200(backgroundRegion);
    if (controlText)
        DebugFree(controlText, __FILE__, 0xba3);
    if (toolTipText)
        DebugFree(toolTipText, __FILE__, 0xba4);
    if (ownsFont)
        DeleteObject((HGDIOBJ)fontHandle);
    if (textLines)
        DebugFree(textLines, __FILE__, 0xba8);
    if (field_0x1c4)
        DebugFree(field_0x1c4, __FILE__, 0xba9);
}

// 0x004705d0
void UIControl::UnknownVirtualSlot49(int enable) {
    if (enabled == enable)
        return;
    enabled = enable;
    if (enable) {
        UnknownVirtualSlot29(savedState);
        SetFontColor(currentTextColor);
    } else {
        if (stateImages[4]) {
            savedState = currentState;
            UnknownVirtualSlot29(4);
        }
        currentTextColor = textColor;
        SetFontColor(0x999999);
    }
}

// 0x00470f10
void UIControl::UnknownVirtualSlot39() {
    UnknownGameUiDialog* owner = ownerDialog;
    int left = field_0x3c[0];
    field_0x2c[0] = (int)(left * owner->scaleX);
    int top = field_0x3c[1];
    field_0x2c[1] = (int)(top * owner->scaleY);
    int right = field_0x3c[2];
    field_0x2c[2] = (int)(right * owner->scaleX);
    int bottom = field_0x3c[3];
    field_0x2c[3] = (int)(bottom * owner->scaleY);
    if (anchorControl && relAnchor) {
        field_0x2c[1] += anchorControl->field_0x2c[1];
        field_0x2c[3] += anchorControl->field_0x2c[1];
        field_0x2c[0] += anchorControl->field_0x2c[0];
        field_0x2c[2] += anchorControl->field_0x2c[0];
    } else {
        field_0x2c[1] += owner->screenArea.top;
        field_0x2c[3] += owner->screenArea.top;
        field_0x2c[0] += owner->screenArea.left;
        field_0x2c[2] += owner->screenArea.left;
    }
    field_0x4c[0] = field_0x2c[0] < 0 ? -field_0x2c[0] : 0;
    field_0x4c[1] = field_0x2c[1] < 0 ? -field_0x2c[1] : 0;
    int width = right - left;
    int height = bottom - top;
    int over = field_0x2c[2] - g_TrackGame->renderTarget->field_0x0c;
    int under = field_0x2c[3] - g_TrackGame->renderTarget->field_0x10;
    if (over > 0)
        width -= over;
    field_0x4c[2] = width;
    if (under > 0)
        height -= under;
    field_0x4c[3] = height;
}

// 0x00472e30
UIAnim::UIAnim(void* textures, void* palette) {
    animTextures = textures;
    currentFrame = 0;
    frameCount = 0;
    passesLeft = 0;
    field_0x18 = 0;
    lastStepTime = 0;
    playForwards = 1;
    frameDelay = 0;
    playBackwards = 0;
    animPalette = palette;
    memset(frameList, 0, sizeof(frameList));
}

// 0x00472fe0
UIFrame* UIAnim::Advance() {
    int now = UnknownFunction4bfa80();
    g_UnknownGlobal65b5c8 = now;
    if (abs(now - lastStepTime) > frameDelay || frameList[currentFrame]->isSound == 1) {
        lastStepTime = now;
        if (playBackwards) {
            if (!playForwards && currentFrame > 0)
                return frameList[--currentFrame];
            playForwards = 0;
            if (passesLeft == 0x7fff || (passesLeft && --field_0x18 > 0))
                return frameList[currentFrame = frameCount];
        } else {
            if (!playForwards && currentFrame + 1 < frameCount)
                return frameList[++currentFrame];
            playForwards = 0;
            if (passesLeft == 0x7fff || (passesLeft && --field_0x18 > 0))
                currentFrame = 0;
        }
    }
    return frameList[currentFrame];
}

// 0x004733a0
void UIButton::UnknownVirtualSlot49(int enable) {
    if (enabled == enable)
        return;
    enabled = enable;
    if (enable) {
        UnknownVirtualSlot29(0);
        SetFontColor(currentTextColor);
    } else {
        if (stateImages[4]) {
            savedState = currentState;
            UnknownVirtualSlot29(4);
        }
        currentTextColor = textColor;
        SetFontColor(0x999999);
    }
}

// 0x004749f0
void UIScrollCtl::UnknownVirtualSlot60(int value) {
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
            iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            for (control = UnknownFunction472750(iterator); control; control = UnknownFunction472750(iterator)) {
                if (control->controlType == 3)
                    ((UIListBox*)control)->ScrollBy(-1);
            }
            goto send;
        case 10:
            iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            for (control = UnknownFunction472750(iterator); control; control = UnknownFunction472750(iterator)) {
                if (control->controlType == 3)
                    ((UIListBox*)control)->ScrollBy(1);
            }
        send:
            UnknownFunction472730(iterator);
            event.kind = 0xe;
            event.code = eventCode;
            event.controlName = GetName();
            event.dialog = ownerDialog;
            event.gui = ownerGui;
            event.control = this;
            ownerDialog->UnknownVirtualSlot29(&event);
            break;
        }
        break;
    }
}

// 0x00475c70
UIListBox::UIListBox(int id, int rows, CameraRect* area, UnknownGameUiDialog* owner)
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
    field_0x24c = UnknownFunction477b60;
    field_0x218 = (int)this;
    autoSort = 0;
    field_0x220 = 0;
    selectable = 1;
    allowWScroll = 1;
    field_0x234 = 0;
    field_0x230 = 0;
    field_0x22c = 0;
    lastClickTime = 0;
    field_0x240 = 0;
    rowCount = 0;
    firstVisibleRow = 0;
    lastPageRowCount = 0;
    memset(rowTable, 0, rowCapacity * sizeof(UnknownGameUiListRow));
    field_0x244 = 1;
    int height = fontHeight;
    if (!height && (!ownerDialog || !ownerDialog->dialogFontHeight)) {
        visibleRowCount = 1;
    } else {
        int rowsShown = (field_0x2c[3] - field_0x2c[1]) / (height ? height : ownerDialog->dialogFontHeight);
        visibleRowCount = rowsShown < 1 ? 1 : rowsShown;
    }
}

// 0x00477110
int UIListBox::AddImageFileRow(const char* file, int data, int a, int b) {
    if (rowCount >= rowCapacity && !UnknownFunction476f50(rowCount + 1))
        return 0;
    UnknownGameUiListRow row;
    if (a) {
        UnknownTextureStream* stream =
            new(__FILE__, 0x1e20) UnknownTextureStream((int)g_UnknownResourceManager572b44);
        if (stream->UnknownFunction460f50(file, "r", 0)) {
            row.kind = 2;
            row.text = 0;
            row.imageFile = 0;
            row.textLines = 0;
            UIAnim* image = new(__FILE__, 0x1e27) UIAnim(ownerDialog->dialogTextures, 0);
            row.image = image;
            image->LoadFile(file, 0);
            UIFrame* frame = image->frameList[0];
            row.flags = b;
            row.width = frame->frameWidth;
            row.height = frame->frameHeight;
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
        UnknownTgaFile* header = UnknownFunction511d00(file, 0, (int)g_UnknownResourceManager572b44);
        if (!header)
            return 0;
        row.width = header->width;
        row.height = header->height;
        UnknownFunction512dd0(header);
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
    if (rowCount >= rowCapacity && !UnknownFunction476f50(rowCount + 1))
        return 0;
    UnknownGameUiListRow row;
    row.kind = 2;
    row.text = 0;
    row.image = image;
    image->AddRef();
    UIFrame* frame = image->frameList[0];
    row.flags = a;
    row.width = frame->frameWidth;
    row.height = frame->frameHeight;
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

// 0x00477800: the order 0x00477900 sorts by, then the linked lists' orders.
static int UnknownFunction477800(const void* a, const void* b) {
    if (!g_UnknownGlobal65b608->sortingList)
        return 0;
    int result = g_UnknownGlobal65b608->sortingList->field_0x218_control->field_0x24c(a, b);
    if (result)
        return result;
    int left = ((const UnknownGameUiListRow*)a)->sortIndex;
    int right = ((const UnknownGameUiListRow*)b)->sortIndex;
    GameObjectIterator* iterator =
        (GameObjectIterator*)g_UnknownGlobal65b608->sortingList->field_0x218_control->UnknownVirtualSlot53();
    UIListBox* other;
    while ((other = static_cast<UIListBox*>(
                g_UnknownGlobal65b608->sortingList->field_0x218_control->UnknownFunction472790(iterator))) != 0) {
        if (other->rowCount != g_UnknownGlobal65b608->sortingList->field_0x218_control->rowCount) {
            result = -1;
            break;
        }
        result = other->field_0x24c(&other->rowTable[left], &other->rowTable[right]);
        if (result)
            break;
    }
    g_UnknownGlobal65b608->sortingList->field_0x218_control->UnknownFunction472730(iterator);
    return result;
}

// 0x00477bc0
void UIListBox::UpdateScrollBars() {
    UnknownGameUiDialog* owner = ownerDialog;
    if (!owner)
        return;
    int height = 0;
    lastPageRowCount = 0;
    for (int i = rowCount - 1; i >= 0; i--) {
        height += rowTable[i].height;
        if (height > field_0x2c[3] - field_0x2c[1])
            break;
        lastPageRowCount++;
    }
    lastPageRowCount = lastPageRowCount < 0 ? 0 : lastPageRowCount;
    GameObjectIterator iterator(owner->controlContainer, 1, "UIControl");
    UIControl* control;
    while ((control = (UIControl*)iterator.Next()) != 0) {
        if (control->attachId == attachId && (control->controlType == 8 || control->controlType == 7))
            static_cast<UIScrollBar*>(control)->UnknownFunction475200(this);
    }
    UnknownVirtualSlot50();
}

// 0x00477e90
int UIListBox::UnknownVirtualSlot55(int a, int b) {
    POINT* point = (POINT*)b;
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && point) {
        if (a == 0) {
            if (selectable) {
                int found = 0;
                RECT rect = *(RECT*)field_0x2c;
                for (int row = firstVisibleRow; row < firstVisibleRow + visibleRowCount; row++) {
                    rect.bottom = rect.top + GetRowHeight(row);
                    if (PtInRect(&rect, *point) && !(rowTable[row].flags & 1)) {
                        SelectRow(row);
                        UnknownVirtualSlot66(&found);
                        break;
                    }
                    if (!found)
                        rect.top += GetRowHeight(row);
                }
                if (found)
                    return 1;
            }
        } else if (a == 1 && allowWScroll) {
            field_0x22c = point->x;
            return UIControl::UnknownVirtualSlot55(a, b);
        }
    }
    return UIControl::UnknownVirtualSlot55(a, b);
}

// ---------------------------------------------------------------------------
// Drawing, input and loading near misses

// cdecl 0x0047b490: whether `texture` has a pixel of colour `key`.
int UnknownFunction47b490(TextureMap* texture, int key) {
    int found = 0;
    unsigned short* bits = (unsigned short*)texture->UnknownVirtualSlot13(0, 0, 0x11);
    if (bits) {
        int red;
        int green;
        if (texture->field_0x20 == 0x235) {
            red = (key >> 8) & 0xf800;
            green = (key >> 5) & 0x7e0;
        } else if (texture->field_0x20 == 0x22b) {
            red = (key >> 9) & 0x7c00;
            green = (key >> 6) & 0x3e0;
        } else {
            goto unlock;
        }
        {
            int pixel = red | green | ((key >> 3) & 0x1f);
            if (pixel) {
                for (int rows = texture->field_0x18; rows > 0; rows--) {
                    for (int x = 0; x < texture->field_0x14; x++) {
                        if (bits[x] == pixel) {
                            found = 1;
                            break;
                        }
                    }
                    bits += texture->field_0x14;
                }
            }
        }
unlock:
        if (texture->UnknownVirtualSlot14(0))
            return found;
    }
    return 0;
}

// 0x00472960
UIFrame::UIFrame(const char* file, void* textures, int a, int b, void* palette) {
    if (_stricmp(file + strlen(file) - 4, ".wav")) {
        int format = g_TrackGame->renderTarget->field_0x28;
        ownsSound = 0;
        isSound = 0;
        frameSound = 0;
        field_0x20 = (int)textures;
        UnknownTgaFile* image = UnknownFunction5125c0(file, 0, (int)g_UnknownResourceManager572b44);
        if (image) {
            frameWidth = image->width;
            frameHeight = image->height;
            frameTexture = new(__FILE__, 0x1393) PCTextureMap((TextureMapManager*)field_0x20, 1);
            Palette8* pal = (Palette8*)palette;
            frameTexture->UnknownVirtualSlot4(image->bits, image->width, image->height, image->width,
                                            image->width, 0x22b, format,
                                            (UnknownTexturePalette*)(pal ? pal->field_0x708 : 0), 4,
                                            pal ? pal->field_0x70c : 0, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
            if (UnknownFunction47b490(frameTexture, 0xff00ff))
                frameTexture->UnknownVirtualSlot18(0xff00ff);
        } else {
            char message[100];
            frameTexture = 0;
            sprintf(message, "UIFrame(): Warning! Error loading frame: %s\n", file);
        }
        if (image)
            UnknownFunction512dd0(image);
    } else {
        frameSound = new(__FILE__, 0x13b9) Sound((SoundGroup*)a, 1);
        frameSound->UnknownFunction4bc320(file, 0, 1, 3, b, -1);
        ownsSound = 1;
        isSound = 1;
        frameWidth = 0;
        frameHeight = 0;
        frameTexture = 0;
    }
}

// 0x0046ef00: runs the timers and the joystick focus moves.
int UIDialog::UnknownVirtualSlot10(float frameTime) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (dialog->lastFrameTime != 0.0f)
        dialog->lastFrameTime = frameTime;
    else
        dialog->lastFrameTime = 0.001f;
    if (dialog->sendFrameEvent) {
        UnknownDialogEvent event;
        event.gui = dialog->guiManager;
        event.handled = 0;
        event.kind = kDialogFrame;
        event.controlName = 0;
        event.dialog = this;
        event.control = 0;
        dialog->UnknownVirtualSlot29(&event);
        dialog->sendFrameEvent = 0;
    }
    if (!field_0x25_bit2 && !dialog->isClosing) {
        UITimer* timer;
        for (int i = 0; (timer = dialog->timerList.Get(i)) != 0; i++) {
            timer->elapsedTime += (int)(frameTime * 1000.0f);
            if ((unsigned int)timer->elapsedTime >= (unsigned int)timer->periodTime) {
                timer->elapsedTime = 0;
                UIControl* control;
                if ((control = timer->targetControl) != 0) {
                    control->UnknownVirtualSlot60((int)timer);
                } else {
                    UnknownDialogEvent event;
                    event.code = timer->timerId;
                    event.gui = dialog->guiManager;
                    event.handled = 0;
                    event.kind = kDialogTimer;
                    event.controlName = 0;
                    event.dialog = this;
                    event.control = 0;
                    dialog->UnknownVirtualSlot29(&event);
                    if (event.handled)
                        return 1;
                }
            }
        }
        GUIUser* user = dialog->guiUser;
        UIControl* focus = (UIControl*)user->field_0x1d8;
        user->UnknownFunction487fb0(focus && focus->mouseAnim ? (UnknownCursorAnimation*)focus->mouseAnim
                                                                 : dialog->cursorAnimation);
        GUIInputDevice* device;
        int j = 0;
        while ((device = dialog->guiUser->UnknownFunction488310(j++)) != 0) {
            if (device->inputDevice->deviceKind == 2) {
                int x = device->pointerPosition.x;
                int y = device->pointerPosition.y;
                if (focus) {
                    if (x <= -1) {
                        if (dialog->joystickCentred) {
                            UIControl* next = focus->UnknownFunction4727c0();
                            dialog->joystickCentred = 0;
                            if (next)
                                UnknownFunction470000(next, 0, 0);
                        }
                    } else if (x >= 1) {
                        if (dialog->joystickCentred) {
                            UIControl* next = focus->UnknownFunction472810();
                            dialog->joystickCentred = 0;
                            if (next)
                                UnknownFunction470000(next, 0, 0);
                        }
                    } else if (y <= -1) {
                        if (dialog->joystickCentred)
                            dialog->joystickCentred = 0;
                    } else if (y >= 1) {
                        if (dialog->joystickCentred)
                            dialog->joystickCentred = 0;
                    } else {
                        dialog->joystickCentred = 1;
                    }
                }
            }
        }
        UnknownFunction46f120();
        GameObject::UnknownVirtualSlot10(dialog->lastFrameTime);
    }
    return 1;
}

// 0x004734c0: whether `point` is on an opaque pixel of the image.
int UIButton::UnknownVirtualSlot28(POINT point, int state) {
    int opaque = 0;
    int format = g_TrackGame->renderTarget->field_0x28;
    RECT rect = *(RECT*)field_0x2c;
    if (PtInRect(&rect, point)) {
        point.x -= rect.left;
        point.y -= rect.top;
        int x = (int)(point.x / ownerDialog->scaleX);
        int y = (int)(point.y / ownerDialog->scaleY);
        UnknownSurfaceDesc desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)UnknownVirtualSlot48(-1);
        if (!texture->systemSurface->Lock(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / UnknownFunction511970(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.field_0x28[0x18])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.field_0x28[0x18])
                    opaque = 1;
            }
            if (texture->systemSurface->Unlock(0))
                return opaque;
        }
        return opaque;
    }
    return 0;
}

// 0x00478e10: the same test as UIButton slot 28 (0x004734c0).
int UIMultiState::UnknownVirtualSlot28(POINT point, int state) {
    int opaque = 0;
    int format = g_TrackGame->renderTarget->field_0x28;
    RECT rect = *(RECT*)field_0x2c;
    if (PtInRect(&rect, point)) {
        point.x -= rect.left;
        point.y -= rect.top;
        int x = (int)(point.x / ownerDialog->scaleX);
        int y = (int)(point.y / ownerDialog->scaleY);
        UnknownSurfaceDesc desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)UnknownVirtualSlot48(-1);
        if (!texture->systemSurface->Lock(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / UnknownFunction511970(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.field_0x28[0x18])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.field_0x28[0x18])
                    opaque = 1;
            }
            if (texture->systemSurface->Unlock(0))
                return opaque;
        }
        return opaque;
    }
    return 0;
}

// 0x004738a0: draws the image, the text scrolled to keep the caret in view,
// and the caret.
int UIEditBox::UnknownVirtualSlot40() {
    int width = UnknownVirtualSlot61();
    int frames = -1;
    int redraw = redrawFrames;
    if (!controlText)
        SetEditText("");
    if (drawnTexture) {
        if (ownerDialog->dialogBackground) {
            ownerDialog->dialogBackground->UnknownFunction404480((PCTextureMap*)drawnTexture, (CameraRect*)field_0x2c,
                                                           field_0x4c,
                                                           drawnTexture->field_0x30 ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            frames = redrawFrames;
            redrawFrames = redraw;
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot3(field_0x2c, drawnTexture, field_0x4c,
                                                            drawnTexture->field_0x30 ? 0x1008000 : 0x1000000);
            redrawFrames = redraw;
        }
    } else {
        redrawFrames = redraw;
    }
    void* dc;
    int more;
    CameraRect rect;
    do {
        if (textAlign & 1) {
            int offset = width - textWidth;
            field_0x200 = offset > 0 ? 0 : offset;
        } else if (textAlign & 2) {
            if (UnknownVirtualSlot61() > textWidth)
                field_0x200 = width / 2 - textWidth / 2;
            else
                field_0x200 = width - textWidth;
        } else if (textAlign & 4) {
            field_0x200 = width - textWidth;
        } else {
            int offset = width - textWidth;
            field_0x200 = offset > 0 ? 0 : offset;
        }
        int x = field_0x2c[0] + field_0x200;
        int y;
        if (textAlign & 8)
            y = field_0x2c[1];
        else if (textAlign & 0x10)
            y = (field_0x2c[3] - field_0x2c[1]) / 2 - fontHeight / 2 + field_0x2c[1];
        else if (textAlign & 0x20)
            y = field_0x2c[3] - fontHeight;
        else
            y = field_0x2c[1];
        rect.left = x + textLeftMargin;
        rect.top = y + textTopMargin;
        rect.right = field_0x2c[2] - textLeftMargin;
        rect.bottom = field_0x2c[3] - textTopMargin;
        if (ownerDialog->BeginControlDraw(&dc, (CameraRect*)field_0x2c, &more, this)) {
            if (field_0xd0)
                ownerDialog->DrawAlignedText(dc, &rect, controlText, field_0xd0, textColor, fontHeight, 0,
                                                  textDrop, dropColor, 9, 1, 0);
            if (!more && ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
                int caret = rect.left + textWidth;
                int limit = ownerDialog->screenArea.left + field_0x2c[0] + width - 1;
                if (limit < caret)
                    caret = limit;
                rect.left = caret;
                rect.top++;
                rect.right = caret + 1;
                int height = fontHeight;
                if (!height)
                    height = ownerDialog->dialogFontHeight;
                rect.bottom = rect.top + height + 1;
                CameraRect* caretRect = (CameraRect*)&field_0x214;
                *caretRect = rect;
                if (backgroundBrush)
                    FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)backgroundBrush);
                else
                    FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
            }
            ownerDialog->EndControlDraw(dc);
            if (g_TrackGame->display->freezeFrameIndex &&
                ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
                if (ownerDialog && ownerDialog->dialogBackground)
                    ownerDialog->dialogBackground->UnknownFunction404c80();
                HIMC context = ImmGetContext((HWND)g_TrackGame->windowHandle);
                if (ImmGetOpenStatus(context)) {
                    COMPOSITIONFORM composition;
                    composition.dwStyle = CFS_FORCE_POSITION;
                    composition.ptCurrentPos.x = ((CameraRect*)&field_0x214)->left;
                    composition.ptCurrentPos.y = ((CameraRect*)&field_0x214)->top;
                    ImmSetCompositionWindow(context, &composition);
                }
                if (context)
                    ImmReleaseContext((HWND)g_TrackGame->windowHandle, context);
            }
        }
    } while (more);
    if (frames >= 0)
        redrawFrames = frames;
    if (ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this)
        UnknownVirtualSlot50();
    else
        needsRedraw = 1;
    return 1;
}

// 0x00474150: typed characters: Backspace, Enter (kind 10) and inserted
// characters, double-byte ones in two steps; then measures the text and
// sends kind 0x13.
int UIEditBox::UnknownVirtualSlot20(int value) {
    UnknownDialogEvent event;
    char c = (char)value;
    int result = 0;
    event.handled = 0;
    if (!UIControl::UnknownVirtualSlot20(value) &&
        ownerDialog->guiUser->focusControl == (UnknownGuiControl*)this) {
        if (field_0x204_sound) {
            field_0x204_sound->SetVolume(ownerGui->field_0x34c, 0);
            field_0x204_sound->PlayWithOptions(1, 0, 0);
        }
        switch (c) {
        case 8:
            if (field_0xd0 && textLength > 0) {
                char* previous = CharPrevA(controlText, controlText + textLength);
                if (previous) {
                    if (IsDBCSLeadByte(*previous)) {
                        memmove(previous, controlText + textLength, field_0xd0 - textLength + 1);
                        field_0xd0 -= 2;
                        textLength -= 2;
                    } else {
                        memmove(previous, controlText + textLength, field_0xd0 - textLength + 1);
                        field_0xd0--;
                        textLength--;
                    }
                }
            }
            break;
        case 9:
        case 0x1b:
            break;
        case 0xd:
            event.kind = kDialogEditDone;
            event.code = eventCode;
            event.controlName = GetName();
            event.dialog = ownerDialog;
            event.gui = ownerGui;
            event.control = this;
            ownerDialog->UnknownVirtualSlot29(&event);
            if (event.handled)
                return 1;
            break;
        default: {
            int lead = !field_0x228 && IsDBCSLeadByte(c) ? 1 : 0;
            if (!field_0x228 && !lead) {
                c = FilterCharacter(c);
                if (!c)
                    break;
            }
            field_0x228 = 0;
            int room;
            if (lead) {
                field_0x228 = 1;
                room = field_0xd0 < textCapacity - 2;
                field_0x22c = !room;
            } else if (field_0x22c) {
                field_0x22c = 0;
                room = 0;
            } else {
                room = field_0xd0 < textCapacity - 1;
            }
            if (room) {
                memmove(controlText + textLength + 1, controlText + textLength, field_0xd0 - textLength + 1);
                controlText[textLength] = c;
                textLength++;
                field_0xd0++;
            } else if (field_0x208_sound) {
                field_0x208_sound->SetVolume(ownerGui->field_0x34c, 0);
                field_0x208_sound->PlayWithOptions(1, 0, 0);
            }
            break;
        }
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
        redrawFrames++;
        event.kind = kDialogEditChange;
        event.code = eventCode;
        event.controlName = GetName();
        event.dialog = ownerDialog;
        event.gui = ownerGui;
        event.control = this;
        ownerDialog->UnknownVirtualSlot29(&event);
        result = 1;
    }
    return result;
}

// 0x00474880: a click scrolls the arrow's lists by a row, then repeats on a timer.
int UIScrollCtl::UnknownVirtualSlot55(int a, int b) {
    if (ownerDialog->guiUser->field_0x1d8 == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.handled = 0;
        GameObjectIterator* iterator;
        UIControl* control;
        if (controlType == 9) {
            iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            for (control = UnknownFunction472750(iterator); control; control = UnknownFunction472750(iterator)) {
                if (control->controlType == 3) {
                    ((UIListBox*)control)->lastClickTime = 0;
                    ((UIListBox*)control)->ScrollBy(-1);
                }
                if (control->controlType == 6) {
                    ((UIDropDownList*)control)->listPart->lastClickTime = 0;
                    ((UIDropDownList*)control)->listPart->ScrollBy(-1);
                }
            }
        } else {
            iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            for (control = UnknownFunction472750(iterator); control; control = UnknownFunction472750(iterator)) {
                if (control->controlType == 3) {
                    ((UIListBox*)control)->lastClickTime = 0;
                    ((UIListBox*)control)->ScrollBy(1);
                }
                if (control->controlType == 6) {
                    ((UIDropDownList*)control)->listPart->lastClickTime = 0;
                    ((UIDropDownList*)control)->listPart->ScrollBy(1);
                }
            }
        }
        UnknownFunction472730(iterator);
        event.code = eventCode;
        event.kind = 0xe;
        event.controlName = controlName;
        event.dialog = ownerDialog;
        event.control = this;
        event.gui = ownerDialog->guiManager;
        ownerDialog->UnknownVirtualSlot29(&event);
        ownerDialog->AddTimer(0x101, 0xfa, (int)this);
    }
    return UIControl::UnknownVirtualSlot55(a, b);
}

// 0x00479710: dragging the thumb; holding it still repeats kind 0x11.
void UIDDLScrollBar::UnknownVirtualSlot57(int a, int* position) {
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
            event.controlName = ownerList->GetName();
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
                event.controlName = ownerList->GetName();
                event.dialog = ownerDialog;
                event.gui = ownerGui;
                event.control = this;
                ownerDialog->UnknownVirtualSlot29(&event);
            } else {
                event.kind = 3;
                event.code = eventCode;
                event.controlName = ownerList->GetName();
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
                UIListBox* list = ownerList->listPart;
                if (list) {
                    int rows = list->rowCount - list->lastPageRowCount;
                    list->ScrollToRow(UnknownFunction475300(rows), 0);
                }
            }
        }
    }
}

// 0x00478570: draws the state's image, then its text.
int UIMultiState::UnknownVirtualSlot40() {
    int frames = -1;
    int redraw = redrawFrames;
    CameraRect rect;
    if (stateTable && drawnTexture) {
        TextureMap* texture = drawnTexture;
        rect = *(CameraRect*)field_0x2c;
        if (sourceBlit) {
            rect.right = (int)(texture->field_0x14 * ownerDialog->scaleX) + rect.left;
            rect.bottom = (int)(texture->field_0x18 * ownerDialog->scaleY) + rect.top;
        }
        if (ownerDialog->dialogBackground) {
            ownerDialog->dialogBackground->UnknownFunction404480((PCTextureMap*)texture, &rect, 0,
                                                           texture->field_0x30 ? 0x1008000 : 0x1000000,
                                                           backgroundRegion, needsRedraw, &redrawFrames, 0);
            frames = redrawFrames;
        } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, texture, 0,
                                                                     texture->field_0x30 ? 0x1008000 : 0x1000000)) {
            return 0;
        }
    }
    if ((stateTable && stateTable[selectedState].text && stateTable[selectedState].field_0x0c) ||
        (sourceBlit && controlText && field_0xd0)) {
        void* dc;
        int more;
        redrawFrames = redraw;
        do {
            if (ownerDialog->BeginControlDraw(&dc, (CameraRect*)field_0x2c, &more, this)) {
                if (sourceBlit) {
                    rect = *(CameraRect*)field_0x2c;
                    rect.left += stateTable[selectedState].image->GetCurrentFrame()->frameWidth +
                                 textLeftMargin;
                    rect.right -= textLeftMargin;
                    rect.top += textTopMargin;
                    rect.bottom -= textTopMargin;
                    ownerDialog->DrawAlignedText(dc, &rect, controlText, field_0xd0, textColor, fontHeight,
                                                      (UnknownGameUiTextRun*)textLines, textDrop, dropColor,
                                                      0x11, 1, 0);
                } else {
                    ownerDialog->DrawControlText(dc, this, 1);
                }
                ownerDialog->EndControlDraw(dc);
            }
        } while (more);
        if (frames >= 0)
            redrawFrames = frames;
    }
    return 1;
}

// 0x0047a400: names the parts after the list ("<name>_BUT", "_LB", "_SB",
// "_LBK", "_SBK", "_TBK") and lays them out below and beside the text.
void UIDropDownList::UnknownFunction47a400() {
    int left = field_0x2c[0];
    CameraRect full;
    full.left = 0;
    full.top = 0;
    full.right = field_0x2c[2] - left;
    int top = field_0x2c[1];
    full.bottom = field_0x2c[3] - top;
    CameraRect button;
    UIAnim* image = buttonPart->stateImages[0];
    if (image) {
        button = full;
        button.left = full.right + 1;
        button.right = image->frameList[0]->frameWidth + button.left;
    } else {
        button.left = 0;
        button.top = 0;
        button.right = 0;
        button.bottom = 0;
    }
    CameraRect list = full;
    list.top = full.bottom + 1;
    list.bottom = rowHeight + list.top;
    CameraRect scroll = list;
    scroll.top += 2;
    scroll.bottom -= 2;
    scroll.left = button.left;
    scroll.right = button.right;
    CameraRect* area = (CameraRect*)&field_0x20c;
    char name[50];
    area->left = left;
    area->top = top;
    area->right = left + scroll.right;
    area->bottom = top + scroll.bottom;
    sprintf(name, "%s_BUT", GetName());
    buttonPart->SetName(name);
    *(CameraRect*)buttonPart->field_0x2c = button;
    *(CameraRect*)buttonPart->field_0x3c = *(CameraRect*)buttonPart->field_0x2c;
    if (!button.left)
        buttonPart->UnknownVirtualSlot4();
    sprintf(name, "%s_LB", GetName());
    listPart->SetName(name);
    *(CameraRect*)listPart->field_0x2c = list;
    *(CameraRect*)listPart->field_0x3c = *(CameraRect*)listPart->field_0x2c;
    sprintf(name, "%s_SB", GetName());
    scrollBarPart->SetName(name);
    *(CameraRect*)scrollBarPart->field_0x2c = scroll;
    *(CameraRect*)scrollBarPart->field_0x3c = *(CameraRect*)scrollBarPart->field_0x2c;
    sprintf(name, "%s_LBK", GetName());
    staticPart0->SetName(name);
    *(CameraRect*)staticPart0->field_0x2c = list;
    *(CameraRect*)staticPart0->field_0x3c = *(CameraRect*)staticPart0->field_0x2c;
    sprintf(name, "%s_SBK", GetName());
    staticPart1->SetName(name);
    *(CameraRect*)staticPart1->field_0x2c = scroll;
    *(CameraRect*)staticPart1->field_0x3c = *(CameraRect*)staticPart1->field_0x2c;
    sprintf(name, "%s_TBK", GetName());
    staticPart2->SetName(name);
    *(CameraRect*)staticPart2->field_0x2c = full;
    *(CameraRect*)staticPart2->field_0x3c = *(CameraRect*)staticPart2->field_0x2c;
    listPart->textLeftMargin = textLeftMargin;
    listPart->textTopMargin = textTopMargin;
    static_cast<UIListBox*>(listPart)->SetRowTextColor(textColor, -1);
    if (textColor != 0xffffff)
        static_cast<UIListBox*>(listPart)->SetSelectColor(0xffffff);
    listPart->SetDropColor(dropColor);
}

// ---------------------------------------------------------------------------
// The dialog resource parser

// 0x0065b600: the last multi-state text the parser passed on.
char* g_UnknownGlobal65b600;

// 0x0046a920: reads the dialog resource at `offset` of `stream`. Its sections
// are listed between markers ("Set_Anim", "Set_Sound", "Set_Control") into
// the dialog's named entries (+0x9cc), then read in four passes: "Set_Info"
// (pass 0), the images (1), the sounds (2) and the controls with their
// "Set_Default" sections (3).
int UIDialog::UnknownFunction46a920(void* stream, int offset) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    char backgroundImage[0x80];
    backgroundImage[0] = 0;
    int depth = g_TrackGame->renderTarget->field_0x28;
    memset(dialog->sectionTable, 0, sizeof(dialog->sectionTable));
    int setType = 1;
    UnknownParameterBlock parameters;
    parameters.UnknownFunction4b77a0((UnknownParameterStream*)stream, offset, 1);
    char buffer[0x2800];
    parameters.UnknownFunction4b8240(buffer, sizeof(buffer));
    UnknownTokenizer* sections = new(__FILE__, 0x273) UnknownTokenizer(buffer);
    for (char* name = sections->UnknownFunction515df0("?"); name; name = sections->UnknownFunction515df0("?")) {
        if (!_strnicmp(name, "Set_Anim", strlen("Set_Anim"))) {
            setType = 1;
        } else if (!_strnicmp(name, "Set_Sound", strlen("Set_Sound"))) {
            setType = 2;
        } else if (!_strnicmp(name, "Set_Control", strlen("Set_Control"))) {
            setType = 4;
        } else {
            strcpy(dialog->sectionTable[dialog->sectionCount].sectionName, name);
            dialog->sectionTable[dialog->sectionCount].sectionObject = 0;
            if (!_strnicmp(name, "Set_Info", strlen("Set_Info")))
                dialog->sectionTable[dialog->sectionCount].field_0x38 = 0;
            else if (!_strnicmp(name, "Set_Default", strlen("Set_Default")))
                dialog->sectionTable[dialog->sectionCount].field_0x38 = 3;
            else
                dialog->sectionTable[dialog->sectionCount].field_0x38 = setType;
            dialog->sectionCount++;
        }
    }
    delete sections;

    // The "Set_Default" sections change these.
    int defaultMaxItems = 100;
    int defaultScale = 100;
    int defaultEnable = 1;
    int defaultShow = 1;
    int defaultSelectable = 1;
    int defaultTextColor = 0xffff;
    int defaultTextDrop = 1;
    int defaultColors[10];
    defaultColors[4] = 0xffff;
    int defaultSelectBoxColor = 0xff000000;
    int defaultItemBoxColor = 0xff000000;
    char defaultType[0x80];
    strcpy(defaultType, "");
    int defaultGroupId = 0;
    char defaultText[0x80];
    strcpy(defaultText, "");
    int defaultAttachId = 0;
    int defaultNumStates = 2;
    int defaultDefault = 0;
    int defaultAutoSort = 0;
    int defaultSelectColor = 0xff88ff;
    int defaultAllowWScroll = 0;
    int defaultTickNum = 0;
    int defaultFXDelay = 0;
    int defaultMaxLen = 1000;
    int defaultFontHeight = 0;
    int defaultDropColor = 0;
    int defaultKeyBind = 0;
    int defaultBold = 0;
    int defaultItalic = 0;
    int defaultSourceBlit = 0;
    int defaultPre3D = 0;
    int defaultPermanent = 0;
    int defaultShapeBounds = 0;
    int defaultListHeight = 20;
    int defaultRelAnchor = 0;
    int defaultMoveable = 0;
    defaultColors[0] = 0;
    defaultColors[1] = 0x8800;
    defaultColors[2] = 0x66ff;
    defaultColors[3] = 0xff6600;
    defaultColors[5] = 0xffffff;
    defaultColors[6] = 0xbbbbbb;
    defaultColors[7] = 0xbb55cc;
    defaultColors[8] = 0x880000;
    defaultColors[9] = 0x999999;
    char defaultFontName[0x80];
    strcpy(defaultFontName, "");
    char defaultMouseAnim[0x80];
    strcpy(defaultMouseAnim, "");
    char defaultSortLBName[0x80];
    strcpy(defaultSortLBName, "");
    char defaultAnchor[0x80];
    strcpy(defaultAnchor, "");
    char defaultTextAlign[0x80];
    strcpy(defaultTextAlign, "");
    char defaultTextAlignV[0x80];
    strcpy(defaultTextAlignV, "");
    char defaultFX[0x80];
    strcpy(defaultFX, "");
    char defaultFXSound[0x80];
    strcpy(defaultFXSound, "");
    char defaultFXAnimIn[0x80];
    strcpy(defaultFXAnimIn, "");
    char defaultFXAnimOut[0x80];
    strcpy(defaultFXAnimOut, "");
    char defaultFXSoundIn[0x80];
    strcpy(defaultFXSoundIn, "");
    char defaultFXSoundOut[0x80];
    strcpy(defaultFXSoundOut, "");
    char defaultAnimNorm[0x80];
    strcpy(defaultAnimNorm, "");
    char defaultAnimFocus[0x80];
    strcpy(defaultAnimFocus, "");
    char defaultAnimPush[0x80];
    strcpy(defaultAnimPush, "");
    char defaultAnimDisable[0x80];
    strcpy(defaultAnimDisable, "");
    char defaultAnimOff[0x80];
    strcpy(defaultAnimOff, "");
    char defaultAnimOn[0x80];
    strcpy(defaultAnimOn, "");
    char defaultSoundNorm[0x80];
    strcpy(defaultSoundNorm, "");
    char defaultSoundFocus[0x80];
    strcpy(defaultSoundFocus, "");
    char defaultSoundPush[0x80];
    strcpy(defaultSoundPush, "");
    char defaultSoundClick[0x80];
    strcpy(defaultSoundClick, "");
    char defaultSoundEnd[0x80];
    strcpy(defaultSoundEnd, "");
    char defaultAnimThumb[0x80];
    strcpy(defaultAnimThumb, "");
    char defaultAnimListBack[0x80];
    strcpy(defaultAnimListBack, "");
    char defaultAnimScrollBack[0x80];
    strcpy(defaultAnimScrollBack, "");
    char defaultAnimTextBack[0x80];
    strcpy(defaultAnimTextBack, "");
    char defaultStates[50][0x80];
    int n;
    for (n = 0; n < 50; n++)
        strcpy(defaultStates[n], "");
    char defaultStateTexts[50][0x80];
    for (n = 0; n < 50; n++)
        strcpy(defaultStateTexts[n], "");
    char defaultItems[50][0x80];
    for (n = 0; n < 50; n++)
        strcpy(defaultItems[n], "");

    // The current control's values (some keep the last control's).
    int value;
    int groupId;
    short item;
    CameraRect area;
    UIButton* button;
    UIMultiState* multiState;
    UIListBox* listBox;
    UIRadioButton* radioButton;
    UIStatic* staticControl;
    UIScrollBar* scrollBar;
    UIEditBox* editBox;
    UIStaticText* staticText;
    UIDropDownList* dropDownList;
    UIListBox* dropDownListBox;
    short state;
    int fontHeight;
    char type[0x80];
    UIControl* control;
    int isDefault;
    for (int pass = 0; pass < 4; pass++) {
        for (int i = 0; i < dialog->sectionCount; i++) {
            control = 0;
            isDefault = 0;
            parameters.UnknownFunction4b78f0(dialog->sectionTable[i].sectionName);
            switch (dialog->sectionTable[i].field_0x38) {
            case 0:
                if (pass == 0) {
                    parameters.UnknownFunction4b7f10("ScreenWidth", g_TrackGame->renderTarget->field_0x0c,
                                                     &dialog->screenWidth);
                    parameters.UnknownFunction4b7f10("ScreenHeight", g_TrackGame->renderTarget->field_0x10,
                                                     &dialog->screenHeight);
                    parameters.UnknownFunction4b7f10("NoScale", 0, &dialog->scaleToScreen);
                    dialog->scaleToScreen = !dialog->scaleToScreen;
                    parameters.UnknownFunction4b7f10("Popup", 1, &dialog->isPopup);
                    RenderTarget* target = (RenderTarget*)dialog->field_0x18;
                    if (dialog->scaleToScreen) {
                        dialog->scaleX = (float)target->field_0x0c / dialog->screenWidth;
                        dialog->scaleY = (float)target->field_0x10 / dialog->screenHeight;
                    } else if (dialog->screenWidth * 2 > target->field_0x0c ||
                               dialog->screenHeight * 2 > target->field_0x10) {
                        dialog->scaleX = dialog->scaleY = 1.0f;
                    }
                    if (dialog->isPopup) {
                        parameters.UnknownFunction4b7ec0("DlgAlignV", "MIDDLE", buffer, -1);
                        short align;
                        if (!_stricmp(buffer, "TOP"))
                            align = 8;
                        else
                            align = _stricmp(buffer, "BOTTOM") ? 0x10 : 0x20;
                        parameters.UnknownFunction4b7ec0("DlgAlignH", "CENTER", buffer, -1);
                        if (!_stricmp(buffer, "RIGHT"))
                            UnknownFunction46ffc0(align | 4);
                        else if (!_stricmp(buffer, "LEFT"))
                            UnknownFunction46ffc0(align | 1);
                        else
                            UnknownFunction46ffc0(align | 2);
                    } else {
                        dialog->popupAlignment = 9;
                    }
                    for (int color = 0; color < 10; color++) {
                        sprintf(buffer, "%s%d", "TextColor", color);
                        parameters.UnknownFunction4b7f10(buffer, defaultColors[color], &value);
                        dialog->textColors[color] = value;
                    }
                    parameters.UnknownFunction4b7ec0("MouseCursorAnim", "", buffer, -1);
                    if (buffer[0])
                        dialog->cursorAnimation = (UnknownCursorAnimation*)FindSectionObject(buffer);
                    parameters.UnknownFunction4b7ec0("BackgroundFile", "", buffer, -1);
                    if (buffer[0] && dialog->dialogBackground) {
                        UnknownTgaFile* file = UnknownFunction5125c0(buffer, 0, (int)g_UnknownResourceManager572b44);
                        if (file) {
                            dialog->ownsBackground = 1;
                            if (dialog->parentDialog)
                                dialog->parentBackground = dialog->dialogBackground->field_0x2c;
                            dialog->dialogBackground->field_0x2c =
                                new(__FILE__, 0x367) PCTextureMap((TextureMapManager*)dialog->dialogTextures, 1);
                            dialog->dialogBackground->field_0x2c->UnknownVirtualSlot4(
                                file->bits, file->width, file->height, file->width, file->width, 0x22b, depth,
                                dialog->dialogPalette ? (UnknownTexturePalette*)((Palette8*)dialog->dialogPalette)->field_0x708 : 0,
                                4, dialog->dialogPalette ? ((Palette8*)dialog->dialogPalette)->field_0x70c : 0, 0, 0, 2, 1, 0,
                                0x80, 0xff00ff);
                            dialog->dialogBackground->UnknownFunction404da0();
                            UnknownFunction512dd0(file);
                        }
                    } else if (dialog->parentDialog) {
                        dialog->dialogBackground = dialog->parentDialog->dialogBackground;
                    }
                    char fontName[0x80];
                    parameters.UnknownFunction4b7ec0("FontName", "", fontName, -1);
                    float points;
                    parameters.UnknownFunction4b7f40("PointSize", 0, &points);
                    int size = (int)points;
                    if (size < points)
                        size++;
                    parameters.UnknownFunction4b7f40("FontHeight", 0, &points);
                    if (points > 0) {
                        size = (int)points;
                        if (size < points)
                            size++;
                    }
                    if (fontName[0] && size) {
                        if (dialog->dialogFont)
                            DeleteObject((HGDIOBJ)dialog->dialogFont);
                        dialog->dialogFontHeight = (int)(size * dialog->scaleY);
                        strcpy(dialog->dialogFontFace, fontName);
                        int bold;
                        int italic;
                        parameters.UnknownFunction4b7f10("Bold", 0, &bold);
                        parameters.UnknownFunction4b7f10("Italic", 0, &italic);
                        const char* face = dialog->guiManager ? dialog->guiManager->toolTipFontFace : "";
                        if (*face) {
                            strcpy(dialog->dialogFontFace, face);
                            bold = dialog->guiManager->toolTipBold;
                            italic = dialog->guiManager->toolTipItalic;
                        }
                        dialog->dialogFontHeight = (int)((dialog->guiManager ? *(float*)&dialog->guiManager->field_0x3d0 : 0.0f) +
                                                   dialog->dialogFontHeight);
                        dialog->dialogFont = CreateFontA(dialog->dialogFontHeight, 0, 0, 0, bold ? FW_BOLD : FW_MEDIUM, italic,
                                                         0, 0, DEFAULT_CHARSET, 0, 0, 2, 2, dialog->dialogFontFace);
                        dialog->field_0x10c = italic;
                        dialog->field_0x108 = bold;
                    }
                    if (g_TrackGame->renderTarget->field_0x28 == 8) {
                        int load = 1;
                        if (dialog->guiManager) {
                            if (dialog->guiManager->UnknownFunction4864f0())
                                load = 0;
                            dialog->dialogPalette = dialog->guiManager->guiPalette;
                        }
                        parameters.UnknownFunction4b7ec0("PaletteFile", "", buffer, -1);
                        if (buffer[0] && load) {
                            UnknownResourceEntry* resource = g_UnknownResourceManager572b44->UnknownFunction4e9360(buffer, 1);
                            if (resource) {
                                dialog->dialogPalette = new(__FILE__, 0x3b6) Palette8(resource->field_0x14);
                                dialog->ownsPalette = 1;
                            }
                        }
                        if (dialog->guiManager && load)
                            dialog->guiManager->UnknownFunction486150((Palette8*)dialog->dialogPalette);
                    }
                    parameters.UnknownFunction4b7ec0("BackgroundImage", "", backgroundImage, -1);
                    if (backgroundImage[0]) {
                        CameraRect frameArea;
                        frameArea.left = 0;
                        frameArea.top = 0;
                        frameArea.right = dialog->screenArea.right - dialog->screenArea.left;
                        frameArea.bottom = dialog->screenArea.bottom - dialog->screenArea.top;
                        UIStatic* frame = new(__FILE__, 0x3cd) UIStatic(0, &frameArea, dialog);
                        frame->SetName("FRMBIStatic");
                        frame->permanent = 1;
                        dialog->controlContainer->AppendChild(frame, -1);
                        UnknownFunction46a840(frame, 0, 0);
                    }
                }
                break;
            case 1:
                if (pass == 1 && dialog->imageCount < 500) {
                    dialog->imageTable[dialog->imageCount] = new(__FILE__, 0x3df) UIAnim(dialog->dialogTextures, (void*)1);
                    int delay;
                    int loops;
                    parameters.UnknownFunction4b7f10("MSecDelay", 0, &delay);
                    parameters.UnknownFunction4b7f10("LoopCount", 0, &loops);
                    dialog->imageTable[dialog->imageCount]->SetFrameDelay(delay);
                    dialog->imageTable[dialog->imageCount]->SetFrameCount(loops);
                    parameters.UnknownFunction4b7ec0("Files1", "", buffer, -1);
                    UnknownTokenizer files(buffer);
                    for (char* file = files.UnknownFunction515df0(","); file; file = files.UnknownFunction515df0(",")) {
                        if (g_UnknownResourceManager572b44->UnknownFunction4e9360(dialog->sectionTable[i].sectionName, 1))
                            dialog->imageTable[dialog->imageCount]->LoadFromModule(
                                dialog->sectionTable[i].sectionName, (int)g_UnknownResourceManager572b44, (int)dialog->soundGroup,
                                dialog->dialogPalette);
                        else if (g_UnknownResourceManager572b44->UnknownFunction4e9360(file, 1))
                            dialog->imageTable[dialog->imageCount]->LoadFromModule(
                                file, (int)g_UnknownResourceManager572b44, (int)dialog->soundGroup, dialog->dialogPalette);
                        else
                            dialog->imageTable[dialog->imageCount]->LoadFile(file, dialog->dialogPalette);
                    }
                    dialog->sectionTable[i].sectionObject = (Sound*)dialog->imageTable[dialog->imageCount];
                    dialog->imageCount++;
                }
                break;
            case 2:
                if (pass == 2 && dialog->soundCount < 20) {
                    int isStatic;
                    int copies;
                    parameters.UnknownFunction4b7f10("Static", 1, &isStatic);
                    parameters.UnknownFunction4b7f10("Copies", 0, &copies);
                    parameters.UnknownFunction4b7ec0("File", "", buffer, -1);
                    int flags = isStatic ? 1 : 2;
                    if (g_UnknownResourceManager572b44->UnknownFunction4e9360(dialog->sectionTable[i].sectionName, 1)) {
                        dialog->soundTable[dialog->soundCount] =
                            UnknownFunction4bb890(dialog->soundGroup, dialog->sectionTable[i].sectionName, flags, 3, copies, -1);
                    } else if (g_UnknownResourceManager572b44->UnknownFunction4e9360(buffer, 1)) {
                        dialog->soundTable[dialog->soundCount] =
                            UnknownFunction4bb890(dialog->soundGroup, buffer, flags, 3, copies, -1);
                    } else {
                        dialog->soundTable[dialog->soundCount] = new(__FILE__, 0x41e) Sound(dialog->soundGroup, 1);
                        dialog->soundTable[dialog->soundCount]->UnknownFunction4bc320(buffer, 0, flags, 3, copies, -1);
                    }
                    dialog->sectionTable[i].sectionObject = dialog->soundTable[dialog->soundCount];
                    dialog->soundCount++;
                }
                break;
            case 3:
                if (pass != 3)
                    break;
                isDefault = 1;
            case 4: {
                if (pass != 3)
                    break;
                area.top = 0;
                area.left = 0;
                area.bottom = 0;
                area.right = 0;
                if (!isDefault) {
                    if (parameters.UnknownFunction4b7ec0("Xy", "", buffer, -1)) {
                        sscanf(buffer, "%d%*[,.] %d", &area.left, &area.top);
                        area.right = area.left;
                        area.bottom = area.top;
                    }
                    if (parameters.UnknownFunction4b7ec0("Xywh", "", buffer, -1)) {
                        sscanf(buffer, "%d%*[,.] %d%*[,.] %d%*[,.] %d", &area.left, &area.top, &area.right,
                               &area.bottom);
                        area.right += area.left;
                        area.bottom += area.top;
                    }
                    if (parameters.UnknownFunction4b7ec0("Xyxy", "", buffer, -1))
                        sscanf(buffer, "%d%*[,.] %d%*[,.] %d%*[,.] %d", &area.left, &area.top, &area.right,
                               &area.bottom);
                }
                parameters.UnknownFunction4b7f10("GroupId", defaultGroupId, isDefault ? &defaultGroupId : &groupId);
                parameters.UnknownFunction4b7ec0("Type", defaultType, isDefault ? defaultType : type, -1);

                if (!_stricmp(type, "BUTTON") || !_stricmp(type, "SCROLLUP") || !_stricmp(type, "SCROLLDOWN") ||
                    isDefault) {
                    if (!isDefault) {
                        if (!_stricmp(type, "BUTTON")) {
                            control = new(__FILE__, 0x45b) UIButton(0, &area, dialog);
                            button = (UIButton*)AddControl(control, groupId, 0);
                        } else if (!_stricmp(type, "SCROLLDOWN")) {
                            control = new(__FILE__, 0x45c) UIScrollCtl(10, 0, &area, dialog);
                            button = (UIButton*)AddControl(control, groupId, 0);
                        } else if (!_stricmp(type, "SCROLLUP")) {
                            control = new(__FILE__, 0x45d) UIScrollCtl(9, 0, &area, dialog);
                            button = (UIButton*)AddControl(control, groupId, 0);
                        }
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        button->SetNormalImage((UIAnim*)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("AnimFocus", defaultAnimFocus, isDefault ? defaultAnimFocus : buffer,
                                                     -1);
                    if (!isDefault)
                        button->SetFocusImage((UIAnim*)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("AnimPush", defaultAnimPush, isDefault ? defaultAnimPush : buffer, -1);
                    if (!isDefault)
                        button->SetPushImage((UIAnim*)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("AnimDisable", defaultAnimDisable,
                                                     isDefault ? defaultAnimDisable : buffer, -1);
                    if (!isDefault)
                        button->SetDisabledImage((UIAnim*)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("SortLBName", defaultSortLBName,
                                                     isDefault ? defaultSortLBName : buffer, -1);
                    if (!isDefault)
                        button->SetSortList((UIListBox*)FindSectionObject(buffer));
                }

                if (!_stricmp(type, "MULTISTATE") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x474) UIMultiState(0, &area, dialog);
                        multiState = (UIMultiState*)AddControl(control, groupId, 0);
                    }
                    int states;
                    parameters.UnknownFunction4b7f10("NumStates", defaultNumStates,
                                                     isDefault ? &defaultNumStates : &states);
                    if (!isDefault)
                        static_cast<UIMultiState*>(multiState)->SetStateCount(states);
                    int selected;
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &selected);
                    if (!isDefault)
                        static_cast<UIMultiState*>(multiState)->SetCurrentState(selected);
                    state = 0;
                    char stateKey[0x80];
                    char textKey[0x80];
                    char stateBuffer[0x80];
                    char textBuffer[0x80];
                    sprintf(stateKey, "%s%d", "State", state);
                    sprintf(textKey, "%s%d", "StateText", state);
                    int gotState = parameters.UnknownFunction4b7ec0(stateKey, defaultStates[0],
                                                                    isDefault ? defaultStates[0] : stateBuffer, -1);
                    int gotText = parameters.UnknownFunction4b7ec0(textKey, defaultStateTexts[0],
                                                                   isDefault ? defaultStateTexts[0] : textBuffer, -1);
                    while (gotState | gotText) {
                        if (!isDefault) {
                            g_UnknownGlobal65b600 = textBuffer[0] ? textBuffer : 0;
                            multiState->SetStateEntry(state, (UIAnim*)FindSectionObject(stateBuffer),
                                                              g_UnknownGlobal65b600);
                        }
                        state++;
                        // Retail tests the list items' counter here.
                        if (isDefault && item >= 50)
                            break;
                        sprintf(stateKey, "%s%d", "State", state);
                        sprintf(textKey, "%s%d", "StateText", state);
                        gotState = parameters.UnknownFunction4b7ec0(stateKey, state < 50 ? defaultStates[state] : "",
                                                                    isDefault ? defaultStates[state] : stateBuffer, -1);
                        gotText = parameters.UnknownFunction4b7ec0(textKey, state < 50 ? defaultStateTexts[state] : "",
                                                                   isDefault ? defaultStateTexts[state] : textBuffer, -1);
                    }
                }

                if (!_stricmp(type, "LISTBOX") || isDefault) {
                    parameters.UnknownFunction4b7f10("MaxItems", 100, isDefault ? &defaultMaxItems : &value);
                    if (!isDefault) {
                        control = new(__FILE__, 0x4a3) UIListBox(0, value, &area, dialog);
                        listBox = (UIListBox*)AddControl(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SelectRow(value);
                    parameters.UnknownFunction4b7f10("AutoSort", defaultDefault, isDefault ? &defaultAutoSort : &value);
                    if (!isDefault)
                        listBox->SetAutoSort(value);
                    parameters.UnknownFunction4b7f10("SelectColor", defaultSelectColor,
                                                     isDefault ? &defaultSelectColor : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetSelectColor(value);
                    parameters.UnknownFunction4b7f10("Selectable", defaultSelectable,
                                                     isDefault ? &defaultSelectable : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetSelectable(value);
                    parameters.UnknownFunction4b7f10("AllowWScroll", defaultAllowWScroll,
                                                     isDefault ? &defaultAllowWScroll : &value);
                    if (!isDefault)
                        listBox->SetAllowWScroll(value);
                    parameters.UnknownFunction4b7f10("SelectBoxColor", defaultSelectBoxColor,
                                                     isDefault ? &defaultSelectBoxColor : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetSelectBoxColor(value);
                    parameters.UnknownFunction4b7f10("ItemBoxColor", defaultItemBoxColor,
                                                     isDefault ? &defaultItemBoxColor : &value);
                    if (!isDefault)
                        static_cast<UIListBox*>(listBox)->SetItemBoxColor(value, -1);
                    char itemKey[0x80];
                    item = 1;
                    sprintf(itemKey, "%s%d", "Item", 1);
                    if (parameters.UnknownFunction4b7ec0(itemKey, defaultItems[1], isDefault ? defaultItems[1] : buffer,
                                                         -1)) {
                        do {
                            if (!isDefault) {
                                if (buffer[0] == '@')
                                    static_cast<UIListBox*>(listBox)->AddImageRow((UIAnim*)FindSectionObject(buffer + 1), 0, 0);
                                else
                                    static_cast<UIListBox*>(listBox)->AddRow(buffer, 0, 0);
                            }
                            item++;
                            if (isDefault && item >= 50)
                                break;
                            sprintf(itemKey, "%s%d", "Item", item);
                        } while (parameters.UnknownFunction4b7ec0(itemKey, item < 50 ? defaultItems[item] : "",
                                                                  isDefault ? defaultItems[item] : buffer, -1));
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault) {
                        listBox->SetImage(0, FindSectionObject(buffer));
                        listBox->SetImage(1, FindSectionObject(buffer));
                        listBox->SetImage(2, FindSectionObject(buffer));
                        listBox->SetImage(4, FindSectionObject(buffer));
                    }
                }

                if (!_stricmp(type, "RADIOBUTTON") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x4e3) UIRadioButton(0, &area, dialog);
                        radioButton = (UIRadioButton*)AddControl(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault)
                        static_cast<UIMultiState*>(radioButton)->SetCurrentState(value);
                    parameters.UnknownFunction4b7ec0("AnimOff", defaultAnimOff, isDefault ? defaultAnimOff : buffer, -1);
                    if (!isDefault)
                        radioButton->SetStateEntry(0, (UIAnim*)FindSectionObject(buffer), 0);
                    parameters.UnknownFunction4b7ec0("AnimOn", defaultAnimOn, isDefault ? defaultAnimOn : buffer, -1);
                    if (!isDefault)
                        radioButton->SetStateEntry(1, (UIAnim*)FindSectionObject(buffer), 0);
                }

                if (!_stricmp(type, "STATIC") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x4f3) UIStatic(0, &area, dialog);
                        staticControl = (UIStatic*)AddControl(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        staticControl->SetImage(0, FindSectionObject(buffer));
                }

                if (!_stricmp(type, "VSCROLLBAR") || !_stricmp(type, "HSCROLLBAR") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x500) UIScrollBar(7, 0, &area, dialog);
                        scrollBar = (UIScrollBar*)AddControl(control, groupId, 0);
                    }
                    if (!_stricmp(type, "HSCROLLBAR"))
                        control->controlType = 8;
                    int ticks;
                    int position;
                    int scale;
                    parameters.UnknownFunction4b7f10("TickNum", defaultTickNum, isDefault ? &defaultTickNum : &ticks);
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &position);
                    parameters.UnknownFunction4b7f10("Scale", defaultScale, isDefault ? &defaultScale : &scale);
                    if (!isDefault) {
                        static_cast<UIScrollBar*>(scrollBar)->UnknownFunction4754d0(ticks);
                        static_cast<UIScrollBar*>(scrollBar)->UnknownFunction4751c0(scale);
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        scrollBar->field_0x204_image = (UIAnim*)FindSectionObject(buffer);
                    parameters.UnknownFunction4b7ec0("AnimThumb", defaultAnimThumb, isDefault ? defaultAnimThumb : buffer,
                                                     -1);
                    if (!isDefault) {
                        scrollBar->SetImage(0, FindSectionObject(buffer));
                        scrollBar->SetImage(1, FindSectionObject(buffer));
                        scrollBar->SetImage(2, FindSectionObject(buffer));
                        scrollBar->SetImage(3, FindSectionObject(buffer));
                        if (control->field_0x3c[2] == control->field_0x3c[0]) {
                            UIAnim* image = control->stateImages[0];
                            control->field_0x3c[2] = image->frameList[0]->frameWidth + control->field_0x3c[0];
                            control->field_0x3c[3] = control->field_0x3c[1] + image->frameList[0]->frameHeight;
                        }
                        static_cast<UIScrollBar*>(scrollBar)->UnknownFunction4753c0(position, ticks ? ticks
                                                                         : control->field_0x3c[2] - control->field_0x3c[0] + 1);
                        scrollBar->UnknownFunction475160(0);
                    }
                }

                if (!_stricmp(type, "EDITBOX") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x52c) UIEditBox(0, &area, dialog, 100, 0, 0);
                        editBox = (UIEditBox*)AddControl(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7f10("MaxLen", defaultMaxLen, isDefault ? &defaultMaxLen : &value);
                    if (!isDefault)
                        static_cast<UIEditBox*>(editBox)->SetCapacity(value);
                    parameters.UnknownFunction4b7ec0("SoundClick", defaultSoundClick,
                                                     isDefault ? defaultSoundClick : buffer, -1);
                    if (!isDefault)
                        editBox->UnknownFunction473d80((int)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("SoundEnd", defaultSoundEnd, isDefault ? defaultSoundEnd : buffer, -1);
                    if (!isDefault)
                        editBox->UnknownFunction473d90((int)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault) {
                        editBox->SetImage(0, FindSectionObject(buffer));
                        editBox->SetImage(1, FindSectionObject(buffer));
                        editBox->SetImage(2, FindSectionObject(buffer));
                        editBox->SetImage(4, FindSectionObject(buffer));
                    }
                }

                if (!_stricmp(type, "STATICTEXT") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x545) UIStaticText(0, &area, dialog, 0, 0xffff);
                        staticText = (UIStaticText*)AddControl(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        staticText->SetImage(0, FindSectionObject(buffer));
                }

                if (!_strnicmp(type, "DROPDOWNLIST", 12) || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x54f) UIDropDownList(0, &area, dialog, 0, 0xffff);
                        dropDownList = (UIDropDownList*)AddControl(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        dropDownList->buttonPart->SetNormalImage((UIAnim*)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("AnimFocus", defaultAnimFocus, isDefault ? defaultAnimFocus : buffer,
                                                     -1);
                    if (!isDefault)
                        dropDownList->buttonPart->SetFocusImage((UIAnim*)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("AnimPush", defaultAnimPush, isDefault ? defaultAnimPush : buffer, -1);
                    if (!isDefault)
                        dropDownList->buttonPart->SetPushImage((UIAnim*)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("AnimThumb", defaultAnimThumb, isDefault ? defaultAnimThumb : buffer,
                                                     -1);
                    if (!isDefault) {
                        UIScrollBar* thumb = dropDownList->scrollBarPart;
                        thumb->SetImage(0, FindSectionObject(buffer));
                        thumb->SetImage(1, FindSectionObject(buffer));
                        thumb->SetImage(2, FindSectionObject(buffer));
                        thumb->SetImage(3, FindSectionObject(buffer));
                        thumb->UnknownFunction475160(0);
                    }
                    parameters.UnknownFunction4b7f10("ListHeight", defaultListHeight,
                                                     isDefault ? &defaultListHeight : &value);
                    if (!isDefault) {
                        dropDownList->UnknownFunction47a880(value);
                        dropDownListBox = dropDownList->listPart;
                    }
                    char listKey[0x80];
                    item = 1;
                    sprintf(listKey, "%s%d", "Item", 1);
                    if (parameters.UnknownFunction4b7ec0(listKey, defaultItems[1], isDefault ? defaultItems[1] : buffer,
                                                         -1)) {
                        do {
                            if (!isDefault)
                                static_cast<UIListBox*>(dropDownListBox)->AddRow(buffer, 0, 0);
                            item++;
                            if (isDefault && item >= 50)
                                break;
                            sprintf(listKey, "%s%d", "Item", item);
                        } while (parameters.UnknownFunction4b7ec0(listKey, item < 50 ? defaultItems[item] : "",
                                                                  isDefault ? defaultItems[item] : buffer, -1));
                    }
                    control->SetName(dialog->sectionTable[i].sectionName);
                    int leftMargin;
                    int topMargin;
                    int textColor;
                    int dropColor;
                    parameters.UnknownFunction4b7f10("TextLMargin", 0, &leftMargin);
                    parameters.UnknownFunction4b7f10("TextTMargin", 0, &topMargin);
                    parameters.UnknownFunction4b7f10("TextColor", defaultTextColor, &textColor);
                    parameters.UnknownFunction4b7f10("DropColor", defaultDropColor, &dropColor);
                    char soundNorm[0x80];
                    char soundFocus[0x80];
                    char soundPush[0x80];
                    char soundClick[0x80];
                    parameters.UnknownFunction4b7ec0("SoundNorm", defaultSoundNorm, soundNorm, -1);
                    parameters.UnknownFunction4b7ec0("SoundFocus", defaultSoundFocus, soundFocus, -1);
                    parameters.UnknownFunction4b7ec0("SoundPush", defaultSoundPush, soundPush, -1);
                    parameters.UnknownFunction4b7ec0("SoundClick", defaultSoundClick, soundClick, -1);
                    if (!isDefault) {
                        dropDownList->buttonPart->SetSound(0, FindSectionObject(soundNorm));
                        dropDownList->buttonPart->SetSound(1, FindSectionObject(soundFocus));
                        dropDownList->buttonPart->SetSound(2, FindSectionObject(soundPush));
                        dropDownList->buttonPart->SetSound(3, FindSectionObject(soundClick));
                        dropDownList->textLeftMargin = leftMargin;
                        dropDownList->textTopMargin = topMargin;
                        dropDownList->SetFontColor(textColor);
                        dropDownList->SetDropColor(dropColor);
                        dropDownList->UnknownFunction47a400();
                    }
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault) {
                        int row = value - 1 < 0 ? 0 : value - 1;
                        static_cast<UIListBox*>(dropDownListBox)->SelectRow(row);
                        dropDownList->SetText(static_cast<UIListBox*>(dropDownListBox)->GetRowText(row));
                    }
                    parameters.UnknownFunction4b7ec0("AnimTextBack", defaultAnimTextBack,
                                                     isDefault ? defaultAnimTextBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->UnknownFunction47ab20((UIAnim*)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("AnimListBack", defaultAnimListBack,
                                                     isDefault ? defaultAnimListBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->UnknownFunction47a970((UIAnim*)FindSectionObject(buffer));
                    parameters.UnknownFunction4b7ec0("AnimScrollBack", defaultAnimScrollBack,
                                                     isDefault ? defaultAnimScrollBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->UnknownFunction47aa40((UIAnim*)FindSectionObject(buffer));
                }

                // Every control's settings.
                char fontName[0x80];
                parameters.UnknownFunction4b7ec0("FontName", defaultFontName, fontName, -1);
                float height;
                parameters.UnknownFunction4b7f40("FontHeight", (float)defaultFontHeight, &height);
                int size = (int)height;
                if (size < height)
                    size++;
                if (isDefault)
                    defaultFontHeight = size;
                else
                    fontHeight = size;
                char anchor[0x80];
                int relAnchor;
                int moveable;
                char textAlign[0x80];
                char textAlignV[0x80];
                int textLMargin;
                int textTMargin;
                int fxDelay;
                char fx[0x80];
                char fxSound[0x80];
                char fxAnimIn[0x80];
                char fxSoundIn[0x80];
                char fxAnimOut[0x80];
                char fxSoundOut[0x80];
                char mouseAnim[0x80];
                char soundNorm[0x80];
                char soundFocus[0x80];
                char soundPush[0x80];
                char soundClick[0x80];
                char text[0x80];
                int attachId;
                int textColor;
                int dropColor;
                int keyBind;
                int italic;
                int bold;
                int textDrop;
                int sourceBlit;
                int enable;
                int show;
                int pre3D;
                int permanent;
                int shapeBounds;
                char toolTip[0x80];
                parameters.UnknownFunction4b7ec0("Anchor", defaultAnchor, isDefault ? defaultAnchor : anchor, -1);
                parameters.UnknownFunction4b7f10("RelAnchor", defaultRelAnchor, isDefault ? &defaultRelAnchor : &relAnchor);
                parameters.UnknownFunction4b7f10("Moveable", defaultMoveable, isDefault ? &defaultMoveable : &moveable);
                parameters.UnknownFunction4b7ec0("TextAlign", defaultTextAlign, isDefault ? defaultTextAlign : textAlign, -1);
                parameters.UnknownFunction4b7ec0("TextAlignV", defaultTextAlignV, isDefault ? defaultTextAlignV : textAlignV,
                                                 -1);
                parameters.UnknownFunction4b7f10("TextLMargin", 0, &textLMargin);
                parameters.UnknownFunction4b7f10("TextTMargin", 0, &textTMargin);
                parameters.UnknownFunction4b7f10("FXDelay", defaultFXDelay, isDefault ? &defaultFXDelay : &fxDelay);
                parameters.UnknownFunction4b7ec0("FX", defaultFX, isDefault ? defaultFX : fx, -1);
                parameters.UnknownFunction4b7ec0("FXSound", defaultFXSound, isDefault ? defaultFXSound : fxSound, -1);
                parameters.UnknownFunction4b7ec0("FXAnimIn", defaultFXAnimIn, isDefault ? defaultFXAnimIn : fxAnimIn, -1);
                parameters.UnknownFunction4b7ec0("FXSoundIn", defaultFXSoundIn, isDefault ? defaultFXSoundIn : fxSoundIn, -1);
                parameters.UnknownFunction4b7ec0("FXAnimOut", defaultFXAnimOut, isDefault ? defaultFXAnimOut : fxAnimOut, -1);
                parameters.UnknownFunction4b7ec0("FXSoundOut", defaultFXSoundOut, isDefault ? defaultFXSoundOut : fxSoundOut,
                                                 -1);
                parameters.UnknownFunction4b7ec0("MouseAnim", defaultMouseAnim, isDefault ? defaultMouseAnim : mouseAnim, -1);
                parameters.UnknownFunction4b7ec0("SoundNorm", defaultSoundNorm, isDefault ? defaultSoundNorm : soundNorm, -1);
                parameters.UnknownFunction4b7ec0("SoundFocus", defaultSoundFocus, isDefault ? defaultSoundFocus : soundFocus,
                                                 -1);
                parameters.UnknownFunction4b7ec0("SoundPush", defaultSoundPush, isDefault ? defaultSoundPush : soundPush, -1);
                parameters.UnknownFunction4b7ec0("SoundClick", defaultSoundClick, isDefault ? defaultSoundClick : soundClick,
                                                 -1);
                parameters.UnknownFunction4b7ec0("Text", defaultText, isDefault ? defaultText : text, -1);
                parameters.UnknownFunction4b7f10("AttachId", defaultAttachId, isDefault ? &defaultAttachId : &attachId);
                parameters.UnknownFunction4b7f10("TextColor", defaultTextColor, isDefault ? &defaultTextColor : &textColor);
                parameters.UnknownFunction4b7f10("DropColor", defaultDropColor, isDefault ? &defaultDropColor : &dropColor);
                parameters.UnknownFunction4b7f10("KeyBind", defaultKeyBind, isDefault ? &defaultKeyBind : &keyBind);
                parameters.UnknownFunction4b7f10("Italic", defaultItalic, isDefault ? &defaultItalic : &italic);
                parameters.UnknownFunction4b7f10("Bold", defaultBold, isDefault ? &defaultBold : &bold);
                parameters.UnknownFunction4b7f10("TextDrop", defaultTextDrop, isDefault ? &defaultTextDrop : &textDrop);
                parameters.UnknownFunction4b7f10("SourceBlit", defaultSourceBlit, isDefault ? &defaultSourceBlit : &sourceBlit);
                parameters.UnknownFunction4b7f10("Enable", defaultEnable, isDefault ? &defaultEnable : &enable);
                parameters.UnknownFunction4b7f10("Show", defaultShow, isDefault ? &defaultShow : &show);
                parameters.UnknownFunction4b7f10("Pre3D", defaultPre3D, isDefault ? &defaultPre3D : &pre3D);
                parameters.UnknownFunction4b7f10("Permanent", defaultPermanent, isDefault ? &defaultPermanent : &permanent);
                parameters.UnknownFunction4b7f10("ShapeBounds", defaultShapeBounds,
                                                 isDefault ? &defaultShapeBounds : &shapeBounds);
                parameters.UnknownFunction4b7ec0("ToolTipText", "", toolTip, -1);
                if (isDefault)
                    break;

                control->UnknownVirtualSlot52(attachId);
                control->groupId = groupId;
                control->SetAnchor((UIControl*)FindSectionObject(anchor), relAnchor);
                control->SetName(dialog->sectionTable[i].sectionName);
                control->moveable = moveable;
                if (control->field_0x3c[2] == control->field_0x3c[0]) {
                    UIAnim* image;
                    if (!_stricmp(type, "MULTISTATE") || !_stricmp(type, "RADIOBUTTON"))
                        image = ((UIMultiState*)control)->stateTable[((UIMultiState*)control)->selectedState].image;
                    else
                        image = control->stateImages[0];
                    control->field_0x3c[2] = image->frameList[0]->frameWidth + control->field_0x3c[0];
                    control->field_0x3c[3] = control->field_0x3c[1] + image->frameList[0]->frameHeight;
                }
                if (fontName[0] || (fontHeight && dialog->guiManager && dialog->guiManager->dialogFontName)) {
                    control->ownsFont = 1;
                    if (control->fontHandle)
                        DeleteObject((HGDIOBJ)control->fontHandle);
                    control->fontHandle = 0;
                    if (fontHeight)
                        control->fontHeight = fontHeight;
                    if (bold > 500)
                        bold = 1;
                    else if (bold > 1)
                        bold = 0;
                    const char* face = dialog->guiManager ? dialog->guiManager->toolTipFontFace : "";
                    if (*face) {
                        strcpy(control->fontFace, face);
                        bold = dialog->guiManager->toolTipBold;
                        italic = dialog->guiManager->toolTipItalic;
                    }
                    control->fontHeight = (int)((dialog->guiManager ? *(float*)&dialog->guiManager->field_0x3d0 : 0.0f) +
                                                 control->fontHeight);
                    if (control->fontHeight != dialog->dialogFontHeight || bold != dialog->field_0x108 ||
                        italic != dialog->field_0x10c) {
                        control->fontHeight = (int)(control->fontHeight * dialog->scaleY);
                        control->fontHandle = (int)CreateFontA(control->fontHeight, 0, 0, 0, bold ? FW_BOLD : FW_MEDIUM,
                                                                italic, 0, 0, DEFAULT_CHARSET, 0, 0, 2, 2, dialog->dialogFontFace);
                        strcpy(control->fontFace, dialog->dialogFontFace);
                        control->bold = bold;
                        control->italic = italic;
                    }
                }
                if (sourceBlit)
                    control->sourceBlit = 1;
                control->SetShapeBounds(shapeBounds);
                if (dialog->guiManager->languageModule) {
                    int textId;
                    int toolTipId;
                    parameters.UnknownFunction4b7f10("LocalTextId", 0, &textId);
                    parameters.UnknownFunction4b7f10("LocalToolTipId", 0, &toolTipId);
                    if (textId)
                        control->SetTextFromResource(dialog->guiManager->languageModule, textId);
                    if (toolTipId) {
                        char tip[0x400];
                        if (LoadStringA((HINSTANCE)dialog->guiManager->languageModule, toolTipId, tip, sizeof(tip))) {
                            if (control->toolTipText)
                                DebugFree(control->toolTipText, __FILE__, 0x646);
                            control->toolTipText = (char*)DebugMalloc(strlen(tip) + 1, __FILE__, 0x647);
                            strcpy(control->toolTipText, tip);
                        } else {
                            sprintf(tip, "Resource string '%d' load fail\n", toolTipId);
                            if (control->toolTipText)
                                DebugFree(control->toolTipText, __FILE__, 0x64d);
                            control->toolTipText = 0;
                        }
                    }
                } else {
                    if (toolTip[0]) {
                        control->toolTipText = (char*)DebugMalloc(strlen(toolTip) + 1, __FILE__, 0x656);
                        strcpy(control->toolTipText, toolTip);
                    }
                    if (text[0])
                        control->SetText(text);
                }
                control->textLeftMargin = textLMargin;
                control->textTopMargin = textTMargin;
                control->SetTextDrop(textDrop != 0);
                control->SetFontColor(textColor);
                control->SetDropColor(dropColor);
                int align;
                if (!_stricmp(textAlignV, "TOP"))
                    align = 8;
                else
                    align = _stricmp(textAlignV, "BOTTOM") ? 0x10 : 0x20;
                if (!_stricmp(textAlign, "RIGHT")) {
                    align |= 4;
                } else if (!_stricmp(textAlign, "CENTER")) {
                    align |= 2;
                } else {
                    switch (control->controlType) {
                    case 1:
                    case 2:
                    case 4:
                    case 5:
                    case 9:
                    case 10:
                        align |= 2;
                        break;
                    default:
                        align |= 1;
                        break;
                    }
                }
                control->SetTextAlign(align);
                if (!_stricmp(fx, "GROW")) {
                    control->StartTransition(100, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "LSLIDE")) {
                    control->StartTransition(101, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "TSLIDE")) {
                    control->StartTransition(103, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "RSLIDE")) {
                    control->StartTransition(102, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "BSLIDE")) {
                    control->StartTransition(104, (int)FindSectionObject(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "ANIM")) {
                    control->StartTransition(106, 0, 1, 0, -1);
                    control->fxAnimIn = (UIAnim*)FindSectionObject(fxAnimIn);
                    control->fxAnimOut = (UIAnim*)FindSectionObject(fxAnimOut);
                    control->fxSoundIn = FindSectionObject(fxSoundIn);
                    control->fxSoundOut = FindSectionObject(fxSoundOut);
                }
                control->SetSound(0, FindSectionObject(soundNorm));
                control->SetSound(1, FindSectionObject(soundFocus));
                control->SetSound(2, FindSectionObject(soundPush));
                control->SetSound(3, FindSectionObject(soundClick));
                control->UnknownVirtualSlot49(enable != 0);
                control->Show(show != 0, 1);
                control->mouseAnim = (int)FindSectionObject(mouseAnim);
                control->keyBind = keyBind;
                control->post3D = pre3D == 0;
                control->permanent = permanent;
                control->drawnTexture = control->UnknownVirtualSlot48(-1);
                control->UnknownVirtualSlot39();
                dialog->sectionTable[i].sectionObject = (Sound*)control;
                break;
            }
            }
        }
    }

    if (backgroundImage[0]) {
        UIControl* frame = FindControl("FRMBIStatic", 5);
        UIAnim* image = (UIAnim*)FindSectionObject(backgroundImage);
        if (image) {
            int width = image->GetCurrentTexture()->field_0x14;
            int imageHeight = image->GetCurrentTexture()->field_0x18;
            frame->SetImage(0, image);
            frame->field_0x2c[2] = width;
            frame->field_0x2c[3] = imageHeight;
            *(CameraRect*)frame->field_0x3c = *(CameraRect*)frame->field_0x2c;
            frame->drawnTexture = frame->UnknownVirtualSlot48(0);
            dialog->screenArea.right = dialog->screenArea.left + width;
            dialog->screenArea.bottom = dialog->screenArea.top + imageHeight;
            frame->UnknownVirtualSlot39();
        }
    }
    if (dialog->isPopup && dialog->screenWidth && dialog->screenHeight) {
        int align = dialog->popupAlignment;
        RenderTarget* target = (RenderTarget*)dialog->field_0x18;
        if (align & 1) {
            dialog->screenArea.right -= dialog->screenArea.left;
            dialog->screenArea.left = 0;
        } else if (align & 4) {
            int shift = target->field_0x0c - dialog->screenArea.left * 2 - dialog->screenArea.right;
            dialog->screenArea.left += shift;
            dialog->screenArea.right += shift;
        } else {
            int shift = (target->field_0x0c - dialog->screenArea.left - dialog->screenArea.right) / 2 -
                        dialog->screenArea.left;
            dialog->screenArea.left += shift;
            dialog->screenArea.right += shift;
        }
        if (align & 8) {
            dialog->screenArea.bottom -= dialog->screenArea.top;
            dialog->screenArea.top = 0;
        } else if (align & 0x20) {
            int shift = target->field_0x10 - dialog->screenArea.top * 2 - dialog->screenArea.bottom;
            dialog->screenArea.top += shift;
            dialog->screenArea.bottom += shift;
        } else {
            int shift = (target->field_0x10 - dialog->screenArea.top - dialog->screenArea.bottom) / 2 -
                        dialog->screenArea.top;
            dialog->screenArea.top += shift;
            dialog->screenArea.bottom += shift;
        }
    }
    return 1;
}

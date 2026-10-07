// Near misses for gameui.cpp (canonical file: src/reconstructed/GameUi.cpp,
// included below for its types and matched functions). Check: compile this
// file and compare each function with GameUiNearMisses.bindings.json.
//
// 0x0046e8c0 dialog slot 28 (55/64): retail swaps eax and edx.
// 0x0046ea80 (160/164): the two arguments are loaded into the opposite
//   registers.
// 0x0046ff70 (23/71): the same event stores; retail schedules them in field
//   order, VC6 here hoists `this` and the parent's GUI. Seven store orders were
//   tried.
// 0x00470170 UIControl constructor (535/578): retail places the owner == 0
//   block differently.
// 0x00470450 UIControl destructor (88/386): retail threads the tool-tip jumps
//   and keeps a value in edi.
// 0x004705d0 UIControl slot 49 (91/101): a register swap.
// 0x00470f10 UIControl slot 39 (226/332): a register permutation.
// 0x004715d0 UIControl slot 41, the zoom (635/638): retail loads +0x88 before
//   adding +0x80 for the right edge; both source orders give fld +0x80.
// 0x00472130 UIControl slot 22 (82/113): block layout.
// 0x00472e30 UIAnim constructor (34/82): store order.
// 0x00472fe0 UIAnim advance (77/194): retail copies the index with `mov ecx,
//   eax`.
// 0x004733a0 UIButton slot 49 (85/99): a register swap.
// 0x004749f0 UIScrollCtl slot 60 (52/274): retail lets case 10 fall into the
//   shared tail and places case 9 after the return; switch, if/else and goto
//   forms all lay case 9 last.
// 0x004753c0 (165/262): the operand order of the double / unsigned division.
// 0x00475c70 UIListBox constructor (319/382): retail re-tests the row height
//   after the owner's font height test and reloads the owner for it; VC6
//   here threads both tests (five if/ternary forms tried).
// 0x004773a0 adds an image row (169/232): retail loads the frame's width
//   before its height and stores the row's +0x18 later; 40 store orders were
//   tried.
// 0x00477730 UIListBox scroll (188/190): the operand order of an `and`.
// 0x00477800 qsort compare (5/262).
// 0x00477bc0 (89/286): retail keeps zero in a different register.
// 0x00477e90 UIListBox slot 55 (155/344): retail keeps the point in ebx and
//   the row offset in ebp; VC6 here swaps them.
// 0x00477ff0 UIListBox slot 56 (62/74): load order.
// 0x00478040 UIListBox slot 21, the arrow keys (185/324): for VK_END retail
//   computes the last row with `lea edi, [eax - 1]`, VC6 here with `mov edi,
//   ...; dec edi`.
// 0x00478810 UIMultiState slot 48 (13/66): register choice.
// 0x00479df0 UIDDLListBox slot 66 (104/120).
// 0x0047b490 colour-key test (116/210): retail keeps the key pixel in esi and
//   the row count in the key's argument slot.
// 0x00472960 UIFrame constructor from a file (377/562): retail re-tests the
//   loaded image before freeing it on both paths; VC6 here threads the test.
// 0x00472bc0 UIFrame image from a stream (295/315): the palette's two members
//   land in ecx/ebx swapped.
// 0x0046ef00 UIDialog slot 10 (352/555): the timer's +0x0c is cleared before
//   +0x14 is loaded, and the joystick branches sit after the epilogue.
// 0x004734c0 UIButton slot 28 (256/328) and 0x00478e10 UIMultiState slot 28
//   (28/328): retail keeps the result partly in edi, tests Unlock's result
//   and returns separately when Lock fails.
// 0x004738a0 UIEditBox slot 40 (206/975): retail keeps the width and the
//   redraw count in memory.
// 0x00474150 UIEditBox slot 20 (339/1048): Backspace: retail loads the
//   length before the lead-byte test.
// 0x00474880 UIScrollCtl slot 55 (254/355): retail pushes every register in
//   the prologue and keeps `this` in esi.
// 0x00479710 UIDDLScrollBar slot 57 (736/738): the list's +0x204 goes through
//   ebp instead of ecx.
// 0x00478570 UIMultiState slot 40 (139/682): retail keeps &+0x1bc in ebx and
//   shares its spill slot with the DC.
// 0x0047a400 drop-down layout (320/960): store scheduling of the part rects.
// 0x0046a920 UIDialog's resource parser (16 KB; 92% of instructions equal
//   with stack offsets and relocations ignored, 74% raw): the control flow,
//   calls and constants follow retail. Retail keeps the current control in
//   ebx across the section loop and the pass counter in memory; VC6 here
//   gives ebx to the pass counter (and to the state/item counters), so the
//   control is spilled. Its frame is 0x80 bytes smaller (retail allocates a
//   0x80-byte buffer at frame +0x2174 that no instruction reads), which
//   shifts most stack offsets. The jump table makes ckm report an unresolved
//   $L label; compare with sdiff.

#include "../../src/reconstructed/GameUi.cpp"

#include <imm.h>

#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/Palette8.h"
#include "../../src/reconstructed/Parameterblocks.h"
#include "../../src/reconstructed/Tgafile.h"

// 0x0065b5c8: the time stamp of the last image step (UIAnim 0x00472fe0).
int g_UnknownGlobal65b5c8;
int g_UnknownGlobal65b60c;                    // frames of the zoom (slot 41)

// 0x0046e8c0
void UnknownGameUiDialog::UnknownVirtualSlot28(int value) {
    UnknownDialogEvent event;
    event.field_0x20 = 0;
    event.field_0x10 = field_0x30;
    event.field_0x00 = 0;
    event.field_0x04 = 0;
    event.field_0x14 = 0;
    event.field_0x18 = value;
    event.field_0x08 = 5;
    event.field_0x0c = this;
    UnknownVirtualSlot29(&event);
}

// 0x0046ea80
void UIDialog::UnknownFunction46ea80(int id, int value) {
    GameObjectIterator iterator(((UnknownGameUiDialog*)this)->field_0x7f3c, 1, "UIControl");
    UnknownGameUiControl* control;
    while ((control = (UnknownGameUiControl*)iterator.Next()) != 0) {
        if (control->field_0x78 == id)
            control->UnknownVirtualSlot49(value);
    }
}

// 0x0046ff70
void UIDialog::UnknownFunction46ff70(int a, int b) {
    UnknownGameUiDialog* parent = ((UnknownGameUiDialog*)this)->field_0x2c;
    if (parent) {
        UnknownDialogEvent event;
        event.field_0x08 = b;
        event.field_0x20 = 0;
        event.field_0x00 = a;
        event.field_0x04 = 0;
        event.field_0x0c = parent;
        event.field_0x14 = (UnknownGameUiControl*)this;
        event.field_0x10 = parent->field_0x30;
        parent->UnknownVirtualSlot29(&event);
    }
}

// 0x00470170
UnknownGameUiControl::UnknownGameUiControl(int type, int id, CameraRect* area, UnknownGameUiDialog* owner)
    : GameObject(1) {
    UnknownFunction469ce0(this);
    field_0x5c = type;
    field_0x74 = id;
    if (area) {
        *(CameraRect*)field_0x2c = *area;
        *(CameraRect*)field_0x3c = *area;
    } else {
        field_0x3c[0] = field_0x3c[1] = field_0x3c[2] = field_0x3c[3] = 0;
        *(CameraRect*)field_0x2c = *(CameraRect*)field_0x3c;
    }
    field_0x64 = 0;
    field_0x60 = 0;
    field_0x68 = 0;
    field_0x6c = 1;
    field_0x70 = 1;
    field_0x78 = 0;
    field_0x7c = 0;
    field_0xb8 = owner;
    field_0xbc = owner ? owner->field_0x30 : 0;
    field_0x18c = 0;
    field_0x190 = 0;
    field_0xa0 = 0;
    field_0xa4 = 0;
    field_0x1b4 = 0;
    field_0x1b8 = -1;
    field_0x1bc = 0;
    field_0xc0 = 0;
    field_0xf0 = 0;
    field_0xc4 = 0xffff;
    field_0xcc = 0xffff;
    field_0xd4 = 1;
    field_0xc8 = 0;
    field_0xd8 = 1;
    field_0xd0 = 0;
    field_0xe4 = 0;
    field_0x1c0 = 1;
    field_0x168 = 0;
    field_0x1d8 = 0;
    field_0x1c8 = 0;
    field_0x1c4 = 0;
    field_0x180 = 0;
    for (int i = 0; i < 5; i++) {
        field_0x16c[i] = 0;
        field_0x194[i] = 0;
    }
    field_0x164 = 0;
    field_0xac = 0;
    field_0xa8 = 0;
    field_0x1a8 = 0;
    field_0x188 = 0;
    field_0x184 = 0;
    field_0x1b0 = 0;
    field_0x1ac = 0;
    field_0xf4[0] = 0;
    field_0x12c = 0;
    field_0x128 = 0;
    field_0x130[0] = 0;
    field_0x160 = 0;
    field_0x158 = 0;
    field_0x15c = 0;
    field_0xec = 0;
    field_0xe8 = 0;
    field_0x1cc = 0;
    field_0x1d0 = 0;
    field_0xb4 = 0;
    field_0x1d4 = -1;
    if (owner) {
        field_0x1dc = owner->field_0x7f18;
        field_0x18 = owner->UnknownInlineField18();
    } else {
        field_0x1dc = 1;
    }
    field_0x1e0 = 0;
    field_0x1e8 = 0;
}

// 0x00470450
UnknownGameUiControl::~UnknownGameUiControl() {
    if (field_0xb8 && !g_UnknownGlobal56e26c->field_0x2d5_bit1) {
        field_0x25_bit3 = 1;
        ToolTip* tip = 0;
        if (field_0xb8->field_0x34) {
            if (field_0xb8->field_0x34->field_0x1d4 == (UnknownGuiControl*)this) {
                field_0xb8->field_0x34->UnknownFunction487bf0(0);
                tip = field_0xb8->field_0x34->field_0xc0;
            }
            if (field_0xb8->field_0x34 && field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this) {
                field_0xb8->UnknownFunction470000(0, 0, 1);
                tip = field_0xb8->field_0x34->field_0xc0;
            }
        }
        if (tip)
            tip->UnknownFunction486b80(0, 0, 1.0f);
    }
    if (field_0x1b8 != -1 && field_0xb8 && field_0xb8->field_0x110)
        field_0xb8->field_0x110->UnknownFunction404200(field_0x1b8);
    if (field_0xc0)
        operator delete(field_0xc0, __FILE__, 0xba3);
    if (field_0xf0)
        operator delete(field_0xf0, __FILE__, 0xba4);
    if (field_0x12c)
        DeleteObject((HGDIOBJ)field_0x128);
    if (field_0xe4)
        operator delete(field_0xe4, __FILE__, 0xba8);
    if (field_0x1c4)
        operator delete(field_0x1c4, __FILE__, 0xba9);
}

// 0x004705d0
void UnknownGameUiControl::UnknownVirtualSlot49(int enable) {
    if (field_0x6c == enable)
        return;
    field_0x6c = enable;
    if (enable) {
        UnknownVirtualSlot29(field_0x64);
        UnknownFunction470d40(field_0xcc);
    } else {
        if (field_0x16c[4]) {
            field_0x64 = field_0x60;
            UnknownVirtualSlot29(4);
        }
        field_0xcc = field_0xc4;
        UnknownFunction470d40(0x999999);
    }
}

// 0x00470f10
void UnknownGameUiControl::UnknownVirtualSlot39() {
    UnknownGameUiDialog* owner = field_0xb8;
    int left = field_0x3c[0];
    field_0x2c[0] = (int)(left * owner->field_0x9c4);
    int top = field_0x3c[1];
    field_0x2c[1] = (int)(top * owner->field_0x9c8);
    int right = field_0x3c[2];
    field_0x2c[2] = (int)(right * owner->field_0x9c4);
    int bottom = field_0x3c[3];
    field_0x2c[3] = (int)(bottom * owner->field_0x9c8);
    if (field_0x18c && field_0x190) {
        field_0x2c[1] += field_0x18c->field_0x2c[1];
        field_0x2c[3] += field_0x18c->field_0x2c[1];
        field_0x2c[0] += field_0x18c->field_0x2c[0];
        field_0x2c[2] += field_0x18c->field_0x2c[0];
    } else {
        field_0x2c[1] += owner->field_0x160.top;
        field_0x2c[3] += owner->field_0x160.top;
        field_0x2c[0] += owner->field_0x160.left;
        field_0x2c[2] += owner->field_0x160.left;
    }
    field_0x4c[0] = field_0x2c[0] < 0 ? -field_0x2c[0] : 0;
    field_0x4c[1] = field_0x2c[1] < 0 ? -field_0x2c[1] : 0;
    int width = right - left;
    int height = bottom - top;
    int over = field_0x2c[2] - g_UnknownGlobal56e26c->field_0x10->field_0x0c;
    int under = field_0x2c[3] - g_UnknownGlobal56e26c->field_0x10->field_0x10;
    if (over > 0)
        width -= over;
    field_0x4c[2] = width;
    if (under > 0)
        height -= under;
    field_0x4c[3] = height;
}

// 0x004715d0
int UnknownGameUiControl::UnknownVirtualSlot41() {
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
        if (field_0x1a8) {
            field_0x1a8->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
            field_0x1a8->UnknownFunction4bc6b0(1, 0, 0);
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
        field_0x2c[0] = (int)field_0x80;
        field_0x2c[1] = (int)field_0x84;
        field_0x2c[2] = (int)(field_0x80 + field_0x88);
        field_0x2c[3] = (int)(field_0x8c + field_0x84);
        switch (field_0x5c) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
            field_0x1cc = field_0x16c[field_0x60]->UnknownFunction472f90();
            field_0x164++;
            break;
        default:
            field_0x1cc = 0;
            return 1;
        }
    }
    return 0;
}

// 0x00472130
int UnknownGameUiControl::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry) {
    GUIInputDevice* device = field_0xb8->field_0x34->UnknownFunction4882a0(event);
    if (device) {
        POINT position = device->field_0xa4;
        if (event->kind && UnknownVirtualSlot56(event->control, (int*)&position))
            return 1;
    }
    return GameObject::UnknownVirtualSlot22(event, entry);
}

// 0x00472e30
UIAnim::UIAnim(void* textures, void* palette) {
    field_0xf0 = textures;
    field_0x08 = 0;
    field_0x10 = 0;
    field_0x14 = 0;
    field_0x18 = 0;
    field_0x20 = 0;
    field_0x0c = 1;
    field_0x1c = 0;
    field_0x24 = 0;
    field_0xf4 = palette;
    memset(field_0x28, 0, sizeof(field_0x28));
}

// 0x00472fe0
UIFrame* UIAnim::UnknownFunction472fe0() {
    int now = UnknownFunction4bfa80();
    g_UnknownGlobal65b5c8 = now;
    if (abs(now - field_0x20) > field_0x1c || field_0x28[field_0x08]->field_0x08 == 1) {
        field_0x20 = now;
        if (field_0x24) {
            if (!field_0x0c && field_0x08 > 0)
                return field_0x28[--field_0x08];
            field_0x0c = 0;
            if (field_0x14 == 0x7fff || (field_0x14 && --field_0x18 > 0))
                return field_0x28[field_0x08 = field_0x10];
        } else {
            if (!field_0x0c && field_0x08 + 1 < field_0x10)
                return field_0x28[++field_0x08];
            field_0x0c = 0;
            if (field_0x14 == 0x7fff || (field_0x14 && --field_0x18 > 0))
                field_0x08 = 0;
        }
    }
    return field_0x28[field_0x08];
}

// 0x004733a0
void UIButton::UnknownVirtualSlot49(int enable) {
    if (field_0x6c == enable)
        return;
    field_0x6c = enable;
    if (enable) {
        UnknownVirtualSlot29(0);
        UnknownFunction470d40(field_0xcc);
    } else {
        if (field_0x16c[4]) {
            field_0x64 = field_0x60;
            UnknownVirtualSlot29(4);
        }
        field_0xcc = field_0xc4;
        UnknownFunction470d40(0x999999);
    }
}

// 0x004749f0
void UIScrollCtl::UnknownVirtualSlot60(int value) {
    UITimer* timer = (UITimer*)value;
    UnknownDialogEvent event;
    event.field_0x20 = 0;
    GameObjectIterator* iterator;
    UnknownGameUiControl* control;
    switch (timer->field_0x08) {
    case 0x101:
        field_0xb8->UnknownFunction46fec0(timer);
        field_0xb8->UnknownFunction46fce0(0x102, 100, (int)this);
        break;
    case 0x102:
        switch (field_0x5c) {
        case 9:
            iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            for (control = UnknownFunction472750(iterator); control; control = UnknownFunction472750(iterator)) {
                if (control->field_0x5c == 3)
                    ((UIListBox*)control)->UnknownFunction477730(-1);
            }
            goto send;
        case 10:
            iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            for (control = UnknownFunction472750(iterator); control; control = UnknownFunction472750(iterator)) {
                if (control->field_0x5c == 3)
                    ((UIListBox*)control)->UnknownFunction477730(1);
            }
        send:
            UnknownFunction472730(iterator);
            event.field_0x08 = 0xe;
            event.field_0x00 = field_0x74;
            event.field_0x04 = UnknownFunction470df0();
            event.field_0x0c = field_0xb8;
            event.field_0x10 = field_0xbc;
            event.field_0x14 = this;
            field_0xb8->UnknownVirtualSlot29(&event);
            break;
        }
        break;
    }
}

// 0x004753c0
int UnknownGameUiControl::UnknownFunction4753c0(int value, int range) {
    unsigned int position = static_cast<UIScrollBar*>(this)->field_0x21c ? range - value : value;
    UnknownVirtualSlot50();
    if (!static_cast<UIScrollBar*>(this)->field_0x1f0 && UnknownFunction475300(range) == position)
        return 1;
    if (!range) {
        static_cast<UIScrollBar*>(this)->field_0x1ec_float = position;
        return 1;
    }
    unsigned int travel = field_0x5c == 8 ? UnknownVirtualSlot61() - static_cast<UIScrollBar*>(this)->field_0x1f4 : UnknownVirtualSlot62() - static_cast<UIScrollBar*>(this)->field_0x1f8;
    if ((unsigned int)range > 0) {
        float scaled = (double)(travel * position) / (unsigned int)range;
        static_cast<UIScrollBar*>(this)->field_0x1ec_float = scaled < travel ? scaled : travel;
        return 1;
    }
    return 0;
}

// 0x00475c70
UIListBox::UIListBox(int id, int rows, CameraRect* area, UnknownGameUiDialog* owner)
    : UnknownGameUiControl(3, id, area, owner) {
    field_0x210 = 0;
    field_0x20c = 0;
    field_0x1ec = 0;
    field_0x1f4 = 0;
    field_0x1f0 = 0;
    field_0x1fc = rows;
    field_0x204 = 0;
    field_0x208 = 0xffffff;
    field_0x214 = (UnknownGameUiListRow*)DebugMalloc(rows * sizeof(UnknownGameUiListRow), __FILE__, 0x1b50);
    field_0x24c = UnknownFunction477b60;
    field_0x218 = (int)this;
    field_0x21c = 0;
    field_0x220 = 0;
    field_0x224 = 1;
    field_0x228 = 1;
    field_0x234 = 0;
    field_0x230 = 0;
    field_0x22c = 0;
    field_0x23c = 0;
    field_0x240 = 0;
    field_0x1ec = 0;
    field_0x1f0 = 0;
    field_0x204 = 0;
    memset(field_0x214, 0, field_0x1fc * sizeof(UnknownGameUiListRow));
    field_0x244 = 1;
    int height = field_0x160;
    if (!height && (!field_0xb8 || !field_0xb8->field_0xdc)) {
        field_0x200 = 1;
    } else {
        int rowsShown = (field_0x2c[3] - field_0x2c[1]) / (height ? height : field_0xb8->field_0xdc);
        field_0x200 = rowsShown < 1 ? 1 : rowsShown;
    }
}

// 0x004773a0
int UnknownGameUiControl::UnknownFunction4773a0(UIAnim* image, int data, int a) {
    if (static_cast<UIListBox*>(this)->field_0x1ec >= static_cast<UIListBox*>(this)->field_0x1fc && !UnknownFunction476f50(static_cast<UIListBox*>(this)->field_0x1ec + 1))
        return 0;
    UnknownGameUiListRow row;
    row.field_0x00 = 2;
    row.field_0x14 = 0;
    row.field_0x20 = image;
    image->AddRef();
    UIFrame* frame = image->field_0x28[0];
    row.field_0x04 = a;
    row.field_0x08 = frame->field_0x0c;
    row.field_0x0c = frame->field_0x10;
    row.field_0x18 = 0;
    row.field_0x24 = 0;
    row.field_0x30 = 0;
    static_cast<UIListBox*>(this)->field_0x214[static_cast<UIListBox*>(this)->field_0x1ec] = row;
    UnknownFunction476930(static_cast<UIListBox*>(this)->field_0x1ec, data);
    static_cast<UIListBox*>(this)->field_0x1ec++;
    static_cast<UIListBox*>(this)->field_0x200 = UnknownFunction476ee0();
    UnknownFunction477bc0();
    if (static_cast<UIListBox*>(this)->field_0x21c)
        UnknownFunction477900(1);
    return 1;
}

// 0x00477730
void UIListBox::UnknownFunction477730(int delta) {
    int handled = 0;
    int count = field_0x1ec;
    if (!count)
        return;
    int first = field_0x1f0 += delta;
    if (field_0x244) {
        if (first < 0)
            field_0x1f0 = first + count;
        int index = field_0x1f0 % count;
        if (index < 0)
            index = -index;
        field_0x1f0 = index;
    } else {
        int last = count - field_0x200;
        if (last < first)
            first = last;
        field_0x1f0 = first < 0 ? 0 : first;
    }
    if (field_0x200 == 1) {
        UnknownVirtualSlot65(field_0x1f0);
        UnknownVirtualSlot66((int)&handled);
        if (handled)
            return;
    }
    UnknownVirtualSlot50();
    UnknownFunction477bc0();
}

// 0x00477800: the order 0x00477900 sorts by, then the linked lists' orders.
static int UnknownFunction477800(const void* a, const void* b) {
    if (!g_UnknownGlobal65b608->field_0x9bc)
        return 0;
    int result = g_UnknownGlobal65b608->field_0x9bc->field_0x218_control->field_0x24c(a, b);
    if (result)
        return result;
    int left = ((const UnknownGameUiListRow*)a)->field_0x10;
    int right = ((const UnknownGameUiListRow*)b)->field_0x10;
    GameObjectIterator* iterator =
        (GameObjectIterator*)g_UnknownGlobal65b608->field_0x9bc->field_0x218_control->UnknownVirtualSlot53();
    UIListBox* other;
    while ((other = static_cast<UIListBox*>(
                g_UnknownGlobal65b608->field_0x9bc->field_0x218_control->UnknownFunction472790(iterator))) != 0) {
        if (other->field_0x1ec != g_UnknownGlobal65b608->field_0x9bc->field_0x218_control->field_0x1ec) {
            result = -1;
            break;
        }
        result = other->field_0x24c(&other->field_0x214[left], &other->field_0x214[right]);
        if (result)
            break;
    }
    g_UnknownGlobal65b608->field_0x9bc->field_0x218_control->UnknownFunction472730(iterator);
    return result;
}

// 0x00477bc0
void UnknownGameUiControl::UnknownFunction477bc0() {
    UnknownGameUiDialog* owner = field_0xb8;
    if (!owner)
        return;
    int height = 0;
    static_cast<UIListBox*>(this)->field_0x204 = 0;
    for (int i = static_cast<UIListBox*>(this)->field_0x1ec - 1; i >= 0; i--) {
        height += static_cast<UIListBox*>(this)->field_0x214[i].field_0x0c;
        if (height > field_0x2c[3] - field_0x2c[1])
            break;
        static_cast<UIListBox*>(this)->field_0x204++;
    }
    static_cast<UIListBox*>(this)->field_0x204 = static_cast<UIListBox*>(this)->field_0x204 < 0 ? 0 : static_cast<UIListBox*>(this)->field_0x204;
    GameObjectIterator iterator(owner->field_0x7f3c, 1, "UIControl");
    UnknownGameUiControl* control;
    while ((control = (UnknownGameUiControl*)iterator.Next()) != 0) {
        if (control->field_0x7c == field_0x7c && (control->field_0x5c == 8 || control->field_0x5c == 7))
            control->UnknownFunction475200(this);
    }
    UnknownVirtualSlot50();
}

// 0x00477e90
int UIListBox::UnknownVirtualSlot55(int a, int b) {
    POINT* point = (POINT*)b;
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this && point) {
        if (a == 0) {
            if (field_0x224) {
                int found = 0;
                RECT rect = *(RECT*)field_0x2c;
                for (int row = field_0x1f0; row < field_0x1f0 + field_0x200; row++) {
                    rect.bottom = rect.top + UnknownFunction4769a0(row);
                    if (PtInRect(&rect, *point) && !(field_0x214[row].field_0x04 & 1)) {
                        UnknownFunction476a60(row);
                        UnknownVirtualSlot66((int)&found);
                        break;
                    }
                    if (!found)
                        rect.top += UnknownFunction4769a0(row);
                }
                if (found)
                    return 1;
            }
        } else if (a == 1 && field_0x228) {
            field_0x22c = point->x;
            return UnknownGameUiControl::UnknownVirtualSlot55(a, b);
        }
    }
    return UnknownGameUiControl::UnknownVirtualSlot55(a, b);
}

// 0x00477ff0
int UIListBox::UnknownVirtualSlot56(int a, int* position) {
    if ((UnknownGuiControl*)this == field_0xb8->field_0x34->field_0x1d8 && a == 1) {
        int delta = field_0x234;
        field_0x234 = 0;
        field_0x230 += delta;
    }
    return UnknownGameUiControl::UnknownVirtualSlot56(a, position);
}

// 0x00478040
int UIListBox::UnknownVirtualSlot21(int key) {
    if (!UnknownGameUiControl::UnknownVirtualSlot21(key) &&
        field_0xb8->field_0x34->field_0x1d4 == (UnknownGuiControl*)this && field_0x224) {
        int row;
        switch (key) {
        case VK_LEFT:
        case VK_UP:
            if (UnknownFunction476950() <= 0)
                return 0;
            UnknownFunction476a60(UnknownFunction476950() - 1);
            break;
        case VK_RIGHT:
        case VK_DOWN:
            if (UnknownFunction476950() >= field_0x1ec - 1)
                return 0;
            UnknownFunction476a60(UnknownFunction476950() + 1);
            break;
        case VK_HOME:
            if (UnknownFunction476950() <= 0)
                return 0;
            UnknownFunction476a60(0);
            break;
        case VK_END:
            row = field_0x1ec - 1;
            if (UnknownFunction476950() >= row)
                return 0;
            UnknownFunction476a60(row);
            break;
        case VK_PRIOR:
            row = UnknownFunction476950() - field_0x200;
            if (row < 0)
                row = 0;
            if (row == UnknownFunction476950())
                return 0;
            UnknownFunction476a60(row);
            break;
        case VK_NEXT:
            row = UnknownFunction476950() + field_0x200;
            if (row > field_0x1ec - 1)
                row = field_0x1ec - 1;
            if (row == UnknownFunction476950())
                return 0;
            UnknownFunction476a60(row);
            break;
        default:
            return 0;
        }
        int handled;
        UnknownVirtualSlot66((int)&handled);
        return 1;
    }
    return 0;
}

// 0x00478810
TextureMap* UIMultiState::UnknownVirtualSlot48(int state) {
    UIAnim* image;
    if ((field_0x60 == 4 && (image = field_0x1f4[field_0x1f0].field_0x04) != 0) ||
        (image = field_0x1f4[field_0x1f0].field_0x00) != 0)
        return image->UnknownFunction472f90();
    return 0;
}

// 0x00479df0
void UIDDLListBox::UnknownVirtualSlot66(int value) {
    UnknownDialogEvent event;
    event.field_0x20 = 0;
    field_0x250->UnknownFunction470b20(UnknownFunction476d20(UnknownFunction476950()));
    field_0x250->UnknownFunction47a2d0(0);
    event.field_0x08 = 2;
    event.field_0x00 = field_0x250->field_0x74;
    event.field_0x04 = field_0x250->UnknownFunction470df0();
    event.field_0x0c = field_0xb8;
    event.field_0x14 = this;
    event.field_0x10 = field_0xb8->field_0x30;
    field_0xb8->UnknownVirtualSlot29(&event);
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
        int format = g_UnknownGlobal56e26c->field_0x10->field_0x28;
        field_0x1c = 0;
        field_0x08 = 0;
        field_0x14 = 0;
        field_0x20 = (int)textures;
        UnknownTgaFile* image = UnknownFunction5125c0(file, 0, (int)g_UnknownResourceManager572b44);
        if (image) {
            field_0x0c = image->width;
            field_0x10 = image->height;
            field_0x18 = new(__FILE__, 0x1393) PCTextureMap((TextureMapManager*)field_0x20, 1);
            ColorMapper* mapper;
            UnknownPaletteInterface* surfacePalette;
            if (palette) {
                surfacePalette = ((Palette8*)palette)->field_0x70c;
                mapper = ((Palette8*)palette)->field_0x708;
            } else {
                surfacePalette = 0;
                mapper = 0;
            }
            field_0x18->UnknownVirtualSlot4(image->bits, image->width, image->height, image->width,
                                            image->width, 0x22b, format, (UnknownTexturePalette*)mapper, 4,
                                            surfacePalette, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
            if (UnknownFunction47b490(field_0x18, 0xff00ff))
                field_0x18->UnknownVirtualSlot18(0xff00ff);
        } else {
            char message[100];
            field_0x18 = 0;
            sprintf(message, "UIFrame(): Warning! Error loading frame: %s\n", file);
        }
        if (image)
            UnknownFunction512dd0(image);
    } else {
        field_0x14 = new(__FILE__, 0x13b9) Sound((SoundGroup*)a, 1);
        field_0x14->UnknownFunction4bc320(file, 0, 1, 3, b, -1);
        field_0x1c = 1;
        field_0x08 = 1;
        field_0x0c = 0;
        field_0x10 = 0;
        field_0x18 = 0;
    }
}

// 0x00472bc0
int UIFrame::UnknownFunction472bc0(void* stream, int offset, void* palette) {
    if (stream) {
        UnknownTgaFile* image = UnknownFunction511dd0((UnknownTextureStream*)stream, 0, offset);
        if (image) {
            int format = g_UnknownGlobal56e26c->field_0x10->field_0x28;
            field_0x0c = image->width;
            field_0x10 = image->height;
            field_0x18 = new(__FILE__, 0x13d6) PCTextureMap((TextureMapManager*)field_0x20, 1);
            ColorMapper* mapper;
            UnknownPaletteInterface* surfacePalette;
            if (palette) {
                surfacePalette = ((Palette8*)palette)->field_0x70c;
                mapper = ((Palette8*)palette)->field_0x708;
            } else {
                surfacePalette = 0;
                mapper = 0;
            }
            field_0x18->UnknownVirtualSlot4(image->bits, image->width, image->height, image->width,
                                            image->width, 0x22b, format, (UnknownTexturePalette*)mapper, 4,
                                            surfacePalette, 0, 0, 2, 1, 0, 0x80, 0xff00ff);
            if (UnknownFunction47b490(field_0x18, 0xff00ff))
                field_0x18->UnknownVirtualSlot18(0xff00ff);
            UnknownFunction512dd0(image);
            return 1;
        }
    }
    return 0;
}

// 0x0046ef00: runs the timers and the joystick focus moves.
int UIDialog::UnknownVirtualSlot10(float frameTime) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (dialog->field_0x9c0 != 0.0f)
        dialog->field_0x9c0 = frameTime;
    else
        dialog->field_0x9c0 = 0.001f;
    if (dialog->field_0x7f40) {
        UnknownDialogEvent event;
        event.field_0x10 = dialog->field_0x30;
        event.field_0x20 = 0;
        event.field_0x08 = 0x12;
        event.field_0x04 = 0;
        event.field_0x0c = this;
        event.field_0x14 = 0;
        dialog->UnknownVirtualSlot29(&event);
        dialog->field_0x7f40 = 0;
    }
    if (!field_0x25_bit2 && !dialog->field_0x7f50) {
        UITimer* timer;
        for (int i = 0; (timer = dialog->field_0x7f24.Get(i)) != 0; i++) {
            timer->field_0x0c += (int)(frameTime * 1000.0f);
            if ((unsigned int)timer->field_0x0c >= (unsigned int)timer->field_0x10) {
                timer->field_0x0c = 0;
                UnknownGameUiControl* control;
                if ((control = timer->field_0x14) != 0) {
                    control->UnknownVirtualSlot60((int)timer);
                } else {
                    UnknownDialogEvent event;
                    event.field_0x00 = timer->field_0x08;
                    event.field_0x10 = dialog->field_0x30;
                    event.field_0x20 = 0;
                    event.field_0x08 = 7;
                    event.field_0x04 = 0;
                    event.field_0x0c = this;
                    event.field_0x14 = 0;
                    dialog->UnknownVirtualSlot29(&event);
                    if (event.field_0x20)
                        return 1;
                }
            }
        }
        GUIUser* user = dialog->field_0x34;
        UnknownGameUiControl* focus = (UnknownGameUiControl*)user->field_0x1d8;
        user->UnknownFunction487fb0(focus && focus->field_0x180 ? (UnknownCursorAnimation*)focus->field_0x180
                                                                 : dialog->field_0x95c);
        GUIInputDevice* device;
        int j = 0;
        while ((device = dialog->field_0x34->UnknownFunction488310(j++)) != 0) {
            if (device->field_0xac->deviceKind == 2) {
                int x = device->field_0xa4.x;
                int y = device->field_0xa4.y;
                if (focus) {
                    if (x <= -1) {
                        if (dialog->field_0x7f1c) {
                            UnknownGameUiControl* next = focus->UnknownFunction4727c0();
                            dialog->field_0x7f1c = 0;
                            if (next)
                                UnknownFunction470000(next, 0, 0);
                        }
                    } else if (x >= 1) {
                        if (dialog->field_0x7f1c) {
                            UnknownGameUiControl* next = focus->UnknownFunction472810();
                            dialog->field_0x7f1c = 0;
                            if (next)
                                UnknownFunction470000(next, 0, 0);
                        }
                    } else if (y <= -1) {
                        if (dialog->field_0x7f1c)
                            dialog->field_0x7f1c = 0;
                    } else if (y >= 1) {
                        if (dialog->field_0x7f1c)
                            dialog->field_0x7f1c = 0;
                    } else {
                        dialog->field_0x7f1c = 1;
                    }
                }
            }
        }
        UnknownFunction46f120();
        GameObject::UnknownVirtualSlot10(dialog->field_0x9c0);
    }
    return 1;
}

// 0x004734c0: whether `point` is on an opaque pixel of the image.
int UIButton::UnknownVirtualSlot28(POINT point, int state) {
    int opaque = 0;
    int format = g_UnknownGlobal56e26c->field_0x10->field_0x28;
    RECT rect = *(RECT*)field_0x2c;
    if (PtInRect(&rect, point)) {
        point.x -= rect.left;
        point.y -= rect.top;
        int x = (int)(point.x / field_0xb8->field_0x9c4);
        int y = (int)(point.y / field_0xb8->field_0x9c8);
        UnknownSurfaceDesc desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)UnknownVirtualSlot48(-1);
        if (!texture->field_0x70->UnknownMethod25(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / UnknownFunction511970(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.field_0x28[0x18])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.field_0x28[0x18])
                    opaque = 1;
            }
            if (texture->field_0x70->UnknownMethod32(0))
                return opaque;
        }
        return opaque;
    }
    return 0;
}

// 0x00478e10: the same test as UIButton slot 28 (0x004734c0).
int UIMultiState::UnknownVirtualSlot28(POINT point, int state) {
    int opaque = 0;
    int format = g_UnknownGlobal56e26c->field_0x10->field_0x28;
    RECT rect = *(RECT*)field_0x2c;
    if (PtInRect(&rect, point)) {
        point.x -= rect.left;
        point.y -= rect.top;
        int x = (int)(point.x / field_0xb8->field_0x9c4);
        int y = (int)(point.y / field_0xb8->field_0x9c8);
        UnknownSurfaceDesc desc;
        desc.size = sizeof(desc);
        PCTextureMap* texture = (PCTextureMap*)UnknownVirtualSlot48(-1);
        if (!texture->field_0x70->UnknownMethod25(0, &desc, 1, 0)) {
            int offset = desc.pitch * y / UnknownFunction511970(format) + x;
            if (format == 8) {
                if (((unsigned char*)desc.surface)[offset] != *(unsigned char*)&desc.field_0x28[0x18])
                    opaque = 1;
            } else {
                if (((unsigned short*)desc.surface)[offset] != *(unsigned short*)&desc.field_0x28[0x18])
                    opaque = 1;
            }
            if (texture->field_0x70->UnknownMethod32(0))
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
    int redraw = field_0x1bc;
    if (!field_0xc0)
        UnknownFunction473da0("");
    if (field_0x1cc) {
        if (field_0xb8->field_0x110) {
            field_0xb8->field_0x110->UnknownFunction404480((PCTextureMap*)field_0x1cc, (CameraRect*)field_0x2c,
                                                           field_0x4c,
                                                           field_0x1cc->field_0x30 ? 0x1008000 : 0x1000000,
                                                           field_0x1b8, field_0x1c0, &field_0x1bc, 0);
            frames = field_0x1bc;
            field_0x1bc = redraw;
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot3(field_0x2c, field_0x1cc, field_0x4c,
                                                            field_0x1cc->field_0x30 ? 0x1008000 : 0x1000000);
            field_0x1bc = redraw;
        }
    } else {
        field_0x1bc = redraw;
    }
    void* dc;
    int more;
    CameraRect rect;
    do {
        if (field_0xd8 & 1) {
            int offset = width - field_0x1fc;
            field_0x200 = offset > 0 ? 0 : offset;
        } else if (field_0xd8 & 2) {
            if (UnknownVirtualSlot61() > field_0x1fc)
                field_0x200 = width / 2 - field_0x1fc / 2;
            else
                field_0x200 = width - field_0x1fc;
        } else if (field_0xd8 & 4) {
            field_0x200 = width - field_0x1fc;
        } else {
            int offset = width - field_0x1fc;
            field_0x200 = offset > 0 ? 0 : offset;
        }
        int x = field_0x2c[0] + field_0x200;
        int y;
        if (field_0xd8 & 8)
            y = field_0x2c[1];
        else if (field_0xd8 & 0x10)
            y = (field_0x2c[3] - field_0x2c[1]) / 2 - field_0x160 / 2 + field_0x2c[1];
        else if (field_0xd8 & 0x20)
            y = field_0x2c[3] - field_0x160;
        else
            y = field_0x2c[1];
        rect.left = x + field_0xe8;
        rect.top = y + field_0xec;
        rect.right = field_0x2c[2] - field_0xe8;
        rect.bottom = field_0x2c[3] - field_0xec;
        if (field_0xb8->UnknownFunction46ed70(&dc, (CameraRect*)field_0x2c, &more, this)) {
            if (field_0xd0)
                field_0xb8->UnknownFunction46f550(dc, &rect, field_0xc0, field_0xd0, field_0xc4, field_0x160, 0,
                                                  field_0xd4, field_0xc8, 9, 1, 0);
            if (!more && field_0xb8->field_0x34->field_0x1d4 == (UnknownGuiControl*)this) {
                int caret = rect.left + field_0x1fc;
                int limit = field_0xb8->field_0x160.left + field_0x2c[0] + width - 1;
                if (limit < caret)
                    caret = limit;
                rect.left = caret;
                rect.top++;
                rect.right = caret + 1;
                int height = field_0x160;
                if (!height)
                    height = field_0xb8->field_0xdc;
                rect.bottom = rect.top + height + 1;
                CameraRect* caretRect = (CameraRect*)&field_0x214;
                *caretRect = rect;
                if (field_0x20c)
                    FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)field_0x20c);
                else
                    FrameRect((HDC)dc, (RECT*)&rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
            }
            field_0xb8->UnknownFunction46eeb0(dc);
            if (g_UnknownGlobal56e26c->field_0x0c->field_0x6c &&
                field_0xb8->field_0x34->field_0x1d4 == (UnknownGuiControl*)this) {
                if (field_0xb8 && field_0xb8->field_0x110)
                    field_0xb8->field_0x110->UnknownFunction404c80();
                HIMC context = ImmGetContext((HWND)g_UnknownGlobal56e26c->field_0x31c);
                if (ImmGetOpenStatus(context)) {
                    COMPOSITIONFORM composition;
                    composition.dwStyle = CFS_FORCE_POSITION;
                    composition.ptCurrentPos.x = ((CameraRect*)&field_0x214)->left;
                    composition.ptCurrentPos.y = ((CameraRect*)&field_0x214)->top;
                    ImmSetCompositionWindow(context, &composition);
                }
                if (context)
                    ImmReleaseContext((HWND)g_UnknownGlobal56e26c->field_0x31c, context);
            }
        }
    } while (more);
    if (frames >= 0)
        field_0x1bc = frames;
    if (field_0xb8->field_0x34->field_0x1d4 == (UnknownGuiControl*)this)
        UnknownVirtualSlot50();
    else
        field_0x1c0 = 1;
    return 1;
}

// 0x00474150: typed characters: Backspace, Enter (kind 10) and inserted
// characters, double-byte ones in two steps; then measures the text and
// sends kind 0x13.
int UIEditBox::UnknownVirtualSlot20(int value) {
    UnknownDialogEvent event;
    char c = (char)value;
    int result = 0;
    event.field_0x20 = 0;
    if (!UnknownGameUiControl::UnknownVirtualSlot20(value) &&
        field_0xb8->field_0x34->field_0x1d4 == (UnknownGuiControl*)this) {
        if (field_0x204_sound) {
            field_0x204_sound->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
            field_0x204_sound->UnknownFunction4bc6b0(1, 0, 0);
        }
        switch (c) {
        case 8:
            if (field_0xd0 && field_0x1ec > 0) {
                char* previous = CharPrevA(field_0xc0, field_0xc0 + field_0x1ec);
                if (previous) {
                    if (IsDBCSLeadByte(*previous)) {
                        memmove(previous, field_0xc0 + field_0x1ec, field_0xd0 - field_0x1ec + 1);
                        field_0xd0 -= 2;
                        field_0x1ec -= 2;
                    } else {
                        memmove(previous, field_0xc0 + field_0x1ec, field_0xd0 - field_0x1ec + 1);
                        field_0xd0--;
                        field_0x1ec--;
                    }
                }
            }
            break;
        case 9:
        case 0x1b:
            break;
        case 0xd:
            event.field_0x08 = 10;
            event.field_0x00 = field_0x74;
            event.field_0x04 = UnknownFunction470df0();
            event.field_0x0c = field_0xb8;
            event.field_0x10 = field_0xbc;
            event.field_0x14 = this;
            field_0xb8->UnknownVirtualSlot29(&event);
            if (event.field_0x20)
                return 1;
            break;
        default: {
            int lead = !field_0x228 && IsDBCSLeadByte(c) ? 1 : 0;
            if (!field_0x228 && !lead) {
                c = UnknownFunction473fc0(c);
                if (!c)
                    break;
            }
            field_0x228 = 0;
            int room;
            if (lead) {
                field_0x228 = 1;
                room = field_0xd0 < field_0x1f0 - 2;
                field_0x22c = !room;
            } else if (field_0x22c) {
                field_0x22c = 0;
                room = 0;
            } else {
                room = field_0xd0 < field_0x1f0 - 1;
            }
            if (room) {
                memmove(field_0xc0 + field_0x1ec + 1, field_0xc0 + field_0x1ec, field_0xd0 - field_0x1ec + 1);
                field_0xc0[field_0x1ec] = c;
                field_0x1ec++;
                field_0xd0++;
            } else if (field_0x208_sound) {
                field_0x208_sound->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
                field_0x208_sound->UnknownFunction4bc6b0(1, 0, 0);
            }
            break;
        }
        }
        field_0xc0[field_0xd0] = 0;
        void* dc;
        if (!((PCRenderTarget*)field_0xb8->UnknownInlineField18())->field_0x48->UnknownMethod17(&dc)) {
            HGDIOBJ font = SelectObject((HDC)dc, (HGDIOBJ)(field_0x128 ? field_0x128 : (int)field_0xb8->field_0xd8));
            SIZE size;
            GetTextExtentPoint32A((HDC)dc, field_0xc0, field_0xd0, &size);
            field_0xdc[1] = size.cx;
            GetTextExtentPoint32A((HDC)dc, field_0xc0, field_0x1ec, &size);
            field_0x1fc = size.cx;
            SelectObject((HDC)dc, font);
            ((PCRenderTarget*)field_0xb8->UnknownInlineField18())->field_0x48->UnknownMethod26(dc);
        }
        UnknownVirtualSlot50();
        field_0x1bc++;
        event.field_0x08 = 0x13;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x10 = field_0xbc;
        event.field_0x14 = this;
        field_0xb8->UnknownVirtualSlot29(&event);
        result = 1;
    }
    return result;
}

// 0x00474880: a click scrolls the arrow's lists by a row, then repeats on a timer.
int UIScrollCtl::UnknownVirtualSlot55(int a, int b) {
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.field_0x20 = 0;
        GameObjectIterator* iterator;
        UnknownGameUiControl* control;
        if (field_0x5c == 9) {
            iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            for (control = UnknownFunction472750(iterator); control; control = UnknownFunction472750(iterator)) {
                if (control->field_0x5c == 3) {
                    ((UIListBox*)control)->field_0x23c = 0;
                    ((UIListBox*)control)->UnknownFunction477730(-1);
                }
                if (control->field_0x5c == 6) {
                    ((UIDropDownList*)control)->field_0x1fc->field_0x23c = 0;
                    ((UIListBox*)control->field_0x1fc)->UnknownFunction477730(-1);
                }
            }
        } else {
            iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            for (control = UnknownFunction472750(iterator); control; control = UnknownFunction472750(iterator)) {
                if (control->field_0x5c == 3) {
                    ((UIListBox*)control)->field_0x23c = 0;
                    ((UIListBox*)control)->UnknownFunction477730(1);
                }
                if (control->field_0x5c == 6) {
                    ((UIDropDownList*)control)->field_0x1fc->field_0x23c = 0;
                    ((UIListBox*)control->field_0x1fc)->UnknownFunction477730(1);
                }
            }
        }
        UnknownFunction472730(iterator);
        event.field_0x00 = field_0x74;
        event.field_0x08 = 0xe;
        event.field_0x04 = field_0xf4;
        event.field_0x0c = field_0xb8;
        event.field_0x14 = this;
        event.field_0x10 = field_0xb8->field_0x30;
        field_0xb8->UnknownVirtualSlot29(&event);
        field_0xb8->UnknownFunction46fce0(0x101, 0xfa, (int)this);
    }
    return UnknownGameUiControl::UnknownVirtualSlot55(a, b);
}

// 0x00479710: dragging the thumb; holding it still repeats kind 0x11.
void UIDDLScrollBar::UnknownVirtualSlot57(int a, int* position) {
    UnknownGameUiControl::UnknownVirtualSlot57(a, position);
    UnknownDialogEvent event;
    event.field_0x20 = 0;
    if (!a && position) {
        int width = UnknownVirtualSlot61();
        int height = UnknownVirtualSlot62();
        UnknownVirtualSlot50();
        if (field_0x5c == 8) {
            if (field_0x1f0)
                field_0x1ec_float = (float)UnknownMinInt(width - field_0x1f4,
                                                       UnknownMaxInt(position[0] - field_0x2c[0], 0));
            else
                field_0x1ec_float = (float)UnknownMinInt(width - field_0x1f4,
                                                       UnknownMaxInt(position[0] - field_0x2c[0] - field_0x1f4 / 2, 0));
        } else {
            if (field_0x1f0 != 0.0f)
                field_0x1ec_float = (float)UnknownMinInt(height - field_0x1f8,
                                                       UnknownMaxInt(position[1] - field_0x2c[1], 0));
            else
                field_0x1ec_float = (float)UnknownMinInt(height - field_0x1f8,
                                                       UnknownMaxInt(position[1] - field_0x2c[1] - field_0x1f8 / 2, 0));
        }
        if (field_0x1f0)
            UnknownFunction4753c0(UnknownFunction475300(field_0x1f0), field_0x1f0);
        if (!field_0x208) {
            event.field_0x08 = 0xf;
            event.field_0x00 = field_0x74;
            event.field_0x04 = field_0x220->UnknownFunction470df0();
            event.field_0x0c = field_0xb8;
            event.field_0x10 = field_0xbc;
            event.field_0x14 = this;
            field_0xb8->UnknownVirtualSlot29(&event);
            field_0x208 = 1;
        }
        if (!event.field_0x20) {
            if (position[0] == field_0x20c && position[1] == field_0x210) {
                field_0x218_float += field_0xb8->field_0x9c0;
                if (field_0x218_float < field_0x214)
                    goto scroll;
                event.field_0x08 = 0x11;
                event.field_0x00 = field_0x74;
                event.field_0x04 = field_0x220->UnknownFunction470df0();
                event.field_0x0c = field_0xb8;
                event.field_0x10 = field_0xbc;
                event.field_0x14 = this;
                field_0xb8->UnknownVirtualSlot29(&event);
            } else {
                event.field_0x08 = 3;
                event.field_0x00 = field_0x74;
                event.field_0x04 = field_0x220->UnknownFunction470df0();
                event.field_0x0c = field_0xb8;
                event.field_0x10 = field_0xbc;
                event.field_0x14 = this;
                field_0xb8->UnknownVirtualSlot29(&event);
                field_0x20c = position[0];
                field_0x210 = position[1];
                field_0x218_float = 0.0f;
            }
            if (!event.field_0x20) {
            scroll:
                UIListBox* list = field_0x220->field_0x1fc;
                if (list) {
                    int rows = list->field_0x1ec - list->field_0x204;
                    list->UnknownFunction476860(UnknownFunction475300(rows), 0);
                }
            }
        }
    }
}

// 0x00478570: draws the state's image, then its text.
int UIMultiState::UnknownVirtualSlot40() {
    int frames = -1;
    int redraw = field_0x1bc;
    CameraRect rect;
    if (field_0x1f4 && field_0x1cc) {
        TextureMap* texture = field_0x1cc;
        rect = *(CameraRect*)field_0x2c;
        if (field_0xa4) {
            rect.right = (int)(texture->field_0x14 * field_0xb8->field_0x9c4) + rect.left;
            rect.bottom = (int)(texture->field_0x18 * field_0xb8->field_0x9c8) + rect.top;
        }
        if (field_0xb8->field_0x110) {
            field_0xb8->field_0x110->UnknownFunction404480((PCTextureMap*)texture, &rect, 0,
                                                           texture->field_0x30 ? 0x1008000 : 0x1000000,
                                                           field_0x1b8, field_0x1c0, &field_0x1bc, 0);
            frames = field_0x1bc;
        } else if (!((RenderTarget*)field_0x18)->UnknownVirtualSlot3(&rect, texture, 0,
                                                                     texture->field_0x30 ? 0x1008000 : 0x1000000)) {
            return 0;
        }
    }
    if ((field_0x1f4 && field_0x1f4[field_0x1f0].field_0x08 && field_0x1f4[field_0x1f0].field_0x0c) ||
        (field_0xa4 && field_0xc0 && field_0xd0)) {
        void* dc;
        int more;
        field_0x1bc = redraw;
        do {
            if (field_0xb8->UnknownFunction46ed70(&dc, (CameraRect*)field_0x2c, &more, this)) {
                if (field_0xa4) {
                    rect = *(CameraRect*)field_0x2c;
                    rect.left += field_0x1f4[field_0x1f0].field_0x00->UnknownFunction472f80()->field_0x0c +
                                 field_0xe8;
                    rect.right -= field_0xe8;
                    rect.top += field_0xec;
                    rect.bottom -= field_0xec;
                    field_0xb8->UnknownFunction46f550(dc, &rect, field_0xc0, field_0xd0, field_0xc4, field_0x160,
                                                      (UnknownGameUiTextRun*)field_0xe4, field_0xd4, field_0xc8,
                                                      0x11, 1, 0);
                } else {
                    field_0xb8->UnknownFunction46f6c0(dc, this, 1);
                }
                field_0xb8->UnknownFunction46eeb0(dc);
            }
        } while (more);
        if (frames >= 0)
            field_0x1bc = frames;
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
    UIAnim* image = field_0x1ec->field_0x16c[0];
    if (image) {
        button = full;
        button.left = full.right + 1;
        button.right = image->field_0x28[0]->field_0x0c + button.left;
    } else {
        button.left = 0;
        button.top = 0;
        button.right = 0;
        button.bottom = 0;
    }
    CameraRect list = full;
    list.top = full.bottom + 1;
    list.bottom = field_0x204 + list.top;
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
    sprintf(name, "%s_BUT", UnknownFunction470df0());
    field_0x1ec->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1ec->field_0x2c = button;
    *(CameraRect*)field_0x1ec->field_0x3c = *(CameraRect*)field_0x1ec->field_0x2c;
    if (!button.left)
        field_0x1ec->UnknownVirtualSlot4();
    sprintf(name, "%s_LB", UnknownFunction470df0());
    field_0x1fc->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1fc->field_0x2c = list;
    *(CameraRect*)field_0x1fc->field_0x3c = *(CameraRect*)field_0x1fc->field_0x2c;
    sprintf(name, "%s_SB", UnknownFunction470df0());
    field_0x200->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x200->field_0x2c = scroll;
    *(CameraRect*)field_0x200->field_0x3c = *(CameraRect*)field_0x200->field_0x2c;
    sprintf(name, "%s_LBK", UnknownFunction470df0());
    field_0x1f0->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1f0->field_0x2c = list;
    *(CameraRect*)field_0x1f0->field_0x3c = *(CameraRect*)field_0x1f0->field_0x2c;
    sprintf(name, "%s_SBK", UnknownFunction470df0());
    field_0x1f4->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1f4->field_0x2c = scroll;
    *(CameraRect*)field_0x1f4->field_0x3c = *(CameraRect*)field_0x1f4->field_0x2c;
    sprintf(name, "%s_TBK", UnknownFunction470df0());
    field_0x1f8->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1f8->field_0x2c = full;
    *(CameraRect*)field_0x1f8->field_0x3c = *(CameraRect*)field_0x1f8->field_0x2c;
    field_0x1fc->field_0xe8 = field_0xe8;
    field_0x1fc->field_0xec = field_0xec;
    field_0x1fc->UnknownFunction476c70(field_0xc4, -1);
    if (field_0xc4 != 0xffffff)
        field_0x1fc->UnknownFunction476b80(0xffffff);
    field_0x1fc->UnknownFunction470d80(field_0xc8);
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
    int depth = g_UnknownGlobal56e26c->field_0x10->field_0x28;
    memset(dialog->field_0x9cc, 0, sizeof(dialog->field_0x9cc));
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
            strcpy(dialog->field_0x9cc[dialog->field_0x7efc].field_0x00, name);
            dialog->field_0x9cc[dialog->field_0x7efc].field_0x34 = 0;
            if (!_strnicmp(name, "Set_Info", strlen("Set_Info")))
                dialog->field_0x9cc[dialog->field_0x7efc].field_0x38 = 0;
            else if (!_strnicmp(name, "Set_Default", strlen("Set_Default")))
                dialog->field_0x9cc[dialog->field_0x7efc].field_0x38 = 3;
            else
                dialog->field_0x9cc[dialog->field_0x7efc].field_0x38 = setType;
            dialog->field_0x7efc++;
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
    UnknownGameUiControl* control;
    int isDefault;
    for (int pass = 0; pass < 4; pass++) {
        for (int i = 0; i < dialog->field_0x7efc; i++) {
            control = 0;
            isDefault = 0;
            parameters.UnknownFunction4b78f0(dialog->field_0x9cc[i].field_0x00);
            switch (dialog->field_0x9cc[i].field_0x38) {
            case 0:
                if (pass == 0) {
                    parameters.UnknownFunction4b7f10("ScreenWidth", g_UnknownGlobal56e26c->field_0x10->field_0x0c,
                                                     &dialog->field_0x174);
                    parameters.UnknownFunction4b7f10("ScreenHeight", g_UnknownGlobal56e26c->field_0x10->field_0x10,
                                                     &dialog->field_0x178);
                    parameters.UnknownFunction4b7f10("NoScale", 0, &dialog->field_0x158);
                    dialog->field_0x158 = !dialog->field_0x158;
                    parameters.UnknownFunction4b7f10("Popup", 1, &dialog->field_0x15c);
                    RenderTarget* target = (RenderTarget*)dialog->field_0x18;
                    if (dialog->field_0x158) {
                        dialog->field_0x9c4 = (float)target->field_0x0c / dialog->field_0x174;
                        dialog->field_0x9c8 = (float)target->field_0x10 / dialog->field_0x178;
                    } else if (dialog->field_0x174 * 2 > target->field_0x0c ||
                               dialog->field_0x178 * 2 > target->field_0x10) {
                        dialog->field_0x9c4 = dialog->field_0x9c8 = 1.0f;
                    }
                    if (dialog->field_0x15c) {
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
                        dialog->field_0x170 = 9;
                    }
                    for (int color = 0; color < 10; color++) {
                        sprintf(buffer, "%s%d", "TextColor", color);
                        parameters.UnknownFunction4b7f10(buffer, defaultColors[color], &value);
                        dialog->field_0x118[color] = value;
                    }
                    parameters.UnknownFunction4b7ec0("MouseCursorAnim", "", buffer, -1);
                    if (buffer[0])
                        dialog->field_0x95c = (UnknownCursorAnimation*)UnknownFunction46e9a0(buffer);
                    parameters.UnknownFunction4b7ec0("BackgroundFile", "", buffer, -1);
                    if (buffer[0] && dialog->field_0x110) {
                        UnknownTgaFile* file = UnknownFunction5125c0(buffer, 0, (int)g_UnknownResourceManager572b44);
                        if (file) {
                            dialog->field_0x7f10 = 1;
                            if (dialog->field_0x2c)
                                dialog->field_0x9b8 = dialog->field_0x110->field_0x2c;
                            dialog->field_0x110->field_0x2c =
                                new(__FILE__, 0x367) PCTextureMap((TextureMapManager*)dialog->field_0x7f20, 1);
                            dialog->field_0x110->field_0x2c->UnknownVirtualSlot4(
                                file->bits, file->width, file->height, file->width, file->width, 0x22b, depth,
                                dialog->field_0x140 ? (UnknownTexturePalette*)((Palette8*)dialog->field_0x140)->field_0x708 : 0,
                                4, dialog->field_0x140 ? ((Palette8*)dialog->field_0x140)->field_0x70c : 0, 0, 0, 2, 1, 0,
                                0x80, 0xff00ff);
                            dialog->field_0x110->UnknownFunction404da0();
                            UnknownFunction512dd0(file);
                        }
                    } else if (dialog->field_0x2c) {
                        dialog->field_0x110 = dialog->field_0x2c->field_0x110;
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
                        if (dialog->field_0xd8)
                            DeleteObject((HGDIOBJ)dialog->field_0xd8);
                        dialog->field_0xdc = (int)(size * dialog->field_0x9c8);
                        strcpy(dialog->field_0xe0, fontName);
                        int bold;
                        int italic;
                        parameters.UnknownFunction4b7f10("Bold", 0, &bold);
                        parameters.UnknownFunction4b7f10("Italic", 0, &italic);
                        const char* face = dialog->field_0x30 ? dialog->field_0x30->field_0x350 : "";
                        if (*face) {
                            strcpy(dialog->field_0xe0, face);
                            bold = dialog->field_0x30->field_0x3d4;
                            italic = dialog->field_0x30->field_0x3d8;
                        }
                        dialog->field_0xdc = (int)((dialog->field_0x30 ? *(float*)&dialog->field_0x30->field_0x3d0 : 0.0f) +
                                                   dialog->field_0xdc);
                        dialog->field_0xd8 = CreateFontA(dialog->field_0xdc, 0, 0, 0, bold ? FW_BOLD : FW_MEDIUM, italic,
                                                         0, 0, DEFAULT_CHARSET, 0, 0, 2, 2, dialog->field_0xe0);
                        dialog->field_0x10c = italic;
                        dialog->field_0x108 = bold;
                    }
                    if (g_UnknownGlobal56e26c->field_0x10->field_0x28 == 8) {
                        int load = 1;
                        if (dialog->field_0x30) {
                            if (dialog->field_0x30->UnknownFunction4864f0())
                                load = 0;
                            dialog->field_0x140 = dialog->field_0x30->field_0x34;
                        }
                        parameters.UnknownFunction4b7ec0("PaletteFile", "", buffer, -1);
                        if (buffer[0] && load) {
                            UnknownResourceEntry* resource = g_UnknownResourceManager572b44->UnknownFunction4e9360(buffer, 1);
                            if (resource) {
                                dialog->field_0x140 = new(__FILE__, 0x3b6) Palette8(resource->field_0x14);
                                dialog->field_0x7f0c = 1;
                            }
                        }
                        if (dialog->field_0x30 && load)
                            dialog->field_0x30->UnknownFunction486150((Palette8*)dialog->field_0x140);
                    }
                    parameters.UnknownFunction4b7ec0("BackgroundImage", "", backgroundImage, -1);
                    if (backgroundImage[0]) {
                        CameraRect frameArea;
                        frameArea.left = 0;
                        frameArea.top = 0;
                        frameArea.right = dialog->field_0x160.right - dialog->field_0x160.left;
                        frameArea.bottom = dialog->field_0x160.bottom - dialog->field_0x160.top;
                        UIStatic* frame = new(__FILE__, 0x3cd) UIStatic(0, &frameArea, dialog);
                        frame->UnknownFunction470dc0("FRMBIStatic");
                        frame->field_0x1e0 = 1;
                        dialog->field_0x7f3c->UnknownFunction469190(frame, -1);
                        UnknownFunction46a840(frame, 0, 0);
                    }
                }
                break;
            case 1:
                if (pass == 1 && dialog->field_0x9b0 < 500) {
                    dialog->field_0x18c[dialog->field_0x9b0] = new(__FILE__, 0x3df) UIAnim(dialog->field_0x7f20, (void*)1);
                    int delay;
                    int loops;
                    parameters.UnknownFunction4b7f10("MSecDelay", 0, &delay);
                    parameters.UnknownFunction4b7f10("LoopCount", 0, &loops);
                    dialog->field_0x18c[dialog->field_0x9b0]->UnknownFunction472f40(delay);
                    dialog->field_0x18c[dialog->field_0x9b0]->UnknownFunction472f20(loops);
                    parameters.UnknownFunction4b7ec0("Files1", "", buffer, -1);
                    UnknownTokenizer files(buffer);
                    for (char* file = files.UnknownFunction515df0(","); file; file = files.UnknownFunction515df0(",")) {
                        if (g_UnknownResourceManager572b44->UnknownFunction4e9360(dialog->field_0x9cc[i].field_0x00, 1))
                            dialog->field_0x18c[dialog->field_0x9b0]->UnknownFunction473160(
                                dialog->field_0x9cc[i].field_0x00, (int)g_UnknownResourceManager572b44, (int)dialog->field_0x114,
                                dialog->field_0x140);
                        else if (g_UnknownResourceManager572b44->UnknownFunction4e9360(file, 1))
                            dialog->field_0x18c[dialog->field_0x9b0]->UnknownFunction473160(
                                file, (int)g_UnknownResourceManager572b44, (int)dialog->field_0x114, dialog->field_0x140);
                        else
                            dialog->field_0x18c[dialog->field_0x9b0]->UnknownFunction4730e0(file, dialog->field_0x140);
                    }
                    dialog->field_0x9cc[i].field_0x34 = (Sound*)dialog->field_0x18c[dialog->field_0x9b0];
                    dialog->field_0x9b0++;
                }
                break;
            case 2:
                if (pass == 2 && dialog->field_0x9b4 < 20) {
                    int isStatic;
                    int copies;
                    parameters.UnknownFunction4b7f10("Static", 1, &isStatic);
                    parameters.UnknownFunction4b7f10("Copies", 0, &copies);
                    parameters.UnknownFunction4b7ec0("File", "", buffer, -1);
                    int flags = isStatic ? 1 : 2;
                    if (g_UnknownResourceManager572b44->UnknownFunction4e9360(dialog->field_0x9cc[i].field_0x00, 1)) {
                        dialog->field_0x960[dialog->field_0x9b4] =
                            UnknownFunction4bb890(dialog->field_0x114, dialog->field_0x9cc[i].field_0x00, flags, 3, copies, -1);
                    } else if (g_UnknownResourceManager572b44->UnknownFunction4e9360(buffer, 1)) {
                        dialog->field_0x960[dialog->field_0x9b4] =
                            UnknownFunction4bb890(dialog->field_0x114, buffer, flags, 3, copies, -1);
                    } else {
                        dialog->field_0x960[dialog->field_0x9b4] = new(__FILE__, 0x41e) Sound(dialog->field_0x114, 1);
                        dialog->field_0x960[dialog->field_0x9b4]->UnknownFunction4bc320(buffer, 0, flags, 3, copies, -1);
                    }
                    dialog->field_0x9cc[i].field_0x34 = dialog->field_0x960[dialog->field_0x9b4];
                    dialog->field_0x9b4++;
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
                            button = (UIButton*)UnknownFunction46a8a0(control, groupId, 0);
                        } else if (!_stricmp(type, "SCROLLDOWN")) {
                            control = new(__FILE__, 0x45c) UIScrollCtl(10, 0, &area, dialog);
                            button = (UIButton*)UnknownFunction46a8a0(control, groupId, 0);
                        } else if (!_stricmp(type, "SCROLLUP")) {
                            control = new(__FILE__, 0x45d) UIScrollCtl(9, 0, &area, dialog);
                            button = (UIButton*)UnknownFunction46a8a0(control, groupId, 0);
                        }
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        button->UnknownFunction473310((UIAnim*)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("AnimFocus", defaultAnimFocus, isDefault ? defaultAnimFocus : buffer,
                                                     -1);
                    if (!isDefault)
                        button->UnknownFunction473330((UIAnim*)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("AnimPush", defaultAnimPush, isDefault ? defaultAnimPush : buffer, -1);
                    if (!isDefault)
                        button->UnknownFunction473350((UIAnim*)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("AnimDisable", defaultAnimDisable,
                                                     isDefault ? defaultAnimDisable : buffer, -1);
                    if (!isDefault)
                        button->UnknownFunction473370((UIAnim*)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("SortLBName", defaultSortLBName,
                                                     isDefault ? defaultSortLBName : buffer, -1);
                    if (!isDefault)
                        button->UnknownFunction473390((UnknownGameUiControl*)UnknownFunction46e9a0(buffer));
                }

                if (!_stricmp(type, "MULTISTATE") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x474) UIMultiState(0, &area, dialog);
                        multiState = (UIMultiState*)UnknownFunction46a8a0(control, groupId, 0);
                    }
                    int states;
                    parameters.UnknownFunction4b7f10("NumStates", defaultNumStates,
                                                     isDefault ? &defaultNumStates : &states);
                    if (!isDefault)
                        multiState->UnknownFunction478860(states);
                    int selected;
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &selected);
                    if (!isDefault)
                        multiState->UnknownFunction478cf0(selected);
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
                            multiState->UnknownFunction4789f0(state, (UIAnim*)UnknownFunction46e9a0(stateBuffer),
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
                        listBox = (UIListBox*)UnknownFunction46a8a0(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault)
                        listBox->UnknownFunction476a60(value);
                    parameters.UnknownFunction4b7f10("AutoSort", defaultDefault, isDefault ? &defaultAutoSort : &value);
                    if (!isDefault)
                        listBox->UnknownFunction477b90(value);
                    parameters.UnknownFunction4b7f10("SelectColor", defaultSelectColor,
                                                     isDefault ? &defaultSelectColor : &value);
                    if (!isDefault)
                        listBox->UnknownFunction476b80(value);
                    parameters.UnknownFunction4b7f10("Selectable", defaultSelectable,
                                                     isDefault ? &defaultSelectable : &value);
                    if (!isDefault)
                        listBox->UnknownFunction477bb0(value);
                    parameters.UnknownFunction4b7f10("AllowWScroll", defaultAllowWScroll,
                                                     isDefault ? &defaultAllowWScroll : &value);
                    if (!isDefault)
                        listBox->UnknownFunction477ba0(value);
                    parameters.UnknownFunction4b7f10("SelectBoxColor", defaultSelectBoxColor,
                                                     isDefault ? &defaultSelectBoxColor : &value);
                    if (!isDefault)
                        listBox->UnknownFunction476cd0(value);
                    parameters.UnknownFunction4b7f10("ItemBoxColor", defaultItemBoxColor,
                                                     isDefault ? &defaultItemBoxColor : &value);
                    if (!isDefault)
                        listBox->UnknownFunction476ba0(value, -1);
                    char itemKey[0x80];
                    item = 1;
                    sprintf(itemKey, "%s%d", "Item", 1);
                    if (parameters.UnknownFunction4b7ec0(itemKey, defaultItems[1], isDefault ? defaultItems[1] : buffer,
                                                         -1)) {
                        do {
                            if (!isDefault) {
                                if (buffer[0] == '@')
                                    listBox->UnknownFunction4773a0((UIAnim*)UnknownFunction46e9a0(buffer + 1), 0, 0);
                                else
                                    listBox->UnknownFunction476d80(buffer, 0, 0);
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
                        listBox->UnknownFunction470730(0, UnknownFunction46e9a0(buffer));
                        listBox->UnknownFunction470730(1, UnknownFunction46e9a0(buffer));
                        listBox->UnknownFunction470730(2, UnknownFunction46e9a0(buffer));
                        listBox->UnknownFunction470730(4, UnknownFunction46e9a0(buffer));
                    }
                }

                if (!_stricmp(type, "RADIOBUTTON") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x4e3) UIRadioButton(0, &area, dialog);
                        radioButton = (UIRadioButton*)UnknownFunction46a8a0(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault)
                        radioButton->UnknownFunction478cf0(value);
                    parameters.UnknownFunction4b7ec0("AnimOff", defaultAnimOff, isDefault ? defaultAnimOff : buffer, -1);
                    if (!isDefault)
                        radioButton->UnknownFunction4789f0(0, (UIAnim*)UnknownFunction46e9a0(buffer), 0);
                    parameters.UnknownFunction4b7ec0("AnimOn", defaultAnimOn, isDefault ? defaultAnimOn : buffer, -1);
                    if (!isDefault)
                        radioButton->UnknownFunction4789f0(1, (UIAnim*)UnknownFunction46e9a0(buffer), 0);
                }

                if (!_stricmp(type, "STATIC") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x4f3) UIStatic(0, &area, dialog);
                        staticControl = (UIStatic*)UnknownFunction46a8a0(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        staticControl->UnknownFunction470730(0, UnknownFunction46e9a0(buffer));
                }

                if (!_stricmp(type, "VSCROLLBAR") || !_stricmp(type, "HSCROLLBAR") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x500) UIScrollBar(7, 0, &area, dialog);
                        scrollBar = (UIScrollBar*)UnknownFunction46a8a0(control, groupId, 0);
                    }
                    if (!_stricmp(type, "HSCROLLBAR"))
                        control->field_0x5c = 8;
                    int ticks;
                    int position;
                    int scale;
                    parameters.UnknownFunction4b7f10("TickNum", defaultTickNum, isDefault ? &defaultTickNum : &ticks);
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &position);
                    parameters.UnknownFunction4b7f10("Scale", defaultScale, isDefault ? &defaultScale : &scale);
                    if (!isDefault) {
                        scrollBar->UnknownFunction4754d0(ticks);
                        scrollBar->UnknownFunction4751c0(scale);
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        scrollBar->field_0x204_image = (UIAnim*)UnknownFunction46e9a0(buffer);
                    parameters.UnknownFunction4b7ec0("AnimThumb", defaultAnimThumb, isDefault ? defaultAnimThumb : buffer,
                                                     -1);
                    if (!isDefault) {
                        scrollBar->UnknownFunction470730(0, UnknownFunction46e9a0(buffer));
                        scrollBar->UnknownFunction470730(1, UnknownFunction46e9a0(buffer));
                        scrollBar->UnknownFunction470730(2, UnknownFunction46e9a0(buffer));
                        scrollBar->UnknownFunction470730(3, UnknownFunction46e9a0(buffer));
                        if (control->field_0x3c[2] == control->field_0x3c[0]) {
                            UIAnim* image = control->field_0x16c[0];
                            control->field_0x3c[2] = image->field_0x28[0]->field_0x0c + control->field_0x3c[0];
                            control->field_0x3c[3] = control->field_0x3c[1] + image->field_0x28[0]->field_0x10;
                        }
                        scrollBar->UnknownFunction4753c0(position, ticks ? ticks
                                                                         : control->field_0x3c[2] - control->field_0x3c[0] + 1);
                        scrollBar->UnknownFunction475160(0);
                    }
                }

                if (!_stricmp(type, "EDITBOX") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x52c) UIEditBox(0, &area, dialog, 100, 0, 0);
                        editBox = (UIEditBox*)UnknownFunction46a8a0(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7f10("MaxLen", defaultMaxLen, isDefault ? &defaultMaxLen : &value);
                    if (!isDefault)
                        editBox->UnknownFunction473c70(value);
                    parameters.UnknownFunction4b7ec0("SoundClick", defaultSoundClick,
                                                     isDefault ? defaultSoundClick : buffer, -1);
                    if (!isDefault)
                        editBox->UnknownFunction473d80((int)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("SoundEnd", defaultSoundEnd, isDefault ? defaultSoundEnd : buffer, -1);
                    if (!isDefault)
                        editBox->UnknownFunction473d90((int)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault) {
                        editBox->UnknownFunction470730(0, UnknownFunction46e9a0(buffer));
                        editBox->UnknownFunction470730(1, UnknownFunction46e9a0(buffer));
                        editBox->UnknownFunction470730(2, UnknownFunction46e9a0(buffer));
                        editBox->UnknownFunction470730(4, UnknownFunction46e9a0(buffer));
                    }
                }

                if (!_stricmp(type, "STATICTEXT") || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x545) UIStaticText(0, &area, dialog, 0, 0xffff);
                        staticText = (UIStaticText*)UnknownFunction46a8a0(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        staticText->UnknownFunction470730(0, UnknownFunction46e9a0(buffer));
                }

                if (!_strnicmp(type, "DROPDOWNLIST", 12) || isDefault) {
                    if (!isDefault) {
                        control = new(__FILE__, 0x54f) UIDropDownList(0, &area, dialog, 0, 0xffff);
                        dropDownList = (UIDropDownList*)UnknownFunction46a8a0(control, groupId, 0);
                    }
                    parameters.UnknownFunction4b7ec0("AnimNorm", defaultAnimNorm, isDefault ? defaultAnimNorm : buffer, -1);
                    if (!isDefault)
                        dropDownList->field_0x1ec->UnknownFunction473310((UIAnim*)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("AnimFocus", defaultAnimFocus, isDefault ? defaultAnimFocus : buffer,
                                                     -1);
                    if (!isDefault)
                        dropDownList->field_0x1ec->UnknownFunction473330((UIAnim*)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("AnimPush", defaultAnimPush, isDefault ? defaultAnimPush : buffer, -1);
                    if (!isDefault)
                        dropDownList->field_0x1ec->UnknownFunction473350((UIAnim*)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("AnimThumb", defaultAnimThumb, isDefault ? defaultAnimThumb : buffer,
                                                     -1);
                    if (!isDefault) {
                        UIScrollBar* thumb = dropDownList->field_0x200;
                        thumb->UnknownFunction470730(0, UnknownFunction46e9a0(buffer));
                        thumb->UnknownFunction470730(1, UnknownFunction46e9a0(buffer));
                        thumb->UnknownFunction470730(2, UnknownFunction46e9a0(buffer));
                        thumb->UnknownFunction470730(3, UnknownFunction46e9a0(buffer));
                        thumb->UnknownFunction475160(0);
                    }
                    parameters.UnknownFunction4b7f10("ListHeight", defaultListHeight,
                                                     isDefault ? &defaultListHeight : &value);
                    if (!isDefault) {
                        dropDownList->UnknownFunction47a880(value);
                        dropDownListBox = dropDownList->field_0x1fc;
                    }
                    char listKey[0x80];
                    item = 1;
                    sprintf(listKey, "%s%d", "Item", 1);
                    if (parameters.UnknownFunction4b7ec0(listKey, defaultItems[1], isDefault ? defaultItems[1] : buffer,
                                                         -1)) {
                        do {
                            if (!isDefault)
                                dropDownListBox->UnknownFunction476d80(buffer, 0, 0);
                            item++;
                            if (isDefault && item >= 50)
                                break;
                            sprintf(listKey, "%s%d", "Item", item);
                        } while (parameters.UnknownFunction4b7ec0(listKey, item < 50 ? defaultItems[item] : "",
                                                                  isDefault ? defaultItems[item] : buffer, -1));
                    }
                    control->UnknownFunction470dc0(dialog->field_0x9cc[i].field_0x00);
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
                        dropDownList->field_0x1ec->UnknownFunction470810(0, UnknownFunction46e9a0(soundNorm));
                        dropDownList->field_0x1ec->UnknownFunction470810(1, UnknownFunction46e9a0(soundFocus));
                        dropDownList->field_0x1ec->UnknownFunction470810(2, UnknownFunction46e9a0(soundPush));
                        dropDownList->field_0x1ec->UnknownFunction470810(3, UnknownFunction46e9a0(soundClick));
                        dropDownList->field_0xe8 = leftMargin;
                        dropDownList->field_0xec = topMargin;
                        dropDownList->UnknownFunction470d40(textColor);
                        dropDownList->UnknownFunction470d80(dropColor);
                        dropDownList->UnknownFunction47a400();
                    }
                    parameters.UnknownFunction4b7f10("Default", defaultDefault, isDefault ? &defaultDefault : &value);
                    if (!isDefault) {
                        int row = value - 1 < 0 ? 0 : value - 1;
                        dropDownListBox->UnknownFunction476a60(row);
                        dropDownList->UnknownFunction470b20(dropDownListBox->UnknownFunction476d20(row));
                    }
                    parameters.UnknownFunction4b7ec0("AnimTextBack", defaultAnimTextBack,
                                                     isDefault ? defaultAnimTextBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->UnknownFunction47ab20((UIAnim*)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("AnimListBack", defaultAnimListBack,
                                                     isDefault ? defaultAnimListBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->UnknownFunction47a970((UIAnim*)UnknownFunction46e9a0(buffer));
                    parameters.UnknownFunction4b7ec0("AnimScrollBack", defaultAnimScrollBack,
                                                     isDefault ? defaultAnimScrollBack : buffer, -1);
                    if (!isDefault && buffer[0])
                        dropDownList->UnknownFunction47aa40((UIAnim*)UnknownFunction46e9a0(buffer));
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
                control->field_0x78 = groupId;
                control->UnknownFunction470830((UnknownGameUiControl*)UnknownFunction46e9a0(anchor), relAnchor);
                control->UnknownFunction470dc0(dialog->field_0x9cc[i].field_0x00);
                control->field_0x168 = moveable;
                if (control->field_0x3c[2] == control->field_0x3c[0]) {
                    UIAnim* image;
                    if (!_stricmp(type, "MULTISTATE") || !_stricmp(type, "RADIOBUTTON"))
                        image = ((UIMultiState*)control)->field_0x1f4[((UIMultiState*)control)->field_0x1f0].field_0x00;
                    else
                        image = control->field_0x16c[0];
                    control->field_0x3c[2] = image->field_0x28[0]->field_0x0c + control->field_0x3c[0];
                    control->field_0x3c[3] = control->field_0x3c[1] + image->field_0x28[0]->field_0x10;
                }
                if (fontName[0] || (fontHeight && dialog->field_0x30 && dialog->field_0x30->field_0x50)) {
                    control->field_0x12c = 1;
                    if (control->field_0x128)
                        DeleteObject((HGDIOBJ)control->field_0x128);
                    control->field_0x128 = 0;
                    if (fontHeight)
                        control->field_0x160 = fontHeight;
                    if (bold > 500)
                        bold = 1;
                    else if (bold > 1)
                        bold = 0;
                    const char* face = dialog->field_0x30 ? dialog->field_0x30->field_0x350 : "";
                    if (*face) {
                        strcpy(control->field_0x130, face);
                        bold = dialog->field_0x30->field_0x3d4;
                        italic = dialog->field_0x30->field_0x3d8;
                    }
                    control->field_0x160 = (int)((dialog->field_0x30 ? *(float*)&dialog->field_0x30->field_0x3d0 : 0.0f) +
                                                 control->field_0x160);
                    if (control->field_0x160 != dialog->field_0xdc || bold != dialog->field_0x108 ||
                        italic != dialog->field_0x10c) {
                        control->field_0x160 = (int)(control->field_0x160 * dialog->field_0x9c8);
                        control->field_0x128 = (int)CreateFontA(control->field_0x160, 0, 0, 0, bold ? FW_BOLD : FW_MEDIUM,
                                                                italic, 0, 0, DEFAULT_CHARSET, 0, 0, 2, 2, dialog->field_0xe0);
                        strcpy(control->field_0x130, dialog->field_0xe0);
                        control->field_0x158 = bold;
                        control->field_0x15c = italic;
                    }
                }
                if (sourceBlit)
                    control->field_0xa4 = 1;
                control->UnknownFunction4709d0(shapeBounds);
                if (dialog->field_0x30->field_0x348) {
                    int textId;
                    int toolTipId;
                    parameters.UnknownFunction4b7f10("LocalTextId", 0, &textId);
                    parameters.UnknownFunction4b7f10("LocalToolTipId", 0, &toolTipId);
                    if (textId)
                        control->UnknownFunction470a80(dialog->field_0x30->field_0x348, textId);
                    if (toolTipId) {
                        char tip[0x400];
                        if (LoadStringA((HINSTANCE)dialog->field_0x30->field_0x348, toolTipId, tip, sizeof(tip))) {
                            if (control->field_0xf0)
                                operator delete(control->field_0xf0, __FILE__, 0x646);
                            control->field_0xf0 = (char*)DebugMalloc(strlen(tip) + 1, __FILE__, 0x647);
                            strcpy(control->field_0xf0, tip);
                        } else {
                            sprintf(tip, "Resource string '%d' load fail\n", toolTipId);
                            if (control->field_0xf0)
                                operator delete(control->field_0xf0, __FILE__, 0x64d);
                            control->field_0xf0 = 0;
                        }
                    }
                } else {
                    if (toolTip[0]) {
                        control->field_0xf0 = (char*)DebugMalloc(strlen(toolTip) + 1, __FILE__, 0x656);
                        strcpy(control->field_0xf0, toolTip);
                    }
                    if (text[0])
                        control->UnknownFunction470b20(text);
                }
                control->field_0xe8 = textLMargin;
                control->field_0xec = textTMargin;
                control->UnknownFunction470d60(textDrop != 0);
                control->UnknownFunction470d40(textColor);
                control->UnknownFunction470d80(dropColor);
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
                    switch (control->field_0x5c) {
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
                control->UnknownFunction470da0(align);
                if (!_stricmp(fx, "GROW")) {
                    control->UnknownFunction470870(100, (int)UnknownFunction46e9a0(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "LSLIDE")) {
                    control->UnknownFunction470870(101, (int)UnknownFunction46e9a0(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "TSLIDE")) {
                    control->UnknownFunction470870(103, (int)UnknownFunction46e9a0(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "RSLIDE")) {
                    control->UnknownFunction470870(102, (int)UnknownFunction46e9a0(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "BSLIDE")) {
                    control->UnknownFunction470870(104, (int)UnknownFunction46e9a0(fxSound), 1, fxDelay, -1);
                } else if (!_stricmp(fx, "ANIM")) {
                    control->UnknownFunction470870(106, 0, 1, 0, -1);
                    control->field_0x184 = (UIAnim*)UnknownFunction46e9a0(fxAnimIn);
                    control->field_0x188 = (UIAnim*)UnknownFunction46e9a0(fxAnimOut);
                    control->field_0x1ac = UnknownFunction46e9a0(fxSoundIn);
                    control->field_0x1b0 = UnknownFunction46e9a0(fxSoundOut);
                }
                control->UnknownFunction470810(0, UnknownFunction46e9a0(soundNorm));
                control->UnknownFunction470810(1, UnknownFunction46e9a0(soundFocus));
                control->UnknownFunction470810(2, UnknownFunction46e9a0(soundPush));
                control->UnknownFunction470810(3, UnknownFunction46e9a0(soundClick));
                control->UnknownVirtualSlot49(enable != 0);
                control->UnknownFunction470660(show != 0, 1);
                control->field_0x180 = (int)UnknownFunction46e9a0(mouseAnim);
                control->field_0x1d8 = keyBind;
                control->field_0x1dc = pre3D == 0;
                control->field_0x1e0 = permanent;
                control->field_0x1cc = control->UnknownVirtualSlot48(-1);
                control->UnknownVirtualSlot39();
                dialog->field_0x9cc[i].field_0x34 = (Sound*)control;
                break;
            }
            }
        }
    }

    if (backgroundImage[0]) {
        UnknownGameUiControl* frame = UnknownFunction46ebf0("FRMBIStatic", 5);
        UIAnim* image = (UIAnim*)UnknownFunction46e9a0(backgroundImage);
        if (image) {
            int width = image->UnknownFunction472f90()->field_0x14;
            int imageHeight = image->UnknownFunction472f90()->field_0x18;
            frame->UnknownFunction470730(0, image);
            frame->field_0x2c[2] = width;
            frame->field_0x2c[3] = imageHeight;
            *(CameraRect*)frame->field_0x3c = *(CameraRect*)frame->field_0x2c;
            frame->field_0x1cc = frame->UnknownVirtualSlot48(0);
            dialog->field_0x160.right = dialog->field_0x160.left + width;
            dialog->field_0x160.bottom = dialog->field_0x160.top + imageHeight;
            frame->UnknownVirtualSlot39();
        }
    }
    if (dialog->field_0x15c && dialog->field_0x174 && dialog->field_0x178) {
        int align = dialog->field_0x170;
        RenderTarget* target = (RenderTarget*)dialog->field_0x18;
        if (align & 1) {
            dialog->field_0x160.right -= dialog->field_0x160.left;
            dialog->field_0x160.left = 0;
        } else if (align & 4) {
            int shift = target->field_0x0c - dialog->field_0x160.left * 2 - dialog->field_0x160.right;
            dialog->field_0x160.left += shift;
            dialog->field_0x160.right += shift;
        } else {
            int shift = (target->field_0x0c - dialog->field_0x160.left - dialog->field_0x160.right) / 2 -
                        dialog->field_0x160.left;
            dialog->field_0x160.left += shift;
            dialog->field_0x160.right += shift;
        }
        if (align & 8) {
            dialog->field_0x160.bottom -= dialog->field_0x160.top;
            dialog->field_0x160.top = 0;
        } else if (align & 0x20) {
            int shift = target->field_0x10 - dialog->field_0x160.top * 2 - dialog->field_0x160.bottom;
            dialog->field_0x160.top += shift;
            dialog->field_0x160.bottom += shift;
        } else {
            int shift = (target->field_0x10 - dialog->field_0x160.top - dialog->field_0x160.bottom) / 2 -
                        dialog->field_0x160.top;
            dialog->field_0x160.top += shift;
            dialog->field_0x160.bottom += shift;
        }
    }
    return 1;
}

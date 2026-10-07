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
// 0x00475c70 UIListBox constructor (316/382).
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
// 0x00479b50 UIDDLStatic constructor (93/103): retail stores a field before
//   the vptr, which needs a mem-initializer of the class's own member.
// 0x00479bf0 UIDDLButton constructor (93/103): as UIDDLStatic's.
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
// 0x00479ea0 UIDropDownList constructor (786/803): every part is allocated
//   with sizeof 0x254 (GameUi.h flattens the derived controls' members into
//   UIControl; retail sizes 0x1f4/0x254/0x224/0x1f0), and the list box's
//   arguments are pushed as (id, rows, area, owner, list), which suggests
//   UIListBox's real parameter order is (id, rows, area, owner).

#include "../../src/reconstructed/GameUi.cpp"

#include <imm.h>

#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/PCTextureMap.h"
#include "../../src/reconstructed/Palette8.h"
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
    unsigned int position = field_0x21c ? range - value : value;
    UnknownVirtualSlot50();
    if (!field_0x1f0 && UnknownFunction475300(range) == position)
        return 1;
    if (!range) {
        field_0x1ec_float = position;
        return 1;
    }
    unsigned int travel = field_0x5c == 8 ? UnknownVirtualSlot61() - field_0x1f4 : UnknownVirtualSlot62() - field_0x1f8;
    if ((unsigned int)range > 0) {
        float scaled = (double)(travel * position) / (unsigned int)range;
        field_0x1ec_float = scaled < travel ? scaled : travel;
        return 1;
    }
    return 0;
}

// 0x00475c70
UIListBox::UIListBox(int id, CameraRect* area, UnknownGameUiDialog* owner, int rows)
    : UnknownGameUiControl(3, id, area, owner) {
    field_0x210 = 0;
    field_0x20c = 0;
    field_0x1ec = 0;
    field_0x1f4 = 0;
    field_0x1f0 = 0;
    field_0x1fc_value = rows;
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
    memset(field_0x214, 0, field_0x1fc_value * sizeof(UnknownGameUiListRow));
    field_0x244 = 1;
    int height = field_0x160;
    if (!height && (!field_0xb8 || !field_0xb8->field_0xdc)) {
        field_0x200 = 1;
    } else {
        if (!height)
            height = field_0xb8->field_0xdc;
        int rowsShown = (field_0x2c[3] - field_0x2c[1]) / height;
        field_0x200 = rowsShown < 1 ? 1 : rowsShown;
    }
}

// 0x004773a0
int UnknownGameUiControl::UnknownFunction4773a0(UIAnim* image, int data, int a) {
    if (field_0x1ec >= field_0x1fc_value && !UnknownFunction476f50(field_0x1ec + 1))
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
    field_0x214[field_0x1ec] = row;
    UnknownFunction476930(field_0x1ec, data);
    field_0x1ec++;
    field_0x200 = UnknownFunction476ee0();
    UnknownFunction477bc0();
    if (field_0x21c)
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
    UnknownGameUiControl* other;
    while ((other = g_UnknownGlobal65b608->field_0x9bc->field_0x218_control->UnknownFunction472790(iterator)) != 0) {
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
    field_0x204 = 0;
    for (int i = field_0x1ec - 1; i >= 0; i--) {
        height += field_0x214[i].field_0x0c;
        if (height > field_0x2c[3] - field_0x2c[1])
            break;
        field_0x204++;
    }
    field_0x204 = field_0x204 < 0 ? 0 : field_0x204;
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
    if ((field_0x60 == 4 && (image = field_0x1f4_states[field_0x1f0].field_0x04) != 0) ||
        (image = field_0x1f4_states[field_0x1f0].field_0x00) != 0)
        return image->UnknownFunction472f90();
    return 0;
}

// 0x00479b50
UIDDLStatic::UIDDLStatic(int id, CameraRect* area, UnknownGameUiDialog* owner, UIDropDownList* list)
    : UIStatic(id, area, owner) {
    field_0x1ec_list = list;
    UnknownFunction469ce0(this);
}

// 0x00479bf0
UIDDLButton::UIDDLButton(int id, CameraRect* area, UnknownGameUiDialog* owner, UIDropDownList* list)
    : UIButton(id, area, owner) {
    field_0x1f0_list = list;
    UnknownFunction469ce0(this);
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
            int offset = width - field_0x1fc_value;
            field_0x200 = offset > 0 ? 0 : offset;
        } else if (field_0xd8 & 2) {
            if (UnknownVirtualSlot61() > field_0x1fc_value)
                field_0x200 = width / 2 - field_0x1fc_value / 2;
            else
                field_0x200 = width - field_0x1fc_value;
        } else if (field_0xd8 & 4) {
            field_0x200 = width - field_0x1fc_value;
        } else {
            int offset = width - field_0x1fc_value;
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
                int caret = rect.left + field_0x1fc_value;
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
            field_0x1fc_value = size.cx;
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
                    control->field_0x23c = 0;
                    ((UIListBox*)control)->UnknownFunction477730(-1);
                }
                if (control->field_0x5c == 6) {
                    control->field_0x1fc->field_0x23c = 0;
                    ((UIListBox*)control->field_0x1fc)->UnknownFunction477730(-1);
                }
            }
        } else {
            iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            for (control = UnknownFunction472750(iterator); control; control = UnknownFunction472750(iterator)) {
                if (control->field_0x5c == 3) {
                    control->field_0x23c = 0;
                    ((UIListBox*)control)->UnknownFunction477730(1);
                }
                if (control->field_0x5c == 6) {
                    control->field_0x1fc->field_0x23c = 0;
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
            event.field_0x04 = field_0x220_list->UnknownFunction470df0();
            event.field_0x0c = field_0xb8;
            event.field_0x10 = field_0xbc;
            event.field_0x14 = this;
            field_0xb8->UnknownVirtualSlot29(&event);
            field_0x208 = 1;
        }
        if (!event.field_0x20) {
            if (position[0] == field_0x20c_value && position[1] == field_0x210_value) {
                field_0x218_float += field_0xb8->field_0x9c0;
                if (field_0x218_float < field_0x214_float)
                    goto scroll;
                event.field_0x08 = 0x11;
                event.field_0x00 = field_0x74;
                event.field_0x04 = field_0x220_list->UnknownFunction470df0();
                event.field_0x0c = field_0xb8;
                event.field_0x10 = field_0xbc;
                event.field_0x14 = this;
                field_0xb8->UnknownVirtualSlot29(&event);
            } else {
                event.field_0x08 = 3;
                event.field_0x00 = field_0x74;
                event.field_0x04 = field_0x220_list->UnknownFunction470df0();
                event.field_0x0c = field_0xb8;
                event.field_0x10 = field_0xbc;
                event.field_0x14 = this;
                field_0xb8->UnknownVirtualSlot29(&event);
                field_0x20c_value = position[0];
                field_0x210_value = position[1];
                field_0x218_float = 0.0f;
            }
            if (!event.field_0x20) {
            scroll:
                UnknownGameUiControl* list = field_0x220_list->field_0x1fc;
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
    if (field_0x1f4_states && field_0x1cc) {
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
    if ((field_0x1f4_states && field_0x1f4_states[field_0x1f0].field_0x08 && field_0x1f4_states[field_0x1f0].field_0x0c) ||
        (field_0xa4 && field_0xc0 && field_0xd0)) {
        void* dc;
        int more;
        field_0x1bc = redraw;
        do {
            if (field_0xb8->UnknownFunction46ed70(&dc, (CameraRect*)field_0x2c, &more, this)) {
                if (field_0xa4) {
                    rect = *(CameraRect*)field_0x2c;
                    rect.left += field_0x1f4_states[field_0x1f0].field_0x00->UnknownFunction472f80()->field_0x0c +
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
    UIAnim* image = field_0x1ec_control->field_0x16c[0];
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
    field_0x1ec_control->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1ec_control->field_0x2c = button;
    *(CameraRect*)field_0x1ec_control->field_0x3c = *(CameraRect*)field_0x1ec_control->field_0x2c;
    if (!button.left)
        field_0x1ec_control->UnknownVirtualSlot4();
    sprintf(name, "%s_LB", UnknownFunction470df0());
    field_0x1fc->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1fc->field_0x2c = list;
    *(CameraRect*)field_0x1fc->field_0x3c = *(CameraRect*)field_0x1fc->field_0x2c;
    sprintf(name, "%s_SB", UnknownFunction470df0());
    field_0x200_control->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x200_control->field_0x2c = scroll;
    *(CameraRect*)field_0x200_control->field_0x3c = *(CameraRect*)field_0x200_control->field_0x2c;
    sprintf(name, "%s_LBK", UnknownFunction470df0());
    field_0x1f0_control->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1f0_control->field_0x2c = list;
    *(CameraRect*)field_0x1f0_control->field_0x3c = *(CameraRect*)field_0x1f0_control->field_0x2c;
    sprintf(name, "%s_SBK", UnknownFunction470df0());
    field_0x1f4_control->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1f4_control->field_0x2c = scroll;
    *(CameraRect*)field_0x1f4_control->field_0x3c = *(CameraRect*)field_0x1f4_control->field_0x2c;
    sprintf(name, "%s_TBK", UnknownFunction470df0());
    field_0x1f8_control->UnknownFunction470dc0(name);
    *(CameraRect*)field_0x1f8_control->field_0x2c = full;
    *(CameraRect*)field_0x1f8_control->field_0x3c = *(CameraRect*)field_0x1f8_control->field_0x2c;
    field_0x1fc->field_0xe8 = field_0xe8;
    field_0x1fc->field_0xec = field_0xec;
    field_0x1fc->UnknownFunction476c70(field_0xc4, -1);
    if (field_0xc4 != 0xffffff)
        field_0x1fc->UnknownFunction476b80(0xffffff);
    field_0x1fc->UnknownFunction470d80(field_0xc8);
}

// 0x00479ea0
UIDropDownList::UIDropDownList(int id, CameraRect* area, UnknownGameUiDialog* owner, const char* text,
                               unsigned int color)
    : UIStaticText(id, area, owner, text, color) {
    UnknownFunction469ce0(this);
    CameraRect rect;
    field_0x5c = 6;
    field_0x204 = 0x14;
    rect.left = 0;
    rect.top = 0;
    rect.right = 10;
    rect.bottom = 10;
    field_0x1ec_control = new(__FILE__, 0x24e3) UIDDLButton(0, &rect, owner, this);
    field_0x1fc = new(__FILE__, 0x24e4) UIDDLListBox(0, &rect, owner, 100, this);
    field_0x200_control = new(__FILE__, 0x24e5) UIDDLScrollBar(7, 0, &rect, owner, this);
    field_0x1f0_control = new(__FILE__, 0x24e6) UIDDLStatic(0, &rect, owner, this);
    field_0x1f4_control = new(__FILE__, 0x24e7) UIDDLStatic(0, &rect, owner, this);
    field_0x1f8_control = new(__FILE__, 0x24e8) UIDDLStatic(0, &rect, owner, this);
    field_0x1f0_control->UnknownFunction470830(this, 1);
    field_0x1f4_control->UnknownFunction470830(this, 1);
    field_0x1f8_control->UnknownFunction470830(this, 1);
    field_0x1ec_control->UnknownFunction470830(this, 1);
    field_0x1fc->UnknownFunction470830(this, 1);
    field_0x200_control->UnknownFunction470830(this, 1);
    field_0x1fc->UnknownVirtualSlot52(0x4d43);
    field_0x200_control->UnknownVirtualSlot52(0x4d43);
    UnknownFunction469190(field_0x1ec_control, -1);
    UnknownFunction469190(field_0x1f8_control, -1);
    field_0x208 = 0;
    UnknownFunction47a2d0(0);
    owner->UnknownFunction46a840(field_0x1ec_control, 0, 0);
    owner->UnknownFunction46a840(field_0x1fc, 0, 0);
    owner->UnknownFunction46a840(field_0x200_control, 0, 0);
    owner->UnknownFunction46a840(field_0x1f0_control, 0, 0);
    owner->UnknownFunction46a840(field_0x1f8_control, 0, 0);
    owner->UnknownFunction46a840(field_0x1f4_control, 0, 0);
}

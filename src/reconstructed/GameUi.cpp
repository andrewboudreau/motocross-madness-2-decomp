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
#include "DialogProc.h"
#include "GUIManager.h"
#include "MatrixUtil.h"
#include "PCAudio.h"
#include "PCRenderTarget.h"
#include "RenderTarget.h"
#include "RenderInterfaces.h"
#include "TextureMap.h"
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

// cdecl 0x0047b570: resizes a DebugMalloc'd block (KrustyUI.cpp).
void* UnknownFunction47b570(void* block, unsigned int size);
// 0x0065b608: the dialog whose list box is being sorted (0x00477900).
UnknownGameUiDialog* g_UnknownGlobal65b608;

// cdecl 0x00477b60: the list rows' default order (by text).
int UnknownFunction477b60(const void* a, const void* b);
// cdecl 0x00477800: the qsort comparison 0x00477900 sorts with
// (samples/ui/GameUiNearMisses.cpp).
int UnknownFunction477800(const void* a, const void* b);

// A dialog's named sound (0x3c bytes; the table is at UIDialog+0x9cc).
struct UnknownGameUiDialogSound {
    char field_0x00[0x34];                    // name
    Sound* field_0x34;
    int field_0x38;
};

// A dialog as its controls see it: UIDialog (0x7f58 bytes) with the
// members read here. Never constructed as such.
class UnknownGameUiDialog : public UIDialog {
public:
    virtual void UnknownVirtualSlot27();      // 0x0046a300: creates the controls
    virtual void UnknownVirtualSlot28(int value); // 0x0046e8c0
    virtual void UnknownVirtualSlot29(UnknownDialogEvent* event); // 0x00470020: the dialog's procedure
    virtual void UnknownVirtualSlot30();      // 0x00470040
    virtual void UnknownVirtualSlot31(int value); // 0x00470050

    void* UnknownInlineField18() { return field_0x18; }

    UnknownGameUiDialog* field_0x2c;          // parent dialog
    GUIManager* field_0x30;
    GUIUser* field_0x34;
    unsigned char field_0x38[0xbc - 0x38];
    int field_0xbc;                           // shown
    unsigned char field_0xc0[0xc8 - 0xc0];
    int field_0xc8;                           // animates its controls
    void (*field_0xcc)(UnknownDialogEvent* event); // the procedure (slot 29)
    void (*field_0xd0)(UIDialog* dialog, int value); // slot 31
    void (*field_0xd4)();                     // slot 30
    void* field_0xd8;                         // font (HFONT)
    int field_0xdc;                           // font height
    unsigned char field_0xe0[0x110 - 0xe0];
    BackgroundImage* field_0x110;
    unsigned char field_0x114[0x140 - 0x114];
    void* field_0x140;                        // palette
    int field_0x144;
    int field_0x148;
    int field_0x14c;
    unsigned char field_0x150[0x154 - 0x150];
    int field_0x154;
    unsigned char field_0x158[0x160 - 0x158];
    CameraRect field_0x160;                   // screen area
    int field_0x170;
    int field_0x174;                          // where controls slide in from (x)
    int field_0x178;                          // (y)
    int field_0x17c;                          // result
    int field_0x180;
    unsigned char field_0x184[0x9b0 - 0x184];
    int field_0x9b0;
    unsigned char field_0x9b4[0x9bc - 0x9b4];
    UnknownGameUiControl* field_0x9bc;        // the list box being sorted
    float field_0x9c0;                        // frame time
    float field_0x9c4;                        // horizontal scale
    float field_0x9c8;                        // vertical scale
    UnknownGameUiDialogSound field_0x9cc[500];
    int field_0x7efc;                         // sounds
    void* field_0x7f00;
    void* field_0x7f04;
    unsigned char field_0x7f08[0x7f18 - 0x7f08];
    int field_0x7f18;
    unsigned char field_0x7f1c[0x7f20 - 0x7f1c];
    void* field_0x7f20;                       // textures
    ContainerList<UITimer*> field_0x7f24;     // timers
    unsigned char field_0x7f38[0x7f3c - 0x7f38];
    GameObject* field_0x7f3c;                 // the controls' container
    unsigned char field_0x7f40[0x7f44 - 0x7f40];
    PCTextureMap* field_0x7f44;               // the screen behind it
    int field_0x7f48;                         // its background region
    int field_0x7f4c;
    int field_0x7f50;                         // closing
    int field_0x7f54;                         // the frame it closed on
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
int UnknownGameUiControl::UnknownVirtualSlot34() {
    return (int)field_0xc0;
}

// 0x004703d0
int UnknownGameUiControl::UnknownVirtualSlot35() {
    return field_0xd0;
}

// 0x004703e0
int UnknownGameUiControl::UnknownVirtualSlot36() {
    return (int)field_0xe4;
}

// 0x004703f0
void* UnknownGameUiControl::UnknownVirtualSlot37() {
    return field_0xdc;
}

// 0x00470400
void UnknownGameUiControl::UnknownVirtualSlot52(int id) {
    field_0x7c = id;
}

// 0x00470410
int UnknownGameUiControl::UnknownVirtualSlot61() {
    return field_0x2c[2] - field_0x2c[0];
}

// 0x00470420
int UnknownGameUiControl::UnknownVirtualSlot62() {
    return field_0x2c[3] - field_0x2c[1];
}

// 0x00470640
void UnknownGameUiControl::UnknownVirtualSlot50() {
    field_0x1c0 = 0;
    field_0x1bc = 3;
}

// 0x00470720
int UnknownGameUiControl::UnknownFunction470720() {
    return field_0x6c;
}

// 0x00470730
void UnknownGameUiControl::UnknownFunction470730(int a, void* image) {
    field_0x16c[a] = (UIAnim*)image;
    if (a == 1)
        field_0x16c[3] = (UIAnim*)image;
    UnknownVirtualSlot50();
}

// 0x00470810
void UnknownGameUiControl::UnknownFunction470810(int index, Sound* sound) {
    field_0x194[index] = sound;
}

// 0x00470830
void UnknownGameUiControl::UnknownFunction470830(UnknownGameUiControl* next, int a) {
    field_0x18c = next;
    field_0x190 = a;
}

// 0x00470850
UnknownGameUiControl* UnknownGameUiControl::UnknownFunction470850(UnknownGameUiControl* none) {
    UnknownGameUiControl* control = field_0x18c;
    if (control) {
        UnknownGameUiControl* last;
        do {
            last = control;
            control = control->field_0x18c;
        } while (control);
        return last;
    }
    return none;
}

// 0x00470970
void UnknownGameUiControl::UnknownVirtualSlot29(int state) {
    field_0x60 = state;
    UnknownVirtualSlot50();
}

// 0x004709d0
void UnknownGameUiControl::UnknownFunction4709d0(int value) {
    field_0xa0 = value;
    UnknownVirtualSlot50();
}

// 0x00470a70
void UnknownGameUiControl::UnknownVirtualSlot54(int* value) {
    field_0x1b4 = value;
}

// 0x00470d40
void UnknownGameUiControl::UnknownFunction470d40(unsigned int color) {
    field_0xc4 = color;
    UnknownVirtualSlot50();
}

// 0x00470d60
void UnknownGameUiControl::UnknownFunction470d60(int value) {
    field_0xd4 = value;
    UnknownVirtualSlot50();
}

// 0x00470d80
void UnknownGameUiControl::UnknownFunction470d80(int value) {
    field_0xc8 = value;
    UnknownVirtualSlot50();
}

// 0x00470da0
void UnknownGameUiControl::UnknownFunction470da0(int value) {
    field_0xd8 = value;
    UnknownVirtualSlot50();
}

// 0x00470dc0
void UnknownGameUiControl::UnknownFunction470dc0(const char* name) {
    strncpy(field_0xf4, name, 0x31);
    field_0xf4[0x31] = 0;
}

// 0x00470df0
char* UnknownGameUiControl::UnknownFunction470df0() {
    return field_0xf4;
}

// 0x00471fa0
TextureMap* UnknownGameUiControl::UnknownVirtualSlot48(int state) {
    if (state == -1)
        state = field_0x60;
    if (field_0x16c[state])
        return field_0x16c[state]->UnknownFunction472f90();
    return 0;
}

// ---------------------------------------------------------------------------
// UIAnim

// 0x00472f20
void UIAnim::UnknownFunction472f20(int count) {
    field_0x14 = count;
    field_0x18 = count;
    UnknownFunction472f50();
}

// 0x00472f40
void UIAnim::UnknownFunction472f40(int delay) {
    field_0x1c = delay;
}

// 0x00472f50
void UIAnim::UnknownFunction472f50() {
    if (field_0x24) {
        field_0x08 = field_0x10 - 1;
        field_0x20 = 0;
        field_0x0c = 1;
    } else {
        field_0x08 = 0;
        field_0x20 = 0;
        field_0x0c = 1;
    }
}

// 0x00472f80
UIFrame* UIAnim::UnknownFunction472f80() {
    return field_0x28[field_0x08];
}

// 0x00472f90
TextureMap* UIAnim::UnknownFunction472f90() {
    UIFrame* frame = field_0x28[field_0x08];
    if (frame->field_0x08 == 1)
        return 0;
    return frame->field_0x18;
}

// 0x00472fb0
int UIAnim::UnknownFunction472fb0(UIFrame* frame) {
    if (field_0x10 < 0x31) {
        field_0x28[field_0x10] = frame;
        field_0x10++;
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
    field_0x16c[0] = up;
    field_0x16c[1] = over;
    field_0x16c[2] = down;
    field_0x16c[3] = over;
    field_0x16c[4] = disabled;
    UnknownVirtualSlot50();
}

// 0x00473310
void UIButton::UnknownFunction473310(UIAnim* image) {
    field_0x16c[0] = image;
    UnknownVirtualSlot50();
}

// 0x00473330
void UIButton::UnknownFunction473330(UIAnim* image) {
    field_0x16c[1] = image;
    field_0x16c[3] = image;
    UnknownVirtualSlot50();
}

// 0x00473350
void UIButton::UnknownFunction473350(UIAnim* image) {
    field_0x16c[2] = image;
    UnknownVirtualSlot50();
}

// 0x00473370
void UIButton::UnknownFunction473370(UIAnim* image) {
    field_0x16c[4] = image;
    UnknownVirtualSlot50();
}

// 0x00473390
void UnknownGameUiControl::UnknownFunction473390(UnknownGameUiControl* list) {
    field_0x1ec_control = list;
}

// ---------------------------------------------------------------------------
// UIStatic

// 0x00478f60
UIStatic::UIStatic(int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UnknownGameUiControl(5, id, area, owner) {
}

// 0x00478fb0
UIStatic::~UIStatic() {
}

// 0x00478fc0
TextureMap* UIStatic::UnknownVirtualSlot48(int state) {
    if (field_0x16c[0])
        return field_0x16c[0]->UnknownFunction472f90();
    return 0;
}

// 0x00478fe0
int UIStatic::UnknownVirtualSlot30() {
    return 0;
}

// ---------------------------------------------------------------------------
// UIControl (continued)

// 0x00470990
int UnknownGameUiControl::UnknownVirtualSlot28(POINT point, int state) {
    RECT rect = *(RECT*)field_0x2c;
    return PtInRect(&rect, point);
}

// 0x004709f0
int UnknownGameUiControl::UnknownVirtualSlot51(POINT point) {
    RECT rect = *(RECT*)field_0x2c;
    if (!field_0xa0) {
        if (PtInRect(&rect, point))
            return 1;
    } else if (UnknownVirtualSlot28(point, field_0x60)) {
        return 1;
    }
    return 0;
}

// 0x00471070
void UnknownGameUiControl::UnknownVirtualSlot27() {
    if (field_0x16c[field_0x60])
        field_0x16c[field_0x60]->UnknownFunction472fe0();
    if (field_0x68 == 1 || field_0x68 == 2) {
        if (field_0x184)
            field_0x184->UnknownFunction472fe0();
        if (field_0x188)
            field_0x184->UnknownFunction472fe0();
    }
}

// 0x004710c0
void UnknownGameUiControl::UnknownVirtualSlot38() {
    UnknownGameUiControl* last = UnknownFunction470850(0);
    if (!field_0x70 || (last && (!last->field_0x70 || last->field_0x68))) {
        if (--field_0x1bc < 0) {
            field_0x1bc = 0;
            field_0x1c0 = 1;
        }
        return;
    }
    UIAnim* image = field_0x16c[field_0x60];
    if (!image) {
        field_0x1c0 = 1;
        return;
    }
    if (image->field_0x10 == 1)
        field_0x1c0 = 1;
    else
        field_0x1c0 = 0;
}

// 0x00471140
int UnknownGameUiControl::UnknownVirtualSlot13() {
    int result = GameObject::UnknownVirtualSlot13();
    UnknownGameUiControl* last = UnknownFunction470850(0);
    if ((!last || (last->field_0x70 && !last->field_0x68)) && !field_0x1dc) {
        if (field_0x70)
            return UnknownVirtualSlot40();
        field_0x1c0 = 1;
    }
    return result;
}

// 0x00472250
int UnknownGameUiControl::UnknownVirtualSlot56(int a, int* position) {
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this && !a && field_0x60 == 2) {
        UnknownVirtualSlot29(3);
        UnknownVirtualSlot58(3, field_0xb8->field_0x180);
    }
    field_0x1d4 = -1;
    return 0;
}

// 0x004722b0
void UnknownGameUiControl::UnknownVirtualSlot57(int a, int* position) {
    if (field_0x168 && a == 1) {
        UnknownVirtualSlot50();
        int width = UnknownVirtualSlot61();
        int height = UnknownVirtualSlot62();
        UnknownGameUiDialog* owner = field_0xb8;
        field_0x3c[0] = position[0] - owner->field_0x160.left;
        field_0x3c[2] = field_0x3c[0] + width;
        field_0x3c[1] = position[1] - owner->field_0x160.top;
        field_0x3c[3] = field_0x3c[1] + height;
    }
}

// 0x004725b0
int UnknownGameUiControl::UnknownVirtualSlot31() {
    if (UnknownVirtualSlot30() && field_0xb8->field_0x34->field_0x1d8 != (UnknownGuiControl*)this) {
        UnknownVirtualSlot29(1);
        if (field_0x16c[field_0x60])
            field_0x16c[field_0x60]->UnknownFunction472f50();
        UnknownVirtualSlot58(1, field_0xb8->field_0x180);
        ToolTip* tip = field_0xb8->field_0x34->field_0xc0;
        if (tip)
            tip->UnknownFunction486b10((UnknownGuiControl*)this);
    }
    return 1;
}

// 0x00472630
void UnknownGameUiControl::UnknownVirtualSlot32(int value) {
    if (field_0x6c)
        UnknownVirtualSlot29(0);
    ToolTip* tip = field_0xb8->field_0x34->field_0xc0;
    if (tip)
        tip->UnknownFunction486b80(0, 0, 1.0f);
}

// 0x00472670
void UnknownGameUiControl::UnknownVirtualSlot58(int index, int a) {
    if (field_0x194[index]) {
        field_0x194[index]->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
        field_0x194[index]->UnknownFunction4bc6b0(1, 0, 0);
    }
}

// 0x004726b0
int UnknownGameUiControl::UnknownVirtualSlot53() {
    return (int)new(__FILE__, 0x1267) GameObjectIterator(field_0xb8->field_0x7f3c, 1, "UIControl");
}

// 0x00472730
void UnknownGameUiControl::UnknownFunction472730(GameObjectIterator* iterator) {
    delete iterator;
}

// 0x00472750
UnknownGameUiControl* UnknownGameUiControl::UnknownFunction472750(GameObjectIterator* iterator) {
    if (!field_0x7c)
        return 0;
    UnknownGameUiControl* control;
    do {
        control = (UnknownGameUiControl*)iterator->Next();
    } while ((control && control->field_0x7c != field_0x7c) || control == this);
    return control;
}

// 0x00472790
UnknownGameUiControl* UnknownGameUiControl::UnknownFunction472790(GameObjectIterator* iterator) {
    UnknownGameUiControl* control = UnknownFunction472750(iterator);
    while (control && control->field_0x5c != field_0x5c)
        control = UnknownFunction472750(iterator);
    return control;
}

// 0x00472860
void UnknownGameUiControl::UnknownVirtualSlot63(CameraRect* in, CameraRect* out) {
    *out = *in;
    if (field_0xa4 && field_0x1cc) {
        out->right = (int)(field_0x1cc->field_0x14 * field_0xb8->field_0x9c4) + out->left;
        out->bottom = (int)(field_0x1cc->field_0x18 * field_0xb8->field_0x9c8) + out->top;
    }
}

// 0x004728e0
void UnknownGameUiControl::UnknownVirtualSlot64(CameraRect* in, CameraRect* out) {
    *out = *in;
    if (field_0x60 == 2) {
        out->top++;
        out->left++;
        out->bottom++;
        out->right++;
    }
    out->left += field_0xe8;
    out->top += field_0xec;
    out->right -= field_0xe8;
    out->bottom -= field_0xec;
}

// ---------------------------------------------------------------------------
// UIButton (continued)

// 0x004731f0
UIButton::UIButton(int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UnknownGameUiControl(1, id, area, owner) {
    field_0x1ec_control = 0;
    UnknownFunction4732d0(0, 0, 0, 0);
    UnknownFunction470810(0, 0);
    UnknownFunction470810(1, 0);
    UnknownFunction470810(2, 0);
    UnknownFunction470810(3, 0);
    UnknownFunction470810(4, 0);
}

// 0x00473410
int UIButton::UnknownVirtualSlot56(int a, int* position) {
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this && !a && field_0x60 == 2) {
        UnknownDialogEvent event;
        event.field_0x20 = a;
        if (field_0x1ec_control)
            field_0x1ec_control->UnknownFunction477900(1);
        event.field_0x08 = 1;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x10 = field_0xbc;
        event.field_0x14 = this;
        field_0xb8->UnknownVirtualSlot29(&event);
        if (event.field_0x20)
            return 1;
    }
    return UnknownGameUiControl::UnknownVirtualSlot56(a, position);
}

// ---------------------------------------------------------------------------
// UIEditBox

// 0x00473630
UIEditBox::UIEditBox(int id, CameraRect* area, UnknownGameUiDialog* owner, int size, int a, int b)
    : UnknownGameUiControl(0xb, id, area, owner) {
    char* text = field_0xc0;
    if (!text) {
        int capacity = size + 1;
        if (capacity >= 1000)
            capacity = 1000;
        else if (capacity < 1)
            capacity = 1;
        field_0x1f0 = capacity;
    } else {
        int capacity = size + 1;
        int length = strlen(text);
        field_0x1f0 = length > capacity ? length : capacity;
    }
    field_0x204 = a;
    field_0x208 = b;
    field_0x200 = 0;
    field_0x1ec = 0;
    field_0xd0 = 0;
    field_0x1fc_value = 0;
    field_0x210 = 0;
    field_0x20c = 0;
    field_0x228 = 0;
    field_0x22c = 0;
    if (!text)
        field_0xc0 = (char*)DebugMalloc(field_0x1f0, __FILE__, 0x15c7);
    field_0xc0[0] = 0;
}

// 0x00473770
UIEditBox::~UIEditBox() {
    if (field_0x1b4)
        UnknownVirtualSlot59(1);
    if (field_0xc0) {
        operator delete(field_0xc0, __FILE__, 0x15d2);
        field_0xc0 = 0;
    }
    if (field_0x210)
        operator delete(field_0x210, __FILE__, 0x15d6);
    if (field_0x20c)
        DeleteObject((HGDIOBJ)field_0x20c);
}

// 0x00473820
void UnknownGameUiControl::UnknownFunction473820(char* buffer, int size) {
    UnknownGameUiControl::UnknownVirtualSlot54((int*)buffer);
    field_0x224 = size;
}

// 0x00473840
void UIEditBox::UnknownVirtualSlot59(int value) {
    if (field_0x1b4 && field_0x224) {
        if (value) {
            strncpy((char*)field_0x1b4, field_0xc0, field_0x224);
            ((char*)field_0x1b4)[field_0x224 - 1] = 0;
        } else {
            UnknownFunction473da0((char*)field_0x1b4);
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
char* UnknownGameUiControl::UnknownFunction473ef0(char* buffer, int size) {
    if (field_0xd0 + 1 < size)
        size = field_0xd0 + 1;
    strncpy(buffer, field_0xc0, size);
    buffer[size - 1] = 0;
    return buffer;
}

// 0x00474060
void UIEditBox::UnknownFunction474060(unsigned long color) {
    if (field_0x20c)
        DeleteObject((HGDIOBJ)field_0x20c);
    if (color)
        field_0x20c = CreateSolidBrush(color);
    else
        field_0x20c = 0;
    UnknownVirtualSlot50();
}

// 0x004740b0
void UIEditBox::UnknownVirtualSlot33(int value) {
    UnknownDialogEvent event;
    event.field_0x20 = 0;
    event.field_0x08 = 10;
    event.field_0x00 = field_0x74;
    event.field_0x04 = UnknownFunction470df0();
    event.field_0x0c = field_0xb8;
    event.field_0x10 = field_0xbc;
    event.field_0x14 = this;
    field_0xb8->UnknownVirtualSlot29(&event);
    UnknownVirtualSlot50();
    UnknownGameUiControl::UnknownVirtualSlot33(value);
}

// 0x00474120
int UIEditBox::UnknownVirtualSlot55(int a, int b) {
    if (field_0xb8->field_0x34->field_0x1d4 != (UnknownGuiControl*)this)
        return UnknownGameUiControl::UnknownVirtualSlot55(a, b);
    return 0;
}

// ---------------------------------------------------------------------------
// UIScrollCtl

// 0x00474820
UIScrollCtl::UIScrollCtl(int type, int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UIButton(id, area, owner) {
    field_0x5c = type;
}

// 0x00474870
UIScrollCtl::~UIScrollCtl() {
}

// 0x00474b10
int UIScrollCtl::UnknownVirtualSlot56(int a, int* position) {
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this && !a && field_0x60 == 2) {
        UnknownDialogEvent event;
        event.field_0x20 = a;
        event.field_0x08 = 1;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x10 = field_0xbc;
        event.field_0x14 = this;
        field_0xb8->UnknownVirtualSlot29(&event);
        if (event.field_0x20)
            return 1;
    }
    return UnknownGameUiControl::UnknownVirtualSlot56(a, position);
}

// ---------------------------------------------------------------------------
// UIScrollBar

// 0x00474ba0
UIScrollBar::UIScrollBar(int type, int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UnknownGameUiControl(type, id, area, owner) {
    field_0x1ec = 0;
    field_0x1f0 = 0;
    field_0x1f4 = 0;
    field_0x1f8 = 0;
    field_0x1fc_value = 0;
    field_0x208 = 0;
    field_0x204 = 0;
    field_0x210 = 0;
    field_0x20c = 0;
    field_0x218 = 0;
    field_0x21c = 0;
    field_0x200 = 100;
    field_0x214_float = 0.2f;
}

// 0x00474c50
UIScrollBar::~UIScrollBar() {
    if (field_0x1b4)
        UnknownVirtualSlot59(1);
}

// 0x00474cb0
void UIScrollBar::UnknownVirtualSlot59(int value) {
    if (value)
        *field_0x1b4 = UnknownFunction475500();
    else
        UnknownFunction4753c0(*field_0x1b4, field_0x200);
}

// 0x00475160
void UIScrollBar::UnknownFunction475160(int state) {
    UnknownGameUiDialog* owner = field_0xb8;
    field_0x1f4 = (int)(field_0x16c[state]->field_0x28[0]->field_0x0c * owner->field_0x9c4);
    field_0x1f8 = (int)(field_0x16c[state]->field_0x28[0]->field_0x10 * owner->field_0x9c8);
}

// 0x004751c0
void UnknownGameUiControl::UnknownFunction4751c0(int range) {
    field_0x200 = range;
    UnknownVirtualSlot50();
}

// 0x004754d0
void UnknownGameUiControl::UnknownFunction4754d0(int range) {
    if (range > 0)
        field_0x1f0 = range - 1;
    else
        field_0x1f0 = 0;
    UnknownVirtualSlot50();
}

// 0x004755c0
int UnknownGameUiControl::UnknownFunction4755c0() {
    return field_0x1f0;
}

// ---------------------------------------------------------------------------
// UIListBox

// 0x00475fa0
void UIListBox::UnknownVirtualSlot27() {
    for (int i = 0; i < field_0x1ec; i++) {
        UIAnim* image = field_0x214[i].field_0x20;
        if (image)
            image->UnknownFunction472fe0();
    }
}

// 0x00475fe0
void UIListBox::UnknownVirtualSlot59(int value) {
    if (value) {
        *field_0x1b4 = UnknownFunction476950();
    } else {
        field_0x23c = 0;
        UnknownFunction476a60(*field_0x1b4);
    }
}

// ---------------------------------------------------------------------------
// UIFrame and UIAnim

// 0x00472d00: `module` is the image's name and `id` the resource manager
// holding it; `a` is the sound group a missing image is loaded from as a sound.
UIFrame::UIFrame(void* module, int id, void* textures, int a, int b, int c, void* palette) {
    if (!b) {
        field_0x1c = 0;
        field_0x08 = 0;
        field_0x14 = 0;
        field_0x20 = (int)textures;
        UnknownResourceEntry* entry =
            ((UnknownResourceManager*)id)->UnknownFunction4e9360((const char*)module, 1);
        if (!UnknownFunction472bc0(entry->field_0x14, entry->field_0x18, palette) && a) {
            UnknownFunction4bb890((SoundGroup*)a, (const char*)module, 1, 3, c, -1);
            field_0x1c = 1;
            field_0x08 = 1;
            field_0x0c = 0;
            field_0x10 = 0;
            field_0x18 = 0;
        }
    }
}

// 0x00472dc0
UIFrame::~UIFrame() {
    if (field_0x18)
        field_0x18->Release();
    if (field_0x1c)
        field_0x14->Release();
}

// 0x00472eb0
UIAnim::~UIAnim() {
    for (int i = 0; i < field_0x10; i++) {
        if (field_0x28[i])
            field_0x28[i]->Release();
    }
}

// 0x004730b0
TextureMap* UIAnim::UnknownFunction4730b0() {
    UIFrame* frame = UnknownFunction472fe0();
    while (frame->field_0x08 == 1) {
        frame->field_0x14->UnknownFunction4bc6b0(1, 0, 0);
        frame = UnknownFunction472fe0();
    }
    return frame->field_0x18;
}

// 0x004730e0
void UIAnim::UnknownFunction4730e0(const char* file, void* palette) {
    UnknownFunction472fb0(new(__FILE__, 0x14d1) UIFrame(file, field_0xf0, 0, 0, palette));
}

// 0x00473160
void UIAnim::UnknownFunction473160(void* module, int id, int a, void* palette) {
    UnknownFunction472fb0(new(__FILE__, 0x14e6) UIFrame(module, id, field_0xf0, a, 0, 0, palette));
}

// ---------------------------------------------------------------------------
// UIListBox (continued)

// 0x00475e20
UIListBox::~UIListBox() {
    if (field_0x1b4)
        UnknownVirtualSlot59(1);
    if (field_0x20c)
        DeleteObject((HGDIOBJ)field_0x20c);
    if (field_0x210)
        DeleteObject((HGDIOBJ)field_0x210);
    for (int i = 0; i < field_0x1ec; i++) {
        operator delete(field_0x214[i].field_0x14, __FILE__, 0x1b79);
        operator delete(field_0x214[i].field_0x18, __FILE__, 0x1b7a);
        operator delete(field_0x214[i].field_0x24, __FILE__, 0x1b7b);
        UIAnim* image = field_0x214[i].field_0x20;
        if (image && !image->field_0xf4)
            image->Release();
        if (field_0x214[i].field_0x34) {
            DeleteObject((HGDIOBJ)field_0x214[i].field_0x34);
            for (int j = 0; j < field_0x1ec; j++) {
                if (field_0x214[j].field_0x34 == field_0x214[i].field_0x34)
                    field_0x214[j].field_0x34 = 0;
            }
        }
    }
    operator delete(field_0x214, __FILE__, 0x1b88);
}

// 0x00476860
int UnknownGameUiControl::UnknownFunction476860(int row, int a) {
    int last = field_0x1ec - field_0x204;
    if (row < last)
        last = row;
    field_0x1f0 = last < 0 ? 0 : last;
    if (field_0x200 == 1)
        UnknownVirtualSlot65(field_0x1f0);
    UnknownVirtualSlot50();
    if (a)
        UnknownFunction477bc0();
    return 1;
}

// 0x004768d0
int UnknownGameUiControl::UnknownFunction4768d0(int row) {
    if (row == -1) {
        row = field_0x1f4;
        if (row == -1)
            return 0;
    }
    return field_0x214[row].field_0x1c;
}

// 0x00476900
int UIListBox::UnknownFunction476900(int data) {
    for (int i = 0; i < field_0x1ec; i++) {
        if (field_0x214[i].field_0x1c == data)
            return i;
    }
    return -1;
}

// 0x00476930
void UnknownGameUiControl::UnknownFunction476930(int row, int data) {
    field_0x214[row].field_0x1c = data;
}

// 0x00476950
int UnknownGameUiControl::UnknownFunction476950() {
    if (field_0x1ec > 0)
        return field_0x1f4;
    return -1;
}

// 0x00476970
char* UIListBox::UnknownFunction476970(int row) {
    if (field_0x1ec && row >= 0 && row < field_0x1ec)
        return (char*)field_0x214[row].field_0x00;
    return 0;
}

// 0x004769a0
int UIListBox::UnknownFunction4769a0(int row) {
    if (field_0x1ec && row >= 0 && row < field_0x1ec)
        return field_0x214[row].field_0x0c;
    return 0;
}

// 0x00476b80
void UnknownGameUiControl::UnknownFunction476b80(unsigned int color) {
    field_0x208 = color;
    UnknownVirtualSlot50();
}

// 0x00476d20
char* UnknownGameUiControl::UnknownFunction476d20(int row) {
    if (field_0x1f4 >= 0 && row == -1)
        return field_0x214[field_0x1f4].field_0x14;
    if (field_0x1ec && row >= 0 && row < field_0x1ec)
        return field_0x214[row].field_0x14;
    return 0;
}

// 0x004777f0
void UnknownGameUiControl::UnknownFunction4777f0(int (*compare)(const void* a, const void* b)) {
    field_0x24c = compare;
}

// 0x00477b60
int UnknownFunction477b60(const void* a, const void* b) {
    const char* left = ((const UnknownGameUiListRow*)a)->field_0x14;
    if (!left)
        return 1;
    const char* right = ((const UnknownGameUiListRow*)b)->field_0x14;
    if (!right)
        return -1;
    return _stricmp(left, right);
}

// 0x00477b90
void UIListBox::UnknownFunction477b90(int value) {
    field_0x21c = value;
}

// 0x00477ba0
void UIListBox::UnknownFunction477ba0(int value) {
    field_0x228 = value;
}

// 0x00477bb0
void UnknownGameUiControl::UnknownFunction477bb0(int a) {
    field_0x224 = a;
}

// 0x00477ce0
void UIListBox::UnknownVirtualSlot50() {
    UnknownGameUiControl::UnknownVirtualSlot50();
    if (field_0xb8 && field_0xb8->field_0x110) {
        for (int i = 0; i < field_0x1ec; i++)
            field_0x214[i].field_0x30 = 0;
    }
}

// 0x00477e60
void UnknownGameUiControl::UnknownFunction477e60(int a) {
    field_0x240 = a;
    if (!field_0x200)
        field_0x200 = 1;
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
    UnknownGameUiControl::UnknownVirtualSlot57(a, position);
}

// 0x00479220
void UIListBox::UnknownVirtualSlot29(int state) {
    field_0x1c0 = 1;
    field_0x60 = state;
}

// ---------------------------------------------------------------------------
// UIMultiState

// 0x004781f0
UIMultiState::UIMultiState(int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UnknownGameUiControl(2, id, area, owner) {
    field_0x1ec = 0;
    field_0x1f0 = 0;
    field_0x1f4_states = 0;
    UnknownFunction478860(2);
}

// 0x00478260
int UIMultiState::UnknownVirtualSlot34() {
    return (int)field_0x1f4_states[field_0x1f0].field_0x08;
}

// 0x00478280
int UIMultiState::UnknownVirtualSlot35() {
    return field_0x1f4_states[field_0x1f0].field_0x0c;
}

// 0x004782a0
int UIMultiState::UnknownVirtualSlot36() {
    return (int)field_0x1f4_states[field_0x1f0].field_0x1c;
}

// 0x004782c0
void* UIMultiState::UnknownVirtualSlot37() {
    return field_0x1f4_states[field_0x1f0].field_0x14;
}

// 0x00478300
UIMultiState::~UIMultiState() {
    if (field_0x1ec && (!field_0xb8 || !field_0xb8->field_0x9b0)) {
        for (int i = 0; i < field_0x1ec; i++) {
            UIAnim* image = field_0x1f4_states[i].field_0x00;
            if (image && !image->field_0xf4)
                operator delete(image, __FILE__, 0x210e);
            image = field_0x1f4_states[i].field_0x04;
            if (image && !image->field_0xf4)
                operator delete(image, __FILE__, 0x210f);
            if (field_0x1f4_states[i].field_0x08)
                operator delete(field_0x1f4_states[i].field_0x08, __FILE__, 0x2110);
            if (field_0x1f4_states[i].field_0x1c)
                operator delete(field_0x1f4_states[i].field_0x1c, __FILE__, 0x2111);
        }
    }
    if (field_0x1f4_states)
        operator delete(field_0x1f4_states, __FILE__, 0x2114);
}

// 0x00478450
void UIMultiState::UnknownVirtualSlot38() {
    UnknownGameUiControl* last = UnknownFunction470850(0);
    if (!field_0x70 || (last && (!last->field_0x70 || last->field_0x68))) {
        if (--field_0x1bc < 0) {
            field_0x1bc = 0;
            field_0x1c0 = 1;
        }
        return;
    }
    UIAnim* image;
    if (!field_0x1f4_states || !(image = field_0x1f4_states[field_0x1f0].field_0x00)) {
        field_0x1c0 = 1;
        return;
    }
    if (image->field_0x10 == 1)
        field_0x1c0 = 1;
    else
        field_0x1c0 = 0;
}

// 0x004784e0
void UIMultiState::UnknownVirtualSlot27() {
    if (!field_0x1f4_states)
        return;
    UIAnim* image;
    if (field_0x60 == 4)
        image = field_0x1f4_states[field_0x1f0].field_0x04;
    else
        image = field_0x1f4_states[field_0x1f0].field_0x00;
    if (image)
        image->UnknownFunction472fe0();
}

// 0x00478520
void UIMultiState::UnknownVirtualSlot29(int state) {
    field_0x1bc = 3;
    field_0x60 = state;
}

// 0x00478540
void UIMultiState::UnknownVirtualSlot59(int value) {
    if (value)
        *field_0x1b4 = UnknownFunction4755c0();
    else
        UnknownFunction478cf0(*field_0x1b4);
}

// 0x004789f0
void UIMultiState::UnknownFunction4789f0(int index, UIAnim* image, const char* text) {
    if (index < field_0x1ec && image) {
        UIAnim* old = field_0x1f4_states[index].field_0x00;
        if (old && !old->field_0xf4)
            old->Release();
        field_0x1f4_states[index].field_0x00 = image;
    }
    if (text)
        UnknownFunction478ad0(index, text);
}

// 0x00478cf0
void UnknownGameUiControl::UnknownFunction478cf0(int value) {
    if (value < field_0x1ec)
        field_0x1f0 = value;
    UnknownVirtualSlot50();
}

// 0x00478d10
void UIMultiState::UnknownFunction478d10() {
    UnknownGameUiState* states = field_0x1f4_states;
    if (!states)
        return;
    int start = field_0x1f0;
    int count = field_0x1ec;
    int index = (start + 1) % count;
    field_0x1f0 = index;
    while (!states[index].field_0x10 && index != start) {
        index = (index + 1) % count;
        field_0x1f0 = index;
    }
    UnknownVirtualSlot50();
}

// ---------------------------------------------------------------------------
// UIStaticText

// 0x00478ff0
UIStaticText::UIStaticText(int id, CameraRect* area, UnknownGameUiDialog* owner, const char* text,
                           unsigned int color)
    : UnknownGameUiControl(0xc, id, area, owner) {
    field_0xc4 = color;
    if (text) {
        field_0xc0 = (char*)DebugMalloc(strlen(text) + 1, __FILE__, 0x22ec);
        strcpy(field_0xc0, text);
        field_0xd4 = 0;
    } else {
        field_0xc0 = 0;
        field_0xd4 = 0;
    }
}

// 0x004790f0
UIStaticText::~UIStaticText() {
}

// 0x00479100
void UIStaticText::UnknownVirtualSlot38() {
    int empty = 0;
    if (!field_0xc0 || !field_0xd0)
        empty = 1;
    UnknownGameUiControl* last = UnknownFunction470850(0);
    if (!field_0x70 || empty || (last && (!last->field_0x70 || last->field_0x68))) {
        if (--field_0x1bc < 0) {
            field_0x1bc = 0;
            field_0x1c0 = 1;
        }
    }
}

// 0x00479170
int UIStaticText::UnknownVirtualSlot40() {
    UnknownGameUiControl::UnknownVirtualSlot40();
    field_0x1c0 = 1;
    return 1;
}

// 0x00479190
int UIStaticText::UnknownVirtualSlot55(int a, int b) {
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.field_0x20 = a;
        event.field_0x08 = 1;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x10 = field_0xbc;
        event.field_0x14 = this;
        field_0xb8->UnknownVirtualSlot29(&event);
        if (event.field_0x20)
            return 1;
    }
    return UnknownGameUiControl::UnknownVirtualSlot55(a, b);
}

// ---------------------------------------------------------------------------
// UIRadioButton

// 0x00479240
UIRadioButton::UIRadioButton(int id, CameraRect* area, UnknownGameUiDialog* owner)
    : UIMultiState(id, area, owner) {
    UnknownFunction469ce0(this);
    field_0x5c = 4;
}

// 0x004792d0
UIRadioButton::~UIRadioButton() {
}

// 0x004792e0
void UIRadioButton::UnknownVirtualSlot59(int value) {
    if (value)
        *field_0x1b4 = UnknownFunction4793f0();
    else
        UnknownFunction479310(*field_0x1b4);
}

// 0x004795c0
int UIRadioButton::UnknownVirtualSlot53() {
    return (int)new(__FILE__, 0x23b9) GameObjectIterator(field_0xb8->field_0x7f3c, 1, "UIRadioButton");
}

// ---------------------------------------------------------------------------
// The drop-down list's parts

// 0x00479640
UIDDLScrollBar::UIDDLScrollBar(int type, int id, CameraRect* area, UnknownGameUiDialog* owner,
                               UIDropDownList* list)
    : UIScrollBar(type, id, area, owner) {
    field_0x220_list = list;
    UnknownFunction469ce0(this);
}

// 0x004796e0
void UIDDLScrollBar::UnknownVirtualSlot33(int value) {
    UnknownGameUiControl::UnknownVirtualSlot33(value);
    if (!field_0x220_list->UnknownFunction47a800((UnknownGameUiControl*)value))
        field_0x220_list->UnknownFunction47a2d0(0);
}

// 0x00479bc0
void UIDDLStatic::UnknownVirtualSlot33(int value) {
    UnknownGameUiControl::UnknownVirtualSlot33(value);
    if (!field_0x1ec_list->UnknownFunction47a800((UnknownGameUiControl*)value))
        field_0x1ec_list->UnknownFunction47a2d0(0);
}

// 0x00479c90
int UIDDLButton::UnknownVirtualSlot55(int a, int b) {
    UnknownGameUiControl::UnknownVirtualSlot55(a, b);
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this)
        field_0x1f0_list->UnknownFunction47a2d0(1 - field_0x1f0_list->field_0x208);
    return 0;
}

// 0x00479ce0
void UIDDLButton::UnknownVirtualSlot33(int value) {
    UnknownGameUiControl::UnknownVirtualSlot33(value);
    if (!field_0x1f0_list->UnknownFunction47a800((UnknownGameUiControl*)value))
        field_0x1f0_list->UnknownFunction47a2d0(0);
}

// 0x00479d10
UIDDLListBox::UIDDLListBox(int id, CameraRect* area, UnknownGameUiDialog* owner, int rows,
                           UIDropDownList* list)
    : UIListBox(id, area, owner, rows) {
    field_0x250 = list;
    UnknownFunction469ce0(this);
}

// 0x00479db0
int UIDDLListBox::UnknownVirtualSlot65(int row) {
    int result = UIListBox::UnknownVirtualSlot65(row);
    if (result)
        field_0x250->UnknownFunction470b20(UnknownFunction476d20(row));
    return result;
}

// 0x00479e70
void UIDDLListBox::UnknownVirtualSlot33(int value) {
    UnknownGameUiControl::UnknownVirtualSlot33(value);
    if (!field_0x250->UnknownFunction47a800((UnknownGameUiControl*)value))
        field_0x250->UnknownFunction47a2d0(0);
}

// ---------------------------------------------------------------------------
// UIDropDownList

// 0x0047a1f0
UIDropDownList::~UIDropDownList() {
    if (field_0x1b4)
        UnknownVirtualSlot59(1);
    if (!field_0x208) {
        field_0x1f0_control->Release();
        field_0x1f4_control->Release();
        field_0x1fc->Release();
        field_0x200_control->Release();
    }
}

// 0x0047a290
void UIDropDownList::UnknownVirtualSlot52(int id) {
    field_0x7c = id;
    if (!id)
        id = 0x4d43;
    field_0x1fc->UnknownVirtualSlot52(id);
    field_0x200_control->UnknownVirtualSlot52(id);
}

// 0x0047a7d0
void UIDropDownList::UnknownVirtualSlot33(int value) {
    UnknownGameUiControl::UnknownVirtualSlot33(value);
    if (!UnknownFunction47a800((UnknownGameUiControl*)value))
        UnknownFunction47a2d0(0);
}

// 0x0047a800
int UIDropDownList::UnknownFunction47a800(UnknownGameUiControl* control) {
    if (control == this || control == field_0x200_control || control == field_0x1ec_control ||
        control == field_0x1fc || control == field_0x1f0_control || control == field_0x1f4_control ||
        control == field_0x1f8_control)
        return 1;
    return 0;
}

// 0x0047a850
int UIDropDownList::UnknownVirtualSlot61() {
    if (!field_0x1ec_control->field_0x3c[0])
        return field_0x3c[2] - field_0x3c[0];
    return field_0x1ec_control->field_0x3c[2] - field_0x3c[0];
}

// 0x0047a870
int UIDropDownList::UnknownVirtualSlot62() {
    return field_0x1fc->field_0x3c[3] - field_0x3c[1];
}

// 0x0047ad80
void UIDropDownList::UnknownVirtualSlot59(int value) {
    if (value) {
        *field_0x1b4 = field_0x1fc->UnknownFunction476950();
    } else {
        field_0x1fc->UnknownFunction476a60(*field_0x1b4);
        field_0x1fc->UnknownVirtualSlot66(0);
    }
}

// 0x0047add0
int UIDropDownList::UnknownVirtualSlot55(int a, int b) {
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this) {
        UnknownFunction47a2d0(field_0x208 == 0);
        field_0xb8->field_0x34->UnknownFunction487790((UnknownGuiControl*)this, 0, 0);
        return 1;
    }
    return UnknownGameUiControl::UnknownVirtualSlot55(a, b);
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
        operator delete(block, __FILE__, 0x2782);
        return 0;
    }
    void* resized = DebugMalloc(size, __FILE__, 0x2787);
    if (resized && block) {
        int length = _msize(block);
        memcpy(resized, block, length < (int)size ? length : size);
        operator delete(block, __FILE__, 0x278e);
    }
    return resized;
}

// ---------------------------------------------------------------------------
// UIDialog

// 0x0046a010
int UIDialog::Release() {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    UnknownDialogEvent event;
    event.field_0x20 = 0;
    UnknownFunction46ecc0(1);
    dialog->UnknownVirtualSlot31(1);
    event.field_0x10 = (GUIManager*)dialog->field_0x30;
    event.field_0x08 = 6;
    event.field_0x00 = 0;
    event.field_0x04 = 0;
    event.field_0x0c = this;
    event.field_0x14 = 0;
    dialog->UnknownVirtualSlot29(&event);
    return GameObject::Release();
}

// 0x0046a780
int UIDialog::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (event->kind && (unsigned int)event->control < 0x20)
        dialog->field_0x34->field_0x3c[event->control] = 1;
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
        dialog->field_0x34->field_0x3c[event->control] = 0;
    if (!field_0x25_bit2) {
        if (event->kind == 1)
            UnknownFunction46f120();
        if (event->kind) {
            UnknownFunction46fe40(0x101);
            UnknownFunction46fe40(0x102);
        }
        return GameObject::UnknownVirtualSlot22(event, entry);
    }
    return 0;
}

// 0x0046a840
UnknownGameUiControl* UIDialog::UnknownFunction46a840(UnknownGameUiControl* control, int group, int region) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (control) {
        control->UnknownInlineSetField18(field_0x18);
        control->field_0x1dc = dialog->field_0x7f18;
        control->field_0xbc = dialog->field_0x30;
        control->field_0x78 = group;
        control->field_0x160 = dialog->field_0xdc;
        if (dialog->field_0x110)
            control->field_0x1b8 = dialog->field_0x110->UnknownFunction4040f0(region);
        return control;
    }
    return 0;
}

// 0x0046a8a0
GameObject* UIDialog::UnknownFunction46a8a0(GameObject* control, int a, int b) {
    if (control) {
        UnknownFunction46a840((UnknownGameUiControl*)control, a, b);
        ((UnknownGameUiDialog*)this)->field_0x7f3c->UnknownFunction469190(control, -1);
        return control;
    }
    return 0;
}

// 0x0046e9a0
Sound* UIDialog::UnknownFunction46e9a0(const char* name) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (!name)
        return 0;
    for (int i = 0; i < dialog->field_0x7efc; i++) {
        if (!_stricmp(name, dialog->field_0x9cc[i].field_0x00))
            return dialog->field_0x9cc[i].field_0x34;
    }
    if (dialog->field_0x30)
        return (Sound*)dialog->field_0x30->UnknownFunction485ec0((int)name);
    return 0;
}

// 0x0046ea60
void UIDialog::UnknownFunction46ea60(int value) {
    ((UnknownGameUiDialog*)this)->field_0xbc = value;
    if (value)
        GameObject::UnknownVirtualSlot5();
    else
        GameObject::UnknownVirtualSlot4();
}

// 0x0046eb30
void UIDialog::UnknownFunction46eb30(int id, int value) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    GameObjectIterator iterator(dialog->field_0x7f3c, 1, "UIControl");
    UnknownGameUiControl* control;
    while ((control = (UnknownGameUiControl*)iterator.Next()) != 0) {
        if (control->field_0x78 == id)
            control->UnknownFunction470660(value, 1);
    }
    if (!value && dialog->field_0x110)
        dialog->field_0x110->UnknownFunction404da0();
}

// 0x0046ebf0
UnknownGameUiControl* UIDialog::UnknownFunction46ebf0(const char* name, int flags) {
    UnknownGameUiControl* found = 0;
    GameObjectIterator iterator(((UnknownGameUiDialog*)this)->field_0x7f3c, 1, "UIControl");
    UnknownGameUiControl* control = (UnknownGameUiControl*)iterator.Next();
    while (control) {
        if (!_stricmp(control->UnknownFunction470df0(), name)) {
            if (!flags || control->field_0x5c == flags || (flags == 8 && control->field_0x5c == 7))
                found = control;
            break;
        }
        control = (UnknownGameUiControl*)iterator.Next();
    }
    return found;
}

// 0x0046ecc0
void UIDialog::UnknownFunction46ecc0(int value) {
    GameObjectIterator iterator(((UnknownGameUiDialog*)this)->field_0x7f3c, 1, "UIControl");
    UnknownGameUiControl* control;
    while ((control = (UnknownGameUiControl*)iterator.Next()) != 0) {
        if (control->field_0x1b4)
            control->UnknownVirtualSlot59(value);
    }
    ((UnknownGameUiDialog*)this)->UnknownVirtualSlot31(value);
}

// 0x0046f1c0
int UIDialog::UnknownVirtualSlot13() {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (!field_0x25_bit2) {
        GameObjectIterator iterator(dialog->field_0x7f3c, 1, "UIControl");
        if (dialog->field_0x110) {
            UnknownGameUiControl* control;
            while ((control = (UnknownGameUiControl*)iterator.Next()) != 0) {
                if (control->field_0x1b8 >= 0 && (!control->field_0x1c0 || control->field_0x1bc))
                    dialog->field_0x110->UnknownFunction404240(control->field_0x1b8,
                                                               (CameraRect*)&control->field_0x2c);
            }
        }
        GameObject::UnknownVirtualSlot13();
    } else if (dialog->field_0x7f44 && !dialog->field_0x7f18) {
        if (dialog->field_0x110)
            dialog->field_0x110->UnknownFunction404480(dialog->field_0x7f44, &dialog->field_0x160, 0, 0x1000000,
                                                       dialog->field_0x7f48, 1, &dialog->field_0x7f4c, 0);
        else
            ((RenderTarget*)dialog->UnknownInlineField18())->UnknownVirtualSlot3(&dialog->field_0x160,
                                                                                 dialog->field_0x7f44, 0, 0x1000000);
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
    } else if (dialog->field_0x7f44 && dialog->field_0x7f18) {
        if (dialog->field_0x110)
            dialog->field_0x110->UnknownFunction404480(dialog->field_0x7f44, &dialog->field_0x160, 0, 0x1000000,
                                                       dialog->field_0x7f48, 1, &dialog->field_0x7f4c, 0);
        else
            ((RenderTarget*)dialog->UnknownInlineField18())->UnknownVirtualSlot3(&dialog->field_0x160,
                                                                                 dialog->field_0x7f44, 0, 0x1000000);
    }
    if (dialog->field_0x7f50 && dialog->field_0x7f54 != ((RenderTarget*)dialog->UnknownInlineField18())->field_0x1c)
        UnknownVirtualSlot26();
    return 1;
}

// 0x0046fe40
void UIDialog::UnknownFunction46fe40(int id) {
    UITimer* timer;
    for (int i = 0; (timer = ((UnknownGameUiDialog*)this)->field_0x7f24.Get(i)) != 0; i++) {
        if (timer->field_0x08 == id)
            ((UnknownGameUiDialog*)this)->field_0x7f24.Remove(timer);
    }
}

// 0x0046fec0
void UIDialog::UnknownFunction46fec0(void* timer) {
    UITimer* entry;
    for (int i = 0; (entry = ((UnknownGameUiDialog*)this)->field_0x7f24.Get(i)) != 0; i++) {
        if (entry == timer)
            ((UnknownGameUiDialog*)this)->field_0x7f24.Remove((UITimer*)timer);
    }
}

// 0x0046ff30
void UIDialog::UnknownFunction46ff30(int result) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (!dialog->field_0x7f50) {
        dialog->field_0x17c = result;
        dialog->field_0x7f50 = 1;
        dialog->field_0x7f54 = ((RenderTarget*)field_0x18)->field_0x1c;
    }
}

// 0x0046ff60
void UIDialog::UnknownFunction46ff60() {
    ((UnknownGameUiDialog*)this)->field_0x154 = 0;
    UnknownVirtualSlot26();
}

// 0x0046ffc0
void UIDialog::UnknownFunction46ffc0(int value) {
    ((UnknownGameUiDialog*)this)->field_0x170 = value;
}

// 0x0046ffd0
void UIDialog::UnknownFunction46ffd0(void* background) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (dialog->field_0x2c) {
        dialog->field_0x2c->UnknownFunction46ffd0(background);
        dialog->field_0x110 = (BackgroundImage*)background;
    } else {
        dialog->field_0x110 = (BackgroundImage*)background;
    }
}

// 0x00470000
int UIDialog::UnknownFunction470000(UnknownGameUiControl* control, int a, int b) {
    return ((UnknownGameUiDialog*)this)->field_0x34->UnknownFunction487730((UnknownGuiControl*)control,
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
        UnknownFunction4700b0(b, rect);
    else
        UnknownFunction470110();
    GameObject::UnknownVirtualSlot16(a);
}

// 0x004700b0
void UIDialog::UnknownFunction4700b0(int dim, CameraRect* rect) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    UnknownFunction470110();
    if (!rect)
        rect = &dialog->field_0x160;
    dialog->field_0x7f44 = dialog->field_0x30->UnknownFunction486170(dim, rect);
    if (dialog->field_0x110) {
        dialog->field_0x7f48 = dialog->field_0x110->UnknownFunction4040f0(1);
        dialog->field_0x110->UnknownFunction404da0();
        dialog->field_0x110->field_0x30 = 0;
    }
}

// 0x00470110
void UIDialog::UnknownFunction470110() {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (dialog->field_0x7f44)
        dialog->field_0x7f44->Release();
    if (dialog->field_0x110 && dialog->field_0x7f48 >= 0) {
        dialog->field_0x110->UnknownFunction404200(dialog->field_0x7f48);
        dialog->field_0x110->field_0x30 = 1;
    }
    dialog->field_0x7f44 = 0;
    dialog->field_0x7f48 = -1;
    dialog->field_0x7f4c = 0;
}

// ---------------------------------------------------------------------------
// UIControl: text and transitions

// 0x00470660
void UnknownGameUiControl::UnknownFunction470660(int a, int b) {
    if (field_0x1d0) {
        if (a) {
            field_0x68 = 1;
            if (field_0xb4 == 0x6a)
                field_0x1d0 = &UnknownGameUiControl::UnknownVirtualSlot46;
            field_0x1cc = 0;
            field_0x70 = a;
            return;
        }
        if (field_0xb4 == 0x6a && b && field_0xb8->field_0xc8) {
            field_0x1cc = 0;
            field_0x68 = 2;
            field_0x1d0 = &UnknownGameUiControl::UnknownVirtualSlot47;
            return;
        }
        field_0x68 = 2;
        field_0x70 = 0;
        UnknownVirtualSlot50();
        field_0x1cc = 0;
        return;
    }
    field_0x68 = 0;
    field_0x70 = a;
    UnknownVirtualSlot50();
}

// 0x00470760
void UnknownGameUiControl::UnknownFunction470760(int a, const char* image) {
    if (field_0xb8) {
        UIAnim* anim = new(__FILE__, 0xc10) UIAnim(field_0xb8->field_0x7f20, 0);
        anim->UnknownFunction4730e0(image, field_0xb8->field_0x140);
        UnknownFunction470730(a, anim);
    }
    UnknownVirtualSlot50();
}

// 0x00470870
void UnknownGameUiControl::UnknownFunction470870(int type, int a, int b, int delay, int c) {
    if (b) {
        UnknownFunction470660(1, 1);
        field_0x68 = 1;
    }
    field_0xb4 = type;
    switch (type) {
    case 100:
        field_0x1d0 = &UnknownGameUiControl::UnknownVirtualSlot41;
        break;
    case 101:
        field_0x1d0 = &UnknownGameUiControl::UnknownVirtualSlot42;
        break;
    case 102:
        field_0x1d0 = &UnknownGameUiControl::UnknownVirtualSlot43;
        break;
    case 103:
        field_0x1d0 = &UnknownGameUiControl::UnknownVirtualSlot44;
        break;
    case 104:
        field_0x1d0 = &UnknownGameUiControl::UnknownVirtualSlot45;
        break;
    case 106:
        field_0x1d0 = &UnknownGameUiControl::UnknownVirtualSlot46;
        break;
    default:
        field_0x1d0 = 0;
        break;
    }
    field_0xa8 = 0;
    field_0xb0 = c;
    if (delay == 0xffff)
        field_0xac = rand() % 2000;
    else
        field_0xac = delay;
    field_0x1a8 = (Sound*)a;
    UnknownVirtualSlot50();
}

// 0x00470a80
void UnknownGameUiControl::UnknownFunction470a80(void* module, int id) {
    char text[0x400];
    if (LoadStringA((HINSTANCE)module, id, text, 0x400)) {
        if (field_0x5c == 0xb)
            UnknownFunction473da0(text);
        else
            UnknownFunction470b20(text);
    } else {
        sprintf(text, "Resource string '%d' load fail\n", id);
        if (field_0x5c == 0xb)
            UnknownFunction473da0(text);
        else
            UnknownFunction470b20("Resource String Unavailable");
    }
    UnknownVirtualSlot50();
}

// 0x00470b20
void UnknownGameUiControl::UnknownFunction470b20(const char* text) {
    if (text) {
        if (field_0x5c == 0xb) {
            UnknownFunction473da0((char*)text);
        } else {
            if (field_0xc0)
                operator delete(field_0xc0, __FILE__, 0xcc2);
            if (field_0xe4)
                operator delete(field_0xe4, __FILE__, 0xcc3);
            field_0xc0 = (char*)DebugMalloc(strlen(text) + 1, __FILE__, 0xcc4);
            strcpy(field_0xc0, text);
            field_0xe4 = field_0xb8->UnknownFunction46f890(field_0xc0, field_0xc4, (void*)field_0x128,
                                                           field_0x1c4, field_0xdc);
            if (!field_0xe4) {
                field_0xd0 = field_0xdc[0] = strlen(field_0xc0);
                HDC dc;
                if (!g_UnknownGlobal56e26c->PCTarget()->field_0x48->UnknownMethod17((void**)&dc)) {
                    HGDIOBJ font =
                        (HGDIOBJ)(field_0x128 ? field_0x128 : (field_0xb8 ? (int)field_0xb8->field_0xd8 : 0));
                    HGDIOBJ old = SelectObject(dc, font);
                    SIZE size;
                    GetTextExtentPoint32A(dc, field_0xc0, field_0xd0, &size);
                    SelectObject(dc, old);
                    g_UnknownGlobal56e26c->PCTarget()->field_0x48->UnknownMethod26(dc);
                    field_0xdc[1] = size.cx;
                } else {
                    field_0xdc[1] = 0;
                }
            }
        }
        UnknownVirtualSlot50();
    } else {
        if (field_0xc0)
            operator delete(field_0xc0, __FILE__, 0xcde);
        if (field_0xe4)
            operator delete(field_0xe4, __FILE__, 0xcdf);
        field_0xc0 = 0;
        field_0xe4 = 0;
    }
}

// 0x00470e00
int UnknownGameUiControl::UnknownVirtualSlot10(float frameTime) {
    UnknownVirtualSlot27();
    int draw = 1;
    if (field_0x70) {
        UnknownGameUiControl* last = UnknownFunction470850(0);
        if (!last || (last->field_0x70 && !last->field_0x68))
            draw = !UnknownFunction471500();
    }
    if (draw) {
        field_0x1cc = UnknownVirtualSlot48(-1);
        UnknownVirtualSlot38();
        UnknownVirtualSlot39();
        if (field_0xb8->field_0x34->field_0x2c && field_0x1d4 != -1) {
            POINT position = field_0xb8->field_0x34->field_0x2c->field_0xa4;
            UnknownVirtualSlot57(field_0x1d4, (int*)&position);
        }
    }
    field_0x2c[0] = field_0x2c[0] < 0 ? 0 : field_0x2c[0];
    field_0x2c[1] = field_0x2c[1] < 0 ? 0 : field_0x2c[1];
    int right = field_0x2c[2];
    field_0x2c[2] = g_UnknownGlobal56e26c->field_0x10->field_0x0c < right ? g_UnknownGlobal56e26c->field_0x10->field_0x0c : right;
    int bottom = field_0x2c[3];
    field_0x2c[3] = g_UnknownGlobal56e26c->field_0x10->field_0x10 < bottom ? g_UnknownGlobal56e26c->field_0x10->field_0x10 : bottom;
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x004711a0
int UnknownGameUiControl::UnknownVirtualSlot15() {
    int result = GameObject::UnknownVirtualSlot15();
    UnknownGameUiControl* last = UnknownFunction470850(0);
    if ((!last || (last->field_0x70 && !last->field_0x68)) && field_0x1dc) {
        if (field_0x70)
            result = UnknownVirtualSlot40();
        else
            field_0x1c0 = 1;
    }
    BackgroundImage* background = field_0xb8->field_0x110;
    if (background && field_0x1b8 > -1 &&
        (!field_0x70 || (last && (!last->field_0x70 || last->field_0x68))))
        background->UnknownFunction404cb0(field_0x1b8);
    return result;
}

// 0x00471500
int UnknownGameUiControl::UnknownFunction471500() {
    int done = 1;
    UnknownGameUiDialog* owner = field_0xb8;
    int (UnknownGameUiControl::*step)();
    if (owner->field_0xc8 && (step = field_0x1d0) != 0 && field_0x68 && field_0x5c != 3 && field_0x5c != 0xc) {
        field_0xa8 += (int)(owner->field_0x9c0 * 1000.0f);
        done = 0;
        if (field_0xa8 < field_0xac)
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
int UnknownGameUiControl::UnknownVirtualSlot55(int a, int b) {
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this) {
        if (!a) {
            UnknownVirtualSlot29(2);
            UnknownVirtualSlot58(2, field_0xb8->field_0x180);
        }
        field_0x1d4 = a;
        if (a == 1 && field_0xb8->field_0x110)
            field_0xb8->field_0x110->UnknownFunction404da0();
        field_0xb8->field_0x34->UnknownFunction487790((UnknownGuiControl*)this, 0, 0);
        ToolTip* tip = field_0xb8->field_0x34->field_0xc0;
        if (tip) {
            tip->field_0x64 = 0;
            tip->field_0x60 = -1.0f;
        }
    }
    return 0;
}

// 0x00472320
int UnknownGameUiControl::UnknownVirtualSlot20(int value) {
    UnknownDialogEvent event;
    event.field_0x20 = 0;
    if (field_0xb8->field_0x34->field_0x1d4 == (UnknownGuiControl*)this) {
        event.field_0x08 = 0xb;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x10 = field_0xbc;
        event.field_0x14 = this;
        event.field_0x1c = value;
        event.field_0x18 = 1;
        field_0xb8->UnknownVirtualSlot29(&event);
        if (event.field_0x20)
            return 1;
        if (!event.field_0x18)
            return 1;
    }
    return GameObject::UnknownVirtualSlot20(value);
}

// 0x004723d0
int UnknownGameUiControl::UnknownVirtualSlot21(int value) {
    UnknownDialogEvent event;
    event.field_0x20 = 0;
    if (field_0xb8->field_0x34->field_0x1d4 == (UnknownGuiControl*)this) {
        event.field_0x08 = 0xc;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x10 = field_0xbc;
        event.field_0x14 = this;
        event.field_0x1c = value;
        event.field_0x18 = 1;
        field_0xb8->UnknownVirtualSlot29(&event);
        if (event.field_0x20)
            return 1;
        if (!event.field_0x18)
            return 1;
    }
    return GameObject::UnknownVirtualSlot21(value);
}

// 0x00472480
int UnknownGameUiControl::UnknownFunction472480(int* position) {
    int result = 0;
    GameObject* next = field_0x0C;
    while (next) {
        if (strstr(next->field_0x28, "UIControl,"))
            break;
        next = next->field_0x0C;
    }
    if (next) {
        result = ((UnknownGameUiControl*)next)->UnknownFunction472480(position);
        if (result)
            return result;
    }
    if (field_0x10 && strstr(field_0x10->field_0x28, "UIControl,")) {
        result = ((UnknownGameUiControl*)field_0x10)->UnknownFunction472480(position);
        if (result)
            return result;
    }
    if (UnknownVirtualSlot51(*(POINT*)position) && UnknownVirtualSlot30()) {
        UnknownGameUiControl* last = UnknownFunction470850(0);
        if (field_0x6c && field_0x70 && (!last || (last->field_0x70 && !last->field_0x68))) {
            GUIUser* user = field_0xb8->field_0x34;
            UnknownGuiControl* hover = user->field_0x1d8;
            if (!user->field_0x3c[0] && !user->field_0x3c[1] && hover != (UnknownGuiControl*)this) {
                result = field_0xb8->UnknownFunction470000(this, 0, 0);
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
UnknownGameUiControl* UnknownGameUiControl::UnknownFunction4727c0() {
    UnknownGameUiControl* control = this;
    while (control) {
        if (control != this && control->field_0x6c && control->field_0x70 && control->field_0x5c != 5 &&
            control->field_0x5c != 0xc)
            return control;
        if (control->field_0x08) {
            control = (UnknownGameUiControl*)control->field_0x08;
        } else {
            while (control->field_0x0C)
                control = (UnknownGameUiControl*)control->field_0x0C;
        }
        if (control == this)
            return 0;
    }
    return control;
}

// 0x00472810
UnknownGameUiControl* UnknownGameUiControl::UnknownFunction472810() {
    UnknownGameUiControl* control = this;
    while (control) {
        if (control != this && control->field_0x6c && control->field_0x70 && control->field_0x5c != 5 &&
            control->field_0x5c != 0xc)
            return control;
        control = (UnknownGameUiControl*)control->field_0x0C;
        if (!control)
            control = (UnknownGameUiControl*)field_0xb8->field_0x7f3c->field_0x10;
        if (control == this)
            return 0;
    }
    return control;
}

// ---------------------------------------------------------------------------
// UIListBox: rows

// 0x004769e0
int UIListBox::UnknownVirtualSlot65(int row) {
    if ((field_0x1ec && row < field_0x1ec && row >= 0) || (!field_0x1ec && !row)) {
        field_0x1f4 = row;
        if (row >= field_0x1f0) {
            int last = field_0x200 + field_0x1f0 - 1;
            if (row <= (last < 0 ? 0 : last))
                goto shown;
            int first = row - field_0x200 + 1;
            row = first < 0 ? 0 : first;
        }
        field_0x1f0 = row;
    shown:
        UnknownVirtualSlot50();
        return 1;
    }
    return 1;
}

// 0x00476a60
int UnknownGameUiControl::UnknownFunction476a60(int row) {
    int result = UnknownVirtualSlot65(row);
    UnknownFunction477bc0();
    if (field_0xb8) {
        GameObjectIterator* iterator = (GameObjectIterator*)UnknownVirtualSlot53();
        UnknownGameUiControl* control;
        while ((control = UnknownFunction472790(iterator)) != 0)
            control->UnknownVirtualSlot65(row);
        UnknownFunction472730(iterator);
    }
    return result;
}

// 0x00476ad0
int UnknownGameUiControl::UnknownFunction476ad0(const char* text) {
    for (int i = 0; i < field_0x1ec; i++) {
        if (field_0x214[i].field_0x00 == 1 && !_stricmp(text, field_0x214[i].field_0x14))
            return UnknownFunction476a60(i);
    }
    return 0;
}

// 0x00476b30
int UnknownGameUiControl::UnknownFunction476b30(int data) {
    for (int i = 0; i < field_0x1ec; i++) {
        if (UnknownFunction4768d0(i) == data)
            return UnknownFunction476a60(i);
    }
    return 0;
}

// 0x00476ba0
void UnknownGameUiControl::UnknownFunction476ba0(unsigned int color, int row) {
    if (row == -1) {
        if (field_0x20c)
            DeleteObject((HGDIOBJ)field_0x20c);
        field_0x20c = color != 0xff000000 ? CreateSolidBrush(color) : 0;
    } else if (row < field_0x1ec) {
        void* brush = field_0x214[row].field_0x34;
        if (brush) {
            for (int i = 0; i < field_0x1ec; i++) {
                if (field_0x214[i].field_0x34 == brush)
                    goto shared;
            }
            DeleteObject((HGDIOBJ)brush);
        }
    shared:
        field_0x214[row].field_0x34 = color != 0xff000000 ? CreateSolidBrush(color) : 0;
    }
    UnknownVirtualSlot50();
}

// 0x00476c70
void UnknownGameUiControl::UnknownFunction476c70(unsigned int color, int row) {
    if (row == -1) {
        UnknownFunction470d40(color);
        UnknownVirtualSlot50();
        return;
    }
    if (row < field_0x1ec)
        field_0x214[row].field_0x28 = color;
    UnknownVirtualSlot50();
}

// 0x00476cd0
void UnknownGameUiControl::UnknownFunction476cd0(unsigned int color) {
    if (field_0x210)
        DeleteObject((HGDIOBJ)field_0x210);
    field_0x210 = color != 0xff000000 ? CreateSolidBrush(color) : 0;
    UnknownVirtualSlot50();
}

// 0x00476ee0
int UnknownGameUiControl::UnknownFunction476ee0() {
    int height = 0;
    int rows = 0;
    int i = field_0x1f0;
    int count = field_0x1ec;
    if (i < count) {
        int space = field_0x2c[3] - field_0x2c[1];
        for (; i < count; i++) {
            height += field_0x214[i].field_0x0c;
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
int UnknownGameUiControl::UnknownFunction476f50(int rows) {
    if (rows > field_0x1fc_value) {
        UnknownGameUiListRow* resized =
            (UnknownGameUiListRow*)UnknownFunction47b570(field_0x214, rows * sizeof(UnknownGameUiListRow));
        if (resized) {
            memset(&resized[field_0x1fc_value], 0, (rows - field_0x1fc_value) * sizeof(UnknownGameUiListRow));
            field_0x1fc_value = rows;
            field_0x214 = resized;
            return 1;
        }
        return 0;
    }
    return 1;
}

// 0x00476ff0
void UnknownGameUiControl::UnknownFunction476ff0(int row, const char* text) {
    if (row < field_0x1ec && row >= 0) {
        if (field_0x214[row].field_0x14)
            operator delete(field_0x214[row].field_0x14, __FILE__, 0x1e0b);
        if (field_0x214[row].field_0x24)
            operator delete(field_0x214[row].field_0x24, __FILE__, 0x1e0c);
        field_0x214[row].field_0x08 = strlen(text);
        field_0x214[row].field_0x14 = (char*)DebugMalloc(field_0x214[row].field_0x08 + 1, __FILE__, 0x1e0e);
        strcpy(field_0x214[row].field_0x14, text);
        field_0x214[row].field_0x24 = field_0xb8->UnknownFunction46f890(
            field_0x214[row].field_0x14, field_0xc4, (void*)field_0x128, field_0x1c4, 0);
    }
    UnknownVirtualSlot50();
}

// 0x00476d80
int UnknownGameUiControl::UnknownFunction476d80(const char* text, int data, int a) {
    if (text && (field_0x1ec < field_0x1fc_value || UnknownFunction476f50(field_0x1ec + 1))) {
        UnknownGameUiListRow* row = &field_0x214[field_0x1ec];
        row->field_0x08 = strlen(text);
        int height = field_0x160;
        if (!height)
            height = field_0xb8->field_0xdc;
        row->field_0x20 = 0;
        row->field_0x18 = 0;
        row->field_0x28 = 0;
        row->field_0x34 = 0;
        row->field_0x0c = height;
        row->field_0x00 = 1;
        row->field_0x04 = a;
        row->field_0x14 = (char*)DebugMalloc(row->field_0x08 + 1, __FILE__, 0x1dc7);
        strcpy(row->field_0x14, text);
        row->field_0x24 = field_0xb8->UnknownFunction46f890(row->field_0x14, field_0xc4, (void*)field_0x128,
                                                            field_0x1c4, 0);
        UnknownFunction476930(field_0x1ec, data);
        row->field_0x30 = 0;
        field_0x1ec++;
        field_0x200 = UnknownFunction476ee0();
        UnknownFunction477bc0();
        if (field_0x21c)
            UnknownFunction477900(1);
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// UIMultiState and UIRadioButton: states

// 0x00478860
int UnknownGameUiControl::UnknownFunction478860(int count) {
    if (field_0x1ec > count) {
        for (int i = count; i < field_0x1ec; i++) {
            UIAnim* image = field_0x1f4_states[i].field_0x00;
            if (image && !image->field_0xf4)
                operator delete(image, __FILE__, 0x21b6);
            image = field_0x1f4_states[i].field_0x04;
            if (image && !image->field_0xf4)
                operator delete(image, __FILE__, 0x21b7);
            if (field_0x1f4_states[i].field_0x08)
                operator delete(field_0x1f4_states[i].field_0x08, __FILE__, 0x21b8);
            if (field_0x1f4_states[i].field_0x1c)
                operator delete(field_0x1f4_states[i].field_0x1c, __FILE__, 0x21b9);
        }
        field_0x1f4_states = (UnknownGameUiState*)UnknownFunction47b570(field_0x1f4_states,
                                                                         count * sizeof(UnknownGameUiState));
    } else if (field_0x1ec < count) {
        field_0x1f4_states = (UnknownGameUiState*)UnknownFunction47b570(field_0x1f4_states,
                                                                         count * sizeof(UnknownGameUiState));
        for (int i = field_0x1ec; i < count; i++) {
            field_0x1f4_states[i].field_0x00 = 0;
            field_0x1f4_states[i].field_0x04 = 0;
            field_0x1f4_states[i].field_0x08 = 0;
            field_0x1f4_states[i].field_0x1c = 0;
            field_0x1f4_states[i].field_0x10 = 1;
        }
    }
    field_0x1f0 = 0;
    field_0x1ec = count;
    UnknownVirtualSlot50();
    return 1;
}

// 0x00478a50
void UnknownGameUiControl::UnknownFunction478a50(int index, void* module, int id) {
    char text[0x400];
    if (LoadStringA((HINSTANCE)module, id, text, 0x400)) {
        UnknownFunction478ad0(index, text);
    } else {
        sprintf(text, "Resource string '%d' load fail\n", id);
        UnknownFunction478ad0(index, "Resource String Unavailable");
    }
    UnknownVirtualSlot50();
}

// 0x00478ad0
void UnknownGameUiControl::UnknownFunction478ad0(int index, const char* text) {
    int current = field_0x1f0;
    if (index < field_0x1ec) {
        field_0x1f0 = index;
        if (field_0x1f4_states[index].field_0x08)
            operator delete(field_0x1f4_states[index].field_0x08, __FILE__, 0x2210);
        if (field_0x1f4_states[index].field_0x1c)
            operator delete(field_0x1f4_states[index].field_0x1c, __FILE__, 0x2211);
        if (text) {
            field_0x1f4_states[index].field_0x08 = (char*)DebugMalloc(strlen(text) + 1, __FILE__, 0x2213);
            strcpy(field_0x1f4_states[index].field_0x08, text);
            field_0x1f4_states[index].field_0x1c = field_0xb8->UnknownFunction46f890(
                field_0x1f4_states[index].field_0x08, field_0xc4, (void*)field_0x128, field_0x1c4,
                field_0x1f4_states[index].field_0x14);
            if (!field_0x1f4_states[index].field_0x1c) {
                field_0x1f4_states[index].field_0x14[0] = strlen(field_0x1f4_states[index].field_0x08);
                field_0x1f4_states[index].field_0x0c = field_0x1f4_states[index].field_0x14[0];
                HDC dc;
                if (!g_UnknownGlobal56e26c->PCTarget()->field_0x48->UnknownMethod17((void**)&dc)) {
                    HGDIOBJ font =
                        (HGDIOBJ)(field_0x128 ? field_0x128 : (field_0xb8 ? (int)field_0xb8->field_0xd8 : 0));
                    HGDIOBJ old = SelectObject(dc, font);
                    SIZE size;
                    GetTextExtentPoint32A(dc, field_0x1f4_states[index].field_0x08,
                                          field_0x1f4_states[index].field_0x0c, &size);
                    SelectObject(dc, old);
                    g_UnknownGlobal56e26c->PCTarget()->field_0x48->UnknownMethod26(dc);
                    field_0x1f4_states[index].field_0x14[1] = size.cx;
                } else {
                    field_0x1f4_states[index].field_0x14[1] = 0;
                }
            }
        } else {
            field_0x1f4_states[index].field_0x08 = 0;
            field_0x1f4_states[index].field_0x1c = 0;
        }
        field_0x1f0 = current;
    }
    UnknownVirtualSlot50();
}

// 0x00478d70
int UIMultiState::UnknownVirtualSlot55(int a, int b) {
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.field_0x20 = a;
        UnknownFunction478d10();
        event.field_0x08 = 1;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x10 = field_0xbc;
        event.field_0x14 = this;
        field_0xb8->UnknownVirtualSlot29(&event);
        if (event.field_0x20)
            return 1;
    }
    return UnknownGameUiControl::UnknownVirtualSlot55(a, b);
}

// 0x00479310
int UnknownGameUiControl::UnknownFunction479310(int index) {
    int count = 0;
    GameObjectIterator iterator(field_0xb8->field_0x7f3c, 1, "UIRadioButton");
    if (!index)
        UnknownFunction478cf0(1);
    UnknownGameUiControl* control;
    while ((control = (UnknownGameUiControl*)iterator.Next()) != 0) {
        if (control->field_0x7c == field_0x7c) {
            count++;
            if (index) {
                if (count == index)
                    control->UnknownFunction478cf0(1);
                else
                    control->UnknownFunction478cf0(0);
            } else if (control != this) {
                control->UnknownVirtualSlot29(0);
                control->UnknownFunction478cf0(0);
            }
        }
    }
    return 1;
}

// 0x004793f0
int UnknownGameUiControl::UnknownFunction4793f0() {
    int index = 0;
    GameObjectIterator iterator(field_0xb8->field_0x7f3c, 1, "UIRadioButton");
    UnknownGameUiControl* control;
    while ((control = (UnknownGameUiControl*)iterator.Next()) != 0) {
        if (control->field_0x7c == field_0x7c) {
            index++;
            if (control->UnknownFunction4755c0() == 1)
                return index;
        }
    }
    return 0;
}

// 0x004794c0
int UIRadioButton::UnknownVirtualSlot55(int a, int b) {
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this && !a) {
        UnknownDialogEvent event;
        event.field_0x20 = a;
        if (!UnknownFunction4755c0()) {
            UnknownFunction478cf0(1);
            GameObjectIterator* iterator = (GameObjectIterator*)UnknownVirtualSlot53();
            UnknownGameUiControl* control;
            while ((control = UnknownFunction472790(iterator)) != 0) {
                if (control != this && control->UnknownFunction4755c0() == 1) {
                    control->UnknownVirtualSlot29(0);
                    control->UnknownFunction478cf0(0);
                }
            }
            UnknownFunction472730(iterator);
        }
        event.field_0x08 = 1;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x14 = this;
        event.field_0x10 = field_0xb8->field_0x30;
        field_0xb8->UnknownVirtualSlot29(&event);
        if (event.field_0x20)
            return 1;
    }
    return UnknownGameUiControl::UnknownVirtualSlot55(a, b);
}

// 0x00477490
int UnknownGameUiControl::UnknownFunction477490(int row) {
    if (row < field_0x1ec && row >= 0) {
        operator delete(field_0x214[row].field_0x14, __FILE__, 0x1e80);
        operator delete(field_0x214[row].field_0x24, __FILE__, 0x1e81);
        operator delete(field_0x214[row].field_0x18, __FILE__, 0x1e82);
        void* brush = field_0x214[row].field_0x34;
        if (brush) {
            for (int i = 0; i < field_0x1ec; i++) {
                if (field_0x214[i].field_0x34 == brush)
                    goto shared;
            }
            DeleteObject((HGDIOBJ)brush);
        }
    shared:
        if (row < field_0x1ec - 1) {
            for (int i = row; i < field_0x1ec; i++)
                field_0x214[i] = field_0x214[i + 1];
        }
        memset(&field_0x214[field_0x1ec - 1], 0, sizeof(UnknownGameUiListRow));
        int count = field_0x1ec - 1;
        field_0x1ec = count <= 0 ? 0 : count;
        if (field_0x1f4 == row && field_0x1f4 == field_0x1ec) {
            UnknownVirtualSlot65(field_0x1f4 - 1 <= 0 ? 0 : field_0x1f4 - 1);
        }
        UnknownFunction477bc0();
        return 1;
    }
    return 0;
}

// 0x004775f0
void UnknownGameUiControl::UnknownFunction4775f0() {
    for (int i = 0; i < field_0x1ec; i++) {
        operator delete(field_0x214[i].field_0x14, __FILE__, 0x1ecf);
        operator delete(field_0x214[i].field_0x18, __FILE__, 0x1ed0);
        operator delete(field_0x214[i].field_0x24, __FILE__, 0x1ed1);
        if (field_0x214[i].field_0x34) {
            DeleteObject((HGDIOBJ)field_0x214[i].field_0x34);
            for (int j = 0; j < field_0x1ec; j++) {
                if (field_0x214[j].field_0x34 == field_0x214[i].field_0x34)
                    field_0x214[j].field_0x34 = 0;
            }
        }
        UIAnim* image = field_0x214[i].field_0x20;
        if (image && !image->field_0xf4)
            image->Release();
    }
    field_0x1ec = 0;
    field_0x1f0 = 0;
    UnknownFunction476a60(0);
    field_0x204 = 0;
    memset(field_0x214, 0, field_0x1fc_value * sizeof(UnknownGameUiListRow));
    UnknownFunction477bc0();
}

// 0x00477900
void UnknownGameUiControl::UnknownFunction477900(int a) {
    if (a || !field_0xb8->field_0x9bc || !g_UnknownGlobal65b608) {
        field_0xb8->field_0x9bc = this;
        g_UnknownGlobal65b608 = field_0xb8;
    }
    if (!field_0x1ec)
        return;
    field_0x220 = 1;
    for (int i = 0; i < field_0x1ec; i++)
        field_0x214[i].field_0x10 = i;
    int selected = field_0x1f4;
    qsort(field_0x214, field_0x1ec, sizeof(UnknownGameUiListRow), UnknownFunction477800);
    UnknownVirtualSlot50();
    if (field_0x7c > 0) {
        GameObjectIterator iterator(field_0xb8->field_0x7f3c, 1, "UIControl");
        UnknownGameUiControl* control;
        while ((control = (UnknownGameUiControl*)iterator.Next()) != 0) {
            if (control->field_0x7c == field_0x7c && control->field_0x5c == 3 && control != this &&
                control->field_0x1ec == field_0x1ec) {
                UnknownGameUiListRow* rows = (UnknownGameUiListRow*)DebugMalloc(
                    control->field_0x1fc_value * sizeof(UnknownGameUiListRow), __FILE__, 0x1f87);
                for (int j = 0; j < field_0x1ec; j++)
                    rows[j] = control->field_0x214[field_0x214[j].field_0x10];
                operator delete(control->field_0x214, __FILE__, 0x1f8e);
                control->field_0x214 = rows;
                control->UnknownVirtualSlot50();
            }
        }
    }
    for (int k = 0; k < field_0x1ec; k++) {
        if (field_0x214[k].field_0x10 == selected) {
            UnknownFunction476a60(k);
            break;
        }
    }
    field_0x220 = 0;
}

// ---------------------------------------------------------------------------
// UIScrollBar: position

// 0x004751e0
int UnknownGameUiControl::UnknownFunction4751e0(UnknownGameUiControl* list) {
    return list->field_0x1ec - list->field_0x204;
}

// 0x00475200
int UnknownGameUiControl::UnknownFunction475200(UnknownGameUiControl* list) {
    UnknownVirtualSlot50();
    if (field_0x21c) {
        int count = list->field_0x1ec;
        field_0x1fc_value = count - list->UnknownFunction4755c0();
    } else {
        field_0x1fc_value = list->UnknownFunction4755c0();
    }
    if (UnknownFunction4751e0(list) == field_0x1fc_value)
        return 1;
    unsigned int travel;
    if (field_0x5c == 8)
        travel = UnknownVirtualSlot61() - field_0x1f4;
    else
        travel = UnknownVirtualSlot62() - field_0x1f8;
    unsigned int range = list->field_0x1ec - list->field_0x204;
    if (range) {
        float position = field_0x1fc_value * (float)travel / range;
        field_0x1ec_float = position < travel ? position : travel;
        return 1;
    }
    return 0;
}

// 0x00475300
int UnknownGameUiControl::UnknownFunction475300(int value) {
    int width = UnknownVirtualSlot61();
    int height = UnknownVirtualSlot62();
    unsigned int position;
    int travel;
    if (field_0x5c == 8) {
        position = (int)(field_0x21c ? width - field_0x1ec_float : field_0x1ec_float);
    } else {
        position = (int)(field_0x21c ? height - field_0x1ec_float : field_0x1ec_float);
    }
    travel = field_0x5c == 8 ? width - field_0x1f4 : height - field_0x1f8;
    return (int)(travel ? (double)(position * value) / travel : 0.0);
}

// 0x00475500
int UnknownGameUiControl::UnknownFunction475500() {
    int width = UnknownVirtualSlot61();
    int height = UnknownVirtualSlot62();
    int position;
    if (!field_0x21c) {
        position = (int)field_0x1ec_float;
        if (position < field_0x1ec_float)
            position++;
    } else {
        if (field_0x5c == 8)
            position = (int)(width - field_0x1ec_float);
        else
            position = (int)(height - field_0x1ec_float);
    }
    if (field_0x200 && width) {
        if (field_0x5c == 8)
            return (unsigned int)(field_0x200 * position) / (unsigned int)(width - field_0x1f4);
        return (unsigned int)(field_0x200 * position) / (unsigned int)(height - field_0x1f8);
    }
    return position;
}

// ---------------------------------------------------------------------------
// UIEditBox: text

// 0x00473c70
void UnknownGameUiControl::UnknownFunction473c70(int size) {
    char saved[1000];
    int kept = 0;
    if (field_0xc0) {
        strcpy(saved, field_0xc0);
        operator delete(field_0xc0, __FILE__, 0x169a);
        kept = 1;
    }
    int capacity = size + 1;
    if (capacity >= 1000)
        capacity = 1000;
    else if (capacity < 1)
        capacity = 1;
    field_0x1f0 = capacity;
    field_0x200 = 0;
    field_0x1ec = 0;
    field_0x1fc_value = 0;
    field_0xd0 = 0;
    field_0xc0 = (char*)DebugMalloc(capacity, __FILE__, 0x16a3);
    if (kept) {
        int max = field_0x1f0 - 1;
        int n = strlen(saved);
        int length;
        if (n < max)
            length = n;
        else
            length = max;
        strncpy(field_0xc0, saved, length);
        field_0xc0[length] = 0;
    }
    UnknownVirtualSlot50();
}

// 0x00473da0
void UnknownGameUiControl::UnknownFunction473da0(char* text) {
    if (!text)
        text = "";
    if (!field_0xc0)
        field_0xc0 = (char*)DebugMalloc(field_0x1f0, __FILE__, 0x16c0);
    int max = field_0x1f0 - 1;
    int n = strlen(text);
    int length;
    if (n < max)
        length = n;
    else
        length = max;
    strncpy(field_0xc0, text, length);
    field_0xc0[length] = 0;
    field_0x1ec = length;
    field_0xd0 = length;
    HDC dc;
    if (!g_UnknownGlobal56e26c->PCTarget()->field_0x48->UnknownMethod17((void**)&dc)) {
        HGDIOBJ font = (HGDIOBJ)(field_0x128 ? field_0x128 : (field_0xb8 ? (int)field_0xb8->field_0xd8 : 0));
        HGDIOBJ old = SelectObject(dc, font);
        SIZE size;
        GetTextExtentPoint32A(dc, field_0xc0, field_0xd0, &size);
        SelectObject(dc, old);
        g_UnknownGlobal56e26c->PCTarget()->field_0x48->UnknownMethod26(dc);
        field_0xdc[1] = size.cx;
        field_0x1fc_value = size.cx;
    } else {
        field_0xdc[1] = 0;
        field_0x1fc_value = 0;
    }
    UnknownVirtualSlot50();
}

// 0x00473f30
void UnknownGameUiControl::UnknownFunction473f30(const char* characters) {
    if (characters) {
        int length = strlen(characters);
        field_0x210 = DebugMalloc(length + 1, __FILE__, 0x16f7);
        if (field_0x210)
            strcpy((char*)field_0x210, characters);
    } else {
        if (field_0x210)
            operator delete(field_0x210, __FILE__, 0x16fb);
        field_0x210 = 0;
    }
}

// 0x00473fc0
char UnknownGameUiControl::UnknownFunction473fc0(char c) {
    if (!field_0x210)
        return c;
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        char other = c <= 'Z' ? c + 0x20 : c - 0x20;
        if (strchr((char*)field_0x210, c))
            return c;
        if (strchr((char*)field_0x210, other))
            return other;
        return 0;
    }
    if (strchr((char*)field_0x210, c))
        return c;
    return 0;
}

// ---------------------------------------------------------------------------
// UIDialog: resources and text

// 0x0046a8e0
int UIDialog::UnknownFunction46a8e0(const char* name) {
    int result = 0;
    UnknownResourceEntry* entry = g_UnknownResourceManager572b44->UnknownFunction4e9360(name, 1);
    if (entry)
        result = UnknownFunction46a920(entry->field_0x14, entry->field_0x18);
    return result;
}

// 0x0046eeb0
void UIDialog::UnknownFunction46eeb0(void* dc) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    if (dialog->field_0x7f04)
        SelectObject((HDC)dc, (HGDIOBJ)dialog->field_0x7f04);
    if (!dialog->field_0x110 && dc)
        ((PCRenderTarget*)dialog->UnknownInlineField18())->field_0x48->UnknownMethod26(dc);
    if (dialog->field_0x7f00)
        DeleteObject((HGDIOBJ)dialog->field_0x7f00);
}

// 0x0046f3c0
void UIDialog::UnknownFunction46f3c0(void* dc, int x, int y, const char* text, int length,
                                     unsigned int color, int shadow, unsigned int shadowColor) {
    if (shadow) {
        SetTextColor((HDC)dc, shadowColor);
        TextOutA((HDC)dc, x - 1, y - 1, text, length);
    }
    SetTextColor((HDC)dc, color);
    TextOutA((HDC)dc, x, y, text, length);
}

// 0x0046fc80
int UIDialog::UnknownFunction46fc80(const char* text, const char* position, int length) {
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
    if (dialog->field_0x34->field_0x2c && dialog->field_0x7f3c->field_0x10) {
        GUIUser* user = dialog->field_0x34;
        UnknownGuiControl* focus = user->field_0x1d8;
        if (focus && ((UnknownGameUiControl*)focus)->field_0xb8 != (UnknownGameUiDialog*)this)
            focus = 0;
        POINT position = user->field_0x2c->field_0xa4;
        if (dialog->field_0x7f3c->field_0x10) {
            user->field_0x1dc = 0;
            int result = ((UnknownGameUiControl*)dialog->field_0x7f3c->field_0x10)->UnknownFunction472480((int*)&position);
            if (focus && !dialog->field_0x34->field_0x1cc && !result)
                dialog->field_0x34->UnknownFunction487730(0, 0, 1);
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
void UIDialog::UnknownFunction46f420(void* dc, int x, int y, const char* text, int length,
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
void UIDialog::UnknownFunction46f550(void* dc, CameraRect* rect, const char* text, int length,
                                     unsigned int color, int height, UnknownGameUiTextRun* runs, int shadow,
                                     unsigned int shadowColor, int flags, int transparent,
                                     UnknownGameUiControl* control) {
    if (transparent)
        SetBkMode((HDC)dc, TRANSPARENT);
    else
        SetBkMode((HDC)dc, OPAQUE);
    if (flags != 9) {
        if (control) {
            g_UnknownGlobal65b5e0.cx = ((int*)control->UnknownVirtualSlot37())[1];
            g_UnknownGlobal65b5e0.cy = control->field_0x160;
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
        height = ((UnknownGameUiDialog*)this)->field_0xdc;
    if (flags & 0x10)
        g_UnknownGlobal65b584 = (rect->bottom - rect->top) / 2 - height / 2 + rect->top;
    else if (flags & 0x20)
        g_UnknownGlobal65b584 = rect->bottom - height;
    else
        g_UnknownGlobal65b584 = rect->top;
    if (runs)
        UnknownFunction46f420(dc, g_UnknownGlobal65b5a8, g_UnknownGlobal65b584, text, length, runs, color, shadow,
                              shadowColor);
    else
        UnknownFunction46f3c0(dc, g_UnknownGlobal65b5a8, g_UnknownGlobal65b584, text, length, color, shadow,
                              shadowColor);
}

SIZE g_UnknownGlobal65b5a0;                   // the control text's size
int g_UnknownGlobal65b5e8;                    // its position
int g_UnknownGlobal65b5c0;

// 0x0046f6c0
void UIDialog::UnknownFunction46f6c0(void* dc, UnknownGameUiControl* control, int transparent) {
    CameraRect rect = *(CameraRect*)control->field_0x2c;
    rect.left += control->field_0xe8;
    rect.top += control->field_0xec;
    rect.right -= control->field_0xe8;
    rect.bottom -= control->field_0xec;
    if (transparent)
        SetBkMode((HDC)dc, TRANSPARENT);
    else
        SetBkMode((HDC)dc, OPAQUE);
    if (control->field_0xd8 != 9) {
        g_UnknownGlobal65b5a0.cx = ((int*)control->UnknownVirtualSlot37())[1];
        g_UnknownGlobal65b5a0.cy = control->field_0x160;
    }
    if (control->field_0xd8 & 2)
        g_UnknownGlobal65b5e8 = (rect.right - rect.left) / 2 - g_UnknownGlobal65b5a0.cx / 2 + rect.left;
    else if (control->field_0xd8 & 4)
        g_UnknownGlobal65b5e8 = rect.right - g_UnknownGlobal65b5a0.cx;
    else
        g_UnknownGlobal65b5e8 = rect.left;
    if (control->field_0xd8 & 0x10)
        g_UnknownGlobal65b5c0 =
            (rect.bottom - rect.top) / 2 - ((UnknownGameUiDialog*)this)->field_0xdc / 2 + rect.top;
    else if (control->field_0xd8 & 0x20)
        g_UnknownGlobal65b5c0 = rect.bottom - ((UnknownGameUiDialog*)this)->field_0xdc;
    else
        g_UnknownGlobal65b5c0 = rect.top;
    if (control->UnknownVirtualSlot36())
        UnknownFunction46f420(dc, g_UnknownGlobal65b5e8, g_UnknownGlobal65b5c0,
                              (const char*)control->UnknownVirtualSlot34(), control->UnknownVirtualSlot35(),
                              (UnknownGameUiTextRun*)control->UnknownVirtualSlot36(), control->field_0xc4,
                              control->field_0xc4, control->field_0xc8);
    else
        UnknownFunction46f3c0(dc, g_UnknownGlobal65b5e8, g_UnknownGlobal65b5c0,
                              (const char*)control->UnknownVirtualSlot34(), control->UnknownVirtualSlot35(),
                              control->field_0xc4, control->field_0xd4, control->field_0xc8);
}

CameraRect* g_UnknownGlobal65b588;            // the clip rectangle
CameraRect g_UnknownGlobal65b5f0;
int g_UnknownGlobal65b5ec;

// 0x0046ed70
int UIDialog::UnknownFunction46ed70(void** dc, CameraRect* rect, int* a, UnknownGameUiControl* control) {
    UnknownGameUiDialog* dialog = (UnknownGameUiDialog*)this;
    g_UnknownGlobal65b588 = rect;
    *a = 0;
    dialog->field_0x7f04 = 0;
    if (dialog->field_0x110) {
        g_UnknownGlobal65b588 = &g_UnknownGlobal65b5f0;
        g_UnknownGlobal65b5f0 = *rect;
        if (control) {
            g_UnknownGlobal65b5ec = dialog->field_0x110->UnknownFunction4049d0(
                dc, rect, control->field_0x1b8, control->field_0x1c0, &control->field_0x1bc, a,
                &g_UnknownGlobal65b5f0);
        } else {
            int frames = 1;
            g_UnknownGlobal65b5ec =
                dialog->field_0x110->UnknownFunction4049d0(dc, rect, -1, 0, &frames, a, &g_UnknownGlobal65b5f0);
        }
    } else {
        ((PCRenderTarget*)dialog->UnknownInlineField18())->field_0x48->UnknownMethod17(dc);
        g_UnknownGlobal65b5ec = 1;
    }
    if (*dc && control) {
        dialog->field_0x7f04 =
            SelectObject((HDC)*dc, (HGDIOBJ)(control->field_0x128 ? control->field_0x128 : (int)dialog->field_0xd8));
        dialog->field_0x7f00 = CreateRectRgn(g_UnknownGlobal65b588->left, g_UnknownGlobal65b588->top,
                                             g_UnknownGlobal65b588->right, g_UnknownGlobal65b588->bottom);
        SelectClipRgn((HDC)*dc, (HRGN)dialog->field_0x7f00);
    } else {
        dialog->field_0x7f00 = 0;
    }
    return g_UnknownGlobal65b5ec;
}

// 0x0046fce0
void UIDialog::UnknownFunction46fce0(int a, int time, int b) {
    if (time)
        ((UnknownGameUiDialog*)this)->field_0x7f24.Add(
            new(__FILE__, 0xa76) UITimer(a, time, (UnknownGameUiControl*)b));
}

// ---------------------------------------------------------------------------
// UIControl: transitions

// 0x00471df0
int UnknownGameUiControl::UnknownVirtualSlot46() {
    if (field_0x164 == 0) {
        field_0x184->field_0x24 = 0;
        field_0x184->UnknownFunction472f50();
        field_0x164 = 1;
        if (field_0x1ac) {
            field_0x1ac->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
            field_0x1ac->UnknownFunction4bc6b0(1, 0, 0);
        }
    }
    if (field_0x164 == 1 && field_0x184->field_0x08 <= field_0x184->field_0x10 - 1) {
        field_0x1cc = field_0x184->UnknownFunction472f90();
        UnknownVirtualSlot50();
        if (field_0x184->field_0x08 == field_0x184->field_0x10 - 1)
            field_0x164 = 2;
        return 0;
    }
    field_0x164 = 0;
    return 1;
}

// 0x00471eb0
int UnknownGameUiControl::UnknownVirtualSlot47() {
    if (field_0x164 == 0) {
        if (field_0x188 == field_0x184) {
            field_0x188->field_0x24 = 1;
            field_0x164 = 2;
        } else {
            field_0x164 = 1;
        }
        field_0x188->UnknownFunction472f50();
        if (field_0x1b0) {
            field_0x1b0->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
            field_0x1b0->UnknownFunction4bc6b0(1, 0, 0);
        }
    }
    if ((field_0x164 == 1 && field_0x188->field_0x08 < field_0x188->field_0x10 - 1) ||
        (field_0x164 == 2 && field_0x188->field_0x08 >= 0)) {
        field_0x1cc = field_0x188->UnknownFunction472f90();
        UnknownVirtualSlot50();
        if (field_0x188->field_0x08 == 0)
            field_0x164 = 3;
        return 0;
    }
    field_0x164 = 0;
    field_0x70 = 0;
    return 1;
}

int g_UnknownGlobal65b610;                       // frames of the slide

// 0x00471850
int UnknownGameUiControl::UnknownVirtualSlot42() {
    if (field_0x164 == 0 && field_0xb0) {
        g_UnknownGlobal65b610 = field_0xb0 < 0 ? 25 : field_0xb0;
        int left = field_0x3c[0];
        field_0x80 = (float)(left - field_0x3c[2] - 1);
        field_0x90 = ((float)left - field_0x80) / g_UnknownGlobal65b610;
        if (field_0x1a8) {
            field_0x1a8->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
            field_0x1a8->UnknownFunction4bc6b0(1, 0, 0);
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
        switch (field_0x5c) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
            field_0x1cc = field_0x16c[field_0x60]->UnknownFunction472f90();
            break;
        default:
            field_0x1cc = 0;
            return 1;
        }
    }
    field_0x164++;
    return 0;
}

int g_UnknownGlobal65b614;                       // frames of the slide

// 0x004719c0
int UnknownGameUiControl::UnknownVirtualSlot44() {
    if (field_0x164 == 0 && field_0xb0) {
        g_UnknownGlobal65b614 = field_0xb0 < 0 ? 25 : field_0xb0;
        int top = field_0x3c[1];
        field_0x84 = (float)(top - field_0x3c[3]);
        field_0x94 = ((float)top - field_0x84) / g_UnknownGlobal65b614;
        if (field_0x1a8) {
            field_0x1a8->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
            field_0x1a8->UnknownFunction4bc6b0(1, 0, 0);
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
        switch (field_0x5c) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
            field_0x1cc = field_0x16c[field_0x60]->UnknownFunction472f90();
            break;
        default:
            field_0x1cc = 0;
            return 1;
        }
    }
    field_0x164++;
    return 0;
}

int g_UnknownGlobal65b618;                       // frames of the slide

// 0x00471b30
int UnknownGameUiControl::UnknownVirtualSlot43() {
    if (field_0x164 == 0 && field_0xb0) {
        g_UnknownGlobal65b618 = field_0xb0 < 0 ? 25 : field_0xb0;
        field_0x80 = (float)field_0xb8->field_0x174;
        field_0x90 = ((float)field_0x3c[0] - field_0x80) / g_UnknownGlobal65b618;
        if (field_0x1a8) {
            field_0x1a8->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
            field_0x1a8->UnknownFunction4bc6b0(1, 0, 0);
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
    if (field_0x2c[0] < field_0xb8->field_0x174) {
        switch (field_0x5c) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
            field_0x1cc = field_0x16c[field_0x60]->UnknownFunction472f90();
            break;
        default:
            field_0x1cc = 0;
            return 1;
        }
    }
    field_0x164++;
    return 0;
}

int g_UnknownGlobal65b61c;                       // frames of the slide

// 0x00471c90
int UnknownGameUiControl::UnknownVirtualSlot45() {
    if (field_0x164 == 0 && field_0xb0) {
        g_UnknownGlobal65b61c = field_0xb0 < 0 ? 25 : field_0xb0;
        field_0x84 = (float)field_0xb8->field_0x178;
        field_0x94 = ((float)field_0x3c[1] - field_0x84) / g_UnknownGlobal65b61c;
        if (field_0x1a8) {
            field_0x1a8->UnknownFunction4bcbe0(field_0xbc->field_0x34c, 0);
            field_0x1a8->UnknownFunction4bc6b0(1, 0, 0);
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
    if (field_0x2c[1] < field_0xb8->field_0x178) {
        switch (field_0x5c) {
        case 1:
        case 2:
        case 4:
        case 5:
        case 9:
        case 10:
            field_0x1cc = field_0x16c[field_0x60]->UnknownFunction472f90();
            break;
        default:
            field_0x1cc = 0;
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
    event.field_0x20 = 0;
    if (field_0x208) {
        event.field_0x08 = 0x10;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x10 = field_0xbc;
        event.field_0x14 = this;
        field_0xb8->UnknownVirtualSlot29(&event);
    }
    field_0x208 = 0;
    if (field_0xb8->field_0x34->field_0x1d8 == (UnknownGuiControl*)this && !a && field_0x60 == 2) {
        event.field_0x08 = 1;
        event.field_0x00 = field_0x74;
        event.field_0x04 = UnknownFunction470df0();
        event.field_0x0c = field_0xb8;
        event.field_0x10 = field_0xbc;
        event.field_0x14 = this;
        field_0xb8->UnknownVirtualSlot29(&event);
        if (event.field_0x20)
            return 1;
    }
    return UnknownGameUiControl::UnknownVirtualSlot56(a, position);
}

// ---------------------------------------------------------------------------
// UIListBox: rows

unsigned int g_UnknownGlobal65b59c;           // time of the last list box click

// 0x00477d30
void UnknownGameUiControl::UnknownFunction477d30(int* handled) {
    UnknownDialogEvent event;
    event.field_0x20 = 0;
    event.field_0x08 = 2;
    event.field_0x00 = field_0x74;
    event.field_0x04 = field_0xf4;
    event.field_0x0c = field_0xb8;
    event.field_0x14 = this;
    event.field_0x10 = field_0xb8->field_0x30;
    field_0xb8->UnknownVirtualSlot29(&event);
    if (handled)
        *handled = 0;
    if (!event.field_0x20) {
        static unsigned int s_doubleClickTime = GetDoubleClickTime();
        g_UnknownGlobal65b59c = UnknownFunction4bfa80();
        if (field_0x1f4 == field_0x238 && g_UnknownGlobal65b59c - field_0x23c <= s_doubleClickTime) {
            event.field_0x08 = 0xd;
            event.field_0x00 = field_0x74;
            event.field_0x04 = field_0xf4;
            event.field_0x0c = field_0xb8;
            event.field_0x14 = this;
            event.field_0x10 = field_0xb8->field_0x30;
            field_0xb8->UnknownVirtualSlot29(&event);
            if (event.field_0x20) {
                if (handled)
                    *handled = 1;
                return;
            }
            field_0x23c = 0;
            return;
        } else {
            field_0x23c = g_UnknownGlobal65b59c;
            field_0x238 = field_0x1f4;
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
    field_0x1fc->field_0x2c[3] = field_0x1fc->field_0x2c[1] + height;
    *(CameraRect*)field_0x1fc->field_0x3c = *(CameraRect*)field_0x1fc->field_0x2c;
    field_0x1f0_control->field_0x2c[3] = field_0x1f0_control->field_0x2c[1] + height;
    *(CameraRect*)field_0x1f0_control->field_0x3c = *(CameraRect*)field_0x1f0_control->field_0x2c;
    field_0x200_control->field_0x2c[3] = field_0x200_control->field_0x2c[1] + height;
    field_0x200_control->field_0x2c[1] += 2;
    field_0x200_control->field_0x2c[3] -= 2;
    *(CameraRect*)field_0x200_control->field_0x3c = *(CameraRect*)field_0x200_control->field_0x2c;
    field_0x1f4_control->field_0x2c[3] = field_0x1f4_control->field_0x2c[1] + height;
    *(CameraRect*)field_0x1f4_control->field_0x3c = *(CameraRect*)field_0x1f4_control->field_0x2c;
    field_0x204 = height;
}

// 0x0047a970
void UIDropDownList::UnknownFunction47a970(UIAnim* image) {
    field_0x1f0_control->UnknownFunction470730(0, image);
    field_0x1f0_control->field_0x2c[2] = image->UnknownFunction472f80()->field_0x0c + field_0x1f0_control->field_0x2c[0];
    field_0x1f0_control->field_0x2c[3] = image->UnknownFunction472f80()->field_0x10 + field_0x1f0_control->field_0x2c[1];
    *(CameraRect*)field_0x1f0_control->field_0x3c = *(CameraRect*)field_0x1f0_control->field_0x2c;
    *(CameraRect*)field_0x1fc->field_0x3c = *(CameraRect*)field_0x1f0_control->field_0x2c;
    *(CameraRect*)field_0x1fc->field_0x2c = *(CameraRect*)field_0x1fc->field_0x3c;
    UnknownFunction47a880(image->UnknownFunction472f80()->field_0x10);
}

// 0x0047aa40
void UIDropDownList::UnknownFunction47aa40(UIAnim* image) {
    field_0x1f4_control->UnknownFunction470730(0, image);
    field_0x1f4_control->field_0x2c[2] = image->UnknownFunction472f80()->field_0x0c + field_0x1f4_control->field_0x2c[0];
    field_0x1f4_control->field_0x2c[3] = image->UnknownFunction472f80()->field_0x10 + field_0x1f4_control->field_0x2c[1];
    *(CameraRect*)field_0x1f4_control->field_0x3c = *(CameraRect*)field_0x1f4_control->field_0x2c;
    *(CameraRect*)field_0x200_control->field_0x2c = *(CameraRect*)field_0x1f4_control->field_0x2c;
    field_0x200_control->field_0x2c[1] += 2;
    field_0x200_control->field_0x2c[3] -= 2;
    *(CameraRect*)field_0x200_control->field_0x3c = *(CameraRect*)field_0x200_control->field_0x2c;
}

// 0x0047a2d0
void UIDropDownList::UnknownFunction47a2d0(int open) {
    field_0x1f0_control->UnknownFunction470660(open, 1);
    field_0x1f4_control->UnknownFunction470660(open, 1);
    field_0x1fc->UnknownFunction470660(open, 1);
    field_0x200_control->UnknownFunction470660(open, 1);
    if (!field_0x208 && open) {
        field_0xb8->field_0x7f3c->UnknownFunction469190(field_0x1f0_control, -1);
        field_0xb8->field_0x7f3c->UnknownFunction469190(field_0x1f4_control, -1);
        field_0xb8->field_0x7f3c->UnknownFunction469190(field_0x1fc, -1);
        field_0xb8->field_0x7f3c->UnknownFunction469190(field_0x200_control, -1);
        field_0x1fc->UnknownFunction477bc0();
    } else if (field_0x208 && !open) {
        field_0x1f0_control->UnknownFunction4691f0();
        field_0x1f4_control->UnknownFunction4691f0();
        field_0x1fc->UnknownFunction4691f0();
        field_0x200_control->UnknownFunction4691f0();
        if (field_0xb8->field_0x110)
            field_0xb8->field_0x110->UnknownFunction404da0();
    }
    field_0x208 = open;
}

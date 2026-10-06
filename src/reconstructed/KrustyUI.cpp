#include "KrustyUI.h"

#include <stdlib.h>
#include <string.h>

#include "DebugAlloc.h"
#include "DirectPlayMessages.h"
#include "GameUi.h"
#include "UIDialog.h"
#include "RenderTarget.h"
#include "TrackGame.h"

// cdecl 0x0047b570 (gameui.cpp): resizes a DebugMalloc'd block.
void* UnknownFunction47b570(void* block, unsigned int size);

// Global at 0x0068a498: 36 short strings loaded from string resources
// 5000-5035 (KrustyUI 0x004988a0).
char g_UnknownStrings68a498[36][16];

// cdecl 0x005053b0, called first on shutdown with 0.
void UnknownFunction5053b0(int value);

// cdecl 0x0049bda0: the GUI's progress callback.
void UnknownFunction49bda0() {
    g_UnknownGlobal56e26c->mode.UnknownFunction523580();
}

// 0x004987f0
KrustyUI::KrustyUI(int flags) : GameObject(flags) {
    field_0x2c = 0;
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x3c = 0;
    field_0x48c = 0;
    field_0x490 = 0;
    field_0x464 = 0;
    field_0x40 = 0;
    field_0x44 = 0;
    field_0x494 = 0;
    field_0x498 = 0;
    field_0x4a8 = 0;
    field_0x4ac = 0;
    field_0x4b0 = 0;
    field_0x4a4 = 0;
    field_0x4a0 = 0;
    field_0x49c = 0;
    field_0x60 = 0;
    field_0x64 = 0;
}

// 0x0049b470
KrustyUI::~KrustyUI() {
    UnknownFunction4999b0();
    if (field_0x48)
        operator delete(field_0x48, __FILE__, 1306);
    if (field_0x50)
        operator delete(field_0x50, __FILE__, 1307);
    if (field_0x58)
        operator delete(field_0x58, __FILE__, 1308);
    if (field_0x60)
        operator delete(field_0x60, __FILE__, 1309);
}

// 0x004999b0
void KrustyUI::UnknownFunction4999b0() {
    UnknownFunction5053b0(0);
    if (!g_UnknownGlobal56e26c->field_0x2d5_bit1)
        field_0x2c->UnknownFunction485d50();
    if (field_0x464) {
        field_0x464->Release();
        field_0x464 = 0;
    }
}

// 0x0049b530: tells the "ProgressBar" control 1 (0x0047b370).
void KrustyUI::UnknownFunction49b530() {
    if (field_0x490) {
        UnknownGameUiControl* bar = field_0x490->UnknownFunction46ebf0("ProgressBar", 0);
        if (bar)
            bar->UnknownFunction47b370(1);
    }
}

// 0x0049bb80
void KrustyUI::UnknownFunction49bb80() {
    if (field_0x60)
        operator delete(field_0x60, __FILE__, 1453);
    field_0x60 = 0;
    field_0x64 = 0;
}

// 0x00499ad0
void KrustyUI::UnknownVirtualSlot4() {
    GameObject::UnknownVirtualSlot4();
}

// 0x00499ac0
void KrustyUI::UnknownVirtualSlot5() {
    GameObject::UnknownVirtualSlot5();
}

// 0x00499ae0
int KrustyUI::UnknownVirtualSlot10(float frameTime) {
    return GameObject::UnknownVirtualSlot10(frameTime);
}

// 0x00499af0
int KrustyUI::UnknownVirtualSlot18() {
    GameObject::UnknownVirtualSlot18();
    return 1;
}

// 0x00499a40
int KrustyUI::UnknownVirtualSlot23(UnknownControlEvent* event, UnknownInputEntry* entry) {
    field_0x4a4 = 0;
    return GameObject::UnknownVirtualSlot23(event, entry) != 0;
}

// 0x00499a70: DPSYS_HOST (this player became the session host) sets the
// network object's +0x10.
int KrustyUI::UnknownVirtualSlot24(int type, void* data, int from, int to, int flags) {
    if (GameObject::UnknownVirtualSlot24(type, data, from, to, flags))
        return 1;
    if (type == DPSYS_HOST)
        g_UnknownGlobal56e26c->field_0x08->isHost = 1;
    return 0;
}

// 0x00499980: also hands the value to the +0x464 scene.
int KrustyUI::UnknownVirtualSlot25(void* value) {
    GameObject::UnknownVirtualSlot25(value);
    if (field_0x464)
        field_0x464->UnknownVirtualSlot25(value);
    return 1;
}

// 0x004999f0
void KrustyUI::UnknownFunction4999f0(GameObject* parent) {
    if (field_0x464) {
        parent->UnknownFunction469190(field_0x464, -1);
        ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(field_0x468);
    }
}

// 0x00499a20
void KrustyUI::UnknownFunction499a20() {
    if (field_0x464) {
        field_0x464->UnknownFunction4691f0();
        ((RenderTarget*)field_0x18)->UnknownFunction4e8cf0(0);
    }
}

// 0x00499b00
void KrustyUI::UnknownFunction499b00() {
    if (field_0x2c)
        field_0x2c->UnknownFunction486630(1);
}

// 0x00499b10
void KrustyUI::UnknownFunction499b10() {
    if (field_0x2c)
        field_0x2c->UnknownFunction486630(0);
}

// 0x0049a4a0: turns "MediaControl" on, hides the GUI and opens Exit1Dlg.
void KrustyUI::UnknownFunction49a4a0() {
    UnknownFunction468dd0("MediaControl");
    field_0x2c->UnknownFunction486630(0);
    field_0x2c->UnknownFunction485a70(new(__FILE__, 836) Exit1Dlg, 0, 2, 0, 0, 0, 0, 1);
}

// 0x0049ba70
int KrustyUI::UnknownFunction49ba70(int a, int b, int c, const char* name, int d, int e) {
    field_0x60 = (UnknownKrustyUIEntry*)UnknownFunction47b570(field_0x60, (field_0x64 + 1) * sizeof(UnknownKrustyUIEntry));
    field_0x60[field_0x64].field_0x00 = a;
    field_0x60[field_0x64].field_0x04 = b;
    field_0x60[field_0x64].field_0x08 = c;
    int length = strlen(name);
    int count = length > 63 ? 63 : length;
    strncpy(field_0x60[field_0x64].field_0x14, name, count);
    field_0x60[field_0x64].field_0x14[count] = 0;
    field_0x60[field_0x64].field_0x0c = d;
    field_0x60[field_0x64].field_0x10 = e;
    return ++field_0x64 - 1;
}

// A random value in [0, 1).
static inline float RandomUnit() {
    return rand() * (1.0f / 32768);
}

// 0x0049b020
void KrustyUI::UnknownFunction49b020(const char** names, int count) {
    int* used = (int*)DebugCalloc(36, sizeof(int), __FILE__, 1120);
    srand(UnknownFunction4bfa80());
    for (int i = count; i > 0; i--) {
        // Through a float local: VC6 otherwise folds the two scales into one.
        float unit = RandomUnit();
        int index = (int)(unit * 36);
        if (index >= 35)
            index = 35;
        int start = index;
        while (used[index]) {
            if (++index == 36)
                index = 0;
            if (index == start)
                break;
        }
        used[index] = 1;
        *names++ = g_UnknownStrings68a498[index];
    }
    operator delete(used, __FILE__, 1157);
}

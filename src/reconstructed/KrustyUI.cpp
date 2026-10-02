#include "KrustyUI.h"

#include "DebugAlloc.h"
#include "GameUi.h"
#include "TrackGame.h"

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

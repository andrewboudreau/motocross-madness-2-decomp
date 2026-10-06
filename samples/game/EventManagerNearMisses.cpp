// Near-miss EventManager candidates, kept out of src/reconstructed until
// they match. See docs/EVENTMANAGER.md.
//
// UnknownFunction45cb20 (0x0045cb20, 67 bytes): cdecl progress callback that
// 0x0045cb70 and 0x0045cdc0 pass by address. 63 of 67 bytes match: retail
// loads *step into edx and the bar value into ecx; VC6 here swaps them.
// Compound, spelled-out, commuted, local-variable, void* and struct
// parameter forms, and compiling the function alone, do not change it.
//
// EventManager::UnknownFunction45e710 (0x0045e710, 531 bytes): leaves the
// race for a menu. Control flow, calls, the TransDlg `new` (retail line 1064)
// and its EH state match; 454 of 533 bytes. After the `new` retail loads the
// global into eax (the short form, 2 bytes less) and the menu into edx where
// VC6 here picks ecx and eax; it also reuses ecx for TrackGame+0xc4c. Local,
// base-pointer, assignment-in-argument and declaration-placement forms do
// not change it.
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/GameUi.h"
#include "../../src/reconstructed/SoundInterface.h"
#include "../../src/reconstructed/TrackGame.h"
#include "../../src/reconstructed/UIDialog.h"

// IMM32, called through the linker's import thunk.
extern "C" void* __stdcall ImmAssociateContext(void* window, void* context);

// 0x0045cb20: adds *step to KrustyUI's "ProgressBar"; without a step it
// calls KrustyUI 0x0049b530.
void UnknownFunction45cb20(int* step) {
    KrustyUI* ui = g_UnknownGlobal56e26c->ui;
    if (step) {
        if (ui->field_0x490) {
            UnknownGameUiControl* bar = ui->field_0x490->UnknownFunction46ebf0("ProgressBar", 0);
            bar->field_0x1f0 += *step;
        }
    } else {
        ui->UnknownFunction49b530();
    }
}

// 0x0045e710: leaves the race for `menu`: restores the UI, the window's
// input context and 640x480x16, and shows the transition dialog.
void EventManager::UnknownFunction45e710(int menu) {
    if (!g_UnknownGlobal56e26c->ui)
        return;
    UnknownTrackGameObject56cItem* item = g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485df0();
    if (item)
        item->UnknownFunction46ff30(0);
    if (g_UnknownGlobal56e26c->ui->field_0x494)
        ImmAssociateContext(g_UnknownGlobal56e26c->field_0x31c, g_UnknownGlobal56e26c->ui->field_0x494);
    if (g_UnknownGlobal56e26c->ui->field_0x498)
        g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction4868b0(1);
    g_UnknownGlobal56e26c->uiInteractionBlocked = 0;
    g_UnknownGlobal56e26c->field_0x3438 = 0;
    g_UnknownGlobal56e26c->field_0x3434 = 0;
    if (!g_UnknownGlobal56e26c->mode.field_0xa20)
        g_UnknownGlobal56e26c->UnknownFunction521a40();
    UnknownDisplayMode* current = &g_UnknownGlobal56e26c->field_0x0c->field_0x10[g_UnknownGlobal56e26c->mode.field_0xa4c];
    if (current->width != 640 || current->height != 480 || current->bitDepth != 16)
        g_UnknownGlobal56e26c->UnknownVirtualSlot19(
            g_UnknownGlobal56e26c->field_0x0c->UnknownFunction52d250(640, 480, 16, 0, 0));
    g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485ef0();
    UnknownKrustyUIGuiLayer* layer = g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction486540(0);
    layer->field_0xc0->field_0x5c = g_UnknownGlobal56e26c->mode.field_0x6d4;
    if (!g_UnknownGlobal56e26c->field_0x3428 && !g_UnknownGlobal56e26c->ui->field_0x4a8 &&
        g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 0 && g_UnknownGlobal56e26c->mode.field_0x27f8.field_0x00 != 4)
        g_UnknownGlobal56e26c->ui->UnknownFunction49bbb0();
    g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction486630(1);
    TransDlg* dialog = new(__FILE__, 1064) TransDlg;
    g_UnknownGlobal56e26c->ui->field_0x2c->UnknownFunction485a70(dialog, 0, 2, 0, 0, 0, 0, 1);
    dialog->UnknownFunction455c40(menu);
    ((PCSoundInterface*)g_UnknownGlobal56e26c->field_0x04)->UnknownFunction4be9b0(0);
}

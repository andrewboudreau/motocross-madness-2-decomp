// Near-miss ControlInterface candidates, kept out of src/reconstructed until
// they match. See docs/INPUT_DEVICES.md.
//
// ControlInterface::UnknownFunction43cf00 (0x0043cf00, 363 bytes + jump
// table): everything matches except the modifier stamp. Retail loads the
// keyboard pointer, skips the load of +0x16d8 when it is null and stores
// the (zero) pointer itself; VC6 here emits `jmp` + `xor eax, eax` for the
// zero. Ternary, if/else, local-variable and inline-helper forms all do.
//
// PCControlInterface::UnknownVirtualSlot3 (0x004bf4f0, 88 bytes + jump
// table): the keyboard and mouse cases call the same slot, so their tails
// merge. Retail keeps the keyboard case whole and has the mouse case jump
// into it; VC6 here does the opposite. Case order, early-return and
// result-variable forms do not change it.
#include "../../src/reconstructed/ControlInterface.h"
#include "../../src/reconstructed/PCControl.h"
#include "../../src/reconstructed/JoystickDevice.h"
#include "../../src/reconstructed/KeyboardDevice.h"
#include "../../src/reconstructed/MouseDevice.h"
#include "../../src/reconstructed/UnknownObject56e26c.h"

// 0x0043cf00: has the keyboard, mouse and joysticks read their input, then
// passes each queued event, stamped with the keyboard's modifier state, to
// the global object (slot 13 for releases, 14 for presses) with the
// device's entry for that control.
int ControlInterface::UnknownFunction43cf00(int value) {
    field_0x38 = 0;
    if (field_0x34)
        field_0x34->UnknownVirtualSlot6(value);
    if (field_0x30)
        field_0x30->UnknownVirtualSlot6(value);
    for (int i = 0; i < 8; i++) {
        if (field_0x10[i])
            field_0x10[i]->UnknownVirtualSlot20(value);
    }
    for (int j = 0; j < field_0x38; j++) {
        UnknownControlEvent* event = &field_0x3c[j];
        if (field_0x34)
            event->modifiers = field_0x34->field_0x16d8;
        else
            event->modifiers = 0;
        switch (event->kind) {
        case 0:
            if (event->pressed == 0)
                g_UnknownGlobal56e26c->UnknownVirtualSlot13(
                    event, &field_0x34->field_0x260[event->control]);
            if (event->pressed == 1)
                g_UnknownGlobal56e26c->UnknownVirtualSlot14(
                    event, &field_0x34->field_0x260[event->control]);
            break;
        case 1:
            if (event->pressed == 0)
                g_UnknownGlobal56e26c->UnknownVirtualSlot13(event, &field_0x30->field_0x260[event->control]);
            if (event->pressed == 1)
                g_UnknownGlobal56e26c->UnknownVirtualSlot14(event, &field_0x30->field_0x260[event->control]);
            break;
        case 2:
        case 3:
            if (event->pressed == 0)
                g_UnknownGlobal56e26c->UnknownVirtualSlot13(
                    event, &field_0x10[event->device]->field_0x264[event->control]);
            if (event->pressed == 1)
                g_UnknownGlobal56e26c->UnknownVirtualSlot14(
                    event, &field_0x10[event->device]->field_0x264[event->control]);
            break;
        }
    }
    return 1;
}

// 0x004bf4f0: `control` on one device: kind 0 the keyboard, 1 the mouse,
// 2 and 3 joystick `device`.
int PCControlInterface::UnknownVirtualSlot3(int control, int kind, int modifier, int device) {
    switch (kind) {
    case 0:
        if (field_0x34)
            return field_0x34->UnknownVirtualSlot5(control, modifier, 0);
        break;
    case 1:
        if (field_0x30)
            return field_0x30->UnknownVirtualSlot5(control, modifier, 0);
        break;
    case 2:
    case 3:
        if (field_0x10[device])
            return field_0x10[device]->UnknownVirtualSlot2(control, modifier, 0);
        break;
    }
    return 0;
}

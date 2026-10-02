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
//
// UnknownReportError (0x004bf6a0, 978 bytes with jump tables): the switch
// cases (values traced from retail's decision tree, in retail body order)
// and both sprintf calls match, but VC6 here merges different case tails:
// retail keeps some cases as `push string; jmp` into a shared
// `lea ecx, [esp + 4]` and the rest whole. 518 of 978 bytes match.
#include <stdio.h>

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

// 0x004bf6a0: formats a DirectInput result with the caller's __FILE__ and
// __LINE__ (the text is not used further). Placed here by address only.
void UnknownReportError(long result, const char* file, int line) {
    char name[256];
    char text[1024];
    switch (result) {
    case (long)0x80004001:
        sprintf(name, "DIERR_UNSUPPORTED");
        break;
    case (long)0x8000000a:
        sprintf(name, "E_PENDING");
        break;
    case (long)0x80004002:
        sprintf(name, "DIERR_NOINTERFACE");
        break;
    case (long)0x80004005:
        sprintf(name, "DIERR_GENERIC");
        break;
    case (long)0x80040110:
        sprintf(name, "DIERR_NOAGGREGATION");
        break;
    case (long)0x80040201:
        sprintf(name, "DIERR_DEVICEFULL");
        break;
    case (long)0x80040200:
        sprintf(name, "DIERR_INSUFFICIENTPRIVS");
        break;
    case (long)0x80040154:
        sprintf(name, "DIERR_DEVICENOTREG");
        break;
    case (long)0x80040202:
        sprintf(name, "DIERR_MOREDATA");
        break;
    case (long)0x80040203:
        sprintf(name, "DIERR_NOTDOWNLOADED");
        break;
    case (long)0x80040204:
        sprintf(name, "DIERR_HASEFFECTS");
        break;
    case (long)0x80040205:
        sprintf(name, "DIERR_NOTEXCLUSIVEACQUIRED");
        break;
    case (long)0x80040206:
        sprintf(name, "DIERR_INCOMPLETEEFFECT");
        break;
    case (long)0x80040207:
        sprintf(name, "DIERR_NOTBUFFERED");
        break;
    case (long)0x80040208:
        sprintf(name, "DIERR_EFFECTPLAYING");
        break;
    case (long)0x80040209:
        sprintf(name, "DIERR_UNPLUGGED");
        break;
    case (long)0x8004020a:
        sprintf(name, "DIERR_REPORTFULL");
        break;
    case (long)0x80070002:
        sprintf(name, "DIERR_OBJECTNOTFOUND|DIERR_NOTFOUND");
        break;
    case (long)0x80070077:
        sprintf(name, "DIERR_BADDRIVERVER");
        break;
    case (long)0x80070057:
        sprintf(name, "DIERR_INVALIDPARAM");
        break;
    case (long)0x8007000e:
        sprintf(name, "DIERR_OUTOFMEMORY");
        break;
    case (long)0x80070015:
        sprintf(name, "DIERR_NOTINITIALIZED");
        break;
    case (long)0x8007001e:
        sprintf(name, "DIERR_INPUTLOST");
        break;
    case (long)0x800700aa:
        sprintf(name, "DIERR_ACQUIRED");
        break;
    case (long)0x8007000c:
        sprintf(name, "DIERR_NOTACQUIRED");
        break;
    case (long)0x80070005:
        sprintf(name, "DIERR_OTHERAPPHASPRIO|DIERR_READONLY|DIERR_HANDLEEXISTS");
        break;
    case (long)0x8007047e:
        sprintf(name, "DIERR_OLDDIRECTINPUTVERSION");
        break;
    case (long)0x800704df:
        sprintf(name, "DIERR_ALREADYINITIALIZED");
        break;
    case (long)0x80070481:
        sprintf(name, "DIERR_BETADIRECTINPUTVERSION");
        break;
    case 1:
        sprintf(name, "DI_NOTATTACHED|DI_BUFFEROVERFLOW|DI_PROPNOEFFECT|DI_NOEFFECT");
        break;
    case 3:
        sprintf(name, "DI_DOWNLOADSKIPPED");
        break;
    case 12:
        sprintf(name, "DI_TRUNCATEDANDRESTARTED");
        break;
    case 8:
        sprintf(name, "DI_TRUNCATED");
        break;
    case 4:
        sprintf(name, "DI_EFFECTRESTARTED");
        break;
    default:
        sprintf(name, "Unknown Error");
        break;
    }
    sprintf(text, "DirectInput Error %s in file %s at line %d\n", name, file, line);
}

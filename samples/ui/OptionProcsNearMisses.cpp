// Near misses for OptionProcs.cpp (canonical file:
// src/reconstructed/OptionProcs.cpp, included below for its types and its
// matched functions). Check: compile this file and compare each function
// with OptionProcsNearMisses.bindings.json.
//
// OptControlsDlg::UnknownVirtualSlot22 (0x004b5600, 341 bytes, 324 match):
//   the same instructions with `this` and the second branch's list box in
//   swapped registers (retail: this in ebx, the list in ebp; VC6 here the
//   reverse). An else around the base call, a nested 0x004b5570 test, one
//   list/row variable for both branches and a `this` copy do not move it.
// OptControlsDlg::UnknownFunction4b5760 (0x004b5760, 708 bytes, 259 match):
//   the same flow; retail keeps the mapping code in esi (the axis loop's
//   counter register) and rebuilds row + 1 in edi, VC6 here puts the code in
//   edi and row + 1 in esi, so every later push differs. Declaration order,
//   per-case `kind = 2` and reusing one variable for the axis and the code do
//   not move it.

#include "../../src/reconstructed/OptionProcs.cpp"

// 0x004b5600 (near miss)
int OptControlsDlg::UnknownVirtualSlot22(UnknownControlEvent* event, UnknownInputEntry* entry)
{
    int other;
    char text[128];
    if (!field_0x7f70)
        return GameObject::UnknownVirtualSlot22(event, entry);
    if (event->kind == 0 && event->control == 1) {
        UIListBox* keys = static_cast<UIListBox*>(UnknownFunction46ebf0("MapKeyListBox", 3));
        int row = keys->UnknownFunction476950();
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449350(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row, text);
        keys->UnknownFunction476ff0(row, text);
    } else {
        UIListBox* keys = static_cast<UIListBox*>(UnknownFunction46ebf0("MapKeyListBox", 3));
        int row = keys->UnknownFunction476950();
        if (!UnknownFunction4b5570(row, event->kind, event->control, text))
            return 1;
        if (g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448cc0(row, event->kind, event->control, &other)) {
            keys->UnknownFunction476ff0(other, "");
            g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, other, -1, 0);
        }
        keys->UnknownFunction476ff0(row, text);
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row, event->kind, event->control);
    }
    UnknownFunction4b5a20();
    return 1;
}

// 0x004b5760 (near miss)
void OptControlsDlg::UnknownFunction4b5760()
{
    JoystickDevice* joystick = g_UnknownGlobal56e26c->field_0x14->activeJoystick;
    MouseDevice* mouse = g_UnknownGlobal56e26c->field_0x14->mouse;
    UIListBox* keys = static_cast<UIListBox*>(UnknownFunction46ebf0("MapKeyListBox", 3));
    int row = keys->UnknownFunction476950();
    int code;
    int kind;
    int other;
    char text[128];
    if (row >= 5)
        return;
    if (joystick) {
        for (int i = 0; i < 6; i++) {
            float delta = joystick->UnknownFunction489e20(i) - field_0x7f58[i];
            if (delta < 0.0f)
                delta = -delta;
            if (delta > 16384.0f) {
                switch (i) {
                case 0:
                    code = -3;
                    break;
                case 1:
                    code = -5;
                    break;
                case 2:
                    code = -7;
                    break;
                case 3:
                    code = -9;
                    break;
                case 4:
                    code = -11;
                    break;
                case 5:
                    code = -13;
                    break;
                }
                kind = 2;
                goto found;
            }
        }
    }
    if (!mouse)
        return;
    if (mouse->UnknownVirtualSlot4(-3, 0x3f, 0) || mouse->UnknownVirtualSlot4(-2, 0x3f, 0)) {
        code = -3;
    } else if (mouse->UnknownVirtualSlot4(-5, 0x3f, 0) || mouse->UnknownVirtualSlot4(-4, 0x3f, 0)) {
        code = -5;
    } else {
        return;
    }
    kind = 1;
found:
    if (row % 2)
        row--;
    if (g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448cc0(row, kind, code, &other)) {
        keys->UnknownFunction476ff0(other, "");
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, other, -1, 0);
        if (other < 4) {
            g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, other + 1, -1, 0);
            keys->UnknownFunction476ff0(other + 1, "");
        }
    }
    g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row, kind, code);
    g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449350(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row, text);
    keys->UnknownFunction476ff0(row, text);
    if (row < 4) {
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row + 1, kind, code);
        keys->UnknownFunction476ff0(row + 1, text);
    }
    UnknownFunction4b5a20();
}


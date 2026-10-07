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
// OptControlsDlg::MapMovedInput (0x004b5760, 708 bytes, 259 match):
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
    if (!waitingForInput)
        return GameObject::UnknownVirtualSlot22(event, entry);
    if (event->kind == 0 && event->control == 1) {
        UIListBox* keys = static_cast<UIListBox*>(FindControl("MapKeyListBox", 3));
        int row = keys->GetSelectedRow();
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449350(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row, text);
        keys->SetRowText(row, text);
    } else {
        UIListBox* keys = static_cast<UIListBox*>(FindControl("MapKeyListBox", 3));
        int row = keys->GetSelectedRow();
        if (!GetInputText(row, event->kind, event->control, text))
            return 1;
        if (g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448cc0(row, event->kind, event->control, &other)) {
            keys->SetRowText(other, "");
            g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, other, -1, 0);
        }
        keys->SetRowText(row, text);
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row, event->kind, event->control);
    }
    EndInputWait();
    return 1;
}

// 0x004b5760 (near miss)
void OptControlsDlg::MapMovedInput()
{
    JoystickDevice* joystick = g_UnknownGlobal56e26c->field_0x14->activeJoystick;
    MouseDevice* mouse = g_UnknownGlobal56e26c->field_0x14->mouse;
    UIListBox* keys = static_cast<UIListBox*>(FindControl("MapKeyListBox", 3));
    int row = keys->GetSelectedRow();
    int code;
    int kind;
    int other;
    char text[128];
    if (row >= 5)
        return;
    if (joystick) {
        for (int i = 0; i < 6; i++) {
            float delta = joystick->UnknownFunction489e20(i) - axisValuesAtWait[i];
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
        keys->SetRowText(other, "");
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, other, -1, 0);
        if (other < 4) {
            g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, other + 1, -1, 0);
            keys->SetRowText(other + 1, "");
        }
    }
    g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row, kind, code);
    g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction449350(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row, text);
    keys->SetRowText(row, text);
    if (row < 4) {
        g_UnknownGlobal56e26c->field_0x33fc->UnknownFunction448c90(g_UnknownGlobal56e26c->field_0x33fc->field_0x00, row + 1, kind, code);
        keys->SetRowText(row + 1, text);
    }
    EndInputWait();
}


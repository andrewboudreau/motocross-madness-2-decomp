#include "PCKeyboardDevice.h"

#include "UnknownObject56e26c.h"

// 0x004c43c0
PCKeyboardDevice::PCKeyboardDevice() {}

// 0x004c4460: creates the system keyboard device with the keyboard data
// format, cooperative level 6 and a 16-entry buffer, then acquires it; on
// any failure the device is released.
int PCKeyboardDevice::UnknownVirtualSlot2() {
    if (g_UnknownGlobal56e26c->field_0x14->field_0xcc0->UnknownMethod9(
            GUID_SysKeyboard, IID_IDirectInputDevice7A, &field_0x25c, 0) < 0)
        goto failed;
    if (field_0x25c->UnknownMethod11(&c_dfDIKeyboard) < 0)
        goto failed;
    if (field_0x25c->UnknownMethod13(g_UnknownGlobal56e26c->field_0x31c, 6) < 0)
        goto failed;
    field_0x18.size = sizeof(field_0x18);
    field_0x25c->UnknownMethod15(&field_0x18);
    UnknownVirtualSlot3();
    if (!UnknownMethod4c2710(1, 0, 0, 16))
        goto failed;
    if (field_0x25c->UnknownMethod7() < 0)
        goto failed;
    return 1;
failed:
    if (field_0x25c) {
        field_0x25c->UnknownMethod2();
        field_0x25c = 0;
    }
    return 0;
}

// 0x004c4520: `control` through the mapping table, then slot 5.
int PCKeyboardDevice::UnknownVirtualSlot4(int control, int modifier, UnknownInputEntry* entry) {
    UnknownControlMapping* mapping = g_UnknownGlobal56e26c->field_0x14->field_0xcbc;
    if (!mapping)
        return 0;
    mapping->UnknownFunction43cba0(control, &control);
    if (control == -1)
        return 0;
    return UnknownVirtualSlot5(control, modifier, entry);
}

// 0x004c4570: whether `key` is down with `modifier` held; copies its entry.
int PCKeyboardDevice::UnknownVirtualSlot5(int key, int modifier, UnknownInputEntry* entry) {
    if (UnknownFunction48a240(modifier) && field_0x260[key].state == 1) {
        if (entry) {
            entry->state = 1;
            entry->field_0x04 = field_0x260[key].field_0x04;
            entry->field_0x08 = field_0x260[key].field_0x08;
            entry->field_0x0c = field_0x260[key].field_0x0c;
            entry->field_0x10 = field_0x260[key].field_0x10;
        }
        return 1;
    }
    return 0;
}

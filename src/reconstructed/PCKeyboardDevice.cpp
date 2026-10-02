#include "PCKeyboardDevice.h"

#include "UnknownObject56e26c.h"

// 0x004c43c0
PCKeyboardDevice::PCKeyboardDevice() {}

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

#include "PCKeyboardDevice.h"

#include "TrackGame.h"

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

// 0x004c45e0: reads the keyboard. A lost device is re-acquired; NumLock
// (0x45) is taken from the immediate state, every other key from the
// buffered events, which are queued on the ControlInterface. Then the key
// repeat runs for the six lists and the modifiers are refreshed.
int PCKeyboardDevice::UnknownVirtualSlot6(int value) {
    unsigned char keys[256];
    UnknownDeviceObjectData events[16];
    long count;
    if (!field_0x25c)
        return 0;
    UnknownFunction4c4830();
    count = -1;
    long result = field_0x25c->UnknownMethod9(sizeof(keys), keys);
    if (result < 0) {
        if (result == (long)0x8007001e) {
            if (field_0x25c->UnknownMethod7() < 0) {
                UnknownReportError(result, __FILE__, 1090);
                return 0;
            }
            return 1;
        }
        UnknownReportError(result, __FILE__, 1096);
        return 0;
    }
    if (keys[0x45] & 0x80)
        field_0x260[0x45].state = 1;
    else
        field_0x260[0x45].state = 0;
    result = field_0x25c->UnknownMethod10(sizeof(UnknownDeviceObjectData), 0, &count, 1);
    if (result == (long)0x8007001e || result == (long)0x8007000c) {
        if (field_0x25c->UnknownMethod7() < 0) {
            UnknownReportError(result, __FILE__, 1113);
            return 0;
        }
        result = field_0x25c->UnknownMethod10(sizeof(UnknownDeviceObjectData), 0, &count, 1);
    }
    if (result >= 0)
        result = field_0x25c->UnknownMethod10(sizeof(UnknownDeviceObjectData), events, &count, 0);
    if (result < 0)
        return 0;
    for (int i = 0; i < count; i++) {
        int key = events[i].offset;
        if (events[i].data & 0x80) {
            field_0x260[key].state = 1;
            field_0x260[key].field_0x10 = field_0x260[key].field_0x08;
            field_0x260[key].field_0x08 = events[i].timeStamp;
            g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(key, field_0x0c, 1, 0);
        } else if (key != 0x45) {
            field_0x260[key].state = 0;
            field_0x260[key].field_0x0c = field_0x260[key].field_0x04;
            field_0x260[key].field_0x04 = events[i].timeStamp;
            g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(key, field_0x0c, 0, 0);
        }
    }
    for (int list = 0; list < 6; list++)
        UnknownFunction48a0c0(list, value);
    UnknownFunction4c4830();
    return 1;
}

// 0x004c4830: bit 0 left Shift, 1 right Shift, 2 left Ctrl, 3 right Ctrl,
// 4 left Alt, 5 right Alt (DirectInput key codes), 7 the grave key.
void PCKeyboardDevice::UnknownFunction4c4830() {
    field_0x16d8 = 0;
    if (field_0x260[0x2a].state == 1)
        field_0x16d8 = 1;
    if (field_0x260[0x36].state == 1)
        field_0x16d8 |= 2;
    if (field_0x260[0x38].state == 1)
        field_0x16d8 |= 0x10;
    if (field_0x260[0xb8].state == 1)
        field_0x16d8 |= 0x20;
    if (field_0x260[0x1d].state == 1)
        field_0x16d8 |= 4;
    if (field_0x260[0x9d].state == 1)
        field_0x16d8 |= 8;
    if (field_0x260[0x29].state == 1)
        field_0x16d8 |= 0x80;
}

#include <string.h>

#include "PCMouseDevice.h"
#include "UnknownObject56e26c.h"

// 0x004c48c0
PCMouseDevice::PCMouseDevice() {
    memset(field_0x2d8, 0, sizeof(field_0x2d8));
}

// 0x004c4970: creates the system mouse device, records its axis and button
// counts, sets the mouse data format, cooperative level 5 and a 16-entry
// buffer, then acquires it.
int PCMouseDevice::UnknownVirtualSlot2() {
    UnknownDeviceCaps caps;
    if (g_UnknownGlobal56e26c->field_0x14->field_0xcc0->UnknownMethod9(
            GUID_SysMouse, IID_IDirectInputDevice7A, &field_0x25c, 0) < 0)
        goto failed;
    caps.size = sizeof(caps);
    if (field_0x25c->UnknownMethod3(&caps) < 0)
        goto failed;
    field_0x08 = caps.buttons;
    field_0x04 = caps.axes;
    if (field_0x25c->UnknownMethod11(&c_dfDIMouse) < 0)
        goto failed;
    if (field_0x25c->UnknownMethod13(g_UnknownGlobal56e26c->field_0x31c, 5) < 0)
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
    return 0;
}

// 0x004c4a50: mouse subtypes 3-5 select modes 0-2 in +0x10.
void PCMouseDevice::UnknownVirtualSlot3() {
    int subtype = DEVICE_SUBTYPE(field_0x18.deviceType);
    if (subtype == 3)
        field_0x10 = 0;
    else if (subtype == 4)
        field_0x10 = 1;
    else if (subtype == 5)
        field_0x10 = 2;
}

// 0x004c4a90: whether `button` is down (with `modifier` held, when there is
// a keyboard); copies its entry.
int PCMouseDevice::UnknownVirtualSlot5(int button, int modifier, UnknownInputEntry* entry) {
    UnknownInterface56e26c* keyboard = g_UnknownGlobal56e26c->field_0x14->field_0x34;
    if ((!keyboard || keyboard->UnknownFunction48a240(modifier)) &&
        field_0x260[button].state == 1) {
        if (entry) {
            entry->state = 1;
            entry->field_0x04 = field_0x260[button].field_0x04;
            entry->field_0x08 = field_0x260[button].field_0x08;
            entry->field_0x0c = field_0x260[button].field_0x0c;
            entry->field_0x10 = field_0x260[button].field_0x10;
        }
        return 1;
    }
    return 0;
}

// 0x004c4b10: controls -2 to -5 are movement directions (beyond 30),
// -14 and -15 a double click of buttons 0 and 1 (presses under 175 apart),
// and -16 to -19 a direction while button 0 is down; others go through the
// mapping table to slot 5.
int PCMouseDevice::UnknownVirtualSlot4(int control, int modifier, UnknownInputEntry* entry) {
    if (control < 0) {
        switch (control) {
        case -2:
            if (field_0x2d8[0] < -30)
                return 1;
            break;
        case -3:
            if (field_0x2d8[0] > 30)
                return 1;
            break;
        case -4:
            if (field_0x2d8[1] < -30)
                return 1;
            break;
        case -5:
            if (field_0x2d8[1] > 30)
                return 1;
            break;
        case -14:
            if (field_0x260[0].field_0x08 != field_0x260[0].field_0x10 &&
                field_0x260[0].state == 1 &&
                field_0x260[0].field_0x08 - field_0x260[0].field_0x10 < 175)
                return 1;
            break;
        case -15:
            if (field_0x260[1].field_0x08 != field_0x260[1].field_0x10 &&
                field_0x260[1].state == 1 &&
                field_0x260[1].field_0x08 - field_0x260[1].field_0x10 < 175)
                return 1;
            break;
        case -16:
            if (field_0x260[0].state == 1 && UnknownVirtualSlot4(-2, 0x3f, 0))
                return 1;
            break;
        case -17:
            if (field_0x260[0].state == 1 && UnknownVirtualSlot4(-3, 0x3f, 0))
                return 1;
            break;
        case -18:
            if (field_0x260[0].state == 1 && UnknownVirtualSlot4(-4, 0x3f, 0))
                return 1;
            break;
        case -19:
            if (field_0x260[0].state == 1 && UnknownVirtualSlot4(-5, 0x3f, 0))
                return 1;
            break;
        }
        return 0;
    }
    UnknownControlMapping* mapping = g_UnknownGlobal56e26c->field_0x14->field_0xcbc;
    if (!mapping)
        return 0;
    mapping->UnknownFunction43cc10(control, &control);
    if (control == -1)
        return 0;
    return UnknownVirtualSlot5(control, modifier, entry);
}

// 0x004c4cf0: reads the buffered button events (offsets 12-15, re-acquiring
// a lost device) and queues them on the ControlInterface, then reads the
// movement into +0x2d8 and feeds x and y to the bindings.
int PCMouseDevice::UnknownVirtualSlot6(int value) {
    UnknownDeviceObjectData events[16];
    long count;
    int i;
    if (!field_0x25c)
        return 0;
    count = -1;
    long result;
    result = field_0x25c->UnknownMethod10(sizeof(UnknownDeviceObjectData), 0, &count, 1);
    if (result == (long)0x8007001e || result == (long)0x8007000c) {
        if (field_0x25c->UnknownMethod7() < 0) {
            UnknownReportError(result, __FILE__, 1316);
            return 0;
        }
        result = field_0x25c->UnknownMethod10(sizeof(UnknownDeviceObjectData), 0, &count, 1);
    }
    if (result < 0)
        goto failed;
    if (field_0x25c->UnknownMethod10(sizeof(UnknownDeviceObjectData), events, &count, 0) < 0)
        goto failed;
    for (i = 0; i < count; i++) {
        int button;
        switch (events[i].offset) {
        case 12:
            button = 0;
            break;
        case 13:
            button = 1;
            break;
        case 14:
            button = 2;
            break;
        case 15:
            button = 3;
            break;
        default:
            continue;
        }
        if (events[i].data & 0x80) {
            field_0x260[button].state = 1;
            field_0x260[button].field_0x10 = field_0x260[button].field_0x08;
            field_0x260[button].field_0x08 = events[i].timeStamp;
            g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(button, field_0x0c, 1, 0);
        } else {
            field_0x260[button].state = 0;
            field_0x260[button].field_0x0c = field_0x260[button].field_0x04;
            field_0x260[button].field_0x04 = events[i].timeStamp;
            g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(button, field_0x0c, 0, 0);
        }
    }
    if (field_0x25c->UnknownMethod9(sizeof(field_0x2d8), field_0x2d8) < 0)
        goto failed;
    UnknownFunction48a550(0, (float)field_0x2d8[0]);
    UnknownFunction48a550(1, (float)field_0x2d8[1]);
    return 1;
failed:
    return 0;
}

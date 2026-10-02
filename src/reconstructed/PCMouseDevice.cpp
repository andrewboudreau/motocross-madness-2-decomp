#include <string.h>

#include "PCMouseDevice.h"
#include "KeyboardDevice.h"
#include "TrackGame.h"

// 0x004c48c0
PCMouseDevice::PCMouseDevice() {
    memset(movement, 0, sizeof(movement));
}

// 0x004c4970: creates the system mouse device, records its axis and button
// counts, sets the mouse data format, cooperative level 5 and a 16-entry
// buffer, then acquires it.
int PCMouseDevice::UnknownVirtualSlot2() {
    UnknownDeviceCaps caps;
    if (g_UnknownGlobal56e26c->field_0x14->directInput->CreateDeviceEx(
            GUID_SysMouse, IID_IDirectInputDevice7A, &device, 0) < 0)
        goto failed;
    caps.size = sizeof(caps);
    if (device->GetCapabilities(&caps) < 0)
        goto failed;
    buttonCount = caps.buttons;
    axisCount = caps.axes;
    if (device->SetDataFormat(&c_dfDIMouse) < 0)
        goto failed;
    if (device->SetCooperativeLevel(g_UnknownGlobal56e26c->field_0x31c, 5) < 0)
        goto failed;
    deviceInfo.size = sizeof(deviceInfo);
    device->GetDeviceInfo(&deviceInfo);
    UnknownVirtualSlot3();
    if (!UnknownMethod4c2710(1, 0, 0, 16))
        goto failed;
    if (device->Acquire() < 0)
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004c4a50: mouse subtypes 3-5 select modes 0-2 in +0x10.
void PCMouseDevice::UnknownVirtualSlot3() {
    int subtype = DEVICE_SUBTYPE(deviceInfo.deviceType);
    if (subtype == 3)
        deviceSubtype = 0;
    else if (subtype == 4)
        deviceSubtype = 1;
    else if (subtype == 5)
        deviceSubtype = 2;
}

// 0x004c4a90: whether `button` is down (with `modifier` held, when there is
// a keyboard); copies its entry.
int PCMouseDevice::UnknownVirtualSlot5(int button, int modifier, UnknownInputEntry* entry) {
    KeyboardDevice* keyboard = g_UnknownGlobal56e26c->field_0x14->keyboard;
    if ((!keyboard || keyboard->UnknownFunction48a240(modifier)) &&
        buttonStates[button].state == 1) {
        if (entry) {
            entry->state = 1;
            entry->releaseTime = buttonStates[button].releaseTime;
            entry->pressTime = buttonStates[button].pressTime;
            entry->previousReleaseTime = buttonStates[button].previousReleaseTime;
            entry->previousPressTime = buttonStates[button].previousPressTime;
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
            if (movement[0] < -30)
                return 1;
            break;
        case -3:
            if (movement[0] > 30)
                return 1;
            break;
        case -4:
            if (movement[1] < -30)
                return 1;
            break;
        case -5:
            if (movement[1] > 30)
                return 1;
            break;
        case -14:
            if (buttonStates[0].pressTime != buttonStates[0].previousPressTime &&
                buttonStates[0].state == 1 &&
                buttonStates[0].pressTime - buttonStates[0].previousPressTime < 175)
                return 1;
            break;
        case -15:
            if (buttonStates[1].pressTime != buttonStates[1].previousPressTime &&
                buttonStates[1].state == 1 &&
                buttonStates[1].pressTime - buttonStates[1].previousPressTime < 175)
                return 1;
            break;
        case -16:
            if (buttonStates[0].state == 1 && UnknownVirtualSlot4(-2, 0x3f, 0))
                return 1;
            break;
        case -17:
            if (buttonStates[0].state == 1 && UnknownVirtualSlot4(-3, 0x3f, 0))
                return 1;
            break;
        case -18:
            if (buttonStates[0].state == 1 && UnknownVirtualSlot4(-4, 0x3f, 0))
                return 1;
            break;
        case -19:
            if (buttonStates[0].state == 1 && UnknownVirtualSlot4(-5, 0x3f, 0))
                return 1;
            break;
        }
        return 0;
    }
    UnknownControlMapping* mapping = g_UnknownGlobal56e26c->field_0x14->mapping;
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
    if (!device)
        return 0;
    count = -1;
    long result;
    result = device->GetDeviceData(sizeof(UnknownDeviceObjectData), 0, &count, 1);
    if (result == (long)0x8007001e || result == (long)0x8007000c) {
        if (device->Acquire() < 0) {
            UnknownReportError(result, __FILE__, 1316);
            return 0;
        }
        result = device->GetDeviceData(sizeof(UnknownDeviceObjectData), 0, &count, 1);
    }
    if (result < 0)
        goto failed;
    if (device->GetDeviceData(sizeof(UnknownDeviceObjectData), events, &count, 0) < 0)
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
            buttonStates[button].state = 1;
            buttonStates[button].previousPressTime = buttonStates[button].pressTime;
            buttonStates[button].pressTime = events[i].timeStamp;
            g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(button, deviceKind, 1, 0);
        } else {
            buttonStates[button].state = 0;
            buttonStates[button].previousReleaseTime = buttonStates[button].releaseTime;
            buttonStates[button].releaseTime = events[i].timeStamp;
            g_UnknownGlobal56e26c->field_0x14->UnknownFunction43cea0(button, deviceKind, 0, 0);
        }
    }
    if (device->GetDeviceState(sizeof(movement), movement) < 0)
        goto failed;
    UnknownFunction48a550(0, (float)movement[0]);
    UnknownFunction48a550(1, (float)movement[1]);
    return 1;
failed:
    return 0;
}

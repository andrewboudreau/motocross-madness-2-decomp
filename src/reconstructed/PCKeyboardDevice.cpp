#include "PCKeyboardDevice.h"

#include "TrackGame.h"

// 0x004c43c0
PCKeyboardDevice::PCKeyboardDevice() {}

// 0x004c4460: creates the system keyboard device with the keyboard data
// format, cooperative level 6 and a 16-entry buffer, then acquires it; on
// any failure the device is released.
int PCKeyboardDevice::UnknownVirtualSlot2() {
    if (g_TrackGame->controlInterface->directInput->CreateDeviceEx(
            GUID_SysKeyboard, IID_IDirectInputDevice7A, &device, 0) < 0)
        goto failed;
    if (device->SetDataFormat(&c_dfDIKeyboard) < 0)
        goto failed;
    if (device->SetCooperativeLevel(g_TrackGame->field_0x31c, 6) < 0)
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
    if (device) {
        device->Release();
        device = 0;
    }
    return 0;
}

// 0x004c4520: `control` through the mapping table, then slot 5.
int PCKeyboardDevice::UnknownVirtualSlot4(int control, int modifier, UnknownInputEntry* entry) {
    UnknownControlMapping* mapping = g_TrackGame->controlInterface->mapping;
    if (!mapping)
        return 0;
    mapping->UnknownFunction43cba0(control, &control);
    if (control == -1)
        return 0;
    return UnknownVirtualSlot5(control, modifier, entry);
}

// 0x004c4570: whether `key` is down with `modifier` held; copies its entry.
int PCKeyboardDevice::UnknownVirtualSlot5(int key, int modifier, UnknownInputEntry* entry) {
    if (UnknownFunction48a240(modifier) && keyStates[key].state == 1) {
        if (entry) {
            entry->state = 1;
            entry->releaseTime = keyStates[key].releaseTime;
            entry->pressTime = keyStates[key].pressTime;
            entry->previousReleaseTime = keyStates[key].previousReleaseTime;
            entry->previousPressTime = keyStates[key].previousPressTime;
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
    if (!device)
        return 0;
    UnknownFunction4c4830();
    count = -1;
    long result = device->GetDeviceState(sizeof(keys), keys);
    if (result < 0) {
        if (result == (long)0x8007001e) {
            if (device->Acquire() < 0) {
                UnknownReportError(result, __FILE__, 1090);
                return 0;
            }
            return 1;
        }
        UnknownReportError(result, __FILE__, 1096);
        return 0;
    }
    if (keys[0x45] & 0x80)
        keyStates[0x45].state = 1;
    else
        keyStates[0x45].state = 0;
    result = device->GetDeviceData(sizeof(UnknownDeviceObjectData), 0, &count, 1);
    if (result == (long)0x8007001e || result == (long)0x8007000c) {
        if (device->Acquire() < 0) {
            UnknownReportError(result, __FILE__, 1113);
            return 0;
        }
        result = device->GetDeviceData(sizeof(UnknownDeviceObjectData), 0, &count, 1);
    }
    if (result >= 0)
        result = device->GetDeviceData(sizeof(UnknownDeviceObjectData), events, &count, 0);
    if (result < 0)
        return 0;
    for (int i = 0; i < count; i++) {
        int key = events[i].offset;
        if (events[i].data & 0x80) {
            keyStates[key].state = 1;
            keyStates[key].previousPressTime = keyStates[key].pressTime;
            keyStates[key].pressTime = events[i].timeStamp;
            g_TrackGame->controlInterface->UnknownFunction43cea0(key, deviceKind, 1, 0);
        } else if (key != 0x45) {
            keyStates[key].state = 0;
            keyStates[key].previousReleaseTime = keyStates[key].releaseTime;
            keyStates[key].releaseTime = events[i].timeStamp;
            g_TrackGame->controlInterface->UnknownFunction43cea0(key, deviceKind, 0, 0);
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
    modifierState = 0;
    if (keyStates[0x2a].state == 1)
        modifierState = 1;
    if (keyStates[0x36].state == 1)
        modifierState |= 2;
    if (keyStates[0x38].state == 1)
        modifierState |= 0x10;
    if (keyStates[0xb8].state == 1)
        modifierState |= 0x20;
    if (keyStates[0x1d].state == 1)
        modifierState |= 4;
    if (keyStates[0x9d].state == 1)
        modifierState |= 8;
    if (keyStates[0x29].state == 1)
        modifierState |= 0x80;
}

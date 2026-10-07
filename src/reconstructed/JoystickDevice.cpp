#include "JoystickDevice.h"

#include "KeyboardDevice.h"
#include "TrackGame.h"

// 0x00489800
JoystickDevice::JoystickDevice(int index) : PCInputDevice(2) {
    joystickIndex = index;
    for (int i = 0; i < 6; i++) {
        axisValues[i] = 0;
        axisBindings[i].Init(1, 1);
    }
    for (int j = 0; j < 32; j++)
        buttonStates[j].state = 0;
    directionFlipped = g_TrackGame->GetRegistryFlag("JoyDirectionFlipped", 0);
}

// 0x00489920
JoystickDevice::~JoystickDevice() {}

// Modifier test shared by the button queries: through the keyboard when
// there is one, else only "none" (0, 0x80000000 or 0x3f) holds. Inline: it
// has no retail body of its own.
// 0x00489980: with a keyboard, `modifier` must hold on it; without one,
// only "none" (0, 0x80000000 or 0x3f) is accepted.
int JoystickDevice::UnknownVirtualSlot2(int button, int modifier, UnknownInputEntry* entry) {
    KeyboardDevice* keyboard = g_TrackGame->controlInterface->keyboard;
    if (((keyboard && keyboard->UnknownFunction48a240(modifier)) ||
         (!keyboard && (modifier == 0 || modifier == (int)0x80000000 || modifier == 0x3f))) &&
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

// 0x00489a20: axis n's binding reports controls -(2n + 2) (low) and
// -(2n + 3) (high).
int JoystickDevice::UnknownFunction489a20(UnknownControlBinding* binding, float step,
                                          float interval) {
    binding->field_0x00 = this;
    binding->field_0x1c = step;
    binding->field_0x20 = interval;
    axisBindings[binding->field_0x08].Add(binding);
    switch (binding->field_0x08) {
    case 0:
        binding->UnknownFunction43cce0(-2, -3);
        break;
    case 1:
        binding->UnknownFunction43cce0(-5, -4);
        break;
    case 2:
        binding->UnknownFunction43cce0(-7, -6);
        break;
    case 3:
        binding->UnknownFunction43cce0(-9, -8);
        break;
    case 4:
        binding->UnknownFunction43cce0(-11, -10);
        break;
    case 5:
        binding->UnknownFunction43cce0(-13, -12);
        break;
    }
    return 1;
}

// 0x00489b70: drops every binding with id `id`.
void JoystickDevice::UnknownVirtualSlot0(int id) {
    for (int axis = 0; axis < 6; axis++) {
        for (int i = 0; i < axisBindings[axis].m_count; i++) {
            UnknownControlBinding* binding = axisBindings[axis].Get(i);
            if (binding->field_0x04 == id)
                axisBindings[axis].Remove(binding);
        }
    }
}

// 0x00489c00
void JoystickDevice::UnknownFunction489c00(int axis, float value, float range) {
    for (int i = 0; i < axisBindings[axis].m_count; i++)
        axisBindings[axis].Get(i)->UnknownFunction43cd20(value, range);
}

// 0x00489c60: controls -2 to -13 are axis directions (below 16384 or above
// 49152, for enabled axes); others go through the control mapping to slot 2.
int JoystickDevice::UnknownFunction489c60(int control, int modifier, UnknownInputEntry* entry) {
    if (control < 0) {
        switch (control) {
        case -2:
            if (field_0x14_axis0 && axisValues[0] < 16384.0f)
                return 1;
            break;
        case -3:
            if (field_0x14_axis0 && axisValues[0] > 49152.0f)
                return 1;
            break;
        case -4:
            if (field_0x14_axis1 && axisValues[1] < 16384.0f)
                return 1;
            break;
        case -5:
            if (field_0x14_axis1 && axisValues[1] > 49152.0f)
                return 1;
            break;
        case -6:
            if (field_0x14_axis2 && axisValues[2] < 16384.0f)
                return 1;
            break;
        case -7:
            if (field_0x14_axis2 && axisValues[2] > 49152.0f)
                return 1;
            break;
        case -8:
            if (field_0x14_axis3 && axisValues[3] < 16384.0f)
                return 1;
            break;
        case -9:
            if (field_0x14_axis3 && axisValues[3] > 49152.0f)
                return 1;
            break;
        case -10:
            if (field_0x14_axis4 && axisValues[4] < 16384.0f)
                return 1;
            break;
        case -11:
            if (field_0x14_axis4 && axisValues[4] > 49152.0f)
                return 1;
            break;
        case -12:
            if (field_0x14_axis5 && axisValues[5] < 16384.0f)
                return 1;
            break;
        case -13:
            if (field_0x14_axis5 && axisValues[5] > 49152.0f)
                return 1;
            break;
        }
        return 0;
    }
    UnknownControlMapping* mapping = g_TrackGame->controlInterface->mapping;
    if (!mapping)
        return 0;
    mapping->UnknownFunction43cbd0(control, &control, joystickIndex);
    if (control == -1)
        return 0;
    return UnknownVirtualSlot2(control, modifier, entry);
}

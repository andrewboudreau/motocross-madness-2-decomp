#include "JoystickDevice.h"

#include "KeyboardDevice.h"
#include "TrackGame.h"

// 0x00489800
JoystickDevice::JoystickDevice(int index) : PCInputDevice(2) {
    field_0x260 = index;
    for (int i = 0; i < 6; i++) {
        field_0x4e4[i] = 0;
        field_0x4fc[i].Init(1, 1);
    }
    for (int j = 0; j < 32; j++)
        field_0x264[j].state = 0;
    field_0x574_bit0 = g_UnknownGlobal56e26c->UnknownVirtualSlot22("JoyDirectionFlipped", 0);
}

// 0x00489920
JoystickDevice::~JoystickDevice() {}

// Modifier test shared by the button queries: through the keyboard when
// there is one, else only "none" (0, 0x80000000 or 0x3f) holds. Inline: it
// has no retail body of its own.
// 0x00489980: with a keyboard, `modifier` must hold on it; without one,
// only "none" (0, 0x80000000 or 0x3f) is accepted.
int JoystickDevice::UnknownVirtualSlot2(int button, int modifier, UnknownInputEntry* entry) {
    KeyboardDevice* keyboard = g_UnknownGlobal56e26c->field_0x14->field_0x34;
    if (((keyboard && keyboard->UnknownFunction48a240(modifier)) ||
         (!keyboard && (modifier == 0 || modifier == (int)0x80000000 || modifier == 0x3f))) &&
        field_0x264[button].state == 1) {
        if (entry) {
            entry->state = 1;
            entry->field_0x04 = field_0x264[button].field_0x04;
            entry->field_0x08 = field_0x264[button].field_0x08;
            entry->field_0x0c = field_0x264[button].field_0x0c;
            entry->field_0x10 = field_0x264[button].field_0x10;
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
    field_0x4fc[binding->field_0x08].Add(binding);
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
        for (int i = 0; i < field_0x4fc[axis].m_count; i++) {
            UnknownControlBinding* binding = field_0x4fc[axis].Get(i);
            if (binding->field_0x04 == id)
                field_0x4fc[axis].Remove(binding);
        }
    }
}

// 0x00489c00
void JoystickDevice::UnknownFunction489c00(int axis, float value, float range) {
    for (int i = 0; i < field_0x4fc[axis].m_count; i++)
        field_0x4fc[axis].Get(i)->UnknownFunction43cd20(value, range);
}

// 0x00489c60: controls -2 to -13 are axis directions (below 16384 or above
// 49152, for enabled axes); others go through the control mapping to slot 2.
int JoystickDevice::UnknownFunction489c60(int control, int modifier, UnknownInputEntry* entry) {
    if (control < 0) {
        switch (control) {
        case -2:
            if (field_0x14_axis0 && field_0x4e4[0] < 16384.0f)
                return 1;
            break;
        case -3:
            if (field_0x14_axis0 && field_0x4e4[0] > 49152.0f)
                return 1;
            break;
        case -4:
            if (field_0x14_axis1 && field_0x4e4[1] < 16384.0f)
                return 1;
            break;
        case -5:
            if (field_0x14_axis1 && field_0x4e4[1] > 49152.0f)
                return 1;
            break;
        case -6:
            if (field_0x14_axis2 && field_0x4e4[2] < 16384.0f)
                return 1;
            break;
        case -7:
            if (field_0x14_axis2 && field_0x4e4[2] > 49152.0f)
                return 1;
            break;
        case -8:
            if (field_0x14_axis3 && field_0x4e4[3] < 16384.0f)
                return 1;
            break;
        case -9:
            if (field_0x14_axis3 && field_0x4e4[3] > 49152.0f)
                return 1;
            break;
        case -10:
            if (field_0x14_axis4 && field_0x4e4[4] < 16384.0f)
                return 1;
            break;
        case -11:
            if (field_0x14_axis4 && field_0x4e4[4] > 49152.0f)
                return 1;
            break;
        case -12:
            if (field_0x14_axis5 && field_0x4e4[5] < 16384.0f)
                return 1;
            break;
        case -13:
            if (field_0x14_axis5 && field_0x4e4[5] > 49152.0f)
                return 1;
            break;
        }
        return 0;
    }
    UnknownControlMapping* mapping = g_UnknownGlobal56e26c->field_0x14->field_0xcbc;
    if (!mapping)
        return 0;
    mapping->UnknownFunction43cbd0(control, &control, field_0x260);
    if (control == -1)
        return 0;
    return UnknownVirtualSlot2(control, modifier, entry);
}

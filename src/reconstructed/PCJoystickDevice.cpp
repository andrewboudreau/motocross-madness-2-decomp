#include <math.h>
#include <stdio.h>
#include <string.h>

#include "PCJoystickDevice.h"

#include "UnknownObject56e26c.h"

// 0x004c2770: repeats JoystickDevice's initialisation with PCJoystickDevice's
// own fields interleaved.
PCJoystickDevice::PCJoystickDevice(int index) : JoystickDevice(index) {
    field_0x260 = index;
    for (int k = 0; k < 5; k++)
        field_0x578[k] = 0;
    for (int i = 0; i < 6; i++) {
        field_0x58c[i] = 0;
        field_0x4e4[i] = 0;
        field_0x5a4[i] = 0;
        field_0x4fc[i].Init(1, 1);
    }
    for (int j = 0; j < 32; j++)
        field_0x264[j].state = 0;
    for (int n = 0; n < 4; n++)
        field_0x5d8[n] = -1;
    field_0x5e8 = 0;
    field_0x5d4_bit0 = 1;
    field_0x5d4_bit1 = 1;
    field_0x5d5 = 0;
    field_0x574_bit0 = g_UnknownGlobal56e26c->UnknownVirtualSlot22("JoyDirectionFlipped", 0);
}

// 0x004c28d0: releases the effects and resets the device (calls bind
// statically in a destructor).
PCJoystickDevice::~PCJoystickDevice() {
    UnknownVirtualSlot15();
    UnknownVirtualSlot16();
}

// 0x004c3b20: POV index in hundredths of a degree, as radians; -1 when
// centred (-1 or 0xffff) or out of range.
int PCJoystickDevice::UnknownVirtualSlot3(int index, float* angle) {
    if (field_0x5e8 > index) {             // +0x5e8: POV count
        unsigned long value = field_0x5d8[index];
        if (value != (unsigned long)-1 && value != 0xffff) {
            *angle = value * (0.01f * 3.1415926f / 180.0f);
            return 1;
        }
        *angle = -1.0f;
        return 1;
    }
    *angle = -1.0f;
    return 0;
}

// 0x004c3ba0: the slot 3 angle as a direction (0, 0 when centred).
int PCJoystickDevice::UnknownVirtualSlot4(int index, float* x, float* y) {
    float angle;
    if (UnknownVirtualSlot3(index, &angle)) {
        if (angle == -1.0f) {
            *y = 0.0f;
            *x = 0.0f;
            return 1;
        }
        *x = (float)sin(angle);
        *y = (float)cos(angle);
        return 1;
    }
    *x = 0.0f;
    return 0;
}

// 0x004c3c10: property 9 (consistent with DIPROP_AUTOCENTER) for device type 3.
int PCJoystickDevice::UnknownVirtualSlot5(int enable) {
    if (field_0x0c == 3)
        return UnknownMethod4c2710(9, 0, 0, enable != 0);
    return 0;
}

// 0x004c40b0: stops effect `effect`.
int PCJoystickDevice::UnknownVirtualSlot11(int effect) {
    if (field_0x25c && effect < 5 && field_0x578[effect] && field_0x0c == 3)
        return field_0x578[effect]->UnknownMethod8() >= 0;
    return 0;
}

// 0x004c40f0: starts effect `effect`.
int PCJoystickDevice::UnknownVirtualSlot12(int effect, unsigned long iterations,
                                           unsigned long flags) {
    if (field_0x25c && effect < 5 && field_0x578[effect] && field_0x0c == 3)
        return field_0x578[effect]->UnknownMethod7(iterations, flags) >= 0;
    return 0;
}

// 0x004c4140: nonzero while effect `effect` reports status 1 (playing).
int PCJoystickDevice::UnknownVirtualSlot13(int effect) {
    if (field_0x25c && effect < 5 && field_0x578[effect] && field_0x0c == 3) {
        unsigned long status;
        field_0x578[effect]->UnknownMethod9(&status);
        return status == 1;
    }
    return 0;
}

// 0x004c3af0: joystick subtypes 4-7 are recorded in +0x10.
void PCJoystickDevice::UnknownVirtualSlot19() {
    int subtype = DEVICE_SUBTYPE(field_0x18.deviceType);
    if (subtype == 4)
        field_0x10 = subtype;
    else if (subtype == 5)
        field_0x10 = subtype;
    else if (subtype == 6)
        field_0x10 = subtype;
    else if (subtype == 7)
        field_0x10 = subtype;
}

// 0x00689958: effects stored by the enumeration callback.
static int s_UnknownEffectCount;

// 0x004c4260: method 19 callback; copies each effect into the caller's
// array, stopping at 32. The formatted line is not used further.
static int __stdcall UnknownEnumEffectsCallback(const UnknownEffectInfo* info, void* context) {
    UnknownEffectInfo* effects = (UnknownEffectInfo*)context;
    if (!effects)
        return 0;
    effects[s_UnknownEffectCount] = *info;
    char typeName[260];
    UnknownFunction4beef0(effects[s_UnknownEffectCount].guid, typeName);
    char text[528];   // the frame fixes this size
    sprintf(text, "\t '%s' '%s'\n", typeName, effects[s_UnknownEffectCount].name);
    s_UnknownEffectCount++;
    return s_UnknownEffectCount < 32;
}

// 0x004c4190: clears five entries, then enumerates the device's effects into
// `effects` and stores how many were found.
int PCJoystickDevice::UnknownVirtualSlot14(UnknownEffectInfo* effects, int* count) {
    if (field_0x0c != 3)
        return 0;
    for (int i = 0; i < 5; i++) {
        memset(&effects[i], 0, sizeof(effects[i]));
        effects[i].size = sizeof(effects[i]);
    }
    if (!field_0x25c)
        return 0;
    char text[256];
    sprintf(text, "ForceFeedback Joystick Detected\nJoystickDevice::EnumEffects ('%s')\n",
            field_0x18.productName);
    s_UnknownEffectCount = 0;
    if (field_0x25c->UnknownMethod19(UnknownEnumEffectsCallback, effects, 0) < 0)
        return 0;
    *count = s_UnknownEffectCount;
    return 1;
}

// 0x004c4320
void PCJoystickDevice::UnknownVirtualSlot15() {
    for (int i = 0; i < 5; i++) {
        if (field_0x578[i]) {
            field_0x578[i]->UnknownMethod2();
            field_0x578[i] = 0;
        }
    }
}

// 0x004c4350: command 1.
int PCJoystickDevice::UnknownVirtualSlot16() {
    if (field_0x25c && field_0x0c == 3)
        return UnknownVirtualSlot17(1);
    return 0;
}

// 0x004c4370
int PCJoystickDevice::UnknownVirtualSlot17(int command) {
    if (field_0x25c && field_0x0c == 3)
        field_0x25c->UnknownMethod22(command);
    return 0;
}

// 0x004c4390: command 4 when paused, 8 when resumed.
int PCJoystickDevice::UnknownVirtualSlot18(int paused) {
    if (field_0x0c == 3) {
        if (paused)
            return UnknownVirtualSlot17(4);
        return UnknownVirtualSlot17(8);
    }
    return 1;
}

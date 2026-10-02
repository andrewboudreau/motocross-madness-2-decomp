#include <math.h>
#include <stdio.h>
#include <string.h>

#include "PCJoystickDevice.h"

#include "TrackGame.h"

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

// 0x004c2930: creates the device, takes its description and capabilities
// (force feedback makes it type 3), enumerates the axes and reads each axis
// range, with a 5% dead zone. On failure the device is released.
int PCJoystickDevice::UnknownFunction4c2930(const UnknownDeviceInstance* instance) {
    UnknownInputPropertyRange range;
    UnknownDeviceCaps caps;
    int half;
    const UnknownGuid& guid = instance->instanceGuid;
    if (g_UnknownGlobal56e26c->field_0x14->field_0xcc0->UnknownMethod9(
            guid, IID_IDirectInputDevice7A, &field_0x25c, 0) < 0)
        goto failed;
    field_0x18 = *instance;
    field_0x18.size = sizeof(field_0x18);
    field_0x25c->UnknownMethod15(&field_0x18);
    UnknownVirtualSlot19();
    if (g_UnknownGlobal56e26c->field_0x14->field_0xcc0->UnknownMethod5(guid) < 0)
        goto failed;
    if (field_0x25c->UnknownMethod11(&c_dfDIJoystick) < 0)
        goto failed;
    if (field_0x25c->UnknownMethod13(g_UnknownGlobal56e26c->field_0x31c, 5) < 0)
        goto failed;
    caps.size = sizeof(caps);
    if (field_0x25c->UnknownMethod3(&caps) < 0)
        goto failed;
    field_0x08 = caps.buttons;
    field_0x04 = caps.axes;
    if (caps.flags & 0x100)
        field_0x0c = 3;
    field_0x5e8 = caps.povs > 1 ? caps.povs : 1;
    field_0x25c->UnknownMethod4(UnknownEnumObjectsCallback, this, 3);
    range.size = sizeof(range);
    range.headerSize = 0x10;
    range.how = 1;
    if (field_0x14_axis0) {
        range.object = 0;
        if (field_0x25c->UnknownMethod5(4, &range) < 0)
            goto failed;
        field_0x58c[0] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        field_0x5bc[0] = half / (half - half * 0.05f);
    }
    if (field_0x14_axis1) {
        range.object = 4;
        if (field_0x25c->UnknownMethod5(4, &range) < 0)
            goto failed;
        field_0x58c[1] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        field_0x5bc[1] = half / (half - half * 0.05f);
    }
    if (field_0x14_axis2) {
        range.object = 8;
        if (field_0x25c->UnknownMethod5(4, &range) < 0)
            goto failed;
        field_0x58c[2] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        field_0x5bc[2] = half / (half - half * 0.05f);
    }
    if (field_0x14_axis3) {
        range.object = 12;
        if (field_0x25c->UnknownMethod5(4, &range) < 0)
            goto failed;
        field_0x58c[3] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        field_0x5bc[3] = half / (half - half * 0.05f);
    }
    if (field_0x14_axis4) {
        range.object = 16;
        if (field_0x25c->UnknownMethod5(4, &range) < 0)
            goto failed;
        field_0x58c[4] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        field_0x5bc[4] = half / (half - half * 0.05f);
    }
    if (field_0x14_axis5) {
        range.object = 20;
        if (field_0x25c->UnknownMethod5(4, &range) < 0)
            goto failed;
        field_0x58c[5] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        field_0x5bc[5] = half / (half - half * 0.05f);
    }
    if (field_0x0c == 3)
        UnknownVirtualSlot5(0);
    UnknownMethod4c3a10(g_UnknownGlobal56e26c->UnknownVirtualSlot22("BufferedJoystick", 1));
    return 1;
failed:
    if (field_0x25c) {
        field_0x25c->UnknownMethod2();
        field_0x25c = 0;
    }
    return 0;
}

// 0x004c2cb0
int __stdcall PCJoystickDevice::UnknownEnumObjectsCallback(const UnknownObjectInstance* object,
                                                           void* context) {
    PCJoystickDevice* device = (PCJoystickDevice*)context;
    UnknownObjectInstance info;
    info.size = sizeof(info);
    if (device->field_0x25c->UnknownMethod14(&info, object->type, 2) >= 0) {
        if (!memcmp(&object->guidType, &GUID_XAxis, sizeof(UnknownGuid)))
            device->field_0x14_axis0 = 1;
        if (!memcmp(&object->guidType, &GUID_YAxis, sizeof(UnknownGuid)))
            device->field_0x14_axis1 = 1;
        if (!memcmp(&object->guidType, &GUID_ZAxis, sizeof(UnknownGuid)))
            device->field_0x14_axis2 = 1;
        if (!memcmp(&object->guidType, &GUID_RxAxis, sizeof(UnknownGuid)))
            device->field_0x14_axis3 = 1;
        if (!memcmp(&object->guidType, &GUID_RyAxis, sizeof(UnknownGuid)))
            device->field_0x14_axis4 = 1;
        if (!memcmp(&object->guidType, &GUID_RzAxis, sizeof(UnknownGuid)))
            device->field_0x14_axis5 = 1;
    }
    return 1;
}

// 0x004c2d90: like KeyboardDevice 0x0048a0c0, with the axis below 16384
// stepping the bindings down and above 49152 stepping them up.
void PCJoystickDevice::UnknownMethod4c2d90(int axis, int value) {
    for (int i = 0; i < field_0x4fc[axis].m_count; i++) {
        UnknownControlBinding* binding = field_0x4fc[axis].Get(i);
        binding->field_0x18 += g_UnknownGlobal56e26c->field_0x2f0;
        binding->field_0x14 += g_UnknownGlobal56e26c->field_0x2f0;
        if (field_0x4e4[axis] < 16384.0f) {
            if (binding->field_0x14 - g_UnknownGlobal56e26c->field_0x2f0 > binding->field_0x20) {
                binding->UnknownFunction43cd10();
                binding->field_0x14 = 0;
            }
            while (binding->field_0x14 > binding->field_0x20) {
                binding->UnknownFunction43cd90(-binding->field_0x1c);
                binding->field_0x14 -= binding->field_0x20;
            }
        } else if (field_0x4e4[axis] > 49152.0f) {
            if (binding->field_0x18 - g_UnknownGlobal56e26c->field_0x2f0 > binding->field_0x20) {
                binding->UnknownFunction43cd10();
                binding->field_0x18 = 0;
            }
            while (binding->field_0x18 > binding->field_0x20) {
                binding->UnknownFunction43cd90(binding->field_0x1c);
                binding->field_0x18 -= binding->field_0x20;
            }
        } else {
            binding->UnknownFunction43cd10();
        }
    }
}

// 0x004c2ef0: button n is at state offset 0x30 + n (as DIJOFS_BUTTON).
int PCJoystickDevice::UnknownFunction4c2ef0(int offset) {
    switch (offset) {
    case 0x30:
        return 0;
    case 0x31:
        return 1;
    case 0x32:
        return 2;
    case 0x33:
        return 3;
    case 0x34:
        return 4;
    case 0x35:
        return 5;
    case 0x36:
        return 6;
    case 0x37:
        return 7;
    case 0x38:
        return 8;
    case 0x39:
        return 9;
    case 0x3a:
        return 10;
    case 0x3b:
        return 11;
    case 0x3c:
        return 12;
    case 0x3d:
        return 13;
    case 0x3e:
        return 14;
    case 0x3f:
        return 15;
    case 0x40:
        return 16;
    case 0x41:
        return 17;
    case 0x42:
        return 18;
    case 0x43:
        return 19;
    case 0x44:
        return 20;
    case 0x45:
        return 21;
    case 0x46:
        return 22;
    case 0x47:
        return 23;
    case 0x48:
        return 24;
    case 0x49:
        return 25;
    case 0x4a:
        return 26;
    case 0x4b:
        return 27;
    case 0x4c:
        return 28;
    case 0x4d:
        return 29;
    case 0x4e:
        return 30;
    case 0x4f:
        return 31;
    }
    return -1;
}

// Two-value helpers; as inline functions they keep their operands on the
// FPU stack, as retail does.
static inline float AxisMax(float a, float b) {
    return a > b ? a : b;
}

static inline float AxisMin(float a, float b) {
    return a < b ? a : b;
}

// 0x004c3090
int PCJoystickDevice::UnknownFunction4c3090(int axis, int value) {
    float scaled = (value - 32768) * field_0x5bc[axis] + 32768.0f;
    return (int)AxisMin(AxisMax(scaled, 0.0f), 65535.0f);
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

// 0x004c3c40: creates effect `effect` as a constant force on the X and Y
// axes (gain 9000, no trigger).
int PCJoystickDevice::UnknownVirtualSlot6(int effect, long magnitude, unsigned long duration) {
    if (field_0x25c && field_0x0c == 3 && effect < 5) {
        unsigned long axes[2];
        long direction[2];
        UnknownEffectParams params;
        long force = magnitude;
        direction[0] = 0;
        direction[1] = 0;
        axes[0] = 0;
        axes[1] = 4;
        params.size = sizeof(params);
        params.flags = 0x22;
        params.duration = duration;
        params.samplePeriod = 0;
        params.gain = 9000;
        params.triggerButton = (unsigned long)-1;
        params.triggerRepeatInterval = 0;
        params.axisCount = 2;
        params.axes = axes;
        params.direction = direction;
        params.envelope = 0;
        params.typeSpecificSize = sizeof(force);
        params.typeSpecific = &force;
        return field_0x25c->UnknownMethod18(GUID_ConstantForce, &params, &field_0x578[effect], 0) >= 0;
    }
    return 0;
}

// 0x004c3d20: sets the direction (slot 8), then the magnitude.
int PCJoystickDevice::UnknownVirtualSlot7(int effect, long* direction, long magnitude) {
    if (field_0x25c && field_0x0c == 3 && effect < 5) {
        long force = magnitude;
        UnknownVirtualSlot8(effect, direction);
        UnknownEffectParams params = { sizeof(params) };
        params.typeSpecificSize = sizeof(force);
        params.typeSpecific = &force;
        return field_0x578[effect]->UnknownMethod6(&params, 0x100) >= 0;
    }
    return 0;
}

// 0x004c3db0: sets a two-value direction.
int PCJoystickDevice::UnknownVirtualSlot8(int effect, long* direction) {
    if (field_0x25c && field_0x0c == 3 && effect < 5) {
        long value[2];
        value[0] = direction[0];
        value[1] = direction[1];
        UnknownEffectParams params = { sizeof(params) };
        params.flags = 0x22;
        params.axisCount = 2;
        params.axes = 0;
        params.direction = value;
        return field_0x578[effect]->UnknownMethod6(&params, 0x40) >= 0;
    }
    return 0;
}

// 0x004c3e40: creates a constant force with an envelope; `button` (or -1)
// triggers it.
int PCJoystickDevice::UnknownVirtualSlot9(int effect, unsigned long duration, long direction,
                                          long magnitude, unsigned long attackTime,
                                          unsigned long attackLevel, unsigned long fadeTime,
                                          unsigned long fadeLevel, int button) {
    if (field_0x25c && field_0x0c == 3 && effect < 5) {
        unsigned long axes[2];
        long directions[2];
        UnknownEnvelope envelope;
        UnknownEffectParams params;
        long force = magnitude;
        envelope.size = sizeof(envelope);
        envelope.attackTime = attackTime;
        envelope.attackLevel = attackLevel;
        envelope.fadeTime = fadeTime;
        envelope.fadeLevel = fadeLevel;
        axes[0] = 0;
        axes[1] = 4;
        directions[0] = direction;
        directions[1] = 0;
        params.size = sizeof(params);
        params.flags = 0x22;
        params.duration = duration;
        params.samplePeriod = 10000;
        params.gain = 10000;
        params.triggerButton = button == -1 ? -1 : button + 0x30;
        params.triggerRepeatInterval = 0;
        params.axisCount = 2;
        params.axes = axes;
        params.direction = directions;
        params.envelope = &envelope;
        params.typeSpecificSize = sizeof(force);
        params.typeSpecific = &force;
        return field_0x25c->UnknownMethod18(GUID_ConstantForce, &params, &field_0x578[effect], 0) >= 0;
    }
    return 0;
}

// 0x004c3f70: creates a square wave with an envelope; `button` (or -1)
// triggers it.
int PCJoystickDevice::UnknownVirtualSlot10(int effect, unsigned long duration, unsigned long period,
                                           unsigned long magnitude, unsigned long attackTime,
                                           unsigned long attackLevel, unsigned long fadeTime,
                                           unsigned long fadeLevel, int button) {
    if (field_0x25c && field_0x0c == 3 && effect < 5) {
        unsigned long axes[2];
        long direction[2];
        UnknownPeriodic wave;
        UnknownEnvelope envelope;
        UnknownEffectParams params;
        wave.magnitude = magnitude;
        wave.offset = 0;
        wave.phase = 0;
        wave.period = period;
        envelope.size = sizeof(envelope);
        envelope.attackTime = attackTime;
        envelope.attackLevel = attackLevel;
        envelope.fadeTime = fadeTime;
        envelope.fadeLevel = fadeLevel;
        axes[0] = 0;
        axes[1] = 4;
        direction[0] = 0;
        direction[1] = 0;
        params.size = sizeof(params);
        params.flags = 0x22;
        params.duration = duration;
        params.samplePeriod = 0;
        params.gain = 10000;
        params.triggerButton = button == -1 ? -1 : button + 0x30;
        params.triggerRepeatInterval = 0;
        params.axisCount = 2;
        params.axes = axes;
        params.direction = direction;
        params.envelope = &envelope;
        params.typeSpecificSize = sizeof(wave);
        params.typeSpecific = &wave;
        return field_0x25c->UnknownMethod18(GUID_Square, &params, &field_0x578[effect], 0) >= 0;
    }
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

// Inline (no retail body of its own): a failed poll is retried by
// re-acquiring a lost (0x8007001e) or unacquired (0x8007000c) device; any
// other failure is reported. Nonzero when reading can go ahead.
inline int PCJoystickDevice::CheckPollResult(long result) {
    if (result < 0) {
        if (result == (long)0x8007001e || result == (long)0x8007000c) {
            if (field_0x25c->UnknownMethod7() < 0) {
                UnknownReportError(result, __FILE__, 540);
                return 0;
            }
        } else {
            UnknownReportError(result, __FILE__, 544);
            return 0;
        }
    }
    return 1;
}

// 0x004c3960: polls the device (see CheckPollResult), then reads it with
// the buffered (bit 0 of +0x5d4) or immediate reader. Gamepads with bit 1 also run
// 0x004c2d90 for each of the six lists.
int PCJoystickDevice::UnknownVirtualSlot20(int value) {
    if (!field_0x25c)
        return 0;
    if (!CheckPollResult(field_0x25c->UnknownMethod25()))
        return 0;
    if (field_0x5d4_bit0)
        UnknownMethod4c3100(value);
    else
        UnknownMethod4c3790(value);
    if (DEVICE_SUBTYPE(field_0x18.deviceType) == 4 && field_0x5d4_bit1) {
        for (int i = 0; i < 6; i++)
            UnknownMethod4c2d90(i, value);
    }
    return 1;
}

// 0x004c3a10: switches buffered input on (16 entries) or off. Property 1 is
// consistent with DIPROP_BUFFERSIZE; the device is unacquired around it.
int PCJoystickDevice::UnknownMethod4c3a10(int buffered) {
    if (!field_0x25c)
        return 0;
    field_0x25c->UnknownMethod8();
    UnknownInputProperty property;
    memset(&property, 0, sizeof(property));
    property.size = sizeof(property);
    property.headerSize = 0x10;
    property.object = 0;
    property.how = 0;
    property.data = buffered ? 16 : 0;
    long result = field_0x25c->UnknownMethod6(1, &property);
    if (result < 0) {
        UnknownReportError(result, __FILE__, 590);
        goto failed;
    }
    field_0x5d4_bit0 = buffered;
    if (field_0x25c->UnknownMethod7() < 0)
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004c3ae0
void PCJoystickDevice::UnknownMethod4c3ae0(unsigned char value) {
    field_0x5d5 = value;
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

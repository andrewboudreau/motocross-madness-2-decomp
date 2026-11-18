#include <math.h>
#include <stdio.h>
#include <string.h>

#include "PCJoystickDevice.h"

#include "TrackGame.h"

// 0x004c2770: repeats JoystickDevice's initialisation with PCJoystickDevice's
// own fields interleaved.
PCJoystickDevice::PCJoystickDevice(int index) : JoystickDevice(index) {
    joystickIndex = index;
    for (int k = 0; k < 5; k++)
        effects[k] = 0;
    for (int i = 0; i < 6; i++) {
        axisMaximums[i] = 0;
        axisValues[i] = 0;
        previousRawAxes[i] = 0;
        axisBindings[i].Init(1, 1);
    }
    for (int j = 0; j < 32; j++)
        buttonStates[j].state = 0;
    for (int n = 0; n < 4; n++)
        povValues[n] = -1;
    povCount = 0;
    bufferedInput = 1;
    axisRepeatEnabled = 1;
    joystickFilter = 0;
    directionFlipped = g_TrackGame->GetRegistryFlag("JoyDirectionFlipped", 0);
}

// 0x004c28d0: releases the effects and resets the device (calls bind
// statically in a destructor).
PCJoystickDevice::~PCJoystickDevice() {
    ReleaseEffects();
    ResetForceFeedback();
}

// 0x004c2930: creates the device, takes its description and capabilities
// (force feedback makes it type 3), enumerates the axes and reads each axis
// range, with a 5% dead zone. On failure the device is released.
int PCJoystickDevice::OpenDevice(const DIDeviceInstance* instance) {
    DIPropRange range;
    DIDevCaps caps;
    int half;
    const Guid& guid = instance->instanceGuid;
    if (g_TrackGame->controlInterface->directInput->CreateDeviceEx(
            guid, IID_IDirectInputDevice7A, &device, 0) < 0)
        goto failed;
    deviceInfo = *instance;
    deviceInfo.size = sizeof(deviceInfo);
    device->GetDeviceInfo(&deviceInfo);
    DetectSubtype();
    if (g_TrackGame->controlInterface->directInput->GetDeviceStatus(guid) < 0)
        goto failed;
    if (device->SetDataFormat(&c_dfDIJoystick) < 0)
        goto failed;
    if (device->SetCooperativeLevel(g_TrackGame->windowHandle, DISCL_EXCLUSIVE | DISCL_FOREGROUND) < 0)
        goto failed;
    caps.size = sizeof(caps);
    if (device->GetCapabilities(&caps) < 0)
        goto failed;
    buttonCount = caps.buttons;
    axisCount = caps.axes;
    if (caps.flags & 0x100)
        deviceKind = 3;
    povCount = caps.povs > 1 ? caps.povs : 1;
    device->EnumObjects(EnumAxisObjectsCallback, this, DIDFT_AXIS);
    range.size = sizeof(range);
    range.headerSize = 0x10;
    range.how = DIPH_BYOFFSET;
    if (hasXAxis) {
        range.object = 0;
        if (device->GetProperty(DIPROP_RANGE, &range) < 0)
            goto failed;
        axisMaximums[0] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        deadZoneScales[0] = half / (half - half * 0.05f);
    }
    if (hasYAxis) {
        range.object = 4;
        if (device->GetProperty(DIPROP_RANGE, &range) < 0)
            goto failed;
        axisMaximums[1] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        deadZoneScales[1] = half / (half - half * 0.05f);
    }
    if (hasZAxis) {
        range.object = 8;
        if (device->GetProperty(DIPROP_RANGE, &range) < 0)
            goto failed;
        axisMaximums[2] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        deadZoneScales[2] = half / (half - half * 0.05f);
    }
    if (hasRxAxis) {
        range.object = 12;
        if (device->GetProperty(DIPROP_RANGE, &range) < 0)
            goto failed;
        axisMaximums[3] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        deadZoneScales[3] = half / (half - half * 0.05f);
    }
    if (hasRyAxis) {
        range.object = 16;
        if (device->GetProperty(DIPROP_RANGE, &range) < 0)
            goto failed;
        axisMaximums[4] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        deadZoneScales[4] = half / (half - half * 0.05f);
    }
    if (hasRzAxis) {
        range.object = 20;
        if (device->GetProperty(DIPROP_RANGE, &range) < 0)
            goto failed;
        axisMaximums[5] = (float)range.maximum;
        half = (range.maximum + 1) / 2;
        deadZoneScales[5] = half / (half - half * 0.05f);
    }
    if (deviceKind == 3)
        SetAutoCenter(0);
    SetBufferedInput(g_TrackGame->GetRegistryFlag("BufferedJoystick", 1));
    return 1;
failed:
    if (device) {
        device->Release();
        device = 0;
    }
    return 0;
}

// 0x004c2cb0
int __stdcall PCJoystickDevice::EnumAxisObjectsCallback(const DIDeviceObjectInstance* object,
    void* context) {
    PCJoystickDevice* device = (PCJoystickDevice*)context;
    DIDeviceObjectInstance info;
    info.size = sizeof(info);
    if (device->device->GetObjectInfo(&info, object->type, DIPH_BYID) >= 0) {
        if (!memcmp(&object->guidType, &GUID_XAxis, sizeof(Guid)))
            device->hasXAxis = 1;
        if (!memcmp(&object->guidType, &GUID_YAxis, sizeof(Guid)))
            device->hasYAxis = 1;
        if (!memcmp(&object->guidType, &GUID_ZAxis, sizeof(Guid)))
            device->hasZAxis = 1;
        if (!memcmp(&object->guidType, &GUID_RxAxis, sizeof(Guid)))
            device->hasRxAxis = 1;
        if (!memcmp(&object->guidType, &GUID_RyAxis, sizeof(Guid)))
            device->hasRyAxis = 1;
        if (!memcmp(&object->guidType, &GUID_RzAxis, sizeof(Guid)))
            device->hasRzAxis = 1;
    }
    return 1;
}

// 0x004c2d90: like KeyboardDevice 0x0048a0c0, with the axis below 16384
// stepping the bindings down and above 49152 stepping them up.
void PCJoystickDevice::UpdateAxisRepeat(int axis, int value) {
    for (int i = 0; i < axisBindings[axis].m_count; i++) {
        ControlBinding* binding = axisBindings[axis].Get(i);
        binding->secondKeyTimer += g_TrackGame->frameTime;
        binding->firstKeyTimer += g_TrackGame->frameTime;
        if (axisValues[axis] < 16384.0f) {
            if (binding->firstKeyTimer - g_TrackGame->frameTime > binding->repeatInterval) {
                binding->Recenter();
                binding->firstKeyTimer = 0;
            }
            while (binding->firstKeyTimer > binding->repeatInterval) {
                binding->Adjust(-binding->repeatStep);
                binding->firstKeyTimer -= binding->repeatInterval;
            }
        } else if (axisValues[axis] > 49152.0f) {
            if (binding->secondKeyTimer - g_TrackGame->frameTime > binding->repeatInterval) {
                binding->Recenter();
                binding->secondKeyTimer = 0;
            }
            while (binding->secondKeyTimer > binding->repeatInterval) {
                binding->Adjust(binding->repeatStep);
                binding->secondKeyTimer -= binding->repeatInterval;
            }
        } else {
            binding->Recenter();
        }
    }
}

// 0x004c2ef0: button n is at state offset 0x30 + n (as DIJOFS_BUTTON).
int PCJoystickDevice::ButtonIndexFromOffset(int offset) {
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
int PCJoystickDevice::ApplyAxisDeadZone(int axis, int value) {
    float scaled = (value - 32768) * deadZoneScales[axis] + 32768.0f;
    return (int)AxisMin(AxisMax(scaled, 0.0f), 65535.0f);
}

static inline int AxisAbs(int value) {
    return value < 0 ? -value : value;
}

inline int PCJoystickDevice::FilterAxisSpike(int axis, unsigned long value) {
    if (joystickFilter) {
        int current = (int)axisValues[axis];
        if (AxisAbs(current - (int)value) > 10000 && (int)previousRawAxes[axis] == current)
            return current;
    }
    return value;
}

// 0x004c3100: reads up to 16 buffered events (re-acquiring a lost device).
// Buttons are queued on the ControlInterface; gamepads store raw axes,
// other joysticks filter spikes, apply the X/Y dead zone and feed the
// bindings; POV values are kept.
// Not exact (128 of 1665 bytes in place, 1641 compiled; docs/JOYSTICK_BUFFERED_STATE.md).
int PCJoystickDevice::ReadBufferedState(int) {
    DIDeviceObjectData events[16];
    long count;
    int i;
    unsigned long value;
    float axis;
    if (!device)
        return 0;
    count = 16;
    long result = device->GetDeviceData(sizeof(DIDeviceObjectData), events, &count, 0);
    if (result == (long)0x8007001e || result == (long)0x8007000c) {
        if (device->Acquire() < 0) {
            ReportDirectInputError(result, __FILE__, 317);
            return 0;
        }
        result = device->GetDeviceData(sizeof(DIDeviceObjectData), events, &count, 0);
    }
    if (result < 0)
        return 0;
    for (i = 0; i < count; i++) {
        int button = ButtonIndexFromOffset(events[i].offset);
        if (button != -1) {
            if (events[i].data & 0x80) {
                buttonStates[button].state = 1;
                buttonStates[button].previousPressTime = buttonStates[button].pressTime;
                buttonStates[button].pressTime = events[i].timeStamp;
                g_TrackGame->controlInterface->QueueEvent(button, 2, 1, joystickIndex);
            } else {
                buttonStates[button].state = 0;
                buttonStates[button].previousReleaseTime = buttonStates[button].releaseTime;
                buttonStates[button].releaseTime = events[i].timeStamp;
                g_TrackGame->controlInterface->QueueEvent(button, 2, 0, joystickIndex);
            }
            continue;
        }
        if (DEVICE_SUBTYPE(deviceInfo.deviceType) == 4) {
            switch (events[i].offset) {
            case 0:
                axisValues[0] = (float)events[i].data;
                break;
            case 4:
                axisValues[1] = (float)events[i].data;
                break;
            case 8:
                axisValues[2] = (float)events[i].data;
                break;
            case 12:
                axisValues[3] = (float)events[i].data;
                break;
            case 16:
                axisValues[4] = (float)events[i].data;
                break;
            case 20:
                axisValues[5] = (float)events[i].data;
                break;
            }
        } else {
            switch (events[i].offset) {
            case 0:
                value = ApplyAxisDeadZone(0, FilterAxisSpike(0, events[i].data));
                axis = axisValues[0] = (float)value;
                previousRawAxes[0] = (float)events[i].data;
                FeedAxis(0, axis, axisMaximums[0]);
                break;
            case 4:
                value = ApplyAxisDeadZone(1, FilterAxisSpike(1, events[i].data));
                axis = axisValues[1] = (float)value;
                previousRawAxes[1] = (float)events[i].data;
                if (directionFlipped)
                    axis = axisMaximums[1] - axis;
                FeedAxis(1, axis, axisMaximums[1]);
                break;
            case 8:
                value = FilterAxisSpike(2, events[i].data);
                axis = axisValues[2] = (float)value;
                previousRawAxes[2] = (float)events[i].data;
                FeedAxis(2, axisMaximums[2] - axis, axisMaximums[2]);
                break;
            case 12:
                value = FilterAxisSpike(3, events[i].data);
                axis = axisValues[3] = (float)value;
                previousRawAxes[3] = (float)events[i].data;
                FeedAxis(3, axis, axisMaximums[3]);
                break;
            case 16:
                value = FilterAxisSpike(4, events[i].data);
                axis = axisValues[4] = (float)value;
                previousRawAxes[4] = (float)events[i].data;
                FeedAxis(4, axis, axisMaximums[4]);
                break;
            case 20:
                value = FilterAxisSpike(5, events[i].data);
                axis = axisValues[5] = (float)value;
                previousRawAxes[5] = (float)events[i].data;
                FeedAxis(5, axis, axisMaximums[5]);
                break;
            }
        }
        switch (events[i].offset) {
        case 0x20:
            povValues[0] = events[i].data;
            break;
        case 0x24:
            povValues[1] = events[i].data;
            break;
        case 0x28:
            povValues[2] = events[i].data;
            break;
        case 0x2c:
            povValues[3] = events[i].data;
            break;
        }
    }
    return 1;
}

// 0x004c3790: reads the immediate state. Non-gamepads feed the six axes to
// their bindings (axis 1 flipped by the JoyDirectionFlipped bit, axis 2
// always); button changes are queued as events, and the POV values kept.
// Not exact (453 of 454 bytes: retail pushes the zero register for the
// release arm's argument where this pushes eax; docs/JOYSTICK_IMMEDIATE_STATE.md
// lists the forms tried).
int PCJoystickDevice::ReadImmediateState(int value) {
    DIJoyState state;
    if (!device)
        return 0;
    if (device->GetDeviceState(sizeof(state), &state) < 0)
        return 0;
    if (DEVICE_SUBTYPE(deviceInfo.deviceType) != 4) {
        float axis;
        axis = (float)state.axes[0];
        axisValues[0] = axis;
        FeedAxis(0, axis, axisMaximums[0]);
        axis = axisValues[1] = (float)state.axes[1];
        if (directionFlipped)
            axis = axisMaximums[1] - axis;
        FeedAxis(1, axis, axisMaximums[1]);
        axis = (float)state.axes[2];
        axisValues[2] = axis;
        FeedAxis(2, axisMaximums[2] - axis, axisMaximums[2]);
        axis = (float)state.axes[3];
        axisValues[3] = axis;
        FeedAxis(3, axis, axisMaximums[3]);
        axis = (float)state.axes[4];
        axisValues[4] = axis;
        FeedAxis(4, axis, axisMaximums[4]);
        axis = (float)state.axes[5];
        axisValues[5] = axis;
        FeedAxis(5, axis, axisMaximums[5]);
    }
    for (int i = 0; i < 32; i++) {
        int previous = buttonStates[i].state;
        int pressed = buttonStates[i].state = state.buttons[i] >> 7;
        if (pressed != previous) {
            if (pressed == 0) {
                buttonStates[i].previousReleaseTime = 0;
                buttonStates[i].releaseTime = 0;
                g_TrackGame->controlInterface->QueueEvent(i, 2, pressed, joystickIndex);
            } else {
                buttonStates[i].previousPressTime = 0;
                buttonStates[i].pressTime = 0;
                g_TrackGame->controlInterface->QueueEvent(i, 2, 1, joystickIndex);
            }
        }
    }
    for (int j = 0; j < 4; j++)
        povValues[j] = state.pov[j];
    return 1;
}

// Inline (no retail body of its own): a failed poll is retried by
// re-acquiring a lost (0x8007001e) or unacquired (0x8007000c) device; any
// other failure is reported. Nonzero when reading can go ahead.
inline int PCJoystickDevice::CheckPollResult(long result) {
    if (result < 0) {
        if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
            if (device->Acquire() < 0) {
                ReportDirectInputError(result, __FILE__, 540);
                return 0;
            }
        } else {
            ReportDirectInputError(result, __FILE__, 544);
            return 0;
        }
    }
    return 1;
}

// 0x004c3960: polls the device (see CheckPollResult), then reads it with
// the buffered (bit 0 of +0x5d4) or immediate reader. Gamepads with bit 1 also run
// 0x004c2d90 for each of the six lists.
int PCJoystickDevice::Update(int value) {
    if (!device)
        return 0;
    if (!CheckPollResult(device->Poll()))
        return 0;
    if (bufferedInput)
        ReadBufferedState(value);
    else
        ReadImmediateState(value);
    if (DEVICE_SUBTYPE(deviceInfo.deviceType) == 4 && axisRepeatEnabled) {
        for (int i = 0; i < 6; i++)
            UpdateAxisRepeat(i, value);
    }
    return 1;
}

// 0x004c3a10: switches buffered input on (16 entries) or off. Property 1 is
// consistent with DIPROP_BUFFERSIZE; the device is unacquired around it.
int PCJoystickDevice::SetBufferedInput(int buffered) {
    if (!device)
        return 0;
    device->Unacquire();
    DIPropDword property;
    memset(&property, 0, sizeof(property));
    property.size = sizeof(property);
    property.headerSize = 0x10;
    property.object = 0;
    property.how = DIPH_DEVICE;
    property.data = buffered ? 16 : 0;
    long result = device->SetProperty(DIPROP_BUFFERSIZE, &property);
    if (result < 0) {
        ReportDirectInputError(result, __FILE__, 590);
        goto failed;
    }
    bufferedInput = buffered;
    if (device->Acquire() < 0)
        goto failed;
    return 1;
failed:
    return 0;
}

// 0x004c3ae0
void PCJoystickDevice::SetJoystickFilter(unsigned char value) {
    joystickFilter = value;
}

// 0x004c3af0: joystick subtypes 4-7 are recorded in +0x10.
void PCJoystickDevice::DetectSubtype() {
    int subtype = DEVICE_SUBTYPE(deviceInfo.deviceType);
    if (subtype == 4)
        deviceSubtype = subtype;
    else if (subtype == 5)
        deviceSubtype = subtype;
    else if (subtype == 6)
        deviceSubtype = subtype;
    else if (subtype == 7)
        deviceSubtype = subtype;
}

// 0x004c3b20: POV index in hundredths of a degree, as radians; -1 when
// centred (-1 or 0xffff) or out of range.
int PCJoystickDevice::GetPovAngle(int index, float* angle) {
    if (povCount > index) {             // +0x5e8: POV count
        unsigned long value = povValues[index];
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
int PCJoystickDevice::GetPovDirection(int index, float* x, float* y) {
    float angle;
    if (GetPovAngle(index, &angle)) {
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
int PCJoystickDevice::SetAutoCenter(int enable) {
    if (deviceKind == 3)
        return SetDwordProperty(DIPROP_AUTOCENTER, 0, DIPH_DEVICE, enable != 0);
    return 0;
}

// 0x004c3c40: creates effect `effect` as a constant force on the X and Y
// axes (gain 9000, no trigger).
int PCJoystickDevice::CreateConstantForce(int effect, long magnitude, unsigned long duration) {
    if (device && deviceKind == 3 && effect < 5) {
        unsigned long axes[2];
        long direction[2];
        DIEffect params;
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
        return device->CreateEffect(GUID_ConstantForce, &params, &effects[effect], 0) >= 0;
    }
    return 0;
}

// 0x004c3d20: sets the direction (slot 8), then the magnitude.
int PCJoystickDevice::SetConstantForce(int effect, long* direction, long magnitude) {
    if (device && deviceKind == 3 && effect < 5) {
        long force = magnitude;
        SetDirection(effect, direction);
        DIEffect params = { sizeof(params) };
        params.typeSpecificSize = sizeof(force);
        params.typeSpecific = &force;
        return effects[effect]->SetParameters(&params, 0x100) >= 0;
    }
    return 0;
}

// 0x004c3db0: sets a two-value direction.
int PCJoystickDevice::SetDirection(int effect, long* direction) {
    if (device && deviceKind == 3 && effect < 5) {
        long value[2];
        value[0] = direction[0];
        value[1] = direction[1];
        DIEffect params = { sizeof(params) };
        params.flags = 0x22;
        params.axisCount = 2;
        params.axes = 0;
        params.direction = value;
        return effects[effect]->SetParameters(&params, 0x40) >= 0;
    }
    return 0;
}

// 0x004c3e40: creates a constant force with an envelope; `button` (or -1)
// triggers it.
int PCJoystickDevice::CreateEnvelopedForce(int effect, unsigned long duration, long direction,
                                           long magnitude, unsigned long attackTime,
                                           unsigned long attackLevel, unsigned long fadeTime,
                                           unsigned long fadeLevel, int button) {
    if (device && deviceKind == 3 && effect < 5) {
        unsigned long axes[2];
        long directions[2];
        DIEnvelope envelope;
        DIEffect params;
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
        return device->CreateEffect(GUID_ConstantForce, &params, &effects[effect], 0) >= 0;
    }
    return 0;
}

// 0x004c3f70: creates a square wave with an envelope; `button` (or -1)
// triggers it.
int PCJoystickDevice::CreateSquareWave(int effect, unsigned long duration, unsigned long period,
                                       unsigned long magnitude, unsigned long attackTime,
                                       unsigned long attackLevel, unsigned long fadeTime,
                                       unsigned long fadeLevel, int button) {
    if (device && deviceKind == 3 && effect < 5) {
        unsigned long axes[2];
        long direction[2];
        DIPeriodic wave;
        DIEnvelope envelope;
        DIEffect params;
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
        return device->CreateEffect(GUID_Square, &params, &effects[effect], 0) >= 0;
    }
    return 0;
}

// 0x004c40b0: stops effect `effect`.
int PCJoystickDevice::StopEffect(int effect) {
    if (device && effect < 5 && effects[effect] && deviceKind == 3)
        return effects[effect]->Stop() >= 0;
    return 0;
}

// 0x004c40f0: starts effect `effect`.
int PCJoystickDevice::StartEffect(int effect, unsigned long iterations,
                                  unsigned long flags) {
    if (device && effect < 5 && effects[effect] && deviceKind == 3)
        return effects[effect]->Start(iterations, flags) >= 0;
    return 0;
}

// 0x004c4140: nonzero while effect `effect` reports status 1 (playing).
int PCJoystickDevice::IsEffectPlaying(int effect) {
    if (device && effect < 5 && effects[effect] && deviceKind == 3) {
        unsigned long status;
        effects[effect]->GetEffectStatus(&status);
        return status == 1;
    }
    return 0;
}

// 0x00689958: effects stored by the enumeration callback.
static int s_UnknownEffectCount;

// The callback (0x004c4260) follows EnumEffects in retail.
static int __stdcall EnumEffectsCallback(const DIEffectInfo* info, void* context);

// 0x004c4190: clears five entries, then enumerates the device's effects into
// `effects` and stores how many were found.
int PCJoystickDevice::EnumEffects(DIEffectInfo* effects, int* count) {
    if (deviceKind != 3)
        return 0;
    for (int i = 0; i < 5; i++) {
        memset(&effects[i], 0, sizeof(effects[i]));
        effects[i].size = sizeof(effects[i]);
    }
    if (!device)
        return 0;
    char text[256];
    sprintf(text, "ForceFeedback Joystick Detected\nJoystickDevice::EnumEffects ('%s')\n",
            deviceInfo.productName);
    s_UnknownEffectCount = 0;
    if (device->EnumEffects(EnumEffectsCallback, effects, 0) < 0)
        return 0;
    *count = s_UnknownEffectCount;
    return 1;
}

// 0x004c4260: the IDirectInputDevice2::EnumEffects (method 19) callback; copies each effect into the caller's
// array, stopping at 32. The formatted line is not used further.
static int __stdcall EnumEffectsCallback(const DIEffectInfo* info, void* context) {
    DIEffectInfo* effects = (DIEffectInfo*)context;
    if (!effects)
        return 0;
    effects[s_UnknownEffectCount] = *info;
    char typeName[260];
    GetForceFeedbackEffectName(effects[s_UnknownEffectCount].guid, typeName);
    char text[528];   // the frame fixes this size
    sprintf(text, "\t '%s' '%s'\n", typeName, effects[s_UnknownEffectCount].name);
    s_UnknownEffectCount++;
    return s_UnknownEffectCount < 32;
}

// 0x004c4320
void PCJoystickDevice::ReleaseEffects() {
    for (int i = 0; i < 5; i++) {
        if (effects[i]) {
            effects[i]->Release();
            effects[i] = 0;
        }
    }
}

// 0x004c4350: command 1.
int PCJoystickDevice::ResetForceFeedback() {
    if (device && deviceKind == 3)
        return SendForceFeedbackCommand(1);
    return 0;
}

// 0x004c4370
int PCJoystickDevice::SendForceFeedbackCommand(int command) {
    if (device && deviceKind == 3)
        device->SendForceFeedbackCommand(command);
    return 0;
}

// 0x004c4390: command 4 when paused, 8 when resumed.
int PCJoystickDevice::PauseForceFeedback(int paused) {
    if (deviceKind == 3) {
        if (paused)
            return SendForceFeedbackCommand(4);
        return SendForceFeedbackCommand(8);
    }
    return 1;
}

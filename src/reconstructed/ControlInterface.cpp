#include "ControlInterface.h"
#include "InputDevice.h"
#include "JoystickDevice.h"
#include "KeyboardDevice.h"
#include "MouseDevice.h"
#include "TrackGame.h"

// Two-value helpers; inline functions keep their operands as spilled
// arguments, as retail does.
static inline float BindingMax(float a, float b) {
    return a > b ? a : b;
}

static inline float BindingMin(float a, float b) {
    return a < b ? a : b;
}

// 0x0043caa0
int TestInputEvent(int control, int kind, const UnknownControlEvent* event, int modifier) {
    if (event->control == control && event->kind == kind &&
        (!g_TrackGame->controlInterface->keyboard ||
         g_TrackGame->controlInterface->keyboard->ModifierHolds(modifier)))
        return 1;
    return 0;
}

// 0x0043cae0
UnknownControlMapping::UnknownControlMapping() {
    Clear();
}

// 0x0043caf0
void UnknownControlMapping::Clear() {
    for (int i = 0; i < 50; i++) {
        keys[i] = -1;
        for (int j = 0; j < 8; j++)
            joystickButtons[j][i] = -1;
        mouseButtons[i] = -1;
    }
}

// 0x0043cb30
void UnknownControlMapping::SetKey(int control, int key) {
    if (control >= 0 && control < 50)
        keys[control] = key;
}

// 0x0043cb50
void UnknownControlMapping::SetJoystickButton(int control, int button, int device) {
    if (control >= 0 && control < 50 && device < 8)
        joystickButtons[device][control] = button;
}

// 0x0043cb80
void UnknownControlMapping::SetMouseButton(int control, int button) {
    if (control >= 0 && control < 50)
        mouseButtons[control] = button;
}

// 0x0043cba0
void UnknownControlMapping::GetKey(int control, int* result) {
    if (control >= 0 && control < 50)
        *result = keys[control];
    else
        *result = -1;
}

// 0x0043cbd0
void UnknownControlMapping::GetJoystickButton(int control, int* result, int device) {
    if (control >= 0 && control < 50 && device < 8)
        *result = joystickButtons[device][control];
    else
        *result = -1;
}

// 0x0043cc10
void UnknownControlMapping::GetMouseButton(int control, int* result) {
    if (control >= 0 && control < 50)
        *result = mouseButtons[control];
    else
        *result = -1;
}

// 0x0043cc40
UnknownControlBinding::UnknownControlBinding() {
    Reset();
}

// 0x0043cc50
UnknownControlBinding::UnknownControlBinding(float minimum, float maximum, float center,
                                             int id, int axis, float deadZone) {
    Reset();
    minValue = minimum;
    maxValue = maximum;
    currentValue = center;
    centerValue = center;
    bindingId = id;
    axisIndex = axis;
    deadZoneHigh = center + deadZone;
    deadZoneLow = center - deadZone;
}

// 0x0043cca0
void UnknownControlBinding::Reset() {
    currentValue = 0;
    centerValue = 0;
    minValue = 0;
    maxValue = 0;
    deadZoneHigh = 0;
    deadZoneLow = 0;
    owner = 0;
    bindingId = 0;
    axisIndex = -1;
    firstKey = -1;
    secondKey = -1;
    firstKeyTimer = 0;
    secondKeyTimer = 0;
    repeatStep = 0;
    repeatInterval = 0;
}

// 0x0043cce0
void UnknownControlBinding::SetKeys(int a, int b) {
    firstKey = a;
    secondKey = b;
}

// 0x0043cd00
UnknownControlBinding::~UnknownControlBinding() {
    Unbind();
}

// 0x0043cd10
void UnknownControlBinding::Recenter() {
    currentValue = centerValue;
}

// 0x0043cd20: maps value/range onto [min, max]; inside the dead zone the
// value snaps to the centre.
void UnknownControlBinding::SetFromAxis(float value, float range) {
    float mapped = (maxValue - minValue) * (value / range) + minValue;
    if (mapped < deadZoneHigh && mapped > deadZoneLow)
        mapped = centerValue;
    currentValue = BindingMin(BindingMax(mapped, minValue), maxValue);
}

// 0x0043cd90
void UnknownControlBinding::Adjust(float delta) {
    currentValue += delta;
    currentValue = BindingMin(BindingMax(currentValue, minValue), maxValue);
}

// 0x0043cde0: asks the owning device to drop bindings with this id.
void UnknownControlBinding::Unbind() {
    if (g_TrackGame->controlInterface && owner)
        owner->RemoveBindings(bindingId);
}

// 0x0043ce00
ControlInterface::ControlInterface() {
    joystickCount = 0;
    activeJoystickIndex = 0;
    activeJoystick = 0;
    for (int i = 0; i < 8; i++)
        joysticks[i] = 0;
    mouse = 0;
    keyboard = 0;
    mapping = 0;
    queuedEventCount = 0;
}

// 0x0043ce60
ControlInterface::~ControlInterface() {}

// 0x0043ce70
int ControlInterface::SetMapping(UnknownControlMapping* newMapping) {
    if (!newMapping)
        return 0;
    mapping = newMapping;
    return 1;
}

// 0x0043cea0
void ControlInterface::QueueEvent(int control, int kind, int pressed, int device) {
    events[queuedEventCount].control = control;
    events[queuedEventCount].device = device;
    events[queuedEventCount].kind = kind;
    events[queuedEventCount].pressed = pressed;
    events[queuedEventCount].modifiers = 0x3f;
    queuedEventCount++;
}

// 0x0043cf00: has the keyboard, mouse and joysticks read their input, then
// passes each queued event, stamped with the keyboard's modifier state, to
// the global object (slot 13 for releases, 14 for presses) with the
// device's entry for that control.
int ControlInterface::DispatchInput(int value) {
    queuedEventCount = 0;
    if (keyboard)
        keyboard->Update(value);
    if (mouse)
        mouse->Update(value);
    for (int i = 0; i < 8; i++) {
        if (joysticks[i])
            joysticks[i]->Update(value);
    }
    for (int j = 0; j < queuedEventCount; j++) {
        UnknownControlEvent* event = &events[j];
        // Retail stores the null keyboard pointer itself as the modifier
        // state: the pointer is tested as the int that then receives the
        // state. `keyboard ? keyboard->modifierState : 0` and every other
        // zero-constant form emit a separate `xor eax, eax` instead.
        int modifiers = (int)keyboard;
        if (modifiers)
            modifiers = keyboard->modifierState;
        event->modifiers = modifiers;
        switch (event->kind) {
        case 0:
            if (event->pressed == 0)
                g_TrackGame->OnControlRelease(
                    event, &keyboard->keyStates[event->control]);
            if (event->pressed == 1)
                g_TrackGame->OnControlPress(
                    event, &keyboard->keyStates[event->control]);
            break;
        case 1:
            if (event->pressed == 0)
                g_TrackGame->OnControlRelease(
                    event, &mouse->buttonStates[event->control]);
            if (event->pressed == 1)
                g_TrackGame->OnControlPress(
                    event, &mouse->buttonStates[event->control]);
            break;
        case 2:
        case 3:
            if (event->pressed == 0)
                g_TrackGame->OnControlRelease(
                    event, &joysticks[event->device]->buttonStates[event->control]);
            if (event->pressed == 1)
                g_TrackGame->OnControlPress(
                    event, &joysticks[event->device]->buttonStates[event->control]);
            break;
        }
    }
    return 1;
}

// 0x0043d080
int DecodeTextureSize(int bits, int index, int byteIndex) {
    switch (bits >> (index * 2) >> (byteIndex * 8) & 3) {
    case 0:
        return 0x200;
    case 1:
        return 0x40;
    case 2:
        return 0x80;
    case 3:
        return 0x100;
    }
    return 0;
}

// 0x0057985c: writable zero-filled storage, shared by every interface.
// The original name and translation unit ownership are not established.
int g_LastBindingId;

// 0x0043ce90: callers supply their ControlInterface in ecx, but this body
// accesses only the global counter and returns its incremented value.
int ControlInterface::NextBindingId() {
    return ++g_LastBindingId;
}

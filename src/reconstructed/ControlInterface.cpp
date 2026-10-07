#include "ControlInterface.h"
#include "InputDevice.h"
#include "KeyboardDevice.h"
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
int UnknownFunction43caa0(int control, int kind, const UnknownControlEvent* event, int modifier) {
    if (event->control == control && event->kind == kind &&
        (!g_TrackGame->controlInterface->keyboard ||
         g_TrackGame->controlInterface->keyboard->UnknownFunction48a240(modifier)))
        return 1;
    return 0;
}

// 0x0043cae0
UnknownControlMapping::UnknownControlMapping() {
    UnknownFunction43caf0();
}

// 0x0043caf0
void UnknownControlMapping::UnknownFunction43caf0() {
    for (int i = 0; i < 50; i++) {
        keys[i] = -1;
        for (int j = 0; j < 8; j++)
            joystickButtons[j][i] = -1;
        mouseButtons[i] = -1;
    }
}

// 0x0043cb30
void UnknownControlMapping::UnknownFunction43cb30(int control, int key) {
    if (control >= 0 && control < 50)
        keys[control] = key;
}

// 0x0043cb50
void UnknownControlMapping::UnknownFunction43cb50(int control, int button, int device) {
    if (control >= 0 && control < 50 && device < 8)
        joystickButtons[device][control] = button;
}

// 0x0043cb80
void UnknownControlMapping::UnknownFunction43cb80(int control, int button) {
    if (control >= 0 && control < 50)
        mouseButtons[control] = button;
}

// 0x0043cba0
void UnknownControlMapping::UnknownFunction43cba0(int control, int* result) {
    if (control >= 0 && control < 50)
        *result = keys[control];
    else
        *result = -1;
}

// 0x0043cbd0
void UnknownControlMapping::UnknownFunction43cbd0(int control, int* result, int device) {
    if (control >= 0 && control < 50 && device < 8)
        *result = joystickButtons[device][control];
    else
        *result = -1;
}

// 0x0043cc10
void UnknownControlMapping::UnknownFunction43cc10(int control, int* result) {
    if (control >= 0 && control < 50)
        *result = mouseButtons[control];
    else
        *result = -1;
}

// 0x0043cc40
UnknownControlBinding::UnknownControlBinding() {
    UnknownFunction43cca0();
}

// 0x0043cc50
UnknownControlBinding::UnknownControlBinding(float minimum, float maximum, float center,
                                             int id, int axis, float deadZone) {
    UnknownFunction43cca0();
    field_0x2c = minimum;
    field_0x30 = maximum;
    field_0x24 = center;
    field_0x28 = center;
    field_0x04 = id;
    field_0x08 = axis;
    field_0x34 = center + deadZone;
    field_0x38 = center - deadZone;
}

// 0x0043cca0
void UnknownControlBinding::UnknownFunction43cca0() {
    field_0x24 = 0;
    field_0x28 = 0;
    field_0x2c = 0;
    field_0x30 = 0;
    field_0x34 = 0;
    field_0x38 = 0;
    field_0x00 = 0;
    field_0x04 = 0;
    field_0x08 = -1;
    field_0x0c = -1;
    field_0x10 = -1;
    field_0x14 = 0;
    field_0x18 = 0;
    field_0x1c = 0;
    field_0x20 = 0;
}

// 0x0043cce0
void UnknownControlBinding::UnknownFunction43cce0(int a, int b) {
    field_0x0c = a;
    field_0x10 = b;
}

// 0x0043cd00
UnknownControlBinding::~UnknownControlBinding() {
    UnknownFunction43cde0();
}

// 0x0043cd10
void UnknownControlBinding::UnknownFunction43cd10() {
    field_0x24 = field_0x28;
}

// 0x0043cd20: maps value/range onto [min, max]; inside the dead zone the
// value snaps to the centre.
void UnknownControlBinding::UnknownFunction43cd20(float value, float range) {
    float mapped = (field_0x30 - field_0x2c) * (value / range) + field_0x2c;
    if (mapped < field_0x34 && mapped > field_0x38)
        mapped = field_0x28;
    field_0x24 = BindingMin(BindingMax(mapped, field_0x2c), field_0x30);
}

// 0x0043cd90
void UnknownControlBinding::UnknownFunction43cd90(float delta) {
    field_0x24 += delta;
    field_0x24 = BindingMin(BindingMax(field_0x24, field_0x2c), field_0x30);
}

// 0x0043cde0: asks the owning device to drop bindings with this id.
void UnknownControlBinding::UnknownFunction43cde0() {
    if (g_TrackGame->controlInterface && field_0x00)
        field_0x00->UnknownVirtualSlot0(field_0x04);
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
int ControlInterface::UnknownFunction43ce70(UnknownControlMapping* newMapping) {
    if (!newMapping)
        return 0;
    mapping = newMapping;
    return 1;
}

// 0x0043cea0
void ControlInterface::UnknownFunction43cea0(int control, int kind, int pressed, int device) {
    events[queuedEventCount].control = control;
    events[queuedEventCount].device = device;
    events[queuedEventCount].kind = kind;
    events[queuedEventCount].pressed = pressed;
    events[queuedEventCount].modifiers = 0x3f;
    queuedEventCount++;
}

// 0x0043d080
int UnknownFunction43d080(int bits, int index, int byteIndex) {
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

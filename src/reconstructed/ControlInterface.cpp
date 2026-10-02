#include "ControlInterface.h"
#include "InputDevice.h"
#include "UnknownObject56e26c.h"

// Two-value helpers; inline functions keep their operands as spilled
// arguments, as retail does.
static inline float BindingMax(float a, float b) {
    return a > b ? a : b;
}

static inline float BindingMin(float a, float b) {
    return a < b ? a : b;
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
    if (g_UnknownGlobal56e26c->field_0x14 && field_0x00)
        field_0x00->UnknownVirtualSlot0(field_0x04);
}

// 0x0043ce00
ControlInterface::ControlInterface() {
    field_0x04 = 0;
    field_0x08 = 0;
    field_0x0c = 0;
    for (int i = 0; i < 8; i++)
        field_0x10[i] = 0;
    field_0x30 = 0;
    field_0x34 = 0;
    field_0xcbc = 0;
    field_0x38 = 0;
}

// 0x0043ce60
ControlInterface::~ControlInterface() {}

// 0x0043ce70
int ControlInterface::UnknownFunction43ce70(UnknownControlMapping* mapping) {
    if (!mapping)
        return 0;
    field_0xcbc = mapping;
    return 1;
}

// 0x0043cea0
void ControlInterface::UnknownFunction43cea0(int control, int kind, int pressed, int device) {
    field_0x3c[field_0x38].control = control;
    field_0x3c[field_0x38].device = device;
    field_0x3c[field_0x38].kind = kind;
    field_0x3c[field_0x38].pressed = pressed;
    field_0x3c[field_0x38].field_0x10 = 0x3f;
    field_0x38++;
}

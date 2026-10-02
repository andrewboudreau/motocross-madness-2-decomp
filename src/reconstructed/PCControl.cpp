#include <string.h>

#include "PCControl.h"

#include "DebugAlloc.h"
#include "PCJoystickDevice.h"
#include "PCKeyboardDevice.h"
#include "PCMouseDevice.h"
#include "UnknownObject56e26c.h"

// 0x004beef0: the effect GUID's name in `name` (empty and 0 when unknown).
// Placed here by address only: it directly precedes PCControlInterface.
int UnknownFunction4beef0(UnknownGuid guid, char* name) {
    strcpy(name, "");
    if (!memcmp(&GUID_ConstantForce, &guid, sizeof(guid))) {
        strcpy(name, "GUID_ConstantForce");
        return 1;
    }
    if (!memcmp(&GUID_RampForce, &guid, sizeof(guid))) {
        strcpy(name, "GUID_RampForce");
        return 1;
    }
    if (!memcmp(&GUID_Square, &guid, sizeof(guid))) {
        strcpy(name, "GUID_Square");
        return 1;
    }
    if (!memcmp(&GUID_Sine, &guid, sizeof(guid))) {
        strcpy(name, "GUID_Sine");
        return 1;
    }
    if (!memcmp(&GUID_Triangle, &guid, sizeof(guid))) {
        strcpy(name, "GUID_Triangle");
        return 1;
    }
    if (!memcmp(&GUID_SawtoothUp, &guid, sizeof(guid))) {
        strcpy(name, "GUID_SawtoothUp");
        return 1;
    }
    if (!memcmp(&GUID_SawtoothDown, &guid, sizeof(guid))) {
        strcpy(name, "GUID_SawtoothDown");
        return 1;
    }
    if (!memcmp(&GUID_Spring, &guid, sizeof(guid))) {
        strcpy(name, "GUID_Spring");
        return 1;
    }
    if (!memcmp(&GUID_Damper, &guid, sizeof(guid))) {
        strcpy(name, "GUID_Damper");
        return 1;
    }
    if (!memcmp(&GUID_Inertia, &guid, sizeof(guid))) {
        strcpy(name, "GUID_Inertia");
        return 1;
    }
    if (!memcmp(&GUID_Friction, &guid, sizeof(guid))) {
        strcpy(name, "GUID_Friction");
        return 1;
    }
    if (!memcmp(&GUID_CustomForce, &guid, sizeof(guid))) {
        strcpy(name, "GUID_CustomForce");
        return 1;
    }
    return 0;
}

// 0x004bf1f0
PCControlInterface::PCControlInterface() {
    field_0xcc0 = 0;
    field_0xcc4 = 1;
}

// 0x004bf240: creates DirectInput (version 0x700), the keyboard and the
// mouse, then the attached joysticks; without any joystick, one is created
// anyway. The first joystick becomes the active one.
int PCControlInterface::UnknownVirtualSlot1() {
    if (DirectInputCreateEx(g_UnknownGlobal56e26c->field_0x318, 0x700, IID_IDirectInput7A,
                            (void**)&field_0xcc0, 0) < 0)
        return 0;
    field_0x34 = new(__FILE__, 107) PCKeyboardDevice;
    if (!field_0x34)
        return 0;
    field_0x34->UnknownVirtualSlot2();
    field_0x30 = new(__FILE__, 114) PCMouseDevice;
    if (!field_0x30)
        return 0;
    field_0x30->UnknownVirtualSlot2();
    field_0x04 = 0;
    field_0xcc0->UnknownMethod4(4, UnknownEnumDevicesCallback, this, 1);
    if (!field_0x04)
        field_0x10[0] = new(__FILE__, 125) PCJoystickDevice(field_0x04);
    field_0x0c = field_0x10[0];
    return 1;
}

// 0x004bf3c0: opens each attached joystick, stopping at eight; a joystick
// that fails to open is deleted.
int __stdcall PCControlInterface::UnknownEnumDevicesCallback(const UnknownDeviceInstance* instance,
                                                             void* context) {
    PCControlInterface* control = (PCControlInterface*)context;
    control->field_0x10[control->field_0x04] = new(__FILE__, 73) PCJoystickDevice(control->field_0x04);
    if (control->field_0x10[control->field_0x04] &&
        ((PCJoystickDevice*)control->field_0x10[control->field_0x04])->UnknownFunction4c2930(instance)) {
        control->field_0x04++;
        if (control->field_0x04 == 8)
            return 0;
    } else {
        delete control->field_0x10[control->field_0x04];
        control->field_0x10[control->field_0x04] = 0;
    }
    return 1;
}

// 0x004bf490
void PCControlInterface::UnknownFunction4bf490(int acquire) {
    field_0xcc4 = acquire;
    if (field_0x34)
        field_0x34->UnknownMethod4c26d0(field_0xcc4);
    if (field_0x30)
        field_0x30->UnknownMethod4c26d0(field_0xcc4);
    for (int i = 0; i < 8; i++) {
        if (field_0x10[i])
            field_0x10[i]->UnknownMethod4c26d0(field_0xcc4);
    }
}

// 0x004bf560: `control` on the keyboard, the active joystick or the mouse.
int PCControlInterface::UnknownVirtualSlot2(int control, int modifier) {
    if (field_0x34 && field_0x34->UnknownVirtualSlot4(control, modifier, 0))
        return 1;
    if (field_0x10[field_0x08] && field_0x10[field_0x08]->UnknownFunction489c60(control, modifier, 0))
        return 1;
    if (field_0x30 && field_0x30->UnknownVirtualSlot4(control, modifier, 0))
        return 1;
    return 0;
}

// 0x004bf5e0
int PCControlInterface::UnknownVirtualSlot4(int modifier) {
    if (field_0x34)
        return field_0x34->UnknownFunction48a240(modifier);
    return 0;
}

// 0x004bf600
PCControlInterface::~PCControlInterface() {
    delete field_0x34;
    for (int i = 0; i < 8; i++)
        delete field_0x10[i];
    delete field_0x30;
    if (field_0xcc0) {
        field_0xcc0->UnknownMethod2();
        field_0xcc0 = 0;
    }
}

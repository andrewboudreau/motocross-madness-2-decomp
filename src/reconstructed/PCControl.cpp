#include <string.h>

#include "PCControl.h"

#include "DebugAlloc.h"
#include "PCJoystickDevice.h"
#include "PCKeyboardDevice.h"
#include "PCMouseDevice.h"
#include "TrackGame.h"

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
    directInput = 0;
    field_0xcc4 = 1;
}

// 0x004bf240: creates DirectInput (version 0x700), the keyboard and the
// mouse, then the attached joysticks; without any joystick, one is created
// anyway. The first joystick becomes the active one.
int PCControlInterface::UnknownVirtualSlot1() {
    if (DirectInputCreateEx(g_TrackGame->field_0x318, 0x700, IID_IDirectInput7A,
                            (void**)&directInput, 0) < 0)
        return 0;
    keyboard = new(__FILE__, 107) PCKeyboardDevice;
    if (!keyboard)
        return 0;
    keyboard->UnknownVirtualSlot2();
    mouse = new(__FILE__, 114) PCMouseDevice;
    if (!mouse)
        return 0;
    mouse->UnknownVirtualSlot2();
    joystickCount = 0;
    directInput->EnumDevices(4, UnknownEnumDevicesCallback, this, 1);
    if (!joystickCount)
        joysticks[0] = new(__FILE__, 125) PCJoystickDevice(joystickCount);
    activeJoystick = joysticks[0];
    return 1;
}

// 0x004bf3c0: opens each attached joystick, stopping at eight; a joystick
// that fails to open is deleted.
int __stdcall PCControlInterface::UnknownEnumDevicesCallback(const UnknownDeviceInstance* instance,
                                                             void* context) {
    PCControlInterface* control = (PCControlInterface*)context;
    control->joysticks[control->joystickCount] = new(__FILE__, 73) PCJoystickDevice(control->joystickCount);
    if (control->joysticks[control->joystickCount] &&
        ((PCJoystickDevice*)control->joysticks[control->joystickCount])->UnknownFunction4c2930(instance)) {
        control->joystickCount++;
        if (control->joystickCount == 8)
            return 0;
    } else {
        delete control->joysticks[control->joystickCount];
        control->joysticks[control->joystickCount] = 0;
    }
    return 1;
}

// 0x004bf490
void PCControlInterface::UnknownFunction4bf490(int acquire) {
    field_0xcc4 = acquire;
    if (keyboard)
        keyboard->UnknownMethod4c26d0(field_0xcc4);
    if (mouse)
        mouse->UnknownMethod4c26d0(field_0xcc4);
    for (int i = 0; i < 8; i++) {
        if (joysticks[i])
            joysticks[i]->UnknownMethod4c26d0(field_0xcc4);
    }
}

// 0x004bf560: `control` on the keyboard, the active joystick or the mouse.
int PCControlInterface::UnknownVirtualSlot2(int control, int modifier) {
    if (keyboard && keyboard->UnknownVirtualSlot4(control, modifier, 0))
        return 1;
    if (joysticks[activeJoystickIndex] && joysticks[activeJoystickIndex]->UnknownFunction489c60(control, modifier, 0))
        return 1;
    if (mouse && mouse->UnknownVirtualSlot4(control, modifier, 0))
        return 1;
    return 0;
}

// 0x004bf5e0
int PCControlInterface::UnknownVirtualSlot4(int modifier) {
    if (keyboard)
        return keyboard->UnknownFunction48a240(modifier);
    return 0;
}

// 0x004bf600
PCControlInterface::~PCControlInterface() {
    delete keyboard;
    for (int i = 0; i < 8; i++)
        delete joysticks[i];
    delete mouse;
    if (directInput) {
        directInput->Release();
        directInput = 0;
    }
}

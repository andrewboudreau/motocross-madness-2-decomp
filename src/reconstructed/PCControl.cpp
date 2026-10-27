#include <stdio.h>
#include <string.h>

#include "PCControl.h"

#include "DebugAlloc.h"
#include "PCJoystickDevice.h"
#include "PCKeyboardDevice.h"
#include "PCMouseDevice.h"
#include "TrackGame.h"

// 0x004beef0: the effect GUID's name in `name` (empty and 0 when unknown).
// Placed here by address only: it directly precedes PCControlInterface.
int GetForceFeedbackEffectName(UnknownGuid guid, char* name) {
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
    devicesAcquired = 1;
}

// 0x004bf240: creates DirectInput (version 0x700), the keyboard and the
// mouse, then the attached joysticks; without any joystick, one is created
// anyway. The first joystick becomes the active one.
int PCControlInterface::CreateDevices() {
    if (DirectInputCreateEx(g_TrackGame->instanceHandle, 0x700, IID_IDirectInput7A,
                            (void**)&directInput, 0) < 0)
        return 0;
    keyboard = new(__FILE__, 107) PCKeyboardDevice;
    if (!keyboard)
        return 0;
    keyboard->CreateDevice();
    mouse = new(__FILE__, 114) PCMouseDevice;
    if (!mouse)
        return 0;
    mouse->CreateDevice();
    joystickCount = 0;
    directInput->EnumDevices(DIDEVTYPE_JOYSTICK, EnumJoystickCallback, this, DIEDFL_ATTACHEDONLY);
    if (!joystickCount)
        joysticks[0] = new(__FILE__, 125) PCJoystickDevice(joystickCount);
    activeJoystick = joysticks[0];
    return 1;
}

// 0x004bf3c0: opens each attached joystick, stopping at eight; a joystick
// that fails to open is deleted.
int __stdcall PCControlInterface::EnumJoystickCallback(const UnknownDeviceInstance* instance,
    void* context) {
    PCControlInterface* control = (PCControlInterface*)context;
    control->joysticks[control->joystickCount] = new(__FILE__, 73) PCJoystickDevice(control->joystickCount);
    if (control->joysticks[control->joystickCount] &&
        ((PCJoystickDevice*)control->joysticks[control->joystickCount])->OpenDevice(instance)) {
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
void PCControlInterface::SetDevicesAcquired(int acquire) {
    devicesAcquired = acquire;
    if (keyboard)
        keyboard->SetAcquired(devicesAcquired);
    if (mouse)
        mouse->SetAcquired(devicesAcquired);
    for (int i = 0; i < 8; i++) {
        if (joysticks[i])
            joysticks[i]->SetAcquired(devicesAcquired);
    }
}

// 0x004bf4f0: `control` on one device: kind 0 the keyboard, 1 the mouse,
// 2 and 3 joystick `device`; 0 when that device is missing.
int PCControlInterface::IsDeviceButtonDown(int control, int kind, int modifier, int device) {
    switch (kind) {
    case 0:
        if (keyboard)
            return keyboard->IsButtonDown(control, modifier, 0);
        return 0;
    case 1:
        if (mouse)
            return mouse->IsButtonDown(control, modifier, 0);
        return 0;
    case 2:
    case 3:
        if (joysticks[device])
            return joysticks[device]->IsButtonDown(control, modifier, 0);
        return 0;
    }
    return 0;
}

// 0x004bf560: `control` on the keyboard, the active joystick or the mouse.
int PCControlInterface::IsControlDown(int control, int modifier) {
    if (keyboard && keyboard->IsControlDown(control, modifier, 0))
        return 1;
    if (joysticks[activeJoystickIndex] && joysticks[activeJoystickIndex]->GetControlState(control, modifier, 0))
        return 1;
    if (mouse && mouse->IsControlDown(control, modifier, 0))
        return 1;
    return 0;
}

// 0x004bf5e0
int PCControlInterface::ModifierHolds(int modifier) {
    if (keyboard)
        return keyboard->ModifierHolds(modifier);
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

// 0x004bf6a0: formats a DirectInput result with the caller's __FILE__ and
// __LINE__ (the text is not used further). Placed in PCControl.cpp by address only.
void UnknownReportError(long result, const char* file, int line) {
    char name[256];
    char text[1024];
    switch (result) {
    case (long)0x80004001:
        sprintf(name, "DIERR_UNSUPPORTED");
        break;
    case (long)0x8000000a:
        sprintf(name, "E_PENDING");
        break;
    case (long)0x80004002:
        sprintf(name, "DIERR_NOINTERFACE");
        break;
    case (long)0x80004005:
        sprintf(name, "DIERR_GENERIC");
        break;
    case (long)0x80040110:
        sprintf(name, "DIERR_NOAGGREGATION");
        break;
    case (long)0x80040201:
        sprintf(name, "DIERR_DEVICEFULL");
        break;
    case (long)0x80040200:
        sprintf(name, "DIERR_INSUFFICIENTPRIVS");
        break;
    case (long)0x80040154:
        sprintf(name, "DIERR_DEVICENOTREG");
        break;
    case (long)0x80040202:
        sprintf(name, "DIERR_MOREDATA");
        break;
    case (long)0x80040203:
        sprintf(name, "DIERR_NOTDOWNLOADED");
        break;
    case (long)0x80040204:
        sprintf(name, "DIERR_HASEFFECTS");
        break;
    case (long)0x80040205:
        sprintf(name, "DIERR_NOTEXCLUSIVEACQUIRED");
        break;
    case (long)0x80040206:
        sprintf(name, "DIERR_INCOMPLETEEFFECT");
        break;
    case (long)0x80040207:
        sprintf(name, "DIERR_NOTBUFFERED");
        break;
    case (long)0x80040208:
        sprintf(name, "DIERR_EFFECTPLAYING");
        break;
    case (long)0x80040209:
        sprintf(name, "DIERR_UNPLUGGED");
        break;
    case (long)0x8004020a:
        sprintf(name, "DIERR_REPORTFULL");
        break;
    case (long)0x80070002:
        sprintf(name, "DIERR_OBJECTNOTFOUND|DIERR_NOTFOUND");
        break;
    case (long)0x80070077:
        sprintf(name, "DIERR_BADDRIVERVER");
        break;
    case (long)0x80070057:
        sprintf(name, "DIERR_INVALIDPARAM");
        break;
    case (long)0x8007000e:
        sprintf(name, "DIERR_OUTOFMEMORY");
        break;
    case (long)0x80070015:
        sprintf(name, "DIERR_NOTINITIALIZED");
        break;
    case (long)0x8007001e:
        sprintf(name, "DIERR_INPUTLOST");
        break;
    case (long)0x800700aa:
        sprintf(name, "DIERR_ACQUIRED");
        break;
    case (long)0x8007000c:
        sprintf(name, "DIERR_NOTACQUIRED");
        break;
    case (long)0x80070005:
        sprintf(name, "DIERR_OTHERAPPHASPRIO|DIERR_READONLY|DIERR_HANDLEEXISTS");
        break;
    case (long)0x8007047e:
        sprintf(name, "DIERR_OLDDIRECTINPUTVERSION");
        break;
    case (long)0x800704df:
        sprintf(name, "DIERR_ALREADYINITIALIZED");
        break;
    case (long)0x80070481:
        sprintf(name, "DIERR_BETADIRECTINPUTVERSION");
        break;
    case 1:
        sprintf(name, "DI_NOTATTACHED|DI_BUFFEROVERFLOW|DI_PROPNOEFFECT|DI_NOEFFECT");
        break;
    case 2:
        sprintf(name, "DI_POLLEDDEVICE");
        break;
    case 3:
        sprintf(name, "DI_DOWNLOADSKIPPED");
        break;
    case 12:
        sprintf(name, "DI_TRUNCATEDANDRESTARTED");
        break;
    case 8:
        sprintf(name, "DI_TRUNCATED");
        break;
    case 4:
        sprintf(name, "DI_EFFECTRESTARTED");
        break;
    default:
        sprintf(name, "Unknown Error");
        break;
    }
    sprintf(text, "DirectInput Error %s in file %s at line %d\n", name, file, line);
}

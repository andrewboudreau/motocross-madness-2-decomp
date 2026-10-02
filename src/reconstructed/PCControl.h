#pragma once

#include "ControlInterface.h"

struct UnknownDeviceInstance;

// RTTI: PCControlInterface : ControlInterface. Its functions pass the literal
// __FILE__ "D:\\aardvark\\VC\\krusty2\\PCControl.cpp", which names the TU.
// It owns the DirectInput object (+0xcc0) and the keyboard, mouse and up to
// eight joysticks it creates.
class PCControlInterface : public ControlInterface {
public:
    PCControlInterface();                  // 0x004bf1f0
    virtual ~PCControlInterface();         // 0x004bf600 (deleting wrapper 0x004bf220)
    virtual int UnknownVirtualSlot1();     // 0x004bf240
    virtual int UnknownVirtualSlot2(int control, int modifier);                       // 0x004bf560
    // 0x004bf4f0, near miss in samples/control
    virtual int UnknownVirtualSlot3(int control, int kind, int modifier, int device);
    virtual int UnknownVirtualSlot4(int modifier);                                    // 0x004bf5e0

    // 0x004bf490: acquires (nonzero) or unacquires every device.
    void UnknownFunction4bf490(int acquire);
    // 0x004bf3c0: joystick enumeration callback.
    static int __stdcall UnknownEnumDevicesCallback(const UnknownDeviceInstance* instance,
                                                    void* context);

    int field_0xcc4;                       // acquired; 1 initially
};

#pragma once

#include "JoystickDevice.h"

// RTTI: PCJoystickDevice : JoystickDevice. PCInputDeviceType.cpp is the
// nearest source reference; the TU is not established. Slots 15-18 drive the
// device's force feedback: method 22 of the +0x25c interface (consistent
// with IDirectInputDevice2::SendForceFeedbackCommand) with commands 1, 4 and
// 8, only for device type 3 (+0x0c).
class PCJoystickDevice : public JoystickDevice {
public:
    explicit PCJoystickDevice(int index); // 0x004c2770
    virtual ~PCJoystickDevice();          // 0x004c28d0 (deleting wrapper 0x004c28b0)

    virtual void UnknownVirtualSlot15();
    virtual int UnknownVirtualSlot16();
    virtual int UnknownVirtualSlot17(int command);
    virtual int UnknownVirtualSlot18(int paused);
    virtual void UnknownVirtualSlot19(); // 0x004c3af0, near miss in samples/inputdevice

protected:
    UnknownInputInterface* field_0x578[5]; // released by slot 15
    int field_0x58c[6];
    int field_0x5a4[6];
    int field_0x5bc[6];
    unsigned char field_0x5d4_bit0 : 1;
    unsigned char field_0x5d4_bit1 : 1;
    unsigned char field_0x5d5;
    int field_0x5d8[4];                    // -1 initially
    int field_0x5e8;
};

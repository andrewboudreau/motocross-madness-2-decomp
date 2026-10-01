#pragma once

// RTTI: InputDevice (root) -> PCInputDevice -> Joystick/Keyboard/MouseDevice
// -> PCJoystick/PCKeyboard/PCMouseDevice. InputDevice's slot 0 is _purecall and
// slot 1 its destructor, so a pure virtual is declared before the destructor.
// Its code sits near ContainerList.h references; the TU is not established.
// Names are provisional.
class InputDevice {
public:
    explicit InputDevice(int id); // 0x00489780

    virtual void UnknownVirtualSlot0() = 0;
    virtual ~InputDevice();       // 0x004897d0 (deleting wrapper 0x004897b0)

    int UnknownFunction4897e0(int value); // 0x004897e0

protected:
    int field_0x04;
    int field_0x08;
    int field_0x0c;               // id passed to the constructor
    int field_0x10;               // -1 initially
    unsigned char field_0x14_bits : 6;
};

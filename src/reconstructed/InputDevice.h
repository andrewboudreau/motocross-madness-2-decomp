#pragma once

// RTTI: InputDevice (root) -> PCInputDevice -> Joystick/Keyboard/MouseDevice
// -> PCJoystick/PCKeyboard/PCMouseDevice. InputDevice's slot 0 is _purecall and
// slot 1 its destructor, so a pure virtual is declared before the destructor.
// Its code sits near ContainerList.h references; the TU is not established.
// Names are provisional.
class InputDevice {
public:
    explicit InputDevice(int id); // 0x00489780

    virtual void UnknownVirtualSlot0(int id) = 0; // drops bindings by id
    virtual ~InputDevice();       // 0x004897d0 (deleting wrapper 0x004897b0)

    int UnknownFunction4897e0(int value); // 0x004897e0

protected:
    int axisCount;                  // +0x04
    int buttonCount;                // +0x08
    int deviceKind;                 // +0x0c; id passed to the constructor
    int deviceSubtype;              // +0x10; -1 initially
    unsigned char field_0x14_axis0 : 1;  // axis enable bits; JoystickDevice
    unsigned char field_0x14_axis1 : 1;  // 0x00489c60 tests one per axis
    unsigned char field_0x14_axis2 : 1;
    unsigned char field_0x14_axis3 : 1;
    unsigned char field_0x14_axis4 : 1;
    unsigned char field_0x14_axis5 : 1;
};

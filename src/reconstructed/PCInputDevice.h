#pragma once

#include "InputDevice.h"

// COM-style device at PCInputDevice+0x25c (`this` on the stack). Method 8 is
// called before Release when the device is destroyed, consistent with
// IDirectInputDevice::Unacquire, and PCJoystickDevice uses method 22 like
// IDirectInputDevice2::SendForceFeedbackCommand; the identity is inference.
struct UnknownInputInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();   // Release
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();   // Unacquire
    virtual long __stdcall UnknownMethod9();
    virtual long __stdcall UnknownMethod10();
    virtual long __stdcall UnknownMethod11();
    virtual long __stdcall UnknownMethod12();
    virtual long __stdcall UnknownMethod13();
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall UnknownMethod15();
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall UnknownMethod17();
    virtual long __stdcall UnknownMethod18();
    virtual long __stdcall UnknownMethod19();
    virtual long __stdcall UnknownMethod20();
    virtual long __stdcall UnknownMethod21();
    virtual long __stdcall UnknownMethod22(int command); // SendForceFeedbackCommand
};

// cdecl 0x004bfa80: the value the keyboard and mouse stamp into their input
// entries when they are constructed.
unsigned int UnknownFunction4bfa80();

// 20-byte input entry (keys at KeyboardDevice+0x260, buttons at
// MouseDevice+0x260): a state and four stamped values.
struct UnknownInputEntry {
    int state;
    unsigned int field_0x04;
    unsigned int field_0x08;
    unsigned int field_0x0c;
    unsigned int field_0x10;
};

// RTTI: PCInputDevice : InputDevice. PCInputDeviceType.cpp is the nearest
// source reference; the TU is not established.
class PCInputDevice : public InputDevice {
public:
    explicit PCInputDevice(int id); // 0x004c25f0
    virtual ~PCInputDevice();       // 0x004c2650 (deleting wrapper 0x004c2630)

protected:
    unsigned char field_0x18[0x244];   // cleared by the constructor
    UnknownInputInterface* field_0x25c;
};

#pragma once

#include "JoystickDevice.h"

// COM-style effect object held at PCJoystickDevice+0x578. Methods 7-9 are used
// like IDirectInputEffect::Start, Stop and GetEffectStatus; the identity is
// inference.
struct UnknownEffectInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();   // Release
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7(unsigned long iterations, unsigned long flags); // Start
    virtual long __stdcall UnknownMethod8();   // Stop
    virtual long __stdcall UnknownMethod9(unsigned long* status); // GetEffectStatus
};

// RTTI: PCJoystickDevice : JoystickDevice. PCInputDeviceType.cpp is the
// nearest source reference; the TU is not established. Slots 15-18 drive the
// device's force feedback: method 22 of the +0x25c interface (consistent
// with IDirectInputDevice2::SendForceFeedbackCommand) with commands 1, 4 and
// 8, only for device type 3 (+0x0c).
class PCJoystickDevice : public JoystickDevice {
public:
    explicit PCJoystickDevice(int index); // 0x004c2770
    virtual ~PCJoystickDevice();          // 0x004c28d0 (deleting wrapper 0x004c28b0)

    virtual int UnknownVirtualSlot3(int index, float* angle);
    virtual int UnknownVirtualSlot4(int index, float* x, float* y);
    virtual int UnknownVirtualSlot5(int enable);
    virtual int UnknownVirtualSlot11(int effect);
    virtual int UnknownVirtualSlot12(int effect, unsigned long iterations, unsigned long flags);
    virtual int UnknownVirtualSlot13(int effect);
    virtual int UnknownVirtualSlot14(UnknownEffectInfo* effects, int* count);
    virtual void UnknownVirtualSlot15();
    virtual int UnknownVirtualSlot16();
    virtual int UnknownVirtualSlot17(int command);
    virtual int UnknownVirtualSlot18(int paused);
    virtual void UnknownVirtualSlot19();

protected:
    UnknownEffectInterface* field_0x578[5]; // released by slot 15
    int field_0x58c[6];
    int field_0x5a4[6];
    int field_0x5bc[6];
    unsigned char field_0x5d4_bit0 : 1;
    unsigned char field_0x5d4_bit1 : 1;
    unsigned char field_0x5d5;
    int field_0x5d8[4];                    // -1 initially
    int field_0x5e8;
};

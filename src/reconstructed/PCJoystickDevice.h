#pragma once

#include "JoystickDevice.h"

// 0x38-byte effect parameter block passed to device method 18 and effect
// method 6; the layout matches DIEFFECT (DirectX 6 size).
struct UnknownEffectParams {
    unsigned long size;
    unsigned long flags;
    unsigned long duration;
    unsigned long samplePeriod;
    unsigned long gain;
    unsigned long triggerButton;
    unsigned long triggerRepeatInterval;
    unsigned long axisCount;
    unsigned long* axes;
    long* direction;
    struct UnknownEnvelope* envelope;
    unsigned long typeSpecificSize;
    void* typeSpecific;
    unsigned long startDelay;
};

// Attack/fade envelope (DIENVELOPE layout).
struct UnknownEnvelope {
    unsigned long size;
    unsigned long attackLevel;
    unsigned long attackTime;
    unsigned long fadeLevel;
    unsigned long fadeTime;
};

// Periodic-force parameters (DIPERIODIC layout).
struct UnknownPeriodic {
    unsigned long magnitude;
    long offset;
    unsigned long phase;
    unsigned long period;
};

// 0x50-byte immediate joystick state (DIJOYSTATE layout).
struct UnknownJoystickState {
    long axes[6];
    long sliders[2];
    unsigned long pov[4];
    unsigned char buttons[32];
};

// COM-style effect object held at PCJoystickDevice+0x578. Methods 7-9 are used
// like IDirectInputEffect::Start, Stop and GetEffectStatus; the identity is
// inference.
struct UnknownEffectInterface {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall Release();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall SetParameters(const struct UnknownEffectParams* params,
                                          unsigned long flags); // SetParameters
    virtual long __stdcall Start(unsigned long iterations, unsigned long flags);
    virtual long __stdcall Stop();
    virtual long __stdcall GetEffectStatus(unsigned long* status);
};

// RTTI: PCJoystickDevice : JoystickDevice. Slot 20 and 0x004c3a10 pass the
// literal __FILE__ "D:\aardvark\VC\krusty2\PCInputDeviceType.cpp", so at
// least those were compiled in PCInputDeviceType.cpp. Slots 15-18 drive the
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
    virtual int UnknownVirtualSlot6(int effect, long magnitude, unsigned long duration);
    virtual int UnknownVirtualSlot7(int effect, long* direction, long magnitude);
    virtual int UnknownVirtualSlot8(int effect, long* direction);
    virtual int UnknownVirtualSlot9(int effect, unsigned long duration, long direction,
                                    long magnitude, unsigned long attackTime,
                                    unsigned long attackLevel, unsigned long fadeTime,
                                    unsigned long fadeLevel, int button);
    virtual int UnknownVirtualSlot10(int effect, unsigned long duration, unsigned long period,
                                     unsigned long magnitude, unsigned long attackTime,
                                     unsigned long attackLevel, unsigned long fadeTime,
                                     unsigned long fadeLevel, int button);
    virtual int UnknownVirtualSlot11(int effect);
    virtual int UnknownVirtualSlot12(int effect, unsigned long iterations, unsigned long flags);
    virtual int UnknownVirtualSlot13(int effect);
    virtual int UnknownVirtualSlot14(UnknownEffectInfo* effects, int* count);
    virtual void UnknownVirtualSlot15();
    virtual int UnknownVirtualSlot16();
    virtual int UnknownVirtualSlot17(int command);
    virtual int UnknownVirtualSlot18(int paused);
    virtual void UnknownVirtualSlot19();
    virtual int UnknownVirtualSlot20(int value);

    // 0x004c2930: opens the device described by `instance`.
    int UnknownFunction4c2930(const UnknownDeviceInstance* instance);
    // 0x004c2cb0: object enumeration callback; marks the axes present.
    static int __stdcall UnknownEnumObjectsCallback(const UnknownObjectInstance* object,
                                                    void* context);
    // 0x004c2d90: key-repeat stepping driven by an axis direction (gamepads).
    void UnknownMethod4c2d90(int axis, int value);
    // 0x004c3090: applies the axis dead-zone scale around 32768, clamped to
    // 0..65535.
    int UnknownFunction4c3090(int axis, int value);
    // 0x004c2ef0: button index for a joystick state offset, or -1.
    int UnknownFunction4c2ef0(int offset);
    int UnknownMethod4c3100(int value);            // near miss in samples/inputdevice
    int FilterAxisSpike(int axis, unsigned long value); // inline (near miss sample)
    int UnknownMethod4c3790(int value);            // near miss in samples/inputdevice
    int UnknownMethod4c3a10(int buffered);
    int CheckPollResult(long result);              // inline
    void UnknownMethod4c3ae0(unsigned char value);

protected:
    UnknownEffectInterface* field_0x578[5]; // released by slot 15
    float field_0x58c[6];                  // axis ranges (flip base for axes 1, 2)
    float field_0x5a4[6];                  // previous raw axis values
    float field_0x5bc[6];                  // dead-zone scale per axis
    unsigned char field_0x5d4_bit0 : 1;
    unsigned char field_0x5d4_bit1 : 1;
    unsigned char field_0x5d5;
    int field_0x5d8[4];                    // -1 initially
    int field_0x5e8;
};

#pragma once

#include "ContainerList.h"
#include "PCInputDevice.h"

// RTTI: JoystickDevice : PCInputDevice. It overrides slot 0, introduces
// slot 2 (0x00489980) and pure slots 3-20 that PCJoystickDevice implements.
// Signatures of slots not yet reconstructed are placeholders. TU not
// established.
class JoystickDevice : public PCInputDevice {
public:
    explicit JoystickDevice(int index); // 0x00489800

    virtual void UnknownVirtualSlot0(); // 0x00489b70, not reconstructed
    virtual ~JoystickDevice();          // 0x00489920 (deleting wrapper 0x00489900)
    virtual void UnknownVirtualSlot2(); // 0x00489980, not reconstructed
    virtual int UnknownVirtualSlot3(int index, float* angle) = 0;
    virtual int UnknownVirtualSlot4(int index, float* x, float* y) = 0;
    virtual int UnknownVirtualSlot5(int enable) = 0;
    virtual int UnknownVirtualSlot6(int effect, long magnitude, unsigned long duration) = 0;
    virtual int UnknownVirtualSlot7(int effect, long* direction, long magnitude) = 0;
    virtual int UnknownVirtualSlot8(int effect, long* direction) = 0;
    virtual int UnknownVirtualSlot9(int effect, unsigned long duration, long direction,
                                    long magnitude, unsigned long attackTime,
                                    unsigned long attackLevel, unsigned long fadeTime,
                                    unsigned long fadeLevel, int button) = 0;
    virtual int UnknownVirtualSlot10(int effect, unsigned long duration, unsigned long period,
                                     unsigned long magnitude, unsigned long attackTime,
                                     unsigned long attackLevel, unsigned long fadeTime,
                                     unsigned long fadeLevel, int button) = 0;
    virtual int UnknownVirtualSlot11(int effect) = 0;
    virtual int UnknownVirtualSlot12(int effect, unsigned long iterations, unsigned long flags) = 0;
    virtual int UnknownVirtualSlot13(int effect) = 0;
    virtual int UnknownVirtualSlot14(UnknownEffectInfo* effects, int* count) = 0;
    virtual void UnknownVirtualSlot15() = 0;
    virtual int UnknownVirtualSlot16() = 0;
    virtual int UnknownVirtualSlot17(int command) = 0;
    virtual int UnknownVirtualSlot18(int paused) = 0;
    virtual void UnknownVirtualSlot19() = 0;
    virtual int UnknownVirtualSlot20(int value) = 0;

protected:
    int field_0x260;                     // constructor argument
    UnknownInputEntry field_0x264[32];
    int field_0x4e4[6];
    ContainerList<int> field_0x4fc[6];
    unsigned char field_0x574_bit0 : 1;  // "JoyDirectionFlipped" setting
};

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
    virtual void UnknownVirtualSlot3() = 0;
    virtual void UnknownVirtualSlot4() = 0;
    virtual void UnknownVirtualSlot5() = 0;
    virtual void UnknownVirtualSlot6() = 0;
    virtual void UnknownVirtualSlot7() = 0;
    virtual void UnknownVirtualSlot8() = 0;
    virtual void UnknownVirtualSlot9() = 0;
    virtual void UnknownVirtualSlot10() = 0;
    virtual void UnknownVirtualSlot11() = 0;
    virtual void UnknownVirtualSlot12() = 0;
    virtual void UnknownVirtualSlot13() = 0;
    virtual void UnknownVirtualSlot14() = 0;
    virtual void UnknownVirtualSlot15() = 0;
    virtual int UnknownVirtualSlot16() = 0;
    virtual int UnknownVirtualSlot17(int command) = 0;
    virtual int UnknownVirtualSlot18(int paused) = 0;
    virtual void UnknownVirtualSlot19() = 0;
    virtual void UnknownVirtualSlot20() = 0;

protected:
    int field_0x260;                     // constructor argument
    UnknownInputEntry field_0x264[32];
    int field_0x4e4[6];
    ContainerList<int> field_0x4fc[6];
    unsigned char field_0x574_bit0 : 1;  // "JoyDirectionFlipped" setting
};

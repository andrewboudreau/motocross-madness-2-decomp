#pragma once

#include "ContainerList.h"
#include "PCInputDevice.h"

// RTTI: JoystickDevice : PCInputDevice. It introduces slot 2 (0x00489980)
// and pure slots 3-20 that PCJoystickDevice implements. TU not established.
class JoystickDevice : public PCInputDevice {
public:
    explicit JoystickDevice(int index); // 0x00489800
    virtual ~JoystickDevice();          // 0x00489920 (deleting wrapper 0x00489900)

protected:
    int field_0x260;                     // constructor argument
    UnknownInputEntry field_0x264[32];
    int field_0x4e4[6];
    ContainerList<int> field_0x4fc[6];
    unsigned char field_0x574_bit0 : 1;  // "JoyDirectionFlipped" setting
};

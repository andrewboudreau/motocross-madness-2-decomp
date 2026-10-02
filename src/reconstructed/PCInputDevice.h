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
    virtual long __stdcall UnknownMethod3(struct UnknownDeviceCaps* caps); // GetCapabilities
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6(int property, struct UnknownInputProperty* value); // SetProperty
    virtual long __stdcall UnknownMethod7();   // Acquire
    virtual long __stdcall UnknownMethod8();   // Unacquire
    virtual long __stdcall UnknownMethod9(unsigned long size, void* state); // GetDeviceState
    virtual long __stdcall UnknownMethod10(unsigned long size, struct UnknownDeviceObjectData* data,
                                           long* count, unsigned long flags); // GetDeviceData
    virtual long __stdcall UnknownMethod11(const struct UnknownDataFormat* format); // SetDataFormat
    virtual long __stdcall UnknownMethod12();
    virtual long __stdcall UnknownMethod13(void* window, unsigned long flags); // SetCooperativeLevel
    virtual long __stdcall UnknownMethod14();
    virtual long __stdcall UnknownMethod15(struct UnknownDeviceInstance* info); // GetDeviceInfo
    virtual long __stdcall UnknownMethod16();
    virtual long __stdcall UnknownMethod17();
    virtual long __stdcall UnknownMethod18(const struct UnknownGuid& type,
                                           const struct UnknownEffectParams* params,
                                           struct UnknownEffectInterface** effect,
                                           void* outer); // CreateEffect
    virtual long __stdcall UnknownMethod19(int (__stdcall* callback)(const struct UnknownEffectInfo*, void*),
                                           void* context, unsigned long type); // EnumEffects
    virtual long __stdcall UnknownMethod20();
    virtual long __stdcall UnknownMethod21();
    virtual long __stdcall UnknownMethod22(int command); // SendForceFeedbackCommand
    virtual long __stdcall UnknownMethod23();
    virtual long __stdcall UnknownMethod24();
    virtual long __stdcall UnknownMethod25(); // Poll
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

// 20-byte property block passed to interface method 6; the layout matches
// DIPROPDWORD (a 16-byte header, then one value).
struct UnknownInputProperty {
    unsigned long size;
    unsigned long headerSize;
    unsigned long object;
    unsigned long how;
    unsigned long data;
};

// 16-byte GUID (Win32 GUID layout).
struct UnknownGuid {
    unsigned long data1;
    unsigned short data2;
    unsigned short data3;
    unsigned char data4[8];
};

// 0x244-byte device description at PCInputDevice+0x18. Its size and the
// offsets used (type at +0x24, product name at +0x12c) match DIDEVICEINSTANCEA.
struct UnknownDeviceInstance {
    unsigned long size;
    UnknownGuid instanceGuid;
    UnknownGuid productGuid;
    unsigned long deviceType;      // subtype in bits 8-15
    char instanceName[260];
    char productName[260];
    UnknownGuid driverGuid;
    unsigned short usagePage;
    unsigned short usage;
};

// Device subtype: bits 8-15 of the type, spelled like Windows HIBYTE (as in
// DirectInput's GET_DIDEVICE_SUBTYPE). The cast chain is what makes VC6 load
// the byte and then mask it again, as retail does.
#define DEVICE_SUBTYPE(type) ((unsigned char)(((unsigned short)(type) >> 8) & 0xff))

// 0x124-byte effect description filled by interface method 19's callback; the
// layout matches DIEFFECTINFOA.
struct UnknownEffectInfo {
    unsigned long size;
    UnknownGuid guid;
    unsigned long effectType;
    unsigned long staticParams;
    unsigned long dynamicParams;
    char name[260];
};

// cdecl 0x004beef0: writes a name for a known effect GUID; nonzero if known.
int UnknownFunction4beef0(UnknownGuid guid, char* name);

// cdecl 0x004bf6a0: reports a failed result with the caller's __FILE__ and
// __LINE__ (it maps DirectX error codes to text).
void UnknownReportError(long result, const char* file, int line);

// DirectInput object at ControlInterface+0xcc0; method 9 creates devices like
// IDirectInput7::CreateDeviceEx.
struct UnknownDirectInput {
    virtual long __stdcall UnknownMethod0();
    virtual long __stdcall UnknownMethod1();
    virtual long __stdcall UnknownMethod2();
    virtual long __stdcall UnknownMethod3();
    virtual long __stdcall UnknownMethod4();
    virtual long __stdcall UnknownMethod5();
    virtual long __stdcall UnknownMethod6();
    virtual long __stdcall UnknownMethod7();
    virtual long __stdcall UnknownMethod8();
    virtual long __stdcall UnknownMethod9(const UnknownGuid& device, const UnknownGuid& iid,
                                          UnknownInputInterface** result, void* outer);
};

// 24-byte data format descriptor (DIDATAFORMAT layout).
struct UnknownDataFormat {
    unsigned long size;
    unsigned long objectSize;
    unsigned long flags;
    unsigned long dataSize;
    unsigned long objectCount;
    const void* objects;
};

// 0x2c-byte device capabilities (DIDEVCAPS layout).
struct UnknownDeviceCaps {
    unsigned long size;
    unsigned long flags;
    unsigned long deviceType;
    unsigned long axes;
    unsigned long buttons;
    unsigned long povs;
    unsigned long field_0x18[5];
};

// GUID and data-format constants. The GUID values decoded from retail are
// the standard GUID_SysMouse ({6F1D2B60-D5A0-11CF-BFC7-444553540000}),
// GUID_SysKeyboard (...2B61...) and IID_IDirectInputDevice7A
// ({57D7C6BC-2356-11D3-8E9D-00C04F6844AE}); the data formats have the sizes
// of c_dfDIKeyboard (256 objects, 256 bytes) and c_dfDIMouse (7, 16).
extern "C" const UnknownGuid GUID_SysMouse;              // 0x00556b20
extern "C" const UnknownGuid GUID_SysKeyboard;           // 0x00556b30
extern "C" const UnknownGuid IID_IDirectInputDevice7A;   // 0x00556a50
extern "C" const UnknownDataFormat c_dfDIKeyboard;       // 0x00558f18
extern "C" const UnknownDataFormat c_dfDIMouse;          // 0x00558f30

// 16-byte buffered input event (DIDEVICEOBJECTDATA layout).
struct UnknownDeviceObjectData {
    unsigned long offset;      // key or object
    unsigned long data;        // bit 7: pressed
    unsigned long timeStamp;
    unsigned long sequence;
};

// RTTI: PCInputDevice : InputDevice. PCInputDeviceType.cpp is the nearest
// source reference; the TU is not established.
class PCInputDevice : public InputDevice {
public:
    explicit PCInputDevice(int id); // 0x004c25f0
    virtual ~PCInputDevice();       // 0x004c2650 (deleting wrapper 0x004c2630)

    // 0x004c26d0: interface method 7 when nonzero, else method 8; 1 on success.
    int UnknownMethod4c26d0(int acquire);
    // 0x004c2710: sets a one-value property through interface method 6.
    int UnknownMethod4c2710(int property, unsigned long object, unsigned long how,
                            unsigned long data);

protected:
    UnknownDeviceInstance field_0x18;   // cleared by the constructor
    UnknownInputInterface* field_0x25c;
};

#pragma once

class InputDevice;

// The keyboard at ControlInterface+0x34, as FollowCamera slots 55 and 56 see
// it: they call slot 5 with 0x38 and 0x2a (DirectInput left Alt and left
// Shift), and JoystickDevice calls KeyboardDevice's 0x0048a240 on it.
// KeyboardDevice itself is not used for this member because the two views
// conflict: KeyboardDevice 0x0048a0c0 tests slot 5's full eax (int), while
// FollowCamera slot 56 returns it unconverted as bool (KrustyBikeCamera's
// override of slot 56 is bool).
class UnknownInterface56e26c {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual bool UnknownVirtualSlot5(int a, int b, int c);

    // 0x0048a240 (in KeyboardDevice's code): whether modifier state
    // `modifier` currently holds.
    int UnknownFunction48a240(int modifier);
};

// 0x3c-byte binding of a control to an input device (not polymorphic). The
// joystick lists at JoystickDevice+0x4fc hold pointers to these, and
// JoystickDevice::0x00489a20 stores the device at +0x00. Its helpers sit
// just before ControlInterface's constructor; the TU is not established.
struct UnknownControlBinding {
    UnknownControlBinding();                                  // 0x0043cc40
    UnknownControlBinding(float minimum, float maximum, float center, int id,
                          int axis, float deadZone);         // 0x0043cc50

    ~UnknownControlBinding();                                 // 0x0043cd00: unbinds

    void UnknownFunction43cca0();                             // 0x0043cca0: reset
    void UnknownFunction43cce0(int a, int b);                 // 0x0043cce0
    void UnknownFunction43cd10();                             // 0x0043cd10: recentre
    void UnknownFunction43cd20(float value, float range);     // 0x0043cd20
    void UnknownFunction43cd90(float delta);                  // 0x0043cd90
    void UnknownFunction43cde0();                             // 0x0043cde0: unbind

    InputDevice* field_0x00;  // owning device
    int field_0x04;           // id the device's slot 0 unbinds by
    int field_0x08;           // axis or list index; -1 when reset
    int field_0x0c;           // -1 when reset
    int field_0x10;           // -1 when reset
    float field_0x14;         // KeyboardDevice 0x0048a0c0: timer for key +0x0c
    float field_0x18;         // timer for key +0x10
    float field_0x1c;         // step per repeat
    float field_0x20;         // repeat interval
    float field_0x24;         // current value, kept within [min, max]
    float field_0x28;         // centre; used inside the dead zone
    float field_0x2c;         // minimum
    float field_0x30;         // maximum
    float field_0x34;         // centre + dead zone
    float field_0x38;         // centre - dead zone
};

// 20-byte input event queued by 0x0043cea0.
struct UnknownControlEvent {
    int control;
    int kind;
    int device;
    int pressed;
    int field_0x10;           // always 0x3f
};

// Mapping table at ControlInterface+0xcbc (installed by 0x0043ce70).
class UnknownControlMapping {
public:
    // 0x0043cbd0: control `control` of device `device`, or -1.
    void UnknownFunction43cbd0(int control, int* result, int device);
    // 0x0043cba0: the keyboard key for `control`, or -1.
    void UnknownFunction43cba0(int control, int* result);
};

// RTTI: ControlInterface (root; PCControlInterface derives from it). Slot 0
// is the deleting destructor and slots 1-4 are _purecall. The global object
// at 0x0056e26c holds one at +0x14: the input readers queue events through
// 0x0043cea0 on it and read its +0xcbc mapping, members this constructor
// initialises. Names are provisional.
class ControlInterface {
public:
    ControlInterface();                    // 0x0043ce00
    virtual ~ControlInterface();           // 0x0043ce60 (deleting wrapper 0x0043ce40)
    virtual void UnknownVirtualSlot1() = 0;
    virtual int UnknownVirtualSlot2(int a, int b) = 0;
    virtual void UnknownVirtualSlot3() = 0;
    virtual void UnknownVirtualSlot4() = 0;

    int UnknownFunction43ce70(UnknownControlMapping* mapping);           // 0x0043ce70
    void UnknownFunction43cea0(int control, int kind, int pressed, int device); // 0x0043cea0

    int field_0x04;
    int field_0x08;
    int field_0x0c;
    InputDevice* field_0x10[8];            // joysticks
    InputDevice* field_0x30;
    UnknownInterface56e26c* field_0x34;    // keyboard (see above)
    int field_0x38;                        // queued events
    UnknownControlEvent field_0x3c[160];
    UnknownControlMapping* field_0xcbc;
};

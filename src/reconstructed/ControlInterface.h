#pragma once

class InputDevice;
class KeyboardDevice;
class JoystickDevice;
class MouseDevice;

// FollowCamera's view of the keyboard at ControlInterface+0x34. FollowCamera
// slots 55 and 56 call slot 5 with 0x38 and 0x2a (DirectInput left Alt and
// left Shift) and return its result unconverted as bool (KrustyBikeCamera's
// override of slot 56 is bool), while KeyboardDevice's own callers test the
// full eax (int). The two declarations cannot be one type, so FollowCamera
// reads the member through this view.
class UnknownKeyboardBoolView {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual bool UnknownVirtualSlot5(int key, int modifier, void* entry);
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
    int modifiers;            // 0x3f when queued; the keyboard's state when dispatched
};

// Mapping table at ControlInterface+0xcbc (installed by 0x0043ce70): the
// keyboard key, joystick button (per joystick) and mouse button for each of
// 50 controls, -1 when unmapped.
class UnknownControlMapping {
public:
    UnknownControlMapping();                                       // 0x0043cae0
    void UnknownFunction43caf0();                                  // 0x0043caf0: clear
    void UnknownFunction43cb30(int control, int key);              // 0x0043cb30
    void UnknownFunction43cb50(int control, int button, int device); // 0x0043cb50
    void UnknownFunction43cb80(int control, int button);           // 0x0043cb80
    void UnknownFunction43cba0(int control, int* result);          // 0x0043cba0
    void UnknownFunction43cbd0(int control, int* result, int device); // 0x0043cbd0
    void UnknownFunction43cc10(int control, int* result);          // 0x0043cc10

    int keys[50];
    int joystickButtons[8][50];
    int mouseButtons[50];
};

// cdecl 0x0043caa0: whether `event` is control `control` of device kind
// `kind` with `modifier` held (when there is a keyboard).
int UnknownFunction43caa0(int control, int kind, const UnknownControlEvent* event, int modifier);

// cdecl 0x0043d080: maps the 2-bit field `index` of byte `byteIndex` of
// `bits` to 0x200, 0x40, 0x80 or 0x100.
int UnknownFunction43d080(int bits, int index, int byteIndex);

// RTTI: ControlInterface (root; PCControlInterface derives from it). Slot 0
// is the deleting destructor and slots 1-4 are _purecall. The global object
// at 0x0056e26c holds one at +0x14: the input readers queue events through
// 0x0043cea0 on it and read its +0xcbc mapping, members this constructor
// initialises. Names are provisional.
class ControlInterface {
public:
    ControlInterface();                    // 0x0043ce00
    virtual ~ControlInterface();           // 0x0043ce60 (deleting wrapper 0x0043ce40)
    virtual int UnknownVirtualSlot1() = 0;                       // set up the devices
    virtual int UnknownVirtualSlot2(int control, int modifier) = 0; // control state
    virtual int UnknownVirtualSlot3(int control, int kind, int modifier, int device) = 0;
    virtual int UnknownVirtualSlot4(int modifier) = 0;           // modifier test

    int UnknownFunction43ce70(UnknownControlMapping* newMapping);           // 0x0043ce70
    void UnknownFunction43cea0(int control, int kind, int pressed, int device); // 0x0043cea0
    int UnknownFunction43cf00(int value);   // 0x0043cf00, near miss in samples/control

    int joystickCount;                        // joystick count
    int activeJoystickIndex;                        // active joystick index
    JoystickDevice* activeJoystick;            // active joystick
    JoystickDevice* joysticks[8];
    MouseDevice* mouse;
    KeyboardDevice* keyboard;    // keyboard (see above)
    int queuedEventCount;                        // queued events
    UnknownControlEvent events[160];
    UnknownControlMapping* mapping;
    struct UnknownDirectInput* directInput;
};

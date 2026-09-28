// PCKeyboardDevice is confirmed by RTTI. Vtable slot 3 shares the one-byte
// no-op body at 0x00464e90. PCControl.cpp exists in the recovered source list;
// exact translation-unit ownership of this virtual is still provisional.
class PCKeyboardDevice {
public:
    virtual void UnknownVirtualSlot3();
};

void PCKeyboardDevice::UnknownVirtualSlot3() {
}

// RTTI confirms UIControl and its derived UI classes. These virtuals are shared
// by the UIControl family and have unambiguous retail bodies.
//
// Semantic field/method names are not known yet, so the probe uses byte offsets
// rather than inventing members. Slot numbers are evidence-backed by the retail
// vtable and intentionally remain in the provisional method names.
class UIControl {
public:
    virtual int UnknownVirtualSlot34();
    virtual int UnknownVirtualSlot35();
    virtual int UnknownVirtualSlot36();
    virtual void* UnknownVirtualSlot37();
    virtual void UnknownVirtualSlot52(int value);
    virtual void UnknownVirtualSlot54(int value);
    virtual void UnknownVirtualSlot50();
    virtual int UnknownVirtualSlot61();
    virtual int UnknownVirtualSlot62();
};

int UIControl::UnknownVirtualSlot34() {
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0xC0);
}
int UIControl::UnknownVirtualSlot35() {
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0xD0);
}
int UIControl::UnknownVirtualSlot36() {
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0xE4);
}
void* UIControl::UnknownVirtualSlot37() {
    return reinterpret_cast<char*>(this) + 0xDC;
}
void UIControl::UnknownVirtualSlot52(int value) {
    *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x7C) = value;
}
void UIControl::UnknownVirtualSlot54(int value) {
    *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x1B4) = value;
}

void UIControl::UnknownVirtualSlot50() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x1C0) = 0;
    *reinterpret_cast<unsigned int*>(p + 0x1BC) = 3;
}

// These two are semantically obvious but intentionally *not* smoke targets yet:
// VC6 uses a two-register subtraction shape while clang folds the RHS into the
// subtract instruction. They are useful compiler-calibration candidates.
int UIControl::UnknownVirtualSlot61() {
    char* p = reinterpret_cast<char*>(this);
    return *reinterpret_cast<int*>(p + 0x34) - *reinterpret_cast<int*>(p + 0x2C);
}
int UIControl::UnknownVirtualSlot62() {
    char* p = reinterpret_cast<char*>(this);
    return *reinterpret_cast<int*>(p + 0x38) - *reinterpret_cast<int*>(p + 0x30);
}

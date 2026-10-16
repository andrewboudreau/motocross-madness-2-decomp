// RTTI confirms UIControl and its derived UI classes. These virtuals are shared
// by the UIControl family and have unambiguous retail bodies.
//
// The method names are the canonical ones from src/reconstructed/GameUi.h, where
// the same slots (34-37, 50, 52, 54, 61, 62) are reconstructed with named members.
// The probe keeps byte offsets rather than members.
class UIControl {
public:
    virtual int GetControlText();
    virtual int GetTextLength();
    virtual int GetTextLines();
    virtual void* GetTextExtent();
    virtual void SetAttachId(int value);
    virtual void BindValue(int value);
    virtual void RequestRedraw();
    virtual int GetWidth();
    virtual int GetHeight();
};

int UIControl::GetControlText() {
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0xC0);
}
int UIControl::GetTextLength() {
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0xD0);
}
int UIControl::GetTextLines() {
    return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0xE4);
}
void* UIControl::GetTextExtent() {
    return reinterpret_cast<char*>(this) + 0xDC;
}
void UIControl::SetAttachId(int value) {
    *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x7C) = value;
}
void UIControl::BindValue(int value) {
    *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x1B4) = value;
}

void UIControl::RequestRedraw() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x1C0) = 0;
    *reinterpret_cast<unsigned int*>(p + 0x1BC) = 3;
}

// These two are semantically obvious but intentionally *not* smoke targets yet:
// VC6 uses a two-register subtraction shape while clang folds the RHS into the
// subtract instruction. They are useful compiler-calibration candidates.
int UIControl::GetWidth() {
    char* p = reinterpret_cast<char*>(this);
    return *reinterpret_cast<int*>(p + 0x34) - *reinterpret_cast<int*>(p + 0x2C);
}
int UIControl::GetHeight() {
    char* p = reinterpret_cast<char*>(this);
    return *reinterpret_cast<int*>(p + 0x38) - *reinterpret_cast<int*>(p + 0x30);
}

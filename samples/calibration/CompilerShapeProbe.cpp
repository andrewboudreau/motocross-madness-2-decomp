// Compiler-shape probes: semantics are strongly indicated by the retail bodies,
// but modern clang intentionally emits different instruction encodings/shapes.
// These are useful for deciding whether a VC6 profile reproduces the retail code.
class UIControl {
public:
    virtual int GetWidth();
    virtual int GetHeight();
};
int UIControl::GetWidth() {
    char* p = reinterpret_cast<char*>(this);
    return *reinterpret_cast<int*>(p + 0x34) - *reinterpret_cast<int*>(p + 0x2C);
}
int UIControl::GetHeight() {
    char* p = reinterpret_cast<char*>(this);
    return *reinterpret_cast<int*>(p + 0x38) - *reinterpret_cast<int*>(p + 0x30);
}

class UIStatic {
public:
    virtual int IsInteractive();
};
int UIStatic::IsInteractive() {
    return 0;
}

// UIMultiState indexed element access. Retail VC6 emits the effective address
// with SIB base/index ordering opposite clang's equivalent encoding. These
// methods are valuable because they expose a 32-byte element stride and two
// object-layout fields (+0x1F0 index, +0x1F4 base pointer).
class UIMultiState {
public:
    virtual int GetControlText();
    virtual int GetTextLength();
    virtual int GetTextLines();
    virtual void* GetTextExtent();
};
int UIMultiState::GetControlText() {
    char* p = reinterpret_cast<char*>(this);
    int i = *reinterpret_cast<int*>(p + 0x1F0);
    char* q = *reinterpret_cast<char**>(p + 0x1F4);
    return *reinterpret_cast<int*>(q + i * 32 + 8);
}
int UIMultiState::GetTextLength() {
    char* p = reinterpret_cast<char*>(this);
    int i = *reinterpret_cast<int*>(p + 0x1F0);
    char* q = *reinterpret_cast<char**>(p + 0x1F4);
    return *reinterpret_cast<int*>(q + i * 32 + 12);
}
int UIMultiState::GetTextLines() {
    char* p = reinterpret_cast<char*>(this);
    int i = *reinterpret_cast<int*>(p + 0x1F0);
    char* q = *reinterpret_cast<char**>(p + 0x1F4);
    return *reinterpret_cast<int*>(q + i * 32 + 28);
}
void* UIMultiState::GetTextExtent() {
    char* p = reinterpret_cast<char*>(this);
    int i = *reinterpret_cast<int*>(p + 0x1F0);
    char* q = *reinterpret_cast<char**>(p + 0x1F4);
    return q + i * 32 + 20;
}

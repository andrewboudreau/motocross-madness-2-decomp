#pragma once

// GUI page and control objects (their code sits among gameui.cpp's
// literals). Kept out of KrustyUI.h: declaring them there changed VC6's
// register choice in TrackGame slot 1 (see docs/TRACKGAME.md).

// A control found by name (its +0x1f0 is a progress bar's value).
class UnknownGameUiControl {
public:
    void UnknownFunction47b370(int value);    // 0x0047b370
    void UnknownFunction470b20(const char* text); // 0x00470b20 (TrackRecord.cpp)
    void UnknownFunction476d80(const char* text, int a, int b); // 0x00476d80: adds a list row
    void UnknownFunction4775f0();             // 0x004775f0
    void UnknownFunction477900(int a);        // 0x00477900
    void UnknownFunction477bb0(int a);        // 0x00477bb0
    void UnknownFunction479310(int a);        // 0x00479310
    void UnknownFunction478a50(int index, void* module, int id); // 0x00478a50: a state's resource string
    void UnknownFunction478ad0(int index, const char* text);    // 0x00478ad0: a state's text

    unsigned char field_0x000[0x1f0];
    int field_0x1f0;
};

// Page object at KrustyUI+0x490.
class UnknownGameUiPage {
public:
    UnknownGameUiControl* UnknownFunction46ebf0(const char* name, int flags); // 0x0046ebf0
};

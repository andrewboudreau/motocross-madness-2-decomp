#pragma once

// GUI page and control objects (their code sits among gameui.cpp's
// literals). Kept out of KrustyUI.h: declaring them there changed VC6's
// register choice in TrackGame slot 1 (see docs/TRACKGAME.md).

// A control found by name (its +0x1f0 is a progress bar's value).
class UnknownGameUiControl {
public:
    void UnknownFunction47b370(int value);    // 0x0047b370

    unsigned char field_0x000[0x1f0];
    int field_0x1f0;
};

// Page object at KrustyUI+0x490.
class UnknownGameUiPage {
public:
    UnknownGameUiControl* UnknownFunction46ebf0(const char* name, int flags); // 0x0046ebf0
};

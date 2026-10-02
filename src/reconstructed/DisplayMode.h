#pragma once

// 36-byte display-mode record. Tables of these sit at +0x10 of the display
// object, with the current index at +0x0c: Camera 0x0042e550 reads the
// current mode's size through RenderTarget+0x04, and Game slot 8 prints
// "%d x %d %d bit" from the object at Game+0x0c.
struct UnknownDisplayMode {
    int width;
    int height;
    int bitDepth;
    int refreshRate;          // the field PCGame slot 34's duplicate test ignores
    int field_0x10;
    int field_0x14;           // usable; PCGame slot 34 and 0x004c0760 clear it
    int field_0x18;           // usable; PCGame slot 34 clears it
    int field_0x1c;           // PCGame slot 31 copies it to Display+0x58
    int field_0x20;           // and this to Display+0x5c
};

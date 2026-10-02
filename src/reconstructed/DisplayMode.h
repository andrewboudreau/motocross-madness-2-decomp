#pragma once

// 36-byte display-mode record. Tables of these sit at +0x10 of the display
// object, with the current index at +0x0c: Camera 0x0042e550 reads the
// current mode's size through RenderTarget+0x04, and Game slot 8 prints
// "%d x %d %d bit" from the object at Game+0x0c.
struct UnknownDisplayMode {
    int width;
    int height;
    int bitDepth;
    int field_0x0c[6];
};

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
    int hardwareUsable;       // +0x14 listed for the Direct3D renderer (OptGraphicsDlg, ChooseDisplayMode); FilterDisplayModes and 0x004c0760 clear it
    int bladeUsable;          // +0x18 listed when bladeRenderer is set; FilterDisplayModes clears it when two surfaces exceed video memory
    int textureMemory;        // +0x1c PCGame slot 31 copies it to Display textureMemory (+0x58)
    int nonLocalTextureMemory; // +0x20 and this to nonLocalTextureMemory (+0x5c)
};

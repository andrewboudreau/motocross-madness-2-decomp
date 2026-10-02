// Near-miss PCGame candidates, kept out of src/reconstructed until they
// match. See docs/PCGAME.md.
//
// PCGame::UnknownFunction4c16f0 (0x004c16f0, 771 bytes): 767 of 771 bytes
// match. The four stores in the profile copy loop encode their address as
// [mode table + offset] where retail has [offset + mode table] (SIB base and
// index swapped). Pointer-walk, `(table + i)->`, reference and 16-byte
// struct-copy forms do not change it.
#include <stdio.h>

#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/PCGame.h"

// 0x004c16f0: loads the display's saved profile. Fails (0) unless the saved
// mode list matches the display's; marks the display disabled (+0xb74 bit 1)
// when the profile says so.
int PCGame::UnknownFunction4c16f0(UnknownDisplay* display) {
    char name[256];
    sprintf(name, "DriverInfo\\%s\\TextureCacheLimit", display->field_0x4bc);
    display->field_0x60 = UnknownVirtualSlot20(name, 0);
    if (display->field_0x60 <= 0)
        display->field_0x60 = 0x7fffffff;
    sprintf(name, "DriverInfo\\%s\\TotalVideoMemory", display->field_0x4bc);
    display->field_0x54 = UnknownVirtualSlot20(name, 0);
    sprintf(name, "DriverInfo\\%s\\NumberOfModes", display->field_0x4bc);
    int count = UnknownVirtualSlot20(name, 0);
    if (count != display->field_0x08)
        return 0;
    unsigned long size = count * sizeof(UnknownDisplayMode);
    sprintf(name, "DriverInfo\\%s\\Modes", display->field_0x4bc);
    UnknownDisplayMode* saved = (UnknownDisplayMode*)DebugMalloc(size, __FILE__, 1726);
    if (!saved)
        return 0;
    UnknownVirtualSlot24(name, saved, &size);
    int i;
    for (i = 0; i < count; i++) {
        if (display->field_0x10[i].width != saved[i].width ||
            display->field_0x10[i].height != saved[i].height ||
            display->field_0x10[i].bitDepth != saved[i].bitDepth ||
            display->field_0x10[i].refreshRate != saved[i].refreshRate)
            break;
    }
    if (i != count) {
        operator delete(saved, __FILE__, 1844);
        return 0;
    }
    for (i = 0; i < count; i++) {
        display->field_0x10[i].field_0x14 = saved[i].field_0x14;
        display->field_0x10[i].field_0x18 = saved[i].field_0x18;
        display->field_0x10[i].field_0x1c = saved[i].field_0x1c;
        display->field_0x10[i].field_0x20 = saved[i].field_0x20;
    }
    sprintf(name, "DriverInfo\\%s\\PartialTextureBlt", display->field_0x4bc);
    display->field_0x5bc = UnknownVirtualSlot20(name, 0);
    sprintf(name, "DriverInfo\\%s\\Use8BitTextures", display->field_0x4bc);
    display->field_0x70_bit0 = UnknownVirtualSlot22(name, 1);
    sprintf(name, "DriverInfo\\%s\\IsAGP", display->field_0x4bc);
    display->field_0x9f0 = UnknownVirtualSlot22(name, 0);
    if (field_0x2d4_bit1)
        sprintf(name, "DriverInfo\\%s\\DisabledFullScreen", display->field_0x4bc);
    else
        sprintf(name, "DriverInfo\\%s\\DisabledWindowed", display->field_0x4bc);
    if (UnknownVirtualSlot22(name, 0)) {
        display->field_0xb74_bit1 = 1;
        return 1;
    }
    sprintf(name, "DriverInfo\\%s\\DisabledHardware", display->field_0x4bc);
    if (UnknownVirtualSlot22(name, 0)) {
        display->field_0xb74_bit1 = 1;
        return 1;
    }
    operator delete(saved, __FILE__, 1841);
    return 1;
}

// Near-miss SoultreeMaterial.cpp candidate, kept out of src/reconstructed
// until it matches. See src/reconstructed/SoultreeMaterial.h.
//
// 0x004ff180 (652 bytes including its 6-entry jump table at 0x004ff3f4):
// 650/652 after relocation. Only the vtable register of the last switch case
// (mapping type 8, render state 0x80) differs: retail uses edx, VC6 here
// continues its eax/edx alternation and picks eax. Case order permutations,
// return-versus-break, default labels, explicit empty cases 5/7, a local
// target pointer and a PCRenderTarget cast do not change it.
#include "../../src/reconstructed/SoultreeMaterial.h"

// 0x00689ed0 (defined in SoultreeMaterial.cpp)
extern int g_UnknownInt689ed0;

// 0x004ff180
void SoultreeMaterial::UnknownFunction4ff180()
{
    if (field_0x70) {
        field_0x70->Select();
        int format = field_0x70->texture->field_0x20;
        if (format != 0x613 && format != 0x115c && format != 0x22b8) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0xf, 0, 0);
        } else if (field_0xa8) {
            if (field_0xbc)
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot18(0);
            else
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot18(0x80);
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot18(0);
        }
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 4);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 2, 2);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 3, 0);
        if (format == 0x115c || format == 0x22b8 || (format == 0x613 && field_0xbc)) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 1, 0);
            if (!field_0xbc && field_0xc8 != 6) {
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 2);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 5, 2);
            } else {
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 4);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 5, 2);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 6, 0);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 1, 0);
            }
        } else if (field_0xbc) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 1, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 2);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 6, 0);
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 0, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 2);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 5, 2);
        }
    } else {
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot11(0);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 1, 1);
        if (field_0xbc) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 1, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 2);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 5, 0);
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 0, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 4, 1);
        }
    }
    if (field_0xb4) {
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot6(0, 0xc, &g_UnknownInt689ed0);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, 0xc, 3);
    }
    switch (field_0xc8) {
    case 3:
    case 6:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 1, 0);
        break;
    case 4:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x1b, 1, 0);
        break;
    case 8:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(0x80, 1, 0);
        break;
    }
}

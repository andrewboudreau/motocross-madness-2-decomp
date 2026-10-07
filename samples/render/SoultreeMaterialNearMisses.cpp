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
#include "../../src/reconstructed/D3DConstants.h"

// 0x00689ed0 (defined in SoultreeMaterial.cpp)
extern int g_UnknownInt689ed0;

// 0x004ff180
void SoultreeMaterial::ApplyRenderStates()
{
    if (field_0x70) {
        field_0x70->Select();
        int format = field_0x70->texture->field_0x20;
        if (format != 1555 && format != 4444 && format != 8888) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHATESTENABLE, 0, 0);
        } else if (hasColorKey) {
            if (hasAlpha)
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot18(0);
            else
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot18(0x80);
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot18(0);
        }
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        if (format == 4444 || format == 8888 || (format == 1555 && hasAlpha)) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            if (!hasAlpha && mappingType != 6) {
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            } else {
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            }
        } else if (hasAlpha) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        }
    } else {
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot11(0);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
        if (hasAlpha) {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        } else {
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 0, 0);
            ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
    }
    if (clampTexture) {
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot6(0, 0xc, &g_UnknownInt689ed0);
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot7(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
    }
    switch (mappingType) {
    case 3:
    case 6:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        break;
    case 4:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_ALPHABLENDENABLE, 1, 0);
        break;
    case 8:
        ((RenderTarget*)field_0x18)->UnknownVirtualSlot8(D3DRENDERSTATE_WRAP0, 1, 0);
        break;
    }
}

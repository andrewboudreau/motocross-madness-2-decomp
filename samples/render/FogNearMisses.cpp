// Near-miss Fog.cpp candidates (src/reconstructed/Fog.cpp, Fog.h), kept out of
// src/reconstructed until they match.
//
// Fog slot 14 (0x00462850, 965 bytes): calls, arguments, the backdrop vertices
// and the render-state sequence match. Retail reaches one shared `return 0`
// block (with `xor eax, eax`) from both the camera-projection failure and the
// draw failure; VC6 here keeps eax from the failed call and emits two blocks,
// which shifts every later byte. Early return, nested else, a result flag and
// `== 0` forms do not merge them.

#include "../../src/reconstructed/Fog.h"

#include "../../src/reconstructed/Camera.h"
#include "../../src/reconstructed/Display.h"
#include "../../src/reconstructed/PCRenderTarget.h"
#include "../../src/reconstructed/D3DConstants.h"

#define TARGET() ((PCRenderTarget*)field_0x18)
#define VIEWPORT(i) ((unsigned int)TARGET()->field_0x08->field_0x1a0[i])

// A D3DFVF_TLVERTEX (screen-space backdrop corner).
struct UnknownFogVertex {
    float x;
    float y;
    float z;
    float rhw;
    unsigned int color;
    unsigned int specular;
    float tu;
    float tv;
};

// 0x00462850
int Fog::UnknownVirtualSlot14()
{
    if (field_0x25_bit0) {
        TARGET()->field_0x08->UnknownFunction42e960(1.0f, fogEnd);
        TARGET()->field_0x08->UnknownVirtualSlot28();
        if (!TARGET()->field_0x08->UnknownVirtualSlot32(&TARGET()->field_0x08->projectionMatrix)) {
            return 0;
        }
        if (!drawnByFogOn && TARGET()->field_0x04->isPowerVR
            && (TARGET()->field_0x34 || TARGET()->fillMode == D3DFILL_WIREFRAME)) {
            UnknownFogVertex vertices[4];
            vertices[0].x = (float)VIEWPORT(0);
            vertices[0].y = (float)VIEWPORT(1);
            vertices[0].z = 0.999999f;
            vertices[0].rhw = 1.0f;
            vertices[0].color = field_0x2c;
            vertices[0].specular = 0;
            vertices[1].x = (float)VIEWPORT(2) + (float)VIEWPORT(0);
            vertices[1].y = (float)VIEWPORT(1);
            vertices[1].z = 0.999999f;
            vertices[1].rhw = 1.0f;
            vertices[1].color = field_0x2c;
            vertices[1].specular = 0;
            vertices[2].x = (float)VIEWPORT(2) + (float)VIEWPORT(0);
            vertices[2].y = (float)VIEWPORT(3) + (float)VIEWPORT(1);
            vertices[2].z = 0.999999f;
            vertices[2].rhw = 1.0f;
            vertices[2].color = field_0x2c;
            vertices[2].specular = 0;
            vertices[3].x = (float)VIEWPORT(0);
            vertices[3].y = (float)VIEWPORT(3) + (float)VIEWPORT(1);
            vertices[3].z = 0.999999f;
            vertices[3].rhw = 1.0f;
            vertices[3].color = field_0x2c;
            vertices[3].specular = 0;
            TARGET()->UnknownVirtualSlot7(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
            TARGET()->UnknownVirtualSlot7(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            if (TARGET()->fillMode != D3DFILL_SOLID) {
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID, 0);
            }
            if (!TARGET()->UnknownVirtualSlot16(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, (int)vertices, 4, 0)) {
                return 0;
            }
            if (TARGET()->fillMode != D3DFILL_SOLID) {
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FILLMODE, TARGET()->fillMode, 0);
            }
        }
        if (renderFog) {
            float density = 1.0f;
            TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGENABLE, 1, 0);
            TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGCOLOR, field_0x2c, 0);
            float start = fogStart;
            float end = fogEnd;
            if (field_0x44 == D3DPRASTERCAPS_FOGTABLE) {
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGSTART, *(int*)&start, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGEND, *(int*)&end, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGDENSITY, *(int*)&density, 0);
                density = 0.22f;
                if (TARGET()->field_0x04->isPowerVR) {
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_NONE, 0);
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_EXP, 0);
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGDENSITY, *(int*)&density, 0);
                } else {
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_NONE, 0);
                    TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR, 0);
                }
                return 1;
            }
            if (field_0x44 == D3DPRASTERCAPS_FOGVERTEX) {
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGSTART, *(int*)&fogStart, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGEND, *(int*)&fogEnd, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGDENSITY, *(int*)&density, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE, 0);
                TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR, 0);
                return 1;
            }
            TARGET()->UnknownVirtualSlot8(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE, 0);
        }
    }
    return 1;
}

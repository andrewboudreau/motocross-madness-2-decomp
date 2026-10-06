#pragma once

// PCVideoCard.cpp (literal __FILE__ "D:\\aardvark\\VC\\krusty2\\PCVideoCard.cpp",
// 0x005714f4). The class itself is UnknownDisplay in Display.h (RTTI
// PCVideoCard : VideoCard). Retail code 0x004c94a0..0x004cb667; names are
// provisional.

#include "Display.h"
#include "Guid.h"

class PCRenderTarget;
class PCTextureMap;
class TextureMap;
struct UnknownRect;

// DirectX GUIDs (dxguid; the retail bytes are these interface IDs).
extern "C" const UnknownGuid IID_IDirectDraw7;               // 0x005560b0
extern "C" const UnknownGuid IID_IDirect3D7;                 // 0x00556180
extern "C" const UnknownGuid IID_IDirectDrawGammaControl;    // 0x00556140
extern "C" const UnknownGuid IID_IDirect3DHALDevice;         // 0x005561b0

typedef int(__stdcall* UnknownEnumCallback)(UnknownGuid* guid, char* description, char* name,
                                            void* context);

// ddraw.dll entry points, called through the import thunks 0x00533040 and
// 0x00533046 (DirectDrawEnumerateExA is looked up with GetProcAddress).
extern "C" long __stdcall DirectDrawEnumerateA(UnknownEnumCallback callback, void* context);
extern "C" long __stdcall DirectDrawCreateEx(UnknownGuid* guid, void** object, const UnknownGuid& iid,
                                             void* outer);

// 0x64-byte blit effects (the DDBLTFX layout).
struct UnknownBltFx {
    unsigned long size;
    unsigned char field_0x04[0x50 - 0x04];
    unsigned long fillColor;
    unsigned char field_0x54[0x64 - 0x54];
};

// 0x00570570: the gamma ramp set when the card supports one (zero
// initialised .data, only referenced by 0x004c9f30).
extern unsigned short g_UnknownGammaRamp570570[3][256];

// cdecl helpers after wrecker.cpp's code, called around cooperative level
// changes when PCGame+0x538 (IMM32.DLL) is loaded.
void UnknownFunction52ff00();
void UnknownFunction52ff20();

// 0x004c9600: enumerates the DirectDraw devices for `window` into
// g_UnknownDisplays68a754; returns their count.
int UnknownFunction4c9600(void* window);

// 0x004ca9f0: copies the target's surface into the texture; 0 on failure.
int UnknownFunction4ca9f0(PCRenderTarget* target, TextureMap* texture);

// 0x004cb330: the mean absolute channel difference (1/256 units) of two
// 16-bit textures, or -1 when one cannot be locked (near miss,
// samples/render/PCVideoCardNearMisses.cpp).
float UnknownFunction4cb330(TextureMap* first, TextureMap* second);

// 0x004caf70: one PartialTexBlt pass over `area` (see PCVideoCard.cpp).
int UnknownFunction4caf70(PCRenderTarget* target, UnknownRect* area, PCTextureMap* image,
                          PCTextureMap* expected, PCTextureMap* rendered, UnknownDisplay* display,
                          int present);

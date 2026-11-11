// PixelBlend.cpp -- three of the four cdecl pixel helpers at 0x0043b320..0x0043b7f9 that
// TextService.cpp's font drawing calls (declared in TextService.h; names provisional).
//
// Owner TU unknown: the helpers sit between CollisionPoint.cpp (0x0043a1e0..0x0043b320,
// __FILE__ xref 0x0043a347) and ConstraintMethodCollisionModel.cpp (contact callback
// 0x0043b800, __FILE__ xref 0x0043c7f9) with no literal, initialiser or RTTI tying them to
// either, so this file has a neutral name, not a retail TU path.  Roles are decoded from
// instruction behaviour (tier 1); names are tier 3.  Retail calls them from outside the
// unit (0x0050b338, 0x0050b637 and 0x0050b937 call the three blends).  Each is strict exact
// with relocations resolved (PixelBlend.bindings.json).  Constants: 0x5511e0 = 1/15,
// 0x5511e4 = 1/255.
// Not here: the fourth helper, ConvertColorToFormat 0x0043b660, which is strict exact only
// with an unsigned short return (samples/physics/constraint/PixelBlend.cpp, PackColor),
// while TextService.h declares it returning unsigned int for DrawTextInto, whose format
// dispatch matches retail that way; the stand-in in src/bringup/Group2_OverlayText.cpp
// supplies it until the two declarations are reconciled.
// Blend: dest = texel*t + dest*(1-t) per channel, in place (retail writes the blended bytes
// back over the dest array), where t is the texel's alpha; a nonzero `color` replaces the
// texel's colour channels (keeping its alpha).
#include "TextService.h"

static const float kInv15  = 1.0f / 15.0f;
static const float kInv255 = 1.0f / 255.0f;

// 0x0043b320: ARGB4444 blend.
unsigned short BlendFontTexel4444(unsigned short texel, unsigned short dest, unsigned short color)
{
    unsigned char A[4], B[4];
    A[0] = (unsigned char)((texel >> 12) & 0xf); A[1] = (unsigned char)((texel >> 8) & 0xf);
    A[2] = (unsigned char)((texel >> 4) & 0xf);  A[3] = (unsigned char)(texel & 0xf);
    B[0] = (unsigned char)((dest >> 12) & 0xf);  B[1] = (unsigned char)((dest >> 8) & 0xf);
    B[2] = (unsigned char)((dest >> 4) & 0xf);   B[3] = (unsigned char)(dest & 0xf);
    float t = A[0] * kInv15;
    if (color) {
        A[1] = (unsigned char)((color >> 8) & 0xf);
        A[2] = (unsigned char)((color >> 4) & 0xf);
        A[3] = (unsigned char)(color & 0xf);
    }
    for (unsigned int i = 0; i < 4; i++)
        B[i] = (unsigned char)(A[i] * t + B[i] * (1.0f - t));
    return (unsigned short)((((((B[0] << 4) | B[1]) << 4) | B[2]) << 4) | B[3]);
}

// 0x0043b440: ARGB1555 blend (1-bit alpha, used unscaled; 5-bit colour channels).
unsigned short BlendFontTexel1555(unsigned short texel, unsigned short dest, unsigned short color)
{
    unsigned char A[4], B[4];
    A[0] = (unsigned char)((texel >> 15) & 1);    A[1] = (unsigned char)((texel >> 10) & 0x1f);
    A[2] = (unsigned char)((texel >> 5) & 0x1f);  A[3] = (unsigned char)(texel & 0x1f);
    B[0] = (unsigned char)((dest >> 15) & 1);     B[1] = (unsigned char)((dest >> 10) & 0x1f);
    B[2] = (unsigned char)((dest >> 5) & 0x1f);   B[3] = (unsigned char)(dest & 0x1f);
    float t = (float)A[0];
    if (color) {
        A[1] = (unsigned char)((color >> 10) & 0x1f);
        A[2] = (unsigned char)((color >> 5) & 0x1f);
        A[3] = (unsigned char)(color & 0x1f);
    }
    for (unsigned int i = 0; i < 4; i++)
        B[i] = (unsigned char)(A[i] * t + B[i] * (1.0f - t));
    return (unsigned short)((((((B[0] << 5) | B[1]) << 5) | B[2]) << 5) | B[3]);
}

// 0x0043b560: ARGB8888 blend.
unsigned int BlendFontTexel32(unsigned int texel, unsigned int dest, unsigned int color)
{
    unsigned char A[4], B[4];
    A[0] = (unsigned char)(texel >> 24); A[1] = (unsigned char)(texel >> 16);
    A[2] = (unsigned char)(texel >> 8);  A[3] = (unsigned char)texel;
    B[0] = (unsigned char)(dest >> 24);  B[1] = (unsigned char)(dest >> 16);
    B[2] = (unsigned char)(dest >> 8);   B[3] = (unsigned char)dest;
    float t = A[0] * kInv255;
    if (color) {
        A[1] = (unsigned char)(color >> 16);
        A[2] = (unsigned char)(color >> 8);
        A[3] = (unsigned char)color;
    }
    for (unsigned int i = 0; i < 4; i++)
        B[i] = (unsigned char)(A[i] * t + B[i] * (1.0f - t));
    return (((((unsigned int)B[0] << 8) | B[1]) << 8) | B[2]) << 8 | B[3];
}

// 0x0043b660, the last of the four pixel helpers that share the retail address range
// 0x0043b320..0x0043b7f9 (owner TU unknown; no literal ties them to CollisionPoint.cpp or
// ConstraintMethodCollisionModel.cpp).  The three blends before it are strict exact in
// src/reconstructed/PixelBlend.cpp.  This one is strict exact too, but only with an unsigned
// short return, while src/reconstructed/TextService.h declares it (ConvertColorToFormat)
// returning unsigned int for DrawTextInto, whose format dispatch matches retail that way
// (retail pushes the returned eax unextended at 0x0050b0d3 and 0x0050b111); it stays here
// until the two declarations are reconciled.  Role decoded from instruction behaviour
// (tier 1); the name is tier 3.  Called from 0x0050b0bd and 0x0050b0fb.
// Constants: 0x5511e4 = 1/255, 0x5511e8 = 31.0.

static const float kInv255 = 1.0f / 255.0f;

// 0x0043b660: pack an ARGB8888 colour for a surface format (0x115c = 4444, 0x613 = 1555).
unsigned short PackColor(unsigned int argb, int format)
{
    // Alpha, red and green share a 3-byte stack array in retail ([esp+8..0xa]); blue
    // stays in a register (tier 1 layout, the array shape is what reproduces it).
    unsigned char c[3];
    c[0] = (unsigned char)(argb >> 24);
    c[1] = (unsigned char)(argb >> 16);
    c[2] = (unsigned char)(argb >> 8);
    unsigned char b = (unsigned char)argb;
    if (format == 0x115c) {
        unsigned int A = (unsigned char)(c[0] * kInv255 * 15.0f);
        unsigned int R = (unsigned char)(c[1] * kInv255 * 15.0f);
        unsigned int G = (unsigned char)(c[2] * kInv255 * 15.0f);
        unsigned int B = (unsigned char)(b * kInv255 * 15.0f);
        return (unsigned short)((((((A << 4) | R) << 4) | G) << 4) | B);
    }
    if (format == 0x613) {
        unsigned int R = (unsigned char)(c[1] * kInv255 * 31.0f);
        unsigned char A = c[0] > 0x7f;
        unsigned int G = (unsigned char)(c[2] * kInv255 * 31.0f);
        unsigned int B = (unsigned char)(b * kInv255 * 31.0f);
        return (unsigned short)((((((A << 5) | R) << 5) | G) << 5) | B);
    }
    return 0;
}

// Pixel helpers that share the retail TU with the constraint code
// (0x0043b320..0x0043b7f9). Roles are decoded from instruction behaviour (tier 1);
// names are tier 3. They have external linkage: retail calls them from outside this
// TU (0x0050b0bd and 0x0050b0fb call PackColor; 0x0050b338, 0x0050b637 and 0x0050b937
// call the three blends).
// Constants: 0x5511e0 = 1/15, 0x5511e4 = 1/255, 0x5511e8 = 31.0.
// Blend: B = A*t + B*(1-t) per channel, in place (the retail result bytes are written
// back over B), where t is the alpha of A (unless a third argument replaces A's colour
// channels).

static const float kInv15  = 1.0f / 15.0f;
static const float kInv255 = 1.0f / 255.0f;

// 0x0043b320: ARGB4444 blend.
unsigned short BlendArgb4444(unsigned short a, unsigned short b, unsigned short colorOverride)
{
    unsigned char A[4], B[4];
    A[0] = (unsigned char)((a >> 12) & 0xf); A[1] = (unsigned char)((a >> 8) & 0xf);
    A[2] = (unsigned char)((a >> 4) & 0xf);  A[3] = (unsigned char)(a & 0xf);
    B[0] = (unsigned char)((b >> 12) & 0xf); B[1] = (unsigned char)((b >> 8) & 0xf);
    B[2] = (unsigned char)((b >> 4) & 0xf);  B[3] = (unsigned char)(b & 0xf);
    float t = A[0] * kInv15;
    if (colorOverride) {
        A[1] = (unsigned char)((colorOverride >> 8) & 0xf);
        A[2] = (unsigned char)((colorOverride >> 4) & 0xf);
        A[3] = (unsigned char)(colorOverride & 0xf);
    }
    for (unsigned int i = 0; i < 4; i++)
        B[i] = (unsigned char)(A[i] * t + B[i] * (1.0f - t));
    return (unsigned short)((((((B[0] << 4) | B[1]) << 4) | B[2]) << 4) | B[3]);
}

// 0x0043b440: ARGB1555 blend (1-bit alpha, 5-bit colour channels).
unsigned short BlendArgb1555(unsigned short a, unsigned short b, unsigned short colorOverride)
{
    unsigned char A[4], B[4];
    A[0] = (unsigned char)((a >> 15) & 1);  A[1] = (unsigned char)((a >> 10) & 0x1f);
    A[2] = (unsigned char)((a >> 5) & 0x1f); A[3] = (unsigned char)(a & 0x1f);
    B[0] = (unsigned char)((b >> 15) & 1);  B[1] = (unsigned char)((b >> 10) & 0x1f);
    B[2] = (unsigned char)((b >> 5) & 0x1f); B[3] = (unsigned char)(b & 0x1f);
    float t = (float)A[0];
    if (colorOverride) {
        A[1] = (unsigned char)((colorOverride >> 10) & 0x1f);
        A[2] = (unsigned char)((colorOverride >> 5) & 0x1f);
        A[3] = (unsigned char)(colorOverride & 0x1f);
    }
    for (unsigned int i = 0; i < 4; i++)
        B[i] = (unsigned char)(A[i] * t + B[i] * (1.0f - t));
    return (unsigned short)((((((B[0] << 5) | B[1]) << 5) | B[2]) << 5) | B[3]);
}

// 0x0043b560: ARGB8888 blend.
unsigned int BlendArgb8888(unsigned int a, unsigned int b, unsigned int colorOverride)
{
    unsigned char A[4], B[4];
    A[0] = (unsigned char)(a >> 24); A[1] = (unsigned char)(a >> 16);
    A[2] = (unsigned char)(a >> 8);  A[3] = (unsigned char)a;
    B[0] = (unsigned char)(b >> 24); B[1] = (unsigned char)(b >> 16);
    B[2] = (unsigned char)(b >> 8);  B[3] = (unsigned char)b;
    float t = A[0] * kInv255;
    if (colorOverride) {
        A[1] = (unsigned char)(colorOverride >> 16);
        A[2] = (unsigned char)(colorOverride >> 8);
        A[3] = (unsigned char)colorOverride;
    }
    for (unsigned int i = 0; i < 4; i++)
        B[i] = (unsigned char)(A[i] * t + B[i] * (1.0f - t));
    return (((((unsigned int)B[0] << 8) | B[1]) << 8) | B[2]) << 8 | B[3];
}

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

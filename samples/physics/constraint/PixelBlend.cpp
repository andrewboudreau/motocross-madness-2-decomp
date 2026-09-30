// Static pixel helpers that share the retail TU with the constraint code
// (0x0043b320..0x0043b7f9). Roles are decoded from instruction behaviour (tier 1);
// names are tier 3. Constants: 0x5511e0 = 1/15, 0x5511e4 = 1/255, 0x5511e8 = 31.0.
// Blend: out = A*t + B*(1-t) per channel, where t is the alpha of A (unless a
// third argument replaces A's colour channels).


static const float kInv15  = 1.0f / 15.0f;
static const float kInv255 = 1.0f / 255.0f;

// 0x0043b320: ARGB4444 blend.
static unsigned short BlendArgb4444(unsigned short a, unsigned short b, unsigned short colorOverride)
{
    unsigned char A[4], B[4], R[4];
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
        R[i] = (unsigned char)(A[i] * t + B[i] * (1.0f - t));
    return (unsigned short)((((((R[0] << 4) | R[1]) << 4) | R[2]) << 4) | R[3]);
}

// 0x0043b440: ARGB1555 blend (1-bit alpha, 5-bit colour channels).
static unsigned short BlendArgb1555(unsigned short a, unsigned short b, unsigned short colorOverride)
{
    unsigned char A[4], B[4], R[4];
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
        R[i] = (unsigned char)(A[i] * t + B[i] * (1.0f - t));
    return (unsigned short)((((((R[0] << 5) | R[1]) << 5) | R[2]) << 5) | R[3]);
}

// 0x0043b560: ARGB8888 blend.
static unsigned int BlendArgb8888(unsigned int a, unsigned int b, unsigned int colorOverride)
{
    unsigned char A[4], B[4], R[4];
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
        R[i] = (unsigned char)(A[i] * t + B[i] * (1.0f - t));
    return (((((unsigned int)R[0] << 8) | R[1]) << 8) | R[2]) << 8 | R[3];
}

// 0x0043b660: pack an ARGB8888 colour for a surface format (0x115c = 4444, 0x613 = 1555).
static unsigned short PackColor(unsigned int argb, int format)
{
    unsigned char a = (unsigned char)(argb >> 24);
    unsigned char r = (unsigned char)(argb >> 16);
    unsigned char g = (unsigned char)(argb >> 8);
    unsigned char b = (unsigned char)argb;
    if (format == 0x115c) {
        unsigned int A = (unsigned char)(a * kInv255 * 15.0f);
        unsigned int R = (unsigned char)(r * kInv255 * 15.0f);
        unsigned int G = (unsigned char)(g * kInv255 * 15.0f);
        unsigned int B = (unsigned char)(b * kInv255 * 15.0f);
        return (unsigned short)((((((A << 4) | R) << 4) | G) << 4) | B);
    }
    if (format == 0x613) {
        unsigned int R = (unsigned char)(r * kInv255 * 31.0f);
        unsigned int A = a > 0x7f;
        unsigned int G = (unsigned char)(g * kInv255 * 31.0f);
        unsigned int B = (unsigned char)(b * kInv255 * 31.0f);
        return (unsigned short)((((((A << 5) | R) << 5) | G) << 5) | B);
    }
    return 0;
}

// Keep the static helpers referenced so the object file emits them (retail has no
// external callers visible in this TU either; the callers are elsewhere via table/inline).
unsigned int PixelBlendKeepAlive(unsigned int a, unsigned int b, unsigned int c, int fmt)
{
    return BlendArgb4444((unsigned short)a, (unsigned short)b, (unsigned short)c)
         + BlendArgb1555((unsigned short)a, (unsigned short)b, (unsigned short)c)
         + BlendArgb8888(a, b, c) + PackColor(a, fmt);
}

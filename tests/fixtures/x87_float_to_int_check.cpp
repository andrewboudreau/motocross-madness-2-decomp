// Compares src/bringup/X87FloatToInt.h against a /QIfist FISTP oracle in all
// four x87 rounding modes, and checks the out-of-range result.
#include <float.h>
#include <stdio.h>
#include <string.h>

#include "X87FloatToInt.h"

int OracleFloatToInt(float value);

static float FromBits(unsigned bits)
{
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static unsigned g_failures;
static unsigned g_checked;

static void Check(float value)
{
    int expected = OracleFloatToInt(value);
    int actual = X87FloatToInt(value);
    ++g_checked;
    if (expected != actual && g_failures++ < 10) {
        unsigned bits;
        memcpy(&bits, &value, sizeof(bits));
        printf("mismatch %08x: fistp %d helper %d\n", bits, expected, actual);
    }
}

static void Expect(float value, int expected)
{
    int actual = X87FloatToInt(value);
    ++g_checked;
    if (expected != actual && g_failures++ < 10)
        printf("mismatch %.9g: expected %d helper %d\n", value, expected, actual);
}

int main()
{
    static const unsigned modes[4] = {_RC_NEAR, _RC_DOWN, _RC_UP, _RC_CHOP};
    unsigned saved = _control87(0, 0);
    for (int m = 0; m < 4; m++) {
        _control87(modes[m], _MCW_RC);
        unsigned bits;
        // Every magnitude below 2^31, both signs, at a stride.
        for (bits = 0; bits < 0x4f000000; bits += 251) {
            Check(FromBits(bits));
            Check(FromBits(bits | 0x80000000));
        }
        // Dense coverage of the fractional range 1/1024 .. 2^13.
        for (bits = 0x3a800000; bits < 0x46000000; bits += 13) {
            Check(FromBits(bits));
            Check(FromBits(bits | 0x80000000));
        }
        // Every exact tie k + 0.5 and every integer near the call sites' ranges.
        for (int k = -(1 << 22); k < (1 << 22); k++) {
            Check((float)k + 0.5f);
            Check((float)k);
        }
        Check(FromBits(0xcf000000));  // -2^31, in range
    }
    _control87(saved, _MCW_RC);

    // Round to nearest, ties to even, and the `v - 0.5f` call-site form.
    Expect(0.5f, 0);
    Expect(1.5f, 2);
    Expect(2.5f, 2);
    Expect(-0.5f, 0);
    Expect(-1.5f, -2);
    Expect(-2.5f, -2);
    Expect(2.0f - 0.5f, 2);
    Expect(3.0f - 0.5f, 2);
    Expect(3.75f - 0.5f, 3);
    Expect(-0.25f - 0.5f, -1);
    // Integer indefinite for out-of-range values, infinities and NaN.
    Expect(2147483648.0f, (int)0x80000000u);
    Expect(-4294967296.0f, (int)0x80000000u);
    Expect(FromBits(0x7f800000), (int)0x80000000u);
    Expect(FromBits(0xff800000), (int)0x80000000u);
    Expect(FromBits(0x7fc00000), (int)0x80000000u);
    Expect(2147483520.0f, 2147483520);

    printf("%u conversions checked, %u mismatches\n", g_checked, g_failures);
    return g_failures ? 1 : 0;
}

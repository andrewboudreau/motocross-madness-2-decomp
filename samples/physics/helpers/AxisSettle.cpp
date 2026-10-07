// 0x004cb6e0 (1058 bytes, __cdecl): per-axis settle of two accumulators against a third.  The
// function sits alone between PeakHold.cpp (0x4cb668..) and PhysicsBody (0x4cbc50..) with the
// Math3D.h vector set .CRT$XCU 220-223 (0x4cbb10.., globals 0x689970..) behind it, so it is its
// own unit (Pe..Ph; no __FILE__ string, no RTTI).  Called from KrustyBike 0x495d51 and
// SoulTreePhysics 0x50229f.  Declared in math/Math3D.h; the roles of a, b and c are tier 3.
//
// For each axis k (bit 1 << k of 'mask'), nothing happens when c[k] is zero.  Otherwise:
//  * axis free (bit clear) and a[k] == 0: a nonzero b[k] of the opposite sign to c[k] either absorbs
//    c[k] (|b| > |c|) or is zeroed;
//  * axis locked or a[k] != 0: with t = a[k] + dt*b[k] and d = dt*c[k], b[k] += c[k] unless t and d
//    have opposite signs with |t| <= |d| and t != 0, in which case a[k] and b[k] are zeroed.
// Retail writes the sum as b + c on the x axis and c + b on y and z, so the three axes are spelled
// out rather than generated from one helper.
#include "math/Math3D.h"

// Sign as -1 / +1 (zero counts as positive) and the magnitude by conditional negation: retail
// compares against 0.0f and uses fchs, not fabs.
inline int SignOf(float x)
{
    return x < 0.0f ? -1 : 1;
}

inline float Magnitude(float x)
{
    return x < 0.0f ? -x : x;
}

void UnknownAxisSettle_4cb6e0(float* a, float* b, const float* c, float dt, unsigned char mask)
{
    if (c[0] != 0.0f) {
        if (!(mask & 1) && a[0] == 0.0f) {
            if (b[0] != 0.0f && SignOf(b[0]) != SignOf(c[0])) {
                if (Magnitude(b[0]) > Magnitude(c[0]))
                    b[0] = b[0] + c[0];
                else
                    b[0] = 0.0f;
            }
        } else {
            float t = dt * b[0] + a[0];
            float d = dt * c[0];
            if (SignOf(t) == SignOf(d) || Magnitude(t) > Magnitude(d) || t == 0.0f) {
                b[0] = b[0] + c[0];
            } else {
                a[0] = 0.0f;
                b[0] = 0.0f;
            }
        }
    }
    if (c[1] != 0.0f) {
        if (!(mask & 2) && a[1] == 0.0f) {
            if (b[1] != 0.0f && SignOf(b[1]) != SignOf(c[1])) {
                if (Magnitude(b[1]) > Magnitude(c[1]))
                    b[1] = c[1] + b[1];
                else
                    b[1] = 0.0f;
            }
        } else {
            float t = dt * b[1] + a[1];
            float d = dt * c[1];
            if (SignOf(t) == SignOf(d) || Magnitude(t) > Magnitude(d) || t == 0.0f) {
                b[1] = c[1] + b[1];
            } else {
                a[1] = 0.0f;
                b[1] = 0.0f;
            }
        }
    }
    if (c[2] != 0.0f) {
        if (!(mask & 4) && a[2] == 0.0f) {
            if (b[2] != 0.0f && SignOf(b[2]) != SignOf(c[2])) {
                if (Magnitude(b[2]) > Magnitude(c[2]))
                    b[2] = c[2] + b[2];
                else
                    b[2] = 0.0f;
            }
        } else {
            float t = dt * b[2] + a[2];
            float d = dt * c[2];
            if (SignOf(t) == SignOf(d) || Magnitude(t) > Magnitude(d) || t == 0.0f) {
                b[2] = c[2] + b[2];
            } else {
                a[2] = 0.0f;
                b[2] = 0.0f;
            }
        }
    }
}

// OrientationAngles.cpp -- 0x004b5a60: Euler-style angle extraction from a pair of direction
// vectors. Translation-unit ownership is PROVISIONAL (tier 3): nearest __FILE__ references are
// OptionProcs.cpp / overlay.cpp (proximity only). Callers: 0x0040bec0 and five others.
//
// All names are tier 3, derived from the arithmetic. The routine takes a "forward" vector a
// and an "up" vector b (both by value, 3 floats each). It returns 0 if any of the first three
// pointers is null, otherwise 1.
#include <math.h>
#include "../common/Math3D.h"

static inline float AbsF(float x) { return x < 0.0f ? -x : x; }

int OrientationAnglesFromVectors(Vec3 a, Vec3 b, float* yaw, float* pitch, float* roll,
                                 float* sinRoll, float* cosRoll, float* cosPitch,
                                 float* sinPitch)
{
    if (!yaw || !pitch || !roll)
        return 0;

    // Heading of a in the x/z plane: (cos, sin) = (a.z, a.x) / |(a.z, a.x)|.
    float c, s;
    float len = (float)sqrt(a.x * a.x + a.z * a.z);
    if (len == 0.0f) {
        c = 1.0f;
        s = 0.0f;
    } else {
        c = a.z / len;
        s = a.x / len;
    }
    *yaw = (float)acos(c);
    if (a.x < 0.0f)
        *yaw = -*yaw;

    // Turn a and b about y by the heading, then take the pitch of a.
    float az = c * a.z + s * a.x;
    float bz = c * b.z + s * b.x;
    float bx = c * b.x - s * b.z;
    len = (float)sqrt(az * az + a.y * a.y);
    if (len == 0.0f) {
        *cosPitch = 1.0f;
        *sinPitch = 0.0f;
    } else {
        *cosPitch = az / len;
        *sinPitch = a.y / len;
    }
    *pitch = (float)acos(*cosPitch);
    if (a.y < 0.0f)
        *pitch = -*pitch;

    // Roll of b about the pitched axis.
    float by = b.y * *cosPitch - bz * *sinPitch;
    len = (float)sqrt(by * by + bx * bx);
    if (len == 0.0f) {
        *cosRoll = 1.0f;
        *sinRoll = 0.0f;
    } else {
        *cosRoll = by / len;
        *sinRoll = bx / len;
    }
    *roll = (float)acos(*cosRoll);
    if (bx < 0.0f)
        *roll = -*roll;

    // Near straight up/down the yaw and roll axes coincide: fold roll into yaw.
    if (AbsF(*pitch - 1.5707964f) < 0.1745f) {
        *yaw = *yaw - *roll;
        if (*yaw < 0.0)
            *yaw += 6.2831855f;
        *roll = 0.0f;
    } else if (AbsF(*pitch + 1.5707964f) < 0.1745f) {
        *yaw = *yaw + *roll;
        if (*yaw > 6.2831855f)
            *yaw -= 6.2831855f;
        *roll = 0.0f;
    }
    return 1;
}
